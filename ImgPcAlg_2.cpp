#include "ImgPcAlg.h"

// --- GLCM 及其优化实现 ---

namespace GLCM {

        // 内部私有函数：计算相位谱 (Phase Spectrum)
        // 移入匿名命名空间，限制作用域
        namespace {
        cv::UMat getPhaseSpec(cv::InputArray src, int grayLevels) {
            cv::UMat fSrc;
            src.getUMat().convertTo(fSrc, CV_32F);

            // 性能优化：填充到 DFT 最优尺寸
            int w = cv::getOptimalDFTSize(fSrc.cols);
            int h = cv::getOptimalDFTSize(fSrc.rows);
            cv::UMat padded;
            cv::copyMakeBorder(fSrc, padded, 0, h - fSrc.rows, 0, w - fSrc.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

            cv::UMat complexImg;
            cv::dft(padded, complexImg, cv::DFT_COMPLEX_OUTPUT);

            std::vector<cv::UMat> planes;
            cv::split(complexImg, planes);
            cv::UMat phase;
            cv::phase(planes[0], planes[1], phase);

            // 裁剪回有效区域并归一化
            phase = phase(cv::Rect(0, 0, fSrc.cols, fSrc.rows));
            cv::normalize(phase, phase, 0, grayLevels - 1, cv::NORM_MINMAX);

            cv::UMat phaseUint;
            phase.convertTo(phaseUint, CV_8U);
            return phaseUint;
        }
    }


    void ImageResizer::process(const cv::UMat& src, cv::UMat& dst, ScaleStrategy strategy) {
        if (src.empty()) return;

        cv::Size newSize = src.size();

        if (strategy == ScaleStrategy::ToPowerOfTwo) {
            // 使用位运算找到 <= 当前值的最大 2 的幂次
            // 这种方式比 std::pow 更高效、更符合嵌入式或高性能计算习惯
            auto floorPowerOfTwo = [](int n) -> int {
                if (n <= 0) return 0;
                // 如果已经是 2 的幂次，直接返回
                if ((n & (n - 1)) == 0) return n;
                // 位操作：填充所有低位为 1，然后减去一半
                n |= n >> 1;
                n |= n >> 2;
                n |= n >> 4;
                n |= n >> 8;
                n |= n >> 16;
                return n - (n >> 1);
            };

            int w = floorPowerOfTwo(src.cols);
            int h = floorPowerOfTwo(src.rows);

            // 阈值保护：至少保留 16 像素，防止纹理信息完全丢失
            newSize.width = std::max(16, w);
            newSize.height = std::max(16, h);
        }
        // ByFactor 策略则沿用 BaseAlg 的逻辑，或在这里实现具体的比例缩放

        if (newSize != src.size()) {
            // 使用 INTER_AREA，这是下采样公认质量最好的插值方式
            cv::resize(src, dst, newSize, 0, 0, cv::INTER_AREA);
        } else {
            src.copyTo(dst);
        }

    };

    struct GLCMParallelTask : public cv::ParallelLoopBody {
        const cv::Mat& img;
        int dx, dy, levels;
        std::vector<cv::Mat>& localHistograms;

        GLCMParallelTask(const cv::Mat& i, int _dx, int _dy, int l, std::vector<cv::Mat>& h)
            : img(i), dx(_dx), dy(_dy), levels(l), localHistograms(h) {
        }

        void operator()(const cv::Range& range) const override {
            // 关键：使用 getThreadNum 必须保证 localHistograms 的 size 等于 getNumThreads
            int threadId = cv::getThreadNum();
            cv::Mat& myMat = localHistograms[threadId];

            for (int y = range.start; y < range.end; ++y) {
                int ty = y + dy;
                if (ty < 0 || ty >= img.rows) continue;

                const uchar* pSrc = img.ptr<uchar>(y);
                const uchar* pTar = img.ptr<uchar>(ty);

                for (int x = 0; x < img.cols; ++x) {
                    int tx = x + dx;
                    if (tx < 0 || tx >= img.cols) continue;

                    // 安全量化：防止索引超出 [0, levels-1]
                    uchar v1 = std::min((int)pSrc[x], levels - 1);
                    uchar v2 = std::min((int)pTar[tx], levels - 1);
                    myMat.at<float>(v1, v2)++;
                }
            }
        }
    };

    GLCmat::GLCmat(cv::InputArray img, int levels, int dx, int dy)
        : m_levels(levels)
    {
        cv::Mat mat = img.getMat();
        // 核心修正：如果图片值范围不是 [0, levels-1]，必须映射
        double minV, maxV;
        cv::minMaxLoc(mat, &minV, &maxV);
        if (maxV >= levels || minV < 0 || mat.type() != CV_8U) {
            double scale = (levels - 1.0) / (std::max(maxV - minV, 1.0));
            mat.convertTo(mat, CV_8U, scale, -minV * scale);
        }

        // 执行计算
        computeParallel(mat, dx, dy);

        // 修正 5：计算均值和方差，否则后续 getCorrelation 为 0
        computeStatistics();
    }

    void GLCmat::computeParallel(const cv::Mat& img, int dx, int dy) {
        // 1. 增加安全裕量。虽然线程通常不多，但分配 128 或 256 个 Mat 头的开销几乎为 0
        // 这能有效防止 getThreadNum() 返回异常索引
        int safety_size = std::max(cv::getNumThreads() + 64, 128);

        std::vector<cv::Mat> locals(safety_size);
        for (int i = 0; i < safety_size; ++i) {
            locals[i] = cv::Mat::zeros(m_levels, m_levels, CV_32F);
        }

        cv::parallel_for_(cv::Range(0, img.rows), [&](const cv::Range& range) {
            int threadId = cv::getThreadNum();
            // 二次防御：万一索引真的逆天了，我们做个检查
            if (threadId < 0 || threadId >= safety_size) return;

            cv::Mat& myMat = locals[threadId];
            for (int y = range.start; y < range.end; ++y) {
                int ty = y + dy;
                if (ty < 0 || ty >= img.rows) continue;
                const uchar* pSrc = img.ptr<uchar>(y);
                const uchar* pTar = img.ptr<uchar>(ty);
                for (int x = 0; x < img.cols; ++x) {
                    int tx = x + dx;
                    if (tx < 0 || tx >= img.cols) continue;
                    int v1 = std::min((int)pSrc[x], m_levels - 1);
                    int v2 = std::min((int)pTar[tx], m_levels - 1);
                    myMat.at<float>(v1, v2)++;
                }
            }
        });

        // 4. 汇总：将各线程的局部结果累加到主矩阵 m_glcm
        m_glcm = cv::Mat::zeros(m_levels, m_levels, CV_32F);
        for (const auto& local_m : locals) {
            m_glcm += local_m;
        }

        // 5. 归一化：将频次转化为概率
        double total_sum = cv::sum(m_glcm)[0];
        if (total_sum > 1e-9) {
            m_glcm /= total_sum;
        }
    }

    void GLCmat::computeStatistics() {
        // 合并循环以提高缓存命中率
        // 预计算边际概率 Px(i) 和 Py(j) 也是一种方法，但在小矩阵(256x256)下直接遍历是可以接受的

        m_meanX = 0; m_meanY = 0;
        m_varX = 0; m_varY = 0;

        // 计算均值
        for (int i = 0; i < m_levels; ++i) {
            const float* ptr = m_glcm.ptr<float>(i);
            for (int j = 0; j < m_levels; ++j) {
                float p = ptr[j];
                if (p <= 0) continue; // Skip zero probabilities
                m_meanX += i * p;
                m_meanY += j * p;
            }
        }

        // 计算方差
        for (int i = 0; i < m_levels; ++i) {
            const float* ptr = m_glcm.ptr<float>(i);
            for (int j = 0; j < m_levels; ++j) {
                float p = ptr[j];
                if (p <= 0) continue;
                m_varX += p * (i - m_meanX) * (i - m_meanX);
                m_varY += p * (j - m_meanY) * (j - m_meanY);
            }
        }
    }

    double GLCmat::getCorrelation() const {
        double cov = 0.0;
        for (int i = 0; i < m_levels; ++i) {
            const float* ptr = m_glcm.ptr<float>(i);
            for (int j = 0; j < m_levels; ++j) {
                if (ptr[j] > 0)
                    cov += ptr[j] * (i - m_meanX) * (j - m_meanY);
            }
        }
        double stdDevProduct = std::sqrt(m_varX * m_varY);
        return (stdDevProduct > 1e-9) ? (cov / stdDevProduct) : 0.0;
    }

    double GLCmat::getHomogeneity() const {
        double homo = 0.0;
        for (int i = 0; i < m_levels; ++i) {
            const float* ptr = m_glcm.ptr<float>(i);
            for (int j = 0; j < m_levels; ++j) {
                if (ptr[j] > 0)
                    homo += ptr[j] / (1.0 + std::abs(i - j));
            }
        }
        return homo;
    }

    GLCMAlg::GLCMAlg(cv::InputArray img, int levels, int dx, int dy, ScaleStrategy strategy) {
        m_processInput = false;

        cv::UMat uSrc = img.getUMat();
        // 优化点：在做 DFT (PhaseSpec) 之前先 Resize，极大地加速 DFT
        // 相位谱对尺度变化相对鲁棒，先下采样再做 DFT 是很常见的优化
        cv::UMat resizedSrc;
        ImageResizer::process(uSrc, resizedSrc, strategy);

        // 计算相位谱
        cv::UMat phase = getPhaseSpec(resizedSrc, levels);

        // 生成 GLCM (注意这里传入 None，因为我们已经在上面 Resize 过了)
        m_glcmPtr = std::make_unique<GLCmat>(phase, levels, dx, dy);
    }
}

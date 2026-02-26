#include "ImgPcAlg.h"

QString CORRNAME = "GLCMcorr",
        HOMONAME = "GLCMhomo";

// --- GLCM 及其优化实现 ---

namespace GLCM
{

    // 内部辅助：计算相位谱
    static cv::UMat getPhaseSpecInternal(cv::InputArray src, int grayLevels)
    {
        cv::UMat fSrc;
        src.getUMat().convertTo(fSrc, CV_32F);

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

        phase = phase(cv::Rect(0, 0, fSrc.cols, fSrc.rows));
        // 映射到灰度级
        cv::normalize(phase, phase, 0, grayLevels - 1, cv::NORM_MINMAX);

        cv::UMat phaseUint;
        phase.convertTo(phaseUint, CV_8U);
        return phaseUint;
    }

    std::shared_ptr<GLCmat> getPSGLCM(cv::InputArray img, int levels, int dx, int dy, ScaleStrategy strategy)
    {
        cv::UMat processed;
        if (strategy == ScaleStrategy::ToPowerOfTwo) {
            int w = 1 << (int)std::floor(std::log2(img.cols()));
            int h = 1 << (int)std::floor(std::log2(img.rows()));
            cv::resize(img, processed, cv::Size(std::max(16, w), std::max(16, h)), 0, 0, cv::INTER_AREA);
        }
        else {
            processed = img.getUMat();
        }

        cv::UMat phase = getPhaseSpecInternal(processed, levels);
        return std::make_shared<GLCmat>(phase, levels, dx, dy);
    }

    GLCmat::GLCmat(cv::InputArray img, int levels, int dx, int dy) : m_levels(levels)
    {
        cv::Mat mat = img.getMat();
        if (mat.type() != CV_8U) {
            double minV, maxV;
            cv::minMaxLoc(mat, &minV, &maxV);
            double scale = (levels - 1.0) / std::max(maxV - minV, 1.0);
            mat.convertTo(mat, CV_8U, scale, -minV * scale);
        }
        computeGLCM(mat, dx, dy);
        computeStatistics();
    }

    void GLCmat::computeGLCM(const cv::Mat& img, int dx, int dy)
    {
        m_glcm = cv::Mat::zeros(m_levels, m_levels, CV_32F);

        for (int y = 0; y < img.rows; ++y) {
            int ty = y + dy;
            if (ty < 0 || ty >= img.rows) continue;

            const uchar* pSrc = img.ptr<uchar>(y);
            const uchar* pTar = img.ptr<uchar>(ty);

            for (int x = 0; x < img.cols; ++x) {
                int tx = x + dx;
                if (tx < 0 || tx >= img.cols) continue;

                int i = pSrc[x] % m_levels;
                int j = pTar[tx] % m_levels;
                m_glcm.at<float>(i, j)++;
            }
        }

        double sumVal = cv::sum(m_glcm)[0];
        if (sumVal > 1e-9) m_glcm /= sumVal;
    }

    void GLCmat::computeStatistics()
    {
        // 关键优化：使用 cv::reduce 快速计算边际分布
        cv::Mat pX, pY;
        cv::reduce(m_glcm, pY, 1, cv::REDUCE_SUM); // 对行求和 -> P(i)
        cv::reduce(m_glcm, pX, 0, cv::REDUCE_SUM); // 对列求和 -> P(j)

        m_meanX = 0; m_meanY = 0;
        m_varX = 0; m_varY = 0;

        for (int i = 0; i < m_levels; ++i) {
            float valX = pX.at<float>(0, i);
            float valY = pY.at<float>(i, 0);
            m_meanX += i * valX;
            m_meanY += i * valY;
        }

        for (int i = 0; i < m_levels; ++i) {
            m_varX += std::pow(i - m_meanX, 2) * pX.at<float>(0, i);
            m_varY += std::pow(i - m_meanY, 2) * pY.at<float>(i, 0);
        }
    }

    double GLCmat::getCorrelation() const
    {
        double cov = 0.0;
        for (int i = 0; i < m_levels; ++i) {
            const float* ptr = m_glcm.ptr<float>(i);
            for (int j = 0; j < m_levels; ++j) {
                if (ptr[j] > 1e-12)
                    cov += ptr[j] * (i - m_meanX) * (j - m_meanY);
            }
        }
        double sigma = std::sqrt(m_varX * m_varY);
        return (sigma > 1e-9) ? (cov / sigma) : 1.0; // 若方差为0，说明完全一致，相关性应为1而非0
    }

    double GLCmat::getHomogeneity() const
    {
        double homo = 0.0;
        for (int i = 0; i < m_levels; ++i) {
            const float* ptr = m_glcm.ptr<float>(i);
            for (int j = 0; j < m_levels; ++j) {
                homo += ptr[j] / (1.0 + std::abs(i - j));
            }
        }
        return homo;
    }

    GLCMAlg::GLCMAlg(cv::InputArray img, int levels, int dx, int dy, ScaleStrategy strategy)
    {
        m_glcmPtr = getPSGLCM(img, levels, dx, dy, strategy);
        processInput = false;
    }
}

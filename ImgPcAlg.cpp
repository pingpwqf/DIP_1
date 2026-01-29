#include "ImgPcAlg.h"

#include <QDebug>

// --- 基础工具实现 ---

void AlgInterface::CheckAlg() const
{
    if(!name.isEmpty()) qDebug() << name;
    else qDebug() << "this algorithm is not expected to own a name";
}

BaseAlg::BaseAlg(cv::InputArray img, int f) : m_factor(std::max(1, f)) {
    if (img.empty()) throw std::invalid_argument("Reference image is empty.");

    // 使用 CV_32F 确保精度，这对后续计算至关重要
    img.getUMat().convertTo(m_refImg, CV_32F);
    downsample(m_refImg, m_downRef);
}

void BaseAlg::validate(const cv::UMat& input) const {
    if (input.empty() || input.size() != m_refImg.size())
        throw std::invalid_argument("Input size mismatch.");
}

void BaseAlg::downsample(const cv::UMat& src, cv::UMat& dst) const {
    if (m_factor > 1) {
        // INTER_AREA 是下采样最好的插值方式，避免波纹效应
        cv::resize(src, dst, cv::Size(src.cols / m_factor, src.rows / m_factor), 0, 0, cv::INTER_AREA);
    }
    else {
        src.copyTo(dst);
    }
}

cv::UMat BaseAlg::prepareInput(cv::InputArray input) const {
    cv::UMat in = input.getUMat();
    validate(in);
    if (in.type() != CV_32F) {
        in.convertTo(in, CV_32F);
    }
    return in;
}

// --- 具体算法 (NIPC, ZNCC, MSV) ---

NIPCAlg::NIPCAlg(cv::InputArray img, int f) : BaseAlg(img, f) {
    name = NIPCNAME;
    m_refNorm = cv::norm(m_downRef, cv::NORM_L2);
    if (m_refNorm < 1e-9) throw std::runtime_error("Reference image has zero norm (black image?).");
}

double NIPCAlg::process(cv::InputArray input) const {
    cv::UMat inDown;
    downsample(prepareInput(input), inDown);

    double inNorm = cv::norm(inDown, cv::NORM_L2);
    // 避免除以零
    if (inNorm < 1e-9) return 0.0;

    // dot product
    return m_downRef.dot(inDown) / (m_refNorm * inNorm);
}

double ZNCCAlg::process(cv::InputArray input) const {
    cv::UMat inputImg = prepareInput(input);
    cv::UMat downInput;
    downsample(inputImg, downInput);

    // 输入图也需要减均值吗？标准的 ZNCC 公式通常在 matchTemplate 内部处理了归一化
    // TM_CCOEFF_NORMED 内部会处理均值移除和方差归一化

    cv::UMat result;
    cv::matchTemplate(downInput, m_downRef, result, cv::TM_CCOEFF_NORMED);

    // 优化：仅在必要时从 GPU 下载数据
    float score = 0;
    result.copyTo(cv::Mat(1, 1, CV_32F, &score));

    // 边界处理：如果输入是纯色图，结果可能是 NaN
    return std::isnan(score) ? 0.0 : static_cast<double>(score);
}

double MSVAlg::process(cv::InputArray input) const {
    cv::UMat in = prepareInput(input);
    // 使用 L1 范数代替循环，极大提升速度
    return cv::norm(m_refImg, in, cv::NORM_L1) / static_cast<double>(m_refImg.total());
}

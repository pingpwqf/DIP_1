#include "ImgPcAlg.h"

QString MSVNAME = "MSV",
    NIPCNAME = "NIPC",
    ZNCCNAME = "ZNCC";

BaseAlg::BaseAlg(cv::InputArray img, int f) : m_factor(std::max(1, f)) {
    if (img.empty()) throw std::invalid_argument("Reference image is empty.");
    img.getUMat().convertTo(m_refImg, CV_32F);
    downsample(m_refImg, m_downRef);
}

void BaseAlg::downsample(const cv::UMat& src, cv::UMat& dst) const {
    if (m_factor > 1) {
        cv::resize(src, dst, cv::Size(src.cols / m_factor, src.rows / m_factor), 0, 0, cv::INTER_AREA);
    }
    else {
        src.copyTo(dst);
    }
}

cv::UMat BaseAlg::prepareInput(cv::InputArray input) const {
    cv::UMat in = input.getUMat();
    if (in.size() != m_refImg.size()) throw std::invalid_argument("Input size mismatch.");
    if (in.type() != CV_32F) in.convertTo(in, CV_32F);
    return in;
}

// NIPC: 归一化图像相位相关
NIPCAlg::NIPCAlg(cv::InputArray img, int f) : BaseAlg(img, f) {
    m_refNorm = cv::norm(m_downRef, cv::NORM_L2);
    if (m_refNorm < 1e-9) throw std::runtime_error("Reference image is invalid (too dark).");
}

double NIPCAlg::process(cv::InputArray input) const {
    ensureInputNotEmpty(input);
    cv::UMat inDown;
    downsample(prepareInput(input), inDown);
    double inNorm = cv::norm(inDown, cv::NORM_L2);
    if (inNorm < 1e-9) return 0.0;
    return m_downRef.dot(inDown) / (m_refNorm * inNorm);
}

// ZNCC: 零均值归一化互相关 (优化 GPU 提取分数)
double ZNCCAlg::process(cv::InputArray input) const {
    ensureInputNotEmpty(input);
    cv::UMat downInput;
    downsample(prepareInput(input), downInput);

    cv::UMat result;
    cv::matchTemplate(downInput, m_downRef, result, cv::TM_CCOEFF_NORMED);

    // 关键优化：直接使用 minMaxLoc 拿结果，避免显存到内存的碎片拷贝
    double maxVal;
    cv::minMaxLoc(result, nullptr, &maxVal);
    return std::isnan(maxVal) ? 0.0 : maxVal;
}

// MSV: 平均绝对差
double MSVAlg::process(cv::InputArray input) const {
    ensureInputNotEmpty(input);
    cv::UMat in = prepareInput(input);
    return cv::norm(m_refImg, in, cv::NORM_L1) / static_cast<double>(m_refImg.total());
}

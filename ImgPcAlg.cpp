#include "ImgPcAlg.h"
#include <QDebug>

void BaseAlg::downsample()
{
    cv::resize(refImg, downRef, cv::Size(refImg.cols / factor, refImg.rows / factor), 0, 0, cv::INTER_AREA);
}

double MSVAlg::process(const cv::Mat& inputImg) const
{
    if(refImg.size() != inputImg.size()) {
        throw std::invalid_argument("Input image size does not match reference image size.");
    }
    return cv::norm(refImg, inputImg, cv::NORM_L1);
}

void MSVAlg::checkAlg() const
{
    qDebug() << "MSV";
}

double NIPCAlg::process(const cv::Mat& inputImg) const
{
    if(refImg.size() != inputImg.size()) {
        throw std::invalid_argument("Input image size does not match reference image size.");
    }
    cv::Mat result, downImg;
    cv::resize(inputImg, downImg, cv::Size(inputImg.cols / factor, inputImg.rows / factor), 0, 0, cv::INTER_AREA);
    cv::matchTemplate(downRef, downImg, result, cv::TM_CCORR_NORMED);
    return static_cast<double>(result.at<float>(0, 0));
}

void NIPCAlg::checkAlg() const
{
    qDebug() << "NIPC";
}

double ZNCCAlg::process(const cv::Mat& inputImg) const
{
    if(refImg.size() != inputImg.size()) {
        throw std::invalid_argument("Input image size does not match reference image size.");
    }
    cv::Mat result, downImg;
    cv::resize(inputImg, downImg, cv::Size(inputImg.cols / factor, inputImg.rows / factor), 0, 0, cv::INTER_AREA);
    cv::matchTemplate(downRef, downImg, result, cv::TM_CCOEFF_NORMED);
    return static_cast<double>(result.at<float>(0, 0));
}

void ZNCCAlg::checkAlg() const
{
    qDebug() << "ZNCC";
}

std::unique_ptr<BaseAlg> createMSVAlgProcessor(cv::Mat image)
{
    std::unique_ptr<BaseAlg> BasePtr = std::make_unique<MSVAlg>(image);
    return std::move(BasePtr);
}

std::unique_ptr<BaseAlg> createNIPCAlgProcessor(cv::Mat image)
{
    std::unique_ptr<BaseAlg> BasePtr = std::make_unique<NIPCAlg>(image);
    return std::move(BasePtr);
}

std::unique_ptr<BaseAlg> createZNCCAlgProcessor(cv::Mat image)
{
    std::unique_ptr<BaseAlg> BasePtr = std::make_unique<ZNCCAlg>(image);
    return std::move(BasePtr);
}

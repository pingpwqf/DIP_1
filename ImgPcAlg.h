#pragma once
#include <opencv2/opencv.hpp>

class BaseAlg
{
protected:
    cv::Mat refImg;

    int factor = 2;
    cv::Mat downRef;
    void downsample();

public:
    BaseAlg(const cv::Mat img) : refImg(img) {};
    BaseAlg() = default;
    virtual ~BaseAlg() = default;

    BaseAlg(const BaseAlg&) = delete;
    BaseAlg operator=(const BaseAlg&) = delete;

    virtual double process(const cv::Mat&) const = 0;
    virtual void checkAlg() const = 0;
};

class MSVAlg : public BaseAlg
{
public:
    MSVAlg() = default;
    MSVAlg(const cv::Mat img) : BaseAlg(img) {};
    virtual ~MSVAlg() = default;
    MSVAlg(const MSVAlg&) = delete;
    MSVAlg operator=(const MSVAlg&) = delete;
    virtual double process(const cv::Mat& input) const override;
    virtual void checkAlg() const override;
};

class NIPCAlg : public BaseAlg
{
public:
    NIPCAlg() = default;
    NIPCAlg(const cv::Mat img) : BaseAlg(img) {
        downsample();
    };
    virtual ~NIPCAlg() = default;
    NIPCAlg(const NIPCAlg&) = delete;
    NIPCAlg operator=(const NIPCAlg&) = delete;
    virtual double process(const cv::Mat& input) const override;
    virtual void checkAlg() const override;
};

class ZNCCAlg : public BaseAlg
{
public:
    ZNCCAlg() = default;
    ZNCCAlg(const cv::Mat img) : BaseAlg(img) {
        downsample();
    };
    virtual ~ZNCCAlg() = default;
    ZNCCAlg(const ZNCCAlg&) = delete;
    ZNCCAlg operator=(const ZNCCAlg&) = delete;
    virtual double process(const cv::Mat& input) const override;
    virtual void checkAlg() const override;
};

std::unique_ptr<BaseAlg> createMSVAlgProcessor(cv::Mat);
std::unique_ptr<BaseAlg> createNIPCAlgProcessor(cv::Mat);
std::unique_ptr<BaseAlg> createZNCCAlgProcessor(cv::Mat);

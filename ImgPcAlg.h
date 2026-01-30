#pragma once
#include <opencv2/opencv.hpp>
#include <QString>
#include <QVector>
#include <QRunnable>
#include <memory>


QString MSVNAME = "MSV",
    NIPCNAME = "NIPC",
    ZNCCNAME = "ZNCC",
    CORRNAME = "GLCMcorr",
    HOMONAME = "GLCMhomo";

// 策略枚举：控制图像预处理的缩放行为
enum class ScaleStrategy {
    None,
    ToPowerOfTwo,      // 下采样至最近的 2 的幂次 (你的需求)
    ByFactor       // 按指定因子缩放
};

// 接口层
class AlgInterface : public QRunnable {
public:
    virtual ~AlgInterface() = default;
    virtual double process(cv::InputArray input) const = 0;
    virtual double process() const = 0;
    bool expectInput() const { return m_processInput; }
    void checkAlg() const;
    void run();
    QString Name(){return name;}

protected:
    AlgInterface() = default;
    bool m_processInput = true;
    QString name = "No name";
};

class BaseAlg : public AlgInterface {
public:
    BaseAlg(const BaseAlg&) = delete;
    BaseAlg& operator=(const BaseAlg&) = delete;
    virtual ~BaseAlg() = default;

    void validate(const cv::UMat& input) const;
    double process(cv::InputArray input) const override = 0;
    double process() const override {
        throw std::logic_error("This algorithm requires an input image.");
    }

protected:
    explicit BaseAlg(cv::InputArray img, int f = 2);

    // 增加 const 正确性与引用传递
    void downsample(const cv::UMat& src, cv::UMat& dst) const;
    cv::UMat prepareInput(cv::InputArray input) const;

    int m_factor;
    cv::UMat m_refImg;
    cv::UMat m_downRef;
};

class MSVAlg : public BaseAlg
{
public:
    explicit MSVAlg(cv::InputArray img, int f = 2) :BaseAlg(img, f) {
        name = MSVNAME;
    }
    double process(cv::InputArray input) const override;
};

class NIPCAlg : public BaseAlg
{
public:
    NIPCAlg(cv::InputArray img, int f = 2);
    double process(cv::InputArray input) const override;
private:
    double m_refNorm;
};

class ZNCCAlg : public BaseAlg
{
public:
    explicit ZNCCAlg(cv::InputArray img, int f = 2) :BaseAlg(img, f) {
        name = ZNCCNAME;
    }
    double process(cv::InputArray input) const override;
};

// GLCM 模块
namespace GLCM {

// 辅助工具：处理尺寸调整
class ImageResizer {
public:
    // 核心功能：根据策略调整图像大小
    static void process(const cv::UMat& src, cv::UMat& dst, ScaleStrategy strategy);
};

class GLCmat {
public:
    // 构造函数增加 resizeStrategy 选项
    GLCmat(cv::InputArray img, int levels, int dx, int dy);

    double getCorrelation() const;
    double getHomogeneity() const;

private:
    cv::Mat m_glcm;
    int m_levels;
    double m_meanX = 0, m_meanY = 0;
    double m_varX = 0, m_varY = 0;

    void computeParallel(const cv::Mat& img, int dx, int dy);
    void computeStatistics(); // 将统计计算独立出来
};

class GLCMAlg : public AlgInterface {
public:
    // 增加策略参数，默认使用 PowerOfTwo 以提升大图的 DFT 速度
    GLCMAlg(cv::InputArray img, int levels, int dx, int dy, ScaleStrategy strategy);

    double process(cv::InputArray) const final {
        throw std::logic_error("This algorithm uses pre-calculated reference.");
    }
    virtual double process() const override = 0;

protected:
    std::unique_ptr<GLCmat> m_glcmPtr;
};

class GLCMcorrAlg final : public GLCMAlg {
public:
    GLCMcorrAlg(cv::InputArray img, int levels = 16, int dx = 1, int dy = 0,
                ScaleStrategy strategy = ScaleStrategy::ToPowerOfTwo)
        : GLCMAlg(img, levels, dx, dy, strategy){name = CORRNAME;}
    double process() const override { return m_glcmPtr->getCorrelation(); }
};

class GLCMhomoAlg final : public GLCMAlg {
public:
    GLCMhomoAlg(cv::InputArray img, int levels = 16, int dx = 1, int dy = 0,
                ScaleStrategy strategy = ScaleStrategy::ToPowerOfTwo)
        : GLCMAlg(img, levels, dx, dy, strategy){name = HOMONAME;}
    double process() const override { return m_glcmPtr->getHomogeneity(); }
};
}

template<typename T>
class AlgRegistry
{
public:
    using Creator = std::function<std::unique_ptr<AlgInterface>(cv::InputArray)>;

    static AlgRegistry instance()
    {
        static AlgRegistry reg;
        return reg;
    }
    void Register(T a_name, Creator creator)
    {
        nameList.append(a_name);
        storage[a_name] = creator;
    }
    std::unique_ptr<AlgInterface> get(T a_name, cv::InputArray img){
        if(storage.find(a_name) != storage.end()) return storage[a_name](img);
        return nullptr;
    }
    QVector<T> names()
    {
        return nameList;
    }

private:
    QVector<T> nameList;
    std::unordered_map<T, Creator> storage;
};

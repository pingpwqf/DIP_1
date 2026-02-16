#pragma once
#include <opencv2/opencv.hpp>
#include <memory>
#include <cmath>
#include <stdexcept>
#include <QVector>
#include <QString>
#include <QDebug>

extern QString MSVNAME,NIPCNAME,ZNCCNAME,
        CORRNAME,HOMONAME;

// 策略枚举
enum class ScaleStrategy {
    None,
    ToPowerOfTwo, // 适配 DFT 性能
    ByFactor      // 固定比例
};

// 接口层：统一处理逻辑
class AlgInterface {
public:
    virtual ~AlgInterface() = default;

    /**
     * @brief 统一执行接口
     * @param input 输入图像。若算法内置了参考图或 GLCM，则该参数可为空（cv::noArray()）
     */
    virtual double process(cv::InputArray input = cv::noArray()) const = 0;
    bool expectInput() const {return processInput;}

protected:
    AlgInterface() = default;
    bool processInput = true;
};

// 基础算法类：负责参考图管理与下采样
class BaseAlg : public AlgInterface {
public:
    BaseAlg(const BaseAlg&) = delete;
    BaseAlg& operator=(const BaseAlg&) = delete;
    virtual ~BaseAlg() = default;

protected:
    explicit BaseAlg(cv::InputArray img, int f = 2);

    void downsample(const cv::UMat& src, cv::UMat& dst) const;
    cv::UMat prepareInput(cv::InputArray input) const;

    // 检查输入有效性
    void ensureInputNotEmpty(cv::InputArray input) const {
        if (input.empty()) throw std::invalid_argument("Input image is required for this algorithm.");
    }

    int m_factor;
    cv::UMat m_refImg;
    cv::UMat m_downRef;
};

// 互相关相关算法实现
class NIPCAlg final : public BaseAlg {
public:
    NIPCAlg(cv::InputArray img, int f = 2);
    double process(cv::InputArray input = cv::noArray()) const override;
private:
    double m_refNorm;
};

class ZNCCAlg final : public BaseAlg {
public:
    ZNCCAlg(cv::InputArray img, int f = 2) : BaseAlg(img, f) {}
    double process(cv::InputArray input = cv::noArray()) const override;
};

class MSVAlg final : public BaseAlg {
public:
    MSVAlg(cv::InputArray img, int f = 2) : BaseAlg(img, f) {}
    double process(cv::InputArray input = cv::noArray()) const override;
};

// GLCM 模块：独立命名空间
namespace GLCM {
    class GLCmat {
    public:
        GLCmat(cv::InputArray img, int levels, int dx, int dy);
        double getCorrelation() const;
        double getHomogeneity() const;

    private:
        cv::Mat m_glcm;
        int m_levels;
        double m_meanX = 0, m_meanY = 0;
        double m_varX = 0, m_varY = 0;

        void computeGLCM(const cv::Mat& img, int dx, int dy);
        void computeStatistics();
    };

    std::shared_ptr<GLCmat> getPSGLCM(cv::InputArray img, int levels, int dx, int dy, ScaleStrategy strategy);

    class GLCMAlg : public AlgInterface {
    public:
        GLCMAlg(cv::InputArray img, int levels = 8, int dx = 1, int dy = 0, ScaleStrategy strategy = ScaleStrategy::ToPowerOfTwo);
        explicit GLCMAlg(std::shared_ptr<GLCmat> glcmPtr) : m_glcmPtr(glcmPtr) {}

    protected:
        std::shared_ptr<GLCmat> m_glcmPtr;
    };

    class GLCMcorrAlg final : public GLCMAlg {
    public:
        using GLCMAlg::GLCMAlg;
        double process(cv::InputArray = cv::noArray()) const override { return m_glcmPtr->getCorrelation(); }
    };

    class GLCMhomoAlg final : public GLCMAlg {
    public:
        using GLCMAlg::GLCMAlg;
        double process(cv::InputArray = cv::noArray()) const override { return m_glcmPtr->getHomogeneity(); }
    };
}

template<typename T>
class AlgRegistry
{
public:
    using Creator = std::function<std::shared_ptr<AlgInterface>(cv::InputArray)>;

    static AlgRegistry& instance()
    {
        static AlgRegistry reg;
        return reg;
    }

    void Register(T a_name, Creator creator)
    {
        if(!nameList.contains(a_name)) nameList.emplaceBack(a_name);
        storage[a_name] = creator;
    }

    std::shared_ptr<AlgInterface> get(T a_name, cv::InputArray img){
        if(storage.find(a_name) != storage.end()) return storage[a_name](img);
        else {
            qDebug() << "storage is full";
            return nullptr;
        }
    }

    QVector<T> names() const { return nameList; }

private:
    QVector<T> nameList;
    std::unordered_map<T, Creator> storage;
};

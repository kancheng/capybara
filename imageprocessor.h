#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QString>
#include <QImage>
#include <QObject>
#ifdef HAVE_OPENCV
#include <opencv2/opencv.hpp>
#endif

// 圖像處理器類 / Image processor class
class ImageProcessor : public QObject
{
    Q_OBJECT

public:
    explicit ImageProcessor(QObject *parent = nullptr);
    
#ifdef HAVE_OPENCV
    // 載入圖像 / Load image
    cv::Mat loadImage(const QString &path);
    
    // 將 cv::Mat 轉換為 QImage / Convert cv::Mat to QImage
    static QImage matToQImage(const cv::Mat &mat);
    
    // 將 QImage 轉換為 cv::Mat / Convert QImage to cv::Mat
    static cv::Mat qImageToMat(const QImage &image);
    
    // OpenCV 操作函數 / OpenCV operation functions
    
    // 高斯模糊 / Gaussian blur
    static cv::Mat applyGaussianBlur(const cv::Mat &input, int ksize, double sigmaX, double sigmaY = 0);
    
    // Canny 邊緣檢測 / Canny edge detection
    static cv::Mat applyCanny(const cv::Mat &input, double threshold1, double threshold2, int apertureSize = 3);
    
    // 亮度對比度調整 / Brightness and contrast adjustment
    static cv::Mat applyBrightnessContrast(const cv::Mat &input, double alpha, int beta);
    
    // Gamma 校正 / Gamma correction
    static cv::Mat applyGamma(const cv::Mat &input, double gamma);
    
    // 顏色空間轉換 / Color space conversion
    static cv::Mat convertColor(const cv::Mat &input, int code);
    
    // 調整大小 / Resize
    static cv::Mat resize(const cv::Mat &input, int width, int height, int interpolation = cv::INTER_LINEAR);
    
    // 旋轉 / Rotate
    static cv::Mat rotate(const cv::Mat &input, double angle);
    
    // 翻轉 / Flip
    static cv::Mat flip(const cv::Mat &input, int flipCode);
    
    // 閾值處理 / Threshold
    static cv::Mat threshold(const cv::Mat &input, double thresh, double maxval, int type);
    
    // 形態學操作 / Morphological operations
    static cv::Mat morphologyEx(const cv::Mat &input, int op, int shape, int ksize);
    
    // 保存圖像 / Save image
    static bool saveImage(const cv::Mat &image, const QString &path, const QVector<int> &params = QVector<int>());

private:
    cv::Mat currentImage;
#endif
};

#endif // IMAGEPROCESSOR_H

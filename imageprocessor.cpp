#include "imageprocessor.h"
#include <QDebug>
#ifdef HAVE_OPENCV
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>
#endif

ImageProcessor::ImageProcessor(QObject *parent)
    : QObject(parent)
{
}

#ifdef HAVE_OPENCV
cv::Mat ImageProcessor::loadImage(const QString &path)
{
    cv::Mat img = cv::imread(path.toStdString(), cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        qDebug() << "無法載入圖像 / Failed to load image:" << path;
    }
    return img;
}

QImage ImageProcessor::matToQImage(const cv::Mat &mat)
{
    if (mat.empty()) {
        return QImage();
    }
    
    cv::Mat temp;
    
    // 根據通道數和深度轉換 / Convert based on channels and depth
    switch (mat.type()) {
        case CV_8UC4: {
            // BGRA to RGBA
            cv::cvtColor(mat, temp, cv::COLOR_BGRA2RGBA);
            QImage image(temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGBA8888);
            return image.copy();
        }
        case CV_8UC3: {
            // BGR to RGB
            cv::cvtColor(mat, temp, cv::COLOR_BGR2RGB);
            QImage image(temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
            return image.copy();
        }
        case CV_8UC1: {
            // Grayscale
            QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
            return image.copy();
        }
        default: {
            // 轉換為 8UC3 / Convert to 8UC3
            mat.convertTo(temp, CV_8UC3);
            cv::cvtColor(temp, temp, cv::COLOR_BGR2RGB);
            QImage image(temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
            return image.copy();
        }
    }
}

cv::Mat ImageProcessor::qImageToMat(const QImage &image)
{
    if (image.isNull()) {
        return cv::Mat();
    }
    
    QImage img = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(img.height(), img.width(), CV_8UC3, (void*)img.constBits(), img.bytesPerLine());
    cv::Mat result;
    cv::cvtColor(mat, result, cv::COLOR_RGB2BGR);
    return result.clone();
}

cv::Mat ImageProcessor::applyGaussianBlur(const cv::Mat &input, int ksize, double sigmaX, double sigmaY)
{
    if (input.empty()) return cv::Mat();
    
    cv::Mat result;
    // 確保 ksize 為奇數 / Ensure ksize is odd
    if (ksize % 2 == 0) ksize++;
    cv::GaussianBlur(input, result, cv::Size(ksize, ksize), sigmaX, sigmaY);
    return result;
}

cv::Mat ImageProcessor::applyCanny(const cv::Mat &input, double threshold1, double threshold2, int apertureSize)
{
    if (input.empty()) return cv::Mat();
    
    cv::Mat gray, result;
    if (input.channels() > 1) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input.clone();
    }
    cv::Canny(gray, result, threshold1, threshold2, apertureSize);
    return result;
}

cv::Mat ImageProcessor::applyBrightnessContrast(const cv::Mat &input, double alpha, int beta)
{
    if (input.empty()) return cv::Mat();
    
    cv::Mat result;
    input.convertTo(result, -1, alpha, beta);
    return result;
}

cv::Mat ImageProcessor::applyGamma(const cv::Mat &input, double gamma)
{
    if (input.empty()) return cv::Mat();
    
    cv::Mat lookupTable(1, 256, CV_8U);
    uchar* p = lookupTable.ptr();
    for (int i = 0; i < 256; ++i) {
        p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, gamma) * 255.0);
    }
    
    cv::Mat result;
    cv::LUT(input, lookupTable, result);
    return result;
}

cv::Mat ImageProcessor::convertColor(const cv::Mat &input, int code)
{
    if (input.empty()) return cv::Mat();
    
    cv::Mat result;
    cv::cvtColor(input, result, code);
    return result;
}

cv::Mat ImageProcessor::resize(const cv::Mat &input, int width, int height, int interpolation)
{
    if (input.empty()) return cv::Mat();
    
    cv::Mat result;
    cv::resize(input, result, cv::Size(width, height), 0, 0, interpolation);
    return result;
}

cv::Mat ImageProcessor::rotate(const cv::Mat &input, double angle)
{
    if (input.empty()) return cv::Mat();
    
    cv::Point2f center(input.cols / 2.0, input.rows / 2.0);
    cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, angle, 1.0);
    cv::Mat result;
    cv::warpAffine(input, result, rotationMatrix, input.size());
    return result;
}

cv::Mat ImageProcessor::flip(const cv::Mat &input, int flipCode)
{
    if (input.empty()) return cv::Mat();
    
    cv::Mat result;
    cv::flip(input, result, flipCode);
    return result;
}

cv::Mat ImageProcessor::threshold(const cv::Mat &input, double thresh, double maxval, int type)
{
    if (input.empty()) return cv::Mat();
    
    cv::Mat gray, result;
    if (input.channels() > 1) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input.clone();
    }
    cv::threshold(gray, result, thresh, maxval, type);
    return result;
}

cv::Mat ImageProcessor::morphologyEx(const cv::Mat &input, int op, int shape, int ksize)
{
    if (input.empty()) return cv::Mat();
    
    cv::Mat kernel = cv::getStructuringElement(shape, cv::Size(ksize, ksize));
    cv::Mat result;
    cv::morphologyEx(input, result, op, kernel);
    return result;
}

bool ImageProcessor::saveImage(const cv::Mat &image, const QString &path, const QVector<int> &params)
{
    if (image.empty()) {
        qDebug() << "無法保存空圖像 / Cannot save empty image";
        return false;
    }
    
    std::vector<int> compressionParams;
    for (int param : params) {
        compressionParams.push_back(param);
    }
    
    return cv::imwrite(path.toStdString(), image, compressionParams);
}
#endif // HAVE_OPENCV

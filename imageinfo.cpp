#include "imageinfo.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <climits>
#ifdef HAVE_OPENCV
#include <opencv2/imgcodecs.hpp>
#endif

ImageInfo ImageInfo::analyzeImage(const QString &path)
{
    ImageInfo info;
    info.path = path;
    
    // 獲取文件信息 / Get file information
    QFileInfo fileInfo(path);
    info.fileName = fileInfo.fileName();
    info.fileSize = fileInfo.size();
    info.format = fileInfo.suffix().toLower();
    
#ifdef HAVE_OPENCV
    // 使用 OpenCV 讀取圖像信息 / Use OpenCV to read image information
    cv::Mat img = cv::imread(path.toStdString(), cv::IMREAD_UNCHANGED);
    
    if (img.empty()) {
        qDebug() << "無法讀取圖像 / Failed to read image:" << path;
        return info;
    }
    
    info.width = img.cols;
    info.height = img.rows;
    info.channels = img.channels();
    info.depth = img.depth();
#else
    qDebug() << "OpenCV 未啟用，無法讀取圖像信息 / OpenCV not enabled, cannot read image info:" << path;
#endif
    
    return info;
}

QString ImageInfo::toString() const
{
    QString result;
    result += QString("文件名 / File Name: %1\n").arg(fileName);
    result += QString("路徑 / Path: %1\n").arg(path);
    result += QString("尺寸 / Size: %1 x %2\n").arg(width).arg(height);
    result += QString("通道數 / Channels: %1\n").arg(channels);
    result += QString("深度 / Depth: %1 (%2)\n").arg(depth).arg(depthToString(depth));
    result += QString("格式 / Format: %1\n").arg(format.toUpper());
    result += QString("文件大小 / File Size: %1 bytes (%2 KB)\n")
              .arg(fileSize)
              .arg(QString::number(fileSize / 1024.0, 'f', 2));
    return result;
}

QString ImageInfo::depthToString(int depth)
{
#ifdef HAVE_OPENCV
    switch (depth) {
        case CV_8U: return "8-bit unsigned";
        case CV_8S: return "8-bit signed";
        case CV_16U: return "16-bit unsigned";
        case CV_16S: return "16-bit signed";
        case CV_32S: return "32-bit signed";
        case CV_32F: return "32-bit float";
        case CV_64F: return "64-bit float";
        default: return "Unknown";
    }
#else
    Q_UNUSED(depth);
    return "Unknown (OpenCV not available)";
#endif
}

void DirectoryStats::addImage(const ImageInfo &info)
{
    if (info.width == 0 || info.height == 0) {
        return;  // 跳過無效圖像 / Skip invalid images
    }
    
    totalImages++;
    totalSize += info.fileSize;
    
    // 統計格式 / Count format
    formatCount[info.format.toUpper()]++;
    
    // 統計通道數 / Count channels
    channelCount[info.channels]++;
    
    // 統計深度類型 / Count depth type
    depthCount[info.depth]++;
    
    // 更新寬度統計 / Update width statistics
    if (info.width < minWidth) minWidth = info.width;
    if (info.width > maxWidth) maxWidth = info.width;
    
    // 更新高度統計 / Update height statistics
    if (info.height < minHeight) minHeight = info.height;
    if (info.height > maxHeight) maxHeight = info.height;
    
    // 計算平均尺寸（簡單累加，最後計算平均值）/ Calculate average size
    avgWidth = (avgWidth * (totalImages - 1) + info.width) / totalImages;
    avgHeight = (avgHeight * (totalImages - 1) + info.height) / totalImages;
}

void DirectoryStats::reset()
{
    totalImages = 0;
    formatCount.clear();
    channelCount.clear();
    depthCount.clear();
    minWidth = INT_MAX;
    maxWidth = 0;
    avgWidth = 0;
    minHeight = INT_MAX;
    maxHeight = 0;
    avgHeight = 0;
    totalSize = 0;
}

QString DirectoryStats::toString() const
{
    if (totalImages == 0) {
        return QString("無圖像文件 / No image files");
    }
    
    QString result;
    result += QString("=== 目錄統計 / Directory Statistics ===\n\n");
    result += QString("總圖像數 / Total Images: %1\n").arg(totalImages);
    result += QString("總文件大小 / Total Size: %1 bytes (%2 MB)\n\n")
              .arg(totalSize)
              .arg(QString::number(totalSize / (1024.0 * 1024.0), 'f', 2));
    
    // 格式統計 / Format statistics
    result += QString("格式分布 / Format Distribution:\n");
    for (auto it = formatCount.begin(); it != formatCount.end(); ++it) {
        result += QString("  %1: %2 張 / %2 images\n").arg(it.key()).arg(it.value());
    }
    result += "\n";
    
    // 通道數統計 / Channel count statistics
    result += QString("通道數分布 / Channel Distribution:\n");
    for (auto it = channelCount.begin(); it != channelCount.end(); ++it) {
        result += QString("  %1 通道 / %1 channels: %2 張 / %2 images\n")
                  .arg(it.key()).arg(it.value());
    }
    result += "\n";
    
    // 深度類型統計 / Depth type statistics
    result += QString("深度類型分布 / Depth Type Distribution:\n");
    for (auto it = depthCount.begin(); it != depthCount.end(); ++it) {
        result += QString("  %1: %2 張 / %2 images\n")
                  .arg(ImageInfo::depthToString(it.key()))
                  .arg(it.value());
    }
    result += "\n";
    
    // 尺寸統計 / Size statistics
    result += QString("尺寸統計 / Size Statistics:\n");
    result += QString("  寬度 / Width: 最小 %1, 最大 %2, 平均 %3\n")
              .arg(minWidth == INT_MAX ? 0 : minWidth)
              .arg(maxWidth)
              .arg(avgWidth);
    result += QString("  高度 / Height: 最小 %1, 最大 %2, 平均 %3\n")
              .arg(minHeight == INT_MAX ? 0 : minHeight)
              .arg(maxHeight)
              .arg(avgHeight);
    
    return result;
}

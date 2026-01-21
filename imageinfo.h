#ifndef IMAGEINFO_H
#define IMAGEINFO_H

#include <QString>
#include <QMap>
#include <QFileInfo>
#ifdef HAVE_OPENCV
#include <opencv2/opencv.hpp>
#endif

// 圖像信息結構 / Image information structure
struct ImageInfo {
    QString path;
    QString fileName;
    int width;
    int height;
    int channels;
    int depth;  // CV_8U, CV_16U, etc.
    QString format;  // jpg, png, etc.
    qint64 fileSize;  // 文件大小（字節）/ File size in bytes
    
    ImageInfo() : width(0), height(0), channels(0), depth(0), fileSize(0) {}
    
    // 分析圖像並返回信息 / Analyze image and return information
    static ImageInfo analyzeImage(const QString &path);
    
    // 轉換為字符串顯示 / Convert to string for display
    QString toString() const;
    
    // 獲取深度類型字符串 / Get depth type string
    static QString depthToString(int depth);
};

// 目錄統計結構 / Directory statistics structure
struct DirectoryStats {
    int totalImages;
    QMap<QString, int> formatCount;  // 格式計數 / Format count
    QMap<int, int> channelCount;     // 通道數計數 / Channel count
    QMap<int, int> depthCount;       // 深度類型計數 / Depth type count
    int minWidth, maxWidth, avgWidth;
    int minHeight, maxHeight, avgHeight;
    qint64 totalSize;  // 總文件大小 / Total file size
    
    DirectoryStats() : totalImages(0), 
                      minWidth(INT_MAX), maxWidth(0), avgWidth(0),
                      minHeight(INT_MAX), maxHeight(0), avgHeight(0),
                      totalSize(0) {}
    
    // 添加圖像信息到統計 / Add image info to statistics
    void addImage(const ImageInfo &info);
    
    // 重置統計 / Reset statistics
    void reset();
    
    // 轉換為字符串顯示 / Convert to string for display
    QString toString() const;
};

#endif // IMAGEINFO_H

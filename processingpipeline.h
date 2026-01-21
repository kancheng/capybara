#ifndef PROCESSINGPIPELINE_H
#define PROCESSINGPIPELINE_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QList>
#ifdef HAVE_OPENCV
#include <opencv2/opencv.hpp>
#endif
#include "imageprocessor.h"

// Processing operation structure
struct ProcessingOperation {
    QString operation;  // Operation name: "gaussianBlur", "canny", etc.
    QJsonObject parameters;  // Operation parameters
    
    ProcessingOperation() {}
    ProcessingOperation(const QString &op, const QJsonObject &params)
        : operation(op), parameters(params) {}
    
    // Convert to JSON
    QJsonObject toJson() const;
    
    // Load from JSON
    static ProcessingOperation fromJson(const QJsonObject &json);
};

// Processing pipeline class
class ProcessingPipeline
{
public:
    ProcessingPipeline();
    
    // Add operation
    void addOperation(const ProcessingOperation &op);
    
    // Remove operation
    void removeOperation(int index);
    
    // Clear operations
    void clear();
    
    // Get operations list
    QList<ProcessingOperation> getOperations() const { return operations; }
    
#ifdef HAVE_OPENCV
    // Apply pipeline to image
    cv::Mat apply(const cv::Mat &input);
#endif
    
    // JSON serialization
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &json);
    
    // Save to file
    bool saveToFile(const QString &filename);
    
    // Load from file
    bool loadFromFile(const QString &filename);
    
    // Get operation count
    int count() const { return operations.size(); }
    
private:
    QList<ProcessingOperation> operations;
    
#ifdef HAVE_OPENCV
    // Apply single operation
    cv::Mat applyOperation(const cv::Mat &input, const ProcessingOperation &op);
#endif
};

#endif // PROCESSINGPIPELINE_H

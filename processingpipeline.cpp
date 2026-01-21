#include "processingpipeline.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

ProcessingPipeline::ProcessingPipeline()
{
}

void ProcessingPipeline::addOperation(const ProcessingOperation &op)
{
    operations.append(op);
}

void ProcessingPipeline::removeOperation(int index)
{
    if (index >= 0 && index < operations.size()) {
        operations.removeAt(index);
    }
}

void ProcessingPipeline::clear()
{
    operations.clear();
}

#ifdef HAVE_OPENCV
cv::Mat ProcessingPipeline::apply(const cv::Mat &input)
{
    if (input.empty()) {
        return cv::Mat();
    }
    
    cv::Mat result = input.clone();
    
    for (const ProcessingOperation &op : operations) {
        result = applyOperation(result, op);
        if (result.empty()) {
            qDebug() << "Operation failed:" << op.operation;
            break;
        }
    }
    
    return result;
}

cv::Mat ProcessingPipeline::applyOperation(const cv::Mat &input, const ProcessingOperation &op)
{
    if (input.empty()) {
        return cv::Mat();
    }
    
    QString opName = op.operation;
    QJsonObject params = op.parameters;
    
    if (opName == "gaussianBlur") {
        int ksize = params["ksize"].toInt(5);
        double sigmaX = params["sigmaX"].toDouble(1.0);
        double sigmaY = params.value("sigmaY").toDouble(0.0);
        return ImageProcessor::applyGaussianBlur(input, ksize, sigmaX, sigmaY);
    }
    else if (opName == "canny") {
        double threshold1 = params["threshold1"].toDouble(50);
        double threshold2 = params["threshold2"].toDouble(150);
        int apertureSize = params.value("apertureSize").toInt(3);
        return ImageProcessor::applyCanny(input, threshold1, threshold2, apertureSize);
    }
    else if (opName == "brightnessContrast") {
        double alpha = params["alpha"].toDouble(1.0);
        int beta = params["beta"].toInt(0);
        return ImageProcessor::applyBrightnessContrast(input, alpha, beta);
    }
    else if (opName == "gamma") {
        double gamma = params["gamma"].toDouble(1.0);
        return ImageProcessor::applyGamma(input, gamma);
    }
    else if (opName == "convertColor") {
        int code = params["code"].toInt(6); // cv::COLOR_BGR2GRAY = 6
        return ImageProcessor::convertColor(input, code);
    }
    else if (opName == "resize") {
        int width = params["width"].toInt(input.cols);
        int height = params["height"].toInt(input.rows);
        int interpolation = params.value("interpolation").toInt(1); // cv::INTER_LINEAR = 1
        return ImageProcessor::resize(input, width, height, interpolation);
    }
    else if (opName == "rotate") {
        double angle = params["angle"].toDouble(0.0);
        return ImageProcessor::rotate(input, angle);
    }
    else if (opName == "flip") {
        int flipCode = params["flipCode"].toInt(1);
        return ImageProcessor::flip(input, flipCode);
    }
    else if (opName == "threshold") {
        double thresh = params["thresh"].toDouble(127);
        double maxval = params.value("maxval").toDouble(255);
        int type = params.value("type").toInt(0); // cv::THRESH_BINARY = 0
        return ImageProcessor::threshold(input, thresh, maxval, type);
    }
    else if (opName == "morphologyEx") {
        int op = params["op"].toInt(2); // cv::MORPH_OPEN = 2
        int shape = params.value("shape").toInt(0); // cv::MORPH_RECT = 0
        int ksize = params.value("ksize").toInt(5);
        return ImageProcessor::morphologyEx(input, op, shape, ksize);
    }
    else {
        qDebug() << "Unknown operation:" << opName;
        return input.clone();
    }
}
#endif // HAVE_OPENCV

QJsonObject ProcessingPipeline::toJson() const
{
    QJsonObject json;
    json["version"] = "1.0";
    
    QJsonArray operationsArray;
    for (const ProcessingOperation &op : operations) {
        operationsArray.append(op.toJson());
    }
    json["operations"] = operationsArray;
    
    return json;
}

bool ProcessingPipeline::fromJson(const QJsonObject &json)
{
    if (!json.contains("operations") || !json["operations"].isArray()) {
        return false;
    }
    
    operations.clear();
    QJsonArray operationsArray = json["operations"].toArray();
    
    for (const QJsonValue &value : operationsArray) {
        if (value.isObject()) {
            ProcessingOperation op = ProcessingOperation::fromJson(value.toObject());
            if (!op.operation.isEmpty()) {
                operations.append(op);
            }
        }
    }
    
    return true;
}

bool ProcessingPipeline::saveToFile(const QString &filename)
{
    QJsonObject json = toJson();
    QJsonDocument doc(json);
    
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot open file:" << filename;
        return false;
    }
    
    QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif
    out << doc.toJson(QJsonDocument::Indented);
    
    return true;
}

bool ProcessingPipeline::loadFromFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file:" << filename;
        return false;
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "Invalid JSON file:" << filename;
        return false;
    }
    
    return fromJson(doc.object());
}

QJsonObject ProcessingOperation::toJson() const
{
    QJsonObject json;
    json["operation"] = operation;
    json["parameters"] = parameters;
    return json;
}

ProcessingOperation ProcessingOperation::fromJson(const QJsonObject &json)
{
    ProcessingOperation op;
    if (json.contains("operation") && json.contains("parameters")) {
        op.operation = json["operation"].toString();
        op.parameters = json["parameters"].toObject();
    }
    return op;
}

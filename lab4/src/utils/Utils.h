#ifndef UTILS_H
#define UTILS_H

namespace cv { class Mat; }
class QImage;

cv::Mat ToCvMat(const QImage& image);
QImage ToQImage(const cv::Mat& mat);

#endif // UTILS_H

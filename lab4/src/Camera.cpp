#include "Camera.h"

Camera::Camera(int preferredResolutionWidth, int preferredResolutionHeight)
    : _preferredResolution(preferredResolutionWidth, preferredResolutionHeight)
{
    SetCameraDevice(0);
}

// Get the actual camera resolution
cv::Size Camera::GetResolution(const cv::Size& thresholdResolution) const {
    auto actualResolution = cv::Size(static_cast<int>(_videoCapture.get(cv::CAP_PROP_FRAME_WIDTH)),
                                     static_cast<int>(_videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT)));

    if (std::tie(actualResolution.width, actualResolution.height) > std::tie(thresholdResolution.width, thresholdResolution.height))
        return thresholdResolution;

    return actualResolution;
}

bool Camera::IsAvailable() const {
    return _isAvailable;
}

void Camera::SetCameraDevice(int cameraDeviceIndex) {
    _isAvailable = _videoCapture.open(cameraDeviceIndex);

    // If possible, this resolution will be used
    _videoCapture.set(cv::CAP_PROP_FRAME_WIDTH, _preferredResolution.width);
    _videoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, _preferredResolution.height);
    _videoCapture.set(cv::CAP_PROP_BUFFERSIZE, 3);
}

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Camera.h"
#include "RecognitionFrameCounter.h"
#include <QMainWindow>
#include <QMediaDevices>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    bool Errors() const;

private slots:
    void OnAvailableCamerasActivated(int index);
    void OnRunStopClicked();
    void OnClearLoggerClicked();
    void OnVideoInputsChanged();

private:
    Ui::MainWindow* ui;
    bool _hasErrors = false;

    bool _isCameraRun = false;
    Camera _camera;
    QMediaDevices _mediaDevices;
    QString _currentCameraName;
    ImageRecognizer _imageRecognizer;

    bool _isVideoStreamThreadRun = false;
    QMutex _mutex;
    QThread* _videoStreamThread = nullptr;
    QWaitCondition _videoStreamStopper;

    void Init();
    void InitConnects();
    void SetSystemInformation();

    void StartCamera();
    void StopCamera();
    void VideoStream();

    void HandleRecognitions(const std::vector<RecognitionInfo>& recognitions, cv::Mat& videoFrame);

    void ResumeVideoStreamThread();
    void SuspendVideoStreamThread(uint16_t msDelay);
    void DestroyVideoStreamThread();
};

#endif // MAINWINDOW_H

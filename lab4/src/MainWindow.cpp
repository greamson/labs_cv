#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "Utils.h"
#include "TextBrowserLogger.h"
#include <opencv2/imgproc.hpp>
#include <QDir>
#include <QSysInfo>
#include <QSoundEffect>
#include <QProcess>
#include <QMessageBox>
#include <QCameraDevice>
#include <QTimer>
#include <set>

using namespace std;
using namespace cv;

namespace {

constexpr int CAMERA_CONTENT_HINT_LABEL_PAGE  = 0;
constexpr int VIDEO_FRAME_PAGE                = 1;
constexpr int RESTART_RECOGNITION_SYSTEM_PAGE = 2;

// https://stackoverflow.com/a/71973860/17137664
constexpr uint16_t FRAME_READ_NUMBER        = 2;
constexpr uint16_t DROWSINESS_SLEEP_TIME_MS = 3000;
constexpr uint16_t RESTART_SLEEP_TIME_MS    = 1000;

constexpr QColor PALE_YELLOW        = {255, 219, 139};
constexpr QColor CRAIOLA_PERIWINKLE = {197, 208, 230};
constexpr QColor DEEP_CARMINE_PINK  = {239, 48, 56};
constexpr QColor WISTERIA           = {201, 160, 20};

string ToClassName(RecognitionType recognitionType) {
    switch (recognitionType) {
        case AttentiveEye: return QObject::tr("Attentive Eye").toStdString();
        case DrowsyEye:    return QObject::tr("Drowsy Eye").toStdString();
        default:           return "";
    }
}

Scalar ToRecognitionColor(RecognitionType recognitionType) {
    // BGR format
    switch (recognitionType) {
        case AttentiveEye: return {69, 89, 30};
        case DrowsyEye:    return {137, 49, 66};
        default:           return {0, 0, 0};
    }
}

void DrawLabel(Mat& image, const string& text, int left, int top, const Scalar& lineRectangleColor) {
    int baseLine;
    Size labelSize = getTextSize(text, FONT_HERSHEY_COMPLEX, 0.5, 1, &baseLine);
    top = max(top, labelSize.height);

    Point cornerLineTopRight(left + 15, top - 20);

    // For some reason getTextSize does not take "%" into account
    const Point horizontalLineBottomRight(left + labelSize.width + 21, top - 20);
    const Point rectangleTopRight(horizontalLineBottomRight.x, horizontalLineBottomRight.y - labelSize.height - baseLine);

    line(image, Point(left, top), cornerLineTopRight, lineRectangleColor, 2, LINE_AA);
    line(image, cornerLineTopRight, horizontalLineBottomRight, lineRectangleColor, 2, LINE_AA);
    rectangle(image, cornerLineTopRight, rectangleTopRight, lineRectangleColor, FILLED, LINE_AA);

    cornerLineTopRight.x += 3;
    cornerLineTopRight.y -= labelSize.height * 0.25;
    putText(image, text, cornerLineTopRight, FONT_HERSHEY_COMPLEX, 0.5, Scalar(255, 255, 255), 1, LINE_AA); // BGR format
}

} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow),
    _imageRecognizer(QDir::currentPath().append("/resource/neural_network_model/neural_network_model.onnx").toStdString())
{
    ui->setupUi(this);
    Init();
}

bool MainWindow::Errors() const {
    return _hasErrors;
}


void MainWindow::OnAvailableCamerasActivated(int index) {
    auto selectedCameraName = ui->availableCameras->itemText(index);

    if (selectedCameraName == _currentCameraName) // To avoid re-setting the same camera device
        return;

    bool isNeedToResumeVideoStream = false;

    if (_isCameraRun) {
        SuspendVideoStreamThread(175); // 175 ms should be enough for the video stream thread to fall asleep

        // If changed the camera while the previous one was working, then later after changing the camera it will be
        // necessary to resume the video stream thread
        isNeedToResumeVideoStream = true;
    }

    ui->videoFrame->clear();
    _currentCameraName = std::move(selectedCameraName);
    _camera.SetCameraDevice(index);

    const auto newCameraResolution = _camera.GetResolution();
    ui->cameraContentStackedWidget->setFixedSize(newCameraResolution.width, newCameraResolution.height);

    TextBrowserLogger::Log(ui->logger, tr("The camera \"").append(_currentCameraName).append(tr("\" is selected")), CRAIOLA_PERIWINKLE);

    if (isNeedToResumeVideoStream)
        ResumeVideoStreamThread();
}

void MainWindow::OnRunStopClicked() {
    if (!_isCameraRun && !_camera.IsAvailable()) {
        QMessageBox::warning(nullptr, tr("No available cameras"), tr("No available cameras"));
        return;
    }

    _isCameraRun = !_isCameraRun;
    ui->runStop->setText(_isCameraRun ? tr("Stop") : tr("Run"));

    _isCameraRun ? StartCamera() : StopCamera();
}

void MainWindow::OnClearLoggerClicked() {
    TextBrowserLogger::Clear(ui->logger);
}

void MainWindow::OnVideoInputsChanged() {
    // ---> QMediaDevices::videoInputs() leads to a memory leak (at least in Qt 6.2.2) <--- (qgstreamermediadevices.cpp)
    // Check my created Jira for them https://bugreports.qt.io/browse/QTBUG-99898
    // Qt says they will fix it in future versions
    const QList<QCameraDevice> cameras = _mediaDevices.videoInputs();
    QList<QString> existingCameras; // There may be a situation where there are cameras with the same names (descriptions)

    ui->availableCameras->clear();

    for (const auto& camera : cameras) {
        const auto camerasCount = existingCameras.count(camera.description());
        const QString cameraName = camera.description().append(camerasCount > 0 ? QString::number(camerasCount) : "");

        ui->availableCameras->addItem(cameraName);
        existingCameras.emplace_back(camera.description());
    }

    const auto isCurrentCameraDeviceAvailable = existingCameras.contains(_currentCameraName);

    if (_isCameraRun && !isCurrentCameraDeviceAvailable) {
        const auto errorMessage = tr("There is a problem with the camera ").append(_currentCameraName);

        OnRunStopClicked();
        TextBrowserLogger::Log(ui->logger, errorMessage, DEEP_CARMINE_PINK);
        QMessageBox::warning(nullptr, tr("Camera failure"), errorMessage);
    }

    if (cameras.isEmpty()) {
        _camera.SetCameraDevice(0); // Camera will be unavailable after VideoCapture::open
    }
    else if (!_camera.IsAvailable() || !isCurrentCameraDeviceAvailable) {
        _camera.SetCameraDevice(0); // If there were no cameras available before or the current camera device has been deactivated
        const auto cameraResolution = _camera.GetResolution();
        ui->cameraContentStackedWidget->setFixedSize(cameraResolution.width, cameraResolution.height);
        _currentCameraName = existingCameras[0];
        ui->availableCameras->setCurrentIndex(0);
    }
    else {
        ui->availableCameras->setCurrentText(_currentCameraName);
        qDebug() << ui->availableCameras->currentIndex() << ui->availableCameras->currentText();
    }
}

void MainWindow::Init() {
    SetSystemInformation();

    OnVideoInputsChanged(); // For fill combobox on init
    InitConnects();

    if (ui->availableCameras->count())
        _currentCameraName = ui->availableCameras->itemText(0);

    showMaximized();
}

void MainWindow::InitConnects() {
    connect(&_mediaDevices, &QMediaDevices::videoInputsChanged, this, &MainWindow::OnVideoInputsChanged);

    connect(ui->availableCameras, &QComboBox::activated, this, &MainWindow::OnAvailableCamerasActivated);
    connect(ui->runStop, &QPushButton::clicked, this, &MainWindow::OnRunStopClicked);
    connect(ui->clearLogger, &QPushButton::clicked, this, &MainWindow::OnClearLoggerClicked);
}

void MainWindow::SetSystemInformation() {
    QProcess systemProcess;
    QString cpuName, gpuName;

    systemProcess.startCommand("wmic cpu get name");
    systemProcess.waitForFinished();
    cpuName = systemProcess.readAllStandardOutput();
    cpuName.remove(0, cpuName.indexOf('\n') + 1);
    cpuName = cpuName.trimmed();

    systemProcess.startCommand("wmic PATH Win32_videocontroller get VideoProcessor");
    systemProcess.waitForFinished();
    gpuName = systemProcess.readAllStandardOutput();
    gpuName.remove(0, gpuName.indexOf('\n') + 1);
    gpuName = gpuName.trimmed();

    ui->operatingSystem->setText(QSysInfo::prettyProductName());
    ui->cpuName->setText(cpuName);
    ui->gpuName->setText(gpuName);
}

void MainWindow::StartCamera() {
    if (!_videoStreamThread || !_videoStreamThread->isRunning()) {
        _videoStreamThread = QThread::create(&MainWindow::VideoStream, this);
        _isVideoStreamThreadRun = true;
        _videoStreamThread->start();
    }
    else {
        ResumeVideoStreamThread();
    }

    TextBrowserLogger::Log(ui->logger, tr("The camera is running"), PALE_YELLOW);
    ui->cameraContentStackedWidget->setCurrentIndex(VIDEO_FRAME_PAGE);
}

void MainWindow::StopCamera() {
    ui->videoFrame->clear();
    ui->cameraContentStackedWidget->setCurrentIndex(CAMERA_CONTENT_HINT_LABEL_PAGE);
    SuspendVideoStreamThread(50);
    TextBrowserLogger::Log(ui->logger, tr("The camera is stopped"), PALE_YELLOW);
}

void MainWindow::VideoStream() {
    while (_isVideoStreamThreadRun) {
        QMutexLocker locker(&_mutex);

        while (!_isCameraRun)
            _videoStreamStopper.wait(&_mutex);

        Mat videoFrame;
        _camera.Read<FRAME_READ_NUMBER>(videoFrame);

        if (!videoFrame.empty()) {
            Mat flippedVideoFrame;
            flip(videoFrame, flippedVideoFrame, 1);

            const auto recognitions = _imageRecognizer.Recognize(flippedVideoFrame);
            HandleRecognitions(recognitions, flippedVideoFrame);

            ui->videoFrame->setPixmap(QPixmap::fromImage(ToQImage(flippedVideoFrame)));
        }

        // When repeatedly stopping and starting the camera using the GUI without this explicit unlock call, the thread may stall
        // It seems that the QMutexLocker destructor is not always called in time (same thing with std::unique_lock)
        locker.unlock();
    }
}

void MainWindow::HandleRecognitions(const std::vector<RecognitionInfo>& recognitions, cv::Mat& videoFrame) {
    static char confidenceBuffer[10];

    for (const auto& recognitionInfo : recognitions) {
        if(recognitionInfo.recognitionType == AttentiveEye || recognitionInfo.recognitionType == DrowsyEye)
        {
            const auto recognitionColor = ToRecognitionColor(recognitionInfo.recognitionType);
            snprintf(confidenceBuffer, sizeof(confidenceBuffer), " %.2f %%", recognitionInfo.confidence * 100);

            rectangle(videoFrame, recognitionInfo.boundingBox, recognitionColor, 2); // Bounding Box
            DrawLabel(videoFrame, ToClassName(recognitionInfo.recognitionType).append(confidenceBuffer),
                      recognitionInfo.boundingBox.x, recognitionInfo.boundingBox.y, recognitionColor);
        }
    }
}

void MainWindow::ResumeVideoStreamThread() {
    {
        QMutexLocker locker(&_mutex);
        _isCameraRun = true;
    }

    _videoStreamStopper.notify_one();
}

void MainWindow::SuspendVideoStreamThread(uint16_t msDelay) {
    _isCameraRun = false;
    QThread::msleep(msDelay); // Delay so that the thread is exactly paused due to the condition _isCameraRun == false
}

void MainWindow::DestroyVideoStreamThread() {
    if (_videoStreamThread && _videoStreamThread->isRunning()) {
        _isVideoStreamThreadRun = false;
        ResumeVideoStreamThread(); // To unpause the thread if it is paused and immediately stop
        _videoStreamThread->wait();
        delete _videoStreamThread;
        _videoStreamThread = nullptr;
    }
}

MainWindow::~MainWindow() {
    DestroyVideoStreamThread();
    delete ui;
}

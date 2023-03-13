#include "MainWindow.h"
#include <QApplication>

int main(int argc, char** argv) {
    _putenv_s("OPENCV_VIDEOIO_MSMF_ENABLE_HW_TRANSFORMS", "0"); // So there is no delay when opening a camera with MSMF backend

    QApplication application(argc, argv);
    MainWindow mainWindow;

    if (mainWindow.Errors())
        return 1;

    mainWindow.show();
    return QApplication::exec();
}

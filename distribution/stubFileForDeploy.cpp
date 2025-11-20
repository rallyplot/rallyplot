#include <Plotter.h>
#include <QApplication>
#include <QWidget>

int main(int argc, char** argv) {
    QApplication app(argc, argv);  // pulls in QtCore/Gui
    QWidget w;                      // pulls in QtWidgets
    Plotter plotter{};
    return 0;                       // never shown; only for deployment scan
}

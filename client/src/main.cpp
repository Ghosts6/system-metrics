#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("System Metrics Client");

    QWidget *centralWidget = new QWidget(&mainWindow);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    QLabel *label = new QLabel("Client MVP - Awaiting Backend Connection...");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    centralWidget->setLayout(layout);
    mainWindow.setCentralWidget(centralWidget);
    mainWindow.resize(400, 200);
    mainWindow.show();

    return app.exec();
}

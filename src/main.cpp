#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
	QApplication a(argc, argv);
	MainWindow w;
	if(argc == 2)
	{
		w.openFile(QString::fromLocal8Bit(QByteArray(argv[1])));
		//QMessageBox::warning(NULL,QString("aaa"),QString::fromLocal8Bit(QByteArray(argv[1])));
	}
	w.show();

	return a.exec();
	return 0;
}

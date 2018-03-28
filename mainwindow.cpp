#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QPushButton>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	//================== about file =================
	isUntitled = true;
	shouldSave = false;
	curFile = tr("无标题.txt");
	setWindowTitle(curFile + tr(" - MiniWord"));

	//================== right click menu ============
	initRightMenu();

	//================== replace and find dialog ========
	replaceDlg = new ReplaceDlg(this);

	//================= input support ===========
	setFocusPolicy(Qt::ClickFocus);
	setAttribute(Qt::WA_InputMethodEnabled, true);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::initRightMenu()
{
	//get object
	rightMenu = new QMenu(this);
	undoAction = new QAction(tr("撤销(&U)"), this);
	cutAction = new QAction(tr("剪切(&T)"), this);
	copyAction = new QAction(tr("复制(&C)"), this);
	pasteAction = new QAction(tr("粘贴(&P)"), this);
	delAction = new QAction(tr("删除(&D)"), this);
	seleteAllAction = new QAction(tr("全选(&A)"), this);

	//todo:connect action and function

	//setup ui
	rightMenu->addAction(undoAction);
	rightMenu->addSeparator();
	rightMenu->addAction(cutAction);
	rightMenu->addAction(copyAction);
	rightMenu->addAction(pasteAction);
	rightMenu->addAction(delAction);
	rightMenu->addSeparator();
	rightMenu->addAction(seleteAllAction);

	//todo:connect right click and menu->show
}

void MainWindow::newFile()
{
	if (maybeSave()){
	//ok to create a new file
		//todo:clear

		curFile = tr("无标题.txt");
		isUntitled = true;
		shouldSave = false;
	}
}
bool MainWindow::maybeSave()
{
	if (shouldSave){
		QMessageBox box;
		box.setWindowTitle(tr("MiniWord"));
		box.setIcon(QMessageBox::Warning);
		box.setText(tr("是否将更改保存到 ") + curFile + " ?");
		QPushButton * yesBtn = box.addButton(tr("是(&Y"), QMessageBox::YesRole);
		box.addButton(tr("否(&N)"), QMessageBox::NoRole);
		QPushButton * cancelBtn = box.addButton(tr("取消"), QMessageBox::RejectRole);
		box.exec();
		if (box.clickedButton() == yesBtn)
			return save();
		else if (box.clickedButton() == cancelBtn)
			return false;
	}
	return true;
}
bool MainWindow::save()
{
	if (isUntitled){
		return saveAs();
	} else {
		return saveFile(curFile);
	}
}
bool MainWindow::saveAs()
{
	QString path = QFileDialog::getSaveFileName(this, tr("另存为"), curFile);

	if (path.isEmpty()) return false;
	return saveFile(path);
}

bool MainWindow::saveFile(const QString &path)
{
	QFile file(path);
	if (!file.open(QFile::WriteOnly | QFile::Text)){
		QMessageBox::warning(this, tr("MiniWord"), tr("打开文件失败"));
		return false;
	}
	QTextStream out(&file);
	QApplication::setOverrideCursor(Qt::WaitCursor);

	//todo:save data
	//format: out << string;

	QApplication::restoreOverrideCursor();
	isUntitled = false;
	curFile = QFileInfo(path).canonicalFilePath();
	setWindowTitle(curFile + tr(" - MiniWord"));
	return true;
}

bool MainWindow::openFile(const QString &path)
{
	QFile file(path);
	if (!file.open(QFile::ReadOnly | QFile::Text)){
		QMessageBox::warning(this, tr("MiniWord"), tr("无法打开文件"));
		return false;
	}
	QTextStream in(&file);
	QApplication::setOverrideCursor(Qt::WaitCursor);

	//todo:load data
	//format: in >> string

	QApplication::restoreOverrideCursor();

	curFile = QFileInfo(path).canonicalFilePath();
	setWindowTitle(curFile + tr(" - MiniWord"));
	return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (maybeSave()){
		event->accept();//close window
	} else {
		event->ignore();//not close window
	}
}

void MainWindow::keyPressEvent(QKeyEvent * ev)
{

}

void MainWindow::inputMethodEvent(QInputMethodEvent * ev)
{

}

void MainWindow::on_action_New_triggered()
{
	newFile();
}

void MainWindow::on_action_Open_triggered()
{
	if (maybeSave()){
	//ok to open file
		QString path = QFileDialog::getOpenFileName(this);
		if (!path.isEmpty()){
			openFile(path);
		}
	}
}

void MainWindow::on_action_Save_triggered()
{
	save();
}

void MainWindow::on_action_SaveAs_triggered()
{
	saveAs();
}

void MainWindow::on_action_Exit_triggered()
{
	if (maybeSave()){
		qApp->quit();//equal to exit(0)
	}
}

void MainWindow::on_action_Undo_triggered()
{
	//todo
}

void MainWindow::on_action_Cut_triggered()
{
	//todo
}

void MainWindow::on_action_Copy_triggered()
{
	//todo
}

void MainWindow::on_action_Paste_triggered()
{
	//todo
}

void MainWindow::on_action_Delete_triggered()
{
	//todo
}

void MainWindow::on_action_Find_triggered()
{
	replaceDlg->show();
}

void MainWindow::on_action_FindNext_triggered()
{
	//todo
}

void MainWindow::on_action_Replace_triggered()
{
	replaceDlg->show();
}

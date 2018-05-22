#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QMimeData>
#include <QFileDialog>
#include <QFile>
#include <QPushButton>
#include <QInputDialog>
#include <QTextStream>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>

const int FirstQCharX = 30;
const int FirstQCharY = 50;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	showMaximized();
	setMinimumSize(200,200);
	//================== about file =================
	isUntitled = true;
	shouldSave = false;
	curFile = tr("无标题.txt");
	setWindowTitle(curFile + tr(" - MiniWord"));
	setWindowIcon(QIcon("MiniWord.ico"));
	connect(&data, &Data::dataChanged, this, &MainWindow::getDataChanged);
	//================== right click menu ============
	initRightMenu();
	connect(ui->menu_E, &QMenu::aboutToShow, this, &MainWindow::getMenu_E_state);
	//================== dialog ========
	replaceDlg = new ReplaceDlg(this);
	settingsDlg = new SettingDlg(this);
	charnumDlg = new CharNumDlg(this);
	getConfig();
	data.resetStackSize(settingsDlg->maxUndoTime());
	connect(replaceDlg,SIGNAL(FindNext()),this,SLOT(on_action_FindNext_triggered()));
	connect(replaceDlg,SIGNAL(Replace()),this,SLOT(data_replace()));
	connect(replaceDlg,SIGNAL(ReplaceAll()),this,SLOT(data_replace_all()));

	//================= input support ===========
	setFocusPolicy(Qt::ClickFocus);
	setAttribute(Qt::WA_InputMethodEnabled, true);
	centralWidget()->setMouseTracking(true);
	setMouseTracking(true);
	centralWidget()->setAcceptDrops(true);
	setAcceptDrops(true);

	//==================== interface ====================
	MyCursorTimer.start(700);
	MyProtectedUpdateTimer.start(30);

	connect(&MyProtectedUpdateTimer, SIGNAL(timeout()),this,SLOT(RefreshProtectTimer()));
	connect(&data,SIGNAL(WindowUdate()),this,SLOT(ProtectedUpdate()));
	connect(&MyCursorTimer, SIGNAL(timeout()),this,SLOT(ProtectedUpdate()));

	MyScrollBar = new QScrollBar(this);
	MyScrollBar->move(FirstQCharX+TextBoxWidth,FirstQCharY);
	MyScrollBar->setFixedSize(30,600);
	MyScrollBar->show();
	oldScrollPos = 0;

	DataTextHeight = 0;
	DataTextTop = 0;
	DataParaTop = 0;
	TextBoxHeight = 20;
	TextBoxWidth = 1000;

	LineShowFlag = 0;
	IsDragged = false;

	CursorTimer = 0;
	ProtectedUpdateTimer=1;

	PosCur=PosPre=PosLeftUp={0,0,data.begin()};
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
	redoAction = new QAction(tr("重做(&R)"), this);
	cutAction = new QAction(tr("剪切(&T)"), this);
	copyAction = new QAction(tr("复制(&C)"), this);
	pasteAction = new QAction(tr("粘贴(&P)"), this);
    delAction = new QAction(tr("删除(&D)"), this);
    seleteAllAction = new QAction(tr("全选(&A)"), this);

	//connect
	connect(undoAction, &QAction::triggered, this, &MainWindow::on_action_Undo_triggered);
	connect(redoAction, &QAction::triggered, this, &MainWindow::on_action_Redo_triggered);
	connect(cutAction, &QAction::triggered, this, &MainWindow::on_action_Cut_triggered);
	connect(copyAction, &QAction::triggered, this, &MainWindow::on_action_Copy_triggered);
	connect(pasteAction, &QAction::triggered, this, &MainWindow::on_action_Paste_triggered);
	connect(delAction, &QAction::triggered, this, &MainWindow::on_action_Delete_triggered);
	connect(seleteAllAction, &QAction::triggered, this, &MainWindow::on_action_SelectAll_triggered);

    //setup ui
    rightMenu->addAction(undoAction);
	rightMenu->addAction(redoAction);
    rightMenu->addSeparator();
    rightMenu->addAction(cutAction);
    rightMenu->addAction(copyAction);
    rightMenu->addAction(pasteAction);
    rightMenu->addAction(delAction);
    rightMenu->addSeparator();
    rightMenu->addAction(seleteAllAction);
}
void MainWindow::resetRightMenu()
{
    //activate all methods
    undoAction->setDisabled(false);
	redoAction->setDisabled(false);
    cutAction->setDisabled(false);
    copyAction->setDisabled(false);
    pasteAction->setDisabled(false);
    delAction->setDisabled(false);
    seleteAllAction->setDisabled(false);
}
void MainWindow::newFile()
{
	if (maybeSave())
	{
    //ok to create a new file
		data.clear();
        curFile = tr("无标题.txt");
        isUntitled = true;
        shouldSave = false;
		PosLeftUp.DataPos = PosCur.DataPos = PosPre.DataPos = data.begin();
		GetDataHeight();
		data.clearStack();
    }
}

bool MainWindow::maybeSave()
{
	if (isUntitled && data.isEmpty()) return true;
	if (shouldSave)
	{
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
	if (isUntitled)
	{
        return saveAs();
	} else
	{
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
	if (!file.open(QFile::WriteOnly | QFile::Text))
	{
        QMessageBox::warning(this, tr("MiniWord"), tr("打开文件失败"));
        return false;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
	data.save(path);
    QApplication::restoreOverrideCursor();
    isUntitled = false;
	shouldSave = false;
    curFile = QFileInfo(path).canonicalFilePath();
    setWindowTitle(curFile + tr(" - MiniWord"));
    return true;
}
void MainWindow::setConfig() const
{
	//! file format
	//! maxUndoTime
	//! defaultFontSize
	//! spaceStyle
	//! tabSize
	//! tabStyle
	QFile file(qApp->applicationDirPath()+QString("/MiniWordConfig"));
	if (!file.open(QFile::WriteOnly | QFile::Text)){
		//qDebug() << "Mainwindow::setConfig::can not open file";
		return;
	}
	QTextStream out(&file);

	out << settingsDlg->maxUndoTime() << endl;
	out << settingsDlg->defaultFontSize() << endl;
	out << settingsDlg->spaceStyle() << endl;
	out << settingsDlg->tabSize() << endl;
	out << settingsDlg->tabStyle() << endl;

	file.close();
}
void MainWindow::getConfig()
{
	//! file format
	//! maxUndoTime
	//! defaultFontSize
	//! spaceStyle
	//! tabSize
	//! tabStyle
	QFile file(qApp->applicationDirPath()+QString("/MiniWordConfig"));
	if (!file.open(QFile::ReadOnly | QFile::Text)){
		//qDebug() << "Mainwindow::getConfig::can not open file";

		settingsDlg->setMaxUndoTime(20);
		settingsDlg->setFontSize(20);
		FontSizeW = 8;
		FontSizeH = 16;
		SpaceStyle = 0;
		TabWidth = 8;
		TabStyle = 0;
		return;
	}
	QTextStream in(&file);

	int n;
	in >> n;
	if(n < 20)n = 20;
	settingsDlg->setMaxUndoTime(n);
	in >> n;
	if(n<1||n>9)n = 1;
	settingsDlg->setFontSize(n);
	FontSizeW = (n + 1) * 4;
	FontSizeH = FontSizeW * 2;
	in >> n;
	settingsDlg->setSpaceStyle(n);
	SpaceStyle = n;
	in >> n;
	if(n<2||n>16)n = 4;
	settingsDlg->setTabSize(n);
	TabWidth = n;
	in >> n;
	settingsDlg->setTabStyle(n);
	TabStyle = n;

	file.close();
}

bool MainWindow::openFile(const QString &path)
{
    QFile file(path);
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
        QMessageBox::warning(this, tr("MiniWord"), tr("无法打开文件"));
        return false;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
	data.clear();
	data.read(path);

	data.clearStack();
	QApplication::restoreOverrideCursor();
	PosLeftUp.DataPos = PosCur.DataPos = PosPre.DataPos = data.begin();
	DataTextTop = 0;
	GetDataHeight();
    curFile = QFileInfo(path).canonicalFilePath();
    setWindowTitle(curFile + tr(" - MiniWord"));
	isUntitled = false;
	shouldSave = false;
	ProtectedUpdate();
    return true;
}



void MainWindow::closeEvent(QCloseEvent *event)
{
	if (maybeSave())
	{
        event->accept();//close window
	} else
	{
        event->ignore();//not close window
    }
}



void MainWindow::keyPressEvent(QKeyEvent * ev)
{
	CursorTimer=1;
	MyCursorTimer.start(700);
	if(ProtectedUpdateTimer == 0)return;
	switch (ev->key()){
	case Qt::Key_Tab:
		if(PosCur.ShowPosY<0||PosPre.ShowPosY<0)PosLeftUp.DataPos = data.begin();
		if(PosCur.DataPos==PosPre.DataPos)
		{
			PosCur.DataPos = data.add(PosCur.DataPos,tr("\t"));
			PosPre = PosCur;
		}else
		{
			if(PosCur.ShowPosX+TextBoxWidth*PosCur.ShowPosY<PosPre.ShowPosX+TextBoxWidth*PosPre.ShowPosY)
			{
				PosCur.DataPos = data.edit(PosCur.DataPos,PosPre.DataPos,tr("\t"));
				PosPre = PosCur;
			}else
			{
				PosCur.DataPos = data.edit(PosCur.DataPos,PosPre.DataPos,tr("\t"));
				PosPre = PosCur;
			}
		}
		GetDataHeight();
		FindCursor();
		break;
    case Qt::Key_Backspace :
		if(PosCur.DataPos==PosPre.DataPos)
		{
			if(PosCur.ShowPosY < 0)
			{
				FindCursor();
			}
			if(PosLeftUp.DataPos == PosCur.DataPos)
			{
				if(DataTextTop != 0)
				{
					LocateLeftUpCursor(DataTextTop-1);
				}
			}
			PosCur.DataPos=data.del(PosCur.DataPos,PosPre.DataPos,false);
			PosPre=PosCur;
		}else{
			if(PosCur.ShowPosX+TextBoxWidth*PosCur.ShowPosY<PosPre.ShowPosX+TextBoxWidth*PosPre.ShowPosY)
			{
				PosCur.DataPos = data.del(PosCur.DataPos,PosPre.DataPos);
				PosPre = PosCur;
			}else
			{
				PosCur.DataPos = data.del(PosPre.DataPos,PosCur.DataPos);
				PosPre = PosCur;
			}
			if(PosCur.ShowPosY<0||PosPre.ShowPosY<0||PosCur.DataPos==PosLeftUp.DataPos||PosPre.DataPos==PosLeftUp.DataPos)
				PosLeftUp.DataPos = data.begin();
		}
		GetDataHeight();
		FindCursor();
        break;
    case Qt::Key_Enter :
    case Qt::Key_Return :
		if(PosCur.ShowPosY<0||PosPre.ShowPosY<0)PosLeftUp.DataPos = data.begin();
		if(PosCur.DataPos == PosPre.DataPos)
		{
			PosCur.ShowPosY++;
			PosCur.ShowPosX=0;
			PosCur.DataPos=data.add(PosCur.DataPos, tr("\n"));
			PosPre = PosCur;
		}else
		{
			if(PosCur.ShowPosX+TextBoxWidth*PosCur.ShowPosY<PosPre.ShowPosX+TextBoxWidth*PosPre.ShowPosY)
			{
				PosCur.DataPos = data.edit(PosCur.DataPos,PosPre.DataPos,tr("\n"));
				PosPre = PosCur;
			}else
			{
				PosCur.DataPos = data.edit(PosCur.DataPos,PosPre.DataPos,tr("\n"));
				PosPre = PosCur;
			}
		}
		GetDataHeight();
		FindCursor();
        break;
    case Qt::Key_Delete :
		if(PosCur.DataPos==PosPre.DataPos)
		{
			if(PosCur.ShowPosY < 0)
			{
				FindCursor();
			}
			PosCur.DataPos = data.del(PosCur.DataPos,PosPre.DataPos,true);
			PosPre = PosCur;
		}else
		{
			if(PosCur.ShowPosX+TextBoxWidth*PosCur.ShowPosY<PosPre.ShowPosX+TextBoxWidth*PosPre.ShowPosY)
			{
				PosCur.DataPos = data.del(PosCur.DataPos,PosPre.DataPos);
				PosPre = PosCur;
			}else
			{
				PosCur.DataPos = data.del(PosPre.DataPos,PosCur.DataPos);
				PosPre = PosCur;
			}
			if(PosCur.ShowPosY<0||PosPre.ShowPosY<0)PosLeftUp.DataPos = data.begin();
		}
		GetDataHeight();
		FindCursor();
        break;
	case Qt::Key_Up :
		if(PosCur.ShowPosY>0)
		{
			LocateCursor(PosCur.ShowPosX,PosCur.ShowPosY-1);
			if(!(QApplication::keyboardModifiers () == Qt::ShiftModifier))PosPre = PosCur;
		}else
		{
			if(DataTextTop!=0)
			{
				LocateLeftUpCursor(DataTextTop-1);
				LocateCursor(PosCur.ShowPosX,PosCur.ShowPosY-1);
				if(!(QApplication::keyboardModifiers () == Qt::ShiftModifier))PosPre = PosCur;
			}
		}
		ProtectedUpdate();
        break;
	case Qt::Key_Down :
		if(PosCur.ShowPosY<TextBoxHeight-1)
		{
			if(PosCur.ShowPosY != DataTextHeight-DataTextTop-1)
			{
				LocateCursor(PosCur.ShowPosX,PosCur.ShowPosY+1);
				if(!(QApplication::keyboardModifiers () == Qt::ShiftModifier))PosPre = PosCur;
			}
		}else
		{
			if(DataTextHeight-DataTextTop>TextBoxHeight)
			{
				LocateLeftUpCursor(DataTextTop+1);
				LocateCursor(PosCur.ShowPosX,PosCur.ShowPosY+1);
				if(!(QApplication::keyboardModifiers () == Qt::ShiftModifier))PosPre = PosCur;
			}
		}
		ProtectedUpdate();
        break;
	case Qt::Key_Left :
		if(QApplication::keyboardModifiers () == Qt::ShiftModifier)
		{
			PosCur.DataPos--;
		}else if(PosCur.DataPos==PosPre.DataPos)
		{
			PosCur.DataPos--;
			PosPre.DataPos--;
		}else
		{
			if(PosCur.ShowPosY>PosPre.ShowPosY)
			{
				PosCur = PosPre;
			}else if(PosCur.ShowPosY<PosPre.ShowPosY)
			{
				PosPre = PosCur;
			}else if(PosCur.ShowPosX>PosPre.ShowPosX)
			{
				PosCur = PosPre;
			}else if(PosCur.ShowPosX<PosPre.ShowPosX)
			{
				PosPre = PosCur;
			}
		}
		{
			int x = PosCur.ShowPosX;
			GetDataHeight();
			if(x!=0&&x<PosCur.ShowPosX)
			{
				PosCur.ShowPosX = 0;
				PosCur.ShowPosY++;
			}
		}
		FindCursor();
		ProtectedUpdate();
        break;
	case Qt::Key_Right :
		if(QApplication::keyboardModifiers () == Qt::ShiftModifier)
		{
			PosCur.DataPos++;
		}else if(PosCur.DataPos==PosPre.DataPos)
		{
			PosCur.DataPos++;
			PosPre.DataPos++;
		}else{
			if(PosCur.ShowPosY>PosPre.ShowPosY)
			{
				PosPre = PosCur;
			}else if(PosCur.ShowPosY<PosPre.ShowPosY)
			{
				PosCur = PosPre;
			}else if(PosCur.ShowPosX>PosPre.ShowPosX)
			{
				PosPre = PosCur;
			}else if(PosCur.ShowPosX<PosPre.ShowPosX)
			{
				PosCur = PosPre;
			}
		}
		GetDataHeight();
		FindCursor();
		ProtectedUpdate();
        break;
	case Qt::Key_PageUp:
		LocateLeftUpCursor(DataTextTop - TextBoxHeight);
		ProtectedUpdate();
		break;
	case Qt::Key_PageDown:
		LocateLeftUpCursor(DataTextTop + TextBoxHeight);
		ProtectedUpdate();
		break;
    case Qt::Key_Home :
		LocateCursor(0,PosCur.ShowPosY);
		PosPre = PosCur;
		ProtectedUpdate();
        break;
    case Qt::Key_End :
		LocateLeftUpCursor(DataTextHeight-1);
		ProtectedUpdate();
        break;
    default :
		if(QApplication::keyboardModifiers () == Qt::ControlModifier)return;
		if(ev->text()=="")return;
		if(PosCur.ShowPosY<0||PosPre.ShowPosY<0)PosLeftUp.DataPos = data.begin();
		if(PosCur.DataPos==PosPre.DataPos)
		{
			PosCur.DataPos=data.add(PosCur.DataPos, ev->text());
			PosPre = PosCur;
		}else
		{
			if(PosCur.ShowPosX+TextBoxWidth*PosCur.ShowPosY<PosPre.ShowPosX+TextBoxWidth*PosPre.ShowPosY)
			{
				PosCur.DataPos = data.edit(PosCur.DataPos,PosPre.DataPos,ev->text());
				PosPre = PosCur;
			}else
			{
				PosCur.DataPos = data.edit(PosCur.DataPos,PosPre.DataPos,ev->text());
				PosPre = PosCur;
			}
		}
		GetDataHeight();
		FindCursor();
		break;
    }
}
void MainWindow::inputMethodEvent(QInputMethodEvent * ev)
{
	CursorTimer=1;
	MyCursorTimer.start(700);
	if(QApplication::keyboardModifiers () == Qt::ControlModifier)return;
	if(ev->commitString()=="")return;
	if(PosCur.ShowPosY<0||PosPre.ShowPosY<0)PosLeftUp.DataPos = data.begin();
	if(PosCur.DataPos==PosPre.DataPos)
	{
		PosCur.DataPos=data.add(PosCur.DataPos, ev->commitString());
		PosPre = PosCur;
	}else
	{
		if(PosCur.ShowPosX+TextBoxWidth*PosCur.ShowPosY<PosPre.ShowPosX+TextBoxWidth*PosPre.ShowPosY)
		{
			PosCur.DataPos = data.edit(PosCur.DataPos,PosPre.DataPos,ev->commitString());
			PosPre = PosCur;
		}else
		{
			PosCur.DataPos = data.edit(PosCur.DataPos,PosPre.DataPos,ev->commitString());
			PosPre = PosCur;
		}
	}
	GetDataHeight();
	FindCursor();
}
void MainWindow::mousePressEvent(QMouseEvent * ev)
{
	if (ev->button() == Qt::RightButton)//show right click menu
	{
        //judge function state
		if (!PosCur.DataPos || !PosPre.DataPos || PosCur.DataPos == PosPre.DataPos)
		{
            //iterator are overflow or equal
            cutAction->setDisabled(true);
            copyAction->setDisabled(true);
            delAction->setDisabled(true);
        }
		if (data.undoStackEmpty())
		{
			undoAction->setDisabled(true);
		}
		if (data.redoStackEmpty())
		{
			redoAction->setDisabled(true);
		}
		rightMenu->exec(ev->screenPos().toPoint());
        resetRightMenu();
	}else if(ev->button() == Qt::LeftButton)
	{
		int x = ev->pos().x() - FirstQCharX;
		int y = (ev->pos().y() - FirstQCharY)/FontSizeH;
		x = (x>TextBoxWidth-FontSizeW/2?TextBoxWidth-FontSizeW/2:x);
		y = (y>TextBoxHeight-1?TextBoxHeight-1:y);
		LocateCursor(x,y);
		PosPre = PosCur;
		CursorTimer=1;
		MyCursorTimer.start(700);
		ProtectedUpdate();
		IsDragged = true;
	}
}

void MainWindow::mouseReleaseEvent(QMouseEvent *ev)
{
	IsDragged = false;
}
void MainWindow::mouseDoubleClickEvent(QMouseEvent *ev)
{
	if(ev->button() == Qt::LeftButton)
	{
		PosPre.DataPos = PosCur.DataPos--;
		GetDataHeight();
		ProtectedUpdate();
	}
	IsDragged = false;
}
void MainWindow::mouseMoveEvent(QMouseEvent *ev)
{
	if(ProtectedUpdateTimer == 0)return;
	int x = ev->pos().x() - FirstQCharX;
	int y = (ev->pos().y() - FirstQCharY)/FontSizeH;

	if(x>=0&&x<TextBoxWidth&&y>=0&&y<TextBoxHeight)
	{
		setCursor(Qt::IBeamCursor);
	}else{
		setCursor(Qt::ArrowCursor);
	}

	if((ev->buttons() & Qt::LeftButton) && IsDragged)
	{
		if(x<0)x=0;
		x = (x>TextBoxWidth-FontSizeW/2?TextBoxWidth-FontSizeW/2:x);
		if(y<0)LocateLeftUpCursor(DataTextTop-1);
		if(y>=TextBoxHeight)LocateLeftUpCursor(DataTextTop+1);

		LocateCursor(x,y);
		//PosPre = PosCur;
		CursorTimer=1;
		MyCursorTimer.start(700);
		ProtectedUpdate();
	}
}

void MainWindow::wheelEvent(QWheelEvent *ev)
{
	CursorTimer=1;
	MyCursorTimer.start(700);
	if(ev->delta() > 0){
		if(QApplication::keyboardModifiers () == Qt::ControlModifier)
		{
			if(FontSizeW<40)
			{
				ChangeFontSize(FontSizeW+4);
				settingsDlg->setFontSize(FontSizeW/4 - 1);
			}
		}else
		{
			Rolling(0);
		}
	}else{
		if(QApplication::keyboardModifiers () == Qt::ControlModifier)
		{
			if(FontSizeW>8)
			{
				ChangeFontSize(FontSizeW-4);
				settingsDlg->setFontSize(FontSizeW/4 - 1);
			}
		}else
		{
			Rolling(1);
		}
	}
	ProtectedUpdate();
}

void MainWindow::paintEvent(QPaintEvent *ev)
{
	QPainter MyPainter(this);                                           //A Painter
	MyPainter.setFont(QFont("楷体",8));
	QFont FontRestore;
	QPen PenRestore;

    int DrawX=0,DrawY=0;                                                //the position to draw text
	int PosCurPara = 1,PosPrePara = 1;

	int newWidth = width() - 65;
	int newHeigh = height() - 60;
	int ChangeColorFlag = 0;

	if(TextBoxHeight != newHeigh/FontSizeH)
	{
		TextBoxHeight = newHeigh/FontSizeH;
		CursorTimer = 1;
	}

	if(TextBoxWidth != newWidth)
	{
		TextBoxWidth = newWidth;
		int TopPre = DataTextTop;
		GetDataHeight();
		if(TopPre>=DataTextHeight)TopPre = DataTextHeight-1;
		LocateLeftUpCursor(TopPre,1);
		CursorTimer = 1;
	}																	//调整宽和高

	MyPainter.fillRect(FirstQCharX,FirstQCharY,TextBoxWidth,newHeigh,Qt::white);
	if(PosCur.DataPos==PosPre.DataPos)
	{
		if(PosCur.ShowPosX>=0&&PosCur.ShowPosX<=TextBoxWidth&&PosCur.ShowPosY>=0&&PosCur.ShowPosY<TextBoxHeight&&CursorTimer==1)
			MyPainter.drawLine(PosCur.ShowPosX + FirstQCharX,PosCur.ShowPosY * FontSizeH + FirstQCharY,
								PosCur.ShowPosX + FirstQCharX,PosCur.ShowPosY * FontSizeH + FirstQCharY + FontSizeH);
	}else{
		if(PosCur.ShowPosY>PosPre.ShowPosY&&PosCur.ShowPosY>=0)
		{
			FillBlueArea(PosPre,PosCur,&MyPainter);
			ChangeColorFlag = 2;
		}else if(PosCur.ShowPosY<PosPre.ShowPosY&&PosPre.ShowPosY>=0)
		{
			FillBlueArea(PosCur,PosPre,&MyPainter);
			ChangeColorFlag = 1;
		}else if(PosCur.ShowPosX>PosPre.ShowPosX)
		{
			if(PosCur.ShowPosY>=0&&PosCur.ShowPosY<TextBoxHeight)
				MyPainter.fillRect(FirstQCharX+PosPre.ShowPosX,PosPre.ShowPosY*FontSizeH+FirstQCharY,PosCur.ShowPosX-PosPre.ShowPosX,FontSizeH,
									QColor(0,0,255));
			ChangeColorFlag = 2;
		}else if(PosCur.ShowPosX<PosPre.ShowPosX)
		{
			if(PosCur.ShowPosY>=0&&PosCur.ShowPosY<TextBoxHeight)
				MyPainter.fillRect(FirstQCharX+PosCur.ShowPosX,PosCur.ShowPosY*FontSizeH+FirstQCharY,PosPre.ShowPosX-PosCur.ShowPosX,FontSizeH,
									QColor(0,0,255));
			ChangeColorFlag = 1;
		}
	}																	//绘制光标
	MyPainter.setPen(QColor(150,150,150));
	if(LineShowFlag == 0)
	{
		auto i = data.begin().parentNode();
		int PosCurParaCharNum = 0;
		if(ChangeColorFlag == 0)
		{
			while(i != PosCur.DataPos.parentNode())
			{
				i = i->nextNode;
				PosCurPara++;
				PosPrePara++;
			}
			auto j = i->firstHeap;
			while(j != PosCur.DataPos.parentHeap())
			{
				j = j->nextHeap;
				PosCurParaCharNum+=j->charNum;
			}
			PosCurParaCharNum+=PosCur.DataPos.index();
		}else if(ChangeColorFlag == 1)
		{
			while(i != PosCur.DataPos.parentNode())
			{
				i = i->nextNode;
				PosCurPara++;
				PosPrePara++;
			}
			auto j = i->firstHeap;
			while(j != PosCur.DataPos.parentHeap())
			{
				j = j->nextHeap;
				PosCurParaCharNum+=j->charNum;
			}
			PosCurParaCharNum+=PosCur.DataPos.index();
			while(i != PosPre.DataPos.parentNode())
			{
				i = i->nextNode;
				PosPrePara++;
			}
		}else if(ChangeColorFlag == 2)
		{
			while(i != PosPre.DataPos.parentNode())
			{
				i = i->nextNode;
				PosCurPara++;
				PosPrePara++;
			}
			while(i != PosCur.DataPos.parentNode())
			{
				i = i->nextNode;
				PosCurPara++;
			}
			auto j = i->firstHeap;
			while(j != PosCur.DataPos.parentHeap())
			{
				j = j->nextHeap;
				PosCurParaCharNum+=j->charNum;
			}
			PosCurParaCharNum+=PosCur.DataPos.index();
		}
		MyPainter.drawText(FirstQCharX+TextBoxWidth-100,FirstQCharY-8,
		QString("line:%1 colume:%2").arg(PosCurPara).arg(PosCurParaCharNum));
	}else if(LineShowFlag == 1)
	{
		for(int line = 1;line <= DataTextHeight - DataTextTop&&line <= TextBoxHeight;line++)
		{
			if(ChangeColorFlag == 0){
				if(line-1==PosCur.ShowPosY)
					MyPainter.setPen(Qt::black);
				else MyPainter.setPen(QColor(150,150,150));
			}else if(ChangeColorFlag == 1)
			{
				if(line-1>=PosCur.ShowPosY&&line-1<=PosPre.ShowPosY)
					MyPainter.setPen(Qt::black);
				else MyPainter.setPen(QColor(150,150,150));
			}else if(ChangeColorFlag == 2)
			{
				if(line-1>=PosPre.ShowPosY&&line-1<=PosCur.ShowPosY)
					MyPainter.setPen(Qt::black);
				else MyPainter.setPen(QColor(150,150,150));
			}
			MyPainter.drawText(QRect(0,FirstQCharY+(line-1)*FontSizeH,FirstQCharX,FontSizeH),
							   Qt::AlignCenter,QString("%1").arg(line + DataTextTop));
			}
		MyPainter.setPen(QColor(150,150,150));
		MyPainter.drawText(FirstQCharX+TextBoxWidth-100,FirstQCharY-8,
							QString("line:%1 colume:%2").arg(PosCur.ShowPosY+DataTextTop+1).arg(PosCur.ShowPosX/FontSizeW));
	}																	//绘制额外内容

	auto i=PosLeftUp.DataPos;
	int ParaCounter = DataParaTop+1;
	i.clear();
	MyPainter.setFont(QFont("楷体",FontSizeW));
	MyPainter.setPen(Qt::black);
	if(PosCur.ShowPosY<0&&PosPre.ShowPosY>=0)MyPainter.setPen(Qt::white);
	if(PosPre.ShowPosY<0&&PosCur.ShowPosY>=0)MyPainter.setPen(Qt::white);
	if(LineShowFlag == 0&&(i == data.begin()||(i-1&&*(i-1)=='\n')))
	{
		FontRestore = MyPainter.font();
		PenRestore = MyPainter.pen();
		MyPainter.setFont(QFont("楷体",8));
		if((ParaCounter>=PosCurPara&&ParaCounter<=PosPrePara)||(ParaCounter<=PosCurPara&&ParaCounter>=PosPrePara))
			MyPainter.setPen(Qt::black);
		else MyPainter.setPen(QColor(150,150,150));
		MyPainter.drawText(QRect(0,FirstQCharY,FirstQCharX,FontSizeH),
						   Qt::AlignCenter,QString("%1").arg(ParaCounter));
		MyPainter.setFont(FontRestore);
		MyPainter.setPen(PenRestore);
	}
	ParaCounter++;
	while((!i.isOverFlow())&&(DrawY<TextBoxHeight))
	{
		if(ChangeColorFlag==1)
		{
			if(i==PosCur.DataPos)MyPainter.setPen(Qt::white);
			else if(i==PosPre.DataPos)MyPainter.setPen(Qt::black);
		}else if(ChangeColorFlag==2)
		{
			if(i==PosCur.DataPos)MyPainter.setPen(Qt::black);
			else if(i==PosPre.DataPos)MyPainter.setPen(Qt::white);
		}
		if((*i)=='\n')
		{
			DrawY++;
			DrawX=0;
			i++;
		}else if((*i)=='\t')
		{
			if(DrawX<TextBoxWidth)
			{
				int DrawXRestore = DrawX;
				DrawX=(1+DrawX/(FontSizeW*TabWidth))*FontSizeW*TabWidth;
				if(DrawX>=TextBoxWidth)
				{
					DrawX=TextBoxWidth;
				}
				switch(TabStyle)
				{
				case 1:
					break;
				case 2:
					MyPainter.fillRect(FirstQCharX+DrawXRestore+1,FirstQCharY+DrawY*FontSizeH+1,
									   DrawX-DrawXRestore-2,FontSizeH-2,QColor(150,150,150));
					break;
				case 3:
					PenRestore = MyPainter.pen();
					MyPainter.setPen(Qt::black);
					MyPainter.drawRect(FirstQCharX-2,FirstQCharY+DrawY*FontSizeH,
										DrawX,FontSizeH);
					MyPainter.setPen(PenRestore);
					break;
				}
				i++;
			}else
			{
				DrawX=0;
				DrawY++;
			}
		}else if((*i).unicode()>=0x1000&&(*i).unicode()<=0xffff)
		{
			if(DrawX+FontSizeW*2<TextBoxWidth)
			{
				MyPainter.drawText(QRect(FirstQCharX+DrawX,FirstQCharY+DrawY*FontSizeH,2*FontSizeW,FontSizeH),
								Qt::AlignCenter,QString("%1").arg(*i));
				DrawX+=FontSizeW*2;
				i++;
			}else
			{
				DrawY++;
				DrawX=0;
			}
		}else
		{
			if(DrawX+FontSizeW<TextBoxWidth)
			{
				if((*i) == ' ')
				{
					switch(SpaceStyle)
					{
					case 1:break;
					case 2:
						MyPainter.fillRect(FirstQCharX+DrawX+1,FirstQCharY+DrawY*FontSizeH+1,
										   FontSizeW-2,FontSizeH-2,QColor(150,150,150));
						break;
					}
				}else
				{
					MyPainter.drawText(QRect(FirstQCharX+DrawX,FirstQCharY+DrawY*FontSizeH,FontSizeW,FontSizeH),
									   Qt::AlignCenter,QString("%1").arg(*i));
				}
				DrawX+=FontSizeW;
				i++;
			}else
			{
				DrawY++;
				DrawX=0;
			}
		}
		if(LineShowFlag == 0 &&i-1&&*(i-1) == '\n'&&DrawY<TextBoxHeight)
		{
			FontRestore = MyPainter.font();
			PenRestore = MyPainter.pen();
			MyPainter.setFont(QFont("楷体",8));
			if((ParaCounter>=PosCurPara&&ParaCounter<=PosPrePara)||(ParaCounter<=PosCurPara&&ParaCounter>=PosPrePara))
				MyPainter.setPen(Qt::black);
			else MyPainter.setPen(QColor(150,150,150));
			MyPainter.drawText(QRect(0,FirstQCharY+DrawY*FontSizeH,FirstQCharX,FontSizeH),
							   Qt::AlignCenter,QString("%1").arg(ParaCounter));
			ParaCounter++;
			MyPainter.setFont(FontRestore);
			MyPainter.setPen(PenRestore);
		}
	}																	//绘制文本

	MyScrollBar->move(FirstQCharX+TextBoxWidth+5,FirstQCharY);
	MyScrollBar->setFixedSize(15,newHeigh);
	MyScrollBar->setRange(0,DataTextHeight-1);
	MyScrollBar->setSliderPosition(DataTextTop);

	CursorTimer=!CursorTimer;
}
void MainWindow::on_action_New_triggered()
{
    newFile();
}
void MainWindow::on_action_Open_triggered()
{
	if (maybeSave())
	{
    //ok to open file
        QString path = QFileDialog::getOpenFileName(this);
		if (!path.isEmpty())
		{
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

void MainWindow::dragEnterEvent(QDragEnterEvent *ev)
{
	ev->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *ev)
{
	int x = ev->pos().x() - FirstQCharX;
	int y = (ev->pos().y() - FirstQCharY)/FontSizeH;

	if(x<0)x=0;
	x = (x>TextBoxWidth-FontSizeW/2?TextBoxWidth-FontSizeW/2:x);
	if(y<0)LocateLeftUpCursor(DataTextTop-1);
	if(y>=TextBoxHeight)LocateLeftUpCursor(DataTextTop+1);

	LocateCursor(x,y);
	PosPre = PosCur;
	CursorTimer=1;
	MyCursorTimer.start(700);
	ProtectedUpdate();
}

void MainWindow::dropEvent(QDropEvent *ev)
{
	qDebug()<<"111";
	if(ev->mimeData()->hasFormat("text/uri-list"))
	{
		QString fileName = (ev->mimeData()->urls()).first().toLocalFile();
		if(!fileName.isEmpty())
		{
			if(maybeSave())
			{
				openFile(fileName);
			}
		}
	}else
	{
		if(ev->mimeData()->text() == "")return;
		if(PosCur.ShowPosY<0)PosLeftUp.DataPos = data.begin();
		PosCur.DataPos=data.add(PosCur.DataPos,ev->mimeData()->text());
		PosPre = PosCur;
		GetDataHeight();
		FindCursor();
	}
}
void MainWindow::on_action_Exit_triggered()
{
	if (maybeSave())
	{
        qApp->quit();//equal to exit(0)
    }
}
void MainWindow::on_action_Undo_triggered()
{
	CursorTimer=1;
	MyCursorTimer.start(700);
	if(ProtectedUpdateTimer == 0)return;
	PosPre.DataPos = PosCur.DataPos = data.undo(PosCur.DataPos);
	PosLeftUp.DataPos = data.begin();
	PosCur.ShowPosY = PosPre.ShowPosY = -1;
	GetDataHeight();
	FindCursor();
}
void MainWindow::on_action_Cut_triggered()
{
    //todo
	CursorTimer=1;
	MyCursorTimer.start(700);
	if(ProtectedUpdateTimer == 0)return;
	if(PosCur.DataPos == PosPre.DataPos)return;
	if(PosCur.ShowPosY<0||PosPre.ShowPosY<0)PosLeftUp.DataPos = data.begin();
	if(PosCur.ShowPosX+TextBoxWidth*PosCur.ShowPosY<PosPre.ShowPosX+TextBoxWidth*PosPre.ShowPosY)
	{
		PosCur.DataPos = data.cut(PosCur.DataPos,PosPre.DataPos);
		PosPre = PosCur;
	}else
	{
		PosCur.DataPos = data.cut(PosPre.DataPos,PosCur.DataPos);
		PosPre = PosCur;
	}
	GetDataHeight();
	FindCursor();
}
void MainWindow::on_action_Copy_triggered()
{
	FindCursor();
	if(ProtectedUpdateTimer == 0)return;
	ProtectedUpdate();
	if(PosCur.DataPos == PosPre.DataPos)return;
	if(PosCur.ShowPosX+TextBoxWidth*PosCur.ShowPosY<PosPre.ShowPosX+TextBoxWidth*PosPre.ShowPosY)
	{
		data.copy(PosCur.DataPos,PosPre.DataPos);
	}else
	{
		data.copy(PosPre.DataPos,PosCur.DataPos);
	}
}
void MainWindow::on_action_Paste_triggered()
{
    //todo
	CursorTimer = 1;
	MyCursorTimer.start(700);
	if(ProtectedUpdateTimer == 0)return;
	if(PosCur.ShowPosY<0||PosPre.ShowPosY<0)PosLeftUp.DataPos = data.begin();
	if(PosCur.DataPos==PosPre.DataPos)
	{
		PosCur.DataPos=data.paste(PosCur.DataPos,PosPre.DataPos);
		PosPre = PosCur;
	}else{
		if(PosCur.ShowPosX+TextBoxWidth*PosCur.ShowPosY<PosPre.ShowPosX+TextBoxWidth*PosPre.ShowPosY)
		{
			PosCur.DataPos = data.paste(PosCur.DataPos,PosPre.DataPos);
			PosPre = PosCur;
		}else
		{
			PosCur.DataPos = data.paste(PosPre.DataPos,PosCur.DataPos);
			PosPre = PosCur;
		}
	}
	GetDataHeight();
	FindCursor();
}
void MainWindow::on_action_Delete_triggered()
{
	CursorTimer=1;
	MyCursorTimer.start(700);
	if(ProtectedUpdateTimer == 0)return;
	if(PosCur.DataPos==PosPre.DataPos)
	{
		if(PosCur.ShowPosY < 0)
		{
			FindCursor();
		}
		PosCur.DataPos=data.del(PosCur.DataPos,PosPre.DataPos,true);
		PosPre = PosCur;
	}else
	{
		if(PosCur.ShowPosX+TextBoxWidth*PosCur.ShowPosY<PosPre.ShowPosX+TextBoxWidth*PosPre.ShowPosY)
		{
			PosCur.DataPos = data.del(PosCur.DataPos,PosPre.DataPos);
			PosPre = PosCur;
		}else
		{
			PosCur.DataPos = data.del(PosPre.DataPos,PosCur.DataPos);
			PosPre = PosCur;
		}
		if(PosCur.ShowPosY<0||PosPre.ShowPosY<0)PosLeftUp.DataPos = data.begin();
	}
	GetDataHeight();
	FindCursor();
}
void MainWindow::on_action_Find_triggered()
{
    replaceDlg->show();
}
void MainWindow::on_action_FindNext_triggered()
{
	if(replaceDlg->findLeText() == "")return;
	auto i = data.begin();
	if(PosCur.DataPos == PosPre.DataPos)
		i = data.find(PosCur.DataPos, replaceDlg->findLeText());
	else if(PosCur.ShowPosX + TextBoxWidth * PosCur.ShowPosY < PosPre.ShowPosX + TextBoxWidth * PosPre.ShowPosY)
		i = data.find(PosCur.DataPos + 1, replaceDlg->findLeText());
	else
		i = data.find(PosPre.DataPos + 1, replaceDlg->findLeText());

	if(i)
	{
		PosCur.DataPos = i;
		PosPre.DataPos = PosCur.DataPos + replaceDlg->findLeText().length();
	}else
	{
		i = data.find(data.begin(), replaceDlg->findLeText());
		if(i){
			PosCur.DataPos = i;
			PosPre.DataPos = PosCur.DataPos + replaceDlg->findLeText().length();
		}else
		{
			QMessageBox::information(this,"MiniWord","无法找到此字符串");
		}
	}
	GetDataHeight();
	FindCursor();
	ProtectedUpdate();
}
void MainWindow::on_action_Replace_triggered()
{
	replaceDlg->show();
}
void MainWindow::data_replace()
{
	if(ProtectedUpdateTimer == 0)return;
	if(replaceDlg->findLeText() == ""||replaceDlg->replaceLeText() == "")return;
	if(replaceDlg->findLeText() == replaceDlg->replaceLeText())return;
	if(PosCur.DataPos == PosPre.DataPos)
	{
		on_action_FindNext_triggered();
		return;
	}else
	{
		int PareLen = 0;
		int StrLen = replaceDlg->findLeText().length();
		if(PosCur.ShowPosX + TextBoxWidth * PosCur.ShowPosY < PosPre.ShowPosX + TextBoxWidth * PosPre.ShowPosY)
		{
			for(auto i = PosCur.DataPos;i != PosPre.DataPos && PareLen != StrLen;i++)
			{
				if((*i) == replaceDlg->findLeText()[PareLen])PareLen++;
				else break;
			}
			if(PareLen == StrLen)
				PosPre.DataPos = PosCur.DataPos = data.edit(PosCur.DataPos,PosPre.DataPos,replaceDlg->replaceLeText());
			else return on_action_FindNext_triggered();
		}else
		{
			for(auto i = PosPre.DataPos;i != PosCur.DataPos && PareLen != StrLen;i++)
			{
				if((*i) == replaceDlg->findLeText()[PareLen])PareLen++;
				else break;
			}
			if(PareLen == StrLen)
				PosPre.DataPos = PosCur.DataPos = data.edit(PosPre.DataPos,PosCur.DataPos,replaceDlg->replaceLeText());
			else
			{
				on_action_FindNext_triggered();
				return;
			}
		}
	}
	PosLeftUp.DataPos = data.begin();
	on_action_FindNext_triggered();
}

void MainWindow::data_replace_all()
{
	if(ProtectedUpdateTimer == 0)return;
	if(replaceDlg->findLeText() == ""||replaceDlg->replaceLeText() == "")return;
	if(replaceDlg->findLeText() == replaceDlg->replaceLeText())return;
	int len = replaceDlg->findLeText().length();
	auto i = data.begin();
	while(i = data.find(i,replaceDlg->findLeText()))
	{
		i = data.edit(i,(i+len),replaceDlg->replaceLeText());
	}
	PosLeftUp.DataPos = PosPre.DataPos = PosCur.DataPos = data.begin();
	GetDataHeight();
}
void MainWindow::on_action_SelectAll_triggered()
{
	if(ProtectedUpdateTimer == 0)return;
	PosLeftUp.DataPos = PosCur.DataPos = data.begin();
	PosPre.DataPos = data.end();
	DataTextTop = 0;
	GetDataHeight();
	ProtectedUpdate();
}
void MainWindow::getMenu_E_state()
{
    //active all action
    ui->action_Undo->setDisabled(false);
	ui->action_Redo->setDisabled(false);
    ui->action_Cut->setDisabled(false);
    ui->action_Copy->setDisabled(false);
    ui->action_Paste->setDisabled(false);
    ui->action_Delete->setDisabled(false);
    ui->action_Find->setDisabled(false);
    ui->action_FindNext->setDisabled(false);
    ui->action_Replace->setDisabled(false);
    //judge function state
	if (!PosCur.DataPos || !PosPre.DataPos || PosCur.DataPos == PosPre.DataPos)
	{
        ui->action_Cut->setDisabled(true);
        ui->action_Copy->setDisabled(true);
        ui->action_Delete->setDisabled(true);
    }
    //todo if UndoStack is empty then disable undo action
    //judge replacedlg
	if (!replaceDlg->findLeText().length())
	{
        ui->action_FindNext->setDisabled(true);
	}
	if (data.undoStackEmpty())
	{
		ui->action_Undo->setDisabled(true);
	}
	if (data.redoStackEmpty())
	{
		ui->action_Redo->setDisabled(true);
	}
	ui->action_SelectAll->setDisabled(false);
}

void MainWindow::getDataChanged()
{
	shouldSave = true;
	if (isUntitled) return;
	setWindowTitle(curFile + '*' + " - MiniWord");
}

void MainWindow::LocateCursor(int x,int y)
{
	int MoveX = 0;
	int MoveY = 0;
	auto i = PosLeftUp.DataPos;
	i.clear();
	while((!i.isOverFlow()) && MoveY < y)
	{
		if((*i)=='\n')
		{
			i++;
			if(!i.isOverFlow())
			{
				MoveY++;
				MoveX=0;
			}
		}else if((*i)=='\t')
		{
			if(MoveX<TextBoxWidth)
			{
				MoveX=(1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth;
				if(MoveX>=TextBoxWidth)
				{
					MoveX=TextBoxWidth;
				}
				i++;
			}else
			{
				MoveX=0;
				MoveY++;
			}
		}else if((*i).unicode()>=0x1000&&(*i).unicode()<=0xffff)
		{
			if(MoveX+FontSizeW*2<TextBoxWidth)
			{
				MoveX+=FontSizeW*2;
				i++;
			}else
			{
				MoveX=0;
				MoveY++;
			}
		}else
		{
			if(MoveX+FontSizeW<TextBoxWidth)
			{
				MoveX+=FontSizeW;
				i++;
			}else
			{
				MoveX=0;
				MoveY++;
			}
		}
	}
	while((!i.isOverFlow()))
	{
		if((*i)=='\n')
		{
			break;
		}else if((*i)=='\t')
		{
			if((1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth>x)
			{
				if(x==TextBoxWidth-FontSizeW/2)
				{
					MoveX=TextBoxWidth;
					i++;
				}else if(MoveX+FontSizeW<x)
				{
					MoveX=(1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth;
					i++;
				}
				break;
			}else
			{
				MoveX=(1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth;
				i++;
			}
		}else if((*i).unicode()>=0x1000&&(*i).unicode()<=0xffff)
		{
			if(MoveX+FontSizeW*2 < x)
			{
				MoveX+=FontSizeW*2;
				i++;
			}else if(MoveX+FontSizeW < x)
			{
				if(MoveX+2*FontSizeW<TextBoxWidth)
				{
					MoveX+=FontSizeW*2;
					i++;
				}
				break;
			}else
			{
				break;
			}
		}else
		{
			if(MoveX+FontSizeW < x)
			{
				MoveX+=FontSizeW;
				i++;
			}else if(MoveX+FontSizeW/2 < x)
			{
				MoveX+=FontSizeW;
				i++;
				break;
			}else
			{
				break;
			}
		}
	}
	PosCur.ShowPosX = MoveX;
	if(PosCur.ShowPosX > TextBoxWidth)
	{
		PosCur.ShowPosX = TextBoxWidth;
	}
	PosCur.ShowPosY = MoveY;
	PosCur.DataPos = i;
	PosCur.DataPos.clear();
}
void MainWindow:: LocateLeftUpCursor(int newDataTextTop,int flag)
{
	if(newDataTextTop == DataTextTop&&flag == 0)return;
	if(newDataTextTop < 0)newDataTextTop = 0;
	if(newDataTextTop >= DataTextHeight)newDataTextTop = DataTextHeight-1;

	PosCur.ShowPosY -= newDataTextTop - DataTextTop;
	PosPre.ShowPosY -= newDataTextTop - DataTextTop;
	auto i = data.begin();
	int NextDataTextTop = newDataTextTop;
	if(newDataTextTop > DataTextTop && flag == 0)
	{
		i = PosLeftUp.DataPos;
		newDataTextTop -= DataTextTop;
	}else DataParaTop = 0;
	PosLeftUp.DataPos = i;
	int MoveX = 0;
	int MoveY = 0;

	i.clear();
	while(!(i.isOverFlow()) && MoveY!=newDataTextTop)
	{
		if((*i)=='\n')
		{
			if(i+1)DataParaTop++;
			MoveY++;
			MoveX = 0;
			i++;
			PosLeftUp.DataPos = i;
		}else if((*i)=='\t')
		{
			if(MoveX<TextBoxWidth)
			{
				if((1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth>TextBoxWidth)
				{
					MoveX=TextBoxWidth;
				}else
				{
					MoveX=(1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth;
				}
				i++;
			}else
			{
				MoveX=0;
				MoveY++;
				PosLeftUp.DataPos = i;
			}
		}else if((*i).unicode()>=0x1000&&(*i).unicode()<=0xffff)
		{
			if(MoveX+FontSizeW*2<TextBoxWidth)
			{
				MoveX+=FontSizeW*2;
				i++;
			}else
			{
				MoveY++;
				MoveX=0;
				PosLeftUp.DataPos = i;
			}
		}else
		{
			if(MoveX+FontSizeW<TextBoxWidth)
			{
				MoveX+=FontSizeW;
				i++;
			}else
			{
				MoveY++;
				MoveX=0;
				PosLeftUp.DataPos = i;
			}
		}
	}
	DataTextTop = NextDataTextTop;
}

void MainWindow::FillBlueArea(Pos&pos1,Pos&pos2,QPainter*painter)
{
	auto i = pos1.DataPos;
	int MoveX = pos1.ShowPosX;
	int MoveY = pos1.ShowPosY;
	if(pos1.ShowPosY<0)
	{
		i = PosLeftUp.DataPos;
		MoveX = 0;
		MoveY = 0;
	}
	i.clear();
	while(!(i == pos2.DataPos) && MoveY < TextBoxHeight)
	{
		if((*i)=='\n')
		{
			painter->fillRect(MoveX+FirstQCharX,MoveY*FontSizeH+FirstQCharY,2,FontSizeH,QColor(0,0,255));
			MoveY++;
			MoveX = 0;
			i++;
		}else if((*i)=='\t')
		{
			if(MoveX<TextBoxWidth){
				if((1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth>TextBoxWidth)
				{
					painter->fillRect(MoveX+FirstQCharX,MoveY*FontSizeH+FirstQCharY,
									  TextBoxWidth-MoveX,FontSizeH,QColor(0,0,255));
					MoveX=TextBoxWidth;
				}else
				{
					painter->fillRect(MoveX+FirstQCharX,MoveY*FontSizeH+FirstQCharY,
									  (1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth-MoveX,FontSizeH,QColor(0,0,255));
					MoveX=(1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth;
				}
				i++;
			}else
			{
				MoveX=0;
				MoveY++;
			}
		}else if((*i).unicode()>=0x1000&&(*i).unicode()<=0xffff)
		{
			if(MoveX+FontSizeW*2<TextBoxWidth)
			{
				painter->fillRect(MoveX+FirstQCharX,MoveY*FontSizeH+FirstQCharY,2*FontSizeW,FontSizeH,QColor(0,0,255));
				MoveX+=FontSizeW*2;
				i++;
			}else
			{
				MoveY++;
				MoveX=0;
			}
		}else{
			if(MoveX+FontSizeW<TextBoxWidth)
			{
				painter->fillRect(MoveX+FirstQCharX,MoveY*FontSizeH+FirstQCharY,FontSizeW,FontSizeH,QColor(0,0,255));
				MoveX+=FontSizeW;
				i++;
			}else
			{
				MoveY++;
				MoveX=0;
			}
		}
	}
}
void MainWindow::GetDataHeight()
{
	auto i = data.begin();

	int MoveX = 0;
	int MoveY = 0;
	int PosCurX = 0,PosCurY = 0;
	int PosPreX = 0,PosPreY = 0;
	int ParaNum = 0;
	i.clear();
	while(!(i.isOverFlow()))
	{
		if(i == PosLeftUp.DataPos)
		{
			DataTextTop = MoveY;
			DataParaTop =  ParaNum;
			if(MoveX!=0)DataTextTop++;
		}
		if(i == PosCur.DataPos)
		{
			PosCurX = MoveX;
			PosCurY = MoveY;
		}
		if(i == PosPre.DataPos)
		{
			PosPreX = MoveX;
			PosPreY = MoveY;
		}
		if((*i)=='\n')
		{
			if(i+1)ParaNum++;
			MoveY++;
			MoveX = 0;
			i++;
		}else if((*i)=='\t'){
			if(MoveX<TextBoxWidth)
			{
				if((1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth>TextBoxWidth)
				{
					MoveX=TextBoxWidth;
				}else
				{
					MoveX=(1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth;
				}
				i++;
			}else
			{
				MoveX=0;
				MoveY++;
			}
		}else if((*i).unicode()>=0x1000&&(*i).unicode()<=0xffff)
		{
			if(MoveX+FontSizeW*2<TextBoxWidth)
			{
				MoveX+=FontSizeW*2;
				i++;
			}else
			{
				MoveY++;
				MoveX=0;
			}
		}else
		{
			if(MoveX+FontSizeW<TextBoxWidth)
			{
				MoveX+=FontSizeW;
				i++;
			}else
			{
				MoveY++;
				MoveX=0;
			}
		}
	}
	DataTextHeight = MoveY;
	PosCurY -= DataTextTop;
	PosPreY -= DataTextTop;
	PosCur.ShowPosX=PosCurX;
	PosCur.ShowPosY=PosCurY;
	PosPre.ShowPosX=PosPreX;
	PosPre.ShowPosY=PosPreY;
}

void MainWindow::FindCursor()
{
	if(PosCur.ShowPosY<0)
	{
		LocateLeftUpCursor(DataTextTop+PosCur.ShowPosY);
	}else if(PosCur.ShowPosY>=TextBoxHeight)
	{
		LocateLeftUpCursor(DataTextTop+PosCur.ShowPosY-TextBoxHeight+1);
	}
	MyScrollBar->setSliderPosition(DataTextTop);
}

void MainWindow::Rolling(int flag)
{
	if(flag==0)										//向上滑
	{
		if(DataTextTop > 0)
		{
			LocateLeftUpCursor(DataTextTop-1);
		}
	}else{
		if(DataTextHeight - DataTextTop > 1)		//向下滑
		{
			LocateLeftUpCursor(DataTextTop+1);
		}
	}
	MyScrollBar->setSliderPosition(DataTextTop);
}
void MainWindow::ChangeFontSize(int Size)
{
	FontSizeW = Size;
	FontSizeH = FontSizeW*2;

	int TopPre = DataTextTop;
	GetDataHeight();
	if(TopPre >= DataTextHeight)TopPre = DataTextHeight - 1;
	LocateLeftUpCursor(TopPre,1);
	MyScrollBar->setSliderPosition(TopPre);
}

void MainWindow::ProtectedUpdate()
{
	if(ProtectedUpdateTimer == 1)
	{
		ProtectedUpdateTimer = 0;
		QTimer::singleShot(0,this,SLOT(update()));
	}
}

void MainWindow::RefreshProtectTimer()
{
	ProtectedUpdateTimer = 1;
	int newScrollPos = MyScrollBar->sliderPosition();
	if(newScrollPos != oldScrollPos)
	{
		LocateLeftUpCursor(MyScrollBar->sliderPosition());
		ProtectedUpdate();
	}
	oldScrollPos = newScrollPos;
}
void MainWindow::on_action_Redo_triggered()
{
	CursorTimer=1;
	MyCursorTimer.start(700);
	if(ProtectedUpdateTimer == 0)return;
	PosPre.DataPos = PosCur.DataPos = data.redo(PosCur.DataPos);
	PosLeftUp.DataPos = data.begin();
	PosCur.ShowPosY = PosPre.ShowPosY = -1;
	GetDataHeight();
	FindCursor();
}

void MainWindow::on_action_Setting_triggered()
{
	if (settingsDlg->exec() == QDialog::Accepted){
		setConfig();
		data.resetStackSize(settingsDlg->maxUndoTime());
		SpaceStyle = settingsDlg->spaceStyle();
		TabWidth = settingsDlg->tabSize();
		TabStyle = settingsDlg->tabStyle();
		ChangeFontSize((settingsDlg->defaultFontSize() + 1) * 4);
		ProtectedUpdate();
	}
}

void MainWindow::on_action_GetCharNum_triggered()
{
	auto it_begin = data.begin();
	auto it_end = data.end();
	int All_num = 0,Chinese_char_num = 0,English_char_num = 0,Para_num = 1,Number_num = 0,Other_num = 0;
	if(PosCur.DataPos != PosPre.DataPos)
		if(PosCur.ShowPosX+TextBoxWidth*PosCur.ShowPosY<PosPre.ShowPosX+TextBoxWidth*PosPre.ShowPosY)
		{
			it_begin = PosCur.DataPos;
			it_end = PosPre.DataPos;
		}else
		{
			it_begin = PosPre.DataPos;
			it_end = PosCur.DataPos;
		}
	while(it_begin != it_end)
	{
		All_num++;
		if(*it_begin >= 0x4e00&& *it_begin <= 0x9fa5)
		{
			Chinese_char_num++;
		}else if((*it_begin >= 'A' && *it_begin <= 'Z')||(*it_begin >= 'a' && *it_begin <= 'z'))
		{
			English_char_num++;
		}else if(*it_begin >= '0' && *it_begin <= '9')
		{
			Number_num++;
		}else if(*it_begin == '\n')
		{
			Para_num++;
		}else
		{
			Other_num++;
		}
		it_begin++;
	}
	charnumDlg->SetCharNum(All_num,Chinese_char_num,English_char_num,Para_num,Number_num,Other_num);
	charnumDlg->exec();
}

void MainWindow::on_action_AddTime_triggered()
{
	if(ProtectedUpdateTimer == 0)return;
	PosPre.DataPos = PosCur.DataPos = data.add(data.end(),"\n"+QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ddd"));
	GetDataHeight();
	FindCursor();
}

void MainWindow::on_action_toLine_triggered()
{
	int toLine = QInputDialog::getInt(this,"MiniWord-转到行","请输入行号",QLineEdit::NoEcho);
	if(toLine >= 1&&toLine <= DataTextHeight)
	{
		LocateLeftUpCursor(toLine - 1);
		PosCur.DataPos = PosLeftUp.DataPos;
		PosCur.ShowPosX = 0;
		PosCur.ShowPosY = 0;
		PosPre = PosCur;
		ProtectedUpdate();
	}else
	{
		QMessageBox::information(this,"MiniWord","行数不存在");
	}
}

void MainWindow::on_action_toParagraph_triggered()
{
	int toPara = QInputDialog::getInt(this,"MiniWord-转到段","请输入段号",QLineEdit::NoEcho);
	if(toPara > 0)
	{
		int paraCur = 1;
		auto i = data.begin();
		while(i && paraCur != toPara)
		{
			if(*i == '\n')
			{
				i++;
				if(i)
				{
					paraCur++;
				}
			}else i++;
		}
		if(paraCur == toPara)
		{
			PosPre.DataPos = PosCur.DataPos = PosLeftUp.DataPos = i;
			GetDataHeight();
			ProtectedUpdate();
		}else
		{
			QMessageBox::information(this,"MiniWord","段数不存在");
		}
	}else
	{
		QMessageBox::information(this,"MiniWord","段数不存在");
	}
}

void MainWindow::on_action_ShowLine_triggered()
{
	if(LineShowFlag == 0)
	{
		LineShowFlag = 1;
	}else if(LineShowFlag == 1)
	{
		LineShowFlag = 0;
	}
	ProtectedUpdate();
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QPushButton>
#include <QTextStream>

const int FirstQCharX = 10;
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
	//================== right click menu ============
	initRightMenu();
	connect(ui->menu_E, &QMenu::aboutToShow, this, &MainWindow::getMenu_E_state);
	//================== replace and find dialog ========
	replaceDlg = new ReplaceDlg(this);
	//================= input support ===========
	setFocusPolicy(Qt::ClickFocus);
	setAttribute(Qt::WA_InputMethodEnabled, true);
	centralWidget()->setMouseTracking(true);
	setMouseTracking(true);

	MyCursorTimer.start(700);
	MyProtectedUpdateTimer.start(33);
	connect(&MyCursorTimer, SIGNAL(timeout()),this,SLOT(ProtectedUpdate()));
	connect(&MyProtectedUpdateTimer, SIGNAL(timeout()),this,SLOT(RefreshProtectedUpdateTimer()));
	connect(&data,SIGNAL(WindowUdate()),this,SLOT(ProtectedUpdate()));

	DataTextHeight = 0;
	DataTextTop = 0;
	TextBoxHeight = 20;
	TextBoxWidth = 1000;
	FontSizeW = 8;
	FontSizeH = 15;
	TabWidth = 8;
	CursorTimer = 0;

	ProtectedUpdateTimer=1;

	PosCur=PosPre=PosLeftUp={0,0,data.begin()};
	if(!PosCur.DataPos)qDebug("aaa");
	data.add(data.begin(),QString("aslkfjasklfjlkasjflkas\nslkajflkasjflkasflksaj\nlaskjdlasjdlasj\naskldjlaskdjklasdlkasdla\nlaskjdlkajdlkasjldask\nlksadjlasdjaslkjdl"));
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
}
void MainWindow::resetRightMenu()
{
    //activate all methods
    undoAction->setDisabled(false);
    cutAction->setDisabled(false);
    copyAction->setDisabled(false);
    pasteAction->setDisabled(false);
    delAction->setDisabled(false);
    seleteAllAction->setDisabled(false);
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
	switch (ev->key()){
	case Qt::Key_Tab:
		if(PosCur.DataPos==PosPre.DataPos)
			PosPre.DataPos=PosCur.DataPos=data.add(PosCur.DataPos,tr("\t"));
		break;
    case Qt::Key_Backspace :
			PosCur.DataPos = data.del(PosPre.DataPos,PosCur.DataPos);
			PosPre.DataPos=PosCur.DataPos;

        break;
    case Qt::Key_Enter :
    case Qt::Key_Return :
				PosCur.DataPos = data.add(PosCur.DataPos, tr("\n"));

		PosPre.DataPos=PosCur.DataPos;
        break;
		case Qt::Key_Delete :
			PosCur.DataPos = data.del(PosPre.DataPos,PosCur.DataPos, true);
			PosPre.DataPos=PosCur.DataPos;
        break;
	case Qt::Key_Up :
        //todo
        break;
	case Qt::Key_Down :
        //todo
        break;
	case Qt::Key_Left :
		if(QApplication::keyboardModifiers () == Qt::ShiftModifier){
			PosCur.DataPos--;
		}else if(PosCur.DataPos==PosPre.DataPos){
			PosCur.DataPos--;
			PosPre.DataPos--;
		}else{
			if(PosCur.ShowPosY>PosPre.ShowPosY){
				PosCur = PosPre;
			}else if(PosCur.ShowPosY<PosPre.ShowPosY){
				PosPre = PosCur;
			}else if(PosCur.ShowPosX>PosPre.ShowPosX){
				PosCur = PosPre;
			}else if(PosCur.ShowPosX<PosPre.ShowPosX){
				PosPre = PosCur;
			}
		}
		QTimer::singleShot(0,this,SLOT(update()));
        break;
	case Qt::Key_Right :
		if(QApplication::keyboardModifiers () == Qt::ShiftModifier){
			PosCur.DataPos++;
		}else if(PosCur.DataPos==PosPre.DataPos){
			PosCur.DataPos++;
			PosPre.DataPos++;
		}else{
			if(PosCur.ShowPosY>PosPre.ShowPosY){
				PosPre = PosCur;
			}else if(PosCur.ShowPosY<PosPre.ShowPosY){
				PosCur = PosPre;
			}else if(PosCur.ShowPosX>PosPre.ShowPosX){
				PosPre = PosCur;
			}else if(PosCur.ShowPosX<PosPre.ShowPosX){
				PosCur = PosPre;
			}
		}
		QTimer::singleShot(0,this,SLOT(update()));
        break;
    case Qt::Key_Home :
        //todo
        break;
    case Qt::Key_End :
        //todo
        break;
    default :
		if(PosCur.DataPos==PosPre.DataPos)
			PosPre.DataPos=PosCur.DataPos=data.add(PosCur.DataPos, ev->text());
		break;
    }
	CursorTimer=1;
	MyCursorTimer.start(700);
	//QTimer::singleShot(0,this,SLOT(update()));
}
void MainWindow::inputMethodEvent(QInputMethodEvent * ev)
{
	if(PosCur.DataPos==PosPre.DataPos)
		PosPre.DataPos=PosCur.DataPos=data.add(PosCur.DataPos, ev->commitString());
}
void MainWindow::mousePressEvent(QMouseEvent * ev)
{
    if (ev->button() == Qt::RightButton){//show right click menu
        //judge function state
        if (!PosCur.DataPos || !PosPre.DataPos || PosCur.DataPos == PosPre.DataPos){
            //iterator are overflow or equal
            cutAction->setDisabled(true);
            copyAction->setDisabled(true);
            delAction->setDisabled(true);
        }
        //todo if UndoStack is empty then disable undo action
				rightMenu->exec(ev->screenPos().toPoint());
        resetRightMenu();
    }else if(ev->button() == Qt::LeftButton){
		int x = ev->pos().x() - FirstQCharX;
		x = (x>TextBoxWidth-FontSizeW/2?TextBoxWidth-FontSizeW/2:x);
		int y = (ev->pos().y() - FirstQCharY)/FontSizeH;
		y = (y>TextBoxHeight-1?TextBoxHeight-1:y);
		int MoveX = 0;
		int MoveY = 0;
		auto i = PosLeftUp.DataPos;
		for(i = PosLeftUp.DataPos; (!i.isOverFlow()) && MoveY < y;){
			if((*i)=='\n')
			{
				i++;
				if(!i.isOverFlow()){
					MoveY++;
					MoveX=0;
				}
			}else if((*i)=='\t'){
				if(MoveX<TextBoxWidth){
					MoveX=(1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth;
					if(MoveX>=TextBoxWidth){
						MoveX=TextBoxWidth;
					}
					i++;
				}else{
					MoveX=0;
					MoveY++;
				}
			}else if((*i).unicode()>=0x4e00&&(*i).unicode()<=0x9fa5){
				if(MoveX+FontSizeW*2<TextBoxWidth)
				{
					MoveX+=FontSizeW*2;
					i++;
				}else{
					MoveX=0;
					MoveY++;
				}
			}else{
				if(MoveX+FontSizeW<TextBoxWidth)
				{
					MoveX+=FontSizeW;
					i++;
				}else{
					MoveX=0;
					MoveY++;
				}
			}
		}
		for(;(!i.isOverFlow());){
			if((*i)=='\n')
			{
				break;
			}else if((*i)=='\t'){
				if((1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth>x){
					if(x==TextBoxWidth-FontSizeW/2||MoveX+FontSizeW<x){
						MoveX=TextBoxWidth;
						i++;
					}
					break;
				}else{
					MoveX=(1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth;
					i++;
				}
			}else if((*i).unicode()>=0x4e00&&(*i).unicode()<=0x9fa5){
				if(MoveX+FontSizeW*2 < x)
				{
					MoveX+=FontSizeW*2;
					i++;
				}else if(MoveX+FontSizeW < x){
					if(MoveX+2*FontSizeW<TextBoxWidth){
						MoveX+=FontSizeW*2;
						i++;
					}
					break;
				}else{
					break;
				}
			}else{
				if(MoveX+FontSizeW < x)
				{
					MoveX+=FontSizeW;
					i++;
				}else if(MoveX+FontSizeW/2 < x){
					MoveX+=FontSizeW;
					i++;
					break;
				}else{
					break;
				}
			}
		}

		PosCur.DataPos = i;
		PosCur.DataPos.clear();
		PosPre = PosCur;
		MyCursorTimer.start(700);
		CursorTimer=1;
		ProtectedUpdate();
	}
}
void MainWindow::mouseDoubleClickEvent(QMouseEvent *ev)
{
	if(ev->button() == Qt::LeftButton){
		PosPre.DataPos = PosCur.DataPos--;
		QTimer::singleShot(0,this,SLOT(update()));
	}
}
void MainWindow::mouseMoveEvent(QMouseEvent *ev)
{
	int x = ev->pos().x() - FirstQCharX;
	x = (x>TextBoxWidth-FontSizeW/2?TextBoxWidth-FontSizeW/2:x);
	int y = (ev->pos().y() - FirstQCharY)/FontSizeH;
	y = (y>TextBoxHeight-1?TextBoxHeight-1:y);

	if(x>=0&&x<TextBoxWidth&&y>=0&&y<TextBoxHeight){
		setCursor(Qt::IBeamCursor);
	}else{
		setCursor(Qt::ArrowCursor);
	}

	if(ev->buttons() & Qt::LeftButton){
		int x = ev->pos().x() - FirstQCharX;
		x = (x>TextBoxWidth-FontSizeW/2?TextBoxWidth-FontSizeW/2:x);
		int y = (ev->pos().y() - FirstQCharY)/FontSizeH;
		y = (y>TextBoxHeight-1?TextBoxHeight-1:y);
		int MoveX = 0;
		int MoveY = 0;
		auto i = PosLeftUp.DataPos;
		for(i = PosLeftUp.DataPos; (!i.isOverFlow()) && MoveY < y;){
			if((*i)=='\n')
			{
				i++;
				if(!i.isOverFlow()){
					MoveY++;
					MoveX=0;
				}
			}else if((*i)=='\t'){
				if(MoveX<TextBoxWidth){
					MoveX=(1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth;
					if(MoveX>=TextBoxWidth){
						MoveX=TextBoxWidth;
					}
					i++;
				}else{
					MoveX=0;
					MoveY++;
				}
			}else if((*i).unicode()>=0x4e00&&(*i).unicode()<=0x9fa5){
				if(MoveX+FontSizeW*2<TextBoxWidth)
				{
					MoveX+=FontSizeW*2;
					i++;
				}else{
					MoveX=0;
					MoveY++;
				}
			}else{
				if(MoveX+FontSizeW<TextBoxWidth)
				{
					MoveX+=FontSizeW;
					i++;
				}else{
					MoveX=0;
					MoveY++;
				}
			}
		}
		for(;(!i.isOverFlow());){
			if((*i)=='\n')
			{
				break;
			}else if((*i)=='\t'){
				if((1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth>x){
					if(x==TextBoxWidth-FontSizeW/2||MoveX+FontSizeW<x){
						MoveX=TextBoxWidth;
						i++;
					}
					break;
				}else{
					MoveX=(1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth;
					i++;
				}
			}else if((*i).unicode()>=0x4e00&&(*i).unicode()<=0x9fa5){
				if(MoveX+FontSizeW*2 < x)
				{
					MoveX+=FontSizeW*2;
					i++;
				}else if(MoveX+FontSizeW < x){
					if(MoveX+2*FontSizeW<TextBoxWidth){
						MoveX+=FontSizeW*2;
						i++;
					}
					break;
				}else{
					break;
				}
			}else{
				if(MoveX+FontSizeW < x)
				{
					MoveX+=FontSizeW;
					i++;
				}else if(MoveX+FontSizeW/2 < x){
					MoveX+=FontSizeW;
					i++;
					break;
				}else{
					break;
				}
			}
		}
		PosCur.DataPos = i;
		PosCur.DataPos.clear();
		//PosPre = PosCur;
		ProtectedUpdate();
	}
}

void MainWindow::wheelEvent(QWheelEvent *ev)
{
	if(ev->delta() > 0){
		if(QApplication::keyboardModifiers () == Qt::ControlModifier){
			if(FontSizeW<40){
				FontSizeW+=8;
				FontSizeH+=15;
			}
		}else{

		}
	}else{
		if(QApplication::keyboardModifiers () == Qt::ControlModifier){
			if(FontSizeW>8){
				FontSizeW-=8;
				FontSizeH-=15;
			}
		}else{

		}
	}
	ProtectedUpdate();
}
void MainWindow::paintEvent(QPaintEvent *ev)
{
	QPainter MyPainter(this);                                           //A Painter
	MyPainter.setFont(QFont("楷体",FontSizeW*10/8));

    int DrawX=0,DrawY=0;                                                //the position to draw text

	int newWidth = width() - 20;
	int newHeigh = (height() - 60)/FontSizeH;
	int ChangeColorFlag = 0;

	if(TextBoxHeight != newHeigh)TextBoxHeight = newHeigh;

	if(TextBoxWidth != newWidth)
	{
		TextBoxWidth = newWidth;
	}

	RefreshShowPos();

	MyPainter.fillRect(FirstQCharX,FirstQCharY,TextBoxWidth,TextBoxHeight*FontSizeH,Qt::white);
	if(PosCur.DataPos==PosPre.DataPos){
		if(PosCur.ShowPosX>=0&&PosCur.ShowPosX<=TextBoxWidth&&PosCur.ShowPosY>=0&&PosCur.ShowPosY<TextBoxHeight&&CursorTimer==1)
			MyPainter.drawLine(PosCur.ShowPosX + FirstQCharX,PosCur.ShowPosY * FontSizeH + FirstQCharY,
								PosCur.ShowPosX + FirstQCharX,PosCur.ShowPosY * FontSizeH + FirstQCharY + FontSizeH);
	}else{
		//qDebug("%d %d %d %d\n",PosCur.ShowPosX,PosCur.ShowPosY,PosPre.ShowPosX,PosPre.ShowPosY);
		if(PosCur.ShowPosY>PosPre.ShowPosY){
			FillBlueArea(PosPre,PosCur,&MyPainter);
			ChangeColorFlag = 2;
		}else if(PosCur.ShowPosY<PosPre.ShowPosY){
			FillBlueArea(PosCur,PosPre,&MyPainter);
			ChangeColorFlag = 1;
		}else if(PosCur.ShowPosX>PosPre.ShowPosX){
			MyPainter.fillRect(FirstQCharX+PosPre.ShowPosX,PosPre.ShowPosY*FontSizeH+FirstQCharY,PosCur.ShowPosX-PosPre.ShowPosX,FontSizeH,
								QColor(0,0,255));
			ChangeColorFlag = 2;
		}else if(PosCur.ShowPosX<PosPre.ShowPosX){
			MyPainter.fillRect(FirstQCharX+PosCur.ShowPosX,PosCur.ShowPosY*FontSizeH+FirstQCharY,PosPre.ShowPosX-PosCur.ShowPosX,FontSizeH,
								QColor(0,0,255));
			ChangeColorFlag = 1;
		}
	}
	for(Data::iterator i=PosLeftUp.DataPos;(!i.isOverFlow())&&(DrawY<TextBoxHeight);)
	{
		if(ChangeColorFlag==1){
			if(i==PosCur.DataPos)MyPainter.setPen(Qt::white);
			else if(i==PosPre.DataPos)MyPainter.setPen(Qt::black);
		}else if(ChangeColorFlag==2){
			if(i==PosCur.DataPos)MyPainter.setPen(Qt::black);
			else if(i==PosPre.DataPos)MyPainter.setPen(Qt::white);
		}
		if((*i)=='\n')
		{
			DrawY++;
			DrawX=0;
			i++;
		}else if((*i)=='\t'){
			if(DrawX<TextBoxWidth){
				int a1=DrawX;
				DrawX=(1+DrawX/(FontSizeW*TabWidth))*FontSizeW*TabWidth;
				if(DrawX>=TextBoxWidth){
					DrawX=TextBoxWidth;
				}
				MyPainter.fillRect(FirstQCharX+a1+1,FirstQCharY+DrawY*FontSizeH+1,
								   DrawX-a1-2,FontSizeH-2,QColor(50,50,50,50));
				i++;
			}else{
				DrawX=0;
				DrawY++;
			}
		}else if((*i).unicode()>=0x4e00&&(*i).unicode()<=0x9fa5){
			if(DrawX+FontSizeW*2<TextBoxWidth)
			{
				MyPainter.drawText(QRect(FirstQCharX+DrawX,FirstQCharY+DrawY*FontSizeH,2*FontSizeW,FontSizeH),
								   Qt::AlignCenter,QString("%1").arg(*i));
				DrawX+=FontSizeW*2;
				i++;
			}else{
				DrawY++;
				DrawX=0;
			}
		}else{
			if(DrawX+FontSizeW<TextBoxWidth)
			{
				MyPainter.drawText(QRect(FirstQCharX+DrawX,FirstQCharY+DrawY*FontSizeH,FontSizeW,FontSizeH),
								   Qt::AlignCenter,QString("%1").arg(*i));
				DrawX+=FontSizeW;
				i++;
			}else{
				DrawY++;
				DrawX=0;
			}
		}
	}
	CursorTimer=!CursorTimer;
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
		data.del(PosCur.DataPos, PosPre.DataPos, true);
}
void MainWindow::on_action_Find_triggered()
{
    replaceDlg->show();
}
void MainWindow::on_action_FindNext_triggered()
{
    PosCur.DataPos = data.find(PosCur.DataPos, replaceDlg->findLeText());
}
void MainWindow::on_action_Replace_triggered()
{
    replaceDlg->show();
}
void MainWindow::getMenu_E_state()
{
    //active all action
    ui->action_Undo->setDisabled(false);
    ui->action_Cut->setDisabled(false);
    ui->action_Copy->setDisabled(false);
    ui->action_Paste->setDisabled(false);
    ui->action_Delete->setDisabled(false);
    ui->action_Find->setDisabled(false);
    ui->action_FindNext->setDisabled(false);
    ui->action_Replace->setDisabled(false);
    //judge function state
	if (!PosCur.DataPos || !PosPre.DataPos || PosCur.DataPos == PosPre.DataPos){
        ui->action_Cut->setDisabled(true);
        ui->action_Copy->setDisabled(true);
        ui->action_Delete->setDisabled(true);
    }
    //todo if UndoStack is empty then disable undo action
    //judge replacedlg
    if (!replaceDlg->findLeText().length()){
        ui->action_FindNext->setDisabled(true);
	}
}

void MainWindow::RefreshProtectedUpdateTimer()
{
	ProtectedUpdateTimer=1;
}
void MainWindow::RefreshShowPos()
{
	PosCur.DataPos.clear();
	PosPre.DataPos.clear();

	auto i = PosLeftUp.DataPos;
	bool PosCurFlag=false,PosPreFlag=false;
	int MoveX=0;
	int MoveY=0;
	while((!i.isOverFlow()) && MoveY < TextBoxHeight)
	{
		if((!PosCurFlag)&&i==PosCur.DataPos){
			//qDebug("Yes");
			PosCur.ShowPosX = MoveX;
			PosCur.ShowPosY = MoveY;
			PosCurFlag=true;
		}
		if((!PosPreFlag)&&i==PosPre.DataPos){
			PosPre.ShowPosX = MoveX;
			PosPre.ShowPosY = MoveY;
			PosPreFlag=true;
		}
		if(PosCurFlag&&PosPreFlag)break;

		if((*i)=='\n')
		{
			MoveY++;
			MoveX = 0;
			i++;
		}else if((*i)=='\t'){
			if(MoveX==TextBoxWidth){
				MoveX=0;
				MoveY++;
			}else if((1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth<TextBoxWidth){
				MoveX=(1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth;
				i++;
			}else{
				MoveX=TextBoxWidth;
				i++;
			}
		}else if((*i).unicode() >= 0x4e00&&(*i).unicode() <= 0x9fa5){
			if(MoveX+FontSizeW*2<TextBoxWidth)
			{
				MoveX+=FontSizeW*2;
				i++;
			}else{
				MoveY++;
				MoveX=0;
			}
		}else{
			if(MoveX+FontSizeW<TextBoxWidth)
			{
				MoveX+=FontSizeW;
				i++;
			}else{
				MoveY++;
				MoveX=0;
			}
		}
	}
}

void MainWindow::FillBlueArea(Pos&pos1,Pos&pos2,QPainter*painter)
{
	auto i = pos1.DataPos;
	int MoveX = pos1.ShowPosX;
	int MoveY = pos1.ShowPosY;
	while(!(i == pos2.DataPos) && MoveY < TextBoxHeight){
		if((*i)=='\n')
		{
			painter->fillRect(MoveX+FirstQCharX,MoveY*FontSizeH+FirstQCharY,2,FontSizeH,QColor(0,0,255));
			MoveY++;
			MoveX = 0;
			i++;
		}else if((*i)=='\t'){
			if(MoveX<TextBoxWidth){
				if((1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth>TextBoxWidth){
					painter->fillRect(MoveX+FirstQCharX,MoveY*FontSizeH+FirstQCharY,
									  TextBoxWidth-MoveX,FontSizeH,QColor(0,0,255));
					MoveX=TextBoxWidth;
				}else{
					painter->fillRect(MoveX+FirstQCharX,MoveY*FontSizeH+FirstQCharY,
									  (1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth-MoveX,FontSizeH,QColor(0,0,255));
					MoveX=(1+MoveX/(FontSizeW*TabWidth))*FontSizeW*TabWidth;
				}
				i++;
			}else{
				MoveX=0;
				MoveY++;
			}
		}else if((*i).unicode() >= 0x4e00&&(*i).unicode() <= 0x9fa5){
			if(MoveX+FontSizeW*2<TextBoxWidth)
			{
				painter->fillRect(MoveX+FirstQCharX,MoveY*FontSizeH+FirstQCharY,2*FontSizeW,FontSizeH,QColor(0,0,255));
				MoveX+=FontSizeW*2;
				i++;
			}else{
				MoveY++;
				MoveX=0;
			}
		}else{
			if(MoveX+FontSizeW<TextBoxWidth)
			{
				painter->fillRect(MoveX+FirstQCharX,MoveY*FontSizeH+FirstQCharY,FontSizeW,FontSizeH,QColor(0,0,255));
				MoveX+=FontSizeW;
				i++;
			}else{
				MoveY++;
				MoveX=0;
			}
		}
	}
}

void MainWindow::FindCursor()
{

}

void MainWindow::Rolling(int flag)
{
	if(flag==0){												//向上滑
		if(DataTextTop != 0){
			PosCur.ShowPosY++;
			PosPre.ShowPosY++;
			DataTextTop--;
			//PosLeftUp.DataPos.move(TextBoxWidth,TextBoxWidth);
		}
	}else{
		if(DataTextHeight - DataTextTop > TextBoxHeight/2){		//向下滑
			PosCur.ShowPosY--;
			PosPre.ShowPosY--;
			DataTextTop++;
		}
	}
	ProtectedUpdate();
}

void MainWindow::ProtectedUpdate()
{
	if(ProtectedUpdateTimer == 1){
		ProtectedUpdateTimer = 0;
		QTimer::singleShot(0,this,SLOT(update()));
	}
}

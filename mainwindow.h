#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QString>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QInputMethodEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QScrollBar>
#include "replacedlg.h"
#include "data.h"
#include "settingdlg.h"

namespace Ui {
class MainWindow;
}
struct Pos{
    int ShowPosX;
    int ShowPosY;
    Data::iterator DataPos;
};
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
private slots:
    //=============== about file =================
    void on_action_New_triggered();
    void on_action_Open_triggered();
    void on_action_Save_triggered();
    void on_action_SaveAs_triggered();

    //=============== about window ================
    void on_action_Exit_triggered();

    //=============== about text ==================
    void on_action_Undo_triggered();
    void on_action_Cut_triggered();
    void on_action_Copy_triggered();
    void on_action_Paste_triggered();
    void on_action_Delete_triggered();
    void on_action_Find_triggered();
    void on_action_FindNext_triggered();
    void on_action_Replace_triggered();

    //============== about menu ==============
    void getMenu_E_state();

	//============== about update ============

	void getDataChanged();
	void ProtectedUpdate();					//保护式刷新
	void GetDataHeight();					//获取文本高度并更新与显示相关的信息
	void on_action_Redo_triggered();

	void on_action_Setting_triggered();

private:
    Ui::MainWindow *ui;
    //=================== Object ===============================
    //------------ Right Click Menu-------
    QMenu * rightMenu;
    QAction * undoAction;
		QAction * redoAction;
    QAction * cutAction;
    QAction * copyAction;
    QAction * pasteAction;
    QAction * delAction;
    QAction * seleteAllAction;

		//-----------  dialog ----------
    ReplaceDlg * replaceDlg;
		SettingDlg * settingsDlg;

    //=================== Variable ============================

    //----------- about file -----------
    bool isUntitled;
    bool shouldSave;
    QString curFile;

    //------------ data ---------
		Data data;

    //------------ show ---------
	Pos PosLeftUp;							//最左上角光标
	Pos PosCur;								//当前时刻光标
	Pos PosPre;								//上一时刻光标
	int DataTextHeight;						//文本高度
	int DataTextTop;						//文本顶部
	int TextBoxHeight;						//文本框高度
	int TextBoxWidth;						//文本框顶部
	int FontSizeW;							//字体宽度
	int FontSizeH;							//字体高度
	int TabWidth;							//Tab宽度
	int TabStyle;
	int SpaceStyle;
	bool IsNeededFindCursor;				//判断是否需要定位光标位置
	bool IsDragged;							//判断是否正在拖动鼠标

	int CursorTimer;						//光标闪烁计时器
	QTimer MyCursorTimer;					//光标闪烁定时器

	int ProtectedUpdateTimer;				//保护式刷新计时器
	QTimer MyProtectedUpdateTimer;			//保护式刷新定时器

	QScrollBar *MyScrollBar;				//混动条
	int oldScrollPos;
	void RefreshShowPos();					//刷新光标显示位置
	void LocateCursor(int x,int y);			//光标定位
	void FillBlueArea(Pos &pos1, Pos &pos2, QPainter*painter);
	void Rolling(int flag);					//页面滚动,flag表示向上滚还是向下滚
	void LocateLeftUpCursor(int newDataTextTop,int flag = 0);
											//左上角光标定位,flag表示是否需要绝对定位
	void FindCursor();						//将光标定位于视野中

	void ChangeFontSize(int Size);			//改变字体大小


    //===================== Methods ========================

    //-------- initialize Right Click Menu ---------
    void initRightMenu();
    void resetRightMenu();

    //------------ about file ------------
    void newFile();
    bool maybeSave();
    bool save();
    bool saveAs();
    bool saveFile(const QString & path);

		//----------- about config --------
		void setConfig() const ;
		void getConfig();

protected:
    void closeEvent(QCloseEvent * event);//intercept window close event(to save file)
    void keyPressEvent(QKeyEvent * ev);
    void inputMethodEvent(QInputMethodEvent * ev);
    void mousePressEvent(QMouseEvent * ev);
	void mouseReleaseEvent(QMouseEvent * ev);
	void mouseDoubleClickEvent(QMouseEvent * ev);
	void mouseMoveEvent(QMouseEvent * ev);
	void wheelEvent(QWheelEvent *ev);
    void paintEvent(QPaintEvent * ev);
public:
	bool openFile(const QString & path);
};

#endif // MAINWINDOW_H

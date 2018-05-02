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
#include "replacedlg.h"
#include "data.h"

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
	void RefreshProtectedUpdateTimer();
	void ProtectedUpdate();					//保护式刷新
private:
    Ui::MainWindow *ui;
    //=================== Object ===============================
    //------------ Right Click Menu-------
    QMenu * rightMenu;
    QAction * undoAction;
    QAction * cutAction;
    QAction * copyAction;
    QAction * pasteAction;
    QAction * delAction;
    QAction * seleteAllAction;

    //----------- replace and find dialog ----------
    ReplaceDlg * replaceDlg;

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

	int CursorTimer;						//光标闪烁计时器
	QTimer MyCursorTimer;					//光标闪烁定时器

	int ProtectedUpdateTimer;				//保护式刷新计时器
	QTimer MyProtectedUpdateTimer;			//保护式刷新定时器
	void RefreshShowPos();					//刷新光标显示位置
	void FillBlueArea(Pos &pos1, Pos &pos2, QPainter*painter);
	void FindCursor();						//将光标定位于视野中
	void Rolling(int flag);					//页面滚动,flag表示向上滚还是向下滚

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
    bool openFile(const QString & path);

protected:
    void closeEvent(QCloseEvent * event);//intercept window close event(to save file)
    void keyPressEvent(QKeyEvent * ev);
    void inputMethodEvent(QInputMethodEvent * ev);
    void mousePressEvent(QMouseEvent * ev);
	void mouseDoubleClickEvent(QMouseEvent * ev);
	void mouseMoveEvent(QMouseEvent * ev);
	void wheelEvent(QWheelEvent *ev);
    void paintEvent(QPaintEvent * ev);
};

#endif // MAINWINDOW_H

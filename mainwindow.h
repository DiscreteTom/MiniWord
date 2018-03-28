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
private:
    enum ShowType{
        UpdateALLScreen=1,
        UpdateCursor,
        UpdateBuleArea,
        UpdateBehind
    };
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
    Pos PosLeftUp;
    Pos PosCur;
    Pos PosPre;
    ShowType MyShowType;

    int DataTextHeight;
    int DataTextTop;
    int TextBoxHeight;
    int TextBoxWidth;
    int FontSizeW;
    int FontSizeH;

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
	void paintEvent(QPaintEvent * ev);
};

#endif // MAINWINDOW_H

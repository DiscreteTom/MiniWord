#ifndef CHARNUMDLG_H
#define CHARNUMDLG_H
#include<QDialog>
class CharNumDlg:public QDialog
{
public:
	explicit CharNumDlg(QWidget*parent = NULL);
	void SetCharNum(int allnum, int Chinesenum, int Englishnum, int paranum, int numbernum, int othernum);
protected:
	void paintEvent(QPaintEvent*ev);
private:
	int AllNum;
	int ChineseCharNum;
	int EnglishCharNum;
	int ParaNum;
	int NumberNum;
	int OtherNum;
};

#endif // CHARNUMDLG_H

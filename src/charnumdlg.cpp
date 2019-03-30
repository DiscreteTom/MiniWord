#include<QPainter>
#include<QKeyEvent>
#include "charnumdlg.h"
CharNumDlg::CharNumDlg(QWidget *parent):
	QDialog(parent)
{
	setFixedSize(200,210);
	AllNum = 0;
	ChineseCharNum = 0;
	EnglishCharNum = 0;
	NumberNum = 0;
	OtherNum = 0;
}

void CharNumDlg::SetCharNum(int allnum, int Chinesenum, int Englishnum, int paranum, int numbernum, int othernum)
{
	AllNum = allnum;
	ChineseCharNum = Chinesenum;
	EnglishCharNum = Englishnum;
	ParaNum = paranum;
	NumberNum = numbernum;
	OtherNum = othernum;
}

void CharNumDlg::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);
	painter.setFont(QFont("楷体",12));
	painter.drawText(30,30,QString("全部字符数：%1").arg(AllNum));
	painter.drawText(30,60,QString("中文字符数：%1").arg(ChineseCharNum));
	painter.drawText(30,90,QString("英文字符数：%1").arg(EnglishCharNum));
	painter.drawText(30,120,QString("文本段落数：%1").arg(ParaNum));
	painter.drawText(30,150,QString("数字字符数：%1").arg(NumberNum));
	painter.drawText(30,180,QString("其他字符数：%1").arg(OtherNum));
}

void CharNumDlg::keyPressEvent(QKeyEvent *ev)
{
	if(ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return)
	{
		hide();
	}
}

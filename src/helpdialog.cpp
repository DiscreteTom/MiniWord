#include<QKeyEvent>
#include "helpdialog.h"
#include "ui_helpdialog.h"

HelpDialog::HelpDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::HelpDialog)
{
	ui->setupUi(this);
	QPalette a = ui->textBrowser->palette();
	a.setBrush(QPalette::Base,QBrush(Qt::NoBrush));
	ui->textBrowser->setPalette(a);
	//ui->textBrowser->setFont(QFont("楷体",15));
	setFixedSize(width(),height());
}

HelpDialog::~HelpDialog()
{
	delete ui;
}

void HelpDialog::keyPressEvent(QKeyEvent *ev)
{
	if(ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return)
	{
		hide();
	}
}

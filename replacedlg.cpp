#include "replacedlg.h"
#include "ui_replacedlg.h"

ReplaceDlg::ReplaceDlg(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ReplaceDlg)
{
	ui->setupUi(this);

	setFixedSize(this->width(), this->height());//ban changing size!!!
}

ReplaceDlg::~ReplaceDlg()
{
	delete ui;
}

void ReplaceDlg::on_cancelBtn_clicked()
{
	this->hide();
}

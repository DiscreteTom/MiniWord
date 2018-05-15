#include "settingdlg.h"
#include "ui_settingdlg.h"

SettingDlg::SettingDlg(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SettingDlg)
{
	ui->setupUi(this);
}

SettingDlg::~SettingDlg()
{
	delete ui;
}

void SettingDlg::setMaxUndoTime(int n)
{
	ui->maxUndoTimeSb->setValue(n);
}

void SettingDlg::setDefaultFontSize(int n)
{
	ui->defaultFontSizeSb->setValue(n);
}

int SettingDlg::maxUndoTime() const
{
	return ui->maxUndoTimeSb->value();
}

int SettingDlg::defaultFontSize() const
{
	return ui->defaultFontSizeSb->value();
}



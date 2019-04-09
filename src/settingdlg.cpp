#include "settingdlg.h"
#include "ui_settingdlg.h"

SettingDlg::SettingDlg(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SettingDlg)
{
	ui->setupUi(this);

	setWindowTitle(tr("设置"));
	ui->codeStyleCB->setCurrentIndex(0);
}

SettingDlg::~SettingDlg()
{
	delete ui;
}

void SettingDlg::setMaxUndoTime(int n)
{
	if (n <= 0) n = 20;
	ui->maxUndoTimeSb->setValue(n);
}

void SettingDlg::setTabStyle(int n)
{
	if (n == 2){
		ui->tabStyle2Rdo->setChecked(true);
	} else if (n == 3){
		ui->tabStyle3Rdo->setChecked(true);
	} else {
		ui->tabStyle1Rdo->setChecked(true);
	}
}

void SettingDlg::setSpaceStyle(int n)
{
	if (n == 2){
		ui->spaceStyle2Rdo->setChecked(true);
	} else {
		ui->spaceStyle1Rdo->setChecked(true);
	}
}

void SettingDlg::setCodeStyle(int n)
{
	if(n >= 0 && n < 3)
	{
		ui->codeStyleCB->setCurrentIndex(n);
	}else
	{
		ui->codeStyleCB->setCurrentIndex(0);
	}
}

void SettingDlg::setFontSize(int n)
{
	if (n < 1 || n > 9) n = 1;
	ui->defaultFontSizeSb->setValue(n);
}

void SettingDlg::setFontK(double n)
{
	if(n<1 || n>2)n=1.625;
	ui->paintFontK->setValue(n);
}

void SettingDlg::setTabSize(int n)
{
	if (n < 2 || n > 16) n = 4;
	ui->tabSizeSb->setValue(n);
}

int SettingDlg::maxUndoTime() const
{
	return ui->maxUndoTimeSb->value();
}

int SettingDlg::defaultFontSize() const
{
	return ui->defaultFontSizeSb->value();
}
double SettingDlg::fontK() const
{
	return ui->paintFontK->value();
}
int SettingDlg::tabSize() const
{
	return ui->tabSizeSb->value();
}

int SettingDlg::tabStyle() const
{
	if (ui->tabStyle2Rdo->isChecked()){
		return 2;
	} else if (ui->tabStyle3Rdo->isChecked()){
		return 3;
	} else {
		return 1;
	}
}

int SettingDlg::spaceStyle() const
{
	if (ui->spaceStyle2Rdo->isChecked()){
		return 2;
	} else {
		return 1;
	}
}
int SettingDlg::codeStyle() const
{
	return ui->codeStyleCB->currentIndex();
}

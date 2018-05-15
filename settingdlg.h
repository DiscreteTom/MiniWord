#ifndef SETTINGDLG_H
#define SETTINGDLG_H

#include <QDialog>

namespace Ui {
class SettingDlg;
}

class SettingDlg : public QDialog
{
	Q_OBJECT

public:
	explicit SettingDlg(QWidget *parent = 0);
	~SettingDlg();

	void setMaxUndoTime(int n);
	void setDefaultFontSize(int n);
	int maxUndoTime() const ;
	int defaultFontSize() const ;
private:
	Ui::SettingDlg *ui;
};

#endif // SETTINGDLG_H

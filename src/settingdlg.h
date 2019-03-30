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
	void setFontSize(int n);
	void setTabSize(int n);
	void setTabStyle(int n);
	void setSpaceStyle(int n);
	int maxUndoTime() const ;
	int defaultFontSize() const ;
	int tabSize() const ;
	int tabStyle() const ;
	int spaceStyle() const ;
private:
	Ui::SettingDlg *ui;
};

#endif // SETTINGDLG_H

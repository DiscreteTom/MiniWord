
#ifndef REPLACEDLG_H

#define REPLACEDLG_H



#include <QDialog>

#include <QString>



namespace Ui {

class ReplaceDlg;

}



class ReplaceDlg : public QDialog

{

	Q_OBJECT



public:

	explicit ReplaceDlg(QWidget *parent = 0);

	~ReplaceDlg();



	QString findLeText() const ;

	QString replaceLeText() const ;



private slots:

	void on_cancelBtn_clicked();



private:

	Ui::ReplaceDlg *ui;

};



#endif // REPLACEDLG_H

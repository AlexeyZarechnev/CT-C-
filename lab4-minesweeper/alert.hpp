#ifndef ALERT_HPP
#define ALERT_HPP

#include <QDialog>

class QPushButton;

class Alert : public QDialog
{
	Q_OBJECT
	QPushButton *_ok_button;

  public:
	explicit Alert(QString text, QWidget *parent = nullptr);

  signals:
};

#endif	  // ALERT_HPP

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <QDialog>

class QLineEdit;
class QPushButton;
class Board;
class QMessageBox;
class QValidator;
class Alert;

class Settings : public QDialog
{
	Q_OBJECT
	QLineEdit *_width, *_height, *_mines;
	QPushButton *_start;
	Alert *_invalid_sizes, *_invalid_mines_count;
	Board *_board;
	QValidator *_validator;

  public:
	explicit Settings(Board *board, QWidget *parent = nullptr);
	~Settings();
  public slots:
	void checkInput();
};
#endif	  // MAINWINDOW_HPP

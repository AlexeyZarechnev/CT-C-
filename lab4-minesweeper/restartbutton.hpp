#ifndef RESTARTBUTTON_HPP
#define RESTARTBUTTON_HPP

#include <QtWidgets>

class Board;

class RestartButton : public QPushButton
{
	Q_OBJECT
	Board *_board;

  public:
	explicit RestartButton(Board *board, QWidget *parent = nullptr);

  signals:
};

#endif	  // RESTARTBUTTON_HPP

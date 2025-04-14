#ifndef CHEATBUTTON_HPP
#define CHEATBUTTON_HPP

#include <QtWidgets>

class Board;

class CheatButton : public QPushButton
{
	Q_OBJECT

  public:
	explicit CheatButton(Board *board, QWidget *parent = nullptr);

  signals:
};

#endif	  // CHEATBUTTON_HPP

#ifndef INVERSEBUTTON_HPP
#define INVERSEBUTTON_HPP

#include <QtWidgets>

class Board;

class InverseButton : public QPushButton
{
	Q_OBJECT

  public:
	explicit InverseButton(Board *board, QWidget *parent = nullptr);

  signals:
};

#endif	  // INVERSEBUTTON_HPP

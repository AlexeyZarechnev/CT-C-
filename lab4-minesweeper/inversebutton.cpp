#include "inversebutton.hpp"

#include "board.hpp"

InverseButton::InverseButton(Board *board, QWidget *parent) : QPushButton{ parent }
{
	connect(this, &QPushButton::clicked, board, &Board::invert);
	setText("Режим Левши");
	setFixedSize(100, 50);
}

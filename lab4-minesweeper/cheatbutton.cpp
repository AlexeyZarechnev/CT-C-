#include "cheatbutton.hpp"

#include "board.hpp"

CheatButton::CheatButton(Board *board, QWidget *parent) : QPushButton{ parent }
{
	connect(this, &QPushButton::clicked, board, &Board::cheat);
	setText("Подсмотреть");
	setFixedSize(200, 50);
}

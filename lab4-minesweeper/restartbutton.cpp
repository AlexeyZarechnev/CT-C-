#include "restartbutton.hpp"

#include "board.hpp"

RestartButton::RestartButton(Board *board, QWidget *parent) : QPushButton{ parent }, _board(board)
{
	setMinimumSize(200, 50);
	setMaximumSize(200, 50);
	setText("Рестарт");
	setStyleSheet("text-align: top center");

	QMenu *menu = new QMenu;
	QAction *reset = new QAction("с текущими параметрами");
	connect(reset, &QAction::triggered, board, &Board::reset);
	menu->addAction(reset);
	QAction *chahge_parameters = new QAction("с новыми параметрами");
	connect(chahge_parameters, &QAction::triggered, board, &Board::relaunch);
	menu->addAction(chahge_parameters);

	setMenu(menu);
}

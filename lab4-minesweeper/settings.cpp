#include "settings.hpp"

#include "alert.hpp"
#include "board.hpp"

#include <QMessageBox>
#include <QValidator>
#include <QtWidgets>

Settings::Settings(Board *board, QWidget *parent) :
	QDialog(parent), _invalid_sizes(new Alert("Введите корректные размеры от 1 до 30", this)),
	_invalid_mines_count(new Alert("Введите количество мин не превыщающее количество клеток на поле", this)),
	_board(board), _validator(new QIntValidator(1, 30, this))

{
	setMinimumSize(200, 100);
	QVBoxLayout *mainLayout = new QVBoxLayout;

	QHBoxLayout *widthLayout = new QHBoxLayout;
	widthLayout->addWidget(new QLabel("Ширина поля:"));
	widthLayout->addWidget(_width = new QLineEdit("10"));
	_width->setValidator(_validator);
	mainLayout->addLayout(widthLayout);

	QHBoxLayout *heightLayout = new QHBoxLayout;
	heightLayout->addWidget(new QLabel("Высота поля:"));
	heightLayout->addWidget(_height = new QLineEdit("10"));
	_height->setValidator(_validator);
	mainLayout->addLayout(heightLayout);

	QHBoxLayout *minesLayout = new QHBoxLayout;
	minesLayout->addWidget(new QLabel("Количество мин:"));
	minesLayout->addWidget(_mines = new QLineEdit("10"));
	mainLayout->addLayout(minesLayout);

	mainLayout->addWidget(_start = new QPushButton("Старт"));

	setLayout(mainLayout);
	setWindowTitle("Настройки игры");

	connect(_start, &QPushButton::clicked, this, &Settings::checkInput);
}

Settings::~Settings() {}

void Settings::checkInput()
{
	if (!_width->hasAcceptableInput() || !_height->hasAcceptableInput())
	{
		_invalid_sizes->exec();
		return;
	}
	uint n_width = _width->text().toUInt();
	uint n_height = _height->text().toUInt();
	QIntValidator mines_validator(1, n_width * n_height - 1);
	_mines->setValidator(&mines_validator);
	if (!_mines->hasAcceptableInput())
	{
		_invalid_mines_count->exec();
		return;
	}
	_board->init(n_width, n_height, _mines->text().toUInt(), false);

	accept();
}

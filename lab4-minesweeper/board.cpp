#include "board.hpp"

#include "alert.hpp"
#include "cell.hpp"
#include "cheatbutton.hpp"
#include "inversebutton.hpp"
#include "restartbutton.hpp"
#include "settings.hpp"

#include <QDir>
#include <QRandomGenerator>
#include <QStringRef>
#include <QtWidgets>

Board::Board(QWidget *parent) :
	QWidget{ parent }, _openned(0), _settings(this, this), _dbg(false), _inverse(false), _game_ended(false)
{
	setWindowTitle("Сапёр");
}

bool Board::inversed()
{
	return _inverse;
}

void Board::launchWithSave()
{
	QFile save_file(QDir::current().absoluteFilePath("save.ini"));
	save_file.open(QIODevice::ReadOnly);

	if (!save_file.isOpen())
	{
fail:
		save_file.remove();
		launchWithoutSave();
		return;
	}

	QTextStream save_input(&save_file);

	if (save_input.readLine() != "[Parameters]")
		goto fail;

	QString s_width(save_input.readLine()), s_height(save_input.readLine()), s_mines(save_input.readLine()),
		s_openned(save_input.readLine()), s_flags(save_input.readLine()), s_inverse(save_input.readLine());

	_openned = QStringRef(&s_openned).mid(s_openned.indexOf('=') + 1).toUInt();
	_flags_count = QStringRef(&s_flags).mid(s_flags.indexOf('=') + 1).toUInt();
	_inverse = QStringRef(&s_inverse).mid(s_inverse.indexOf('=') + 1).toUInt();

	init(QStringRef(&s_width).mid(s_width.indexOf('=') + 1).toUInt(),
		 QStringRef(&s_height).mid(s_height.indexOf('=') + 1).toUInt(),
		 QStringRef(&s_mines).mid(s_mines.indexOf('=') + 1).toUInt(),
		 true);

	if (save_input.readLine() != "[Cells]")
		goto fail;

	for (Cell *cell : _board)
		cell->parse(save_input);

	save_file.remove();
	return;
}

void Board::deleteLayout(QLayout *layout)
{
	if (layout)
	{
		QLayoutItem *item;
		while ((item = layout->takeAt(0)) != 0)
		{
			if (item->widget() != 0)
				delete item->widget();
			if (item->layout() != 0)
				deleteLayout(item->layout());

			layout->removeItem(item);
		}
		delete layout;
	}
}

void Board::setDbg(bool enable)
{
	_dbg = enable;
}

void Board::relaunch()
{
	deleteLayout(layout());
	launchWithoutSave();
}

void Board::launchWithoutSave()
{
	_settings.exec();
}

void Board::init(uint width, uint height, uint mines, bool from_save)
{
	_width = width;
	_height = height;
	_mines = mines;
	_board.clear();

	for (uint i = 0; i < _width * _height; ++i)
	{
		_board.push_back(new Cell(i, this, this));
		_board[i]->setSize(20 + (30 - height));
	}

	QGridLayout *cells = new QGridLayout();
	for (uint i = 0; i < _height; ++i)
	{
		for (uint j = 0; j < _width; ++j)
		{
			uint id = i * _width + j;
			cells->addWidget(_board[id], i, j);
		}
	}
	cells->setSpacing(0);

	QHBoxLayout *cells_container = new QHBoxLayout;
	cells_container->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
	cells_container->addLayout(cells);
	cells_container->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));

	QFont f;
	f.setItalic(true);
	f.setPointSize(24);

	QHBoxLayout *information = new QHBoxLayout;
	information->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
	information->addWidget(_flags = new QLabel(QString::number(_flags_count)));
	_flags->setFont(f);
	information->addWidget(new RestartButton(this, this));
	information->addWidget(new InverseButton(this, this));
	information->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));

	QVBoxLayout *main = new QVBoxLayout;
	main->addLayout(information);
	main->addLayout(cells_container);
	if (_dbg)
	{
		QHBoxLayout *cheat = new QHBoxLayout;
		cheat->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
		cheat->addWidget(new CheatButton(this, this));
		cheat->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
		main->addLayout(cheat);
	}
	main->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
	setLayout(main);

	if (!from_save)
		reset();
}

void Board::invert()
{
	_inverse ^= 1;
}

Board::~Board()
{
	if (_openned && !_game_ended)
		save();

	deleteLayout(layout());
}

void Board::save()
{
	QFile save_file(QDir::current().absoluteFilePath("save.ini"));
	if (!save_file.open(QIODevice::ReadWrite))
	{
		save_file.remove();
		return;
	}

	QTextStream save_stream(&save_file);
	save_stream << "[Parameters]\n";
	save_stream << "l_width=";
	save_stream << QString::number(_width);
	save_stream << "\n";
	save_stream << "l_height=";
	save_stream << QString::number(_height);
	save_stream << "\n";
	save_stream << "mines=";
	save_stream << QString::number(_mines);
	save_stream << "\n";
	save_stream << "openned=";
	save_stream << QString::number(_openned);
	save_stream << "\n";
	save_stream << "flags_count=";
	save_stream << QString::number(_flags_count);
	save_stream << "\n";
	save_stream << "inverse=";
	save_stream << QString::number(_inverse);
	save_stream << "\n";
	save_stream << "[Cells]\n";

	for (Cell *cell : _board)
		cell->print(save_stream);
}

void Board::reset()
{
	for (Cell *cell : _board)
		cell->reset();

	_game_ended = false;
	_flags_count = _mines;
	_cheated = false;
	_openned = 0;
	_flags->setText(QString::number(_flags_count));
}

void Board::startGame(uint started)
{
	uint placed_mines = 0;
	QRandomGenerator rnd(time(nullptr));
	while (placed_mines < _mines)
	{
		uint cell = rnd.generate() % _board.size();
		if (cell != started && _board[cell]->placeMine())
			++placed_mines;
	}
	for (Cell *cell : _board)
		cell->countValue();
}

void Board::cheat()
{
	for (Cell *cell : _board)
	{
		if (cell->hasMine())
		{
			if (_cheated)
				cell->cover();
			else
				cell->open();
		}
	}
	_cheated ^= true;
}

void Board::endGame(bool win)
{
	_game_ended = true;
	for (Cell *cell : _board)
	{
		cell->setDisabled(true);
		if (!win && cell->hasMine())
			cell->open();
	}
	Alert alert(win ? "Победа" : "Вы проиграли, как жаль :(", this);
	alert.exec();
}

bool Board::flagDiff(int diff)
{
	if (diff < 0 && _flags_count == 0)
		return false;

	_flags_count += diff;
	_flags->setText(QString::number(_flags_count));
	return true;
}

void Board::checkWin()
{
	if (_openned == _width * _height - _mines && _flags_count == 0)
		endGame(true);
}

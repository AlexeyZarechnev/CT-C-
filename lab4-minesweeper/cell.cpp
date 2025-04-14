#include "cell.hpp"

#include "board.hpp"

#include <QStringRef>
#include <QtWidgets>

const QString closed_stylesheet("border: 1px solid black;\n"
								"background-color: #A9A9A9;\n");
const QString openned_stylesheet("border: 1px solid black;\n"
								 "background-color: #696969;\n");
const QString last_openned_stylesheet("border: 1px solid black;\n"
									  "background-color: red;\n");
const QString highlighted_stylesheet("border: 2px solid blue;\n"
									 "background-color: #A9A9A9;\n");
const QFont fnt("Helvetica", 22, true, true);

Cell::Cell(uint number, Board *board, QWidget *parent) : QPushButton{ parent }, _number(number), _board(board)
{
	setSize(40);
	setFont(fnt);
	setStyleSheet(closed_stylesheet);
	connect(this, &Cell::leftClick, this, &Cell::recUpdate);
	connect(this, &Cell::rightClick, this, &Cell::changeState);
	connect(this, &Cell::middleClick, this, &Cell::fastUpdate);
	show();
}

void Cell::setSize(uint size)
{
	setMinimumSize(size, size);
	setMaximumSize(size, size);
}

bool Cell::placeMine()
{
	if (_value == -1)
		return false;

	_value = -1;
	return true;
}

void Cell::reset()
{
	_value = 0;
	_pressed = false;
	_state = 0;
	setEnabled(true);
	setIcon(QIcon());
	setStyleSheet(closed_stylesheet);
	setText("");
}

void Cell::print(QTextStream &output)
{
	QString name("cell");
	name.append(QString::number(_number)).append("[]").append('=');
	output << name;
	output << QString::number(_value);
	output << '\n';
	output << name;
	output << QString::number(_pressed);
	output << '\n';
	output << name;
	output << QString::number(_state);
	output << '\n';
}

void Cell::parse(QTextStream &input)
{
	QString s_value(input.readLine()), s_openned(input.readLine()), s_flag(input.readLine());

	_value = QStringRef(&s_value).mid(s_value.indexOf('=') + 1).toInt();

	if ((_pressed = QStringRef(&s_openned).mid(s_openned.indexOf('=') + 1).toUInt()))
	{
		open();
	}

	switch ((_state = QStringRef(&s_flag).mid(s_flag.indexOf('=') + 1).toUInt()))
	{
	case 0:
		break;
	case 1:
		setIcon(QIcon(":/icons/flag.png"));
		break;
	case 2:
		setIcon(QIcon(":/icons/question.png"));
		break;
	}
}

void Cell::countValue()
{
	if (_value == -1)
		return;

	uint up = _number - _board->_width;
	uint down = _number + _board->_width;

	auto check = [this](uint number)
	{
		if (number < _board->_board.size() && _board->_board[number]->_value == -1)
			++_value;
	};

	if (_number % _board->_width < _board->_width - 1)
	{
		check(_number + 1);
		check(up + 1);
		check(down + 1);
	}
	if (_number % _board->_width > 0)
	{
		check(_number - 1);
		check(up - 1);
		check(down - 1);
	}
	check(up);
	check(down);
}

void Cell::open()
{
	if (_value == 0)
	{
		setStyleSheet(openned_stylesheet);
		return;
	}

	if (_value == -1)
	{
		setIcon(QIcon(":/icons/mine.png"));
		return;
	}
	setText(QString::number(_value));
	switch (_value)
	{
	case 1:
		setStyleSheet(openned_stylesheet + "color: blue");
		break;
	case 2:
		setStyleSheet(openned_stylesheet + "color: green");
		break;
	case 3:
		setStyleSheet(openned_stylesheet + "color: red");
		break;
	case 4:
		setStyleSheet(openned_stylesheet + "color: purple");
		break;
	default:
		setStyleSheet(openned_stylesheet + "color: yellow");
		break;
	}
}

void Cell::cover()
{
	switch (_state)
	{
	case 0:
		setIcon(QIcon());
		break;
	case 1:
		setIcon(QIcon(":/icons/flag.png"));
		break;
	case 2:
		setIcon(QIcon(":/icons/question.png"));
		break;
	}
	setText("");
}

void Cell::update()
{
	if (!_board->_openned)
		_board->startGame(_number);

	++_board->_openned;
	_pressed = true;
	open();

	if (_value == -1)
	{
		setStyleSheet(last_openned_stylesheet);
		_board->endGame();
		return;
	}

	_board->checkWin();
}

void Cell::recUpdate()
{
	update();
	if (_value)
		return;

	uint up = _number - _board->_width;
	uint down = _number + _board->_width;

	auto check = [this](uint num)
	{
		if (num < _board->_board.size() && !_board->_board[num]->_pressed && !_board->_board[num]->_state)
			_board->_board[num]->recUpdate();
	};

	if (_number % _board->_width != _board->_width - 1)
	{
		check(_number + 1);
		check(up + 1);
		check(down + 1);
	}
	if (_number % _board->_width != 0)
	{
		check(_number - 1);
		check(up - 1);
		check(down - 1);
	}
	check(up);
	check(down);
}

void Cell::fastUpdate()
{
	if (!_pressed)
		return;

	uint up = _number - _board->_width;
	uint down = _number + _board->_width;
	int flags = 0;

	auto check = [this, &flags](uint num)
	{
		if (num < _board->_board.size())
			switch (_board->_board[num]->_state)
			{
			case 1:
				++flags;
				break;
			case 2:
				flags = INT32_MIN;
				break;
			}
	};

	if (_number % _board->_width != _board->_width - 1)
	{
		check(_number + 1);
		check(up + 1);
		check(down + 1);
	}
	if (_number % _board->_width != 0)
	{
		check(_number - 1);
		check(up - 1);
		check(down - 1);
	}
	check(up);
	check(down);

	auto op = [this, flags](uint num)
	{
		if (num < _board->_board.size() && !_board->_board[num]->_pressed && !_board->_board[num]->_state)
		{
			if (flags == _value)
			{
				_board->_board[num]->update();
			}
			else
			{
				_board->_board[num]->highlight();
			}
		}
	};

	if (_number % _board->_width != _board->_width - 1)
	{
		op(_number + 1);
		op(up + 1);
		op(down + 1);
	}
	if (_number % _board->_width != 0)
	{
		op(_number - 1);
		op(up - 1);
		op(down - 1);
	}
	op(up);
	op(down);
}

void Cell::highlight()
{
	setStyleSheet(highlighted_stylesheet);
}

void Cell::changeState()
{
	switch (_state)
	{
	case 0:
		if (_board->flagDiff(-1))
		{
			setIcon(QIcon(":/icons/flag.png"));
			_board->checkWin();
		}
		else
			goto skipInc;
		break;
	case 1:
		_board->flagDiff(1);
		setIcon(QIcon(":/icons/question.png"));
		break;
	case 2:
		setIcon(QIcon());
	}
	_state = (_state + 1) % 3;
skipInc:
	return;
}

bool Cell::hasMine()
{
	return _value == -1;
}

void Cell::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::RightButton && !_pressed)
	{
		if (_board->inversed() && !_state)
			emit leftClick();
		else if (!_board->inversed())
			emit rightClick();
	}
	if (e->button() == Qt::LeftButton && !_pressed)
	{
		if (_board->inversed())
			emit rightClick();
		else if (!_state)
			emit leftClick();
	}

	if (e->button() == Qt::MiddleButton && _pressed)
		emit middleClick();
	if (!_pressed)
		QPushButton::mousePressEvent(e);
}

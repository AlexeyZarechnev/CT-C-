#ifndef CELL_HPP
#define CELL_HPP

#include <QtWidgets>

class Board;

class Cell : public QPushButton
{
	Q_OBJECT
	uint _number;
	Board *_board;
	int _value;
	uint _state;
	bool _pressed;

	void highlight();

  public:
	bool placeMine();
	bool hasMine();

	void setSize(uint size);
	void countValue();
	void reset();
	void open();
	void cover();
	void print(QTextStream &output);
	void parse(QTextStream &input);

	Cell(uint number, Board *board, QWidget *parent = nullptr);

  private slots:
	void mousePressEvent(QMouseEvent *e);

  public slots:
	void update();
	void recUpdate();
	void changeState();
	void fastUpdate();

  signals:
	void leftClick();
	void middleClick();
	void rightClick();
};

#endif	  // CELL_HPP

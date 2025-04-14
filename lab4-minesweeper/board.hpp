#ifndef BOARD_HPP
#define BOARD_HPP

#include "settings.hpp"

#include <QWidget>

class QLabel;
class Cell;
class RestartButton;

class Board : public QWidget
{
	friend class Cell;
	Q_OBJECT
	uint _width;
	uint _height;
	uint _mines;
	uint _openned, _flags_count;

	Settings _settings;
	QLabel *_flags;
	RestartButton *_restart;
	QVector< Cell * > _board;

	void deleteLayout(QLayout *layout);
	void save();

	bool _dbg;
	bool _inverse;
	bool _cheated;
	bool _game_ended;

  public:
	explicit Board(QWidget *parent = nullptr);
	bool inversed();
	void launchWithSave();
	void launchWithoutSave();

	void relaunch();
	void init(uint width, uint height, uint mines, bool from_save);
	bool flagDiff(int diff);
	void checkWin();
	void setDbg(bool enable);
	void startGame(uint started);
	void endGame(bool win = false);
	void cheat();
	~Board();

  public slots:
	void reset();
	void invert();

  signals:
};

#endif	  // BOARD_HPP

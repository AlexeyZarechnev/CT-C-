#include "board.hpp"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

class Minesweeper : public QApplication
{
  public:
	Minesweeper(int &argc, char **argv) : QApplication(argc, argv), board()
	{
		for (int i = 1; i < argc; ++i)
			if (!strcmp(argv[i], "dbg"))
				board.setDbg(true);

		board.launchWithSave();
		board.show();
	}

  private:
	Board board;
};

int main(int argc, char *argv[])
{
	Minesweeper app(argc, argv);
	return app.exec();
}

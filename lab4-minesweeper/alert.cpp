#include "alert.hpp"

#include <QtWidgets>

Alert::Alert(QString text, QWidget *parent) : QDialog{ parent }
{
	QVBoxLayout *layout = new QVBoxLayout;
	setWindowTitle("Внимание!");
	setMinimumSize(200, 100);
	layout->addWidget(new QLabel(text));
	layout->addWidget(_ok_button = new QPushButton("Ок"));
	connect(_ok_button, &QPushButton::clicked, this, &Alert::accept);
	setLayout(layout);
}

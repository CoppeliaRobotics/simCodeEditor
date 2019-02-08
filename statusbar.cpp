#include "statusbar.h"
#include "dialog.h"

StatusBar::StatusBar(Dialog *parent)
    : QStatusBar(parent),
      parent(parent)
{
    addPermanentWidget(lblCursorPos = new QLabel("1:1"));
    lblCursorPos->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    lblCursorPos->setFixedWidth(120);
}

StatusBar::~StatusBar()
{
}

void StatusBar::setCursorInfo(int line, int index)
{
    lblCursorPos->setText(QString("%1:%2").arg(line + 1).arg(index + 1));
}

void StatusBar::setSelectionInfo(int fromLine, int fromIndex, int toLine, int toIndex)
{
    lblCursorPos->setText(QString("%1:%2-%3:%4 S").arg(fromLine + 1).arg(fromIndex + 1).arg(toLine + 1).arg(toIndex + 1));
}

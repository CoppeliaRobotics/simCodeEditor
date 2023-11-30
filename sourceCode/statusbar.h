#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QtWidgets>

class Dialog;

class StatusBar : public QStatusBar
{
    Q_OBJECT

public:
    StatusBar(Dialog *parent);
    virtual ~StatusBar();

    void setCursorInfo(int line, int index);
    void setSelectionInfo(int fromLine, int fromIndex, int toLine, int toIndex);

private:
    Dialog *parent;
    QLabel *lblCursorPos;
};

#endif // STATUSBAR_H

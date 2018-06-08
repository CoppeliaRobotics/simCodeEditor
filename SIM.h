#ifndef SIM_H__INCLUDED
#define SIM_H__INCLUDED

#include <QObject>
#include <QString>
#include <QSemaphore>

class UI;

class SIM : public QObject
{
    Q_OBJECT
public:
    SIM(UI *ui);
public slots:
    void notifyEvent(const QString &msg, int handle);
signals:
    void openModal(const QString &initText, const QString &properties, QSemaphore *sem, QString *text, int *positionAndSize);
    void open(const QString &initText, const QString &properties, int *handle);
    void setText(int handle, const QString &text, int insertMode);
    void getText(int handle, QString *text);
    void show(int handle, int showState);
    void close(int handle, int *positionAndSize);
};

#endif // SIM_H__INCLUDED

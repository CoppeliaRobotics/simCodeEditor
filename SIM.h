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
    SIM(UI *theUi);
public slots:
    void notifyEvent(int handle, const QString &eventType, const QString &data);

signals:
    void openModal(const QString &initText, const QString &properties, QString& text, int *positionAndSize);
    void open(const QString &initText, const QString &properties, int *handle);
    void setText(int handle, const QString &text, int insertMode);
    void getText(int handle, QString *text, int* posAndSize);
    void show(int handle, int showState);
    void close(int handle, int *positionAndSize);
private:
    UI* ui;
};

#endif // SIM_H__INCLUDED

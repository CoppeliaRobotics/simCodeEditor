#ifndef UI_H__INCLUDED
#define UI_H__INCLUDED

#include <QObject>
#include <QString>
#include <QSemaphore>
#include <QMap>

class Dialog;

class SIM;

class UI : public QObject
{
    Q_OBJECT
public:
    UI(SIM *sim);
public slots:
private:
    Dialog * createWindow(bool modalSpecial, const QString &initText, const QString &properties);
public:
    void openModal(const QString &initText, const QString &properties, QString& text, int *positionAndSize);
    void open(const QString &initText, const QString &properties, int *handle);
    void setText(int handle, const QString &text, int insertMode);
    void getText(int handle, QString *text, int* posAndSize);
    void show(int handle, int showState);
    void close(int handle, int *positionAndSize);
    void onSimulationRunning(bool running);

signals:
    void notifyEvent(int handle, const QString &eventType, const QString &data);
    void openURL(const QString &url);
    void requestSimulationStatus();

private:
    int nextEditorHandle = 103800;
    QMap<int, Dialog*> editors;
};

#endif // UI_H__INCLUDED

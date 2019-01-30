#ifndef UI_H__INCLUDED
#define UI_H__INCLUDED

#include <QObject>
#include <QString>
#include <QSemaphore>
#include <QMap>

class CScintillaDlg;

class UI : public QObject
{
    Q_OBJECT
public:
public slots:
private:
    CScintillaDlg * createWindow(bool modalSpecial, const QString &initText, const QString &properties);
public:
    void openModal(const QString &initText, const QString &properties, QString& text, int *positionAndSize);
    void open(const QString &initText, const QString &properties, int *handle);
    void setText(int handle, const QString &text, int insertMode);
    void getText(int handle, QString *text, int* posAndSize);
    void show(int handle, int showState);
    void close(int handle, int *positionAndSize);

signals:
    void notifyEvent(int handle, const QString &eventType, const QString &data);

private:
    int nextEditorHandle = 103800;
    QMap<int, CScintillaDlg*> editors;
};

#endif // UI_H__INCLUDED

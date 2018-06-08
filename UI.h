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
    void openModal(const QString &initText, const QString &properties, QSemaphore *sem, QString *text, int *positionAndSize);
    void open(const QString &initText, const QString &properties, int *handle);
    void setText(int handle, const QString &text, int insertMode);
    void getText(int handle, QString *text);
    void show(int handle, int showState);
    void close(int handle, int *positionAndSize);
signals:
    void notifyEvent(const QString &msg, int handle);
private:
    int nextEditorHandle = 103800;
    QMap<int, CScintillaDlg*> editors;
    CScintillaDlg * editorByHandle(int handle);
};

#endif // UI_H__INCLUDED

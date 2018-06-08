#pragma once

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscistyle.h>
#include <QDialog>
#include <QSemaphore>

class UI;

class CScintillaDlg : public QDialog
{
    Q_OBJECT

public:
    CScintillaDlg(UI *ui, QWidget* pParent = NULL);
    virtual ~CScintillaDlg();

    void setHandle(int handle);
    void setModal(QSemaphore *sem, QString *text, int *positionAndSize);
    void setText(const QString &text);
    QString text() const;

private:
    void setAStyle(int style,unsigned int fore,unsigned int back=0,int size=-1,const char *face=0);
    void closeEvent(QCloseEvent *event);
    std::string getCallTip(const char* txt);

private slots:
    void charAdded(int charAdded);
    void modified(int, int, const char *, int, int, int, int, int, int, int);

private:
    UI *ui;
    QsciScintilla* _scintillaObject;
    int handle;
    struct
    {
        QSemaphore *sem;
        QString *text;
        int *positionAndSize;
    }
    modalData;
    bool isModal = false;
};


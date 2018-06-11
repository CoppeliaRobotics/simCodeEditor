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

    inline QsciScintilla * scintilla() {return _scintillaObject;}
    void setHandle(int handle);
    void setModal(QSemaphore *sem, QString *text, int *positionAndSize);
    void setAStyle(int style,QColor fore,QColor back,int size=-1,const char *face=0);
    void setColorTheme(QColor text_col, QColor background_col, QColor selection_col, QColor comment_col, QColor number_col, QColor string_col, QColor character_col, QColor operator_col, QColor identifier_col, QColor preprocessor_col, QColor keyword1_col, QColor keyword2_col, QColor keyword3_col, QColor keyword4_col);

private:
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


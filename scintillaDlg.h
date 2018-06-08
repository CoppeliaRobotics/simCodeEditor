#pragma once

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscistyle.h>
#include <QDialog>

class CScintillaDlg : public QDialog
{
    Q_OBJECT

public:
    CScintillaDlg(QWidget* pParent = NULL);
    virtual ~CScintillaDlg();

    QsciScintilla* _scintillaObject;

    bool closeRequest;

private:
    void closeEvent(QCloseEvent *event);
    void setAStyle(int style,unsigned int fore,unsigned int back=0,int size=-1,const char *face=0);
    std::string getCallTip(const char* txt);


public slots:
    void charAdded(int charAdded);
    void modified(int, int, const char *, int, int, int, int, int, int, int);
};


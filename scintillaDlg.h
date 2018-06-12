#pragma once

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscistyle.h>
#include <QDialog>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QToolBar>
#include <QAction>
#include <QSemaphore>
#include <QStatusBar>
#include <QComboBox>

class UI;
class ToolBar;
class SearchAndReplacePanel;
class StatusBar;

class CScintillaDlg : public QDialog
{
    Q_OBJECT

public:
    CScintillaDlg(UI *ui, QWidget* pParent = nullptr);
    virtual ~CScintillaDlg();

    inline QsciScintilla * scintilla() {return _scintillaObject;}
    inline ToolBar * toolbar() {return toolBar;}
    inline StatusBar * statusbar() {return statusBar;}
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
    void reloadScript();
    void indent();
    void unindent();

private:
    UI *ui;
    ToolBar *toolBar;
    QsciScintilla* _scintillaObject;
    SearchAndReplacePanel *searchAndReplacePanel;
    StatusBar *statusBar;
    int handle;
    struct
    {
        QSemaphore *sem;
        QString *text;
        int *positionAndSize;
    }
    modalData;
    bool isModal = false;
    friend class ToolBar;
    friend class SearchAndReplacePanel;
    friend class StatusBar;
};

class ToolBar : public QToolBar
{
    Q_OBJECT

public:
    ToolBar(CScintillaDlg *parent = nullptr);
    virtual ~ToolBar();

    QAction *actReload;
    QAction *actShowSearchPanel;
    QAction *actUndo;
    QAction *actRedo;
    QAction *actUnindent;
    QAction *actIndent;
private:
    CScintillaDlg *parent;
    QComboBox *funcNavCombo;
};

class SearchAndReplacePanel : public QWidget
{
    Q_OBJECT

public:
    SearchAndReplacePanel(CScintillaDlg *parent = nullptr);
    virtual ~SearchAndReplacePanel();

public slots:
    void show();

private slots:
    void find();
    void replace();

private:
    CScintillaDlg *parent;
    QLabel *lblFind, *lblReplace;
    QLineEdit *editFind, *editReplace;
    QPushButton *btnFind, *btnReplace, *btnClose;
    QCheckBox *chkRegExp, *chkCaseSens;
};

class StatusBar : public QStatusBar
{
    Q_OBJECT

public:
    StatusBar(CScintillaDlg *parent = nullptr);
    virtual ~StatusBar();

private slots:
    void onCursorPositionChanged(int line, int index);

private:
    CScintillaDlg *parent;
    QLabel *lblCursorPos;
};


#pragma once

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscistyle.h>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QToolBar>
#include <QAction>
#include <QSemaphore>
#include <QStatusBar>
#include <QComboBox>
#include <QStyle>

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

    inline QsciScintilla * scintilla() {return scintilla_;}
    inline ToolBar * toolBar() {return toolBar_;}
    inline SearchAndReplacePanel * searchPanel() {return searchPanel_;}
    inline StatusBar * statusBar() {return statusBar_;}
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
    void textChanged();
    void cursorPosChanged(int line, int index);
    void selectionChanged();
    void reloadScript();
    void indent();
    void unindent();

private:
    void updateCursorSelectionDisplay();

    UI *ui;
    ToolBar *toolBar_;
    QsciScintilla *scintilla_;
    SearchAndReplacePanel *searchPanel_;
    StatusBar *statusBar_;
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

class ToolBar : public QToolBar
{
    Q_OBJECT

public:
    ToolBar(CScintillaDlg *parent = nullptr);
    virtual ~ToolBar();

public slots:
    void updateButtons();

public:
    QAction *actReload;
    QAction *actShowSearchPanel;
    QAction *actUndo;
    QAction *actRedo;
    QAction *actUnindent;
    QAction *actIndent;
    QAction *actFuncNav;
private:
    CScintillaDlg *parent;
    struct {
        QWidget *widget;
        QLabel *label;
        QComboBox *combo;
    } funcNav;
};

class SearchAndReplacePanel : public QWidget
{
    Q_OBJECT

public:
    SearchAndReplacePanel(CScintillaDlg *parent = nullptr);
    virtual ~SearchAndReplacePanel();

public slots:
    void show();
    void hide();

private slots:
    void find();
    void replace();

signals:
    void shown();
    void hidden();

private:
    CScintillaDlg *parent;
    QLabel *lblFind, *lblReplace;
    QComboBox *editFind, *editReplace;
    QPushButton *btnFind, *btnReplace, *btnClose;
    QCheckBox *chkRegExp, *chkCaseSens;
};

class StatusBar : public QStatusBar
{
    Q_OBJECT

public:
    StatusBar(CScintillaDlg *parent = nullptr);
    virtual ~StatusBar();

    void setCursorInfo(int line, int index);
    void setSelectionInfo(int fromLine, int fromIndex, int toLine, int toIndex);

private:
    CScintillaDlg *parent;
    QLabel *lblCursorPos;
};


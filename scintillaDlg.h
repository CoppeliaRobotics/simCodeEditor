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
#include <QStackedWidget>
#include "common.h"

class UI;
class ToolBar;
class SearchAndReplacePanel;
class StatusBar;
class CScintillaDlg;

class CScintillaEdit : public QsciScintilla
{
    Q_OBJECT

public:
    CScintillaEdit(CScintillaDlg *dialog);
    bool isActive() const;
    inline const EditorOptions & editorOptions() { return opts; }
    void setEditorOptions(const EditorOptions &opts);
    void contextMenuEvent(QContextMenuEvent *event);

public slots:
    void setText(const char* txt, int insertMode);
    void setAStyle(int style, QColor fore, QColor back, int size=-1, const char *face = nullptr);
    void onCharAdded(int charAdded);
    void onModified(int, int, const char *, int, int, int, int, int, int, int);
    void onTextChanged();
    void onCursorPosChanged(int line, int index);
    void onSelectionChanged();
    void indentSelectedText();
    void unindentSelectedText();
    void openExternalFile(const QString &filePath);
    void saveExternalFile();

public:
    QString externalFile();
    bool needsSaving();
    bool canSave();

private:
    std::string getCallTip(const char* txt);

    CScintillaDlg *dialog;
    EditorOptions opts;
    struct {
        QString path;
        bool edited;
    } externalFile_;
};

class CScintillaDlg : public QDialog
{
    Q_OBJECT

public:
    CScintillaDlg(const EditorOptions &opts, UI *ui, QWidget* pParent = nullptr);
    virtual ~CScintillaDlg();

    void setEditorOptions(const EditorOptions &opts);
    inline const EditorOptions & editorOptions() { return opts; }
    CScintillaEdit * activeEditor();
    CScintillaEdit * openExternalFile(const QString &filePath);
    void closeExternalFile(const QString &filePath);
    void closeExternalFile(CScintillaEdit *editor);
    void switchEditor(CScintillaEdit *editor);
    inline const QMap<QString, CScintillaEdit*> & editors() {return editors_;}
    bool containsUnsavedFiles();

    inline ToolBar * toolBar() {return toolBar_;}
    inline SearchAndReplacePanel * searchPanel() {return searchPanel_;}
    inline StatusBar * statusBar() {return statusBar_;}
    void setHandle(int handle);
    void setText(const QString &text);
    void setText(const char* txt, int insertMode);
    QString text();
    std::string makeModal(int *positionAndSize);

private:
    void closeEvent(QCloseEvent *event);

private slots:
    void reloadScript();
public slots:
    void updateCursorSelectionDisplay();

private:
    UI *ui;
    ToolBar *toolBar_;
    QMap<QString, CScintillaEdit*> editors_;
    CScintillaEdit *activeEditor_;
    QStackedWidget *stacked_;
    SearchAndReplacePanel *searchPanel_;
    StatusBar *statusBar_;
    int handle;
    int scriptTypeOrHandle;
    EditorOptions opts;
    QString modalText;
    int modalPosAndSize[4];

    friend class Toolbar;
};

class ToolBar : public QToolBar
{
    Q_OBJECT

public:
    ToolBar(bool canRestart,CScintillaDlg *parent = nullptr);
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
    struct {
        QAction *actSave;
        QComboBox *combo;
        QAction *actClose;
    } openFiles;
    struct {
        QComboBox *combo;
    } funcNav;
private:
    CScintillaDlg *parent;
};

class SearchAndReplacePanel : public QWidget
{
    Q_OBJECT

public:
    SearchAndReplacePanel(CScintillaDlg *parent = nullptr);
    virtual ~SearchAndReplacePanel();

public slots:
    void setVisibility(bool v);
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

    friend class CScintillaDlg;
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


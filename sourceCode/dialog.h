#ifndef DIALOG_H
#define DIALOG_H

#include <QtWidgets>
#include "common.h"

class UI;
class Editor;
class ToolBar;
class StatusBar;
class SearchAndReplacePanel;

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(const EditorOptions &opts, UI *ui, QWidget* pParent = nullptr);
    virtual ~Dialog();
public:
    void setEditorOptions(const EditorOptions &opts);
    inline const EditorOptions & editorOptions() { return opts; }
    Editor * activeEditor();
    Editor * openExternalFile(const QString &filePath);
    void closeExternalFile(const QString &filePath);
    void closeExternalFile(Editor *editor);
    void switchEditor(Editor *editor);
    inline const QMap<QString, Editor*> & editors() {return editors_;}
    bool containsUnsavedFiles();

    void show();
    void hide();

    inline ToolBar * toolBar() {return toolBar_;}
    inline SearchAndReplacePanel * searchPanel() {return searchPanel_;}
    inline StatusBar * statusBar() {return statusBar_;}
    void setHandle(int handle);
    void setInitText(const QString &text);
    void setText(const QString &text);
    void setText(const char* txt, int insertMode);
    QString text();
    std::string makeModal(int *positionAndSize);

    void showHelp();
    void showHelp(const QUrl &url);
    void hideHelp();
protected:
    void showHelp(bool v);

private:
    void closeEvent(QCloseEvent *event);

private slots:
    void reject();
    void updateReloadButtonVisualClue();
    void reloadScript();
public slots:
    void onSimulationRunning(bool running);
    void updateCursorSelectionDisplay();

public:
    inline EditorOptions options() const { return opts; }
    void openURL(const QString &url);

private:
    UI *ui;
    ToolBar *toolBar_;
    QMap<QString, Editor*> editors_;
    Editor *activeEditor_;
    QStackedWidget *stacked_;
    QTextBrowser *textBrowser_;
    SearchAndReplacePanel *searchPanel_;
    StatusBar *statusBar_;
    int handle;
    int scriptTypeOrHandle;
    EditorOptions opts;
    static QString modalText;
    static int modalPosAndSize[4];
    int memorizedPos[2] = { -999999,-999999 };
    QString initText_;
    bool scriptRestartInitiallyNeeded_ {false};
    QTimer *dirtyCheckTimer_;
    bool firstTimeSeeingSimulationStatus_ {true};

    friend class Toolbar;
};

#endif // DIALOG_H

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

    void setEditorOptions(const EditorOptions &opts);
    inline const EditorOptions & editorOptions() { return opts; }
    Editor * activeEditor();
    Editor * openExternalFile(const QString &filePath);
    void closeExternalFile(const QString &filePath);
    void closeExternalFile(Editor *editor);
    void switchEditor(Editor *editor);
    inline const QMap<QString, Editor*> & editors() {return editors_;}
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
    QMap<QString, Editor*> editors_;
    Editor *activeEditor_;
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

#endif // DIALOG_H

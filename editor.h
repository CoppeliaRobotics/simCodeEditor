#ifndef EDITOR_H
#define EDITOR_H

#include <QTextEdit>
#include "common.h"

class Dialog;

class Editor : public QTextEdit
{
    Q_OBJECT

public:
    Editor(Dialog *dialog);
    bool isActive() const;
    inline const EditorOptions & editorOptions() { return opts; }
    void setEditorOptions(const EditorOptions &opts);
    void contextMenuEvent(QContextMenuEvent *event);
    QString tokenAtPosition(int pos);
    QString tokenAtPosition2(int pos);
    int positionFromPoint(const QPoint &p);
    QString tokenAt(const QPoint &p);

public slots:
    void setText(const char* txt, int insertMode);
    //void setAStyle(int style, QColor fore, QColor back, int size=-1, const char *face = nullptr, bool bold = false);
    //void onCharAdded(int charAdded);
    //void onUpdateUi(int updated);
    //void onModified(int, int, const char *, int, int, int, int, int, int, int);
    void onTextChanged();
    void onCursorPosChanged();
    void onSelectionChanged();
    void indentSelectedText();
    void unindentSelectedText();
    void openExternalFile(const QString &filePath);
    void saveExternalFile();

public:
    QString externalFile();
    bool needsSaving();
    bool canSave();

    void ensurePositionVisible(int pos);
    void ensureCurrentLineVisible();
    QString selectedText() const;
    QString text(int start = 0, int end = -1) const;
    inline bool isUndoAvailable() { return undoAvailable_; }
    inline bool isRedoAvailable() { return redoAvailable_; }
    inline void insert(const QString &txt) { insertPlainText(txt); }

    inline EditorOptions options() const { return opts; }

private:
    bool undoAvailable_{false};
    bool redoAvailable_{false};
    QString getCallTip(const QString &txt);
    std::string divideString(const char* s) const;

    Dialog *dialog;
    EditorOptions opts;
    struct {
        QString path;
        bool edited;
    } externalFile_;
};

#endif // EDITOR_H

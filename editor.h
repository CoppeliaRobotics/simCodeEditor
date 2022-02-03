#ifndef EDITOR_H
#define EDITOR_H

#include <Qsci/qsciscintilla.h>
#include "common.h"

class Dialog;

class Editor : public QsciScintilla
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
    void setAStyle(int style, QColor fore, QColor back, int size=-1, const char *face = nullptr, bool bold = false);
    void onCharAdded(int charAdded);
    void onUpdateUi(int updated);
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

    inline EditorOptions options() const { return opts; }

private:
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

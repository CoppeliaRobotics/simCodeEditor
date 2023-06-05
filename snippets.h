#ifndef SNIPPETS_H
#define SNIPPETS_H

#include <QtWidgets>

#include "common.h"

struct Snippet
{
    QString name;
    QString content;
    QString filePath;
    QDateTime lastModified;

    bool changed() const;
};

struct SnippetGroup
{
    QString name;
    QString relDir;
    QMap<QString, QDateTime> lastModified;
    QMap<QString, Snippet> snippets;

    bool changed() const;
    bool empty() const;
};

class Dialog;

class SnippetsLibrary
{
public:
    SnippetsLibrary();
    void load(const EditorOptions &opts);
private:
    void loadFromPath(const EditorOptions &opts, const QString &path);
    bool readFile(const EditorOptions &opts, const QString &file, QMap<QString, QString> &meta, QString &content) const;
public:
    bool changed() const;
    bool empty() const;
    void fillMenu(Dialog *parent, QMenu *menu) const;
private:
    QMap<QString, SnippetGroup> snippetGroups;
};

#endif // SNIPPETS_H

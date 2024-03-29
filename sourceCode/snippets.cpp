#include "snippets.h"
#include "dialog.h"
#include "editor.h"
#include <simPlusPlus/Lib.h>

bool Snippet::changed() const
{
    QFileInfo info(filePath);
    return info.lastModified() > lastModified;
}

bool SnippetGroup::changed() const
{
    for(const auto &dir : lastModified.keys())
    {
        QFileInfo info(dir);
        if(info.lastModified() > lastModified.value(dir))
            return true;
    }
    for(const auto &snippet : snippets.values())
        if(snippet.changed())
            return true;
    return false;
}

bool SnippetGroup::empty() const
{
    return snippets.isEmpty();
}

SnippetsLibrary::SnippetsLibrary()
{
}

void SnippetsLibrary::load(const EditorOptions &opts)
{
    snippetGroups.clear();

    QStringList snippetLocations;

    for(auto &path : opts.snippetsPaths)
    {
        QDir snippetsBaseDir(path);
        if(snippetsBaseDir.cd(opts.snippetsGroup))
            snippetLocations << snippetsBaseDir.absolutePath();
    }

    for(const auto &dir : snippetLocations)
        loadFromPath(opts, dir);
}

void SnippetsLibrary::loadFromPath(const EditorOptions &opts, const QString &path)
{
    // scan subdirectories:
    QStringList dirs;
    QDirIterator iDir(path, QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while(iDir.hasNext()) dirs << iDir.next();
    dirs.sort();
    dirs << path;
    QDir root(path);

    for(const auto &dir : dirs)
    {
        QString relDir = root.relativeFilePath(dir);

        QMap<QString, QString> dirMeta;
        QString _;
        readFile(opts, dir + "/__index__." + opts.langExt, dirMeta, _);
        if(relDir == ".") dirMeta["sortKey"] = "~~~~~~~~";

        auto k = dirMeta.value("sortKey", relDir);

        if(!snippetGroups.contains(k))
            snippetGroups[k] = {};
        SnippetGroup &snippetGroup = snippetGroups[k];
        snippetGroup.name = dirMeta.value("name", relDir);
        snippetGroup.relDir = relDir;
        QFileInfo dirInfo(dir);
        snippetGroup.lastModified[dir] = dirInfo.lastModified();

        QDirIterator iFile(dir, QStringList() << "*." + opts.langExt, QDir::Files);
        QStringList files;
        while(iFile.hasNext())
            files << iFile.next();
        files.sort();

        for(const auto &file : files)
        {
            QFileInfo info(file);
            if(info.baseName() == "__index__") continue;
            Snippet snippet;
            QMap<QString, QString> meta;
            readFile(opts, file, meta, snippet.content);
            snippet.name = meta.value("name", info.baseName());
            snippet.filePath = file;
            snippet.lastModified = info.lastModified();
            auto k = meta.value("sortKey", info.baseName());
            snippetGroup.snippets[k] = snippet;
        }
    }
}

bool SnippetsLibrary::readFile(const EditorOptions &opts, const QString &file, QMap<QString, QString> &meta, QString &content) const
{
    QFile f(file);
    if(!f.open(QFile::ReadOnly | QFile::Text))
        return false;

    QTextStream in(&f);
    content = "";
    QString line;
    QRegularExpression re("^\\s*" + opts.langComment + "\\s*@(\\w+) (.*)$");
    while(!in.atEnd())
    {
        line = in.readLine();
        auto m = re.match(line);
        if(m.hasMatch())
        {
            QString key = m.captured(1);
            QString val = m.captured(2);
            meta[key] = val;
        }
        else
        {
            content += line;
            content += "\n";
        }
    }
    f.close();
    return true;
}

bool SnippetsLibrary::changed() const
{
    for(const auto &snippetGroup : snippetGroups)
        if(snippetGroup.changed())
            return true;
    return false;
}

bool SnippetsLibrary::empty() const
{
    for(const auto &snippetGroup : snippetGroups)
        if(!snippetGroup.empty())
            return false;
    return true;
}

void SnippetsLibrary::fillMenu(Dialog *parent, QMenu *menu) const
{
    menu->clear();
    for(const auto &snippetGroup : snippetGroups)
    {
        QMenu *subMenu = menu;
        if(snippetGroup.relDir != ".")
            menu->addMenu(subMenu = new QMenu(snippetGroup.name, parent));

        for(const auto &snippet : snippetGroup.snippets)
        {
            QAction *a = new QAction(snippet.name);
            QString content = snippet.content;
            QObject::connect(a, &QAction::triggered, [=] {
                auto e = parent->activeEditor();
                int line, index;
                e->getCursorPosition(&line, &index);
                int pos = e->positionFromLineIndex(line, index);
                e->insert(content);
            });
            subMenu->addAction(a);
        }
    }
}

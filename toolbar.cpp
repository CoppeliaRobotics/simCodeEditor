#include "toolbar.h"
#include "dialog.h"
#include "editor.h"
#include "searchandreplacepanel.h"

inline bool isDarkMode(QWidget *w)
{
    QColor bg = w->palette().color(QPalette::Window),
           fg = w->palette().color(QPalette::WindowText);
    return bg.value() < fg.value();
}

inline QPixmap loadPixmap(QWidget *w, const uchar *data, int len)
{
    QImage im;
    im.loadFromData(data, len);
    if(isDarkMode(w))
        im.invertPixels();
    return QPixmap::fromImage(im);
}

#include "icons/icons.cpp"
#define ICON(x) QPixmap x = loadPixmap(this, x ## _png, x ## _png_len)

ToolBar::ToolBar(bool canRestart, Dialog *parent)
    : QToolBar(parent),
      parent(parent)
{
    setIconSize(QSize(16, 16));

    ICON(upload);
    addAction(actReload = new QAction(QIcon(upload), "Restart script"));
    actReload->setEnabled(canRestart);

    ICON(search);
    addAction(actShowSearchPanel = new QAction(QIcon(search), "Find and replace"));
    actShowSearchPanel->setCheckable(true);

    ICON(undo);
    addAction(actUndo = new QAction(QIcon(undo), "Undo"));

    ICON(redo);
    addAction(actRedo = new QAction(QIcon(redo), "Redo"));

    ICON(unindent);
    addAction(actUnindent = new QAction(QIcon(unindent), "Unindent"));

    ICON(indent);
    addAction(actIndent = new QAction(QIcon(indent), "Indent"));

    ICON(func);
    funcNav.menu = new QMenu(parent);
    funcNav.act = funcNav.menu->menuAction();
    funcNav.act->setIcon(QIcon(func));
    addAction(funcNav.act);
    QWidget *funcNavWidget = widgetForAction(funcNav.act);
    QToolButton *funcNavButton = qobject_cast<QToolButton*>(funcNavWidget);
    if(funcNavButton)
    {
        funcNavButton->setToolTip("Function navigator");
        connect(funcNav.act, &QAction::triggered, funcNavButton, &QToolButton::showMenu);
    }

    ICON(snippet);
    snippetLib.menu = new QMenu(parent);
    snippetLib.act = snippetLib.menu->menuAction();
    snippetLib.act->setIcon(QIcon(snippet));
    addAction(snippetLib.act);
    QWidget *snippetLibWidget = widgetForAction(snippetLib.act);
    QToolButton *snippetButton = qobject_cast<QToolButton*>(snippetLibWidget);
    if(snippetButton)
    {
        snippetButton->setToolTip("Snippets library");
        connect(snippetLib.act, &QAction::triggered, snippetButton, &QToolButton::showMenu);
    }
    snippetsLibrary.load();
    snippetsLibrary.fillMenu(parent, snippetLib.menu);
    snippetLib.act->setVisible(!snippetsLibrary.empty());

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    addWidget(spacer);

    ICON(save);
    addAction(openFiles.actSave = new QAction(QIcon(save), "Save current file"));
    openFiles.combo = new QComboBox;
    openFiles.actCombo = addWidget(openFiles.combo);
    ICON(close);
    addAction(openFiles.actClose = new QAction(QIcon(close), "Close current file"));

    addAction(actCloseHelp = new QAction("Close help"));
    actCloseHelp->setVisible(false);
    connect(actCloseHelp, &QAction::triggered, [=] {parent->hideHelp();});
}

ToolBar::~ToolBar()
{
}

void getFunctionDefs(const QString &lua, QVector<QString> &names, QVector<int> &pos)
{
    QMap<QString, int> ret;
    QRegularExpression regexp("("
        "function\\s+([a-zA-Z0-9_.:]+)\\s*(\\(.*\\))"
    "|" "([a-zA-Z0-9_.]+)\\s*=\\s*function\\s*(\\(.*\\))"
    ")");
    auto i = regexp.globalMatch(lua);
    while(i.hasNext())
    {
        const auto &m = i.next();
        names.append(m.captured(2) + m.captured(3) + m.captured(4) + m.captured(5));
        pos.append(qMax(m.capturedStart(2), m.capturedStart(4)));
    }
}

void ToolBar::updateButtons()
{
    auto activeEditor = parent->activeEditor();
    actUndo->setEnabled(activeEditor->isUndoAvailable());
    actRedo->setEnabled(activeEditor->isRedoAvailable());

    int fromLine, fromIndex, toLine, toIndex;
    activeEditor->getSelection(&fromLine, &fromIndex, &toLine, &toIndex);
    bool hasSel = fromLine != -1;
    actIndent->setEnabled(hasSel);
    actUnindent->setEnabled(hasSel);

    actShowSearchPanel->setChecked(parent->searchPanel()->isVisible());

    openFiles.actClose->setEnabled(!activeEditor->externalFile().isEmpty());
    openFiles.actSave->setEnabled(activeEditor->needsSaving());

    int i = 0, sel = -1;
    bool obs = openFiles.combo->blockSignals(true);
    openFiles.combo->clear();
    const auto &editors = parent->editors();
    for(auto path : editors.keys())
    {
        QString name("<embedded script>");
        if(!path.isEmpty()) name = path;
        auto editor = editors[path];
        if(editor->needsSaving()) name = "* " + name;
        openFiles.combo->addItem(name, QVariant::fromValue(editor));
        if(editor == parent->activeEditor()) sel = i;
        i++;
    }
    openFiles.combo->setCurrentIndex(sel);
    openFiles.combo->blockSignals(obs);
    openFiles.setVisible(editors.count() > 1);

    funcNav.menu->clear();
    QVector<QString> names;
    QVector<int> pos;
    getFunctionDefs(parent->activeEditor()->text(), names, pos);
    for(int i = 0; i < names.count(); i++)
    {
        int line, index;
        parent->activeEditor()->lineIndexFromPosition(pos[i], &line, &index);
        QAction *a = new QAction(names[i]);
        connect(a, &QAction::triggered, [this, line] {
            auto e = parent->activeEditor();
            e->ensureLineVisible(line);
            e->setSelection(line, 0, line + 1, 0);
            QTimer::singleShot(200, [=] {e->setSelection(line, 0, line, 0);});
        });
        funcNav.menu->addAction(a);
    }
    funcNav.act->setEnabled(!names.isEmpty());

    if(snippetsLibrary.changed())
    {
        snippetsLibrary.load();
        snippetsLibrary.fillMenu(parent, snippetLib.menu);
        snippetLib.act->setVisible(!snippetsLibrary.empty());
    }
}

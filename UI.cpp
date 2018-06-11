#include "UI.h"
#include "debug.h"
#include "v_repLib.h"
#include "scintillaDlg.h"
#include "QtUtils.h"
#include <QDomDocument>
#include <QDomElement>

CScintillaDlg * UI::createWindow(bool modal, const QString &initText, const QString &properties)
{
    ASSERT_THREAD(UI);

    QDomDocument doc;
    doc.setContent(properties.isEmpty() ? "<editor/>" : properties);
    QDomElement e = doc.documentElement();
    if(e.tagName() != "editor") e = {};

    QWidget *parent = (QWidget*)simGetMainWindow(1);

    CScintillaDlg *window = new CScintillaDlg(this, parent);

    window->setWindowTitle(e.attribute("title", "Editor"));

    bool resizable = parseBool(e.attribute("resizable", "false"));
    window->statusbar()->setSizeGripEnabled(resizable);
    bool closeable = parseBool(e.attribute("closeable", "false"));
    Qt::WindowFlags flags = Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint;
#ifdef MAC_VREP
    flags |= Qt::Tool;
#else
    flags |= Qt::Dialog;
#endif
    if(resizable) flags |= Qt::WindowMaximizeButtonHint;
    else flags |= Qt::MSWindowsFixedSizeDialogHint;
    if(modal || closeable) flags |= Qt::WindowCloseButtonHint;
    window->setWindowFlags(flags);

    QStringList sizeStrLst = e.attribute("size", "800 600").split(" ");
    QSize size(sizeStrLst[0].toInt(), sizeStrLst[1].toInt());
    window->resize(size);

    QStringList posStrLst = e.attribute("position", "50 50").split(" ");
    QPoint pos(posStrLst[0].toInt(), posStrLst[1].toInt());
    QString placement = e.attribute("placement", "center");
    QRect frameGeom = parent->frameGeometry();
    if(placement == "absolute")
    {
        window->move(pos);
    }
    else if(placement == "relative")
    {
        window->move(
            pos.x() + pos.x() >= 0 ? frameGeom.left() : (frameGeom.right() - window->width()),
            pos.y() + pos.y() >= 0 ? frameGeom.top() : (frameGeom.bottom() - window->height())
        );
    }
    else if(placement == "center")
    {
        window->move(
            frameGeom.left() + (frameGeom.width() - window->width()) / 2,
            frameGeom.top() + (frameGeom.height() - window->height()) / 2
        );
    }

    bool activate = parseBool(e.attribute("activate", "true"));
    window->setAttribute(Qt::WA_ShowWithoutActivating, !activate);

    bool editable = parseBool(e.attribute("editable", "true"));
    window->scintilla()->setReadOnly(!editable);

    bool searchable = parseBool(e.attribute("searchable", "true"));
    window->toolbar()->actShowSearchPanel->setEnabled(searchable);

    int tab_width = e.attribute("tab-width", "4").toInt();
    window->scintilla()->setTabWidth(tab_width);

    bool is_lua = parseBool(e.attribute("is-lua", "false"));
    // ???

    bool is_vrep_code = parseBool(e.attribute("is-vrep-code", "false"));
    // ???

    int script_handle = e.attribute("script-handle", "-1").toInt();
    if(script_handle != -1)
        window->scintilla()->setText(simGetScriptText(script_handle));
    else
        window->scintilla()->setText(initText);

    QColor text_col = parseColor(e.attribute("text-col", "50 50 50"));
    QColor background_col = parseColor(e.attribute("background-col", "190 190 190"));
    QColor selection_col = parseColor(e.attribute("selection-col", "128 128 255"));
    QColor comment_col = parseColor(e.attribute("comment-col", "0 140 0"));
    QColor number_col = parseColor(e.attribute("number-col", "220 0 220"));
    QColor string_col = parseColor(e.attribute("string-col", "255 255 0"));
    QColor character_col = parseColor(e.attribute("character-col", "255 255 0"));
    QColor operator_col = parseColor(e.attribute("operator-col", "0 0 0"));
    QColor identifier_col = parseColor(e.attribute("identifier-col", "64 64 64"));
    QColor preprocessor_col = parseColor(e.attribute("preprocessor-col", "0 128 128"));
    QColor keyword1_col = parseColor(e.attribute("keyword1-col", "0 0 255"));
    QColor keyword2_col = parseColor(e.attribute("keyword2-col", "152 0 0"));
    QColor keyword3_col = parseColor(e.attribute("keyword3-col", "220 80 20"));
    QColor keyword4_col = parseColor(e.attribute("keyword4-col", "152 64 0"));
    window->setColorTheme(text_col, background_col, selection_col, comment_col, number_col, string_col, character_col, operator_col, identifier_col, preprocessor_col, keyword1_col, keyword2_col, keyword3_col, keyword4_col);

    for(QDomNode n1 = e.firstChild(); !n1.isNull(); n1 = n1.nextSibling())
    {
        QDomElement e1 = n1.toElement();
        if(e1.isNull()) continue;
        if(e1.tagName() == "keyword1")
        {
            QString keywords1 = "";
            for(QDomNode n2 = e1.firstChild(); !n2.isNull(); n2 = n2.nextSibling())
            {
                QDomElement e2 = n2.toElement();
                if(e2.isNull()) continue;
                if(e2.tagName() == "item")
                {
                    QString word = e2.attribute("word");
                    bool autocomplete = parseBool(e2.attribute("autocomplete"));
                    QString calltip = e2.attribute("calltip");
                    if(!keywords1.isEmpty()) keywords1.append(" ");
                    keywords1.append(word);
                }
            }
            window->scintilla()->SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)1, keywords1.toLocal8Bit().data());
        }
        else if(e1.tagName() == "keyword2")
        {
            QString keywords2 = "";
            for(QDomNode n2 = e1.firstChild(); !n2.isNull(); n2 = n2.nextSibling())
            {
                QDomElement e2 = n2.toElement();
                if(e2.isNull()) continue;
                if(e2.tagName() == "item")
                {
                    QString word = e2.attribute("word");
                    bool autocomplete = parseBool(e2.attribute("autocomplete"));
                    QString calltip = e2.attribute("calltip");
                    if(!keywords2.isEmpty()) keywords2.append(" ");
                    keywords2.append(word);
                }
            }
            window->scintilla()->SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)2, keywords2.toLocal8Bit().data());
        }
    }

    window->show();
    if(activate)
    {
        window->raise();
        window->activateWindow();
    }

#if defined(LIN_VREP) || defined(MAC_VREP)
    if(!resizable) window->setFixedSize(window->size());
#endif

    return window;
}

void UI::openModal(const QString &initText, const QString &properties, QSemaphore *sem, QString *text, int *positionAndSize)
{
    ASSERT_THREAD(UI);

    CScintillaDlg *editor = createWindow(true, initText, properties);
    editor->setModal(sem, text, positionAndSize);
}

void UI::open(const QString &initText, const QString &properties, int *handle)
{
    ASSERT_THREAD(UI);

    CScintillaDlg *editor = createWindow(false, initText, properties);
    *handle = nextEditorHandle++;
    editor->setHandle(*handle);
    editors[*handle] = editor;
}

void UI::setText(int handle, const QString &text, int insertMode)
{
    ASSERT_THREAD(UI);

    CScintillaDlg *editor = editors.value(handle);
    if(editor)
    {
        editor->scintilla()->setText(text);
    }
}

void UI::getText(int handle, QString *text)
{
    ASSERT_THREAD(UI);

    CScintillaDlg *editor = editors.value(handle);
    if(editor)
    {
        *text = editor->scintilla()->text();
    }
}

void UI::show(int handle, int showState)
{
    ASSERT_THREAD(UI);

    CScintillaDlg *editor = editors.value(handle);
    if(editor)
    {
    }
}

void UI::close(int handle, int *positionAndSize)
{
    ASSERT_THREAD(UI);

    CScintillaDlg *editor = editors.value(handle);
    if(editor)
    {
        if(positionAndSize)
        {
            positionAndSize[0] = editor->x();
            positionAndSize[1] = editor->y();
            positionAndSize[3] = editor->width();
            positionAndSize[4] = editor->height();
        }
        delete editor;
        editors.remove(handle);
    }
}


#include "UI.h"
#include "debug.h"
#include "v_repLib.h"
#include "scintillaDlg.h"
#include "QtUtils.h"
#include <QDomDocument>
#include <QDomElement>

CScintillaDlg * UI::createWindow(bool modalSpecial, const QString &initText, const QString &properties)
{
    ASSERT_THREAD(UI);

    QDomDocument doc;
    doc.setContent(properties.isEmpty() ? "<editor/>" : properties);
    QDomElement e = doc.documentElement();
    if(e.tagName() != "editor") e = {};

    QWidget *parent = (QWidget *)simGetMainWindow(1);

    bool toolBar = parseBool(e.attribute("toolbar", "false"));
    bool statusBar = parseBool(e.attribute("statusbar", "false"));
    bool canRestart = parseBool(e.attribute("can-restart", "false"));
    bool searchable = parseBool(e.attribute("searchable", "true"));
    CScintillaDlg *window = new CScintillaDlg(toolBar, statusBar, canRestart, searchable, this, parent);

    window->setWindowTitle(e.attribute("title", "Editor"));

    bool resizable = parseBool(e.attribute("resizable", "true"));
    window->statusBar()->setSizeGripEnabled(resizable);
    bool modal = parseBool(e.attribute("modal", "false"));
    window->setModal(modal);
    bool closeable = parseBool(e.attribute("closeable", "true"));
    Qt::WindowFlags flags = Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint; // | Qt::WindowStaysOnTopHint;
#ifdef MAC_VREP
    flags |= Qt::Tool;
#else
    flags |= Qt::Dialog;
#endif
    if(resizable) flags |= Qt::WindowMaximizeButtonHint;
    else flags |= Qt::MSWindowsFixedSizeDialogHint;
    if(modalSpecial || closeable) flags |= Qt::WindowCloseButtonHint;
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
    std::string font(e.attribute("font", "Courier").toStdString());
    int fontSize = e.attribute("font-size", "14").toInt();

    bool activate = parseBool(e.attribute("activate", "true"));
    window->setAttribute(Qt::WA_ShowWithoutActivating, !activate);

    bool editable = parseBool(e.attribute("editable", "true"));
    window->scintilla()->setReadOnly(!editable);

    bool lineNumbers = parseBool(e.attribute("line-numbers", "false"));
    int maxLines = e.attribute("max-lines", "0").toInt();

    int tab_width = e.attribute("tab-width", "4").toInt();
    window->scintilla()->setTabWidth(tab_width);
    window->toolBar()->actShowSearchPanel->setEnabled(searchable);

    bool is_lua = parseBool(e.attribute("is-lua", "false"));

    std::string onClose = e.attribute("on-close", "").toStdString();

    bool wrapWord = parseBool(e.attribute("wrap-word", "false"));

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
    QColor keyword1_col = parseColor(e.attribute("keyword1-col", "152 0 0"));
    QColor keyword2_col = parseColor(e.attribute("keyword2-col", "220 80 20"));
    QColor keyword3_col = parseColor(e.attribute("keyword3-col", "0 0 255"));
    QColor keyword4_col = parseColor(e.attribute("keyword4-col", "152 64 0"));

    std::vector<SScintillaUserKeyword> userKeywords;

    for(QDomNode n1 = e.firstChild(); !n1.isNull(); n1 = n1.nextSibling())
    {
        QDomElement e1 = n1.toElement();
        if(e1.isNull()) continue;
        if(e1.tagName() == "keywords1")
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
                    SScintillaUserKeyword kw;
                    kw.keyword = word.toStdString();
                    kw.autocomplete = autocomplete;
                    kw.callTip = calltip.toStdString();
                    userKeywords.push_back(kw);
                }
            }
            window->scintilla()->SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)1, keywords1.toStdString().c_str());
        }
        else if(e1.tagName() == "keywords2")
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
                    SScintillaUserKeyword kw;
                    kw.keyword = word.toStdString();
                    kw.autocomplete = autocomplete;
                    kw.callTip = calltip.toStdString();
                    userKeywords.push_back(kw);
                }
            }
            window->scintilla()->SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)2, keywords2.toStdString().c_str());
        }
    }

    window->setTheme(modalSpecial, lineNumbers, maxLines, is_lua, onClose, wrapWord, font, fontSize, userKeywords, text_col, background_col, selection_col, comment_col, number_col, string_col, character_col, operator_col, identifier_col, preprocessor_col, keyword1_col, keyword2_col, keyword3_col, keyword4_col);

    if (!modalSpecial)
    {
        window->show();
        if (activate)
        {
            window->raise();
            window->activateWindow();
        }
    }
#if defined(LIN_VREP) || defined(MAC_VREP)
    if(!resizable) window->setFixedSize(window->size());
#endif

    return window;
}

void UI::openModal(const QString &initText, const QString &properties, QString& text, int *positionAndSize)
{
    ASSERT_THREAD(UI);

    CScintillaDlg *editor = createWindow(true, initText, properties);
    text = editor->makeModal(positionAndSize).c_str();
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
    if (editor)
        editor->setText(text.toStdString().c_str(), insertMode);
}

void UI::getText(int handle, QString *text, int* posAndSize)
{
    ASSERT_THREAD(UI);

    CScintillaDlg *editor = editors.value(handle);
    if(editor)
    {
        *text = editor->scintilla()->text();
        if (posAndSize != nullptr)
        {
            posAndSize[0] = editor->x();
            posAndSize[1] = editor->y();
            posAndSize[2] = editor->width();
            posAndSize[3] = editor->height();
        }
    }
}

void UI::show(int handle, int showState)
{
    ASSERT_THREAD(UI);

    CScintillaDlg *editor = editors.value(handle);
    if(editor)
    {
        if (showState)
            editor->show();
        else
            editor->hide();
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
            positionAndSize[2] = editor->width();
            positionAndSize[3] = editor->height();
        }
        delete editor;
        editors.remove(handle);
    }
}

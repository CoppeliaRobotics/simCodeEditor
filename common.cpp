#include "common.h"
#include "QtUtils.h"
#include <QDomDocument>
#include <QDomElement>
#include <QFileInfo>

void EditorOptions::readFromXML(const QString &xml)
{
    QDomDocument doc;
    doc.setContent(xml.isEmpty() ? "<editor/>" : xml);
    QDomElement e = doc.documentElement();
    if(e.tagName() != "editor") e = {};

    toolBar = parseBool(e.attribute("toolbar", "false"));
    statusBar = parseBool(e.attribute("statusbar", "false"));
    canRestart = parseBool(e.attribute("can-restart", "false"));
    searchable = parseBool(e.attribute("searchable", "true"));
    windowTitle = e.attribute("title", "Editor");
    resizable = parseBool(e.attribute("resizable", "true"));
    closeable = parseBool(e.attribute("closeable", "true"));
    modal = parseBool(e.attribute("modal", "false"));
    QStringList sizeStrLst = e.attribute("size", "800 600").split(" ");
    size = QSize(sizeStrLst[0].toInt(), sizeStrLst[1].toInt());
    QStringList posStrLst = e.attribute("position", "50 50").split(" ");
    pos = QPoint(posStrLst[0].toInt(), posStrLst[1].toInt());
    QString placement = e.attribute("placement", "center");
    if(placement == "absolute")
        placement = EditorOptions::Placement::Absolute;
    else if(placement == "relative")
        placement = EditorOptions::Placement::Relative;
    else if(placement == "center")
        placement = EditorOptions::Placement::Center;
    fontFace = e.attribute("font", "Courier");
    fontSize = e.attribute("font-size", "14").toInt();
    activate = parseBool(e.attribute("activate", "true"));
    editable = parseBool(e.attribute("editable", "true"));
    lineNumbers = parseBool(e.attribute("line-numbers", "false"));
    maxLines = e.attribute("max-lines", "0").toInt();
    tab_width = e.attribute("tab-width", "4").toInt();
    isLua = parseBool(e.attribute("is-lua", "false"));
    onClose = e.attribute("on-close", "");
    wrapWord = parseBool(e.attribute("wrap-word", "false"));
    text_col = parseColor(e.attribute("text-col", "50 50 50"));
    background_col = parseColor(e.attribute("background-col", "190 190 190"));
    selection_col = parseColor(e.attribute("selection-col", "128 128 255"));
    comment_col = parseColor(e.attribute("comment-col", "0 140 0"));
    number_col = parseColor(e.attribute("number-col", "220 0 220"));
    string_col = parseColor(e.attribute("string-col", "255 255 0"));
    character_col = parseColor(e.attribute("character-col", "255 255 0"));
    operator_col = parseColor(e.attribute("operator-col", "0 0 0"));
    identifier_col = parseColor(e.attribute("identifier-col", "64 64 64"));
    preprocessor_col = parseColor(e.attribute("preprocessor-col", "0 128 128"));
    keyword1_col = parseColor(e.attribute("keyword1-col", "152 0 0"));
    keyword2_col = parseColor(e.attribute("keyword2-col", "220 80 20"));
    keyword3_col = parseColor(e.attribute("keyword3-col", "0 0 255"));
    keyword4_col = parseColor(e.attribute("keyword4-col", "152 64 0"));
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
                    UserKeyword kw;
                    kw.keyword = word;
                    kw.autocomplete = autocomplete;
                    kw.callTip = calltip;
                    userKeywords.push_back(kw);
                }
            }
            //window->scintilla()->SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)1, keywords1.toStdString().c_str());
            //window->setKeywords(1, o.userKeywords);
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
                    UserKeyword kw;
                    kw.keyword = word;
                    kw.autocomplete = autocomplete;
                    kw.callTip = calltip;
                    userKeywords.push_back(kw);
                }
            }
            //window->scintilla()->SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)2, keywords2.toStdString().c_str());
            //window->setKeywords(2, userKeywords);
        }
    }

    // FIXME: parse XML and add *directories* to luaSearchPath
    //        in the *correct* order (low index => high priority):
    luaSearchPath.push_back("/Users/me/Dev/CoppeliaRobotics/build/output.macos/lua");
}

QString EditorOptions::resolveLuaFilePath(const QString &f)
{
    if(f == "") return "";

    for(auto path : luaSearchPath)
    {
        QString fullPath = path + "/" + f;
        QFileInfo i(fullPath);
        if(i.exists())
            return fullPath;
    }

    return "";
}

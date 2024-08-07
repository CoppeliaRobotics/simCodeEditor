#include "common.h"
#include <QDomDocument>
#include <QDomElement>
#include <QFileInfo>
#include <QByteArray>
#include <QStringList>
#include "plugin.h"
#include <simPlusPlus/Lib.h>

QString EditorOptions::resourcesPath{};

void EditorOptions::readFromXML(const QString &xml)
{
    QDomDocument doc;
    doc.setContent(xml.isEmpty() ? "<editor/>" : xml);
    QDomElement e = doc.documentElement();
    if(e.tagName() != "editor") e = {};

    toolBar = parseBool(e.attribute("toolbar", "false"));
    statusBar = parseBool(e.attribute("statusbar", "false"));
    if(e.hasAttribute("can-restart"))
    {
        bool b = parseBool(e.attribute("can-restart", "false"));
        canRestartInSim = b;
        canRestartInNonsim = b;
    }
    else
    {
        canRestartInSim = parseBool(e.attribute("can-restart-in-sim", "false"));
        canRestartInNonsim = parseBool(e.attribute("can-restart-in-nosim", "false"));
    }
    searchable = parseBool(e.attribute("searchable", "true"));
    windowTitle = e.attribute("title", "Editor");
    resizable = parseBool(e.attribute("resizable", "true"));
    closeable = parseBool(e.attribute("closeable", "true"));
    modal = parseBool(e.attribute("modal", "false"));
    QStringList sizeStrLst = e.attribute("size", "800 600").split(" ");
    size = QSize(sizeStrLst[0].toInt(), sizeStrLst[1].toInt());
    QStringList posStrLst = e.attribute("position", "50 50").split(" ");
    pos = QPoint(posStrLst[0].toInt(), posStrLst[1].toInt());
    QString pl = e.attribute("placement", "center");
    if(pl == "absolute")
        placement = EditorOptions::Placement::Absolute;
    else if(pl == "relative")
        placement = EditorOptions::Placement::Relative;
    else if(pl == "center")
        placement = EditorOptions::Placement::Center;
    fontFace = e.attribute("font",
#ifdef __linux__
            "DejaVu Sans Mono" // prob. available on all Linux platforms. For Ubuntu only "Ubuntu Mono" would be better
#else
            "Courier New" // always available on Windows and macOS. "Courier" & Scintilla is problematic on macOS
#endif
    );
    fontSize = e.attribute("font-size", "14").toInt();
    fontBold = parseBool(e.attribute("font-bold", "false"));
    activate = parseBool(e.attribute("activate", "true"));
    editable = parseBool(e.attribute("editable", "true"));
    clearable = parseBool(e.attribute("clearable", "false"));
    lineNumbers = parseBool(e.attribute("line-numbers", "false"));
    maxLines = e.attribute("max-lines", "0").toInt();
    tab_width = e.attribute("tab-width", "4").toInt();
    if(e.hasAttribute("is-lua"))
        sim::addLog(sim_verbosity_errors, "XML contains deprecated 'is-lua' attribute");
    doesScriptInitiallyNeedRestart = !parseBool(e.attribute("script-up-to-date", "true"));
    QString defaultLang = "none";
    QString defaultLangComment = "";
    QString defaultLangExt = "txt";
    lang = e.attribute("lang", defaultLang);
    if (lang == "lua")
    {
        defaultLangExt = "lua";
        defaultLangComment = "--";
    }
    else if (lang == "python")
    {
        defaultLangExt = "py";
        defaultLangComment = "#";
    }
    else if (lang == "json")
    {
        defaultLangExt = "json";
        defaultLangComment = "//";
    }
    langExt = e.attribute("lang-ext", defaultLangExt);
    langComment = e.attribute("lang-comment", defaultLangComment);
    snippetsGroup = e.attribute("snippets-group", lang);
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
            for(QDomNode n2 = e1.firstChild(); !n2.isNull(); n2 = n2.nextSibling())
            {
                QDomElement e2 = n2.toElement();
                if(e2.isNull()) continue;
                if(e2.tagName() == "item")
                {
                    QString word = e2.attribute("word");
                    bool autocomplete = parseBool(e2.attribute("autocomplete"));
                    QString calltip = e2.attribute("calltip");
                    UserKeyword kw;
                    kw.keyword = word;
                    kw.autocomplete = autocomplete;
                    kw.callTip = calltip;
                    kw.keywordType = 1;
                    userKeywords.push_back(kw);
                }
            }
        }
        else if(e1.tagName() == "keywords2")
        {
            for(QDomNode n2 = e1.firstChild(); !n2.isNull(); n2 = n2.nextSibling())
            {
                QDomElement e2 = n2.toElement();
                if(e2.isNull()) continue;
                if(e2.tagName() == "item")
                {
                    QString word = e2.attribute("word");
                    bool autocomplete = parseBool(e2.attribute("autocomplete"));
                    QString calltip = e2.attribute("calltip");
                    UserKeyword kw;
                    kw.keyword = word;
                    kw.autocomplete = autocomplete;
                    kw.callTip = calltip;
                    kw.keywordType = 2;
                    userKeywords.push_back(kw);
                }
            }
        }
    }

    if(e.hasAttribute("lua-search-paths"))
        sim::addLog(sim_verbosity_errors, "XML contains deprecated 'lua-search-paths' attribute");
    QStringList spl = e.attribute("search-paths", e.attribute("lua-search-paths")).split(";");
    for (int i = 0; i < spl.size(); i++)
    {
        if (spl.at(i).size() > 1)
            scriptSearchPath.push_back(spl.at(i));
    }
}

QString EditorOptions::resolveScriptFilePath(const QString &f_)
{
    if(f_ == "") return "";

    QString f(f_);
    f.replace(".", "/");

    for(auto path : scriptSearchPath)
    {
        QString fullPath = path;
        fullPath.replace("?",f);
        fullPath.replace("//","/");
        QFileInfo i(fullPath);
        if(i.exists())
            return fullPath;
    }

    return "";
}

char * stringBufferCopy(const QString &str)
{
    QByteArray byteArr = str.toLocal8Bit();
    char *buff = reinterpret_cast<char *>(sim::createBuffer(byteArr.length() + 1));
    strcpy(buff, byteArr.data());
    buff[byteArr.length()] = '\0';
    return buff;
}

QColor parseColor(const QString &colorStr)
{
    QColor ret;
    QStringList colorStrLst = colorStr.split(" ");
    ret.setRed(colorStrLst[2].toInt());
    ret.setGreen(colorStrLst[1].toInt());
    ret.setBlue(colorStrLst[0].toInt());
    return ret;
}

bool parseBool(const QString &boolStr)
{
    return boolStr != "false";
}

QString elideLeft(const QString &str, int maxLength)
{
    if(str.length() <= maxLength) return str;
    QString r = str.right(maxLength);
    return QString::fromWCharArray(L"\x2026") + r;
}

QString elideMiddle(const QString &str, int maxLength)
{
    if(str.length() <= maxLength) return str;
    int half = maxLength / 2;
    QString l = str.left(half), r = str.right(half);
    return l + QString::fromWCharArray(L"\x2026") + r;
}

QString elideRight(const QString &str, int maxLength)
{
    if(str.length() <= maxLength) return str;
    QString l = str.left(maxLength);
    return l + QString::fromWCharArray(L"\x2026");
}

#include "editor.h"
#include "dialog.h"
#include "toolbar.h"
#include "UI.h"
#include <SciLexer.h>

#include <Qsci/qscilexer.h>
#include <Qsci/qscilexeravs.h>
#include <Qsci/qscilexerbash.h>
#include <Qsci/qscilexerbatch.h>
#include <Qsci/qscilexercmake.h>
#include <Qsci/qscilexercoffeescript.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexercsharp.h>
#include <Qsci/qscilexercss.h>
#include <Qsci/qscilexercustom.h>
#include <Qsci/qscilexerd.h>
#include <Qsci/qscilexerdiff.h>
#include <Qsci/qscilexeredifact.h>
#include <Qsci/qscilexerfortran.h>
#include <Qsci/qscilexerfortran77.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexeridl.h>
#include <Qsci/qscilexerjava.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexerjson.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscilexermakefile.h>
#include <Qsci/qscilexermarkdown.h>
#include <Qsci/qscilexermatlab.h>
#include <Qsci/qscilexeroctave.h>
#include <Qsci/qscilexerpascal.h>
#include <Qsci/qscilexerperl.h>
#include <Qsci/qscilexerpo.h>
#include <Qsci/qscilexerpostscript.h>
#include <Qsci/qscilexerpov.h>
#include <Qsci/qscilexerproperties.h>
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexerruby.h>
#include <Qsci/qscilexerspice.h>
#include <Qsci/qscilexersql.h>
#include <Qsci/qscilexertcl.h>
#include <Qsci/qscilexertex.h>
#include <Qsci/qscilexerverilog.h>
#include <Qsci/qscilexervhdl.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexeryaml.h>

// implemented in plugin.cpp:
QUrl apiReferenceForSymbol(const QString &sym);

Editor::Editor(Dialog *d)
    : QsciScintilla(d),
      dialog(d)
{
    SendScintilla(QsciScintillaBase::SCI_SETSTYLEBITS, 5);
    setTabWidth(4);
    setTabIndents(true);
    setBackspaceUnindents(true);
    setAutoIndent(true);
    SendScintilla(QsciScintillaBase::SCI_SETUSETABS, 0);

    connect(this, SIGNAL(SCN_CHARADDED(int)), this, SLOT(onCharAdded(int)));
    connect(this, SIGNAL(SCN_MODIFIED(int,int,const char*,int,int,int,int,int,int,int)), this, SLOT(onModified(int,int,const char*,int,int,int,int,int,int,int)));
    connect(this, &QsciScintilla::textChanged, this, &Editor::onTextChanged);
    connect(this, &QsciScintilla::selectionChanged, this, &Editor::onSelectionChanged);
    connect(this, &QsciScintilla::cursorPositionChanged, this, &Editor::onCursorPosChanged);
    connect(this, SIGNAL(SCN_UPDATEUI(int)), this, SLOT(onUpdateUi(int)));
}

bool Editor::isActive() const
{
    return dialog->activeEditor() == this;
}

#define MAP_LEXER_BEGIN if(0){}
#define MAP_LEXER(n, c) else if (o.lang == n) lexer = new c;
#define MAP_LEXER_END else if(0){}

void Editor::setEditorOptions(const EditorOptions &o)
{
    opts = o;
    QsciLexer* lexer = nullptr;

    MAP_LEXER_BEGIN
    MAP_LEXER("avs", QsciLexerAVS)
    MAP_LEXER("bash", QsciLexerBash)
    MAP_LEXER("batch", QsciLexerBatch)
    MAP_LEXER("cmake", QsciLexerCMake)
    MAP_LEXER("cpp", QsciLexerCPP)
    MAP_LEXER("css", QsciLexerCSS)
    MAP_LEXER("csharp", QsciLexerCSharp)
    MAP_LEXER("coffeescript", QsciLexerCoffeeScript)
    MAP_LEXER("d", QsciLexerD)
    MAP_LEXER("diff", QsciLexerDiff)
    MAP_LEXER("edifact", QsciLexerEDIFACT)
    MAP_LEXER("fortran", QsciLexerFortran)
    MAP_LEXER("fortran77", QsciLexerFortran77)
    MAP_LEXER("html", QsciLexerHTML)
    MAP_LEXER("idl", QsciLexerIDL)
    MAP_LEXER("json", QsciLexerJSON)
    MAP_LEXER("java", QsciLexerJava)
    MAP_LEXER("javascript", QsciLexerJavaScript)
    MAP_LEXER("lua", QsciLexerLua)
    MAP_LEXER("makefile", QsciLexerMakefile)
    MAP_LEXER("markdown", QsciLexerMarkdown)
    MAP_LEXER("matlab", QsciLexerMatlab)
    MAP_LEXER("octave", QsciLexerOctave)
    MAP_LEXER("po", QsciLexerPO)
    MAP_LEXER("pov", QsciLexerPOV)
    MAP_LEXER("pascal", QsciLexerPascal)
    MAP_LEXER("perl", QsciLexerPerl)
    MAP_LEXER("postscript", QsciLexerPostScript)
    MAP_LEXER("properties", QsciLexerProperties)
    MAP_LEXER("python", QsciLexerPython)
    MAP_LEXER("qml", QsciLexerJavaScript)
    MAP_LEXER("ruby", QsciLexerRuby)
    MAP_LEXER("sql", QsciLexerSQL)
    MAP_LEXER("spice", QsciLexerSpice)
    MAP_LEXER("tcl", QsciLexerTCL)
    MAP_LEXER("tex", QsciLexerTeX)
    MAP_LEXER("vhdl", QsciLexerVHDL)
    MAP_LEXER("verilog", QsciLexerVerilog)
    MAP_LEXER("xml", QsciLexerXML)
    MAP_LEXER("yaml", QsciLexerYAML)
    MAP_LEXER_END

    if (lexer) setLexer(lexer);

    setReadOnly(!o.editable);
    setTabWidth(o.tab_width);

    // theme

    setAStyle(QsciScintillaBase::STYLE_DEFAULT, o.text_col, o.background_col, o.fontSize, o.fontFace.toUtf8().data(), o.fontBold); // set global default style
    SendScintilla(QsciScintillaBase::SCI_SETCARETFORE,(unsigned long)QColor(Qt::black).rgb());
    SendScintilla(QsciScintillaBase::SCI_STYLECLEARALL); // set all styles

    if(o.wrapWord)
        SendScintilla(QsciScintillaBase::SCI_SETWRAPMODE, QsciScintillaBase::SC_WRAP_WORD);
    else
        SendScintilla(QsciScintillaBase::SCI_SETWRAPMODE, QsciScintillaBase::SC_WRAP_NONE);

    if(o.lineNumbers)
    {
        SendScintilla(QsciScintillaBase::SCI_SETMARGINWIDTHN, (unsigned long)0, (long)48);
        setAStyle(QsciScintillaBase::STYLE_LINENUMBER, (unsigned long)QColor(Qt::white).rgb(), (long)QColor(Qt::darkGray).rgb());
    }
    SendScintilla(QsciScintillaBase::SCI_SETMARGINWIDTHN, (unsigned long)1, (long)0);
    SendScintilla(QsciScintillaBase::SCI_SETSELBACK,(unsigned long)1,(long)o.selection_col.rgb()); // selection color

    SendScintilla(QsciScintillaBase::SCI_INDICSETSTYLE, (unsigned long)20, (long)QsciScintillaBase::INDIC_STRAIGHTBOX);
    SendScintilla(QsciScintillaBase::SCI_INDICSETALPHA, (unsigned long)20, (long)160);
    SendScintilla(QsciScintillaBase::SCI_INDICSETFORE, (unsigned long)20, (long)o.selection_col.rgb());

    QString ss1, sep1, ss2, sep2;
    for(auto kw : o.userKeywords)
    {
        if (kw.keywordType == 1 || o.lang == "python")
        {
            ss1 += sep1 + kw.keyword;
            sep1 = " ";
        }
        else
        {
            ss2 += sep2 + kw.keyword;
            sep2 = " ";
        }
    }

    if(o.lang == "none")
    {
        for(int i=0;i<8;i++)
            SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)i, "");
    }
    if(o.lang == "lua")
    {
        setFolding(QsciScintilla::BoxedTreeFoldStyle);
        setAStyle(SCE_LUA_COMMENT, o.comment_col, o.background_col);
        setAStyle(SCE_LUA_COMMENTLINE, o.comment_col, o.background_col);
        setAStyle(SCE_LUA_COMMENTDOC, o.comment_col, o.background_col);
        setAStyle(SCE_LUA_NUMBER, o.number_col, o.background_col);
        setAStyle(SCE_LUA_STRING, o.string_col, o.background_col);
        setAStyle(SCE_LUA_LITERALSTRING, o.string_col, o.background_col);
        setAStyle(SCE_LUA_CHARACTER, o.character_col, o.background_col);
        setAStyle(SCE_LUA_OPERATOR, o.operator_col, o.background_col);
        setAStyle(SCE_LUA_PREPROCESSOR, o.preprocessor_col, o.background_col);
        setAStyle(SCE_LUA_WORD, o.keyword3_col, o.background_col);
        setAStyle(SCE_LUA_WORD4, o.keyword4_col, o.background_col);
        setAStyle(SCE_LUA_IDENTIFIER, o.identifier_col, o.background_col);
        setAStyle(SCE_LUA_WORD2, o.keyword1_col, o.background_col);
        setAStyle(SCE_LUA_WORD3, o.keyword2_col, o.background_col);
        setAStyle(SCE_LUA_WORD7, o.keyword1_col, o.background_col);
        setAStyle(SCE_LUA_WORD8, o.keyword2_col, o.background_col);
        SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)6, ss1.toUtf8().data());
        SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)7, ss2.toUtf8().data());
    }
    if(o.lang == "python")
    {
        setFolding(QsciScintilla::BoxedTreeFoldStyle);
        setAStyle(SCE_P_COMMENTLINE, o.comment_col, o.background_col);
        setAStyle(SCE_P_COMMENTBLOCK, o.comment_col, o.background_col);
        setAStyle(SCE_P_TRIPLE, o.comment_col, o.background_col);
        setAStyle(SCE_P_TRIPLEDOUBLE, o.comment_col, o.background_col);
        setAStyle(SCE_P_NUMBER, o.number_col, o.background_col);
        setAStyle(SCE_P_STRING, o.string_col, o.background_col);
        setAStyle(SCE_P_STRINGEOL, o.string_col, o.background_col);
        setAStyle(SCE_P_CHARACTER, o.character_col, o.background_col);
        setAStyle(SCE_P_OPERATOR, o.operator_col, o.background_col);
        setAStyle(SCE_P_WORD, o.keyword3_col, o.background_col); // Python keywords
        //setAStyle(SCE_P_IDENTIFIER, o.keyword4_col, o.background_col); // obj & variable
        setAStyle(SCE_P_DEFNAME, o.keyword1_col, o.background_col); // func
        setAStyle(SCE_P_WORD2, o.keyword1_col, o.background_col); // ?? None
        setAStyle(SCE_P_CLASSNAME, o.keyword1_col, o.background_col); // ?? None
        setAStyle(SCE_P_DECORATOR, o.keyword2_col, o.background_col); // ?? None

        // according to https://www.scintilla.org/ScintillaDoc.html#SCI_SETKEYWORDS
        // the keyword set is dependent on the Lexer used.
        // according to LexPython.cpp only two keyword sets are used (0,1)
        // using 0 breaks syntax highlighting of everything
        // using 1 doesn't work at all...
        // for a workaround, change QScintilla/lexers/LexPython.cpp:418 to:
        //       if (!IsAWordChar(sc.ch)) {
        SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)1, (ss1+" "+ss2).toUtf8().data());
    }
    if(o.lang == "json")
    {
        setFolding(QsciScintilla::BoxedTreeFoldStyle);
        setAStyle(SCE_JSON_ERROR, o.comment_col, o.background_col);
        setAStyle(SCE_JSON_LINECOMMENT, o.comment_col, o.background_col);
        setAStyle(SCE_JSON_BLOCKCOMMENT, o.comment_col, o.background_col);
        setAStyle(SCE_JSON_NUMBER, o.keyword3_col, o.background_col);
        setAStyle(SCE_JSON_STRING, o.keyword3_col, o.background_col);
        setAStyle(SCE_JSON_STRINGEOL, o.keyword3_col, o.background_col);
        setAStyle(SCE_JSON_PROPERTYNAME, o.keyword4_col, o.background_col);
        setAStyle(SCE_JSON_KEYWORD, o.keyword3_col, o.background_col);
        setAStyle(SCE_JSON_LDKEYWORD, o.keyword3_col, o.background_col);
    }
    if(o.lang == "qml" || o.lang == "javascript" || o.lang == "cpp" || o.lang == "c")
    {
        setFolding(QsciScintilla::BoxedTreeFoldStyle);
        setAStyle(SCE_C_COMMENT, o.comment_col, o.background_col);
        setAStyle(SCE_C_COMMENTDOC, o.comment_col, o.background_col);
        setAStyle(SCE_C_COMMENTLINEDOC, o.comment_col, o.background_col);
        setAStyle(SCE_C_COMMENTDOCKEYWORD, o.comment_col, o.background_col);
        setAStyle(SCE_C_PREPROCESSOR, o.keyword1_col, o.background_col);
        setAStyle(SCE_C_PREPROCESSORCOMMENT, o.keyword1_col, o.background_col);
        setAStyle(SCE_C_PREPROCESSORCOMMENTDOC, o.keyword1_col, o.background_col);
        setAStyle(SCE_C_OPERATOR, o.keyword1_col, o.background_col);
        setAStyle(SCE_C_NUMBER, o.number_col, o.background_col);
        setAStyle(SCE_C_STRING, o.string_col, o.background_col);
        setAStyle(SCE_C_ESCAPESEQUENCE, o.string_col, o.background_col);
        setAStyle(SCE_C_HASHQUOTEDSTRING, o.string_col, o.background_col);
        setAStyle(SCE_C_STRINGRAW, o.string_col, o.background_col);
        setAStyle(SCE_C_STRINGEOL, o.string_col, o.background_col);
        setAStyle(SCE_C_CHARACTER, o.string_col, o.background_col);
        setAStyle(SCE_C_USERLITERAL, o.string_col, o.background_col);
        //setAStyle(SCE_C_IDENTIFIER, o.keyword4_col, o.background_col);
        //setAStyle(SCE_C_WORD, o.keyword3_col, o.background_col);
        setAStyle(SCE_C_GLOBALCLASS, o.keyword2_col, o.background_col);
    }
    if(o.lang == "xml")
    {
        setFolding(QsciScintilla::BoxedTreeFoldStyle);
        setAStyle(SCE_H_TAG, o.keyword1_col, o.background_col);
        setAStyle(SCE_H_TAGUNKNOWN, o.keyword1_col, o.background_col);
        setAStyle(SCE_H_ATTRIBUTE, o.keyword2_col, o.background_col);
        setAStyle(SCE_H_ATTRIBUTEUNKNOWN, o.keyword2_col, o.background_col);
        setAStyle(SCE_H_NUMBER, o.number_col, o.background_col);
        setAStyle(SCE_H_DOUBLESTRING, o.string_col, o.background_col);
        setAStyle(SCE_H_SINGLESTRING, o.string_col, o.background_col);
        setAStyle(SCE_H_OTHER, o.keyword4_col, o.background_col);
        setAStyle(SCE_H_COMMENT, o.comment_col, o.background_col);
        setAStyle(SCE_H_ENTITY, o.keyword3_col, o.background_col);
    }

    SendScintilla(QsciScintillaBase::SCI_INDICSETSTYLE,(unsigned long)20,(long)QsciScintillaBase::INDIC_STRAIGHTBOX);
    SendScintilla(QsciScintillaBase::SCI_INDICSETALPHA,(unsigned long)20,(long)160);
    SendScintilla(QsciScintillaBase::SCI_INDICSETFORE,(unsigned long)20,(long)o.selection_col.rgb());

#if 0
    SendScintilla(QsciScintillaBase::SCI_STYLESETHOTSPOT, SCE_LUA_WORD2, 1);
    setHotspotUnderline(true);
#endif
}

void Editor::onUpdateUi(int updated)
{   // highlight all occurences of selected text:
    SendScintilla(QsciScintillaBase::SCI_SETINDICATORCURRENT, (int)20);

    int totTextLength = SendScintilla(QsciScintillaBase::SCI_GETLENGTH);
    SendScintilla(QsciScintillaBase::SCI_INDICATORCLEARRANGE, (unsigned long)0, (long)totTextLength);

    int txtL = SendScintilla(QsciScintillaBase::SCI_GETSELTEXT, (unsigned long)0, (long)0) - 1;
    if (txtL >= 1)
    {
        int selStart = SendScintilla(QsciScintillaBase::SCI_GETSELECTIONSTART);

        char* txt = new char[txtL + 1];
        SendScintilla(QsciScintillaBase::SCI_GETSELTEXT, (unsigned long)0, txt);

        SendScintilla(QsciScintillaBase::SCI_SETSEARCHFLAGS, QsciScintillaBase::SCFIND_MATCHCASE | QsciScintillaBase::SCFIND_WHOLEWORD);
        SendScintilla(QsciScintillaBase::SCI_SETTARGETSTART, (int)0);
        SendScintilla(QsciScintillaBase::SCI_SETTARGETEND, (int)totTextLength);

        int p = SendScintilla(QsciScintillaBase::SCI_SEARCHINTARGET, (unsigned long)txtL, txt);
        while (p != -1)
        {
            if (p != selStart)
                SendScintilla(QsciScintillaBase::SCI_INDICATORFILLRANGE, (unsigned long)p, (long)strlen(txt));
            SendScintilla(QsciScintillaBase::SCI_SETTARGETSTART, (int)p + 1);
            SendScintilla(QsciScintillaBase::SCI_SETTARGETEND, (int)totTextLength);
            p = SendScintilla(QsciScintillaBase::SCI_SEARCHINTARGET, (unsigned long)txtL, txt);
        }
        delete[] txt;
    }
}

static QString stripQuotes(const QString &s)
{
    for(const char c : {'\'', '"'})
    {
        if(s.front() == c && s.back() == c)
            return s.mid(1, s.size() - 2);
    }
    return s;
}

void Editor::contextMenuEvent(QContextMenuEvent *event)
{
    QString tok = tokenAtPosition(positionFromPoint(event->pos()));
    QString tok0 = stripQuotes(tokenAtPosition2(positionFromPoint(event->pos())));

    QMenu *menu = createStandardContextMenu();

    for(QString tok1 : QStringList{tok0, tok})
    {
        QString fp = opts.resolveScriptFilePath(tok1);
        if(fp != "")
        {
            menu->addSeparator();
            connect(menu->addAction(QStringLiteral("Open '%1'...").arg(tok1)), &QAction::triggered, [=] {
                dialog->openExternalFile(fp);
            });
            break;
        }
    }

    for(const auto &k : opts.userKeywords)
    {
        if(k.keyword == tok)
        {
            QUrl refUrl = apiReferenceForSymbol(tok);
            if(refUrl.isEmpty()) continue;
            menu->addSeparator();
            connect(menu->addAction(QStringLiteral("Open reference for %1...").arg(tok)), &QAction::triggered, [=] {
#if 1
                dialog->openURL(refUrl.url());
#elif 0
                QDesktopServices::openUrl(refUrl);
#else
                dialog->showHelp(refUrl);
#endif
            });
        }
    }

    if(event->modifiers() & Qt::ShiftModifier && event->modifiers() & Qt::ControlModifier)
    {
        menu->addSeparator();
        QStringList info;
        int pos = positionFromPoint(event->pos());
        info << QString("Position: %1").arg(pos);
        info << QString("Token0: %1 -> %2").arg(tok0, opts.resolveScriptFilePath(tok0));
        info << QString("Token: %1 -> %2").arg(tok, opts.resolveScriptFilePath(tok));
        for(const QString &s : info)
        {
            QAction *a = new QAction(s);
            a->setEnabled(false);
            menu->addAction(a);
        }
    }

    if(opts.clearable)
    {
        menu->addSeparator();
        connect(menu->addAction("Clear contents"), &QAction::triggered, [=] {
            setText("", 0);
        });
    }

    menu->exec(event->globalPos());
    delete menu;
}

QString Editor::tokenAtPosition(int pos)
{
    QString txt{text()};
    auto isID = [] (const QChar c) { return c.isLetterOrNumber() || c == '_' || c == '.'; };
    int id = isID(txt.at(pos));
    if(!id) return {};
    int start = pos, end = pos, newid = id;
    while(start > 0)
    {
        newid = isID(txt.at(start - 1));
        if(newid == id) start--;
        else break;
    }
    while(end < txt.length()-1)
    {
        newid = isID(txt.at(end + 1));
        if(newid == id) end++;
        else break;
    }
    end++;
    // sanitize bounds:
    if(start < 0) start = 0;
    if(end > txt.length()) end = txt.length();
    return text(start, end);
}

QString Editor::tokenAtPosition2(int pos)
{
    int style = SendScintilla(SCI_GETSTYLEAT, (long)pos, (long)0);
    int start = pos, end = pos, newstyle = style;
    while(start > 0)
    {
        newstyle = SendScintilla(SCI_GETSTYLEAT, (long)start - 1, (long)0);
        if(newstyle == style) start--;
        else break;
    }
    int length = SendScintilla(SCI_GETTEXTLENGTH, (long)0, (long)0);
    while(end < (length-1))
    {
        newstyle = SendScintilla(SCI_GETSTYLEAT, (long)end + 1, (long)0);
        if(newstyle == style) end++;
        else break;
    }
    end++;

    // sanitize bounds:
    if(start < 0) start = 0;
    if(end > length) end = length;

    char *buf = new char[end - start + 1];
    SendScintilla(SCI_GETTEXTRANGE, (long)start, (long)end, buf);
    QString txt(buf);
    delete[] buf;
    return txt;
}

int Editor::positionFromPoint(const QPoint &p)
{
    return SendScintilla(SCI_POSITIONFROMPOINT, (long)p.x(), (long)p.y());
}

QString Editor::tokenAt(const QPoint &p)
{
    return tokenAtPosition(positionFromPoint(p));
}

void Editor::setText(const char* txt, int insertMode)
{
    if (insertMode == 0)
        QsciScintilla::setText(txt);
    else
        append(txt);
    bool ro = isReadOnly();
    SendScintilla(QsciScintillaBase::SCI_SETREADONLY, (int)0);

    int lines = SendScintilla(QsciScintillaBase::SCI_GETLINECOUNT);
    if ( (lines > opts.maxLines)&&(opts.maxLines!=0) )
    { // we have to remove lines-_maxLines lines!
        SendScintilla(QsciScintillaBase::SCI_SETSELECTIONSTART, (int)0);
        SendScintilla(QsciScintillaBase::SCI_SETSELECTIONEND, (int)SendScintilla(QsciScintillaBase::SCI_POSITIONFROMLINE, (int)lines - opts.maxLines));
        SendScintilla(QsciScintillaBase::SCI_CLEAR);
    }
    if (insertMode != 0)
        SendScintilla(QsciScintillaBase::SCI_GOTOPOS, (int)SendScintilla(QsciScintillaBase::SCI_POSITIONFROMLINE, (int)SendScintilla(QsciScintillaBase::SCI_GETLINECOUNT) - 1)); // set the cursor and move the view into position
    if (ro)
        SendScintilla(QsciScintillaBase::SCI_SETREADONLY, (int)1);
}

void Editor::setAStyle(int style,QColor fore,QColor back,int size,const char *face,bool bold)
{
    SendScintilla(QsciScintillaBase::SCI_STYLESETFORE,(unsigned long)style,(long)fore.rgb());
    SendScintilla(QsciScintillaBase::SCI_STYLESETBACK,(unsigned long)style,(long)back.rgb());
    if (size>=1)
        SendScintilla(QsciScintillaBase::SCI_STYLESETSIZE,(unsigned long)style,(long)size);
    if (face && strcmp(face, ""))
    {
        SendScintilla(QsciScintillaBase::SCI_STYLESETFONT,(unsigned long)style,face);
        SendScintilla(QsciScintillaBase::SCI_STYLESETBOLD,(unsigned long)style,bold);
    }
}

void Editor::onCharAdded(int charAdded)
{
    auto scintilla_ = this;
    if (scintilla_->SendScintilla(QsciScintillaBase::SCI_AUTOCACTIVE)!=0)
    { // Autocomplete is active
        if (charAdded=='(')
            scintilla_->SendScintilla(QsciScintillaBase::SCI_AUTOCCANCEL);
    }
    if (scintilla_->SendScintilla(QsciScintillaBase::SCI_AUTOCACTIVE)==0)
    { // Autocomplete is not active
        if (scintilla_->SendScintilla(QsciScintillaBase::SCI_CALLTIPACTIVE)!=0)
        { // CallTip is active
        }
        else
        { // Calltip is not active
            if ( (charAdded=='(')||(charAdded==',') )
            { // Do we need to activate a calltip?
                int bufsz=scintilla_->SendScintilla(QsciScintillaBase::SCI_GETCURLINE,(unsigned long)0,(char*)nullptr);
                char *linebuf=new char[bufsz+1];
                int current=scintilla_->SendScintilla(QsciScintillaBase::SCI_GETCURLINE,(unsigned long)bufsz,linebuf);
                linebuf[current]='\0';
                int pos=scintilla_->SendScintilla(QsciScintillaBase::SCI_GETCURRENTPOS);
                std::string line(linebuf);
                delete[] linebuf;
                // 1. Find '('. Not perfect, will also detect e.g. "(" or similar
                int cnt=0;
                int pahr=-1;
                for (pahr=current-1;pahr>0;pahr--)
                {
                    if (line[pahr]==')')
                        cnt--;
                    if (line[pahr]=='(')
                    {
                        cnt++;
                        if (cnt==1)
                            break;
                    }
                }
                if ( (cnt==1)&&(pahr>0) )
                { // 2. Get word
                    int spaceCnt=0;
                    int startword=pahr-1;
                    int endword=startword;
                    while ((startword>=0)&&(isalpha(line[startword])||isdigit(line[startword])||(line[startword]=='_')||(line[startword]=='.')||((line[startword]==' ')&&(spaceCnt>=0)) ))
                    {
                        if (line[startword]==' ')
                        {
                            if ( (spaceCnt==0)&&(endword!=startword) )
                                break;
                            spaceCnt++;
                            endword--;
                        }
                        else
                        {
                            if (spaceCnt>0)
                                spaceCnt=-spaceCnt;
                        }
                        startword--;
                    }
                    std::string s;
                    if (startword!=endword)
                    {
                        s.assign(line.begin()+startword+1,line.begin()+endword+1);
                        s=getCallTip(QString::fromStdString(s)).toStdString();
                    }
                    if (s!="")
                    {
                        // tabs and window scroll are problematic : pos-=line.size()+startword;
                        setAStyle(QsciScintillaBase::STYLE_CALLTIP,Qt::black,Qt::white,opts.fontSize,opts.fontFace.toUtf8().data(),opts.fontBold);
                        scintilla_->SendScintilla(QsciScintillaBase::SCI_CALLTIPUSESTYLE,(int)0);

                        int cursorPosInPixelsFromLeftWindowBorder=scintilla_->SendScintilla(QsciScintillaBase::SCI_POINTXFROMPOSITION,(int)0,(unsigned long)pos);
                        int callTipWidthInPixels=scintilla_->SendScintilla(QsciScintillaBase::SCI_TEXTWIDTH,QsciScintillaBase::STYLE_CALLTIP,s.c_str());
                        int charWidthInPixels=scintilla_->SendScintilla(QsciScintillaBase::SCI_TEXTWIDTH,QsciScintillaBase::STYLE_DEFAULT,"0");
                        int callTipWidthInChars=callTipWidthInPixels/charWidthInPixels;
                        int cursorPosInCharsFromLeftWindowBorder=-5+cursorPosInPixelsFromLeftWindowBorder/charWidthInPixels; // 5 is the width in chars of the left border (line counting)
                        int cursorPosInCharsFromLeftBorder=scintilla_->SendScintilla(QsciScintillaBase::SCI_GETCOLUMN,(int)pos);
                        unsigned off=-std::min(cursorPosInCharsFromLeftWindowBorder,cursorPosInCharsFromLeftBorder);
                        if (callTipWidthInChars<std::min(cursorPosInCharsFromLeftWindowBorder,cursorPosInCharsFromLeftBorder))
                            off=-callTipWidthInChars;

                        scintilla_->SendScintilla(QsciScintillaBase::SCI_CALLTIPSHOW,(unsigned long)pos+off,s.c_str());
                    }
                }
            }
            else
            { // Do we need to activate autocomplete?
                int p=-1+scintilla_->SendScintilla(QsciScintillaBase::SCI_GETCURRENTPOS);
                if (p>=2)
                {
                    int bufsz=scintilla_->SendScintilla(QsciScintillaBase::SCI_GETCURLINE,(unsigned long)0,(char*)nullptr);
                    char *linebuf=new char[bufsz+1];
                    int current=scintilla_->SendScintilla(QsciScintillaBase::SCI_GETCURLINE,(unsigned long)bufsz,linebuf);
                    linebuf[current]='\0';
                    int pos=scintilla_->SendScintilla(QsciScintillaBase::SCI_GETCURRENTPOS);
                    std::string line(linebuf);
                    delete[] linebuf;
                    int ind=(int)line.size()-1;
                    int cnt=0;
                    std::string theWord;
                    while ((ind>=0)&&(isalpha(line[ind])||isdigit(line[ind])||(line[ind]=='_')||(line[ind]=='.') ))
                    {
                        theWord=line[ind]+theWord;
                        ind--;
                        cnt++;
                    }
                    if (theWord.size()>=3)
                    {
                        std::string autoCompletionList;
                        std::vector<std::string> t;
                        std::map<std::string, bool> map;

                        bool hasDot = (theWord.find('.') != std::string::npos);

                        for (size_t i = 0; i < opts.userKeywords.size(); i++)
                        {
                            if ((opts.userKeywords[i].autocomplete) && (opts.userKeywords[i].keyword.size() >= theWord.size()) && (opts.userKeywords[i].keyword.toStdString().compare(0, theWord.size(), theWord) == 0))
                            {
                                std::string n(opts.userKeywords[i].keyword.toStdString());
                                if (!hasDot)
                                {
                                    size_t dp = n.find('.');
                                    if (dp != std::string::npos)
                                        n.erase(n.begin() + dp, n.end()); // we only push the text up to the dot
                                }
                                std::map<std::string, bool>::iterator it = map.find(n);
                                if (it == map.end())
                                {
                                    map[n] = true;
                                    t.push_back(n);
                                }
                            }
                        }

                        std::sort(t.begin(), t.end());

                        for (size_t i = 0; i < t.size(); i++)
                        {
                            autoCompletionList += t[i];
                            if (i != t.size() - 1)
                                autoCompletionList += ' ';
                        }

                        if (autoCompletionList.size()!=0)
                        { // We need to activate autocomplete!
                            scintilla_->SendScintilla(QsciScintillaBase::SCI_AUTOCSETAUTOHIDE,(int)0);
                            scintilla_->SendScintilla(QsciScintillaBase::SCI_AUTOCSTOPS,(unsigned long)0," ()[]{}:;~`',*-+/?!@#$%^&|\\<>\"");
//                            scintilla_->SendScintilla(QsciScintillaBase::SCI_AUTOCSETMAXHEIGHT,(int)100); // it seems that SCI_AUTOCSETMAXHEIGHT and SCI_AUTOCSETMAXWIDTH are not implemented yet!
//                            scintilla_->SendScintilla(QsciScintillaBase::SCI_AUTOCSETMAXWIDTH,(int)500); // it seems that SCI_AUTOCSETMAXHEIGHT and SCI_AUTOCSETMAXWIDTH are not implemented yet!
                            scintilla_->SendScintilla(QsciScintillaBase::SCI_AUTOCSHOW,(unsigned long)cnt,&(autoCompletionList[0]));
                        }
                    }
                }
            }
        }
    }
}

void Editor::onModified(int, int, const char *, int, int, int, int, int, int, int)
{

}

void Editor::onTextChanged()
{
    if(!externalFile_.path.isEmpty())
        externalFile_.edited = true;
    dialog->toolBar()->updateButtons();
}

void Editor::onCursorPosChanged(int line, int index)
{
    dialog->toolBar()->updateButtons();
    dialog->updateCursorSelectionDisplay();
}

void Editor::onSelectionChanged()
{
    dialog->toolBar()->updateButtons();
    dialog->updateCursorSelectionDisplay();
}

void Editor::indentSelectedText()
{
    int lineFrom, indexFrom, lineTo, indexTo;
    getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);
    if(indexTo == 0) lineTo--;
    beginUndoAction();
    for(int line = lineFrom; line <= lineTo && line != -1; line++)
        indent(line);
    endUndoAction();
}

void Editor::unindentSelectedText()
{
    int lineFrom, indexFrom, lineTo, indexTo;
    getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);
    if(indexTo == 0) lineTo--;
    beginUndoAction();
    for(int line = lineFrom; line <= lineTo && line != -1; line++)
        unindent(line);
    endUndoAction();
}

void Editor::openExternalFile(const QString &filePath)
{
    externalFile_.path = filePath;
    externalFile_.edited = false;

    if(filePath.isNull()) return;

    QFile f(filePath);
    if(f.open(QIODevice::ReadOnly))
    {
        QString content = f.readAll();
        f.close();

        bool obs = blockSignals(true);
        setText(content.toUtf8().data(), 0);
        blockSignals(obs);
    }
    QFileInfo i(filePath);
    setReadOnly(!i.isWritable());
    dialog->toolBar()->updateButtons();
}

void Editor::saveExternalFile()
{
    if(externalFile_.path.isEmpty()) return;

    QFile f(externalFile_.path);
    if(f.open(QIODevice::WriteOnly))
    {
        f.write(text().toUtf8());
        f.close();
        externalFile_.edited = false;
        dialog->toolBar()->updateButtons();
    }
    else QMessageBox::information(parentWidget(), "", QStringLiteral("Cannot write to file %1.").arg(externalFile_.path));
}

QString Editor::externalFile()
{
    return externalFile_.path;
}

bool Editor::needsSaving()
{
    if(externalFile_.path.isEmpty())
        return false;
    return externalFile_.edited;
}

bool Editor::canSave()
{
    return !externalFile_.path.isEmpty();
}

std::string Editor::divideString(const char* s) const
{
    size_t w=80;
    std::string t(s);
    std::string retVal;
    std::string off;
    while (t.size()>0)
    {
        if (t.size()>w)
        {
            size_t pos=0;
            while (t.size()>w)
            {
                pos=std::min<size_t>(t.find("=",pos+1),std::min<size_t>(t.find(",",pos+1),t.find("(",pos+1)));
                if (pos!=std::string::npos)
                {
                    if (pos>=w)
                    {
                        retVal+=off+t.substr(0,pos+1);
                        t.erase(0,pos+1);
                        if (off.size()==0)
                            off="\n    ";
                        break;
                    }
                }
                else
                {
                    retVal+=off+t;
                    t.clear();
                    break;
                }
            }
        }
        else
        {
            retVal+=off+t;
            t.clear();
        }
    }
    return(retVal);
}

QString Editor::getCallTip(const QString &txt)
{
    for(auto &k : opts.userKeywords)
        if(txt == k.keyword && !k.callTip.isEmpty())
        {
            std::string t(k.callTip.toStdString());
            std::vector<std::string> s;
            size_t pos=0;
            while ((pos=t.find("\n"))!=std::string::npos)
            {
                s.push_back(t.substr(0,pos));
                t.erase(0,pos+1);
            }
            s.push_back(t);
            std::string retStr;
            size_t w=80;
            for (size_t i=0;i<s.size();i++)
            {
                if (i!=0)
                    retStr+="\n";
                retStr+=divideString(s[i].c_str());
            }
            return QString(retStr.c_str());
        }
    return "";
}

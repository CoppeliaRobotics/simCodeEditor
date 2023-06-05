#include "editor.h"
#include "dialog.h"
#include "toolbar.h"
#include "UI.h"
#include <SciLexer.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexerjson.h>

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

void Editor::setEditorOptions(const EditorOptions &o)
{
    opts = o;
    if (o.lang == EditorOptions::Lang::Lua)
    {
        QsciLexerLua* lexer = new QsciLexerLua;
        setLexer(lexer);
    }
    if (o.lang == EditorOptions::Lang::Python)
    {
        QsciLexerPython* lexer = new QsciLexerPython;
        setLexer(lexer);
    }
    if (o.lang == EditorOptions::Lang::Json)
    {
        QsciLexerJSON* lexer = new QsciLexerJSON;
        setLexer(lexer);
    }

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

    if (o.lang != EditorOptions::Lang::Python)
    {
        setAStyle(SCE_LUA_WORD2, o.keyword1_col, o.background_col);
        setAStyle(SCE_LUA_WORD3, o.keyword2_col, o.background_col);
        setAStyle(SCE_LUA_WORD7, o.keyword1_col, o.background_col);
        setAStyle(SCE_LUA_WORD8, o.keyword2_col, o.background_col);
    }

    if (o.lang == EditorOptions::Lang::Lua)
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

        SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)1,
            // Keywords.
            "and break do else elseif end false for function if "
            "in local nil not or repeat return then true until "
            "while "
        );
        SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)2,
            // Basic functions.
            "rawget rawset require ipairs pairs next "
            "pcall xpcall getmetatable setmetatable "
            "string table math coroutine io os debug "
            "error assert "
            // CoppeliaRobotics/lua/functional.lua:
            "range map reduce filter foreach identity zip negate apply "
            "partial any all iter "
            "operator.add operator.sub operator.mul operator.div operator.mod "
            "operator.idiv operator.pow operator.land operator.lor operator.lxor "
            "operator.lshl operator.lshr operator.eq operator.neq operator.gt "
            "operator.ge operator.lt operator.le "
            "sum prod "
        );
        SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)3,
            // String, table and maths functions.
            "string.byte string.char string.dump string.find "
            "string.len string.lower string.rep string.sub "
            "string.upper string.format string.gfind string.gsub "
            "string.gmatch string.match string.pack string.packsize "
            "string.rep string.reverse string.unpack "
            "string.chars string.bytes "
            "table.concat table.foreach table.foreachi table.getn "
            "table.sort table.insert table.remove table.pack table.unpack "
            "table.move table.compare table.reversed "
            "math.abs math.acos math.asin math.atan math.atan2 "
            "math.ceil math.cos math.deg math.exp math.floor "
            "math.frexp math.ldexp math.log math.log10 math.max "
            "math.min math.mod math.pi math.rad math.random "
            "math.randomseed math.sin math.sqrt math.tan "
            // CoppeliaRobotics/lua/stringx.lua:
            "string.gsplit string.split string.startswith string.endswith "
            "string.trim string.ltrim string.rtrim "
            // CoppeliaRobotics/lua/tablex.lua:
            "table.index table.eq table.join table.slice table.tostring "
            "table.print table.find "
            // CoppeliaRobotics/lua/base16.lua:
            "base16.encode base16.decode "
            // CoppeliaRobotics/lua/base64.lua:
            "base64.encode base64.decode "
            // CoppeliaRobotics/lua/checkargs.lua:
            "checkarg.any checkarg.float checkarg.int checkarg.string "
            "checkarg.bool checkarg.table checkarg.func checkarg.handle "
            "checkarg.object checkarg.union "
            "checkargsEx checkargs "
            // CoppeliaRobotics/lua/grid.lua:
            "Grid "
            // CoppeliaRobotics/lua/matrix.lua:
            "Matrix Matrix3x3 Matrix4x4 Vector Vector3 Vector4 Vector7 "
            // CoppeliaRobotics/lua/var.lua:
            "getvar setvar getlocals "
        //);
        //SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)4,
            // Coroutine, I/O and system facilities.
            "coroutine.create coroutine.resume coroutine.status "
            "coroutine.wrap coroutine.yield "
            "io.close io.flush "
            "io.input io.lines io.open io.output io.read "
            "io.tmpfile io.type io.write io.stdin io.stdout "
            "io.stderr "
            "os.clock os.date os.difftime os.execute "
            "os.exit os.getenv os.remove os.rename os.setlocale "
            "os.time os.tmpname "
            "debug.debug debug.gethook debug.getinfo debug.getlocal "
            "debug.getmetatable debug.getregistry debug.getupvalue "
            "debug.getuservalue debug.sethook debug.setlocal "
            "debug.setmetatable debug.setupvalue debug.setuservalue "
            "debug.traceback debug.upvalueid debug.upvaluejoin "
        );
    }

    if (o.lang == EditorOptions::Lang::Python)
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
//        setAStyle(SCE_P_IDENTIFIER, o.keyword4_col, o.background_col); // obj & variable
        setAStyle(SCE_P_DEFNAME, o.keyword4_col, o.background_col); // func
        setAStyle(SCE_P_WORD2, o.keyword4_col, o.number_col); // ?? None
        setAStyle(SCE_P_CLASSNAME, o.identifier_col, o.string_col); // ?? None
//        setAStyle(SCE_P_DECORATOR, o.identifier_col, o.number_col); // ?? None
    }
    
    if (o.lang == EditorOptions::Lang::Json)
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

    SendScintilla(QsciScintillaBase::SCI_INDICSETSTYLE,(unsigned long)20,(long)QsciScintillaBase::INDIC_STRAIGHTBOX);
    SendScintilla(QsciScintillaBase::SCI_INDICSETALPHA,(unsigned long)20,(long)160);
    SendScintilla(QsciScintillaBase::SCI_INDICSETFORE,(unsigned long)20,(long)o.selection_col.rgb());

    QString ss1, sep1, ss2, sep2;
    for(auto kw : o.userKeywords)
    {
        if (kw.keywordType == 1)
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
    if (o.lang == EditorOptions::Lang::None)
    {
        for (int i=0;i<8;i++)
            SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)i, "");
    }

    SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)6, ss1.toUtf8().data());
    SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)7, ss2.toUtf8().data());

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
                            scintilla_->SendScintilla(QsciScintillaBase::SCI_AUTOCSTOPS,(unsigned long)0," ()[]{}:;~`',=*-+/?!@#$%^&|\\<>\"");
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

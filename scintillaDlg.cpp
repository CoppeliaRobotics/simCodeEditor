#include "scintillaDlg.h"
#include "UI.h"
#include "debug.h"
#include <algorithm>
#include <sstream>
#include <QCloseEvent>
#include <QGuiApplication>
#include <QShortcut>
#include <QLineEdit>
#include <QDebug>
#include <QMenu>
#include <QRegularExpression>
#include <QMessageBox>
#include "v_repLib.h"
#include <SciLexer.h>

CScintillaEdit::CScintillaEdit(CScintillaDlg *d)
    : dialog(d)
{
    QsciLexerLua* lexer=new QsciLexerLua;
    setLexer(lexer);
    SendScintilla(QsciScintillaBase::SCI_SETSTYLEBITS, 5);
    setTabWidth(4);
    SendScintilla(QsciScintillaBase::SCI_SETUSETABS, 0);

    connect(this, SIGNAL(SCN_CHARADDED(int)), this, SLOT(onCharAdded(int)));
    connect(this, SIGNAL(SCN_MODIFIED(int,int,const char*,int,int,int,int,int,int,int)), this, SLOT(onModified(int,int,const char*,int,int,int,int,int,int,int)));
    connect(this, &QsciScintilla::textChanged, this, &CScintillaEdit::onTextChanged);
    connect(this, &QsciScintilla::selectionChanged, this, &CScintillaEdit::onSelectionChanged);
    connect(this, &QsciScintilla::cursorPositionChanged, this, &CScintillaEdit::onCursorPosChanged);
}

void CScintillaEdit::setEditorOptions(const EditorOptions &o)
{
    opts = o;

    setReadOnly(!o.editable);
    setTabWidth(o.tab_width);

    // theme

    setAStyle(QsciScintillaBase::STYLE_DEFAULT, o.text_col, o.background_col, o.fontSize, o.fontFace.c_str()); // set global default style
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

    setAStyle(SCE_LUA_WORD2, o.keyword1_col, o.background_col);
    setAStyle(SCE_LUA_WORD3, o.keyword2_col, o.background_col);

    if(o.isLua)
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
    }

    SendScintilla(QsciScintillaBase::SCI_INDICSETSTYLE,(unsigned long)20,(long)QsciScintillaBase::INDIC_STRAIGHTBOX);
    SendScintilla(QsciScintillaBase::SCI_INDICSETALPHA,(unsigned long)20,(long)160);
    SendScintilla(QsciScintillaBase::SCI_INDICSETFORE,(unsigned long)20,(long)o.selection_col.rgb());

    std::stringstream ss;
    std::string sep = "";
    for(auto kw : o.userKeywords)
    {
        ss << sep << kw.keyword;
        sep = " ";
    }
    SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (unsigned long)1, ss.str().c_str());
}

void CScintillaEdit::contextMenuEvent(QContextMenuEvent *event)
{
    // extract file name at selection or cursor position:

    QString txt = selectedText();
    //QString word = wordAtPoint(event->pos());

    if(txt.isEmpty())
        txt = text(lineAt(event->pos()));

    QVector<QString> matches;
    QRegularExpression re("('([^']+)'|\"([^\"]+)\")");
    auto m = re.globalMatch(txt);
    while(m.hasNext())
    {
        auto match = m.next();
        if(match.hasMatch())
        {
            for(int i = 2; i <= 3; i++)
            {
                std::string fp = opts.resolveLuaFilePath(match.captured(2).toStdString());
                if(fp != "")
                {
                    QString m = QString::fromStdString(fp);
                    if(!matches.contains(m)) matches.append(m);
                }
            }
        }
    }

    QMenu *menu = createStandardContextMenu();
    menu->addSeparator();
    for(auto m : matches)
        connect(menu->addAction(QStringLiteral("Open '%1'...").arg(m)),
                &QAction::triggered, [this, m] {
            dialog->openExternalFile(m);
        });
    menu->exec(event->globalPos());
    delete menu;
}

void CScintillaEdit::setText(const char* txt, int insertMode)
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

void CScintillaEdit::setAStyle(int style,QColor fore,QColor back,int size,const char *face)
{
    SendScintilla(QsciScintillaBase::SCI_STYLESETFORE,(unsigned long)style,(long)fore.rgb());
    SendScintilla(QsciScintillaBase::SCI_STYLESETBACK,(unsigned long)style,(long)back.rgb());
    if (size>=1)
        SendScintilla(QsciScintillaBase::SCI_STYLESETSIZE,(unsigned long)style,(long)size);
    if (face)
        SendScintilla(QsciScintillaBase::SCI_STYLESETFONT,(unsigned long)style,face);
}

void CScintillaEdit::onCharAdded(int charAdded)
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

                char linebuf[1000];
                int current=scintilla_->SendScintilla(QsciScintillaBase::SCI_GETCURLINE,(unsigned long)999,linebuf);
                int pos=scintilla_->SendScintilla(QsciScintillaBase::SCI_GETCURRENTPOS);
                linebuf[current]='\0';
                std::string line(linebuf);
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
                        s=getCallTip(s.c_str());
                    }
                    if (s!="")
                    {
                        // tabs and window scroll are problematic : pos-=line.size()+startword;
                        setAStyle(QsciScintillaBase::STYLE_CALLTIP,Qt::black,Qt::white,opts.fontSize,opts.fontFace.c_str());
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
                    char linebuf[1000];
                    int current=scintilla_->SendScintilla(QsciScintillaBase::SCI_GETCURLINE,(unsigned long)999,linebuf);
                    linebuf[current]='\0';
                    std::string line(linebuf);
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
                            if ((opts.userKeywords[i].autocomplete) && (opts.userKeywords[i].keyword.size() >= theWord.size()) && (opts.userKeywords[i].keyword.compare(0, theWord.size(), theWord) == 0))
                            {
                                std::string n(opts.userKeywords[i].keyword);
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

void CScintillaEdit::onModified(int, int, const char *, int, int, int, int, int, int, int)
{

}

void CScintillaEdit::onTextChanged()
{
    if(!externalFile_.path.isEmpty())
        externalFile_.edited = true;
    dialog->toolBar()->updateButtons();
}

void CScintillaEdit::onCursorPosChanged(int line, int index)
{
    dialog->toolBar()->updateButtons();
    dialog->updateCursorSelectionDisplay();
}

void CScintillaEdit::onSelectionChanged()
{
    dialog->toolBar()->updateButtons();
    dialog->updateCursorSelectionDisplay();
}

void CScintillaEdit::indentSelectedText()
{
    int lineFrom, indexFrom, lineTo, indexTo;
    getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);
    if(indexTo == 0) lineTo--;
    beginUndoAction();
    for(int line = lineFrom; line <= lineTo && line != -1; line++)
        indent(line);
    endUndoAction();
}

void CScintillaEdit::unindentSelectedText()
{
    int lineFrom, indexFrom, lineTo, indexTo;
    getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);
    if(indexTo == 0) lineTo--;
    beginUndoAction();
    for(int line = lineFrom; line <= lineTo && line != -1; line++)
        unindent(line);
    endUndoAction();
}

void CScintillaEdit::setExternalFile(const QString &filePath)
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
    dialog->toolBar()->updateButtons();
}

void CScintillaEdit::saveExternalFile()
{
    if(externalFile_.path.isEmpty()) return;
    if(!externalFile_.edited) return;

    // TODO:
    //  - save contents to file
    //  - reset externalFile_.edited flag
}

QString CScintillaEdit::externalFile()
{
    return externalFile_.path;
}

bool CScintillaEdit::needsSaving()
{
    if(externalFile_.path.isEmpty())
        return false;
    return externalFile_.edited;
}

std::string CScintillaEdit::getCallTip(const char* txt)
{
    for (size_t i = 0; i < opts.userKeywords.size(); i++)
    {
        if ((strcmp(txt, opts.userKeywords[i].keyword.c_str()) == 0) && (opts.userKeywords[i].callTip.size() > 0))
            return(opts.userKeywords[i].callTip);
    }
    return("");
}

CScintillaDlg::CScintillaDlg(const EditorOptions &o, UI *ui, QWidget* pParent)
    : QDialog(pParent),
      opts(o),
      ui(ui)
{
    setAttribute(Qt::WA_DeleteOnClose);

    stacked_ = new QStackedWidget;
    activeEditor_ = new CScintillaEdit(this);
    editors_.insert("", activeEditor_);
    stacked_->addWidget(activeEditor_);

    toolBar_ = new ToolBar(o.canRestart,this);
    if (!o.toolBar)
        toolBar_->setVisible(false);
    searchPanel_ = new SearchAndReplacePanel(this);
    statusBar_ = new StatusBar(this);
    if (!o.statusBar)
        statusBar_->setVisible(false);

    if (o.searchable)
    {
        QShortcut* shortcut = new QShortcut(QKeySequence(tr("Ctrl+f", "Find")), this);
        connect(shortcut, &QShortcut::activated, searchPanel_, &SearchAndReplacePanel::show);
        connect(searchPanel_, &SearchAndReplacePanel::shown, [=]{
            searchPanel_->editFind->setEditText(activeEditor()->selectedText());
        });
    }

    QVBoxLayout *bl=new QVBoxLayout(this);
    bl->setContentsMargins(0,0,0,0);
    bl->setSpacing(0);
    setLayout(bl);
    bl->addWidget(toolBar_);
    bl->addWidget(stacked_);
    bl->addWidget(searchPanel_);
    bl->addWidget(statusBar_);

    connect(toolBar_->actReload, &QAction::triggered, this, &CScintillaDlg::reloadScript);
    connect(toolBar_->actShowSearchPanel, &QAction::toggled, searchPanel_, &SearchAndReplacePanel::setVisibility);
    connect(toolBar_->actUndo, &QAction::triggered, [this]() {
        activeEditor()->undo();
    });
    connect(toolBar_->actRedo, &QAction::triggered, [this]() {
        activeEditor()->redo();
    });
    connect(toolBar_->actIndent, &QAction::triggered, [this]() {
        activeEditor()->indentSelectedText();
    });
    connect(toolBar_->actUnindent, &QAction::triggered, [this]() {
        activeEditor()->unindentSelectedText();
    });
    connect(toolBar_->openFiles.actSave, &QAction::triggered, [this]() {
        auto editor = qvariant_cast<CScintillaEdit*>(toolBar_->openFiles.combo->currentData());
        if(!editor->externalFile().isEmpty())
            editor->saveExternalFile();
    });
    connect(toolBar_->openFiles.actClose, &QAction::triggered, [this]() {
        auto editor = qvariant_cast<CScintillaEdit*>(toolBar_->openFiles.combo->currentData());
        if(!editor->externalFile().isEmpty())
        {
            if(editor->needsSaving())
                if(QMessageBox::Yes != QMessageBox::question(this, "", QStringLiteral("File %1 has not been saved since last change.\n\nAre you sure you want to close it?").arg(editor->externalFile())))
                    return;
            closeExternalFile(editor);
        }
    });
    connect(toolBar_->openFiles.combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this]() {
        auto editor = qvariant_cast<CScintillaEdit*>(toolBar_->openFiles.combo->currentData());
        switchEditor(editor);
    });

    connect(searchPanel_, &SearchAndReplacePanel::shown, toolBar_, &ToolBar::updateButtons);
    connect(searchPanel_, &SearchAndReplacePanel::hidden, toolBar_, &ToolBar::updateButtons);

    toolBar_->updateButtons();
}

CScintillaDlg::~CScintillaDlg() 
{
    // scintilla_ is normally automatically destroyed!
}

void CScintillaDlg::setEditorOptions(const EditorOptions &o)
{
    opts = o;
    for(auto e : editors_)
        e->setEditorOptions(o);

    QWidget *parent = (QWidget *)simGetMainWindow(1);
    setWindowTitle(QString::fromStdString(o.windowTitle));
    statusBar()->setSizeGripEnabled(o.resizable);
    setModal(o.modal);
    Qt::WindowFlags flags = Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint; // | Qt::WindowStaysOnTopHint;
#ifdef MAC_VREP
    flags |= Qt::Tool;
#else
    flags |= Qt::Dialog;
#endif
    if(o.resizable) flags |= Qt::WindowMaximizeButtonHint;
    else flags |= Qt::MSWindowsFixedSizeDialogHint;
    if(o.modalSpecial || o.closeable) flags |= Qt::WindowCloseButtonHint;
    setWindowFlags(flags);
    resize(o.size);
    QRect frameGeom = parent->frameGeometry();
    if(o.placement == EditorOptions::Placement::Absolute)
    {
        move(o.pos);
    }
    else if(o.placement == EditorOptions::Placement::Relative)
    {
        move(
            o.pos.x() + o.pos.x() >= 0 ? frameGeom.left() : (frameGeom.right() - width()),
            o.pos.y() + o.pos.y() >= 0 ? frameGeom.top() : (frameGeom.bottom() - height())
        );
    }
    else if(o.placement == EditorOptions::Placement::Center)
    {
        move(
            frameGeom.left() + (frameGeom.width() - width()) / 2,
            frameGeom.top() + (frameGeom.height() - height()) / 2
        );
    }
    setAttribute(Qt::WA_ShowWithoutActivating, !o.activate);

    toolBar()->actShowSearchPanel->setEnabled(o.searchable);

    if(!o.modalSpecial)
    {
        show();
        if(o.activate)
        {
            raise();
            activateWindow();
        }
    }
#if defined(LIN_VREP) || defined(MAC_VREP)
    if(!o.resizable) setFixedSize(size());
#endif
}

CScintillaEdit * CScintillaDlg::activeEditor()
{
    return activeEditor_;
}

CScintillaEdit * CScintillaDlg::openExternalFile(const QString &filePath)
{
    CScintillaEdit *editor = editors_.value(filePath);

    if(!editor)
    {
        editor = new CScintillaEdit(this);
        editor->setEditorOptions(opts);
        editor->setExternalFile(filePath);
        editors_.insert(filePath, editor);
        stacked_->addWidget(editor);
    }

    switchEditor(editor);

    return editor;
}

void CScintillaDlg::closeExternalFile(const QString &filePath)
{
    auto editor = editors_.value(filePath);
    if(!editor) return;
    closeExternalFile(editor);
}

void CScintillaDlg::closeExternalFile(CScintillaEdit *editor)
{
    if(editor->externalFile().isNull()) return;
    stacked_->removeWidget(editor);
    editors_.remove(editor->externalFile());
    editor->deleteLater();
    //toolBar_->updateButtons();
    switchEditor(editors_.value(""));
}

void CScintillaDlg::switchEditor(CScintillaEdit *editor)
{
    if(!editor) return;

    stacked_->setCurrentWidget(editor);
    activeEditor_ = editor;
    toolBar_->updateButtons();
}

bool CScintillaDlg::containsUnsavedFiles()
{
    for(auto editor : editors_)
        if(editor->needsSaving())
            return true;
    return false;
}

void CScintillaDlg::setHandle(int handle)
{
    this->handle = handle;
}

void CScintillaDlg::setText(const QString &text)
{
    setText(text.toUtf8().data(), 0);
}

void CScintillaDlg::setText(const char* txt, int insertMode)
{
    editors_[""]->setText(txt, insertMode);
}

QString CScintillaDlg::text()
{
    return editors_[""]->text();
}

std::string CScintillaDlg::makeModal(int *positionAndSize)
{
    opts.modalSpecial = true;
    setModal(true);
    exec();
    if(positionAndSize != nullptr)
    {
        for (size_t i = 0; i < 4; i++)
            positionAndSize[i] = modalPosAndSize[i];
    }
    return(modalText.toStdString());
}

void CScintillaDlg::closeEvent(QCloseEvent *event)
{
    if(containsUnsavedFiles() && QMessageBox::Yes != QMessageBox::question(this, "", QStringLiteral("This window contains files which have been modified and not saved.\n\nAre you sure you want to close it?")))
    {
        event->ignore();
        return;
    }

    if(opts.modalSpecial)
    {
        modalText = editors_.value("")->text();
        modalPosAndSize[0] = x();
        modalPosAndSize[1] = y();
        modalPosAndSize[2] = width();
        modalPosAndSize[3] = height();
        QDialog::closeEvent(event);
    }
    else
    {
        event->ignore();
        ui->notifyEvent(handle, "closeEditor", QString::fromStdString(opts.onClose));
    }
}

void CScintillaDlg::reloadScript()
{
    ui->notifyEvent(handle, "restartScript", QString::fromStdString(opts.onClose));
}

void CScintillaDlg::updateCursorSelectionDisplay()
{
    int fromLine, fromIndex, toLine, toIndex;
    activeEditor()->getSelection(&fromLine, &fromIndex, &toLine, &toIndex);
    if(fromLine != -1)
    {
        statusBar_->setSelectionInfo(fromLine, fromIndex, toLine, toIndex);
        return;
    }
    activeEditor()->getCursorPosition(&fromLine, &fromIndex);
    statusBar_->setCursorInfo(fromLine, fromIndex);
}

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

ToolBar::ToolBar(bool canRestart,CScintillaDlg *parent)
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

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    addWidget(spacer);

    funcNav.label = new QLabel("Go to function: ");
    funcNav.combo = new QComboBox;
    funcNav.combo->addItem("functionNavigatorPlaceholder(...)");
    funcNav.widget = new QWidget;
    QHBoxLayout *l = new QHBoxLayout;
    l->setSpacing(0);
    l->setContentsMargins(0,0,0,0);
    funcNav.widget->setLayout(l);
    l->addWidget(funcNav.label);
    l->addWidget(funcNav.combo);
    actFuncNav = addWidget(funcNav.widget);
    actFuncNav->setVisible(false);

    ICON(save);
    addAction(openFiles.actSave = new QAction(QIcon(save), "Save current file"));
    openFiles.combo = new QComboBox;
    addWidget(openFiles.combo);
    ICON(close);
    addAction(openFiles.actClose = new QAction(QIcon(close), "Close current file"));
}

ToolBar::~ToolBar()
{
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
}

SearchAndReplacePanel::SearchAndReplacePanel(CScintillaDlg *parent)
    : QWidget(parent),
      parent(parent)
{
    QGridLayout *layout = new QGridLayout;
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setColumnStretch(1, 10);
    layout->addWidget(chkRegExp = new QCheckBox("Regular expression"), 1, 4);
    layout->addWidget(chkCaseSens = new QCheckBox("Case sensitive"), 2, 4);
    layout->addWidget(lblFind = new QLabel("Find:"), 1, 0);
    lblFind->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(editFind = new QComboBox, 1, 1, 1, 2);
    editFind->setEditable(true);
    editFind->setInsertPolicy(QComboBox::InsertAtTop);
    layout->addWidget(btnFind = new QPushButton("Find"), 1, 3);
    layout->addWidget(btnClose = new QPushButton, 1, 5);
    layout->addWidget(lblReplace = new QLabel("Replace with:"), 2, 0);
    lblReplace->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(editReplace = new QComboBox, 2, 1, 1, 2);
    editReplace->setEditable(true);
    editReplace->setInsertPolicy(QComboBox::InsertAtTop);
    layout->addWidget(btnReplace = new QPushButton("Replace"), 2, 3);
    setLayout(layout);
    btnClose->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    btnClose->setFlat(true);
    btnClose->setStyleSheet("margin-left: 5px; margin-right: 5px; font-size: 14pt;");
    connect(btnClose, &QPushButton::clicked, this, &SearchAndReplacePanel::hide);
    connect(btnFind, &QPushButton::clicked, this, &SearchAndReplacePanel::find);
    connect(btnReplace, &QPushButton::clicked, this, &SearchAndReplacePanel::replace);
    hide();
}

SearchAndReplacePanel::~SearchAndReplacePanel()
{
}

void SearchAndReplacePanel::setVisibility(bool v)
{
    QWidget::setVisible(v);
    if (v)
    {
        int line, index;
        parent->activeEditor()->getCursorPosition(&line, &index);
        parent->activeEditor()->ensureLineVisible(line);
        int txtL = parent->activeEditor()->SendScintilla(QsciScintillaBase::SCI_GETSELTEXT, (unsigned long)0, (long)0) - 1;
        if (txtL >= 1)
        {
            char* txt = new char[txtL + 1];
            parent->activeEditor()->SendScintilla(QsciScintillaBase::SCI_GETSELTEXT, (unsigned long)0, txt);
            editFind->setEditText(txt);
            editFind->lineEdit()->selectAll();
            delete[] txt;
        }
        editFind->setFocus();
    }
}

void SearchAndReplacePanel::show()
{
    QWidget::show();
    int line, index;
    parent->activeEditor()->getCursorPosition(&line, &index);
    parent->activeEditor()->ensureLineVisible(line);

    int txtL = parent->activeEditor()->SendScintilla(QsciScintillaBase::SCI_GETSELTEXT, (unsigned long)0, (long)0) - 1;
    if (txtL >= 1)
    {
        char* txt = new char[txtL + 1];
        parent->activeEditor()->SendScintilla(QsciScintillaBase::SCI_GETSELTEXT, (unsigned long)0, txt);
        editFind->setEditText(txt);
        editFind->lineEdit()->selectAll();
        delete[] txt;
    }
    editFind->setFocus();

    emit shown();
}

void SearchAndReplacePanel::hide()
{
    QWidget::hide();
    emit hidden();
}

void SearchAndReplacePanel::find()
{
    QsciScintilla *sci = parent->activeEditor();
    bool shift = QGuiApplication::keyboardModifiers() & Qt::ShiftModifier;
    // FIXME: reverse search does not work. bug in QScintilla?
    QString what = editFind->currentText();
    if(editFind->findText(what) == -1) editFind->addItem(what);
    if(!sci->findFirst(what, chkRegExp->isChecked(), chkCaseSens->isChecked(), false, false, !shift))
    {
        if(sci->findFirst(what, chkRegExp->isChecked(), chkCaseSens->isChecked(), false, true, !shift))
            parent->statusBar()->showMessage("Search reached end. Continuing from top.", 4000);
        else
            parent->statusBar()->showMessage("No occurrences found.", 4000);
    }
}

void SearchAndReplacePanel::replace()
{
    QString what = editReplace->currentText();
    if(editReplace->findText(what) == -1) editReplace->addItem(what);
    parent->activeEditor()->replace(what);
}

StatusBar::StatusBar(CScintillaDlg *parent)
    : QStatusBar(parent),
      parent(parent)
{
    addPermanentWidget(lblCursorPos = new QLabel("1:1"));
    lblCursorPos->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    lblCursorPos->setFixedWidth(120);
}

StatusBar::~StatusBar()
{
}

void StatusBar::setCursorInfo(int line, int index)
{
    lblCursorPos->setText(QString("%1:%2").arg(line + 1).arg(index + 1));
}

void StatusBar::setSelectionInfo(int fromLine, int fromIndex, int toLine, int toIndex)
{
    lblCursorPos->setText(QString("%1:%2-%3:%4 S").arg(fromLine + 1).arg(fromIndex + 1).arg(toLine + 1).arg(toIndex + 1));
}


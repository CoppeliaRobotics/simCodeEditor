#include "scintillaDlg.h"
#include "UI.h"
#include "debug.h"
#include <algorithm> 
#include <QCloseEvent>
#include <SciLexer.h>
#include <QGuiApplication>
#include <qshortcut.h>
#include <qlineedit.h>

CScintillaDlg::CScintillaDlg(bool toolbar, bool statusbar,bool canRestart,bool searchable, UI *ui, QWidget* pParent)
    : QDialog(pParent),
      ui(ui)
{
    setAttribute(Qt::WA_DeleteOnClose);

    scintilla_ = new QsciScintilla;
    toolBar_ = new ToolBar(canRestart,this);
    if (!toolbar)
        toolBar_->setVisible(false);
    searchPanel_ = new SearchAndReplacePanel(this);
    statusBar_ = new StatusBar(this);
    if (!statusbar)
        statusBar_->setVisible(false);

    if (searchable)
    {
        QShortcut* shortcut = new QShortcut(QKeySequence(tr("Ctrl+f", "Find")), this);
        connect(shortcut, &QShortcut::activated, searchPanel_, &SearchAndReplacePanel::show);
    }

    QVBoxLayout *bl=new QVBoxLayout(this);
    bl->setContentsMargins(0,0,0,0);
    bl->setSpacing(0);
    setLayout(bl);
    bl->addWidget(toolBar_);
    bl->addWidget(scintilla_);
    bl->addWidget(searchPanel_);
    bl->addWidget(statusBar_);

    QsciLexerLua* lexer=new QsciLexerLua;
    scintilla_->setLexer(lexer);
    scintilla_->SendScintilla(QsciScintillaBase::SCI_SETSTYLEBITS,(int)5);
    scintilla_->setTabWidth(4);
    scintilla_->SendScintilla(QsciScintillaBase::SCI_SETUSETABS,(int)0);

    connect(toolBar_->actReload, &QAction::triggered, this, &CScintillaDlg::reloadScript);
    connect(toolBar_->actShowSearchPanel, &QAction::toggled, searchPanel_, &SearchAndReplacePanel::setVisibility);
    connect(toolBar_->actUndo, &QAction::triggered, scintilla_, &QsciScintilla::undo);
    connect(toolBar_->actRedo, &QAction::triggered, scintilla_, &QsciScintilla::redo);
    connect(toolBar_->actUnindent, &QAction::triggered, this, &CScintillaDlg::unindent);
    connect(toolBar_->actIndent, &QAction::triggered, this, &CScintillaDlg::indent);

    connect(searchPanel_, &SearchAndReplacePanel::shown, toolBar_, &ToolBar::updateButtons);
    connect(searchPanel_, &SearchAndReplacePanel::hidden, toolBar_, &ToolBar::updateButtons);

    connect(scintilla_,SIGNAL(SCN_CHARADDED(int)),this,SLOT(charAdded(int)));
    connect(scintilla_,SIGNAL(SCN_MODIFIED(int,int,const char*,int,int,int,int,int,int,int)),this,SLOT(modified(int,int,const char*,int,int,int,int,int,int,int)));
    connect(scintilla_,&QsciScintilla::textChanged,this,&CScintillaDlg::textChanged);
    connect(scintilla_,&QsciScintilla::selectionChanged,this,&CScintillaDlg::selectionChanged);
    connect(scintilla_,&QsciScintilla::cursorPositionChanged,this, &CScintillaDlg::cursorPosChanged);
}

CScintillaDlg::~CScintillaDlg() 
{
    // scintilla_ is normally automatically destroyed!
}

void CScintillaDlg::setText(const char* txt, int insertMode)
{
    if (insertMode == 0)
        scintilla_->setText(txt);
    else
        scintilla_->append(txt);

    bool ro = scintilla_->isReadOnly();
    scintilla_->SendScintilla(QsciScintillaBase::SCI_SETREADONLY, (int)0);

    int lines = scintilla_->SendScintilla(QsciScintillaBase::SCI_GETLINECOUNT);
    if ( (lines > maxLines)&&(maxLines!=0) )
    { // we have to remove lines-_maxLines lines!
        scintilla_->SendScintilla(QsciScintillaBase::SCI_SETSELECTIONSTART, (int)0);
        scintilla_->SendScintilla(QsciScintillaBase::SCI_SETSELECTIONEND, (int)scintilla_->SendScintilla(QsciScintillaBase::SCI_POSITIONFROMLINE, (int)lines - maxLines));
        scintilla_->SendScintilla(QsciScintillaBase::SCI_CLEAR);
    }
    if (insertMode != 0)
        scintilla_->SendScintilla(QsciScintillaBase::SCI_GOTOPOS, (int)scintilla_->SendScintilla(QsciScintillaBase::SCI_POSITIONFROMLINE, (int)scintilla_->SendScintilla(QsciScintillaBase::SCI_GETLINECOUNT) - 1)); // set the cursor and move the view into position
    if (ro)
        scintilla_->SendScintilla(QsciScintillaBase::SCI_SETREADONLY, (int)1);
}

void CScintillaDlg::setHandle(int handle)
{
    this->handle = handle;
}

std::string CScintillaDlg::makeModal(int *positionAndSize)
{
    isModalSpecial = true;
    setModal(true);
    exec();
    if (positionAndSize != nullptr)
    {
        for (size_t i = 0; i < 4; i++)
            positionAndSize[i] = modalPosAndSize[i];
    }
    return(modalText.toStdString());
}

void CScintillaDlg::setAStyle(int style,QColor fore,QColor back,int size,const char *face)
{
    scintilla_->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE,(unsigned long)style,(long)fore.rgb());
    scintilla_->SendScintilla(QsciScintillaBase::SCI_STYLESETBACK,(unsigned long)style,(long)back.rgb());
    if (size>=1)
        scintilla_->SendScintilla(QsciScintillaBase::SCI_STYLESETSIZE,(unsigned long)style,(long)size);
    if (face)
        scintilla_->SendScintilla(QsciScintillaBase::SCI_STYLESETFONT,(unsigned long)style,face);
}

void CScintillaDlg::setTheme(bool modalSpecial,bool lineNumbers, int maxLines,bool isLua, const std::string& onClose,bool wrapWord,const std::string& theFont, int theFontSize, const std::vector<SScintillaUserKeyword>& theUserKeywords,QColor text_col, QColor background_col, QColor selection_col, QColor comment_col, QColor number_col, QColor string_col, QColor character_col, QColor operator_col, QColor identifier_col, QColor preprocessor_col, QColor keyword1_col, QColor keyword2_col, QColor keyword3_col, QColor keyword4_col)
{
    isModalSpecial = modalSpecial;
    font = theFont;
    fontSize = theFontSize;
    this->onClose = onClose.c_str();
    this->maxLines = maxLines;
    userKeywords.assign(theUserKeywords.begin(), theUserKeywords.end());
    setAStyle(QsciScintillaBase::STYLE_DEFAULT, text_col, background_col, fontSize, font.c_str()); // set global default style
    scintilla_->SendScintilla(QsciScintillaBase::SCI_SETCARETFORE,(unsigned long)QColor(Qt::black).rgb());
    scintilla_->SendScintilla(QsciScintillaBase::SCI_STYLECLEARALL); // set all styles

    if (wrapWord)
        scintilla_->SendScintilla(QsciScintillaBase::SCI_SETWRAPMODE, QsciScintillaBase::SC_WRAP_WORD);
    else
        scintilla_->SendScintilla(QsciScintillaBase::SCI_SETWRAPMODE, QsciScintillaBase::SC_WRAP_NONE);

    if (lineNumbers)
    {
        scintilla_->SendScintilla(QsciScintillaBase::SCI_SETMARGINWIDTHN, (unsigned long)0, (long)48);
        setAStyle(QsciScintillaBase::STYLE_LINENUMBER, (unsigned long)QColor(Qt::white).rgb(), (long)QColor(Qt::darkGray).rgb());
    }
    scintilla_->SendScintilla(QsciScintillaBase::SCI_SETMARGINWIDTHN, (unsigned long)1, (long)0);
    scintilla_->SendScintilla(QsciScintillaBase::SCI_SETSELBACK,(unsigned long)1,(long)selection_col.rgb()); // selection color

    setAStyle(SCE_LUA_WORD2, keyword1_col, background_col);
    setAStyle(SCE_LUA_WORD3, keyword2_col, background_col);

    if (isLua)
    {
        scintilla_->setFolding(QsciScintilla::BoxedTreeFoldStyle);
        setAStyle(SCE_LUA_COMMENT, comment_col, background_col);
        setAStyle(SCE_LUA_COMMENTLINE, comment_col, background_col);
        setAStyle(SCE_LUA_COMMENTDOC, comment_col, background_col);
        setAStyle(SCE_LUA_NUMBER, number_col, background_col);
        setAStyle(SCE_LUA_STRING, string_col, background_col);
        setAStyle(SCE_LUA_LITERALSTRING, string_col, background_col);
        setAStyle(SCE_LUA_CHARACTER, character_col, background_col);
        setAStyle(SCE_LUA_OPERATOR, operator_col, background_col);
        setAStyle(SCE_LUA_PREPROCESSOR, preprocessor_col, background_col);
        setAStyle(SCE_LUA_WORD, keyword3_col, background_col);
        setAStyle(SCE_LUA_WORD4, keyword4_col, background_col);
        setAStyle(SCE_LUA_IDENTIFIER, identifier_col, background_col);
    }

    scintilla_->SendScintilla(QsciScintillaBase::SCI_INDICSETSTYLE,(unsigned long)20,(long)QsciScintillaBase::INDIC_STRAIGHTBOX);
    scintilla_->SendScintilla(QsciScintillaBase::SCI_INDICSETALPHA,(unsigned long)20,(long)160);
    scintilla_->SendScintilla(QsciScintillaBase::SCI_INDICSETFORE,(unsigned long)20,(long)selection_col.rgb());
}

void CScintillaDlg::closeEvent(QCloseEvent *event)
{
    if(isModalSpecial)
    {
        modalText = scintilla_->text();
        modalPosAndSize[0] = x();
        modalPosAndSize[1] = y();
        modalPosAndSize[2] = width();
        modalPosAndSize[3] = height();
        QDialog::closeEvent(event);
    }
    else
    {
        event->ignore();
        ui->notifyEvent(handle,"closeEditor", onClose);
    }
}

std::string CScintillaDlg::getCallTip(const char* txt)
{
    for (size_t i = 0; i < userKeywords.size(); i++)
    {
        if ((strcmp(txt, userKeywords[i].keyword.c_str()) == 0) && (userKeywords[i].callTip.size() > 0))
            return(userKeywords[i].callTip);
    }
    return("");
}

void CScintillaDlg::charAdded(int charAdded)
{
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
                        setAStyle(QsciScintillaBase::STYLE_CALLTIP,Qt::black,Qt::white,fontSize,font.c_str());
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

                        for (size_t i = 0; i < userKeywords.size(); i++)
                        {
                            if ((userKeywords[i].autocomplete) && (userKeywords[i].keyword.size() >= theWord.size()) && (userKeywords[i].keyword.compare(0, theWord.size(), theWord) == 0))
                            {
                                std::string n(userKeywords[i].keyword);
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

void CScintillaDlg::modified(int,int,const char*,int,int,int,int,int,int,int)
{
}

void CScintillaDlg::textChanged()
{
    toolBar_->updateButtons();
}

void CScintillaDlg::cursorPosChanged(int line, int index)
{
    toolBar_->updateButtons();
    updateCursorSelectionDisplay();
}

void CScintillaDlg::selectionChanged()
{
    toolBar_->updateButtons();
    updateCursorSelectionDisplay();
}

void CScintillaDlg::reloadScript()
{
    ui->notifyEvent(handle, "restartScript", onClose);
}

void CScintillaDlg::indent()
{
    int lineFrom, indexFrom, lineTo, indexTo;
    scintilla_->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);
    if(indexTo == 0) lineTo--;
    scintilla_->beginUndoAction();
    for(int line = lineFrom; line <= lineTo && line != -1; line++)
        scintilla_->indent(line);
    scintilla_->endUndoAction();
}

void CScintillaDlg::unindent()
{
    int lineFrom, indexFrom, lineTo, indexTo;
    scintilla_->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);
    if(indexTo == 0) lineTo--;
    scintilla_->beginUndoAction();
    for(int line = lineFrom; line <= lineTo && line != -1; line++)
        scintilla_->unindent(line);
    scintilla_->endUndoAction();
}

void CScintillaDlg::updateCursorSelectionDisplay()
{
    int fromLine, fromIndex, toLine, toIndex;
    scintilla_->getSelection(&fromLine, &fromIndex, &toLine, &toIndex);
    if(fromLine != -1)
    {
        statusBar_->setSelectionInfo(fromLine, fromIndex, toLine, toIndex);
        return;
    }
    scintilla_->getCursorPosition(&fromLine, &fromIndex);
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
}

ToolBar::~ToolBar()
{
}

void ToolBar::updateButtons()
{
    actUndo->setEnabled(parent->scintilla()->isUndoAvailable());
    actRedo->setEnabled(parent->scintilla()->isRedoAvailable());

    int fromLine, fromIndex, toLine, toIndex;
    parent->scintilla()->getSelection(&fromLine, &fromIndex, &toLine, &toIndex);
    bool hasSel = fromLine != -1;
    actIndent->setEnabled(hasSel);
    actUnindent->setEnabled(hasSel);

    actShowSearchPanel->setChecked(parent->searchPanel()->isVisible());
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
        parent->scintilla()->getCursorPosition(&line, &index);
        parent->scintilla()->ensureLineVisible(line);
        int txtL = parent->scintilla()->SendScintilla(QsciScintillaBase::SCI_GETSELTEXT, (unsigned long)0, (long)0) - 1;
        if (txtL >= 1)
        {
            char* txt = new char[txtL + 1];
            parent->scintilla()->SendScintilla(QsciScintillaBase::SCI_GETSELTEXT, (unsigned long)0, txt);
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
    parent->scintilla()->getCursorPosition(&line, &index);
    parent->scintilla()->ensureLineVisible(line);

    int txtL = parent->scintilla()->SendScintilla(QsciScintillaBase::SCI_GETSELTEXT, (unsigned long)0, (long)0) - 1;
    if (txtL >= 1)
    {
        char* txt = new char[txtL + 1];
        parent->scintilla()->SendScintilla(QsciScintillaBase::SCI_GETSELTEXT, (unsigned long)0, txt);
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
    QsciScintilla *sci = parent->scintilla();
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
    parent->scintilla()->replace(what);
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


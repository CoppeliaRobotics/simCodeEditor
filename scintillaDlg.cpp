#include "scintillaDlg.h"
#include <algorithm> 
#include <QVBoxLayout>
#include <SciLexer.h>

#define VRGB(r,g,b) ((quint32)((quint8)(r)|((quint8)(g) << 8)|((quint8)(b) << 16)))

const int fontSize=8;
const char* theFont("Courier");

const unsigned int colBlack=VRGB(0,0,0);
const unsigned int colDarkGrey=VRGB(64,64,64);
const unsigned int colWhite=VRGB(255,255,255);
const unsigned int colBackground=VRGB(190,175,175);
const unsigned int colSelection=VRGB(128,128,255);
const unsigned int colComment=VRGB(0,140,0);
const unsigned int colNumber=VRGB(220,0,220);
const unsigned int colString=VRGB(255,255,0);
const unsigned int colCharacter=VRGB(255,255,0);
const unsigned int colOperator=VRGB(0,0,0);
const unsigned int colPreprocessor=VRGB(0,128,128);
const unsigned int colIdentifier=VRGB(64,64,64);
const unsigned int colWord=VRGB(0,0,255);
const unsigned int colWord2=VRGB(152,0,0);
const unsigned int colWord3=VRGB(220,80,20);
const unsigned int colWord4=VRGB(152,64,0);

struct SScintillaColors
{
    int iItem;
    unsigned int rgb;
};

const SScintillaColors syntaxColors[]=
{
    {SCE_LUA_COMMENT,colComment},
    {SCE_LUA_COMMENTLINE,colComment},
    {SCE_LUA_COMMENTDOC,colComment},
    {SCE_LUA_NUMBER,colNumber},
    {SCE_LUA_STRING,colString},
    {SCE_LUA_LITERALSTRING,colString},
    {SCE_LUA_CHARACTER,colCharacter},
    {SCE_LUA_OPERATOR,colOperator},
    {SCE_LUA_PREPROCESSOR,colPreprocessor},
    {SCE_LUA_WORD,colWord},
    {SCE_LUA_WORD2,colWord2},
    {SCE_LUA_WORD3,colWord3},
    {SCE_LUA_WORD4,colWord4},
    {SCE_LUA_IDENTIFIER,colIdentifier},
    {-1,0}
};

CScintillaDlg::CScintillaDlg(QWidget* pParent) : QDialog(pParent)
{
    closeRequest=false;
    setAttribute(Qt::WA_DeleteOnClose);
    _scintillaObject=new QsciScintilla;

    QVBoxLayout *bl=new QVBoxLayout(this);
    bl->setContentsMargins(0,0,0,0);
    setLayout(bl);
    bl->addWidget(_scintillaObject);

    QsciLexerLua* lexer=new QsciLexerLua;
    _scintillaObject->setLexer(lexer);
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_SETSTYLEBITS,(int)5);
    _scintillaObject->setTabWidth(4);
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_SETUSETABS,(int)0);
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_SETMARGINWIDTHN,(unsigned long)0,(long)48); // Line numbers
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_SETMARGINWIDTHN,(unsigned long)1,(long)0); // Symbols
    // Keywords:
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS,(unsigned long)1,"sim.getObjectHandle sim.getObjectName");
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS,(unsigned long)2,"sim.handle_all sim.handle_parent");
    // Colors and main styles:
    setAStyle(QsciScintillaBase::STYLE_DEFAULT,colBlack,colBackground,fontSize,theFont); // set global default style
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_SETCARETFORE,(unsigned long)colBlack);
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_STYLECLEARALL); // set all styles
    setAStyle(QsciScintillaBase::STYLE_LINENUMBER,(unsigned long)colWhite,(long)colDarkGrey);
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_SETSELBACK,(unsigned long)1,(long)colSelection); // selection color
    for (int i=0;syntaxColors[i].iItem!=-1;i++)
        setAStyle(syntaxColors[i].iItem,syntaxColors[i].rgb,colBackground);
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_INDICSETSTYLE,(unsigned long)20,(long)QsciScintillaBase::INDIC_STRAIGHTBOX);
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_INDICSETALPHA,(unsigned long)20,(long)160);
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_INDICSETFORE,(unsigned long)20,(long)colSelection);
    _scintillaObject->setFolding(QsciScintilla::BoxedTreeFoldStyle);

    connect(_scintillaObject,SIGNAL(SCN_CHARADDED(int)),this,SLOT(charAdded(int)));
    connect(_scintillaObject,SIGNAL(SCN_MODIFIED(int,int,const char*,int,int,int,int,int,int,int)),this,SLOT(modified(int,int,const char*,int,int,int,int,int,int,int)));

    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_SETTEXT,(unsigned long)0,"Hello world!\nsim.getObjectHandle(...)\nsim.getObjectName(..)\nsim.handle_all\nsim.handle_parent");
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_EMPTYUNDOBUFFER); // Make sure the undo buffer is empty (otherwise the first undo will remove the whole script --> a bit confusing)
    setWindowTitle("Test window");
    show();
    raise();
    activateWindow();
}

CScintillaDlg::~CScintillaDlg() 
{
    // _scintillaObject is normally automatically destroyed!
}
void CScintillaDlg::setAStyle(int style,unsigned int fore,unsigned int back,int size,const char *face)
{
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE,(unsigned long)style,(long)fore);
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_STYLESETBACK,(unsigned long)style,(long)back);
    if (size>=1)
        _scintillaObject->SendScintilla(QsciScintillaBase::SCI_STYLESETSIZE,(unsigned long)style,(long)size);
    if (face)
        _scintillaObject->SendScintilla(QsciScintillaBase::SCI_STYLESETFONT,(unsigned long)style,face);
}

void CScintillaDlg::closeEvent(QCloseEvent *event)
{
    event->ignore();
    closeRequest=true;
}

std::string CScintillaDlg::getCallTip(const char* txt)
{
    // e.g.
    if (strcmp(txt,"sim.getObjectHandle")==0)
        return("number handle=sim.getObjectHandle(string objectName)");
    return("");
}

void CScintillaDlg::charAdded(int charAdded)
{
    if (_scintillaObject->SendScintilla(QsciScintillaBase::SCI_AUTOCACTIVE)!=0)
    { // Autocomplete is active
        if (charAdded=='(')
            _scintillaObject->SendScintilla(QsciScintillaBase::SCI_AUTOCCANCEL);
    }
    if (_scintillaObject->SendScintilla(QsciScintillaBase::SCI_AUTOCACTIVE)==0)
    { // Autocomplete is not active
        if (_scintillaObject->SendScintilla(QsciScintillaBase::SCI_CALLTIPACTIVE)!=0)
        { // CallTip is active
        }
        else
        { // Calltip is not active
            if ( (charAdded=='(')||(charAdded==',') )
            { // Do we need to activate a calltip?

                char linebuf[1000];
                int current=_scintillaObject->SendScintilla(QsciScintillaBase::SCI_GETCURLINE,(unsigned long)999,linebuf);
                int pos=_scintillaObject->SendScintilla(QsciScintillaBase::SCI_GETCURRENTPOS);
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
                        setAStyle(QsciScintillaBase::STYLE_CALLTIP,VRGB(0,0,0),VRGB(255,255,255),fontSize,theFont);
                        _scintillaObject->SendScintilla(QsciScintillaBase::SCI_CALLTIPUSESTYLE,(int)0);

                        int cursorPosInPixelsFromLeftWindowBorder=_scintillaObject->SendScintilla(QsciScintillaBase::SCI_POINTXFROMPOSITION,(int)0,(unsigned long)pos);
                        int callTipWidthInPixels=_scintillaObject->SendScintilla(QsciScintillaBase::SCI_TEXTWIDTH,QsciScintillaBase::STYLE_CALLTIP,s.c_str());
                        int charWidthInPixels=_scintillaObject->SendScintilla(QsciScintillaBase::SCI_TEXTWIDTH,QsciScintillaBase::STYLE_DEFAULT,"0");
                        int callTipWidthInChars=callTipWidthInPixels/charWidthInPixels;
                        int cursorPosInCharsFromLeftWindowBorder=-5+cursorPosInPixelsFromLeftWindowBorder/charWidthInPixels; // 5 is the width in chars of the left border (line counting)
                        int cursorPosInCharsFromLeftBorder=_scintillaObject->SendScintilla(QsciScintillaBase::SCI_GETCOLUMN,(int)pos);
                        unsigned off=-std::min(cursorPosInCharsFromLeftWindowBorder,cursorPosInCharsFromLeftBorder);
                        if (callTipWidthInChars<std::min(cursorPosInCharsFromLeftWindowBorder,cursorPosInCharsFromLeftBorder))
                            off=-callTipWidthInChars;

                        _scintillaObject->SendScintilla(QsciScintillaBase::SCI_CALLTIPSHOW,(unsigned long)pos+off,s.c_str());
                    }
                }
            }
            else
            { // Do we need to activate autocomplete?
                int p=-1+_scintillaObject->SendScintilla(QsciScintillaBase::SCI_GETCURRENTPOS);
                if (p>=2)
                {
                    char linebuf[1000];
                    int current=_scintillaObject->SendScintilla(QsciScintillaBase::SCI_GETCURLINE,(unsigned long)999,linebuf);
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
                        // Here we need to create a list with all keywords that match "theWord". e.g.
                        std::string autoCompList="sim.getObjectHandle sim.getObjectName";
                        if (autoCompList.size()!=0)
                        { // We need to activate autocomplete!
                            _scintillaObject->SendScintilla(QsciScintillaBase::SCI_AUTOCSETAUTOHIDE,(int)0);
                            _scintillaObject->SendScintilla(QsciScintillaBase::SCI_AUTOCSTOPS,(unsigned long)0," ()[]{}:;~`',=*-+/?!@#$%^&|\\<>\"");
//                            _scintillaObject->SendScintilla(QsciScintillaBase::SCI_AUTOCSETMAXHEIGHT,(int)100); // it seems that SCI_AUTOCSETMAXHEIGHT and SCI_AUTOCSETMAXWIDTH are not implemented yet!
//                            _scintillaObject->SendScintilla(QsciScintillaBase::SCI_AUTOCSETMAXWIDTH,(int)500); // it seems that SCI_AUTOCSETMAXHEIGHT and SCI_AUTOCSETMAXWIDTH are not implemented yet!
                            _scintillaObject->SendScintilla(QsciScintillaBase::SCI_AUTOCSHOW,(unsigned long)cnt,&(autoCompList[0]));
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

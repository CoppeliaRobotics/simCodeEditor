#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QVector>
#include <QColor>
#include <QSize>
#include <QPoint>

struct UserKeyword
{
    QString keyword;
    QString callTip;
    bool autocomplete;
    int keywordType;
};

struct EditorOptions
{
    bool toolBar;
    bool statusBar;
    bool canRestart;
    bool searchable;
    QString windowTitle;
    bool resizable;
    bool closeable;
    bool modal;
    QSize size;
    QPoint pos;
    enum Placement {
        Absolute,
        Relative,
        Center,
        Unknown
    };
    Placement placement {Unknown};
    bool activate;
    bool editable;

    bool modalSpecial {false};
    bool lineNumbers;
    int maxLines;
    int tab_width;
    enum Lang {
        None,
        Lua,
        Python
    };
    Lang lang {None};
    QString langExt {"txt"};
    QString langComment {""};
    QString snippetsGroup;
    QString onClose;
    bool wrapWord;
    QString fontFace;
    int fontSize;
    bool fontBold;
    QVector<UserKeyword> userKeywords;
    QColor text_col;
    QColor background_col;
    QColor selection_col;
    QColor comment_col;
    QColor number_col;
    QColor string_col;
    QColor character_col;
    QColor operator_col;
    QColor identifier_col;
    QColor preprocessor_col;
    QColor keyword1_col;
    QColor keyword2_col;
    QColor keyword3_col;
    QColor keyword4_col;

    QVector<QString> luaSearchPath;

    void readFromXML(const QString &xml);
    QString resolveLuaFilePath(const QString &f);
};

char * stringBufferCopy(const QString &str);
QColor parseColor(const QString &colorStr);
bool parseBool(const QString &boolStr);

#endif // COMMON_H

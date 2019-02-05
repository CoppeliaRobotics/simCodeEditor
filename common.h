#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <string>
#include <QColor>
#include <QSize>
#include <QPoint>

struct UserKeyword
{
    std::string keyword;
    std::string callTip;
    bool autocomplete;
};

struct EditorOptions
{
    bool toolBar;
    bool statusBar;
    bool canRestart;
    bool searchable;
    std::string windowTitle;
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

    bool modalSpecial;
    bool lineNumbers;
    int maxLines;
    int tab_width;
    bool isLua;
    std::string onClose;
    bool wrapWord;
    std::string fontFace;
    int fontSize;
    std::vector<UserKeyword> userKeywords;
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

    void readFromXML(const QString &xml);
};

#endif // COMMON_H

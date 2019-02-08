#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QtWidgets>

class Dialog;

class ToolBar : public QToolBar
{
    Q_OBJECT

public:
    ToolBar(bool canRestart, Dialog *parent);
    virtual ~ToolBar();

public slots:
    void updateButtons();

public:
    QAction *actReload;
    QAction *actShowSearchPanel;
    QAction *actUndo;
    QAction *actRedo;
    QAction *actUnindent;
    QAction *actIndent;
    QAction *actFuncNav;
    struct {
        QAction *actSave;
        QComboBox *combo;
        QAction *actCombo;
        QAction *actClose;
        inline void setVisible(bool v)
        {
            actSave->setVisible(v);
            actCombo->setVisible(v);
            actClose->setVisible(v);
        }
    } openFiles;
    struct {
        QAction *act;
        QMenu *menu;
    } funcNav;

private:
    Dialog *parent;
};

#endif // TOOLBAR_H

#ifndef SEARCHANDREPLACEPANEL_H
#define SEARCHANDREPLACEPANEL_H

#include <QtWidgets>

class Dialog;

class SearchAndReplacePanel : public QWidget
{
    Q_OBJECT

public:
    SearchAndReplacePanel(Dialog *parent);
    virtual ~SearchAndReplacePanel();

public slots:
    void setVisibility(bool v);
    void show();
    void hide();

private slots:
    void find();
    void replace();

signals:
    void shown();
    void hidden();

private:
    Dialog *parent;
    QLabel *lblFind, *lblReplace;
    QComboBox *editFind, *editReplace;
    QPushButton *btnFind, *btnReplace, *btnClose;
    QCheckBox *chkRegExp, *chkCaseSens;

    friend class Dialog;
};

#endif // SEARCHANDREPLACEPANEL_H

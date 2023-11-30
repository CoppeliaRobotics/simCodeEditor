#include "searchandreplacepanel.h"
#include "dialog.h"
#include "editor.h"
#include "statusbar.h"

SearchAndReplacePanel::SearchAndReplacePanel(Dialog *parent)
    : QWidget(parent),
      parent(parent)
{
    QGridLayout *layout = new QGridLayout;
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setColumnStretch(1, 10);
    layout->addWidget(chkRegExp = new QCheckBox("Regular expression"), 1, 5);
    layout->addWidget(chkCaseSens = new QCheckBox("Case sensitive"), 2, 5);
    layout->addWidget(lblFind = new QLabel("Find:"), 1, 0);
    lblFind->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(editFind = new QComboBox, 1, 1, 1, 2);
    editFind->setEditable(true);
    editFind->setInsertPolicy(QComboBox::InsertAtTop);
    editFind->completer()->setCaseSensitivity(Qt::CaseSensitive);
    connect(editFind->lineEdit(), &QLineEdit::returnPressed, [=] {
        find();
    });
    layout->addWidget(btnFind = new QToolButton, 1, 3);
    QAction *actFind = new QAction("Find");
    QAction *actReplace = new QAction("Replace");
    QAction *actReplaceAndFind = new QAction("Replace and find");
    actReplaceAndFind->setCheckable(true);
    actReplaceAndFind->setChecked(true);
    btnFind->setDefaultAction(actFind);
    btnFind->setToolTip("");
    layout->addWidget(btnClose = new QPushButton, 1, 6);
    layout->addWidget(lblReplace = new QLabel("Replace with:"), 2, 0);
    lblReplace->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(editReplace = new QComboBox, 2, 1, 1, 2);
    editReplace->setEditable(true);
    editReplace->setInsertPolicy(QComboBox::InsertAtTop);
    layout->addWidget(btnReplace = new QToolButton, 2, 3);
    btnReplace->setPopupMode(QToolButton::MenuButtonPopup);
    btnReplace->setDefaultAction(actReplace);
    btnReplace->setToolTip("");
    QMenu *m = new QMenu(parent);
    m->addAction(actReplaceAndFind);
    btnReplace->setMenu(m);
    setLayout(layout);
    btnClose->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    btnClose->setFlat(true);
    btnClose->setStyleSheet("margin-left: 5px; margin-right: 5px; font-size: 14pt;");
    connect(btnClose, &QPushButton::clicked, this, &SearchAndReplacePanel::hide);
    connect(actFind, &QAction::triggered, [=] (bool v) {
        find();
    });
    connect(actReplace, &QAction::triggered, [=] (bool v) {
        replace();
        if(actReplaceAndFind->isChecked()) find();
    });
    hide();

    QList<QWidget*> w = {editFind, editReplace, btnFind, btnReplace};
    for(int i = 0; i < w.size() - 1; i++)
        setTabOrder(w[i], w[i + 1]);
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

void SearchAndReplacePanel::toggle()
{
    if(isVisible()) hide();
    else show();
}

void SearchAndReplacePanel::find()
{
    Editor *sci = parent->activeEditor();
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

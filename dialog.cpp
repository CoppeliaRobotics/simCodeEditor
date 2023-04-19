#include "dialog.h"
#include "editor.h"
#include "toolbar.h"
#include "statusbar.h"
#include "searchandreplacepanel.h"
#include <simPlusPlus/Lib.h>
#include "UI.h"

QString Dialog::modalText;
int Dialog::modalPosAndSize[4];

Dialog::Dialog(const EditorOptions &o, UI *ui, QWidget* pParent)
    : QDialog(pParent),
      opts(o),
      ui(ui)
{
    setAttribute(Qt::WA_DeleteOnClose);

    scriptRestartInitiallyNeeded_ = o.doesScriptInitiallyNeedRestart;

    stacked_ = new QStackedWidget;
    activeEditor_ = new Editor(this);
    editors_.insert("", activeEditor_);
    stacked_->addWidget(activeEditor_);
    textBrowser_ = new QTextBrowser;
    textBrowser_->setVisible(false);

    toolBar_ = new ToolBar(this);
    if(!o.toolBar)
        toolBar_->setVisible(false);
    searchPanel_ = new SearchAndReplacePanel(this);
    statusBar_ = new StatusBar(this);
    if(!o.statusBar)
        statusBar_->setVisible(false);

    if(o.searchable)
    {
        QShortcut *shortcut = new QShortcut(QKeySequence(tr("Ctrl+f", "Find")), this);
        connect(shortcut, &QShortcut::activated, searchPanel_, [=] {
            searchPanel_->show();
            searchPanel_->editFind->setEditText(activeEditor()->selectedText());
        });
    }

    QShortcut *saveShortcut = new QShortcut(QKeySequence(tr("Ctrl+s", "Save")), this);
    connect(saveShortcut, &QShortcut::activated, [this] {
        activeEditor_->saveExternalFile();
    });

    QVBoxLayout *bl = new QVBoxLayout;
    bl->setContentsMargins(0,0,0,0);
    bl->setSpacing(0);
    setLayout(bl);
    bl->addWidget(toolBar_);
    QHBoxLayout *hb = new QHBoxLayout;
    hb->addWidget(stacked_);
    hb->addWidget(textBrowser_);
    bl->addLayout(hb);
    bl->addWidget(searchPanel_);
    bl->addWidget(statusBar_);

    connect(toolBar_->actReload, &QAction::triggered, this, &Dialog::reloadScript);
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
        auto editor = qvariant_cast<Editor*>(toolBar_->openFiles.combo->currentData());
        if(!editor->externalFile().isEmpty())
            editor->saveExternalFile();
    });
    connect(toolBar_->openFiles.actClose, &QAction::triggered, [this]() {
        auto editor = qvariant_cast<Editor*>(toolBar_->openFiles.combo->currentData());
        if(!editor->externalFile().isEmpty())
        {
            if(editor->needsSaving())
                if(QMessageBox::Yes != QMessageBox::question(this, "", QStringLiteral("File %1 has not been saved since last change.\n\nAre you sure you want to close it?").arg(editor->externalFile())))
                    return;
            closeExternalFile(editor);
        }
    });
    connect(toolBar_->openFiles.combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this] {
        auto editor = qvariant_cast<Editor*>(toolBar_->openFiles.combo->currentData());
        switchEditor(editor);
    });

    connect(searchPanel_, &SearchAndReplacePanel::shown, toolBar_, &ToolBar::updateButtons);
    connect(searchPanel_, &SearchAndReplacePanel::hidden, toolBar_, &ToolBar::updateButtons);

    toolBar_->updateButtons();

    dirtyCheckTimer_ = new QTimer(this);
    connect(dirtyCheckTimer_, &QTimer::timeout, this, &Dialog::updateReloadButtonVisualClue);
    dirtyCheckTimer_->setInterval(2000);

    ui->requestSimulationStatus();
}

Dialog::~Dialog()
{
    // scintilla_ is normally automatically destroyed!
}

void Dialog::setEditorOptions(const EditorOptions &o)
{
    opts = o;
    for(auto e : editors_)
        e->setEditorOptions(o);

    QWidget *parent = (QWidget *)sim::getMainWindow(1);
    setWindowTitle(o.windowTitle);
    statusBar()->setSizeGripEnabled(o.resizable);
    setModal(o.modal);
    Qt::WindowFlags flags = Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint; // | Qt::WindowStaysOnTopHint;
#ifdef MAC_SIM
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
#if defined(LIN_SIM) || defined(MAC_SIM)
    if(!o.resizable) setFixedSize(size());
#endif
}

Editor * Dialog::activeEditor()
{
    return activeEditor_;
}

Editor * Dialog::openExternalFile(const QString &filePath)
{
    Editor *editor = editors_.value(filePath);

    if(!editor)
    {
        editor = new Editor(this);
        editor->setEditorOptions(opts);
        editor->openExternalFile(filePath);
        editors_.insert(filePath, editor);
        stacked_->addWidget(editor);
    }

    switchEditor(editor);

    return editor;
}

void Dialog::closeExternalFile(const QString &filePath)
{
    auto editor = editors_.value(filePath);
    if(!editor) return;
    closeExternalFile(editor);
}

void Dialog::closeExternalFile(Editor *editor)
{
    if(editor->externalFile().isNull()) return;
    stacked_->removeWidget(editor);
    editors_.remove(editor->externalFile());
    editor->deleteLater();
    //toolBar_->updateButtons();
    switchEditor(editors_.value(""));
}

void Dialog::switchEditor(Editor *editor)
{
    if(!editor) return;

    stacked_->setCurrentWidget(editor);
    activeEditor_ = editor;
    toolBar_->updateButtons();
}

bool Dialog::containsUnsavedFiles()
{
    for(auto editor : editors_)
        if(editor->needsSaving())
            return true;
    return false;
}

void Dialog::setHandle(int handle)
{
    this->handle = handle;
}

void Dialog::setInitText(const QString &text)
{
    initText_ = text;
    setText(text);
}

void Dialog::setText(const QString &text)
{
    setText(text.toUtf8().data(), 0);
}

void Dialog::setText(const char* txt, int insertMode)
{
    editors_[""]->setText(txt, insertMode);
}

QString Dialog::text()
{
    return editors_[""]->text();
}

void Dialog::show()
{
    if(!isVisible())
    {
        QDialog::show();
        if(memorizedPos[0] != -999999)
            move(memorizedPos[0], memorizedPos[1]);
    }
}

void Dialog::hide()
{
    if(isVisible())
    {
        memorizedPos[0] = x();
        memorizedPos[1] = y();
        QDialog::hide();
    }
}


std::string Dialog::makeModal(int *positionAndSize)
{
    setWindowModality(Qt::ApplicationModal);
    exec();
    if(positionAndSize != nullptr)
    {
        for(size_t i = 0; i < 4; i++)
            positionAndSize[i] = Dialog::modalPosAndSize[i];
    }
    return(Dialog::modalText.toStdString());
}

void Dialog::showHelp()
{
    showHelp(true);
}

void Dialog::showHelp(const QUrl &url)
{
    showHelp(true);
    textBrowser_->setSource(url);
}

void Dialog::hideHelp()
{
    showHelp(false);
}

void Dialog::showHelp(bool v)
{
    stacked_->setVisible(!v);
    textBrowser_->setVisible(v);
    toolBar_->actReload->setVisible(!v);
    toolBar_->actShowSearchPanel->setVisible(!v);
    toolBar_->actUndo->setVisible(!v);
    toolBar_->actRedo->setVisible(!v);
    toolBar_->actUnindent->setVisible(!v);
    toolBar_->actIndent->setVisible(!v);
    toolBar_->funcNav.act->setVisible(!v);
    toolBar_->snippetLib.act->setVisible(!v);
    toolBar_->actCloseHelp->setVisible(v);
    toolBar_->openFiles.setVisible(!v && editors_.count() > 1);
    if(v && searchPanel_->isVisible())
        searchPanel_->hide();
}

void Dialog::reject()
{
    if(searchPanel_->isVisible())
    {
        searchPanel_->hide();
    }
}

 void Dialog::closeEvent(QCloseEvent *event)
 {
    if(searchPanel_->isVisible())
    {
        searchPanel_->hide();
    }

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
        event->accept();
    }
    else
    {
        event->ignore();
        ui->notifyEvent(handle, "closeEditor", opts.onClose);
    }
}

void Dialog::updateReloadButtonVisualClue()
{
    auto action = toolBar_->actReload;

    bool dirty = false;
    auto widget = toolBar_->widgetForAction(toolBar_->actReload);
    QString txt = "Restart script";
    QString ss = "";

    if(action->isEnabled())
    {
        dirty = scriptRestartInitiallyNeeded_ || initText_ != text();
        if(dirty)
        {
            txt += " (script has changed since last restart!)";
            ss = "background-color: red;";
        }
    }

    action->setText(txt);
    widget->setStyleSheet(ss);
}

void Dialog::reloadScript()
{
    initText_ = text();
    scriptRestartInitiallyNeeded_ = false;
    updateReloadButtonVisualClue();
    ui->notifyEvent(handle, "restartScript", opts.onClose);
}

void Dialog::onSimulationRunning(bool running)
{
    bool restartButtonEnabled = running && opts.canRestartInSim || !running && opts.canRestartInNonsim;

    if(firstTimeSeeingSimulationStatus_)
        firstTimeSeeingSimulationStatus_ = false;
    else if(!opts.canRestartInNonsim || !opts.canRestartInSim) {
        scriptRestartInitiallyNeeded_ = false;
        initText_ = text();
    }

    toolBar_->actReload->setEnabled(restartButtonEnabled);
    updateReloadButtonVisualClue();

    if(restartButtonEnabled)
        dirtyCheckTimer_->start();
    else
        dirtyCheckTimer_->stop();
}

void Dialog::updateCursorSelectionDisplay()
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

void Dialog::openURL(const QString &url)
{
    ui->openURL(url);
}

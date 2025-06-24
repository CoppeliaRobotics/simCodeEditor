#include "UI.h"
#include "SIM.h"
#include "dialog.h"
#include "common.h"
#include <QDebug>
#include <simPlusPlus-2/Lib.h>
#include "stubs.h"

UI::UI(SIM *sim)
{
    UI *ui = this;
    Qt::ConnectionType sim2ui = Qt::BlockingQueuedConnection;
    QObject::connect(sim, &SIM::openModal, ui, &UI::openModal, sim2ui);
    QObject::connect(sim, &SIM::open, ui, &UI::open, sim2ui);
    QObject::connect(sim, &SIM::setText, ui, &UI::setText, sim2ui);
    QObject::connect(sim, &SIM::getText, ui, &UI::getText, sim2ui);
    QObject::connect(sim, &SIM::show, ui, &UI::show, sim2ui);
    QObject::connect(sim, &SIM::close, ui, &UI::close, sim2ui);
    QObject::connect(sim, &SIM::simulationRunning, ui, &UI::onSimulationRunning, sim2ui);
    Qt::ConnectionType ui2sim = Qt::AutoConnection;
    QObject::connect(ui, &UI::notifyEvent, sim, &SIM::notifyEvent, ui2sim);
    QObject::connect(ui, &UI::openURL, sim, &SIM::openURL, ui2sim);
    QObject::connect(ui, &UI::requestSimulationStatus, sim, &SIM::onRequestSimulationStatus, ui2sim);
}

Dialog * UI::createWindow(bool modalSpecial, const QString &initText, const QString &properties)
{
    ASSERT_THREAD(UI);

    EditorOptions o;
    o.readFromXML(properties);
    o.modalSpecial = modalSpecial;
    o.snippetsPaths << EditorOptions::resourcesPath + "/snippets";

    QWidget *parent = (QWidget *)sim::getMainWindow(1);
    Dialog *window = new Dialog(o, this, parent);
    window->setEditorOptions(o);
    window->setInitText(initText);
    if(!modalSpecial)
        window->show();
    return window;
}

void UI::openModal(const QString &initText, const QString &properties, QString& text, int *positionAndSize)
{
    ASSERT_THREAD(UI);

    Dialog *editor = createWindow(true, initText, properties);
    text = editor->makeModal(positionAndSize).c_str();
}

void UI::open(const QString &initText, const QString &properties, int *handle)
{
    ASSERT_THREAD(UI);
    Dialog *editor = createWindow(false, initText, properties);
    *handle = nextEditorHandle++;
    editor->setHandle(*handle);
    editors[*handle] = editor;
}

void UI::setText(int handle, const QString &text, int insertMode)
{
    ASSERT_THREAD(UI);

    Dialog *editor = editors.value(handle);
    if(editor)
        editor->setText(text.toStdString().c_str(), insertMode);
}

void UI::getText(int handle, QString *text, int* posAndSize)
{
    ASSERT_THREAD(UI);

    Dialog *editor = editors.value(handle);
    if(editor)
    {
        *text = editor->text();
        if(posAndSize != nullptr)
        {
            posAndSize[0] = editor->x();
            posAndSize[1] = editor->y();
            posAndSize[2] = editor->width();
            posAndSize[3] = editor->height();
        }
    }
}

void UI::show(int handle, int showState)
{
    ASSERT_THREAD(UI);

    Dialog *editor = editors.value(handle);
    if(editor)
    {
        if (showState)
            editor->show();
        else
            editor->hide();
    }
}

void UI::close(int handle, int *positionAndSize)
{
    ASSERT_THREAD(UI);

    Dialog *editor = editors.value(handle);
    if(editor)
    {
        if(positionAndSize)
        {
            positionAndSize[0] = editor->x();
            positionAndSize[1] = editor->y();
            positionAndSize[2] = editor->width();
            positionAndSize[3] = editor->height();
        }
        editor->deleteLater();
        editors.remove(handle);
    }
}

void UI::onSimulationRunning(bool running)
{
    for(auto editor : editors.values())
    {
        editor->onSimulationRunning(running);
    }
}

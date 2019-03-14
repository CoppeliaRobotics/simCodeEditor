#include "UI.h"
#include "debug.h"
#include "v_repLib.h"
#include "dialog.h"
#include "common.h"
#include <QDebug>

Dialog * UI::createWindow(bool modalSpecial, const QString &initText, const QString &properties)
{
    ASSERT_THREAD(UI);

    EditorOptions o;
    o.readFromXML(properties);

    QWidget *parent = (QWidget *)simGetMainWindow(1);
    Dialog *window = new Dialog(o, this, parent);
    window->setEditorOptions(o);
    window->setText(initText);
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
        delete editor;
        editors.remove(handle);
    }
}

#include "UI.h"
#include "debug.h"
#include "v_repLib.h"
#include "scintillaDlg.h"

void UI::openModal(const QString &initText, const QString &properties, QSemaphore *sem, QString *text, int *positionAndSize)
{
    CScintillaDlg *editor = new CScintillaDlg(this, (QWidget*)simGetMainWindow(1));
    editor->setText(initText);
    editor->setModal(sem, text, positionAndSize);
    DBG << "created modal editor" << std::endl;
}

void UI::open(const QString &initText, const QString &properties, int *handle)
{
    CScintillaDlg *editor = new CScintillaDlg(this, (QWidget*)simGetMainWindow(1));
    editor->setText(initText);
    DBG << "created editor" << std::endl;
    *handle = nextEditorHandle++;
    editor->setHandle(*handle);
    editors[*handle] = editor;
}

void UI::setText(int handle, const QString &text, int insertMode)
{
    CScintillaDlg *editor = editorByHandle(handle);
    if(editor)
    {
        editor->setText(text);
    }
}

void UI::getText(int handle, QString *text)
{
    CScintillaDlg *editor = editorByHandle(handle);
    if(editor)
    {
        *text = editor->text();
    }
}

void UI::show(int handle, int showState)
{
    CScintillaDlg *editor = editorByHandle(handle);
    if(editor)
    {
    }
}

void UI::close(int handle, int *positionAndSize)
{
    CScintillaDlg *editor = editorByHandle(handle);
    if(editor)
    {
        if(positionAndSize)
        {
            positionAndSize[0] = editor->x();
            positionAndSize[1] = editor->y();
            positionAndSize[3] = editor->width();
            positionAndSize[4] = editor->height();
        }
        delete editor;
        editors.remove(handle);
    }
}

CScintillaDlg * UI::editorByHandle(int handle)
{
    if(editors.contains(handle))
        return editors[handle];
    return nullptr;
}


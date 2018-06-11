#ifndef CODEEDITOR_H__INCLUDED
#define CODEEDITOR_H__INCLUDED

VREP_DLLEXPORT char * codeEditor_openModal(const char *initText, const char *properties, int *positionAndSize)
{
    return vrepPlugin.codeEditor_openModal(initText, properties, positionAndSize);
}

VREP_DLLEXPORT int codeEditor_open(const char *initText, const char *properties)
{
    return vrepPlugin.codeEditor_open(initText, properties);
}

VREP_DLLEXPORT int codeEditor_setText(int handle, const char *text, int insertMode)
{
    return vrepPlugin.codeEditor_setText(handle, text, insertMode);
}

VREP_DLLEXPORT char * codeEditor_getText(int handle)
{
    return vrepPlugin.codeEditor_getText(handle);
}

VREP_DLLEXPORT int codeEditor_show(int handle, int showState)
{
    return vrepPlugin.codeEditor_show(handle, showState);
}

VREP_DLLEXPORT int codeEditor_close(int handle, int *positionAndSize)
{
    return vrepPlugin.codeEditor_close(handle, positionAndSize);
}

#endif // CODEEDITOR_H__INCLUDED

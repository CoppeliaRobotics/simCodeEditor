#include "plugin.h"
#include "v_repPlusPlus/Plugin.h"
#include "scintillaDlg.h"
#include "debug.h"

CScintillaDlg *editor = nullptr;
bool closeIt = false;

class Plugin : public vrep::Plugin
{
public:
    void onStart()
    {
        uiThread();

        if(simGetBooleanParameter(sim_boolparam_headless) > 0)
            throw std::runtime_error("cannot load in headless mode");

        editor = new CScintillaDlg((QWidget*)simGetMainWindow(1));

        simSetModuleInfo("CodeEditor", 0, "Dummy Code Editor Plugin", 0);
        simSetModuleInfo("CodeEditor", 1, __DATE__, 0);
    }

    void onEnd()
    {
        delete editor;
    }

    void onInstancePass(const vrep::InstancePassFlags &flags, bool first)
    {
        if(first)
        {
            simThread();
        }

        if(editor && editor->closeRequest)
        {
            simEventNotification("<event origin=\"codeEditor\" msg=\"close\" handle=\"0\"></event>");
        }
    }

    void onGuiPass()
    {
        if(editor && closeIt)
        {
            delete editor;
            editor = nullptr;
        }
    }
};

VREP_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)

VREP_DLLEXPORT char * codeEditor_openModal(const char *initText, const char *properties, int *positionAndSize)
{
    DBG << "codeEditor_openModal: initText=" << initText << ", properties=" << properties << std::endl;

    if(positionAndSize)
    {
        positionAndSize[0] = 0; // x
        positionAndSize[1] = 0; // y
        positionAndSize[2] = 400; // sx
        positionAndSize[3] = 500; // sy
    }
    const char *txt = "Hello from modal code editor";
    char *buff = simCreateBuffer(strlen(txt) + 1);
    strcpy(buff, txt);
    return buff;
}

VREP_DLLEXPORT int codeEditor_open(const char *initText, const char *properties)
{
    DBG << "codeEditor_open: initText=" << initText << ", properties=" << properties << std::endl;

    return -1;
}

VREP_DLLEXPORT int codeEditor_setText(int handle, const char *text, int insertMode)
{
    DBG << "codeEditor_setText: handle=" << handle << ", text=" << text << ", insertMode=" << insertMode << std::endl;

    return -1;
}

VREP_DLLEXPORT char * codeEditor_getText(int handle)
{
    DBG << "codeEditor_getText: handle=" << handle << std::endl;

    const char *txt = "Hello from code editor";
    char *buff = simCreateBuffer(strlen(txt) + 1);
    strcpy(buff, txt);
    return buff;
}

VREP_DLLEXPORT int codeEditor_show(int handle, int showState)
{
    DBG << "codeEditor_show: handle=" << handle << ", showState=" << showState << std::endl;

    return -1;
}

VREP_DLLEXPORT int codeEditor_close(int handle, int *positionAndSize)
{
    DBG << "codeEditor_close: handle=" << handle << std::endl;

    closeIt = true;

    if(positionAndSize)
    {
        positionAndSize[0] = 0; // x
        positionAndSize[1] = 0; // y
        positionAndSize[2] = 400; // sx
        positionAndSize[3] = 500; // sy
    }

    return -1;
}


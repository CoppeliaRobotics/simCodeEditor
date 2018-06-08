#include "plugin.h"
#include "SIM.h"
#include "UI.h"
#include "v_repPlusPlus/Plugin.h"
#include "scintillaDlg.h"
#include "debug.h"
#include <QString>

UI *ui;
SIM *sim;

class Plugin : public vrep::Plugin
{
public:
    void onStart()
    {
        uiThread();

        if(simGetBooleanParameter(sim_boolparam_headless) > 0)
            throw std::runtime_error("cannot load in headless mode");

        simSetModuleInfo(PLUGIN_NAME, 0, "Code Editor Plugin", 0);
        simSetModuleInfo(PLUGIN_NAME, 1, __DATE__, 0);

        ui = new UI;

        DBG << "CodeEditor plugin initialized" << std::endl;
    }

    void onEnd()
    {
    }

    void onInstancePass(const vrep::InstancePassFlags &flags, bool first)
    {
        if(first)
        {
            simThread();

            sim = new SIM(ui);
        }
    }
};

VREP_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)

char * stringBufferCopy(const QString &str)
{
    QByteArray byteArr = str.toLocal8Bit();
    char *buff = simCreateBuffer(byteArr.length() + 1);
    strcpy(buff, byteArr.data());
    buff[byteArr.length()] = '\0';
    return buff;
}

VREP_DLLEXPORT char * codeEditor_openModal(const char *initText, const char *properties, int *positionAndSize)
{
    DBG << "codeEditor_openModal: initText=" << initText << ", properties=" << properties << std::endl;

    QString text;
    QSemaphore sem;
    sim->openModal(QString(initText), QString(properties), &sem, &text, positionAndSize);
    sem.acquire();

    DBG << "codeEditor_openModal: done" << std::endl;

    return stringBufferCopy(text);
}

VREP_DLLEXPORT int codeEditor_open(const char *initText, const char *properties)
{
    DBG << "codeEditor_open: initText=" << initText << ", properties=" << properties << std::endl;

    int handle = -1;
    sim->open(QString(initText), QString(properties), &handle);

    DBG << "codeEditor_open: done" << std::endl;

    return handle;
}

VREP_DLLEXPORT int codeEditor_setText(int handle, const char *text, int insertMode)
{
    DBG << "codeEditor_setText: handle=" << handle << ", text=" << text << ", insertMode=" << insertMode << std::endl;

    sim->setText(handle, QString(text), insertMode);

    DBG << "codeEditor_setText: done" << std::endl;

    return -1;
}

VREP_DLLEXPORT char * codeEditor_getText(int handle)
{
    DBG << "codeEditor_getText: handle=" << handle << std::endl;

    QString text;
    sim->getText(handle, &text);

    DBG << "codeEditor_getText: done" << std::endl;

    return stringBufferCopy(text);
}

VREP_DLLEXPORT int codeEditor_show(int handle, int showState)
{
    DBG << "codeEditor_show: handle=" << handle << ", showState=" << showState << std::endl;

    sim->show(handle, showState);

    DBG << "codeEditor_show: done" << std::endl;

    return -1;
}

VREP_DLLEXPORT int codeEditor_close(int handle, int *positionAndSize)
{
    DBG << "codeEditor_close: handle=" << handle << std::endl;

    sim->close(handle, positionAndSize);

    DBG << "codeEditor_close: done" << std::endl;

    return -1;
}


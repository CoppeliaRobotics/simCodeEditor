#include "plugin.h"
#include "SIM.h"
#include "UI.h"
#include "v_repPlusPlus/Plugin.h"
#include "common.h"
#include "debug.h"
#include <QString>

class Plugin : public vrep::Plugin
{
public:
    void onStart()
    {
        sim = nullptr;
        uiThread();

        if(simGetBooleanParameter(sim_boolparam_headless) > 0)
            throw std::runtime_error("cannot load in headless mode");

        simSetModuleInfo(PLUGIN_NAME, 0, "Code Editor Plugin", 0);
        simSetModuleInfo(PLUGIN_NAME, 1, BUILD_DATE, 0);

        ui = new UI;

        DEBUG_OUT << "CodeEditor plugin initialized" << std::endl;
    }

    void onEnd()
    {
        UI_THREAD = NULL;
        SIM_THREAD = NULL;
    }

    void onInstancePass(const vrep::InstancePassFlags &flags, bool first)
    {
        if(first)
        {
            simThread();

            sim = new SIM(ui);
        }
    }

    char * codeEditor_openModal(const char *initText, const char *properties, int *positionAndSize)
    { // special: blocking until dlg closed
        ASSERT_THREAD(!UI);

        DEBUG_OUT << "codeEditor_openModal: initText=" << initText << ", properties=" << properties << std::endl;
        QString text;
        if (QThread::currentThreadId() == UI_THREAD)
            ui->openModal(QString(initText), QString(properties),text, positionAndSize);
        else
        {
            if (sim!=nullptr)
                sim->openModal(QString(initText), QString(properties), text, positionAndSize);
        }
        char* retVal = stringBufferCopy(text);
        DEBUG_OUT << "codeEditor_openModal: done" << std::endl;

        return retVal;
    }

    int codeEditor_open(const char *initText, const char *properties)
    {
        DEBUG_OUT << "codeEditor_open: initText=" << initText << ", properties=" << properties << std::endl;

        int handle = -1;
        if (QThread::currentThreadId() == UI_THREAD)
            ui->open(QString(initText), QString(properties), &handle);
        else
        {
            if (sim != nullptr)
                sim->open(QString(initText), QString(properties), &handle);
        }
        DEBUG_OUT << "codeEditor_open: done" << std::endl;

        return handle;
    }

    int codeEditor_setText(int handle, const char *text, int insertMode)
    {
        DEBUG_OUT << "codeEditor_setText: handle=" << handle << ", text=" << text << ", insertMode=" << insertMode << std::endl;

        if(QThread::currentThreadId() == UI_THREAD)
            ui->setText(handle, QString(text), insertMode);
        else
        {
            if (sim != nullptr)
                sim->setText(handle, QString(text), insertMode);
        }
        DEBUG_OUT << "codeEditor_setText: done" << std::endl;

        return -1;
    }

    char * codeEditor_getText(int handle, int* posAndSize)
    {
        DEBUG_OUT << "codeEditor_getText: handle=" << handle << std::endl;

        QString text;
        if(QThread::currentThreadId() == UI_THREAD)
            ui->getText(handle, &text, posAndSize);
        else
        {
            if (sim != nullptr)
                sim->getText(handle, &text, posAndSize);
        }
        DEBUG_OUT << "codeEditor_getText: done" << std::endl;

        return stringBufferCopy(text);
    }

    int codeEditor_show(int handle, int showState)
    {
        DEBUG_OUT << "codeEditor_show: handle=" << handle << ", showState=" << showState << std::endl;

        if(QThread::currentThreadId() == UI_THREAD)
            ui->show(handle, showState);
        else
        {
            if (sim != nullptr)
                sim->show(handle, showState);
        }
        DEBUG_OUT << "codeEditor_show: done" << std::endl;

        return -1;
    }

    int codeEditor_close(int handle, int *positionAndSize)
    {
        DEBUG_OUT << "codeEditor_close: handle=" << handle << std::endl;

        if(QThread::currentThreadId() == UI_THREAD)
            ui->close(handle, positionAndSize);
        else
        {
            if (sim != nullptr)
                sim->close(handle, positionAndSize);
        }
        DEBUG_OUT << "codeEditor_close: done" << std::endl;

        return -1;
    }
    
private:
    UI *ui;
    SIM *sim;
};

VREP_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)

// plugin entrypoints:

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

VREP_DLLEXPORT char * codeEditor_getText(int handle, int *positionAndSize)
{
    return vrepPlugin.codeEditor_getText(handle,positionAndSize);
}

VREP_DLLEXPORT int codeEditor_show(int handle, int showState)
{
    return vrepPlugin.codeEditor_show(handle, showState);
}

VREP_DLLEXPORT int codeEditor_close(int handle, int *positionAndSize)
{
    return vrepPlugin.codeEditor_close(handle, positionAndSize);
}

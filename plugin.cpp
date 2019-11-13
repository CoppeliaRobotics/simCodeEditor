#include "plugin.h"
#include "SIM.h"
#include "UI.h"
#include "simPlusPlus/Plugin.h"
#include "common.h"
#include "debug.h"
#include "api_index.cpp"
#include <QtCore>

class Plugin : public sim::Plugin
{
public:
    void onStart()
    {
        uiThread();

        if(simGetBooleanParameter(sim_boolparam_headless) > 0)
            throw std::runtime_error("cannot load in headless mode");

        simSetModuleInfo(PLUGIN_NAME, 0, "Code Editor Plugin", 0);
        simSetModuleInfo(PLUGIN_NAME, 1, BUILD_DATE, 0);

        ui = new UI;

        // load api index
        {
            QDir appDir(QCoreApplication::applicationDirPath());
#ifdef MAC_SIM
            if(!appDir.cd("../../..")) return;
#endif
            if(!appDir.cd("helpFiles")) return;
            int i = 0;
            while(api_index[i])
            {
                QString k(api_index[i++]);
                QString v(api_index[i++]);
                apiReferenceMap[k] = appDir.absolutePath() + "/" + v;
            }
        }

        DEBUG_OUT << "CodeEditor plugin initialized" << std::endl;
    }

    void onEnd()
    {
        delete ui;
        ui = nullptr;

        UI_THREAD = NULL;
        SIM_THREAD = NULL;
    }

    void onFirstInstancePass(const sim::InstancePassFlags &flags)
    {
        simThread();

        sim = new SIM(ui);
    }

    void onLastInstancePass()
    {
        delete sim;
        sim = nullptr;
    }

    char * codeEditor_openModal(const char *initText, const char *properties, int *positionAndSize)
    { // special: blocking until dlg closed
        ASSERT_THREAD(!UI);

        DEBUG_OUT << "codeEditor_openModal: initText=" << initText << ", properties=" << properties << std::endl;
        QString text;
        if(QThread::currentThreadId() == UI_THREAD)
        {
            ui->openModal(QString(initText), QString(properties),text, positionAndSize);
        }
        else
        {
            if(sim)
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
        if(QThread::currentThreadId() == UI_THREAD)
            ui->open(QString(initText), QString(properties), &handle);
        else
        {
            if(sim)
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
            if(sim)
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
            if(sim)
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
            if(sim)
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
            if(sim)
                sim->close(handle, positionAndSize);
        }
        DEBUG_OUT << "codeEditor_close: done" << std::endl;

        return -1;
    }

    QString apiReferenceForSymbol(const QString &sym)
    {
        return apiReferenceMap.value(sym, "");
    }

private:
    UI *ui;
    SIM *sim;
    QMap<QString, QString> apiReferenceMap;
};

SIM_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)

QString apiReferenceForSymbol(const QString &sym)
{
    return simPlugin->apiReferenceForSymbol(sym);
}

// plugin entrypoints:

SIM_DLLEXPORT char * codeEditor_openModal(const char *initText, const char *properties, int *positionAndSize)
{
    return simPlugin->codeEditor_openModal(initText, properties, positionAndSize);
}

SIM_DLLEXPORT int codeEditor_open(const char *initText, const char *properties)
{
    return simPlugin->codeEditor_open(initText, properties);
}

SIM_DLLEXPORT int codeEditor_setText(int handle, const char *text, int insertMode)
{
    return simPlugin->codeEditor_setText(handle, text, insertMode);
}

SIM_DLLEXPORT char * codeEditor_getText(int handle, int *positionAndSize)
{
    return simPlugin->codeEditor_getText(handle,positionAndSize);
}

SIM_DLLEXPORT int codeEditor_show(int handle, int showState)
{
    return simPlugin->codeEditor_show(handle, showState);
}

SIM_DLLEXPORT int codeEditor_close(int handle, int *positionAndSize)
{
    return simPlugin->codeEditor_close(handle, positionAndSize);
}

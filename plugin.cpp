#include "plugin.h"
#include "SIM.h"
#include "UI.h"
#include "simPlusPlus/Plugin.h"
#include "common.h"
#include "stubs.h"
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

        if(!registerScriptStuff())
            throw std::runtime_error("failed to register script stuff");

        setExtVersion("Code Editor Plugin");
        setBuildDate(BUILD_DATE);

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

        log(sim_verbosity_loadinfo, "CodeEditor plugin initialized");
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

        log(sim_verbosity_debug, boost::format("codeEditor_openModal: initText=%s, properties=%s") % initText % properties);
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

        log(sim_verbosity_debug, "codeEditor_openModal: done");

        return retVal;
    }

    int codeEditor_open(const char *initText, const char *properties)
    {
        log(sim_verbosity_debug, boost::format("codeEditor_open: initText=%s, properties=%s") % initText % properties);

        int handle = -1;
        if(QThread::currentThreadId() == UI_THREAD)
            ui->open(QString(initText), QString(properties), &handle);
        else
        {
            if(sim)
                sim->open(QString(initText), QString(properties), &handle);
        }

        log(sim_verbosity_debug, "codeEditor_open: done");

        return handle;
    }

    int codeEditor_setText(int handle, const char *text, int insertMode)
    {
        log(sim_verbosity_debug, boost::format("codeEditor_setText: handle=%d, text=%s, insertMode=%d") % handle % text % insertMode);

        if(QThread::currentThreadId() == UI_THREAD)
            ui->setText(handle, QString(text), insertMode);
        else
        {
            if(sim)
                sim->setText(handle, QString(text), insertMode);
        }

        log(sim_verbosity_debug, "codeEditor_setText: done");

        return -1;
    }

    char * codeEditor_getText(int handle, int* posAndSize)
    {
        log(sim_verbosity_debug, boost::format("codeEditor_getText: handle=%d") % handle);

        QString text;
        if(QThread::currentThreadId() == UI_THREAD)
            ui->getText(handle, &text, posAndSize);
        else
        {
            if(sim)
                sim->getText(handle, &text, posAndSize);
        }

        log(sim_verbosity_debug, "codeEditor_getText: done");

        return stringBufferCopy(text);
    }

    int codeEditor_show(int handle, int showState)
    {
        log(sim_verbosity_debug, boost::format("codeEditor_getText: handle=%d, showState=%d") % handle % showState);

        if(QThread::currentThreadId() == UI_THREAD)
            ui->show(handle, showState);
        else
        {
            if(sim)
                sim->show(handle, showState);
        }

        log(sim_verbosity_debug, "codeEditor_show: done");

        return -1;
    }

    int codeEditor_close(int handle, int *positionAndSize)
    {
        log(sim_verbosity_debug, boost::format("codeEditor_close: handle=%d") % handle);

        if(QThread::currentThreadId() == UI_THREAD)
            ui->close(handle, positionAndSize);
        else
        {
            if(sim)
                sim->close(handle, positionAndSize);
        }

        log(sim_verbosity_debug, "codeEditor_close: done");

        return -1;
    }

    QUrl apiReferenceForSymbol(const QString &sym)
    {
        QMap<QString, QString>::const_iterator i = apiReferenceMap.find(sym);
        if(i == apiReferenceMap.end()) return {};
        QString refUrl = i.value();
        int anchorPos = refUrl.lastIndexOf("#");
        QString anchor;
        if(anchorPos >= 0)
        {
            anchor = refUrl.mid(anchorPos + 1);
            refUrl = refUrl.left(anchorPos);
        }
        QUrl url(QUrl::fromLocalFile(refUrl));
        url.setFragment(anchor);
        return url;
    }

private:
    UI *ui;
    SIM *sim;
    QMap<QString, QString> apiReferenceMap;
};

SIM_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)

QUrl apiReferenceForSymbol(const QString &sym)
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

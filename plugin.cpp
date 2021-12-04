#include "config.h"
#include "plugin.h"
#include "SIM.h"
#include "UI.h"
#include "simPlusPlus/Plugin.h"
#include "common.h"
#include "stubs.h"
#include <QtCore>
#include <QHostInfo>

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

        QHostInfo::lookupHost("www.coppeliarobotics.com",
            [=] (const QHostInfo &info)
            {
                if(info.error() == QHostInfo::NoError)
                    online = true;
            }
        );
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

        auto p = sim::getStringNamedParam("CodeEditor.verboseErrors");
        if(p)
            verboseErrors = *p == "1" || *p == "true";

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

        sim::addLog(sim_verbosity_debug, "codeEditor_openModal: initText=%s, properties=%s", initText, properties);
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

        sim::addLog(sim_verbosity_debug, "codeEditor_openModal: done");

        return retVal;
    }

    int codeEditor_open(const char *initText, const char *properties)
    {
        sim::addLog(sim_verbosity_debug, "codeEditor_open: initText=%s, properties=%s", initText, properties);

        int handle = -1;
        if(QThread::currentThreadId() == UI_THREAD)
            ui->open(QString(initText), QString(properties), &handle);
        else
        {
            if(sim)
                sim->open(QString(initText), QString(properties), &handle);
        }

        sim::addLog(sim_verbosity_debug, "codeEditor_open: done");

        return handle;
    }

    int codeEditor_setText(int handle, const char *text, int insertMode)
    {
        sim::addLog(sim_verbosity_debug, "codeEditor_setText: handle=%d, text=%s, insertMode=%d", handle, text, insertMode);

        if(QThread::currentThreadId() == UI_THREAD)
            ui->setText(handle, QString(text), insertMode);
        else
        {
            if(sim)
                sim->setText(handle, QString(text), insertMode);
        }

        sim::addLog(sim_verbosity_debug, "codeEditor_setText: done");

        return -1;
    }

    char * codeEditor_getText(int handle, int* posAndSize)
    {
        sim::addLog(sim_verbosity_debug, "codeEditor_getText: handle=%d", handle);

        QString text;
        if(QThread::currentThreadId() == UI_THREAD)
            ui->getText(handle, &text, posAndSize);
        else
        {
            if(sim)
                sim->getText(handle, &text, posAndSize);
        }

        sim::addLog(sim_verbosity_debug, "codeEditor_getText: done");

        return stringBufferCopy(text);
    }

    int codeEditor_show(int handle, int showState)
    {
        sim::addLog(sim_verbosity_debug, "codeEditor_getText: handle=%d, showState=%d", handle, showState);

        if(QThread::currentThreadId() == UI_THREAD)
            ui->show(handle, showState);
        else
        {
            if(sim)
                sim->show(handle, showState);
        }

        sim::addLog(sim_verbosity_debug, "codeEditor_show: done");

        return -1;
    }

    int codeEditor_close(int handle, int *positionAndSize)
    {
        sim::addLog(sim_verbosity_debug, "codeEditor_close: handle=%d", handle);

        if(QThread::currentThreadId() == UI_THREAD)
            ui->close(handle, positionAndSize);
        else
        {
            if(sim)
                sim->close(handle, positionAndSize);
        }

        sim::addLog(sim_verbosity_debug, "codeEditor_close: done");

        return -1;
    }

    QUrl apiReferenceForSymbol(const QString &sym)
    {
        // split symbol (e.g.: "sim.getObject" -> "sim", "getObject")
        QString mod;
        QString func(sym);
        int dotPos = sym.indexOf('.');
        if(dotPos >= 0)
        {
            mod = sym.left(dotPos);
            func = sym.mid(dotPos + 1);
        }

        // locate local "helpFiles" dir
        QDir helpFiles(QCoreApplication::applicationDirPath());
#ifdef MAC_SIM
#if SIM_PROGRAM_FULL_VERSION_NB < 4010000
        if(!helpFiles.cd("../../.."))
        {
            if(verboseErrors)
                sim::addLog(sim_verbosity_errors, "Bad directory layout (<4.1.0)");
            return {};
        }
#else
        // since 4.1.0, we have app bundle layout:
        if(!helpFiles.cd("../Resources"))
        {
            if(verboseErrors)
                sim::addLog(sim_verbosity_errors, "Bad directory layout (can't locate \"Resources\" dir)");
            return {};
        }
#endif // SIM_PROGRAM_FULL_VERSION_NB
#endif // MAC_SIM
        if(!helpFiles.cd("helpFiles"))
        {
            if(verboseErrors)
                sim::addLog(sim_verbosity_errors, "Bad directory layout (can't locate \"helpFiles\" dir)");
            return {};
        }

        // read index/<mod>.json file
        QDir idxDir(helpFiles);
        if(!idxDir.cd("index"))
        {
            if(verboseErrors)
                sim::addLog(sim_verbosity_errors, "Bad directory layout (missing \"index\" dir inside \"helpFiles\" dir)");
            return {};
        }
        QString idx = (mod.isEmpty() ? "__global__" : mod) + ".json";
        if(!idxDir.exists(idx))
        {
            if(verboseErrors)
                sim::addLog(sim_verbosity_errors, "File %s not found", idx.toStdString());
            return {};
        }
        QFile idxFile(idxDir.filePath(idx));
        if(!idxFile.open(QIODevice::ReadOnly))
        {
            if(verboseErrors)
                sim::addLog(sim_verbosity_errors, "Could not open index file %s for reading", idx.toStdString());
            return {};
        }
        QByteArray idxData = idxFile.readAll();
        QJsonDocument idxDoc(QJsonDocument::fromJson(idxData));

        QJsonObject idxObj(idxDoc.object());
        QJsonObject::const_iterator i;
        if(!mod.isEmpty())
        {
            // look for a <mod> key, with an object value
            i = idxObj.constFind(mod);
            if(i == idxObj.constEnd())
            {
                if(verboseErrors)
                    sim::addLog(sim_verbosity_errors, "Bad index file %s (missing \"%s\" key)", idx.toStdString(), mod.toStdString());
                return {};
            }
            idxObj = i.value().toObject();
        }

        // then for a <func> key, with a string value
        i = idxObj.constFind(func);
        if(i == idxObj.constEnd())
        {
            if(verboseErrors)
                sim::addLog(sim_verbosity_errors, "Key \"%s\" not found in index file %s", func.toStdString(), idx.toStdString());
            return {};
        }
        QString fileName = i.value().toString();
        if(fileName.isNull())
        {
            if(verboseErrors)
                sim::addLog(sim_verbosity_errors, "Bad key \"%s\" in index file %s (not a string)", func.toStdString(), idx.toStdString());
            return {};
        }

        // split fragment if any
        int anchorPos = fileName.lastIndexOf("#");
        QString anchor;
        if(anchorPos >= 0)
        {
            anchor = fileName.mid(anchorPos + 1);
            fileName = fileName.left(anchorPos);
        }

        // compose online/offline URL
        QUrl url;
        if(isOnline())
        {
            url.setScheme("https");
            url.setHost("www.coppeliarobotics.com");
            url.setPath("/helpFiles/en/" + fileName);
        }
        else
        {
            url.setScheme("file");
            url.setPath(helpFiles.absolutePath() + "/en/" + fileName);
        }
        if(!anchor.isEmpty())
            url.setFragment(anchor);

        return url;
    }

    bool isOnline() const
    {
        return online;
    }

private:
    UI *ui;
    SIM *sim;
    bool online = false;
    bool verboseErrors = false;
};

SIM_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)

QUrl apiReferenceForSymbol(const QString &sym)
{
    return sim::plugin->apiReferenceForSymbol(sym);
}

bool isOnline()
{
    return sim::plugin->isOnline();
}

// plugin entrypoints:

SIM_DLLEXPORT char * codeEditor_openModal(const char *initText, const char *properties, int *positionAndSize)
{
    return sim::plugin->codeEditor_openModal(initText, properties, positionAndSize);
}

SIM_DLLEXPORT int codeEditor_open(const char *initText, const char *properties)
{
    return sim::plugin->codeEditor_open(initText, properties);
}

SIM_DLLEXPORT int codeEditor_setText(int handle, const char *text, int insertMode)
{
    return sim::plugin->codeEditor_setText(handle, text, insertMode);
}

SIM_DLLEXPORT char * codeEditor_getText(int handle, int *positionAndSize)
{
    return sim::plugin->codeEditor_getText(handle,positionAndSize);
}

SIM_DLLEXPORT int codeEditor_show(int handle, int showState)
{
    return sim::plugin->codeEditor_show(handle, showState);
}

SIM_DLLEXPORT int codeEditor_close(int handle, int *positionAndSize)
{
    return sim::plugin->codeEditor_close(handle, positionAndSize);
}

#include "SIM.h"
#include "UI.h"
#include <simPlusPlus-2/Lib.h>
#include "stubs.h"

SIM::SIM()
{
}

void SIM::notifyEvent(int handle, const QString &eventType, const QString &data, qint64 scriptHandle)
{
    ASSERT_THREAD(!UI);

    if(scriptHandle != -1)
    {
        int stack = sim::createStack();
        sim::pushValueOntoStack(stack, handle);
        sim::pushValueOntoStack(stack, eventType.toStdString());
        sim::callScriptFunctionEx(scriptHandle, data.toUtf8().constData(), stack);
        sim::releaseStack(stack);
    }
    else
    {
        QString xml(
            QStringLiteral("<event origin='codeEditor' msg='%1' handle='%2' data='%3'/>")
                .arg(eventType, QString::number(handle), data)
        );
        sim::eventNotification(xml.toStdString());
    }
}

void SIM::openURL(const QString &url)
{
    int stackHandle = sim::createStack();
    QString s(QStringLiteral("require'simURLDrop'.openURL(\"%1\")@lua").arg(url));
    sim::executeScriptString(sim::getScriptHandleEx(sim_scripttype_sandbox), s.toStdString(), stackHandle);
    sim::releaseStack(stackHandle);
}

void SIM::onRequestSimulationStatus()
{
    bool running = sim::getSimulationState() == sim_simulation_advancing_running;
    emit simulationRunning(running);
}

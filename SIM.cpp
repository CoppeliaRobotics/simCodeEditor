#include "SIM.h"
#include "UI.h"
#include "stubs.h"

SIM::SIM(UI *theUi)
{
    ui = theUi;
    Qt::ConnectionType sim2ui = Qt::BlockingQueuedConnection;
    QObject::connect(this, &SIM::openModal, ui, &UI::openModal, sim2ui);
    QObject::connect(this, &SIM::open, ui, &UI::open, sim2ui);
    QObject::connect(this, &SIM::setText, ui, &UI::setText, sim2ui);
    QObject::connect(this, &SIM::getText, ui, &UI::getText, sim2ui);
    QObject::connect(this, &SIM::show, ui, &UI::show, sim2ui);
    QObject::connect(this, &SIM::close, ui, &UI::close, sim2ui);
    Qt::ConnectionType ui2sim = Qt::AutoConnection;
    QObject::connect(ui, &UI::notifyEvent, this, &SIM::notifyEvent, ui2sim);
    QObject::connect(ui, &UI::openURL, this, &SIM::openURL, ui2sim);
}

void SIM::notifyEvent(int handle, const QString &eventType, const QString &data)
{
    ASSERT_THREAD(!UI);

    QString xml(
        QStringLiteral("<event origin='codeEditor' msg='%1' handle='%2' data='%3'/>")
            .arg(eventType, QString::number(handle), data)
    );
    sim::eventNotification(xml.toStdString());
}

void SIM::openURL(const QString &url)
{
    int stackHandle = sim::createStack();
    QString s(QStringLiteral("simURLDrop.openURL(\"%1\")").arg(url));
    sim::executeScriptString(sim_scripttype_sandboxscript, s.toStdString(), stackHandle);
    sim::releaseStack(stackHandle);
}

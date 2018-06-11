#include "SIM.h"
#include "UI.h"
#include "v_repLib.h"
#include "debug.h"

SIM::SIM(UI *ui)
{
    Qt::ConnectionType sim2ui = Qt::BlockingQueuedConnection;
    QObject::connect(this, &SIM::openModal, ui, &UI::openModal, sim2ui);
    QObject::connect(this, &SIM::open, ui, &UI::open, sim2ui);
    QObject::connect(this, &SIM::setText, ui, &UI::setText, sim2ui);
    QObject::connect(this, &SIM::getText, ui, &UI::getText, sim2ui);
    QObject::connect(this, &SIM::show, ui, &UI::show, sim2ui);
    QObject::connect(this, &SIM::close, ui, &UI::close, sim2ui);
    Qt::ConnectionType ui2sim = Qt::AutoConnection;
    QObject::connect(ui, &UI::notifyEvent, this, &SIM::notifyEvent, ui2sim);
}

void SIM::notifyEvent(const QString &msg, int handle)
{
    ASSERT_THREAD(!UI);

    QString xml = "<event origin='codeEditor' msg='%1' handle='%2'/>";
    simEventNotification(xml.arg(msg).arg(handle).toUtf8().data());
}


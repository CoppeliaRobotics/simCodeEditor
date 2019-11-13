#ifndef PLUGIN_H_INCLUDED
#define PLUGIN_H_INCLUDED

#include "config.h"

#define PLUGIN_NAME "CodeEditor"
#define PLUGIN_VERSION 1

#include <QString>
#include <QUrl>

QUrl apiReferenceForSymbol(const QString &sym);

#endif // PLUGIN_H_INCLUDED


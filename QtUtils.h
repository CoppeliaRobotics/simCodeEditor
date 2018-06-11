#ifndef QTUTILS_H__INCLUDED
#define QTUTILS_H__INCLUDED

#include <QString>
#include <QColor>

char * stringBufferCopy(const QString &str);
QColor parseColor(const QString &colorStr);
bool parseBool(const QString &boolStr);

#endif // QTUTILS_H__INCLUDED

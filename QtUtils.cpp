#include "QtUtils.h"
#include "v_repLib.h"
#include <QByteArray>
#include <QStringList>

char * stringBufferCopy(const QString &str)
{
    QByteArray byteArr = str.toLocal8Bit();
    char *buff = simCreateBuffer(byteArr.length() + 1);
    strcpy(buff, byteArr.data());
    buff[byteArr.length()] = '\0';
    return buff;
}

QColor parseColor(const QString &colorStr)
{
    QColor ret;
    QStringList colorStrLst = colorStr.split(" ");
    ret.setRed(colorStrLst[0].toInt());
    ret.setGreen(colorStrLst[1].toInt());
    ret.setBlue(colorStrLst[2].toInt());
    return ret;
}

bool parseBool(const QString &boolStr)
{
    return boolStr != "false";
}


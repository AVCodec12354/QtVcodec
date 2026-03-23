#ifndef QT_LOG_HELPER_H
#define QT_LOG_HELPER_H

#include <QTextBrowser>
#include <QTime>
#include <QTextDocument>

#pragma once
#include <QString>

class QTextBrowser;

class QTLogger {
public:
    enum Level {
        VERBOSE,
        DEBUG,
        INFO,
        WARN,
        ERROR
    };

    static void setOutput(QTextBrowser* browser);
    static void log(Level level, const QString& msg);

private:
    static QTextBrowser* m_browser;
    static QString levelToColor(Level level);
    static QString levelToString(Level level);
};

#define QTVerbose(tag, msg) QTLogger::log(QTLogger::VERBOSE, QString("[%1] %2").arg(tag).arg(msg))
#define QTDebug(tag, msg)   QTLogger::log(QTLogger::DEBUG, QString("[%1] %2").arg(tag).arg(msg))
#define QTInfo(tag, msg)    QTLogger::log(QTLogger::INFO, QString("[%1] %2").arg(tag).arg(msg))
#define QTWarn(tag, msg)    QTLogger::log(QTLogger::WARN, QString("[%1] %2").arg(tag).arg(msg))
#define QTError(tag, msg)   QTLogger::log(QTLogger::ERROR, QString("[%1] %2").arg(tag).arg(msg))

#endif // QT_LOG_HELPER_H
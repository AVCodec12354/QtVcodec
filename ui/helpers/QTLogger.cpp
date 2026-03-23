#include "QTLogger.h"
#include <QTextBrowser>
#include <QScrollBar>
#include <QTime>

QTextBrowser* QTLogger::m_browser = nullptr;

void QTLogger::setOutput(QTextBrowser* browser) {
    m_browser = browser;

    if (m_browser) {
        m_browser->document()->setMaximumBlockCount(1000);
    }
}

QString QTLogger::levelToString(Level level) {
    switch (level) {
        case VERBOSE: return "VERBOSE";
        case DEBUG:   return "DEBUG";
        case INFO:    return "INFO";
        case WARN:    return "WARN";
        case ERROR:   return "ERROR";
    }
    return "";
}

QString QTLogger::levelToColor(Level level) {
    switch (level) {
        case VERBOSE: return "gray";
        case DEBUG:   return "deepskyblue";
        case INFO:    return "limegreen";
        case WARN:    return "orange";
        case ERROR:   return "red";
    }
    return "black";
}

void QTLogger::log(Level level, const QString& msg) {
    if (!m_browser) return;

    QString time = QTime::currentTime().toString("hh:mm:ss");
    QString levelStr = levelToString(level);
    QString color = levelToColor(level);

    QString formatted = QString(
            "<span style='color:%1;'>[%2] [%3] %4</span>"
    ).arg(color, time, levelStr, msg);

    m_browser->append(formatted);

    // auto scroll
    m_browser->verticalScrollBar()->setValue(
            m_browser->verticalScrollBar()->maximum()
    );
}
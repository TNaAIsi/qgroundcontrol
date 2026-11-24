#pragma once

#include <QtCore/QLoggingCategory>
#include <QtQml/QQmlAbstractUrlInterceptor>

#include "QGCCorePlugin.h"

class QQmlApplicationEngine;

Q_DECLARE_LOGGING_CATEGORY(BGCPluginLog)

class BGCOverrideInterceptor : public QQmlAbstractUrlInterceptor
{
public:
    BGCOverrideInterceptor() = default;
    QUrl intercept(const QUrl &url, QQmlAbstractUrlInterceptor::DataType type) final;
};

class BGCPlugin : public QGCCorePlugin
{
    Q_OBJECT

public:
    explicit BGCPlugin(QObject *parent = nullptr);
    ~BGCPlugin() override;

    static QGCCorePlugin *instance();

    void paletteOverride(const QString &colorName, QGCPalette::PaletteColorInfo_t &colorInfo) final;
    QQmlApplicationEngine *createQmlApplicationEngine(QObject *parent) final;

private:
    BGCOverrideInterceptor *_qmlInterceptor = nullptr;
};

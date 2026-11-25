#include "BGCPlugin.h"

#include <QtCore/QFile>
#include <QtQml/QQmlApplicationEngine>

#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(BGCPluginLog, "BGC.Plugin")

Q_APPLICATION_STATIC(BGCPlugin, _bgcPluginInstance);

BGCPlugin::BGCPlugin(QObject *parent)
    : QGCCorePlugin(parent)
{
    // Keep advanced UI enabled so Analyze View (MAVLink Inspector) remains accessible.
    _showAdvancedUI = true;
}

BGCPlugin::~BGCPlugin()
{
    delete _qmlInterceptor;
    _qmlInterceptor = nullptr;
}

QGCCorePlugin *BGCPlugin::instance()
{
    return _bgcPluginInstance();
}

void BGCPlugin::paletteOverride(const QString &colorName, QGCPalette::PaletteColorInfo_t &colorInfo)
{
    if (colorName == QStringLiteral("brandPrimary")) {
        // Dark neutral slate matching BH branding instead of QGC purple.
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor(0x1B, 0x1F, 0x24);
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor(0x1B, 0x1F, 0x24);
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor(0x1B, 0x1F, 0x24);
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor(0x1B, 0x1F, 0x24);
    } else if (colorName == QStringLiteral("brandAccent")) {
        // BH crimson accent.
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor(0x90, 0x1F, 0x2C);
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor(0x6E, 0x22, 0x2B);
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor(0x90, 0x1F, 0x2C);
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor(0x6E, 0x22, 0x2B);
    }
}

QQmlApplicationEngine *BGCPlugin::createQmlApplicationEngine(QObject *parent)
{
    QQmlApplicationEngine *qmlEngine = QGCCorePlugin::createQmlApplicationEngine(parent);
    if (!qmlEngine) {
        return nullptr;
    }

    if (!_qmlInterceptor) {
        _qmlInterceptor = new BGCOverrideInterceptor();
    }
    qmlEngine->addUrlInterceptor(_qmlInterceptor);
    return qmlEngine;
}

QUrl BGCOverrideInterceptor::intercept(const QUrl &url, QQmlAbstractUrlInterceptor::DataType type)
{
    Q_UNUSED(type);

    if (url.scheme() == QStringLiteral("qrc")) {
        const QString path = url.path();
        if (path.startsWith(QStringLiteral("/res/"))) {
            const QString customPath = QStringLiteral(":/Custom") + path;
            if (QFile::exists(customPath)) {
                return QUrl(QStringLiteral("qrc:/Custom") + path);
            }
        }
    }

    return url;
}

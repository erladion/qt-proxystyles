#include "ModernStyle.h"
#include "WinUIStyle.h"
#include "Windows10Style.h"

#include <QStylePlugin>

class ModernStylePlugin : public QStylePlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID QStyleFactoryInterface_iid FILE "modernstyle.json")

public:
  QStyle* create(const QString& key) override {
    const QString name = key.toLower();
    if (name == QLatin1String("modern")) {
      return new ModernStyle(ModernStyle::Theme::System);
    }
    if (name == QLatin1String("modern-light")) {
      return new ModernStyle(ModernStyle::Theme::Light);
    }
    if (name == QLatin1String("modern-dark")) {
      return new ModernStyle(ModernStyle::Theme::Dark);
    }
    if (name == QLatin1String("winui")) {
      return new WinUIStyle(WinUIStyle::Theme::System);
    }
    if (name == QLatin1String("winui-light")) {
      return new WinUIStyle(WinUIStyle::Theme::Light);
    }
    if (name == QLatin1String("winui-dark")) {
      return new WinUIStyle(WinUIStyle::Theme::Dark);
    }
    if (name == QLatin1String("windows10")) {
      return new Windows10Style(Windows10Style::Theme::System);
    }
    if (name == QLatin1String("windows10-light")) {
      return new Windows10Style(Windows10Style::Theme::Light);
    }
    if (name == QLatin1String("windows10-dark")) {
      return new Windows10Style(Windows10Style::Theme::Dark);
    }
    return nullptr;
  }
};

#include "ModernStylePlugin.moc"

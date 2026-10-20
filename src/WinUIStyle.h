#pragma once

#include <QColor>
#include <QList>
#include <QPointer>
#include <QProxyStyle>

class QTimer;

/*
  Widget style that mimics WinUI 3 (Fluent Design, Windows 11). Works with Qt 5.12+ and Qt 6.

  Colors follow the WinUI theme resources: translucent control fills layered over the
  window, elevation borders with a darker (light theme) or lighter (dark theme) edge,
  accent-colored primary buttons, pill selection indicators and rounded flyouts.
  The accent color is the base system accent; the light and dark shades WinUI uses are
  derived from it.
*/
class WinUIStyle : public QProxyStyle {
  Q_OBJECT

public:
  enum class Theme { System, Light, Dark };

  explicit WinUIStyle(Theme theme = Theme::System, const QColor& accentColor = QColor());

  Theme theme() const;
  void setTheme(Theme theme);

  QColor accentColor() const;
  void setAccentColor(const QColor& color);

  // Rounded menus, combo popups and tooltips need a compositor; off by default on X11
  bool translucentPopups() const;
  void setTranslucentPopups(bool enabled);

  // Resolved theme, taking the system color scheme into account
  bool isDark() const;

  // Segoe UI Variable at 14px when available, as WinUI uses
  static QFont recommendedFont();

  QPalette standardPalette() const override;

  using QProxyStyle::polish;
  using QProxyStyle::unpolish;
  void polish(QPalette& palette) override;
  void polish(QApplication* application) override;
  void unpolish(QApplication* application) override;
  void polish(QWidget* widget) override;
  void unpolish(QWidget* widget) override;

  bool eventFilter(QObject* watched, QEvent* event) override;

  void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter,
                     const QWidget* widget = nullptr) const override;
  void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter,
                   const QWidget* widget = nullptr) const override;
  void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter,
                          const QWidget* widget = nullptr) const override;

  QRect subControlRect(ComplexControl control, const QStyleOptionComplex* option, SubControl subControl,
                       const QWidget* widget = nullptr) const override;
  QRect subElementRect(SubElement element, const QStyleOption* option,
                       const QWidget* widget = nullptr) const override;
  QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size,
                         const QWidget* widget = nullptr) const override;

  int pixelMetric(PixelMetric metric, const QStyleOption* option = nullptr,
                  const QWidget* widget = nullptr) const override;
  int styleHint(StyleHint hint, const QStyleOption* option = nullptr, const QWidget* widget = nullptr,
                QStyleHintReturn* returnData = nullptr) const override;

  QIcon standardIcon(StandardPixmap pixmapType, const QStyleOption* option = nullptr,
                     const QWidget* widget = nullptr) const override;
  QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap& pixmap,
                              const QStyleOption* option) const override;

private:
  void applyPalette(bool accentChanged = false);
  void scheduleAnimationFrame(const QWidget* widget) const;

  void drawButton(const QStyleOption* option, QPainter* painter, bool isAccent, bool isSubtle) const;
  void drawTextField(const QStyleOption* option, QPainter* painter, bool fill) const;
  void drawCheckIndicator(const QStyleOption* option, QPainter* painter, bool isRadio) const;
  void drawItemViewBackground(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
  void drawMenuItem(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
  void drawTabShape(const QStyleOption* option, QPainter* painter) const;
  void drawHeaderSection(const QStyleOption* option, QPainter* painter) const;
  void drawProgressBar(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
  void drawSpinBox(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const;
  void drawComboBox(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const;
  void drawScrollBar(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const;
  void drawSlider(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const;

  Theme m_theme;
  QColor m_accentColor;
  // The palette this style last supplied, to tell its colors from ones the application set
  QPalette m_appliedPalette;
  bool m_translucentPopups;
  bool m_systemIsDark;

  QTimer* m_pAnimationTimer;
  mutable QList<QPointer<QWidget>> m_animatedWidgets;
};

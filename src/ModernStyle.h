#pragma once

#include <QColor>
#include <QList>
#include <QPointer>
#include <QProxyStyle>

class QTimer;

/*
  Flat, rounded widget style built on top of Fusion. Works with Qt 5.12+ and Qt 6.

  All drawing derives its colors from the widget palette, with Highlight acting
  as the accent color. The style ships its own light and dark palettes, which it
  installs as the application palette when set via QApplication::setStyle().
*/
class ModernStyle : public QProxyStyle {
  Q_OBJECT

public:
  enum class Theme { System, Light, Dark };

  explicit ModernStyle(Theme theme = Theme::System, const QColor& accentColor = QColor());

  Theme theme() const;
  void setTheme(Theme theme);

  QColor accentColor() const;
  void setAccentColor(const QColor& color);

  int cornerRadius() const;
  void setCornerRadius(int radius);

  // Resolved theme, taking the system color scheme into account
  bool isDark() const;

  QPalette standardPalette() const override;

  using QProxyStyle::polish;
  using QProxyStyle::unpolish;
  void polish(QPalette& palette) override;
  void polish(QApplication* application) override;
  void polish(QWidget* widget) override;
  void unpolish(QWidget* widget) override;

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

  QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap& pixmap,
                              const QStyleOption* option) const override;

private:
  void applyPalette(bool accentChanged = false);
  void scheduleAnimationFrame(const QWidget* widget) const;

  void drawButtonPanel(const QStyleOption* option, QPainter* painter, bool isDefault, bool isFlat) const;
  void drawToolPanel(const QStyleOption* option, QPainter* painter) const;
  void drawFieldFrame(const QStyleOption* option, QPainter* painter, bool fill) const;
  void drawCheckIndicator(const QStyleOption* option, QPainter* painter, bool isRadio) const;
  void drawItemViewBackground(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
  void drawTabShape(const QStyleOption* option, QPainter* painter) const;
  void drawHeaderSection(const QStyleOption* option, QPainter* painter) const;
  void drawProgressContents(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
  void drawSpinBox(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const;
  void drawComboBox(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const;
  void drawScrollBar(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const;
  void drawSlider(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const;

  Theme m_theme;
  QColor m_accentColor;
  // The palette this style last supplied, to tell its colors from ones the application set
  QPalette m_appliedPalette;
  int m_cornerRadius;
  bool m_systemIsDark;

  QTimer* m_pAnimationTimer;
  mutable QList<QPointer<QWidget>> m_animatedWidgets;
};

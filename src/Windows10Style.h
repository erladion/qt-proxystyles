#pragma once

#include <QColor>
#include <QList>
#include <QPointer>
#include <QProxyStyle>

class QTimer;

/*
  Widget style that mimics the Windows 10 desktop (Win32) controls, as seen in Explorer,
  Control Panel and classic dialogs. Works with Qt 5.12+ and Qt 6, on any platform.

  Square controls with thin borders, light-blue hover and pressed states, Explorer-style
  item selection, green progress bars and blue trackbar thumbs. The light theme follows the
  stock Windows 10 control theme; the dark theme approximates Explorer's dark mode.
  Win32 controls don't follow the system accent, so the accent defaults to #0078D7.
*/
class Windows10Style : public QProxyStyle {
  Q_OBJECT

public:
  enum class Theme { System, Light, Dark };

  explicit Windows10Style(Theme theme = Theme::System, const QColor& accentColor = QColor());

  Theme theme() const;
  void setTheme(Theme theme);

  QColor accentColor() const;
  void setAccentColor(const QColor& color);

  // Resolved theme, taking the system color scheme into account
  bool isDark() const;

  // Segoe UI 9pt when available, the Windows 10 dialog font
  static QFont recommendedFont();

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

  void drawPushButton(const QStyleOption* option, QPainter* painter, bool isDefault, bool isFlat) const;
  void drawToolButtonPanel(const QStyleOption* option, QPainter* painter) const;
  void drawEditFrame(const QStyleOption* option, QPainter* painter, bool fill) const;
  void drawCheckIndicator(const QStyleOption* option, QPainter* painter, bool isRadio) const;
  void drawItemViewBackground(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
  void drawMenuItem(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
  void drawTabShape(const QStyleOption* option, QPainter* painter) const;
  void drawHeaderSection(const QStyleOption* option, QPainter* painter) const;
  void drawProgressContents(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
  void drawSpinBox(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const;
  void drawComboBox(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const;
  void drawScrollBar(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const;
  void drawSlider(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const;
  void drawGroupBox(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const;

  Theme m_theme;
  QColor m_accentColor;
  // The palette this style last supplied, to tell its colors from ones the application set
  QPalette m_appliedPalette;
  bool m_systemIsDark;

  QTimer* m_pAnimationTimer;
  mutable QList<QPointer<QWidget>> m_animatedWidgets;
};

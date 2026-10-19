#include "WinUIStyle.h"

#include "StyleHelpers.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QFontDatabase>
#include <QFrame>
#include <QGroupBox>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QProgressBar>
#include <QSlider>
#include <QSplitterHandle>
#include <QStyleFactory>
#include <QStyleHints>
#include <QStyleOption>
#include <QTabBar>
#include <QTime>
#include <QTreeView>
#include <QTimer>
#include <QToolTip>

using namespace StyleHelpers;

namespace {

constexpr int CONTROL_RADIUS = 4;
constexpr int OVERLAY_RADIUS = 8;
constexpr int CONTROL_HEIGHT = 32;
constexpr int FRAME_WIDTH = 2;
constexpr int FIELD_BORDER = 1;
constexpr int TEXT_PADDING_LEFT = 10;
constexpr int TEXT_PADDING_RIGHT = 6;
constexpr int BUTTON_PADDING = 11;
constexpr int COMBO_ARROW_WIDTH = 36;
constexpr int SPIN_BUTTON_WIDTH = 30;
constexpr int INDICATOR_SIZE = 20;
constexpr int SLIDER_THUMB_SIZE = 20;
constexpr int SLIDER_TRACK_THICKNESS = 4;
constexpr int SCROLLBAR_EXTENT = 14;
constexpr int TAB_TOP_MARGIN = 4;
constexpr int TAB_MIN_WIDTH = 100;
constexpr int MENU_ITEM_INSET_H = 4;
constexpr int MENU_ITEM_INSET_V = 2;
constexpr int MENU_CHECK_SIZE = 16;
constexpr int MENU_COLUMN_SPACING = 12;
constexpr int MENU_SHORTCUT_GAP = 24;
constexpr int SELECTION_PILL_WIDTH = 3;
constexpr int SELECTION_PILL_HEIGHT = 16;
constexpr int ANIMATION_INTERVAL_MS = 16;
constexpr int BUSY_CYCLE_MS = 2000;

// WinUI 3 theme resources (Common_themeresources_any.xaml), named after their XAML keys
struct Tokens {
  QColor m_textPrimary;
  QColor m_textSecondary;
  QColor m_textTertiary;
  QColor m_textDisabled;
  QColor m_controlFill;
  QColor m_controlFillSecondary;
  QColor m_controlFillTertiary;
  QColor m_controlFillDisabled;
  QColor m_controlFillInputActive;
  QColor m_controlStroke;
  QColor m_controlStrokeSecondary;
  QColor m_controlStrongStroke;
  QColor m_controlStrongStrokeDisabled;
  QColor m_controlAltFillSecondary;
  QColor m_controlAltFillTertiary;
  QColor m_controlAltFillQuarternary;
  QColor m_controlStrongFill;
  QColor m_controlStrongFillDisabled;
  QColor m_controlSolidFill;
  QColor m_subtleFillSecondary;
  QColor m_subtleFillTertiary;
  QColor m_divider;
  QColor m_cardFill;
  QColor m_cardStroke;
  QColor m_solidBackgroundTertiary;
  QColor m_flyoutFill;
  QColor m_flyoutStroke;
  QColor m_focusOuter;
  QColor m_focusInner;
  QColor m_textOnAccent;
  QColor m_textOnAccentSecondary;
  QColor m_textOnAccentDisabled;
  QColor m_accentDisabled;
  QColor m_strokeOnAccent;
  QColor m_strokeOnAccentSecondary;
  // Light theme darkens the bottom edge of raised controls; dark theme lightens the top
  bool m_elevationEdgeAtTop;
};

QColor argb(QRgb value) {
  return QColor::fromRgba(value);
}

Tokens makeLightTokens() {
  Tokens tokens;
  tokens.m_textPrimary = argb(0xE4000000);
  tokens.m_textSecondary = argb(0x9E000000);
  tokens.m_textTertiary = argb(0x72000000);
  tokens.m_textDisabled = argb(0x5C000000);
  tokens.m_controlFill = argb(0xB3FFFFFF);
  tokens.m_controlFillSecondary = argb(0x80F9F9F9);
  tokens.m_controlFillTertiary = argb(0x4DF9F9F9);
  tokens.m_controlFillDisabled = argb(0x4DF9F9F9);
  tokens.m_controlFillInputActive = argb(0xFFFFFFFF);
  tokens.m_controlStroke = argb(0x0F000000);
  tokens.m_controlStrokeSecondary = argb(0x29000000);
  tokens.m_controlStrongStroke = argb(0x72000000);
  tokens.m_controlStrongStrokeDisabled = argb(0x37000000);
  tokens.m_controlAltFillSecondary = argb(0x06000000);
  tokens.m_controlAltFillTertiary = argb(0x0F000000);
  tokens.m_controlAltFillQuarternary = argb(0x18000000);
  tokens.m_controlStrongFill = argb(0x72000000);
  tokens.m_controlStrongFillDisabled = argb(0x51000000);
  tokens.m_controlSolidFill = argb(0xFFFFFFFF);
  tokens.m_subtleFillSecondary = argb(0x09000000);
  tokens.m_subtleFillTertiary = argb(0x06000000);
  tokens.m_divider = argb(0x14000000);
  tokens.m_cardFill = argb(0xB3FFFFFF);
  tokens.m_cardStroke = argb(0x0F000000);
  tokens.m_solidBackgroundTertiary = argb(0xFFF9F9F9);
  tokens.m_flyoutFill = argb(0xFFFCFCFC);
  // Stronger than WinUI's, which relies on a drop shadow Qt can't draw
  tokens.m_flyoutStroke = argb(0x24000000);
  tokens.m_focusOuter = argb(0xE4000000);
  tokens.m_focusInner = argb(0xB3FFFFFF);
  tokens.m_textOnAccent = argb(0xFFFFFFFF);
  tokens.m_textOnAccentSecondary = argb(0xB3FFFFFF);
  tokens.m_textOnAccentDisabled = argb(0xFFFFFFFF);
  tokens.m_accentDisabled = argb(0x37000000);
  tokens.m_strokeOnAccent = argb(0x14FFFFFF);
  tokens.m_strokeOnAccentSecondary = argb(0x66000000);
  tokens.m_elevationEdgeAtTop = false;
  return tokens;
}

Tokens makeDarkTokens() {
  Tokens tokens;
  tokens.m_textPrimary = argb(0xFFFFFFFF);
  tokens.m_textSecondary = argb(0xC5FFFFFF);
  tokens.m_textTertiary = argb(0x87FFFFFF);
  tokens.m_textDisabled = argb(0x5DFFFFFF);
  tokens.m_controlFill = argb(0x0FFFFFFF);
  tokens.m_controlFillSecondary = argb(0x15FFFFFF);
  tokens.m_controlFillTertiary = argb(0x08FFFFFF);
  tokens.m_controlFillDisabled = argb(0x0BFFFFFF);
  tokens.m_controlFillInputActive = argb(0xB31E1E1E);
  tokens.m_controlStroke = argb(0x12FFFFFF);
  tokens.m_controlStrokeSecondary = argb(0x18FFFFFF);
  tokens.m_controlStrongStroke = argb(0x8BFFFFFF);
  tokens.m_controlStrongStrokeDisabled = argb(0x28FFFFFF);
  tokens.m_controlAltFillSecondary = argb(0x19000000);
  tokens.m_controlAltFillTertiary = argb(0x0BFFFFFF);
  tokens.m_controlAltFillQuarternary = argb(0x12FFFFFF);
  tokens.m_controlStrongFill = argb(0x8BFFFFFF);
  tokens.m_controlStrongFillDisabled = argb(0x3FFFFFFF);
  tokens.m_controlSolidFill = argb(0xFF454545);
  tokens.m_subtleFillSecondary = argb(0x0FFFFFFF);
  tokens.m_subtleFillTertiary = argb(0x0AFFFFFF);
  tokens.m_divider = argb(0x15FFFFFF);
  tokens.m_cardFill = argb(0x0DFFFFFF);
  tokens.m_cardStroke = argb(0x19000000);
  tokens.m_solidBackgroundTertiary = argb(0xFF282828);
  tokens.m_flyoutFill = argb(0xFF2C2C2C);
  tokens.m_flyoutStroke = argb(0x66000000);
  tokens.m_focusOuter = argb(0xFFFFFFFF);
  tokens.m_focusInner = argb(0xB3000000);
  tokens.m_textOnAccent = argb(0xFF000000);
  tokens.m_textOnAccentSecondary = argb(0x80000000);
  tokens.m_textOnAccentDisabled = argb(0x87FFFFFF);
  tokens.m_accentDisabled = argb(0x28FFFFFF);
  tokens.m_strokeOnAccent = argb(0x14FFFFFF);
  tokens.m_strokeOnAccentSecondary = argb(0x23000000);
  tokens.m_elevationEdgeAtTop = true;
  return tokens;
}

const Tokens& tokensFor(bool dark) {
  static const Tokens light = makeLightTokens();
  static const Tokens darkTokens = makeDarkTokens();
  return dark ? darkTokens : light;
}

const Tokens& tokensFor(const QStyleOption* option) {
  return tokensFor(isDarkPalette(option->palette));
}

QColor defaultAccent() {
  return QColor(0x00, 0x78, 0xD4);
}

QColor systemAccent() {
#if defined(Q_OS_WIN) && QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
  if (qGuiApp) {
    const QColor accent = QGuiApplication::palette().color(QPalette::Accent);
    if (accent.isValid()) {
      return accent;
    }
  }
#endif
  return defaultAccent();
}

QColor withLightness(const QColor& color, qreal lightness) {
  const QColor hsl = color.toHsl();
  return QColor::fromHslF(hsl.hslHueF(), hsl.hslSaturationF(), qBound<qreal>(0.0, lightness, 1.0));
}

// Approximates SystemAccentColorDark1 (light theme) and SystemAccentColorLight2 (dark theme)
QColor themeAccent(const QColor& base, bool dark) {
  const qreal lightness = base.toHsl().lightnessF();
  return withLightness(base, dark ? lightness + (1.0 - lightness) * 0.47 : lightness * 0.87);
}

// Approximates AccentTextFillColorPrimary, used for hyperlinks
QColor accentText(const QColor& base, bool dark) {
  const qreal lightness = base.toHsl().lightnessF();
  return withLightness(base, dark ? lightness + (1.0 - lightness) * 0.66 : lightness * 0.69);
}

QColor optionAccent(const QStyleOption* option) {
  return option->palette.color(QPalette::Active, QPalette::Highlight);
}

// AccentFillColorDefault / Secondary / Tertiary
QColor accentFill(const QStyleOption* option, bool hovered, bool pressed) {
  const QColor accent = optionAccent(option);
  if (pressed) {
    return withAlpha(accent, 0.8);
  }
  if (hovered) {
    return withAlpha(accent, 0.9);
  }
  return accent;
}

/*
  Rounded rect with a 1px border whose top or bottom edge can use a separate color and
  width. This mirrors WinUI's ControlElevationBorderBrush and TextControlElevationBorderBrush.
  The fill and the border are painted as separate, non-overlapping paths, so translucent
  colors don't add up.
*/
void drawElevatedRect(QPainter* painter, const QRectF& rect, qreal radius, const QColor& fill, const QColor& border,
                      const QColor& edge = QColor(), qreal edgeWidth = 1.0, bool edgeAtTop = false) {
  const bool hasEdge = edge.isValid() && edge.alpha() > 0;
  const qreal topWidth = (hasEdge && edgeAtTop) ? edgeWidth : 1.0;
  const qreal bottomWidth = (hasEdge && !edgeAtTop) ? edgeWidth : 1.0;
  const qreal innerRadius = qMax<qreal>(0.0, radius - 1.0);

  QPainterPath outer;
  outer.addRoundedRect(rect, radius, radius);
  QPainterPath inner;
  inner.addRoundedRect(rect.adjusted(1.0, topWidth, -1.0, -bottomWidth), innerRadius, innerRadius);
  const QPainterPath ring = outer.subtracted(inner);

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, true);
  if (fill.isValid() && fill.alpha() > 0) {
    painter->fillPath(inner, fill);
  }
  if (hasEdge) {
    const QRectF bandRect = edgeAtTop ? QRectF(rect.left(), rect.top(), rect.width(), topWidth)
                                      : QRectF(rect.left(), rect.bottom() - bottomWidth, rect.width(), bottomWidth);
    QPainterPath band;
    band.addRect(bandRect);
    if (border.isValid() && border.alpha() > 0) {
      painter->fillPath(ring.subtracted(band), border);
    }
    painter->fillPath(ring.intersected(band), edge);
  } else if (border.isValid() && border.alpha() > 0) {
    painter->fillPath(ring, border);
  }
  painter->restore();
}

// WinUI's focus visual: 2px outer stroke with a 1px inner contrast stroke
void drawFocusRing(QPainter* painter, const QRectF& rect, qreal radius, const Tokens& tokens) {
  fillRoundedRect(painter, rect, radius, Qt::NoBrush, tokens.m_focusOuter, 2.0);
  fillRoundedRect(painter, rect.adjusted(2, 2, -2, -2), qMax<qreal>(0.0, radius - 2.0), Qt::NoBrush,
                  tokens.m_focusInner, 1.0);
}

void drawCheckmark(QPainter* painter, const QRectF& box, const QColor& color, qreal penWidth) {
  const qreal size = box.width();
  const QPointF points[3] = {
    QPointF(box.left() + size * 0.22, box.top() + size * 0.53),
    QPointF(box.left() + size * 0.41, box.top() + size * 0.71),
    QPointF(box.left() + size * 0.78, box.top() + size * 0.32),
  };
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, true);
  QPen pen(color, penWidth);
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::RoundJoin);
  painter->setPen(pen);
  painter->drawPolyline(points, 3);
  painter->restore();
}

void drawSelectionPill(QPainter* painter, const QRectF& itemRect, bool atRight, const QColor& accent) {
  const qreal height = qMin<qreal>(SELECTION_PILL_HEIGHT, itemRect.height() - 8.0);
  if (height <= 0) {
    return;
  }
  const qreal x = atRight ? itemRect.right() - SELECTION_PILL_WIDTH : itemRect.left();
  const QRectF pill(x, itemRect.center().y() - height / 2.0, SELECTION_PILL_WIDTH, height);
  fillRoundedRect(painter, pill, SELECTION_PILL_WIDTH / 2.0, accent);
}

bool isTranslucentWindow(const QWidget* widget) {
  return widget && widget->window()->testAttribute(Qt::WA_TranslucentBackground);
}

bool isComboPopupContainer(const QWidget* widget) {
  return widget && widget->inherits("QComboBoxPrivateContainer");
}

// Frameless line edits inside spin boxes and combos let their parent's field show through
bool isEmbeddedLineEdit(const QWidget* widget) {
  const QWidget* parent = widget ? widget->parentWidget() : nullptr;
  return qobject_cast<const QAbstractSpinBox*>(parent) || qobject_cast<const QComboBox*>(parent);
}

// Tab area below the top margin, where labels and buttons are centered
QRect tabContentRect(const QStyleOptionTab* tab) {
  switch (tab->shape) {
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
      return tab->rect.adjusted(0, 0, 0, -TAB_TOP_MARGIN);
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
      return tab->rect.adjusted(TAB_TOP_MARGIN, 0, 0, 0);
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
      return tab->rect.adjusted(0, 0, -TAB_TOP_MARGIN, 0);
    default:
      return tab->rect.adjusted(0, TAB_TOP_MARGIN, 0, 0);
  }
}

QIcon clearButtonIcon(const QColor& color) {
  QIcon icon;
  for (int size : {16, 32}) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    const qreal scale = size / 16.0;
    QPen pen(color, 1.2 * scale);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.drawLine(QPointF(5, 5) * scale, QPointF(11, 11) * scale);
    painter.drawLine(QPointF(11, 5) * scale, QPointF(5, 11) * scale);
    painter.end();
    icon.addPixmap(pixmap);
  }
  return icon;
}

bool wantsHover(const QWidget* widget) {
  return qobject_cast<const QAbstractButton*>(widget) || qobject_cast<const QComboBox*>(widget) ||
         qobject_cast<const QAbstractSpinBox*>(widget) || qobject_cast<const QAbstractSlider*>(widget) ||
         qobject_cast<const QTabBar*>(widget) || qobject_cast<const QLineEdit*>(widget) ||
         qobject_cast<const QSplitterHandle*>(widget) || qobject_cast<const QGroupBox*>(widget);
}

} // namespace

WinUIStyle::WinUIStyle(Theme theme, const QColor& accentColor)
    : QProxyStyle(QStyleFactory::create(QStringLiteral("Fusion"))),
      m_theme(theme),
      m_accentColor(accentColor.isValid() ? accentColor : systemAccent()),
      m_translucentPopups(QGuiApplication::platformName() != QLatin1String("xcb")),
      m_systemIsDark(false),
      m_pAnimationTimer(new QTimer(this)) {
  setObjectName(QStringLiteral("WinUI"));

  // Fallback for Qt versions without QStyleHints::colorScheme()
  if (qGuiApp) {
    m_systemIsDark = isDarkPalette(QGuiApplication::palette());
  }

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  if (qGuiApp) {
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this]() {
      if (m_theme == Theme::System) {
        applyPalette();
      }
    });
  }
#endif

  m_pAnimationTimer->setInterval(ANIMATION_INTERVAL_MS);
  m_pAnimationTimer->setSingleShot(true);
  connect(m_pAnimationTimer, &QTimer::timeout, this, [this]() {
    // Widgets re-register on their next paint while they still animate
    const QList<QPointer<QWidget>> widgets = m_animatedWidgets;
    m_animatedWidgets.clear();
    for (const QPointer<QWidget>& widget : widgets) {
      if (widget) {
        widget->update();
      }
    }
  });

  m_appliedPalette = standardPalette();
}

WinUIStyle::Theme WinUIStyle::theme() const {
  return m_theme;
}

void WinUIStyle::setTheme(Theme theme) {
  if (m_theme == theme) {
    return;
  }
  m_theme = theme;
  applyPalette();
}

QColor WinUIStyle::accentColor() const {
  return m_accentColor;
}

void WinUIStyle::setAccentColor(const QColor& color) {
  const QColor accent = color.isValid() ? color : defaultAccent();
  if (m_accentColor == accent) {
    return;
  }
  m_accentColor = accent;
  applyPalette(true);
}

bool WinUIStyle::translucentPopups() const {
  return m_translucentPopups;
}

void WinUIStyle::setTranslucentPopups(bool enabled) {
  m_translucentPopups = enabled;
}

bool WinUIStyle::isDark() const {
  if (m_theme != Theme::System) {
    return m_theme == Theme::Dark;
  }
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  if (qGuiApp) {
    const Qt::ColorScheme scheme = QGuiApplication::styleHints()->colorScheme();
    if (scheme != Qt::ColorScheme::Unknown) {
      return scheme == Qt::ColorScheme::Dark;
    }
  }
#endif
  return m_systemIsDark;
}

QFont WinUIStyle::recommendedFont() {
  QFont font = QGuiApplication::font();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  const QStringList families = QFontDatabase::families();
#else
  const QStringList families = QFontDatabase().families();
#endif
  for (const QString& family : {QStringLiteral("Segoe UI Variable Text"), QStringLiteral("Segoe UI Variable"),
                                QStringLiteral("Segoe UI")}) {
    if (families.contains(family)) {
      font.setFamily(family);
      break;
    }
  }
  font.setPixelSize(14);
  return font;
}

void WinUIStyle::applyPalette(bool accentChanged) {
  if (!qApp || QApplication::style() != this) {
    return;
  }
  // Keep roles the application set itself; only colors that came from this style change
  const QPalette next = standardPalette();
  const QPalette palette = mergeCustomRoles(next, m_appliedPalette, QApplication::palette(), accentChanged);
  m_appliedPalette = next;
  QApplication::setPalette(palette);
  QToolTip::setPalette(palette);
}

void WinUIStyle::scheduleAnimationFrame(const QWidget* widget) const {
  if (!widget) {
    return;
  }
  QWidget* target = const_cast<QWidget*>(widget);
  for (const QPointer<QWidget>& existing : m_animatedWidgets) {
    if (existing == target) {
      return;
    }
  }
  m_animatedWidgets.append(target);
  if (!m_pAnimationTimer->isActive()) {
    m_pAnimationTimer->start();
  }
}

namespace {

QPalette buildPalette(bool dark, const QColor& baseAccent) {
  const QColor accent = themeAccent(baseAccent, dark);
  const QColor link = accentText(baseAccent, dark);

  // Solid equivalents of the translucent WinUI fills, for code that reads the palette directly
  QColor window, text, base, alternateBase, button, placeholder, light, midlight, mid, darkShade, shadow, flyout;
  QColor disabledText, disabledBase, disabledButton, disabledHighlight;
  if (dark) {
    window = QColor(0x20, 0x20, 0x20);
    text = QColor(0xFF, 0xFF, 0xFF);
    base = QColor(0x2B, 0x2B, 0x2B);
    alternateBase = QColor(0x27, 0x27, 0x27);
    button = QColor(0x2D, 0x2D, 0x2D);
    placeholder = QColor(0xCF, 0xCF, 0xCF);
    light = QColor(0x3C, 0x3C, 0x3C);
    midlight = QColor(0x33, 0x33, 0x33);
    mid = QColor(0x45, 0x45, 0x45);
    darkShade = QColor(0x1C, 0x1C, 0x1C);
    shadow = QColor(0, 0, 0, 0xCC);
    flyout = QColor(0x2C, 0x2C, 0x2C);
    disabledText = QColor(0x71, 0x71, 0x71);
    disabledBase = QColor(0x26, 0x26, 0x26);
    disabledButton = QColor(0x2A, 0x2A, 0x2A);
    disabledHighlight = QColor(0x43, 0x43, 0x43);
  } else {
    window = QColor(0xF3, 0xF3, 0xF3);
    text = QColor(0x1A, 0x1A, 0x1A);
    base = QColor(0xFF, 0xFF, 0xFF);
    alternateBase = QColor(0xF9, 0xF9, 0xF9);
    button = QColor(0xFB, 0xFB, 0xFB);
    placeholder = QColor(0x61, 0x61, 0x61);
    light = QColor(0xFF, 0xFF, 0xFF);
    midlight = QColor(0xF9, 0xF9, 0xF9);
    mid = QColor(0xE5, 0xE5, 0xE5);
    darkShade = QColor(0xC4, 0xC4, 0xC4);
    shadow = QColor(0, 0, 0, 0x33);
    flyout = QColor(0xFC, 0xFC, 0xFC);
    disabledText = QColor(0x9C, 0x9C, 0x9C);
    disabledBase = QColor(0xF9, 0xF9, 0xF9);
    disabledButton = QColor(0xF5, 0xF5, 0xF5);
    disabledHighlight = QColor(0xBF, 0xBF, 0xBF);
  }

  QPalette palette;
  palette.setColor(QPalette::All, QPalette::Window, window);
  palette.setColor(QPalette::All, QPalette::WindowText, text);
  palette.setColor(QPalette::All, QPalette::Base, base);
  palette.setColor(QPalette::All, QPalette::AlternateBase, alternateBase);
  palette.setColor(QPalette::All, QPalette::Button, button);
  palette.setColor(QPalette::All, QPalette::ButtonText, text);
  palette.setColor(QPalette::All, QPalette::Text, text);
  palette.setColor(QPalette::All, QPalette::BrightText, QColor(0xFF, 0xFF, 0xFF));
  palette.setColor(QPalette::All, QPalette::Light, light);
  palette.setColor(QPalette::All, QPalette::Midlight, midlight);
  palette.setColor(QPalette::All, QPalette::Mid, mid);
  palette.setColor(QPalette::All, QPalette::Dark, darkShade);
  palette.setColor(QPalette::All, QPalette::Shadow, shadow);
  palette.setColor(QPalette::All, QPalette::Highlight, accent);
  palette.setColor(QPalette::All, QPalette::HighlightedText, dark ? QColor(0, 0, 0) : QColor(0xFF, 0xFF, 0xFF));
  palette.setColor(QPalette::All, QPalette::Link, link);
  palette.setColor(QPalette::All, QPalette::LinkVisited, link);
  palette.setColor(QPalette::All, QPalette::ToolTipBase, flyout);
  palette.setColor(QPalette::All, QPalette::ToolTipText, text);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
  palette.setColor(QPalette::All, QPalette::PlaceholderText, placeholder);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
  palette.setColor(QPalette::All, QPalette::Accent, accent);
#endif

  palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
  palette.setColor(QPalette::Disabled, QPalette::Text, disabledText);
  palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText);
  palette.setColor(QPalette::Disabled, QPalette::Base, disabledBase);
  palette.setColor(QPalette::Disabled, QPalette::Button, disabledButton);
  palette.setColor(QPalette::Disabled, QPalette::Highlight, disabledHighlight);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
  palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, disabledText);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
  palette.setColor(QPalette::Disabled, QPalette::Accent, disabledHighlight);
#endif
  return palette;
}

// The style's own palette per theme: the baseline for telling which roles the
// application or a widget overrides. Accent roles aren't compared, so the default
// accent stands in for any.
const QPalette& basePalette(const QStyleOption* option) {
  static const QPalette light = buildPalette(false, defaultAccent());
  static const QPalette dark = buildPalette(true, defaultAccent());
  return isDarkPalette(option->palette) ? dark : light;
}

} // namespace

QPalette WinUIStyle::standardPalette() const {
  return buildPalette(isDark(), m_accentColor);
}

void WinUIStyle::polish(QPalette& palette) {
  palette = standardPalette();
  m_appliedPalette = palette;
}

void WinUIStyle::polish(QApplication* application) {
  QProxyStyle::polish(application);
  QToolTip::setPalette(standardPalette());
}

void WinUIStyle::polish(QWidget* widget) {
  QProxyStyle::polish(widget);
  if (wantsHover(widget)) {
    widget->setAttribute(Qt::WA_Hover, true);
  }
  if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(widget)) {
    view->viewport()->setAttribute(Qt::WA_Hover, true);
  }
  // Must happen before the popup's native window exists, which polish precedes
  if (m_translucentPopups && (qobject_cast<QMenu*>(widget) || widget->inherits("QTipLabel"))) {
    widget->setAttribute(Qt::WA_TranslucentBackground, true);
  }
  if (QComboBox* combo = qobject_cast<QComboBox*>(widget)) {
    // The popup draws its own flyout background, so the list must not cover the rounded corners
    QAbstractItemView* view = combo->view();
    view->viewport()->setAutoFillBackground(false);
    QWidget* container = view->parentWidget();
    if (m_translucentPopups && container && container->isWindow()) {
      container->setAttribute(Qt::WA_TranslucentBackground, true);
    }
  }
}

void WinUIStyle::unpolish(QWidget* widget) {
  if (wantsHover(widget)) {
    widget->setAttribute(Qt::WA_Hover, false);
  }
  if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(widget)) {
    view->viewport()->setAttribute(Qt::WA_Hover, false);
  }
  if (QComboBox* combo = qobject_cast<QComboBox*>(widget)) {
    combo->view()->viewport()->setAutoFillBackground(true);
  }
  QProxyStyle::unpolish(widget);
}

// ---------------------------------------------------------------------------
// Primitives
// ---------------------------------------------------------------------------

void WinUIStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter,
                               const QWidget* widget) const {
  const Tokens& tokens = tokensFor(option);
  switch (element) {
    case PE_FrameFocusRect:
    case PE_FrameDefaultButton:
    case PE_FrameStatusBarItem:
    case PE_PanelMenuBar:
    case PE_PanelScrollAreaCorner:
      return;

    case PE_PanelButtonCommand: {
      const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option);
      const bool isDefault = button && (button->features & QStyleOptionButton::DefaultButton);
      const bool isFlat = button && (button->features & QStyleOptionButton::Flat);
      drawButton(option, painter, isDefault, isFlat);
      return;
    }

    case PE_PanelButtonTool:
    case PE_PanelButtonBevel:
    case PE_IndicatorButtonDropDown:
      drawButton(option, painter, false, option->state & State_AutoRaise);
      return;

    case PE_PanelLineEdit: {
      const QStyleOptionFrame* frame = qstyleoption_cast<const QStyleOptionFrame*>(option);
      if (frame && frame->lineWidth <= 0) {
        if (!isEmbeddedLineEdit(widget)) {
          painter->fillRect(option->rect, option->palette.brush(colorGroup(option), QPalette::Base));
        }
        return;
      }
      drawTextField(option, painter, true);
      return;
    }

    case PE_FrameLineEdit:
      drawTextField(option, painter, false);
      return;

    case PE_IndicatorCheckBox:
    case PE_IndicatorItemViewItemCheck:
      drawCheckIndicator(option, painter, false);
      return;

    case PE_IndicatorRadioButton:
      drawCheckIndicator(option, painter, true);
      return;

    case PE_Frame:
      if (isComboPopupContainer(widget)) {
        const qreal radius = isTranslucentWindow(widget) ? OVERLAY_RADIUS : 0;
        fillRoundedRect(painter, option->rect, radius,
                        overriddenOr(option, QPalette::Base, basePalette(option), tokens.m_flyoutFill),
                        tokens.m_flyoutStroke);
      } else {
        const QColor fill = viewportColor(widget);
        fillRoundedRect(painter, option->rect, CONTROL_RADIUS, fill.isValid() ? QBrush(fill) : QBrush(Qt::NoBrush),
                        tokens.m_cardStroke);
      }
      return;

    case PE_FrameGroupBox: {
      const QStyleOptionFrame* frame = qstyleoption_cast<const QStyleOptionFrame*>(option);
      if (frame && (frame->features & QStyleOptionFrame::Flat)) {
        painter->fillRect(QRect(option->rect.left(), option->rect.top(), option->rect.width(), 1), tokens.m_divider);
        return;
      }
      // Group boxes become WinUI cards
      fillRoundedRect(painter, option->rect, OVERLAY_RADIUS, tokens.m_cardFill, tokens.m_cardStroke);
      return;
    }

    case PE_FrameTabWidget:
      fillRoundedRect(painter, option->rect, OVERLAY_RADIUS, tokens.m_solidBackgroundTertiary, tokens.m_divider);
      return;

    case PE_FrameTabBarBase:
      if (const QStyleOptionTabBarBase* tabBase = qstyleoption_cast<const QStyleOptionTabBarBase*>(option)) {
        const QRect r = tabBase->rect;
        QRect line(r.left(), r.bottom(), r.width(), 1);
        switch (tabBase->shape) {
          case QTabBar::RoundedSouth:
          case QTabBar::TriangularSouth:
            line = QRect(r.left(), r.top(), r.width(), 1);
            break;
          case QTabBar::RoundedWest:
          case QTabBar::TriangularWest:
            line = QRect(r.right(), r.top(), 1, r.height());
            break;
          case QTabBar::RoundedEast:
          case QTabBar::TriangularEast:
            line = QRect(r.left(), r.top(), 1, r.height());
            break;
          default:
            break;
        }
        painter->fillRect(line, tokens.m_divider);
        return;
      }
      break;

    case PE_PanelMenu: {
      const qreal radius = isTranslucentWindow(widget) ? OVERLAY_RADIUS : 0;
      fillRoundedRect(painter, option->rect, radius,
                      overriddenOr(option, QPalette::Base, basePalette(option), tokens.m_flyoutFill));
      return;
    }

    case PE_FrameMenu: {
      const qreal radius = isTranslucentWindow(widget) ? OVERLAY_RADIUS : 0;
      fillRoundedRect(painter, option->rect, radius, Qt::NoBrush, tokens.m_flyoutStroke);
      return;
    }

    case PE_PanelTipLabel: {
      const qreal radius = isTranslucentWindow(widget) ? CONTROL_RADIUS : 0;
      fillRoundedRect(painter, option->rect, radius, option->palette.color(QPalette::ToolTipBase),
                      tokens.m_flyoutStroke);
      return;
    }

    case PE_PanelItemViewItem:
      drawItemViewBackground(option, painter, widget);
      return;

    case PE_PanelItemViewRow:
      if (const QStyleOptionViewItem* item = qstyleoption_cast<const QStyleOptionViewItem*>(option)) {
        if (item->features & QStyleOptionViewItem::Alternate) {
          painter->fillRect(item->rect, item->palette.brush(colorGroup(item), QPalette::AlternateBase));
        }
        return;
      }
      break;

    case PE_IndicatorBranch: {
      if (!(option->state & State_Children)) {
        return;
      }
      Qt::ArrowType arrow = option->direction == Qt::RightToLeft ? Qt::LeftArrow : Qt::RightArrow;
      if (option->state & State_Open) {
        arrow = Qt::DownArrow;
      }
      drawChevron(painter, option->rect, arrow, tokens.m_textSecondary, 8.0, 1.2);
      return;
    }

    case PE_IndicatorArrowUp:
    case PE_IndicatorArrowDown:
    case PE_IndicatorArrowLeft:
    case PE_IndicatorArrowRight:
    case PE_IndicatorSpinUp:
    case PE_IndicatorSpinDown: {
      Qt::ArrowType arrow = Qt::DownArrow;
      if (element == PE_IndicatorArrowUp || element == PE_IndicatorSpinUp) {
        arrow = Qt::UpArrow;
      } else if (element == PE_IndicatorArrowLeft) {
        arrow = Qt::LeftArrow;
      } else if (element == PE_IndicatorArrowRight) {
        arrow = Qt::RightArrow;
      }
      const qreal size = qMin<qreal>(9.0, qMin(option->rect.width(), option->rect.height()) * 0.7);
      const QColor color = (option->state & State_Enabled) ? tokens.m_textSecondary : tokens.m_textDisabled;
      drawChevron(painter, option->rect, arrow, color, size, 1.2);
      return;
    }

    case PE_IndicatorHeaderArrow:
      if (const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader*>(option)) {
        if (header->sortIndicator & QStyleOptionHeader::SortUp) {
          drawChevron(painter, option->rect, Qt::UpArrow, tokens.m_textSecondary, 8.0, 1.2);
        } else if (header->sortIndicator & QStyleOptionHeader::SortDown) {
          drawChevron(painter, option->rect, Qt::DownArrow, tokens.m_textSecondary, 8.0, 1.2);
        }
        return;
      }
      break;

    case PE_IndicatorToolBarSeparator: {
      const QRect r = option->rect;
      if (option->state & State_Horizontal) {
        painter->fillRect(QRect(r.center().x(), r.top() + 6, 1, r.height() - 12), tokens.m_divider);
      } else {
        painter->fillRect(QRect(r.left() + 6, r.center().y(), r.width() - 12, 1), tokens.m_divider);
      }
      return;
    }

    case PE_IndicatorTabClose: {
      const bool enabled = option->state & State_Enabled;
      const bool hovered = enabled && (option->state & State_Raised);
      const bool pressed = enabled && (option->state & State_Sunken);
      if (pressed || hovered) {
        fillRoundedRect(painter, option->rect, CONTROL_RADIUS,
                        pressed ? tokens.m_subtleFillTertiary : tokens.m_subtleFillSecondary);
      }
      const QPointF center = QRectF(option->rect).center();
      painter->save();
      painter->setRenderHint(QPainter::Antialiasing, true);
      QPen pen(hovered ? tokens.m_textPrimary : tokens.m_textSecondary, 1.2);
      pen.setCapStyle(Qt::RoundCap);
      painter->setPen(pen);
      painter->drawLine(center + QPointF(-4, -4), center + QPointF(4, 4));
      painter->drawLine(center + QPointF(4, -4), center + QPointF(-4, 4));
      painter->restore();
      return;
    }

    default:
      break;
  }
  QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void WinUIStyle::drawButton(const QStyleOption* option, QPainter* painter, bool isAccent, bool isSubtle) const {
  const Tokens& tokens = tokensFor(option);
  const bool enabled = option->state & State_Enabled;
  const bool hovered = enabled && (option->state & State_MouseOver);
  const bool pressed = enabled && (option->state & State_Sunken);
  const bool checked = option->state & State_On;
  const QRectF rect(option->rect);

  if (isSubtle && !checked) {
    // AppBarButton-style: no chrome until hovered
    if (pressed || hovered) {
      fillRoundedRect(painter, rect, CONTROL_RADIUS, pressed ? tokens.m_subtleFillTertiary : tokens.m_subtleFillSecondary);
    }
  } else if (isAccent || checked) {
    if (!enabled) {
      fillRoundedRect(painter, rect, CONTROL_RADIUS, tokens.m_accentDisabled);
    } else if (pressed || isSubtle) {
      fillRoundedRect(painter, rect, CONTROL_RADIUS, accentFill(option, hovered, pressed));
    } else {
      drawElevatedRect(painter, rect, CONTROL_RADIUS, accentFill(option, hovered, false), tokens.m_strokeOnAccent,
                       tokens.m_strokeOnAccentSecondary, 1.0, false);
    }
  } else {
    // A Button color set by the application replaces WinUI's translucent fills
    const QColor custom = overriddenColor(option, QPalette::Button, basePalette(option));
    QColor fill = tokens.m_controlFill;
    if (custom.isValid()) {
      fill = custom;
      if (pressed) {
        fill = shade(custom, 0.08);
      } else if (hovered) {
        fill = shade(custom, 0.04);
      }
    } else if (!enabled) {
      fill = tokens.m_controlFillDisabled;
    } else if (pressed) {
      fill = tokens.m_controlFillTertiary;
    } else if (hovered) {
      fill = tokens.m_controlFillSecondary;
    }
    // Pressed and disabled buttons lose their elevation edge
    const QColor edge = (enabled && !pressed) ? tokens.m_controlStrokeSecondary : QColor();
    drawElevatedRect(painter, rect, CONTROL_RADIUS, fill, tokens.m_controlStroke, edge, 1.0,
                     tokens.m_elevationEdgeAtTop);
  }

  if (enabled && hasKeyboardFocus(option)) {
    drawFocusRing(painter, rect, CONTROL_RADIUS, tokens);
  }
}

void WinUIStyle::drawTextField(const QStyleOption* option, QPainter* painter, bool fill) const {
  const Tokens& tokens = tokensFor(option);
  const bool enabled = option->state & State_Enabled;
  const bool hovered = enabled && (option->state & State_MouseOver);
  const bool focused = enabled && (option->state & State_HasFocus);

  // A Base color set by the application replaces WinUI's translucent fills
  const QColor custom = overriddenColor(option, QPalette::Base, basePalette(option));
  QColor fillColor = tokens.m_controlFill;
  if (custom.isValid()) {
    fillColor = (hovered && !focused) ? shade(custom, 0.03) : custom;
  } else if (!enabled) {
    fillColor = tokens.m_controlFillDisabled;
  } else if (focused) {
    fillColor = tokens.m_controlFillInputActive;
  } else if (hovered) {
    fillColor = tokens.m_controlFillSecondary;
  }
  // Strong bottom stroke, replaced by a 2px accent underline while focused
  QColor edge;
  qreal edgeWidth = 1.0;
  if (enabled) {
    edge = focused ? optionAccent(option) : tokens.m_controlStrongStroke;
    edgeWidth = focused ? 2.0 : 1.0;
  }
  drawElevatedRect(painter, option->rect, CONTROL_RADIUS, fill ? fillColor : QColor(), tokens.m_controlStroke, edge,
                   edgeWidth, false);
}

void WinUIStyle::drawCheckIndicator(const QStyleOption* option, QPainter* painter, bool isRadio) const {
  const Tokens& tokens = tokensFor(option);
  const qreal size = qMin(option->rect.width(), option->rect.height());
  QRectF box(0, 0, size, size);
  box.moveCenter(QRectF(option->rect).center());

  const bool enabled = option->state & State_Enabled;
  const bool hovered = enabled && (option->state & State_MouseOver);
  const bool pressed = enabled && (option->state & State_Sunken);
  const bool checked = option->state & State_On;
  const bool partial = !isRadio && (option->state & State_NoChange);
  const qreal radius = isRadio ? size / 2.0 : qMin<qreal>(CONTROL_RADIUS, size / 4.0);

  if (checked || partial) {
    const QColor fill = enabled ? accentFill(option, hovered, pressed) : tokens.m_accentDisabled;
    fillRoundedRect(painter, box, radius, fill);
    QColor glyph = tokens.m_textOnAccentDisabled;
    if (enabled) {
      glyph = pressed ? tokens.m_textOnAccentSecondary : tokens.m_textOnAccent;
    }
    if (isRadio) {
      // The inner dot grows on hover and shrinks while pressed
      qreal dot = size * 0.6;
      if (pressed) {
        dot = size * 0.5;
      } else if (hovered) {
        dot = size * 0.7;
      }
      const QRectF dotRect(box.center().x() - dot / 2.0, box.center().y() - dot / 2.0, dot, dot);
      fillRoundedRect(painter, dotRect, dot / 2.0, enabled ? tokens.m_textOnAccent : tokens.m_textOnAccentDisabled);
    } else if (partial && !checked) {
      painter->save();
      painter->setRenderHint(QPainter::Antialiasing, true);
      QPen pen(glyph, 1.2);
      pen.setCapStyle(Qt::RoundCap);
      painter->setPen(pen);
      painter->drawLine(QPointF(box.center().x() - size * 0.2, box.center().y()),
                        QPointF(box.center().x() + size * 0.2, box.center().y()));
      painter->restore();
    } else {
      drawCheckmark(painter, box, glyph, 1.3);
    }
  } else {
    QColor fill = tokens.m_controlAltFillSecondary;
    QColor border = tokens.m_controlStrongStroke;
    if (!enabled) {
      fill = QColor();
      border = tokens.m_controlStrongStrokeDisabled;
    } else if (pressed) {
      fill = tokens.m_controlAltFillQuarternary;
      border = tokens.m_controlStrongStrokeDisabled;
    } else if (hovered) {
      fill = tokens.m_controlAltFillTertiary;
    }
    const QColor custom = overriddenColor(option, QPalette::Base, basePalette(option));
    if (custom.isValid()) {
      fill = pressed ? shade(custom, 0.08) : custom;
    }
    fillRoundedRect(painter, box, radius, fill.isValid() ? QBrush(fill) : QBrush(Qt::NoBrush), border);
  }
}

void WinUIStyle::drawItemViewBackground(const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionViewItem* item = qstyleoption_cast<const QStyleOptionViewItem*>(option);
  if (!item) {
    return;
  }
  if (item->backgroundBrush.style() != Qt::NoBrush) {
    const QPointF oldOrigin = painter->brushOrigin();
    painter->setBrushOrigin(item->rect.topLeft());
    painter->fillRect(item->rect, item->backgroundBrush);
    painter->setBrushOrigin(oldOrigin);
  }

  const Tokens& tokens = tokensFor(item);
  const bool enabled = item->state & State_Enabled;
  const bool selected = item->state & State_Selected;
  const bool hovered = enabled && (item->state & State_MouseOver);
  if (!selected && !hovered) {
    return;
  }

  const QListView* listView = qobject_cast<const QListView*>(widget);
  QStyleOptionViewItem::ViewItemPosition position = item->viewItemPosition;
  if (position == QStyleOptionViewItem::Invalid && listView) {
    position = QStyleOptionViewItem::OnlyOne;
  }
  if (position == QStyleOptionViewItem::Invalid) {
    // Table cells: square, accent-tinted selection keeps the grid readable
    const QColor color = selected ? withAlpha(optionAccent(item), isDarkPalette(item->palette) ? 0.3 : 0.2)
                                  : tokens.m_subtleFillSecondary;
    painter->fillRect(item->rect, color);
    return;
  }

  const bool isStart = position == QStyleOptionViewItem::Beginning || position == QStyleOptionViewItem::OnlyOne;
  const bool isEnd = position == QStyleOptionViewItem::End || position == QStyleOptionViewItem::OnlyOne;
  const bool rightToLeft = item->direction == Qt::RightToLeft;
  const bool roundLeft = rightToLeft ? isEnd : isStart;
  const bool roundRight = rightToLeft ? isStart : isEnd;

  // ListViewItem: 4,2 margins, 4px corners, with outer corners only on multi-column rows
  const QRect clip = item->rect.adjusted(roundLeft ? 4 : 0, 2, roundRight ? -4 : 0, -2);
  QRectF shape(clip);
  if (!roundLeft) {
    shape.adjust(-CONTROL_RADIUS - 1, 0, 0, 0);
  }
  if (!roundRight) {
    shape.adjust(0, 0, CONTROL_RADIUS + 1, 0);
  }
  const QColor color = (selected && hovered) ? tokens.m_subtleFillTertiary : tokens.m_subtleFillSecondary;
  painter->save();
  painter->setClipRect(clip, Qt::IntersectClip);
  fillRoundedRect(painter, shape, CONTROL_RADIUS, color);
  painter->restore();

  const bool iconMode = listView && listView->viewMode() == QListView::IconMode;
  if (selected && isStart && !iconMode) {
    drawSelectionPill(painter, clip, rightToLeft, enabled ? optionAccent(item) : tokens.m_accentDisabled);
  }
}

// ---------------------------------------------------------------------------
// Controls
// ---------------------------------------------------------------------------

void WinUIStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter,
                             const QWidget* widget) const {
  const Tokens& tokens = tokensFor(option);
  switch (element) {
    case CE_PushButtonBevel:
      if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
        proxy()->drawPrimitive(PE_PanelButtonCommand, button, painter, widget);
        if (button->features & QStyleOptionButton::HasMenu) {
          const int indicatorWidth = proxy()->pixelMetric(PM_MenuButtonIndicator, button, widget);
          const QRect arrowRect(button->rect.right() - indicatorWidth - BUTTON_PADDING + 4, button->rect.top(),
                                indicatorWidth, button->rect.height());
          const bool onAccent = (button->state & State_Enabled) &&
                                ((button->features & QStyleOptionButton::DefaultButton) || (button->state & State_On));
          drawChevron(painter, visualRect(button->direction, button->rect, arrowRect), Qt::DownArrow,
                      onAccent ? tokens.m_textOnAccent : tokens.m_textSecondary, 9.0, 1.2);
        }
        return;
      }
      break;

    case CE_PushButtonLabel:
      if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
        const bool enabled = button->state & State_Enabled;
        const bool pressed = enabled && (button->state & State_Sunken);
        const bool onAccent = (button->features & QStyleOptionButton::DefaultButton) || (button->state & State_On);
        QColor text;
        if (onAccent) {
          text = !enabled ? tokens.m_textOnAccentDisabled
                          : pressed ? tokens.m_textOnAccentSecondary : tokens.m_textOnAccent;
        } else {
          text = !enabled ? tokens.m_textDisabled : pressed ? tokens.m_textSecondary : tokens.m_textPrimary;
        }
        const QPalette::ColorRole role = onAccent ? QPalette::HighlightedText : QPalette::ButtonText;
        const QColor custom = overriddenColor(button, role, basePalette(button));
        if (custom.isValid()) {
          text = pressed ? withAlpha(custom, custom.alphaF() * 0.8) : custom;
        }
        QStyleOptionButton copy(*button);
        copy.palette.setColor(QPalette::ButtonText, text);
        QProxyStyle::drawControl(element, &copy, painter, widget);
        return;
      }
      break;

    case CE_ToolButtonLabel:
      if (const QStyleOptionToolButton* button = qstyleoption_cast<const QStyleOptionToolButton*>(option)) {
        if ((button->state & State_On) && (button->state & State_Enabled)) {
          QStyleOptionToolButton copy(*button);
          copy.palette.setColor(QPalette::ButtonText, tokens.m_textOnAccent);
          QProxyStyle::drawControl(element, &copy, painter, widget);
          return;
        }
      }
      break;

    case CE_CheckBox:
    case CE_RadioButton:
      QProxyStyle::drawControl(element, option, painter, widget);
      if ((option->state & State_Enabled) && hasKeyboardFocus(option)) {
        drawFocusRing(painter, option->rect, CONTROL_RADIUS, tokens);
      }
      return;

    case CE_TabBarTabShape:
      drawTabShape(option, painter);
      return;

    case CE_TabBarTabLabel:
      if (const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
        QStyleOptionTab copy(*tab);
        copy.rect = tabContentRect(tab);
        const bool emphasized = (tab->state & State_Selected) || (tab->state & State_MouseOver);
        QColor text = emphasized ? tokens.m_textPrimary : tokens.m_textSecondary;
        const QColor custom = overriddenColor(tab, QPalette::WindowText, basePalette(tab));
        if (custom.isValid()) {
          text = emphasized ? custom : withAlpha(custom, custom.alphaF() * 0.7);
        }
        setEnabledColor(copy.palette, QPalette::WindowText, text);
        setEnabledColor(copy.palette, QPalette::ButtonText, text);
        QProxyStyle::drawControl(element, &copy, painter, widget);
        return;
      }
      break;

    case CE_HeaderSection:
      drawHeaderSection(option, painter);
      return;

    case CE_HeaderEmptyArea: {
      const QRect r = option->rect;
      painter->fillRect(r, option->palette.color(QPalette::Base));
      if (option->state & State_Horizontal) {
        painter->fillRect(QRect(r.left(), r.bottom(), r.width(), 1), tokens.m_divider);
      } else {
        painter->fillRect(QRect(r.right(), r.top(), 1, r.height()), tokens.m_divider);
      }
      return;
    }

    case CE_ItemViewItem:
      if (const QStyleOptionViewItem* item = qstyleoption_cast<const QStyleOptionViewItem*>(option)) {
        if (item->state & State_Selected) {
          // Selection is a subtle fill plus pill, so text keeps its normal color
          QStyleOptionViewItem copy(*item);
          for (QPalette::ColorGroup group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
            copy.palette.setColor(group, QPalette::HighlightedText, item->palette.color(group, QPalette::Text));
          }
          QProxyStyle::drawControl(element, &copy, painter, widget);
          return;
        }
      }
      break;

    case CE_MenuItem:
      if (const QStyleOptionMenuItem* item = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
        if (item->menuItemType != QStyleOptionMenuItem::Scroller &&
            item->menuItemType != QStyleOptionMenuItem::TearOff) {
          drawMenuItem(item, painter, widget);
          return;
        }
      }
      break;

    case CE_MenuEmptyArea:
    case CE_MenuBarEmptyArea:
    case CE_ToolBar:
      // Transparent: the flyout panel or window background shows through
      return;

    case CE_MenuBarItem:
      if (const QStyleOptionMenuItem* item = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
        const bool enabled = item->state & State_Enabled;
        if (enabled && (item->state & (State_Selected | State_Sunken))) {
          const QColor fill = (item->state & State_Sunken) ? tokens.m_subtleFillTertiary : tokens.m_subtleFillSecondary;
          fillRoundedRect(painter, QRectF(item->rect).adjusted(2, 3, -2, -3), CONTROL_RADIUS, fill);
        }
        int flags = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
        if (!proxy()->styleHint(SH_UnderlineShortcut, item, widget)) {
          flags |= Qt::TextHideMnemonic;
        }
        painter->save();
        painter->setPen(overriddenOr(item, QPalette::ButtonText, basePalette(item),
                                     enabled ? tokens.m_textPrimary : tokens.m_textDisabled));
        painter->drawText(item->rect, flags, item->text);
        painter->restore();
        return;
      }
      break;

    case CE_Splitter:
      if ((option->state & State_Enabled) && (option->state & State_MouseOver)) {
        const QRect r = option->rect;
        const QColor color = withAlpha(optionAccent(option), 0.8);
        if (option->state & State_Horizontal) {
          painter->fillRect(QRect(r.center().x() - 1, r.top(), 2, r.height()), color);
        } else {
          painter->fillRect(QRect(r.left(), r.center().y() - 1, r.width(), 2), color);
        }
      }
      return;

    case CE_ShapedFrame:
      if (const QStyleOptionFrame* frame = qstyleoption_cast<const QStyleOptionFrame*>(option)) {
        const QRect r = frame->rect;
        if (frame->frameShape == QFrame::HLine) {
          painter->fillRect(QRect(r.left(), r.center().y(), r.width(), 1), tokens.m_divider);
          return;
        }
        if (frame->frameShape == QFrame::VLine) {
          painter->fillRect(QRect(r.center().x(), r.top(), 1, r.height()), tokens.m_divider);
          return;
        }
      }
      break;

    case CE_ProgressBar:
      if (const QStyleOptionProgressBar* bar = qstyleoption_cast<const QStyleOptionProgressBar*>(option)) {
        QStyleOptionProgressBar sub(*bar);
        sub.rect = proxy()->subElementRect(SE_ProgressBarGroove, bar, widget);
        proxy()->drawControl(CE_ProgressBarGroove, &sub, painter, widget);
        sub.rect = proxy()->subElementRect(SE_ProgressBarContents, bar, widget);
        proxy()->drawControl(CE_ProgressBarContents, &sub, painter, widget);
        if (hasProgressLabel(bar)) {
          sub.rect = proxy()->subElementRect(SE_ProgressBarLabel, bar, widget);
          proxy()->drawControl(CE_ProgressBarLabel, &sub, painter, widget);
        }
        return;
      }
      break;

    case CE_ProgressBarGroove:
      if (const QStyleOptionProgressBar* bar = qstyleoption_cast<const QStyleOptionProgressBar*>(option)) {
        // A hairline track under a 3px indicator
        const QRectF r(bar->rect);
        const QRectF track = isHorizontal(bar) ? QRectF(r.left(), r.center().y() - 0.5, r.width(), 1.0)
                                               : QRectF(r.center().x() - 0.5, r.top(), 1.0, r.height());
        const bool enabled = bar->state & State_Enabled;
        painter->fillRect(track, enabled ? tokens.m_controlStrongStroke : tokens.m_controlStrongStrokeDisabled);
        return;
      }
      break;

    case CE_ProgressBarContents:
      drawProgressBar(option, painter, widget);
      return;

    case CE_ProgressBarLabel:
      if (const QStyleOptionProgressBar* bar = qstyleoption_cast<const QStyleOptionProgressBar*>(option)) {
        const Qt::Alignment alignment = visualAlignment(bar->direction, Qt::AlignRight | Qt::AlignVCenter);
        painter->save();
        painter->setPen((bar->state & State_Enabled) ? tokens.m_textSecondary : tokens.m_textDisabled);
        painter->drawText(bar->rect, int(alignment), bar->text);
        painter->restore();
        return;
      }
      break;

    default:
      break;
  }
  QProxyStyle::drawControl(element, option, painter, widget);
}

void WinUIStyle::drawMenuItem(const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionMenuItem* item = qstyleoption_cast<const QStyleOptionMenuItem*>(option);
  const Tokens& tokens = tokensFor(item);
  const QRect r = item->rect;
  const bool enabled = item->state & State_Enabled;

  if (item->menuItemType == QStyleOptionMenuItem::Separator) {
    if (item->text.isEmpty()) {
      painter->fillRect(QRect(r.left(), r.center().y(), r.width(), 1), tokens.m_divider);
    } else {
      // Section header
      const QRect textRect = r.adjusted(MENU_ITEM_INSET_H + BUTTON_PADDING, 0, -MENU_ITEM_INSET_H - BUTTON_PADDING, 0);
      painter->save();
      painter->setFont(item->font);
      painter->setPen(tokens.m_textSecondary);
      painter->drawText(visualRect(item->direction, r, textRect),
                        int(visualAlignment(item->direction, Qt::AlignLeft | Qt::AlignVCenter)), item->text);
      painter->restore();
    }
    return;
  }

  // Combo popups share this code, but mark the current item with a pill instead of a check
  const bool comboPopup = qobject_cast<const QComboBox*>(widget);
  const bool selected = enabled && (item->state & State_Selected);
  const bool current = comboPopup && item->checked;
  const QRectF pill = QRectF(r).adjusted(MENU_ITEM_INSET_H, MENU_ITEM_INSET_V, -MENU_ITEM_INSET_H, -MENU_ITEM_INSET_V);
  if (selected || current) {
    const QColor fill =
        (selected && (current || (item->state & State_Sunken))) ? tokens.m_subtleFillTertiary : tokens.m_subtleFillSecondary;
    fillRoundedRect(painter, pill, CONTROL_RADIUS, fill);
  }
  if (current) {
    drawSelectionPill(painter, pill, item->direction == Qt::RightToLeft,
                      enabled ? optionAccent(item) : tokens.m_accentDisabled);
  }

  const QColor textColor =
      overriddenOr(item, QPalette::Text, basePalette(item), enabled ? tokens.m_textPrimary : tokens.m_textDisabled);
  const auto visual = [&](const QRect& rect) { return visualRect(item->direction, r, rect); };
  int x = int(pill.left()) + BUTTON_PADDING;

  if (!comboPopup && item->menuHasCheckableItems) {
    const QRect checkRect(x, r.center().y() - MENU_CHECK_SIZE / 2, MENU_CHECK_SIZE, MENU_CHECK_SIZE);
    if (item->checked) {
      if (item->checkType == QStyleOptionMenuItem::Exclusive) {
        const QRectF dot = QRectF(visual(checkRect)).adjusted(5, 5, -5, -5);
        fillRoundedRect(painter, dot, dot.width() / 2.0, textColor);
      } else {
        drawCheckmark(painter, QRectF(visual(checkRect)), textColor, 1.3);
      }
    }
    x += MENU_CHECK_SIZE + MENU_COLUMN_SPACING;
  }

  const int iconSize = proxy()->pixelMetric(PM_SmallIconSize, item, widget);
  if (item->maxIconWidth > 0 && (!comboPopup || !item->icon.isNull())) {
    if (!item->icon.isNull()) {
      const QIcon::Mode mode = !enabled ? QIcon::Disabled : selected ? QIcon::Active : QIcon::Normal;
      const QPixmap pixmap = item->icon.pixmap(iconSize, mode, item->checked ? QIcon::On : QIcon::Off);
      const QRect iconRect(x, r.center().y() - iconSize / 2, iconSize, iconSize);
      proxy()->drawItemPixmap(painter, visual(iconRect), Qt::AlignCenter, pixmap);
    }
    x += iconSize + MENU_COLUMN_SPACING;
  }

  QString text = item->text;
  QString shortcut;
  const int tab = text.indexOf(QLatin1Char('\t'));
  if (tab >= 0) {
    shortcut = text.mid(tab + 1);
    text = text.left(tab);
  }
  const bool hasSubmenu = item->menuItemType == QStyleOptionMenuItem::SubMenu;
  const int right = int(pill.right()) - BUTTON_PADDING;
  const QRect textRect(x, r.top(), right - x - (hasSubmenu ? 8 + MENU_COLUMN_SPACING : 0), r.height());

  int flags = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
  if (!proxy()->styleHint(SH_UnderlineShortcut, item, widget)) {
    flags |= Qt::TextHideMnemonic;
  }
  painter->save();
  painter->setFont(item->font);
  painter->setPen(textColor);
  painter->drawText(visual(textRect), flags | int(visualAlignment(item->direction, Qt::AlignLeft)), text);
  if (!shortcut.isEmpty()) {
    painter->setPen(enabled ? tokens.m_textSecondary : tokens.m_textDisabled);
    painter->drawText(visual(textRect), flags | int(visualAlignment(item->direction, Qt::AlignRight)), shortcut);
  }
  painter->restore();

  if (hasSubmenu) {
    const QRect arrowRect(right - 8, r.top(), 8, r.height());
    const Qt::ArrowType arrow = item->direction == Qt::RightToLeft ? Qt::LeftArrow : Qt::RightArrow;
    drawChevron(painter, visual(arrowRect), arrow, enabled ? tokens.m_textSecondary : tokens.m_textDisabled, 8.0, 1.2);
  }
}

void WinUIStyle::drawTabShape(const QStyleOption* option, QPainter* painter) const {
  const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option);
  if (!tab) {
    return;
  }
  const Tokens& tokens = tokensFor(tab);
  const bool selected = tab->state & State_Selected;
  const bool hovered = (tab->state & State_Enabled) && (tab->state & State_MouseOver);

  // Draw in north-facing local coordinates; the transform handles the other shapes
  QSizeF size;
  const QTransform transform = tabTransform(tab, &size);
  const qreal width = size.width();
  const qreal height = size.height();
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setTransform(transform, true);

  if (selected) {
    // TabViewItem: rounded top corners and small outward feet that merge into the pane
    const qreal radius = OVERLAY_RADIUS;
    const qreal foot = CONTROL_RADIUS;
    const qreal left = 0.5;
    const qreal right = width - 0.5;
    const qreal top = TAB_TOP_MARGIN + 0.5;
    const qreal baseline = height - 0.5;

    QPainterPath outline;
    outline.moveTo(left - foot, baseline);
    outline.arcTo(QRectF(left - 2 * foot, baseline - 2 * foot, 2 * foot, 2 * foot), 270, 90);
    outline.lineTo(left, top + radius);
    outline.arcTo(QRectF(left, top, 2 * radius, 2 * radius), 180, -90);
    outline.lineTo(right - radius, top);
    outline.arcTo(QRectF(right - 2 * radius, top, 2 * radius, 2 * radius), 90, -90);
    outline.lineTo(right, baseline - foot);
    outline.arcTo(QRectF(right, baseline - 2 * foot, 2 * foot, 2 * foot), 180, 90);

    QPainterPath shape = outline;
    shape.lineTo(right + foot, height);
    shape.lineTo(left - foot, height);
    shape.closeSubpath();

    const QColor fill = tab->documentMode ? tab->palette.color(QPalette::Window) : tokens.m_solidBackgroundTertiary;
    painter->fillPath(shape, fill);
    painter->setPen(QPen(tokens.m_divider, 1.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(outline);
  } else {
    if (hovered) {
      const QRectF pill(2, TAB_TOP_MARGIN + 2, width - 4, height - TAB_TOP_MARGIN - 5);
      painter->setPen(Qt::NoPen);
      painter->setBrush(tokens.m_subtleFillSecondary);
      painter->drawRoundedRect(pill, CONTROL_RADIUS, CONTROL_RADIUS);
    }
    // Dividers between unselected tabs
    const bool isLast = tab->position == QStyleOptionTab::End || tab->position == QStyleOptionTab::OnlyOneTab;
    if (!hovered && !isLast && tab->selectedPosition != QStyleOptionTab::NextIsSelected) {
      const bool mirrored = !isVerticalTab(tab->shape) && tab->direction == Qt::RightToLeft;
      const qreal x = mirrored ? 0.5 : width - 0.5;
      const qreal center = (TAB_TOP_MARGIN + height) / 2.0;
      painter->setPen(QPen(tokens.m_divider, 1.0));
      painter->drawLine(QPointF(x, center - 8), QPointF(x, center + 8));
    }
  }
  painter->restore();
}

void WinUIStyle::drawHeaderSection(const QStyleOption* option, QPainter* painter) const {
  const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader*>(option);
  if (!header) {
    return;
  }
  const Tokens& tokens = tokensFor(header);
  const QRect r = header->rect;
  const bool enabled = header->state & State_Enabled;
  painter->fillRect(r, header->palette.color(QPalette::Base));
  if (enabled && (header->state & State_Sunken)) {
    painter->fillRect(r, tokens.m_subtleFillTertiary);
  } else if (enabled && (header->state & State_MouseOver)) {
    painter->fillRect(r, tokens.m_subtleFillSecondary);
  }

  const bool isLast =
      header->position == QStyleOptionHeader::End || header->position == QStyleOptionHeader::OnlyOneSection;
  const int edge = header->direction == Qt::RightToLeft ? r.left() : r.right();
  if (header->orientation == Qt::Horizontal) {
    painter->fillRect(QRect(r.left(), r.bottom(), r.width(), 1), tokens.m_divider);
    if (!isLast) {
      painter->fillRect(QRect(edge, r.top() + 6, 1, r.height() - 12), tokens.m_divider);
    }
  } else {
    painter->fillRect(QRect(edge, r.top(), 1, r.height()), tokens.m_divider);
  }
}

void WinUIStyle::drawProgressBar(const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionProgressBar* bar = qstyleoption_cast<const QStyleOptionProgressBar*>(option);
  if (!bar) {
    return;
  }
  const Tokens& tokens = tokensFor(bar);
  const bool horizontal = isHorizontal(bar);
  const qreal thickness = 3.0;
  const QRectF bounds(bar->rect);
  const QRectF track = horizontal ? QRectF(bounds.left(), bounds.center().y() - thickness / 2.0, bounds.width(), thickness)
                                  : QRectF(bounds.center().x() - thickness / 2.0, bounds.top(), thickness, bounds.height());
  const qreal length = horizontal ? track.width() : track.height();

  bool reverse = horizontal ? bar->direction == Qt::RightToLeft : true;
  if (bar->invertedAppearance) {
    reverse = !reverse;
  }

  qreal start = 0.0;
  qreal extent = 0.0;
  const qint64 range = qint64(bar->maximum) - bar->minimum;
  if (range <= 0) {
    // Indeterminate: a segment that accelerates across the track
    const qreal phase = (QTime::currentTime().msecsSinceStartOfDay() % BUSY_CYCLE_MS) / qreal(BUSY_CYCLE_MS);
    const qreal eased = phase * phase * (3.0 - 2.0 * phase);
    const qreal segment = length * 0.4;
    start = -segment + eased * (length + segment);
    extent = segment;
    if (qobject_cast<const QProgressBar*>(widget)) {
      scheduleAnimationFrame(widget);
    }
  } else {
    const qreal fraction = qBound<qreal>(0.0, qreal(qint64(bar->progress) - bar->minimum) / range, 1.0);
    if (fraction <= 0.0) {
      return;
    }
    extent = qMax(thickness, length * fraction);
  }
  if (reverse) {
    start = length - start - extent;
  }

  QRectF chunk = horizontal ? QRectF(track.left() + start, track.top(), extent, thickness)
                            : QRectF(track.left(), track.top() + start, thickness, extent);
  chunk = chunk.intersected(track);
  if (chunk.isEmpty()) {
    return;
  }
  const QColor color = (bar->state & State_Enabled) ? optionAccent(bar) : tokens.m_accentDisabled;
  fillRoundedRect(painter, chunk, thickness / 2.0, color);
}

// ---------------------------------------------------------------------------
// Complex controls
// ---------------------------------------------------------------------------

void WinUIStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter,
                                    const QWidget* widget) const {
  switch (control) {
    case CC_SpinBox:
      if (qstyleoption_cast<const QStyleOptionSpinBox*>(option)) {
        drawSpinBox(option, painter, widget);
        return;
      }
      break;
    case CC_ComboBox:
      if (qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
        drawComboBox(option, painter, widget);
        return;
      }
      break;
    case CC_ScrollBar:
      if (qstyleoption_cast<const QStyleOptionSlider*>(option)) {
        drawScrollBar(option, painter, widget);
        return;
      }
      break;
    case CC_Slider:
      if (qstyleoption_cast<const QStyleOptionSlider*>(option)) {
        drawSlider(option, painter, widget);
        return;
      }
      break;
    default:
      break;
  }
  QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void WinUIStyle::drawSpinBox(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionSpinBox* spin = qstyleoption_cast<const QStyleOptionSpinBox*>(option);
  const Tokens& tokens = tokensFor(spin);
  if (spin->frame) {
    drawTextField(spin, painter, true);
  }
  if (spin->buttonSymbols == QAbstractSpinBox::NoButtons) {
    return;
  }

  // NumberBox inline spin buttons: subtle, side by side
  const bool enabled = spin->state & State_Enabled;
  for (SubControl subControl : {SC_SpinBoxUp, SC_SpinBoxDown}) {
    const bool isUp = subControl == SC_SpinBoxUp;
    const QRect r = proxy()->subControlRect(CC_SpinBox, spin, subControl, widget);
    const bool stepEnabled =
        enabled && (spin->stepEnabled & (isUp ? QAbstractSpinBox::StepUpEnabled : QAbstractSpinBox::StepDownEnabled));
    const bool active = stepEnabled && (spin->activeSubControls & subControl);
    const bool pressed = active && (spin->state & State_Sunken);
    if (active && (pressed || (spin->state & State_MouseOver))) {
      fillRoundedRect(painter, QRectF(r).adjusted(2, 4, -2, -4), CONTROL_RADIUS,
                      pressed ? tokens.m_subtleFillTertiary : tokens.m_subtleFillSecondary);
    }
    QColor color = tokens.m_textDisabled;
    if (stepEnabled) {
      color = pressed ? tokens.m_textTertiary : tokens.m_textSecondary;
    }
    if (spin->buttonSymbols == QAbstractSpinBox::PlusMinus) {
      painter->save();
      painter->setRenderHint(QPainter::Antialiasing, true);
      QPen pen(color, 1.2);
      pen.setCapStyle(Qt::RoundCap);
      painter->setPen(pen);
      const QPointF center = QRectF(r).center();
      painter->drawLine(center - QPointF(4, 0), center + QPointF(4, 0));
      if (isUp) {
        painter->drawLine(center - QPointF(0, 4), center + QPointF(0, 4));
      }
      painter->restore();
    } else {
      drawChevron(painter, r, isUp ? Qt::UpArrow : Qt::DownArrow, color, 9.0, 1.2);
    }
  }
}

void WinUIStyle::drawComboBox(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionComboBox* combo = qstyleoption_cast<const QStyleOptionComboBox*>(option);
  const Tokens& tokens = tokensFor(combo);
  const bool enabled = combo->state & State_Enabled;
  const QRect arrow = proxy()->subControlRect(CC_ComboBox, combo, SC_ComboBoxArrow, widget);

  if (combo->editable) {
    if (combo->frame) {
      drawTextField(combo, painter, true);
    }
    const bool arrowActive = enabled && (combo->activeSubControls & SC_ComboBoxArrow);
    if (arrowActive && (combo->state & (State_MouseOver | State_Sunken))) {
      const QColor fill = (combo->state & State_Sunken) ? tokens.m_subtleFillTertiary : tokens.m_subtleFillSecondary;
      fillRoundedRect(painter, QRectF(arrow).adjusted(2, 3, -3, -3), CONTROL_RADIUS, fill);
    }
  } else if (combo->frame) {
    // State_On only means "popup open" here, which should not look checked
    QStyleOptionComboBox panel(*combo);
    panel.state &= ~State_On;
    drawButton(&panel, painter, false, false);
  } else {
    drawButton(combo, painter, false, true);
  }

  if (combo->subControls & SC_ComboBoxArrow) {
    drawChevron(painter, arrow, Qt::DownArrow, enabled ? tokens.m_textSecondary : tokens.m_textDisabled, 9.0, 1.2);
  }
}

void WinUIStyle::drawScrollBar(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionSlider* bar = qstyleoption_cast<const QStyleOptionSlider*>(option);
  const Tokens& tokens = tokensFor(bar);
  const bool horizontal = bar->orientation == Qt::Horizontal;
  const bool enabled = bar->state & State_Enabled;
  const bool hasRange = bar->maximum != bar->minimum;
  // Collapsed to a thin indicator until the pointer is over the bar
  const bool expanded = enabled && hasRange && (bar->state & (State_MouseOver | State_Sunken));
  const QRectF r(bar->rect);

  if (expanded) {
    const QRectF track = r.adjusted(1, 1, -1, -1);
    const qreal radius = (horizontal ? track.height() : track.width()) / 2.0;
    fillRoundedRect(painter, track, radius, tokens.m_flyoutFill, tokens.m_controlStroke);

    for (SubControl subControl : {SC_ScrollBarSubLine, SC_ScrollBarAddLine}) {
      const QRect button = proxy()->subControlRect(CC_ScrollBar, bar, subControl, widget);
      if (button.isEmpty()) {
        continue;
      }
      // Point away from the center so mirrored layouts stay correct
      Qt::ArrowType arrow;
      if (horizontal) {
        arrow = button.center().x() < r.center().x() ? Qt::LeftArrow : Qt::RightArrow;
      } else {
        arrow = button.center().y() < r.center().y() ? Qt::UpArrow : Qt::DownArrow;
      }
      const bool pressed = (bar->activeSubControls & subControl) && (bar->state & State_Sunken);
      drawCaret(painter, button, arrow, pressed ? tokens.m_textSecondary : tokens.m_controlStrongFill, 6.0);
    }
  }

  if (!hasRange || !(bar->subControls & SC_ScrollBarSlider) || !enabled) {
    return;
  }
  const QRectF slider(proxy()->subControlRect(CC_ScrollBar, bar, SC_ScrollBarSlider, widget));
  const qreal thickness = expanded ? 6.0 : 2.0;
  const QRectF thumb = horizontal
                           ? QRectF(slider.left() + 1, r.center().y() - thickness / 2.0, slider.width() - 2, thickness)
                           : QRectF(r.center().x() - thickness / 2.0, slider.top() + 1, thickness, slider.height() - 2);
  fillRoundedRect(painter, thumb, thickness / 2.0, tokens.m_controlStrongFill);
}

void WinUIStyle::drawSlider(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionSlider* slider = qstyleoption_cast<const QStyleOptionSlider*>(option);
  const Tokens& tokens = tokensFor(slider);
  const bool horizontal = slider->orientation == Qt::Horizontal;
  const bool enabled = slider->state & State_Enabled;
  const QColor accent = enabled ? optionAccent(slider) : tokens.m_accentDisabled;
  const QRect groove = proxy()->subControlRect(CC_Slider, slider, SC_SliderGroove, widget);
  const QRect handle = proxy()->subControlRect(CC_Slider, slider, SC_SliderHandle, widget);
  const QPointF handleCenter = QRectF(handle).center();

  if (slider->subControls & SC_SliderGroove) {
    const qreal radius = SLIDER_TRACK_THICKNESS / 2.0;
    fillRoundedRect(painter, groove, radius, enabled ? tokens.m_controlStrongFill : tokens.m_controlStrongFillDisabled);

    // The value part runs from the minimum to the thumb
    QRectF filled(groove);
    if (horizontal) {
      if (slider->upsideDown) {
        filled.setLeft(handleCenter.x());
      } else {
        filled.setRight(handleCenter.x());
      }
    } else {
      if (slider->upsideDown) {
        filled.setTop(handleCenter.y());
      } else {
        filled.setBottom(handleCenter.y());
      }
    }
    fillRoundedRect(painter, filled, radius, accent);
  }

  if ((slider->subControls & SC_SliderTickmarks) && slider->tickPosition != QSlider::NoTicks) {
    const QRect r = slider->rect;
    const int available = (horizontal ? r.width() : r.height()) - SLIDER_THUMB_SIZE;
    int interval = slider->tickInterval;
    if (interval <= 0) {
      interval = slider->singleStep;
      const int spacing = sliderPositionFromValue(slider->minimum, slider->maximum, slider->minimum + interval,
                                                  available, false);
      if (spacing < 4) {
        interval = slider->pageStep;
      }
    }
    interval = qMax(1, interval);
    painter->save();
    painter->setPen(enabled ? tokens.m_controlStrongFill : tokens.m_controlStrongFillDisabled);
    for (qint64 value = slider->minimum; value <= slider->maximum; value += interval) {
      const int position = sliderPositionFromValue(slider->minimum, slider->maximum, int(value), available,
                                                   slider->upsideDown) + SLIDER_THUMB_SIZE / 2;
      if (horizontal) {
        const int x = r.left() + position;
        if (slider->tickPosition & QSlider::TicksAbove) {
          painter->drawLine(x, r.top(), x, r.top() + 3);
        }
        if (slider->tickPosition & QSlider::TicksBelow) {
          painter->drawLine(x, r.bottom() - 3, x, r.bottom());
        }
      } else {
        const int y = r.top() + position;
        if (slider->tickPosition & QSlider::TicksLeft) {
          painter->drawLine(r.left(), y, r.left() + 3, y);
        }
        if (slider->tickPosition & QSlider::TicksRight) {
          painter->drawLine(r.right() - 3, y, r.right(), y);
        }
      }
    }
    painter->restore();
  }

  if (slider->subControls & SC_SliderHandle) {
    // Solid outer disc with an accent dot that grows on hover and shrinks while pressed
    const QRectF thumb(handle);
    const bool handleActive = slider->activeSubControls & SC_SliderHandle;
    const bool pressed = enabled && handleActive && (slider->state & State_Sunken);
    const bool hovered = enabled && handleActive && (slider->state & State_MouseOver);
    fillRoundedRect(painter, thumb, thumb.width() / 2.0, tokens.m_controlSolidFill, tokens.m_controlStrokeSecondary);

    qreal dot = 12.0;
    if (pressed) {
      dot = 10.0;
    } else if (hovered) {
      dot = 14.0;
    }
    const QRectF dotRect(thumb.center().x() - dot / 2.0, thumb.center().y() - dot / 2.0, dot, dot);
    fillRoundedRect(painter, dotRect, dot / 2.0, accent);

    if (enabled && hasKeyboardFocus(slider)) {
      drawFocusRing(painter, thumb.adjusted(-3, -3, 3, 3), thumb.width() / 2.0 + 3, tokens);
    }
  }
}

// ---------------------------------------------------------------------------
// Geometry
// ---------------------------------------------------------------------------

QRect WinUIStyle::subControlRect(ComplexControl control, const QStyleOptionComplex* option, SubControl subControl,
                                 const QWidget* widget) const {
  switch (control) {
    case CC_SpinBox:
      if (const QStyleOptionSpinBox* spin = qstyleoption_cast<const QStyleOptionSpinBox*>(option)) {
        const QRect r = spin->rect;
        const int border = spin->frame ? FIELD_BORDER : 0;
        const bool hasButtons = spin->buttonSymbols != QAbstractSpinBox::NoButtons;
        const int buttonsWidth = hasButtons ? 2 * SPIN_BUTTON_WIDTH + 2 : 0;
        const int innerHeight = r.height() - 2 * border;
        // Up then down, side by side at the trailing end
        const int upX = r.right() - border - buttonsWidth + 1;
        QRect result;
        switch (subControl) {
          case SC_SpinBoxFrame:
            return r;
          case SC_SpinBoxEditField:
            result = QRect(r.left() + border + TEXT_PADDING_LEFT, r.top() + border,
                           r.width() - 2 * border - TEXT_PADDING_LEFT - (hasButtons ? buttonsWidth : TEXT_PADDING_RIGHT),
                           innerHeight);
            break;
          case SC_SpinBoxUp:
            if (!hasButtons) {
              return QRect();
            }
            result = QRect(upX, r.top() + border, SPIN_BUTTON_WIDTH, innerHeight);
            break;
          case SC_SpinBoxDown:
            if (!hasButtons) {
              return QRect();
            }
            result = QRect(upX + SPIN_BUTTON_WIDTH, r.top() + border, SPIN_BUTTON_WIDTH, innerHeight);
            break;
          default:
            return QProxyStyle::subControlRect(control, option, subControl, widget);
        }
        return visualRect(spin->direction, r, result);
      }
      break;

    case CC_ComboBox:
      if (const QStyleOptionComboBox* combo = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
        const QRect r = combo->rect;
        const int border = combo->frame ? FIELD_BORDER : 0;
        const int padding = combo->editable ? TEXT_PADDING_LEFT : BUTTON_PADDING;
        QRect result;
        switch (subControl) {
          case SC_ComboBoxFrame:
            return r;
          case SC_ComboBoxArrow:
            result = QRect(r.right() - border - COMBO_ARROW_WIDTH + 1, r.top() + border, COMBO_ARROW_WIDTH,
                           r.height() - 2 * border);
            break;
          case SC_ComboBoxEditField:
            result = QRect(r.left() + border + padding, r.top() + border,
                           r.width() - 2 * border - padding - COMBO_ARROW_WIDTH, r.height() - 2 * border);
            break;
          default:
            return QProxyStyle::subControlRect(control, option, subControl, widget);
        }
        return visualRect(combo->direction, r, result);
      }
      break;

    case CC_ScrollBar:
      if (const QStyleOptionSlider* bar = qstyleoption_cast<const QStyleOptionSlider*>(option)) {
        const QRect r = bar->rect;
        const bool horizontal = bar->orientation == Qt::Horizontal;
        const int length = horizontal ? r.width() : r.height();
        const int thickness = horizontal ? r.height() : r.width();
        // Arrow buttons are always reserved so the thumb doesn't jump when the bar expands
        const int button = length >= 3 * thickness ? thickness : 0;
        const int grooveLength = length - 2 * button;
        const qint64 range = qint64(bar->maximum) - bar->minimum;
        int sliderLength = grooveLength;
        if (range > 0) {
          const int minimum = proxy()->pixelMetric(PM_ScrollBarSliderMin, bar, widget);
          sliderLength = int(qint64(grooveLength) * bar->pageStep / (range + bar->pageStep));
          sliderLength = qMin(grooveLength, qMax(minimum, sliderLength));
        }
        const int sliderStart = button + sliderPositionFromValue(bar->minimum, bar->maximum, bar->sliderPosition,
                                                                 grooveLength - sliderLength, bar->upsideDown);
        const auto span = [&](int start, int extent) {
          return horizontal ? QRect(r.left() + start, r.top(), extent, r.height())
                            : QRect(r.left(), r.top() + start, r.width(), extent);
        };
        QRect result;
        switch (subControl) {
          case SC_ScrollBarSubLine:
            result = span(0, button);
            break;
          case SC_ScrollBarAddLine:
            result = span(length - button, button);
            break;
          case SC_ScrollBarGroove:
            result = span(button, grooveLength);
            break;
          case SC_ScrollBarSlider:
            result = span(sliderStart, sliderLength);
            break;
          case SC_ScrollBarSubPage:
            result = span(button, sliderStart - button);
            break;
          case SC_ScrollBarAddPage:
            result = span(sliderStart + sliderLength, length - button - sliderStart - sliderLength);
            break;
          case SC_ScrollBarFirst:
          case SC_ScrollBarLast:
            return QRect();
          default:
            return QProxyStyle::subControlRect(control, option, subControl, widget);
        }
        return visualRect(bar->direction, r, result);
      }
      break;

    case CC_Slider:
      if (const QStyleOptionSlider* slider = qstyleoption_cast<const QStyleOptionSlider*>(option)) {
        const QRect r = slider->rect;
        const bool horizontal = slider->orientation == Qt::Horizontal;
        switch (subControl) {
          case SC_SliderGroove:
            // Must span the full length: QSlider maps pixels to values from it
            return horizontal ? QRect(r.left(), r.top() + (r.height() - SLIDER_TRACK_THICKNESS) / 2, r.width(),
                                      SLIDER_TRACK_THICKNESS)
                              : QRect(r.left() + (r.width() - SLIDER_TRACK_THICKNESS) / 2, r.top(),
                                      SLIDER_TRACK_THICKNESS, r.height());
          case SC_SliderHandle: {
            const int available = (horizontal ? r.width() : r.height()) - SLIDER_THUMB_SIZE;
            const int position = sliderPositionFromValue(slider->minimum, slider->maximum, slider->sliderPosition,
                                                         available, slider->upsideDown);
            return horizontal ? QRect(r.left() + position, r.top() + (r.height() - SLIDER_THUMB_SIZE) / 2,
                                      SLIDER_THUMB_SIZE, SLIDER_THUMB_SIZE)
                              : QRect(r.left() + (r.width() - SLIDER_THUMB_SIZE) / 2, r.top() + position,
                                      SLIDER_THUMB_SIZE, SLIDER_THUMB_SIZE);
          }
          case SC_SliderTickmarks:
            return r;
          default:
            break;
        }
      }
      break;

    default:
      break;
  }
  return QProxyStyle::subControlRect(control, option, subControl, widget);
}

QRect WinUIStyle::subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget) const {
  switch (element) {
    case SE_LineEditContents:
      if (const QStyleOptionFrame* frame = qstyleoption_cast<const QStyleOptionFrame*>(option)) {
        if (frame->lineWidth > 0) {
          const QRect r = frame->rect.adjusted(TEXT_PADDING_LEFT, FIELD_BORDER, -TEXT_PADDING_RIGHT, -FIELD_BORDER);
          return visualRect(frame->direction, frame->rect, r);
        }
      }
      break;

    case SE_ProgressBarGroove:
    case SE_ProgressBarContents:
    case SE_ProgressBarLabel:
      if (const QStyleOptionProgressBar* bar = qstyleoption_cast<const QStyleOptionProgressBar*>(option)) {
        const QRect r = bar->rect;
        if (!hasProgressLabel(bar)) {
          return element == SE_ProgressBarLabel ? QRect() : r;
        }
        // Label sits beside the bar; size it for "100%" so the bar doesn't jitter
        const QFontMetrics& metrics = bar->fontMetrics;
        const int labelWidth =
            qMax(metrics.horizontalAdvance(QStringLiteral("100%")), metrics.horizontalAdvance(bar->text)) + 12;
        const QRect result = element == SE_ProgressBarLabel
                                 ? QRect(r.right() - labelWidth + 1, r.top(), labelWidth, r.height())
                                 : QRect(r.left(), r.top(), r.width() - labelWidth, r.height());
        return visualRect(bar->direction, r, result);
      }
      break;

    case SE_TabWidgetTabBar:
      if (const QStyleOptionTabWidgetFrame* frame = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>(option)) {
        // Keep the first tab clear of the pane's rounded corner
        QRect r = QProxyStyle::subElementRect(element, option, widget);
        const QRect bounds = frame->rect;
        if (isVerticalTab(frame->shape)) {
          r.translate(0, OVERLAY_RADIUS);
          r.setBottom(qMin(r.bottom(), bounds.bottom() - OVERLAY_RADIUS));
        } else if (frame->direction == Qt::RightToLeft) {
          r.translate(-OVERLAY_RADIUS, 0);
          r.setLeft(qMax(r.left(), bounds.left() + OVERLAY_RADIUS));
        } else {
          r.translate(OVERLAY_RADIUS, 0);
          r.setRight(qMin(r.right(), bounds.right() - OVERLAY_RADIUS));
        }
        return r;
      }
      break;

    case SE_TabBarTabLeftButton:
    case SE_TabBarTabRightButton:
      if (const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
        QStyleOptionTab copy(*tab);
        copy.rect = tabContentRect(tab);
        return QProxyStyle::subElementRect(element, &copy, widget);
      }
      break;

    default:
      break;
  }
  return QProxyStyle::subElementRect(element, option, widget);
}

QSize WinUIStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size,
                                   const QWidget* widget) const {
  switch (type) {
    case CT_PushButton:
      if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
        QSize result(size.width() + 2 * BUTTON_PADDING + 2, qMax(size.height() + 12, CONTROL_HEIGHT));
        if (!button->text.isEmpty()) {
          result.setWidth(qMax(result.width(), 80));
        }
        return result;
      }
      break;

    case CT_ToolButton: {
      const QSize base = QProxyStyle::sizeFromContents(type, option, size, widget) + QSize(8, 8);
      return base.expandedTo(QSize(CONTROL_HEIGHT, CONTROL_HEIGHT));
    }

    case CT_LineEdit:
      if (const QStyleOptionFrame* frame = qstyleoption_cast<const QStyleOptionFrame*>(option)) {
        if (frame->lineWidth > 0) {
          const QSize base = QProxyStyle::sizeFromContents(type, option, size, widget);
          return QSize(base.width() + TEXT_PADDING_LEFT + TEXT_PADDING_RIGHT, qMax(base.height(), CONTROL_HEIGHT));
        }
      }
      break;

    case CT_SpinBox:
      if (const QStyleOptionSpinBox* spin = qstyleoption_cast<const QStyleOptionSpinBox*>(option)) {
        const int border = spin->frame ? FIELD_BORDER : 0;
        const int trailing =
            spin->buttonSymbols != QAbstractSpinBox::NoButtons ? 2 * SPIN_BUTTON_WIDTH + 2 : TEXT_PADDING_RIGHT;
        return QSize(size.width() + 2 * border + TEXT_PADDING_LEFT + trailing,
                     qMax(size.height() + 2 * border, CONTROL_HEIGHT));
      }
      break;

    case CT_ComboBox:
      if (const QStyleOptionComboBox* combo = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
        const int border = combo->frame ? FIELD_BORDER : 0;
        return QSize(size.width() + 2 * border + BUTTON_PADDING + COMBO_ARROW_WIDTH + 4,
                     qMax(size.height() + 2 * border + 8, CONTROL_HEIGHT));
      }
      break;

    case CT_CheckBox:
    case CT_RadioButton: {
      // WinUI check boxes and radio buttons are 32px tall
      const QSize base = QProxyStyle::sizeFromContents(type, option, size, widget);
      return QSize(base.width() + 4, qMax(base.height(), CONTROL_HEIGHT));
    }

    case CT_MenuItem:
      if (const QStyleOptionMenuItem* item = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
        const QFontMetrics metrics(item->font);
        if (item->menuItemType == QStyleOptionMenuItem::Separator) {
          return QSize(size.width(), item->text.isEmpty() ? 9 : metrics.height() + 12);
        }
        const bool comboPopup = qobject_cast<const QComboBox*>(widget);
        const int iconSize = proxy()->pixelMetric(PM_SmallIconSize, item, widget);
        QString text = item->text;
        const int tab = text.indexOf(QLatin1Char('\t'));
        if (tab >= 0) {
          text = text.left(tab);
        }

        int width = 2 * (MENU_ITEM_INSET_H + BUTTON_PADDING);
        width += comboPopup ? metrics.horizontalAdvance(text) : size.width();
        if (!comboPopup && item->menuHasCheckableItems) {
          width += MENU_CHECK_SIZE + MENU_COLUMN_SPACING;
        }
        if (item->maxIconWidth > 0 && (!comboPopup || !item->icon.isNull())) {
          width += iconSize + MENU_COLUMN_SPACING;
        }
        if (tab >= 0) {
          // QMenu adds the shortcut column width itself
          width += MENU_SHORTCUT_GAP;
        }
        if (item->menuItemType == QStyleOptionMenuItem::SubMenu) {
          width += 8 + MENU_COLUMN_SPACING;
        }
        const int pillHeight = qMax(qMax(metrics.height() + 10, iconSize + 8), 28);
        return QSize(width, pillHeight + 2 * MENU_ITEM_INSET_V);
      }
      break;

    case CT_MenuBarItem:
      return QProxyStyle::sizeFromContents(type, option, size, widget) + QSize(12, 10);

    case CT_ItemViewItem: {
      QSize result = QProxyStyle::sizeFromContents(type, option, size, widget);
      result.setHeight(qMax(result.height() + 8, CONTROL_HEIGHT));
      return result;
    }

    case CT_HeaderSection:
      if (const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader*>(option)) {
        QSize result = QProxyStyle::sizeFromContents(type, option, size, widget);
        if (header->orientation == Qt::Horizontal) {
          result.setHeight(qMax(result.height() + 8, CONTROL_HEIGHT));
        } else {
          result.rwidth() += 4;
        }
        return result;
      }
      break;

    case CT_TabBarTab:
      if (const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
        QSize result = QProxyStyle::sizeFromContents(type, option, size, widget);
        if (isVerticalTab(tab->shape)) {
          result.setHeight(qMax(result.height(), TAB_MIN_WIDTH));
        } else {
          result.setWidth(qMax(result.width(), TAB_MIN_WIDTH));
        }
        return result;
      }
      break;

    default:
      break;
  }
  return QProxyStyle::sizeFromContents(type, option, size, widget);
}

int WinUIStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const {
  switch (metric) {
    case PM_DefaultFrameWidth:
      return FRAME_WIDTH;
    case PM_SpinBoxFrameWidth:
    case PM_ComboBoxFrameWidth:
      return FIELD_BORDER;
    case PM_ButtonDefaultIndicator:
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
    case PM_TabBarTabShiftHorizontal:
    case PM_TabBarTabShiftVertical:
    case PM_MenuHMargin:
    case PM_MenuBarPanelWidth:
    case PM_MenuBarItemSpacing:
      return 0;
    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
      return INDICATOR_SIZE;
    case PM_CheckBoxLabelSpacing:
    case PM_RadioButtonLabelSpacing:
    case PM_LayoutHorizontalSpacing:
    case PM_LayoutVerticalSpacing:
    case PM_HeaderMargin:
      return 8;
    case PM_ScrollBarExtent:
      return SCROLLBAR_EXTENT;
    case PM_ScrollBarSliderMin:
      return 30;
    case PM_SliderThickness:
      return SLIDER_THUMB_SIZE + 4;
    case PM_SliderLength:
    case PM_SliderControlThickness:
      return SLIDER_THUMB_SIZE;
    case PM_SliderTickmarkOffset:
      return 5;
    case PM_FocusFrameHMargin:
      // Also the text margin in item view cells; list and tree rows leave room for the selection pill
      if (qobject_cast<const QListView*>(widget) || qobject_cast<const QTreeView*>(widget)) {
        return 12;
      }
      return 4;
    case PM_MenuVMargin:
    case PM_MenuBarVMargin:
    case PM_MenuBarHMargin:
    case PM_ToolBarItemSpacing:
      return 4;
    case PM_MenuPanelWidth:
      return 1;
    case PM_SmallIconSize:
      return 16;
    case PM_ToolTipLabelFrameWidth:
      return 6;
    case PM_TabBarTabHSpace:
      return 24;
    case PM_TabBarTabVSpace:
      return 14 + TAB_TOP_MARGIN;
    case PM_TabBarBaseOverlap:
      // The selected tab covers the pane's top border, merging into it
      return 1;
    case PM_TabCloseIndicatorWidth:
    case PM_TabCloseIndicatorHeight:
      return 20;
    default:
      break;
  }
  return QProxyStyle::pixelMetric(metric, option, widget);
}

int WinUIStyle::styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget,
                          QStyleHintReturn* returnData) const {
  switch (hint) {
    case SH_DialogButtonBox_ButtonsHaveIcons:
    case SH_EtchDisabledText:
    case SH_DitherDisabledText:
    case SH_ItemView_ShowDecorationSelected:
      return 0;
    case SH_ScrollBar_MiddleClickAbsolutePosition:
      return 1;
    case SH_Slider_AbsoluteSetButtons:
      return Qt::LeftButton;
    case SH_FormLayoutLabelAlignment:
      return int(Qt::AlignLeft | Qt::AlignVCenter);
    case SH_Table_GridLineColor:
      if (option) {
        const QColor base = option->palette.color(QPalette::Base);
        return int(mix(base, option->palette.color(QPalette::Text), 0.1).rgba());
      }
      break;
    default:
      break;
  }
  return QProxyStyle::styleHint(hint, option, widget, returnData);
}

QIcon WinUIStyle::standardIcon(StandardPixmap pixmapType, const QStyleOption* option, const QWidget* widget) const {
  if (pixmapType == SP_LineEditClearButton) {
    QPalette palette = QGuiApplication::palette();
    if (widget) {
      palette = widget->palette();
    } else if (option) {
      palette = option->palette;
    }
    return clearButtonIcon(tokensFor(isDarkPalette(palette)).m_textSecondary);
  }
  return QProxyStyle::standardIcon(pixmapType, option, widget);
}

QPixmap WinUIStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap& pixmap, const QStyleOption* option) const {
  // Selection is a subtle fill; the default accent wash would muddy icons
  if (iconMode == QIcon::Selected) {
    return pixmap;
  }
  return QProxyStyle::generatedIconPixmap(iconMode, pixmap, option);
}

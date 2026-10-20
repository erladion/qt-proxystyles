#include "Windows10Style.h"

#include "StyleHelpers.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QFrame>
#include <QGroupBox>
#include <QLineEdit>
#include <QListView>
#include <QPainter>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QRubberBand>
#include <QSlider>
#include <QStyleFactory>
#include <QStyleHints>
#include <QStyleOption>
#include <QTabBar>
#include <QTextEdit>
#include <QTime>
#include <QTimer>
#include <QToolTip>

using namespace StyleHelpers;

namespace {

constexpr int CONTROL_HEIGHT = 23;
constexpr int BUTTON_MIN_WIDTH = 75;
constexpr int FRAME_WIDTH = 1;
// SM_CXVSCROLL: scroll bars, combo drop buttons and spin buttons share this width
constexpr int SYSTEM_BUTTON_WIDTH = 17;
constexpr int INDICATOR_SIZE = 13;
constexpr int SLIDER_THUMB_LENGTH = 11;
constexpr int SLIDER_THUMB_THICKNESS = 21;
constexpr int SLIDER_TRACK_THICKNESS = 4;
constexpr int SLIDER_THUMB_POINT = 5;
constexpr int TAB_RAISE = 2;
constexpr int MENU_GUTTER_WIDTH = 28;
constexpr int MENU_TEXT_OFFSET = 30;
constexpr int MENU_ARROW_COLUMN = 22;
constexpr int MENU_SHORTCUT_GAP = 24;
constexpr int MENU_ITEM_HEIGHT = 22;
constexpr int ANIMATION_INTERVAL_MS = 16;
constexpr int MARQUEE_CYCLE_MS = 2000;

QColor rgb(QRgb value) {
  return QColor::fromRgb(value);
}

// Accent blended onto white, the way the Windows 10 theme derives its pale blues
QColor tint(const QColor& accent, qreal amount) {
  return mix(QColor(0xFF, 0xFF, 0xFF), accent, amount);
}

// Explorer's selection blues are a brighter take on the accent (#0078D7 -> #008EFF)
QColor brightened(const QColor& accent) {
  const QColor hsv = accent.toHsv();
  return QColor::fromHsvF(hsv.hsvHueF(), hsv.hsvSaturationF(), 1.0);
}

struct Colors {
  QColor m_accent;
  QColor m_text;
  QColor m_textDisabled;
  QColor m_focusRect;
  QColor m_buttonBorder;
  QColor m_buttonHoverFill;
  QColor m_buttonHoverBorder;
  QColor m_buttonPressedFill;
  QColor m_buttonPressedBorder;
  QColor m_buttonDisabledBorder;
  QColor m_buttonDisabledText;
  QColor m_editBorder;
  QColor m_editHoverBorder;
  QColor m_editDisabledBorder;
  QColor m_glyph;
  QColor m_glyphDisabled;
  QColor m_checkBorder;
  QColor m_checkHover;
  QColor m_checkPressed;
  QColor m_checkPressedFill;
  QColor m_checkDisabled;
  QColor m_itemHover;
  QColor m_itemSelected;
  QColor m_itemSelectedBorder;
  QColor m_itemInactive;
  QColor m_scrollTrack;
  QColor m_scrollThumb;
  QColor m_scrollThumbHover;
  QColor m_scrollThumbPressed;
  QColor m_scrollArrow;
  QColor m_scrollArrowHoverFill;
  QColor m_scrollArrowHover;
  QColor m_scrollArrowPressedFill;
  QColor m_scrollArrowPressed;
  QColor m_scrollArrowDisabled;
  QColor m_trackFill;
  QColor m_trackBorder;
  QColor m_sliderThumbHover;
  QColor m_sliderThumbPressed;
  QColor m_sliderThumbDisabled;
  QColor m_tick;
  QColor m_progressFill;
  QColor m_progressBorder;
  QColor m_progressChunk;
  QColor m_tabBorder;
  QColor m_tabFill;
  QColor m_tabSelectedFill;
  QColor m_tabHoverFill;
  QColor m_tabHoverBorder;
  QColor m_groupBorder;
  QColor m_viewBorder;
  QColor m_popupBorder;
  QColor m_separator;
  QColor m_etchShadow;
  QColor m_etchHighlight;
  QColor m_menuFill;
  QColor m_menuBorder;
  QColor m_menuHot;
  QColor m_menuHotDisabled;
  QColor m_menuSeparator;
  QColor m_menuBarFill;
  QColor m_headerSeparator;
  QColor m_headerHover;
  QColor m_headerHoverBorder;
  QColor m_headerPressed;
  QColor m_headerPressedBorder;
  QColor m_headerText;
  QColor m_treeGlyph;
  QColor m_treeGlyphHover;
  QColor m_toolTipBorder;
};

Colors makeLightColors(const QColor& accent) {
  const QColor explorer = brightened(accent);
  Colors colors;
  colors.m_accent = accent;
  colors.m_text = rgb(0x000000);
  colors.m_textDisabled = rgb(0x6D6D6D);
  colors.m_focusRect = rgb(0x000000);
  colors.m_buttonBorder = rgb(0xADADAD);
  colors.m_buttonHoverFill = tint(accent, 0.1);
  colors.m_buttonHoverBorder = accent;
  colors.m_buttonPressedFill = tint(accent, 0.2);
  colors.m_buttonPressedBorder = accent.darker(141);
  colors.m_buttonDisabledBorder = rgb(0xBFBFBF);
  colors.m_buttonDisabledText = rgb(0x838383);
  colors.m_editBorder = rgb(0x7A7A7A);
  colors.m_editHoverBorder = rgb(0x171717);
  colors.m_editDisabledBorder = rgb(0xCCCCCC);
  colors.m_glyph = rgb(0x606060);
  colors.m_glyphDisabled = rgb(0xBFBFBF);
  colors.m_checkBorder = rgb(0x333333);
  colors.m_checkHover = accent;
  colors.m_checkPressed = accent.darker(141);
  colors.m_checkPressedFill = tint(accent, 0.2);
  colors.m_checkDisabled = rgb(0xCCCCCC);
  colors.m_itemHover = tint(explorer, 0.1);
  colors.m_itemSelected = tint(explorer, 0.2);
  colors.m_itemSelectedBorder = tint(explorer, 0.4);
  colors.m_itemInactive = rgb(0xD9D9D9);
  colors.m_scrollTrack = rgb(0xF0F0F0);
  colors.m_scrollThumb = rgb(0xCDCDCD);
  colors.m_scrollThumbHover = rgb(0xA6A6A6);
  colors.m_scrollThumbPressed = rgb(0x606060);
  colors.m_scrollArrow = rgb(0x606060);
  colors.m_scrollArrowHoverFill = rgb(0xDADADA);
  colors.m_scrollArrowHover = rgb(0x000000);
  colors.m_scrollArrowPressedFill = rgb(0x606060);
  colors.m_scrollArrowPressed = rgb(0xFFFFFF);
  colors.m_scrollArrowDisabled = rgb(0xBFBFBF);
  colors.m_trackFill = rgb(0xE7EAEA);
  colors.m_trackBorder = rgb(0xD6D6D6);
  colors.m_sliderThumbHover = rgb(0x171717);
  colors.m_sliderThumbPressed = rgb(0xCCCCCC);
  colors.m_sliderThumbDisabled = rgb(0xCCCCCC);
  colors.m_tick = rgb(0xC4C4C4);
  colors.m_progressFill = rgb(0xE6E6E6);
  colors.m_progressBorder = rgb(0xBCBCBC);
  colors.m_progressChunk = rgb(0x06B025);
  colors.m_tabBorder = rgb(0xD9D9D9);
  colors.m_tabFill = rgb(0xF0F0F0);
  colors.m_tabSelectedFill = rgb(0xFFFFFF);
  colors.m_tabHoverFill = tint(accent, 0.15);
  colors.m_tabHoverBorder = tint(accent, 0.5);
  colors.m_groupBorder = rgb(0xDCDCDC);
  colors.m_viewBorder = rgb(0x828790);
  colors.m_popupBorder = rgb(0x646464);
  colors.m_separator = rgb(0xCCCCCC);
  colors.m_etchShadow = rgb(0xA0A0A0);
  colors.m_etchHighlight = rgb(0xFFFFFF);
  colors.m_menuFill = rgb(0xF2F2F2);
  colors.m_menuBorder = rgb(0xCCCCCC);
  colors.m_menuHot = tint(accent, 0.43);
  colors.m_menuHotDisabled = rgb(0xE6E6E6);
  colors.m_menuSeparator = rgb(0xBDBDBD);
  colors.m_menuBarFill = rgb(0xFFFFFF);
  colors.m_headerSeparator = rgb(0xE5E5E5);
  colors.m_headerHover = tint(accent, 0.15);
  colors.m_headerHoverBorder = tint(accent, 0.26);
  colors.m_headerPressed = tint(accent, 0.26);
  colors.m_headerPressedBorder = tint(accent, 0.5);
  colors.m_headerText = rgb(0x4C607A);
  colors.m_treeGlyph = rgb(0xA6A6A6);
  colors.m_treeGlyphHover = rgb(0x1CC4F7);
  colors.m_toolTipBorder = rgb(0x767676);
  return colors;
}

Colors makeDarkColors(const QColor& accent) {
  Colors colors;
  colors.m_accent = accent;
  colors.m_text = rgb(0xFFFFFF);
  colors.m_textDisabled = rgb(0x6D6D6D);
  colors.m_focusRect = rgb(0xFFFFFF);
  colors.m_buttonBorder = rgb(0x6F6F6F);
  colors.m_buttonHoverFill = rgb(0x454545);
  colors.m_buttonHoverBorder = accent;
  colors.m_buttonPressedFill = rgb(0x666666);
  colors.m_buttonPressedBorder = accent;
  colors.m_buttonDisabledBorder = rgb(0x434343);
  colors.m_buttonDisabledText = rgb(0x6D6D6D);
  colors.m_editBorder = rgb(0x7A7A7A);
  colors.m_editHoverBorder = rgb(0xC8C8C8);
  colors.m_editDisabledBorder = rgb(0x434343);
  colors.m_glyph = rgb(0x9B9B9B);
  colors.m_glyphDisabled = rgb(0x4D4D4D);
  colors.m_checkBorder = rgb(0xC8C8C8);
  colors.m_checkHover = accent.lighter(130);
  colors.m_checkPressed = accent;
  colors.m_checkPressedFill = rgb(0x333333);
  colors.m_checkDisabled = rgb(0x4D4D4D);
  colors.m_itemHover = rgb(0x3A3A3A);
  colors.m_itemSelected = rgb(0x4D4D4D);
  colors.m_itemSelectedBorder = rgb(0x666666);
  colors.m_itemInactive = rgb(0x383838);
  colors.m_scrollTrack = rgb(0x171717);
  colors.m_scrollThumb = rgb(0x4D4D4D);
  colors.m_scrollThumbHover = rgb(0x7A7A7A);
  colors.m_scrollThumbPressed = rgb(0xA6A6A6);
  colors.m_scrollArrow = rgb(0x9B9B9B);
  colors.m_scrollArrowHoverFill = rgb(0x373737);
  colors.m_scrollArrowHover = rgb(0xFFFFFF);
  colors.m_scrollArrowPressedFill = rgb(0xA6A6A6);
  colors.m_scrollArrowPressed = rgb(0x000000);
  colors.m_scrollArrowDisabled = rgb(0x434343);
  colors.m_trackFill = rgb(0x4D4D4D);
  colors.m_trackBorder = rgb(0x6F6F6F);
  colors.m_sliderThumbHover = rgb(0xFFFFFF);
  colors.m_sliderThumbPressed = rgb(0x9B9B9B);
  colors.m_sliderThumbDisabled = rgb(0x4D4D4D);
  colors.m_tick = rgb(0x6F6F6F);
  colors.m_progressFill = rgb(0x333333);
  colors.m_progressBorder = rgb(0x4D4D4D);
  colors.m_progressChunk = rgb(0x06B025);
  colors.m_tabBorder = rgb(0x434343);
  colors.m_tabFill = rgb(0x202020);
  colors.m_tabSelectedFill = rgb(0x2B2B2B);
  colors.m_tabHoverFill = rgb(0x333333);
  colors.m_tabHoverBorder = rgb(0x6F6F6F);
  colors.m_groupBorder = rgb(0x434343);
  colors.m_viewBorder = rgb(0x646464);
  colors.m_popupBorder = rgb(0x767676);
  colors.m_separator = rgb(0x434343);
  colors.m_etchShadow = rgb(0x434343);
  colors.m_etchHighlight = rgb(0x2B2B2B);
  colors.m_menuFill = rgb(0x2B2B2B);
  colors.m_menuBorder = rgb(0x5A5A5A);
  colors.m_menuHot = rgb(0x414141);
  colors.m_menuHotDisabled = rgb(0x333333);
  colors.m_menuSeparator = rgb(0x808080);
  colors.m_menuBarFill = rgb(0x202020);
  colors.m_headerSeparator = rgb(0x434343);
  colors.m_headerHover = rgb(0x333333);
  colors.m_headerHoverBorder = rgb(0x434343);
  colors.m_headerPressed = rgb(0x434343);
  colors.m_headerPressedBorder = rgb(0x666666);
  colors.m_headerText = rgb(0xDEDEDE);
  colors.m_treeGlyph = rgb(0x9B9B9B);
  colors.m_treeGlyphHover = rgb(0xFFFFFF);
  colors.m_toolTipBorder = rgb(0x767676);
  return colors;
}

QColor defaultAccent() {
  return QColor(0x00, 0x78, 0xD7);
}

// Win32 controls are drawn with crisp 1px borders
void drawBox(QPainter* painter, const QRect& rect, const QColor& fill, const QColor& border, int borderWidth = 1) {
  if (fill.isValid()) {
    painter->fillRect(rect, fill);
  }
  if (!border.isValid() || rect.isEmpty()) {
    return;
  }
  for (int i = 0; i < borderWidth; i++) {
    const QRect r = rect.adjusted(i, i, -i, -i);
    painter->fillRect(QRect(r.left(), r.top(), r.width(), 1), border);
    painter->fillRect(QRect(r.left(), r.bottom(), r.width(), 1), border);
    painter->fillRect(QRect(r.left(), r.top(), 1, r.height()), border);
    painter->fillRect(QRect(r.right(), r.top(), 1, r.height()), border);
  }
}

// Win32's dotted focus rectangle, shown only after keyboard navigation
void drawDottedRect(QPainter* painter, const QRect& rect, const QColor& color) {
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, false);
  QPen pen(color, 0);
  pen.setDashPattern({1.0, 1.0});
  painter->setPen(pen);
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(rect.adjusted(0, 0, -1, -1));
  painter->restore();
}

bool isComboPopupContainer(const QWidget* widget) {
  return widget && widget->inherits("QComboBoxPrivateContainer");
}

// Combo drop-down lists keep the classic solid selection
bool isInComboPopup(const QWidget* widget) {
  return widget && isComboPopupContainer(widget->window());
}

bool isTextEdit(const QWidget* widget) {
  return qobject_cast<const QTextEdit*>(widget) || qobject_cast<const QPlainTextEdit*>(widget);
}

// Frameless line edits inside spin boxes and combos let their parent's field show through
bool isEmbeddedLineEdit(const QWidget* widget) {
  const QWidget* parent = widget ? widget->parentWidget() : nullptr;
  return qobject_cast<const QAbstractSpinBox*>(parent) || qobject_cast<const QComboBox*>(parent);
}

bool wantsHover(const QWidget* widget) {
  return qobject_cast<const QAbstractButton*>(widget) || qobject_cast<const QComboBox*>(widget) ||
         qobject_cast<const QAbstractSpinBox*>(widget) || qobject_cast<const QAbstractSlider*>(widget) ||
         qobject_cast<const QTabBar*>(widget) || qobject_cast<const QLineEdit*>(widget) ||
         qobject_cast<const QGroupBox*>(widget) || isTextEdit(widget);
}

} // namespace

Windows10Style::Windows10Style(Theme theme, const QColor& accentColor)
    : QProxyStyle(QStyleFactory::create(QStringLiteral("Fusion"))),
      m_theme(theme),
      m_accentColor(accentColor.isValid() ? accentColor : defaultAccent()),
      m_systemIsDark(false),
      m_pAnimationTimer(new QTimer(this)) {
  setObjectName(QStringLiteral("Windows10"));

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

Windows10Style::Theme Windows10Style::theme() const {
  return m_theme;
}

void Windows10Style::setTheme(Theme theme) {
  if (m_theme == theme) {
    return;
  }
  m_theme = theme;
  applyPalette();
}

QColor Windows10Style::accentColor() const {
  return m_accentColor;
}

void Windows10Style::setAccentColor(const QColor& color) {
  const QColor accent = color.isValid() ? color : defaultAccent();
  if (m_accentColor == accent) {
    return;
  }
  m_accentColor = accent;
  applyPalette(true);
}

bool Windows10Style::isDark() const {
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

QFont Windows10Style::recommendedFont() {
  QFont font = QGuiApplication::font();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  const QStringList families = QFontDatabase::families();
#else
  const QStringList families = QFontDatabase().families();
#endif
  if (families.contains(QStringLiteral("Segoe UI"))) {
    font.setFamily(QStringLiteral("Segoe UI"));
  }
  font.setPointSize(9);
  return font;
}

void Windows10Style::applyPalette(bool accentChanged) {
  if (!qApp || QApplication::style() != this) {
    return;
  }
  // Keep roles the application set itself; only colors that came from this style change
  const QPalette next = standardPalette();
  const QPalette palette = mergeCustomRoles(next, m_appliedPalette, QApplication::palette(), accentChanged);
  m_appliedPalette = next;
  QApplication::setPalette(palette);
}

void Windows10Style::scheduleAnimationFrame(const QWidget* widget) const {
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

QPalette buildPalette(bool dark, const QColor& accent) {

  QColor window, text, base, alternateBase, button, light, midlight, mid, darkShade, shadow;
  QColor link, toolTipBase, toolTipText, placeholder, disabledText, disabledBase, disabledButton;
  if (dark) {
    window = rgb(0x202020);
    text = rgb(0xFFFFFF);
    base = rgb(0x191919);
    alternateBase = rgb(0x262626);
    button = rgb(0x333333);
    light = rgb(0x4D4D4D);
    midlight = rgb(0x3A3A3A);
    mid = rgb(0x6F6F6F);
    darkShade = rgb(0x141414);
    shadow = rgb(0x000000);
    link = accent.lighter(150);
    toolTipBase = rgb(0x2B2B2B);
    toolTipText = rgb(0xFFFFFF);
    placeholder = rgb(0x9B9B9B);
    disabledText = rgb(0x6D6D6D);
    disabledBase = rgb(0x262626);
    disabledButton = rgb(0x2B2B2B);
  } else {
    // COLOR_BTNFACE, COLOR_WINDOW and friends as the Windows 10 theme sets them
    window = rgb(0xF0F0F0);
    text = rgb(0x000000);
    base = rgb(0xFFFFFF);
    alternateBase = rgb(0xF7F7F7);
    button = rgb(0xE1E1E1);
    light = rgb(0xFFFFFF);
    midlight = rgb(0xE3E3E3);
    mid = rgb(0xA0A0A0);
    darkShade = rgb(0xA0A0A0);
    shadow = rgb(0x696969);
    link = rgb(0x0066CC);
    toolTipBase = rgb(0xFFFFFF);
    toolTipText = rgb(0x575757);
    placeholder = rgb(0x6D6D6D);
    disabledText = rgb(0x6D6D6D);
    disabledBase = rgb(0xF0F0F0);
    disabledButton = rgb(0xCCCCCC);
  }

  QPalette palette;
  palette.setColor(QPalette::All, QPalette::Window, window);
  palette.setColor(QPalette::All, QPalette::WindowText, text);
  palette.setColor(QPalette::All, QPalette::Base, base);
  palette.setColor(QPalette::All, QPalette::AlternateBase, alternateBase);
  palette.setColor(QPalette::All, QPalette::Button, button);
  palette.setColor(QPalette::All, QPalette::ButtonText, text);
  palette.setColor(QPalette::All, QPalette::Text, text);
  palette.setColor(QPalette::All, QPalette::BrightText, rgb(0xFFFFFF));
  palette.setColor(QPalette::All, QPalette::Light, light);
  palette.setColor(QPalette::All, QPalette::Midlight, midlight);
  palette.setColor(QPalette::All, QPalette::Mid, mid);
  palette.setColor(QPalette::All, QPalette::Dark, darkShade);
  palette.setColor(QPalette::All, QPalette::Shadow, shadow);
  palette.setColor(QPalette::All, QPalette::Highlight, accent);
  palette.setColor(QPalette::All, QPalette::HighlightedText, rgb(0xFFFFFF));
  palette.setColor(QPalette::All, QPalette::Link, link);
  palette.setColor(QPalette::All, QPalette::LinkVisited, link);
  palette.setColor(QPalette::All, QPalette::ToolTipBase, toolTipBase);
  palette.setColor(QPalette::All, QPalette::ToolTipText, toolTipText);
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
  palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, disabledText);
#endif
  return palette;
}

const QPalette& basePalette(bool dark) {
  static const QPalette light = buildPalette(false, defaultAccent());
  static const QPalette darkPalette = buildPalette(true, defaultAccent());
  return dark ? darkPalette : light;
}

// The style's own palette per theme: the baseline for telling which roles the
// application or a widget overrides. Accent roles aren't compared, so the default
// accent stands in for any.
const QPalette& basePalette(const QStyleOption* option) {
  return basePalette(isDarkPalette(option->palette));
}

// Colors depend on the palette's theme, accent and neutrals; cache the last set, as one
// widget tree rarely mixes them
const Colors& colorsFor(const QStyleOption* option) {
  static Colors cached;
  static bool hasCache = false;
  static bool cachedDark = false;
  static QRgb cachedAccent = 0;
  static QRgb cachedWindow = 0;
  static QRgb cachedText = 0;
  const QPalette& palette = option->palette;
  const bool dark = isDarkPalette(palette);
  const QRgb accent = palette.color(QPalette::Active, QPalette::Highlight).rgba();
  const QRgb window = palette.color(QPalette::Active, QPalette::Window).rgba();
  const QRgb text = palette.color(QPalette::Active, QPalette::WindowText).rgba();
  if (hasCache && cachedDark == dark && cachedAccent == accent && cachedWindow == window && cachedText == text) {
    return cached;
  }
  cached = dark ? makeDarkColors(QColor::fromRgba(accent)) : makeLightColors(QColor::fromRgba(accent));
  // A palette with other window and text colors re-tints the theme's greys to match
  const QPalette& standard = basePalette(dark);
  if (!hasStandardNeutrals(palette, standard)) {
    static QColor Colors::*const neutrals[] = {
      &Colors::m_text,
      &Colors::m_textDisabled,
      &Colors::m_focusRect,
      &Colors::m_buttonBorder,
      &Colors::m_buttonHoverFill,
      &Colors::m_buttonHoverBorder,
      &Colors::m_buttonPressedFill,
      &Colors::m_buttonPressedBorder,
      &Colors::m_buttonDisabledBorder,
      &Colors::m_buttonDisabledText,
      &Colors::m_editBorder,
      &Colors::m_editHoverBorder,
      &Colors::m_editDisabledBorder,
      &Colors::m_glyph,
      &Colors::m_glyphDisabled,
      &Colors::m_checkBorder,
      &Colors::m_checkHover,
      &Colors::m_checkPressed,
      &Colors::m_checkPressedFill,
      &Colors::m_checkDisabled,
      &Colors::m_itemHover,
      &Colors::m_itemSelected,
      &Colors::m_itemSelectedBorder,
      &Colors::m_itemInactive,
      &Colors::m_scrollTrack,
      &Colors::m_scrollThumb,
      &Colors::m_scrollThumbHover,
      &Colors::m_scrollThumbPressed,
      &Colors::m_scrollArrow,
      &Colors::m_scrollArrowHoverFill,
      &Colors::m_scrollArrowHover,
      &Colors::m_scrollArrowPressedFill,
      &Colors::m_scrollArrowPressed,
      &Colors::m_scrollArrowDisabled,
      &Colors::m_trackFill,
      &Colors::m_trackBorder,
      &Colors::m_sliderThumbHover,
      &Colors::m_sliderThumbPressed,
      &Colors::m_sliderThumbDisabled,
      &Colors::m_tick,
      &Colors::m_progressFill,
      &Colors::m_progressBorder,
      &Colors::m_progressChunk,
      &Colors::m_tabBorder,
      &Colors::m_tabFill,
      &Colors::m_tabSelectedFill,
      &Colors::m_tabHoverFill,
      &Colors::m_tabHoverBorder,
      &Colors::m_groupBorder,
      &Colors::m_viewBorder,
      &Colors::m_popupBorder,
      &Colors::m_separator,
      &Colors::m_etchShadow,
      &Colors::m_etchHighlight,
      &Colors::m_menuFill,
      &Colors::m_menuBorder,
      &Colors::m_menuHot,
      &Colors::m_menuHotDisabled,
      &Colors::m_menuSeparator,
      &Colors::m_menuBarFill,
      &Colors::m_headerSeparator,
      &Colors::m_headerHover,
      &Colors::m_headerHoverBorder,
      &Colors::m_headerPressed,
      &Colors::m_headerPressedBorder,
      &Colors::m_headerText,
      &Colors::m_treeGlyph,
      &Colors::m_treeGlyphHover,
      &Colors::m_toolTipBorder,
    };
    for (QColor Colors::*neutral : neutrals) {
      cached.*neutral = retint(cached.*neutral, standard, palette);
    }
  }
  hasCache = true;
  cachedDark = dark;
  cachedAccent = accent;
  cachedWindow = window;
  cachedText = text;
  return cached;
}

} // namespace

QPalette Windows10Style::standardPalette() const {
  return buildPalette(isDark(), m_accentColor);
}

void Windows10Style::polish(QPalette& palette) {
  palette = standardPalette();
  m_appliedPalette = palette;
}

void Windows10Style::polish(QApplication* application) {
  QProxyStyle::polish(application);
  // Tooltips keep a palette of their own; follow the application's from now on
  QToolTip::setPalette(QApplication::palette());
  application->installEventFilter(this);
}

void Windows10Style::unpolish(QApplication* application) {
  application->removeEventFilter(this);
  QProxyStyle::unpolish(application);
}

bool Windows10Style::eventFilter(QObject* watched, QEvent* event) {
  if (event->type() == QEvent::ApplicationPaletteChange && watched == qApp) {
    QToolTip::setPalette(QApplication::palette());
  }
  return QProxyStyle::eventFilter(watched, event);
}

void Windows10Style::polish(QWidget* widget) {
  QProxyStyle::polish(widget);
  if (wantsHover(widget)) {
    widget->setAttribute(Qt::WA_Hover, true);
  }
  if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(widget)) {
    view->viewport()->setAttribute(Qt::WA_Hover, true);
  }
}

void Windows10Style::unpolish(QWidget* widget) {
  if (wantsHover(widget)) {
    widget->setAttribute(Qt::WA_Hover, false);
  }
  if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(widget)) {
    view->viewport()->setAttribute(Qt::WA_Hover, false);
  }
  QProxyStyle::unpolish(widget);
}

// ---------------------------------------------------------------------------
// Primitives
// ---------------------------------------------------------------------------

void Windows10Style::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter,
                                   const QWidget* widget) const {
  const Colors& colors = colorsFor(option);
  switch (element) {
    case PE_FrameFocusRect:
      // Explorer-style views mark the current item with its selection instead
      if (!qobject_cast<const QAbstractItemView*>(widget) && (option->state & State_KeyboardFocusChange)) {
        drawDottedRect(painter, option->rect, colors.m_focusRect);
      }
      return;

    case PE_FrameDefaultButton:
    case PE_FrameStatusBarItem:
    case PE_PanelMenuBar:
      return;

    case PE_PanelButtonCommand: {
      const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option);
      const bool isDefault = button && (button->features & QStyleOptionButton::DefaultButton);
      const bool isFlat = button && (button->features & QStyleOptionButton::Flat);
      drawPushButton(option, painter, isDefault, isFlat);
      return;
    }

    case PE_PanelButtonTool:
    case PE_PanelButtonBevel:
    case PE_IndicatorButtonDropDown:
      drawToolButtonPanel(option, painter);
      return;

    case PE_PanelLineEdit: {
      const QStyleOptionFrame* frame = qstyleoption_cast<const QStyleOptionFrame*>(option);
      if (frame && frame->lineWidth <= 0) {
        if (!isEmbeddedLineEdit(widget)) {
          painter->fillRect(option->rect, option->palette.brush(colorGroup(option), QPalette::Base));
        }
        return;
      }
      drawEditFrame(option, painter, true);
      return;
    }

    case PE_FrameLineEdit:
      drawEditFrame(option, painter, false);
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
        drawBox(painter, option->rect, QColor(), colors.m_popupBorder);
      } else if (isTextEdit(widget)) {
        // Multi-line edits share the single-line edit border and its states
        drawEditFrame(option, painter, false);
      } else {
        drawBox(painter, option->rect, QColor(), colors.m_viewBorder);
      }
      return;

    case PE_FrameGroupBox: {
      const QStyleOptionFrame* frame = qstyleoption_cast<const QStyleOptionFrame*>(option);
      if (frame && (frame->features & QStyleOptionFrame::Flat)) {
        painter->fillRect(QRect(option->rect.left(), option->rect.top(), option->rect.width(), 1),
                          colors.m_groupBorder);
      } else {
        drawBox(painter, option->rect, QColor(), colors.m_groupBorder);
      }
      return;
    }

    case PE_FrameTabWidget:
      drawBox(painter, option->rect, colors.m_tabSelectedFill, colors.m_tabBorder);
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
        painter->fillRect(line, colors.m_tabBorder);
        return;
      }
      break;

    case PE_PanelMenu:
      painter->fillRect(option->rect, overriddenOr(option, QPalette::Base, basePalette(option), colors.m_menuFill));
      return;

    case PE_FrameMenu:
      drawBox(painter, option->rect, QColor(), colors.m_menuBorder);
      return;

    case PE_PanelTipLabel:
      drawBox(painter, option->rect, option->palette.color(QPalette::ToolTipBase), colors.m_toolTipBorder);
      return;

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

    case PE_PanelScrollAreaCorner:
      painter->fillRect(option->rect, colors.m_scrollTrack);
      return;

    case PE_IndicatorBranch: {
      if (!(option->state & State_Children)) {
        return;
      }
      // Windows 8 and later use chevrons instead of triangles in tree views
      Qt::ArrowType arrow = option->direction == Qt::RightToLeft ? Qt::LeftArrow : Qt::RightArrow;
      if (option->state & State_Open) {
        arrow = Qt::DownArrow;
      }
      const QColor color = (option->state & State_MouseOver) ? colors.m_treeGlyphHover : colors.m_treeGlyph;
      drawChevron(painter, option->rect, arrow, color, 8.0, 1.0);
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
      const QColor color = (option->state & State_Enabled) ? colors.m_text : colors.m_glyphDisabled;
      drawCaret(painter, option->rect, arrow, color, qMin<qreal>(7.0, qMin(option->rect.width(), option->rect.height())));
      return;
    }

    case PE_IndicatorHeaderArrow:
      if (const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader*>(option)) {
        if (header->sortIndicator & QStyleOptionHeader::SortUp) {
          drawChevron(painter, option->rect, Qt::UpArrow, colors.m_treeGlyph, 7.0, 1.0);
        } else if (header->sortIndicator & QStyleOptionHeader::SortDown) {
          drawChevron(painter, option->rect, Qt::DownArrow, colors.m_treeGlyph, 7.0, 1.0);
        }
        return;
      }
      break;

    case PE_IndicatorToolBarSeparator: {
      const QRect r = option->rect;
      if (option->state & State_Horizontal) {
        painter->fillRect(QRect(r.center().x(), r.top() + 3, 1, r.height() - 6), colors.m_separator);
      } else {
        painter->fillRect(QRect(r.left() + 3, r.center().y(), r.width() - 6, 1), colors.m_separator);
      }
      return;
    }

    default:
      break;
  }
  QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void Windows10Style::drawPushButton(const QStyleOption* option, QPainter* painter, bool isDefault,
                                    bool isFlat) const {
  const Colors& colors = colorsFor(option);
  const bool enabled = option->state & State_Enabled;
  const bool hovered = enabled && (option->state & State_MouseOver);
  const bool pressed = enabled && (option->state & (State_Sunken | State_On));

  if (isFlat && !hovered && !pressed) {
    return;
  }
  // The theme's button grey is the palette's Button; a custom Button gets derived hover and pressed shades
  const QColor custom = overriddenColor(option, QPalette::Button, basePalette(option));
  const QColor fill = paletteColor(option, QPalette::Button);
  if (!enabled) {
    drawBox(painter, option->rect, fill, colors.m_buttonDisabledBorder);
  } else if (pressed) {
    drawBox(painter, option->rect, custom.isValid() ? shade(custom, 0.12) : colors.m_buttonPressedFill,
            colors.m_buttonPressedBorder);
  } else if (hovered) {
    drawBox(painter, option->rect, custom.isValid() ? shade(custom, 0.06) : colors.m_buttonHoverFill,
            colors.m_buttonHoverBorder);
  } else if (isDefault) {
    // The default button carries a 2px accent border
    drawBox(painter, option->rect, fill, colors.m_accent, 2);
  } else {
    drawBox(painter, option->rect, fill, colors.m_buttonBorder);
  }
}

void Windows10Style::drawToolButtonPanel(const QStyleOption* option, QPainter* painter) const {
  if (!(option->state & State_AutoRaise)) {
    drawPushButton(option, painter, false, false);
    return;
  }
  // Toolbar buttons use the Explorer blues
  const Colors& colors = colorsFor(option);
  const bool enabled = option->state & State_Enabled;
  if (!enabled) {
    return;
  }
  if (option->state & (State_Sunken | State_On)) {
    drawBox(painter, option->rect, colors.m_itemSelected, colors.m_itemSelectedBorder);
  } else if (option->state & State_MouseOver) {
    drawBox(painter, option->rect, colors.m_itemHover, colors.m_itemSelected);
  }
}

void Windows10Style::drawEditFrame(const QStyleOption* option, QPainter* painter, bool fill) const {
  const Colors& colors = colorsFor(option);
  const bool enabled = option->state & State_Enabled;
  QColor border = colors.m_editBorder;
  if (!enabled) {
    border = colors.m_editDisabledBorder;
  } else if (option->state & State_HasFocus) {
    border = colors.m_accent;
  } else if (option->state & State_MouseOver) {
    border = colors.m_editHoverBorder;
  }
  // The theme's field colors are the palette's Base, so a custom Base shows through
  const QColor background = fill ? paletteColor(option, QPalette::Base) : QColor();
  drawBox(painter, option->rect, background, border);
}

void Windows10Style::drawCheckIndicator(const QStyleOption* option, QPainter* painter, bool isRadio) const {
  const Colors& colors = colorsFor(option);
  const int size = qMin(qMin(option->rect.width(), option->rect.height()), INDICATOR_SIZE);
  QRect box(0, 0, size, size);
  box.moveCenter(option->rect.center());

  const bool enabled = option->state & State_Enabled;
  const bool hovered = enabled && (option->state & State_MouseOver);
  const bool pressed = enabled && (option->state & State_Sunken);
  const bool checked = option->state & State_On;
  const bool partial = !isRadio && (option->state & State_NoChange);

  // Border and glyph share one color that follows the state
  QColor ink = colors.m_checkBorder;
  QColor fill = paletteColor(option, QPalette::Base);
  if (!enabled) {
    ink = colors.m_checkDisabled;
    fill = option->palette.color(QPalette::Window);
  } else if (pressed) {
    ink = colors.m_checkPressed;
    fill = colors.m_checkPressedFill;
  } else if (hovered) {
    ink = colors.m_checkHover;
  }

  if (isRadio) {
    const QRectF circle = QRectF(box).adjusted(0.5, 0.5, -0.5, -0.5);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(ink, 1.0));
    painter->setBrush(fill);
    painter->drawEllipse(circle);
    if (checked) {
      const qreal dot = 5.0;
      painter->setPen(Qt::NoPen);
      painter->setBrush(ink);
      painter->drawEllipse(QRectF(circle.center().x() - dot / 2.0, circle.center().y() - dot / 2.0, dot, dot));
    }
    painter->restore();
    return;
  }

  drawBox(painter, box, fill, ink);
  if (partial && !checked) {
    painter->fillRect(box.adjusted(3, 3, -3, -3), ink);
  } else if (checked) {
    const QRectF r(box);
    const QPointF points[3] = {
      QPointF(r.left() + r.width() * 0.22, r.top() + r.height() * 0.50),
      QPointF(r.left() + r.width() * 0.42, r.top() + r.height() * 0.70),
      QPointF(r.left() + r.width() * 0.78, r.top() + r.height() * 0.30),
    };
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QPen pen(ink, 1.3);
    pen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(pen);
    painter->drawPolyline(points, 3);
    painter->restore();
  }
}

void Windows10Style::drawItemViewBackground(const QStyleOption* option, QPainter* painter,
                                            const QWidget* widget) const {
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

  const Colors& colors = colorsFor(item);
  const bool enabled = item->state & State_Enabled;
  const bool selected = item->state & State_Selected;
  const bool hovered = enabled && (item->state & State_MouseOver);
  if (isInComboPopup(widget)) {
    if (selected) {
      painter->fillRect(item->rect, item->palette.brush(QPalette::Active, QPalette::Highlight));
    }
    return;
  }
  if (!selected && !hovered) {
    return;
  }

  // Explorer theme: pale blue fill, with a border once selected; grey in inactive windows
  QColor fill = colors.m_itemHover;
  QColor border;
  if (selected) {
    const bool active = item->state & State_Active;
    fill = active ? colors.m_itemSelected : colors.m_itemInactive;
    border = active ? colors.m_itemSelectedBorder : colors.m_itemInactive;
  }
  painter->fillRect(item->rect, fill);

  QStyleOptionViewItem::ViewItemPosition position = item->viewItemPosition;
  if (position == QStyleOptionViewItem::Invalid && qobject_cast<const QListView*>(widget)) {
    position = QStyleOptionViewItem::OnlyOne;
  }
  if (!border.isValid() || position == QStyleOptionViewItem::Invalid) {
    return;
  }
  // One border around the whole row, not around each cell
  const QRect r = item->rect;
  const bool isStart = position == QStyleOptionViewItem::Beginning || position == QStyleOptionViewItem::OnlyOne;
  const bool isEnd = position == QStyleOptionViewItem::End || position == QStyleOptionViewItem::OnlyOne;
  const bool rightToLeft = item->direction == Qt::RightToLeft;
  painter->fillRect(QRect(r.left(), r.top(), r.width(), 1), border);
  painter->fillRect(QRect(r.left(), r.bottom(), r.width(), 1), border);
  if (rightToLeft ? isEnd : isStart) {
    painter->fillRect(QRect(r.left(), r.top(), 1, r.height()), border);
  }
  if (rightToLeft ? isStart : isEnd) {
    painter->fillRect(QRect(r.right(), r.top(), 1, r.height()), border);
  }
}

// ---------------------------------------------------------------------------
// Controls
// ---------------------------------------------------------------------------

void Windows10Style::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter,
                                 const QWidget* widget) const {
  const Colors& colors = colorsFor(option);
  switch (element) {
    case CE_PushButtonBevel:
      if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
        proxy()->drawPrimitive(PE_PanelButtonCommand, button, painter, widget);
        if (button->features & QStyleOptionButton::HasMenu) {
          const int indicatorWidth = proxy()->pixelMetric(PM_MenuButtonIndicator, button, widget);
          const QRect arrowRect(button->rect.right() - indicatorWidth - 2, button->rect.top(), indicatorWidth,
                                button->rect.height());
          const QColor color = (button->state & State_Enabled) ? colors.m_text : colors.m_buttonDisabledText;
          drawCaret(painter, visualRect(button->direction, button->rect, arrowRect), Qt::DownArrow, color, 7.0);
        }
        return;
      }
      break;

    case CE_PushButtonLabel:
      if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
        if (!(button->state & State_Enabled)) {
          QStyleOptionButton copy(*button);
          copy.palette.setColor(QPalette::ButtonText,
                                overriddenOr(button, QPalette::ButtonText, basePalette(button), colors.m_buttonDisabledText));
          QProxyStyle::drawControl(element, &copy, painter, widget);
          return;
        }
      }
      break;

    case CE_ComboBoxLabel:
      if (const QStyleOptionComboBox* combo = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
        if (combo->editable) {
          break;
        }
        QStyleOptionComboBox copy(*combo);
        const bool enabled = combo->state & State_Enabled;
        if (!enabled) {
          const QColor text = overriddenOr(combo, QPalette::ButtonText, basePalette(combo), colors.m_buttonDisabledText);
          copy.palette.setColor(QPalette::ButtonText, text);
          copy.palette.setColor(QPalette::Text, text);
        } else if ((combo->state & State_HasFocus) && !(combo->state & State_On)) {
          // A focused drop-down list shows its text selected, as Win32 combo boxes do
          const QRect field = proxy()->subControlRect(CC_ComboBox, combo, SC_ComboBoxEditField, widget);
          painter->fillRect(field, combo->palette.color(QPalette::Active, QPalette::Highlight));
          const QColor selectedText = combo->palette.color(QPalette::Active, QPalette::HighlightedText);
          copy.palette.setColor(QPalette::ButtonText, selectedText);
          copy.palette.setColor(QPalette::Text, selectedText);
        }
        QProxyStyle::drawControl(element, &copy, painter, widget);
        return;
      }
      break;

    case CE_TabBarTabShape:
      drawTabShape(option, painter);
      return;

    case CE_HeaderSection:
      drawHeaderSection(option, painter);
      return;

    case CE_HeaderLabel:
      if (const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader*>(option)) {
        QStyleOptionHeader copy(*header);
        setEnabledColor(copy.palette, QPalette::ButtonText,
                        overriddenOr(header, QPalette::ButtonText, basePalette(header), colors.m_headerText));
        QProxyStyle::drawControl(element, &copy, painter, widget);
        return;
      }
      break;

    case CE_HeaderEmptyArea:
      painter->fillRect(option->rect, paletteColor(option, QPalette::Base));
      return;

    case CE_ItemViewItem:
      if (const QStyleOptionViewItem* item = qstyleoption_cast<const QStyleOptionViewItem*>(option)) {
        if ((item->state & State_Selected) && !isInComboPopup(widget)) {
          // Explorer selection is pale, so selected text keeps its normal color
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
    case CE_ToolBar:
    case CE_Splitter:
      return;

    case CE_MenuBarEmptyArea:
      painter->fillRect(option->rect, overriddenOr(option, QPalette::Window, basePalette(option), colors.m_menuBarFill));
      return;

    case CE_MenuBarItem:
      if (const QStyleOptionMenuItem* item = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
        const bool enabled = item->state & State_Enabled;
        painter->fillRect(item->rect, overriddenOr(item, QPalette::Window, basePalette(item), colors.m_menuBarFill));
        if (enabled && (item->state & State_Sunken)) {
          drawBox(painter, item->rect, colors.m_itemSelected, colors.m_itemSelectedBorder);
        } else if (enabled && (item->state & State_Selected)) {
          drawBox(painter, item->rect, colors.m_itemHover, colors.m_itemSelected);
        }
        int flags = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
        if (!proxy()->styleHint(SH_UnderlineShortcut, item, widget)) {
          flags |= Qt::TextHideMnemonic;
        }
        painter->save();
        painter->setPen(overriddenOr(item, QPalette::ButtonText, basePalette(item),
                                     enabled ? colors.m_text : colors.m_textDisabled));
        painter->drawText(item->rect, flags, item->text);
        painter->restore();
        return;
      }
      break;

    case CE_ShapedFrame:
      if (const QStyleOptionFrame* frame = qstyleoption_cast<const QStyleOptionFrame*>(option)) {
        // Etched lines, as SS_ETCHEDHORZ / SS_ETCHEDVERT draw them
        const QRect r = frame->rect;
        if (frame->frameShape == QFrame::HLine) {
          const int y = r.center().y();
          painter->fillRect(QRect(r.left(), y, r.width(), 1), colors.m_etchShadow);
          painter->fillRect(QRect(r.left(), y + 1, r.width(), 1), colors.m_etchHighlight);
          return;
        }
        if (frame->frameShape == QFrame::VLine) {
          const int x = r.center().x();
          painter->fillRect(QRect(x, r.top(), 1, r.height()), colors.m_etchShadow);
          painter->fillRect(QRect(x + 1, r.top(), 1, r.height()), colors.m_etchHighlight);
          return;
        }
      }
      break;

    case CE_RubberBand:
      if (const QStyleOptionRubberBand* band = qstyleoption_cast<const QStyleOptionRubberBand*>(option)) {
        if (band->shape == QRubberBand::Rectangle) {
          const QColor explorer = brightened(colors.m_accent);
          drawBox(painter, band->rect, withAlpha(explorer, 0.25), explorer);
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
      drawBox(painter, option->rect, colors.m_progressFill, colors.m_progressBorder);
      return;

    case CE_ProgressBarContents:
      drawProgressContents(option, painter, widget);
      return;

    case CE_ProgressBarLabel:
      if (const QStyleOptionProgressBar* bar = qstyleoption_cast<const QStyleOptionProgressBar*>(option)) {
        const Qt::Alignment alignment = visualAlignment(bar->direction, Qt::AlignRight | Qt::AlignVCenter);
        proxy()->drawItemText(painter, bar->rect, int(alignment), bar->palette, bar->state & State_Enabled, bar->text,
                              QPalette::WindowText);
        return;
      }
      break;

    default:
      break;
  }
  QProxyStyle::drawControl(element, option, painter, widget);
}

void Windows10Style::drawMenuItem(const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionMenuItem* item = qstyleoption_cast<const QStyleOptionMenuItem*>(option);
  const Colors& colors = colorsFor(item);
  const QRect r = item->rect;
  const bool enabled = item->state & State_Enabled;
  const auto visual = [&](const QRect& rect) { return visualRect(item->direction, r, rect); };

  if (item->menuItemType == QStyleOptionMenuItem::Separator) {
    // Separators start at the text column, leaving the icon gutter clear
    if (item->text.isEmpty()) {
      const QRect line(r.left() + MENU_TEXT_OFFSET, r.center().y(), r.width() - MENU_TEXT_OFFSET - 2, 1);
      painter->fillRect(visual(line), colors.m_menuSeparator);
    } else {
      painter->save();
      painter->setFont(item->font);
      painter->setPen(colors.m_textDisabled);
      const QRect textRect(r.left() + MENU_TEXT_OFFSET, r.top(), r.width() - MENU_TEXT_OFFSET, r.height());
      painter->drawText(visual(textRect), int(visualAlignment(item->direction, Qt::AlignLeft | Qt::AlignVCenter)),
                        item->text);
      painter->restore();
    }
    return;
  }

  // Disabled items still highlight, in grey
  if (item->state & State_Selected) {
    painter->fillRect(r, enabled ? colors.m_menuHot : colors.m_menuHotDisabled);
  }
  const QColor textColor =
      overriddenOr(item, QPalette::Text, basePalette(item), enabled ? colors.m_text : colors.m_textDisabled);
  const QRect gutter(r.left(), r.top(), MENU_GUTTER_WIDTH, r.height());

  if (!item->icon.isNull()) {
    const int iconSize = proxy()->pixelMetric(PM_SmallIconSize, item, widget);
    if (item->checked) {
      // Checked items with an icon frame the icon instead of drawing a check
      QRect frame(0, 0, iconSize + 6, iconSize + 6);
      frame.moveCenter(gutter.center());
      drawBox(painter, visual(frame), colors.m_itemSelected, colors.m_itemSelectedBorder);
    }
    const QIcon::Mode mode = enabled ? QIcon::Normal : QIcon::Disabled;
    const QPixmap pixmap = item->icon.pixmap(iconSize, mode, item->checked ? QIcon::On : QIcon::Off);
    proxy()->drawItemPixmap(painter, visual(gutter), Qt::AlignCenter, pixmap);
  } else if (item->checkType != QStyleOptionMenuItem::NotCheckable && item->checked) {
    QRectF mark(0, 0, 16, 16);
    mark.moveCenter(QRectF(visual(gutter)).center());
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    if (item->checkType == QStyleOptionMenuItem::Exclusive) {
      painter->setPen(Qt::NoPen);
      painter->setBrush(textColor);
      painter->drawEllipse(mark.adjusted(5, 5, -5, -5));
    } else {
      const QPointF points[3] = {
        QPointF(mark.left() + 4.0, mark.top() + 8.0),
        QPointF(mark.left() + 6.5, mark.top() + 10.5),
        QPointF(mark.left() + 12.0, mark.top() + 5.0),
      };
      painter->setPen(QPen(textColor, 1.2));
      painter->drawPolyline(points, 3);
    }
    painter->restore();
  }

  QString text = item->text;
  QString shortcut;
  const int tab = text.indexOf(QLatin1Char('\t'));
  if (tab >= 0) {
    shortcut = text.mid(tab + 1);
    text = text.left(tab);
  }
  const QRect textRect(r.left() + MENU_TEXT_OFFSET, r.top(), r.width() - MENU_TEXT_OFFSET - MENU_ARROW_COLUMN,
                       r.height());
  int flags = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
  if (!proxy()->styleHint(SH_UnderlineShortcut, item, widget)) {
    flags |= Qt::TextHideMnemonic;
  }
  painter->save();
  painter->setFont(item->font);
  painter->setPen(textColor);
  painter->drawText(visual(textRect), flags | int(visualAlignment(item->direction, Qt::AlignLeft)), text);
  if (!shortcut.isEmpty()) {
    painter->drawText(visual(textRect), flags | int(visualAlignment(item->direction, Qt::AlignRight)), shortcut);
  }
  painter->restore();

  if (item->menuItemType == QStyleOptionMenuItem::SubMenu) {
    const QRect arrowRect(r.right() - MENU_ARROW_COLUMN + 1, r.top(), MENU_ARROW_COLUMN - 8, r.height());
    const Qt::ArrowType arrow = item->direction == Qt::RightToLeft ? Qt::LeftArrow : Qt::RightArrow;
    drawCaret(painter, visual(arrowRect), arrow, textColor, 7.0);
  }
}

void Windows10Style::drawTabShape(const QStyleOption* option, QPainter* painter) const {
  const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option);
  if (!tab) {
    return;
  }
  const Colors& colors = colorsFor(tab);
  const bool selected = tab->state & State_Selected;
  const bool hovered = (tab->state & State_Enabled) && (tab->state & State_MouseOver);

  // Draw in north-facing local coordinates; the transform handles the other shapes
  QSizeF localSize;
  const QTransform transform = tabTransform(tab, &localSize);
  const int width = int(localSize.width());
  const int height = int(localSize.height());
  painter->save();
  painter->setTransform(transform, true);

  if (selected) {
    // The selected tab is raised and 2px wider on each side, covering the pane border below it
    const QColor fill = tab->documentMode ? tab->palette.color(QPalette::Window) : colors.m_tabSelectedFill;
    painter->fillRect(QRect(-2, 0, width + 4, height), fill);
    painter->fillRect(QRect(-2, 0, width + 4, 1), colors.m_tabBorder);
    painter->fillRect(QRect(-2, 0, 1, height), colors.m_tabBorder);
    painter->fillRect(QRect(width + 1, 0, 1, height), colors.m_tabBorder);
  } else {
    // Unselected tabs stop above the pane border, so it shows through underneath
    const QRect body(0, TAB_RAISE, width, height - TAB_RAISE - 1);
    const QColor border = hovered ? colors.m_tabHoverBorder : colors.m_tabBorder;
    painter->fillRect(body, hovered ? colors.m_tabHoverFill : colors.m_tabFill);
    const bool mirrored = !isVerticalTab(tab->shape) && tab->direction == Qt::RightToLeft;
    const bool isFirst = tab->position == QStyleOptionTab::Beginning || tab->position == QStyleOptionTab::OnlyOneTab;
    const int trailing = mirrored ? body.left() : body.right();
    const int leading = mirrored ? body.right() : body.left();
    painter->fillRect(QRect(body.left(), body.top(), body.width(), 1), border);
    painter->fillRect(QRect(trailing, body.top(), 1, body.height()), border);
    if (isFirst || hovered) {
      painter->fillRect(QRect(leading, body.top(), 1, body.height()), border);
    }
  }
  painter->restore();
}

void Windows10Style::drawHeaderSection(const QStyleOption* option, QPainter* painter) const {
  const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader*>(option);
  if (!header) {
    return;
  }
  const Colors& colors = colorsFor(header);
  const QRect r = header->rect;
  const bool enabled = header->state & State_Enabled;
  if (enabled && (header->state & State_Sunken)) {
    drawBox(painter, r, colors.m_headerPressed, colors.m_headerPressedBorder);
    return;
  }
  if (enabled && (header->state & State_MouseOver)) {
    drawBox(painter, r, colors.m_headerHover, colors.m_headerHoverBorder);
    return;
  }
  painter->fillRect(r, paletteColor(header, QPalette::Base));
  // Explorer headers have full-height column separators and no bottom line
  const int edge = header->direction == Qt::RightToLeft ? r.left() : r.right();
  if (header->orientation == Qt::Horizontal) {
    painter->fillRect(QRect(edge, r.top(), 1, r.height()), colors.m_headerSeparator);
  } else {
    painter->fillRect(QRect(r.left(), r.bottom(), r.width(), 1), colors.m_headerSeparator);
    painter->fillRect(QRect(edge, r.top(), 1, r.height()), colors.m_headerSeparator);
  }
}

void Windows10Style::drawProgressContents(const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionProgressBar* bar = qstyleoption_cast<const QStyleOptionProgressBar*>(option);
  if (!bar) {
    return;
  }
  const Colors& colors = colorsFor(bar);
  const bool horizontal = isHorizontal(bar);
  const QRect inner = bar->rect.adjusted(1, 1, -1, -1);
  const int length = horizontal ? inner.width() : inner.height();
  if (length <= 0) {
    return;
  }

  bool reverse = horizontal ? bar->direction == Qt::RightToLeft : true;
  if (bar->invertedAppearance) {
    reverse = !reverse;
  }

  int start = 0;
  int extent = 0;
  const qint64 range = qint64(bar->maximum) - bar->minimum;
  if (range <= 0) {
    // Marquee: a block that slides across and wraps around
    const qreal phase = (QTime::currentTime().msecsSinceStartOfDay() % MARQUEE_CYCLE_MS) / qreal(MARQUEE_CYCLE_MS);
    extent = qMax(1, length / 4);
    start = int(-extent + phase * (length + extent));
    if (qobject_cast<const QProgressBar*>(widget)) {
      scheduleAnimationFrame(widget);
    }
  } else {
    const qreal fraction = qBound<qreal>(0.0, qreal(qint64(bar->progress) - bar->minimum) / range, 1.0);
    extent = qRound(length * fraction);
  }
  if (extent <= 0) {
    return;
  }
  if (reverse) {
    start = length - start - extent;
  }

  QRect chunk = horizontal ? QRect(inner.left() + start, inner.top(), extent, inner.height())
                           : QRect(inner.left(), inner.top() + start, inner.width(), extent);
  chunk = chunk.intersected(inner);
  const QColor color = (bar->state & State_Enabled) ? colors.m_progressChunk : colors.m_progressBorder;
  painter->fillRect(chunk, color);
}

// ---------------------------------------------------------------------------
// Complex controls
// ---------------------------------------------------------------------------

void Windows10Style::drawComplexControl(ComplexControl control, const QStyleOptionComplex* option,
                                        QPainter* painter, const QWidget* widget) const {
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
    case CC_GroupBox:
      if (qstyleoption_cast<const QStyleOptionGroupBox*>(option)) {
        drawGroupBox(option, painter, widget);
        return;
      }
      break;
    default:
      break;
  }
  QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void Windows10Style::drawSpinBox(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionSpinBox* spin = qstyleoption_cast<const QStyleOptionSpinBox*>(option);
  const Colors& colors = colorsFor(spin);
  if (spin->frame) {
    drawEditFrame(spin, painter, true);
  } else {
    painter->fillRect(spin->rect, paletteColor(spin, QPalette::Base));
  }
  if (spin->buttonSymbols == QAbstractSpinBox::NoButtons) {
    return;
  }

  // Up-down buttons: flat until hovered, then they take the push button states
  const bool enabled = spin->state & State_Enabled;
  for (SubControl subControl : {SC_SpinBoxUp, SC_SpinBoxDown}) {
    const bool isUp = subControl == SC_SpinBoxUp;
    const QRect r = proxy()->subControlRect(CC_SpinBox, spin, subControl, widget);
    const bool stepEnabled =
        enabled && (spin->stepEnabled & (isUp ? QAbstractSpinBox::StepUpEnabled : QAbstractSpinBox::StepDownEnabled));
    const bool active = stepEnabled && (spin->activeSubControls & subControl);
    if (active && (spin->state & State_Sunken)) {
      drawBox(painter, r, colors.m_buttonPressedFill, colors.m_buttonPressedBorder);
    } else if (active && (spin->state & State_MouseOver)) {
      drawBox(painter, r, colors.m_buttonHoverFill, colors.m_buttonHoverBorder);
    }
    const QColor color = stepEnabled ? colors.m_glyph : colors.m_glyphDisabled;
    if (spin->buttonSymbols == QAbstractSpinBox::PlusMinus) {
      const QPoint center = r.center();
      painter->fillRect(QRect(center.x() - 3, center.y(), 7, 1), color);
      if (isUp) {
        painter->fillRect(QRect(center.x(), center.y() - 3, 1, 7), color);
      }
    } else {
      drawCaret(painter, r, isUp ? Qt::UpArrow : Qt::DownArrow, color, 6.0);
    }
  }
}

void Windows10Style::drawComboBox(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionComboBox* combo = qstyleoption_cast<const QStyleOptionComboBox*>(option);
  const Colors& colors = colorsFor(combo);
  const bool enabled = combo->state & State_Enabled;
  const bool open = combo->state & (State_On | State_Sunken);
  const QRect arrow = proxy()->subControlRect(CC_ComboBox, combo, SC_ComboBoxArrow, widget);

  if (combo->editable) {
    // CBS_DROPDOWN: an edit field with a drop button that lights up on its own
    if (combo->frame) {
      drawEditFrame(combo, painter, true);
    } else {
      painter->fillRect(combo->rect, paletteColor(combo, QPalette::Base));
    }
    const bool arrowHovered = enabled && (combo->activeSubControls & SC_ComboBoxArrow) &&
                              (combo->state & State_MouseOver);
    if (enabled && open) {
      drawBox(painter, arrow, colors.m_buttonPressedFill, colors.m_buttonPressedBorder);
    } else if (arrowHovered) {
      drawBox(painter, arrow, colors.m_buttonHoverFill, colors.m_buttonHoverBorder);
    }
  } else if (combo->frame) {
    // CBS_DROPDOWNLIST looks like a push button
    QStyleOptionComboBox panel(*combo);
    panel.state &= ~(State_On | State_Sunken);
    if (open) {
      panel.state |= State_Sunken;
    }
    drawPushButton(&panel, painter, false, false);
  }

  if (combo->subControls & SC_ComboBoxArrow) {
    drawChevron(painter, arrow, Qt::DownArrow, enabled ? colors.m_glyph : colors.m_glyphDisabled, 7.0, 1.0);
  }
}

void Windows10Style::drawScrollBar(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionSlider* bar = qstyleoption_cast<const QStyleOptionSlider*>(option);
  const Colors& colors = colorsFor(bar);
  const bool horizontal = bar->orientation == Qt::Horizontal;
  const bool enabled = bar->state & State_Enabled;
  painter->fillRect(bar->rect, colors.m_scrollTrack);

  for (SubControl subControl : {SC_ScrollBarSubLine, SC_ScrollBarAddLine}) {
    const QRect button = proxy()->subControlRect(CC_ScrollBar, bar, subControl, widget);
    if (button.isEmpty()) {
      continue;
    }
    const bool active = enabled && (bar->activeSubControls & subControl);
    QColor glyph = enabled ? colors.m_scrollArrow : colors.m_scrollArrowDisabled;
    if (active && (bar->state & State_Sunken)) {
      painter->fillRect(button, colors.m_scrollArrowPressedFill);
      glyph = colors.m_scrollArrowPressed;
    } else if (active && (bar->state & State_MouseOver)) {
      painter->fillRect(button, colors.m_scrollArrowHoverFill);
      glyph = colors.m_scrollArrowHover;
    }
    // Point away from the center so mirrored layouts stay correct
    Qt::ArrowType arrow;
    if (horizontal) {
      arrow = button.center().x() < bar->rect.center().x() ? Qt::LeftArrow : Qt::RightArrow;
    } else {
      arrow = button.center().y() < bar->rect.center().y() ? Qt::UpArrow : Qt::DownArrow;
    }
    drawCaret(painter, button, arrow, glyph, 7.0);
  }

  if (!enabled || bar->maximum == bar->minimum || !(bar->subControls & SC_ScrollBarSlider)) {
    return;
  }
  const QRect slider = proxy()->subControlRect(CC_ScrollBar, bar, SC_ScrollBarSlider, widget);
  const bool sliderActive = bar->activeSubControls & SC_ScrollBarSlider;
  QColor thumb = colors.m_scrollThumb;
  if (sliderActive && (bar->state & State_Sunken)) {
    thumb = colors.m_scrollThumbPressed;
  } else if (sliderActive && (bar->state & State_MouseOver)) {
    thumb = colors.m_scrollThumbHover;
  }
  painter->fillRect(slider, thumb);
}

void Windows10Style::drawSlider(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionSlider* slider = qstyleoption_cast<const QStyleOptionSlider*>(option);
  const Colors& colors = colorsFor(slider);
  const bool horizontal = slider->orientation == Qt::Horizontal;
  const bool enabled = slider->state & State_Enabled;
  const QRect r = slider->rect;
  const QRect groove = proxy()->subControlRect(CC_Slider, slider, SC_SliderGroove, widget);
  const QRect handle = proxy()->subControlRect(CC_Slider, slider, SC_SliderHandle, widget);

  if (slider->subControls & SC_SliderGroove) {
    // The channel runs between the thumb's extreme centers
    const int inset = SLIDER_THUMB_LENGTH / 2;
    const QRect channel = horizontal ? groove.adjusted(inset, 0, -inset, 0) : groove.adjusted(0, inset, 0, -inset);
    drawBox(painter, channel, colors.m_trackFill, colors.m_trackBorder);
  }

  if ((slider->subControls & SC_SliderTickmarks) && slider->tickPosition != QSlider::NoTicks) {
    const int available = (horizontal ? r.width() : r.height()) - SLIDER_THUMB_LENGTH;
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
    for (qint64 value = slider->minimum; value <= slider->maximum; value += interval) {
      const int position = sliderPositionFromValue(slider->minimum, slider->maximum, int(value), available,
                                                   slider->upsideDown) + SLIDER_THUMB_LENGTH / 2;
      if (horizontal) {
        const int x = r.left() + position;
        if (slider->tickPosition & QSlider::TicksAbove) {
          painter->fillRect(QRect(x, r.top(), 1, 4), colors.m_tick);
        }
        if (slider->tickPosition & QSlider::TicksBelow) {
          painter->fillRect(QRect(x, r.bottom() - 3, 1, 4), colors.m_tick);
        }
      } else {
        const int y = r.top() + position;
        if (slider->tickPosition & QSlider::TicksLeft) {
          painter->fillRect(QRect(r.left(), y, 4, 1), colors.m_tick);
        }
        if (slider->tickPosition & QSlider::TicksRight) {
          painter->fillRect(QRect(r.right() - 3, y, 4, 1), colors.m_tick);
        }
      }
    }
  }

  if (slider->subControls & SC_SliderHandle) {
    const bool handleActive = slider->activeSubControls & SC_SliderHandle;
    QColor color = colors.m_accent;
    if (!enabled) {
      color = colors.m_sliderThumbDisabled;
    } else if (handleActive && (slider->state & State_Sunken)) {
      color = colors.m_sliderThumbPressed;
    } else if (handleActive && (slider->state & State_MouseOver)) {
      color = colors.m_sliderThumbHover;
    }

    // A flat bar, pointed toward the ticks when they're only on one side
    const QRectF h(handle);
    const bool pointAfter = slider->tickPosition == QSlider::TicksBelow;
    const bool pointBefore = slider->tickPosition == QSlider::TicksAbove;
    QPolygonF shape;
    if (horizontal) {
      const qreal top = pointBefore ? h.top() + SLIDER_THUMB_POINT : h.top();
      const qreal bottom = pointAfter ? h.bottom() - SLIDER_THUMB_POINT : h.bottom();
      shape << QPointF(h.left(), top);
      if (pointBefore) {
        shape << QPointF(h.center().x(), h.top());
      }
      shape << QPointF(h.right(), top) << QPointF(h.right(), bottom);
      if (pointAfter) {
        shape << QPointF(h.center().x(), h.bottom());
      }
      shape << QPointF(h.left(), bottom);
    } else {
      const qreal left = pointBefore ? h.left() + SLIDER_THUMB_POINT : h.left();
      const qreal right = pointAfter ? h.right() - SLIDER_THUMB_POINT : h.right();
      shape << QPointF(left, h.top()) << QPointF(right, h.top());
      if (pointAfter) {
        shape << QPointF(h.right(), h.center().y());
      }
      shape << QPointF(right, h.bottom()) << QPointF(left, h.bottom());
      if (pointBefore) {
        shape << QPointF(h.left(), h.center().y());
      }
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, pointAfter || pointBefore);
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawPolygon(shape);
    painter->restore();
  }

  if (enabled && hasKeyboardFocus(slider)) {
    drawDottedRect(painter, r, colors.m_focusRect);
  }
}

void Windows10Style::drawGroupBox(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionGroupBox* group = qstyleoption_cast<const QStyleOptionGroupBox*>(option);
  const QRect textRect = proxy()->subControlRect(CC_GroupBox, group, SC_GroupBoxLabel, widget);
  const QRect checkRect = proxy()->subControlRect(CC_GroupBox, group, SC_GroupBoxCheckBox, widget);
  const bool hasCheckBox = group->subControls & SC_GroupBoxCheckBox;

  // The title sits in a gap cut into the frame's top line
  if (group->subControls & SC_GroupBoxFrame) {
    QStyleOptionFrame frame;
    frame.QStyleOption::operator=(*group);
    frame.features = group->features;
    frame.lineWidth = group->lineWidth;
    frame.midLineWidth = group->midLineWidth;
    frame.rect = proxy()->subControlRect(CC_GroupBox, group, SC_GroupBoxFrame, widget);
    painter->save();
    if (!group->text.isEmpty() || hasCheckBox) {
      QRect gap = textRect;
      if (hasCheckBox) {
        gap = gap.united(checkRect);
      }
      QRegion region(group->rect);
      region -= gap.adjusted(-2, 0, 2, 0);
      painter->setClipRegion(region, Qt::IntersectClip);
    }
    proxy()->drawPrimitive(PE_FrameGroupBox, &frame, painter, widget);
    painter->restore();
  }

  if (!group->text.isEmpty() && (group->subControls & SC_GroupBoxLabel)) {
    int alignment = int(group->textAlignment) | Qt::TextShowMnemonic;
    if (!proxy()->styleHint(SH_UnderlineShortcut, group, widget)) {
      alignment |= Qt::TextHideMnemonic;
    }
    proxy()->drawItemText(painter, textRect, alignment, group->palette, group->state & State_Enabled, group->text,
                          QPalette::WindowText);
    if (group->state & State_HasFocus) {
      QStyleOptionFocusRect focus;
      focus.QStyleOption::operator=(*group);
      focus.rect = textRect.adjusted(-1, 0, 1, 0);
      proxy()->drawPrimitive(PE_FrameFocusRect, &focus, painter, widget);
    }
  }

  if (hasCheckBox) {
    QStyleOptionButton box;
    box.QStyleOption::operator=(*group);
    box.rect = checkRect;
    proxy()->drawPrimitive(PE_IndicatorCheckBox, &box, painter, widget);
  }
}

// ---------------------------------------------------------------------------
// Geometry
// ---------------------------------------------------------------------------

QRect Windows10Style::subControlRect(ComplexControl control, const QStyleOptionComplex* option,
                                     SubControl subControl, const QWidget* widget) const {
  switch (control) {
    case CC_SpinBox:
      if (const QStyleOptionSpinBox* spin = qstyleoption_cast<const QStyleOptionSpinBox*>(option)) {
        const QRect r = spin->rect;
        const int border = spin->frame ? FRAME_WIDTH : 0;
        const bool hasButtons = spin->buttonSymbols != QAbstractSpinBox::NoButtons;
        const int buttonWidth = hasButtons ? SYSTEM_BUTTON_WIDTH : 0;
        const int innerHeight = r.height() - 2 * border;
        const int buttonX = r.right() - border - buttonWidth + 1;
        QRect result;
        switch (subControl) {
          case SC_SpinBoxFrame:
            return r;
          case SC_SpinBoxEditField:
            result = QRect(r.left() + border + 1, r.top() + border, r.width() - 2 * border - 1 - buttonWidth,
                           innerHeight);
            break;
          case SC_SpinBoxUp:
            if (!hasButtons) {
              return QRect();
            }
            result = QRect(buttonX, r.top() + border, buttonWidth, innerHeight / 2);
            break;
          case SC_SpinBoxDown:
            if (!hasButtons) {
              return QRect();
            }
            result = QRect(buttonX, r.top() + border + innerHeight / 2, buttonWidth, innerHeight - innerHeight / 2);
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
        const int border = combo->frame ? FRAME_WIDTH : 0;
        QRect result;
        switch (subControl) {
          case SC_ComboBoxFrame:
            return r;
          case SC_ComboBoxArrow:
            result = QRect(r.right() - border - SYSTEM_BUTTON_WIDTH + 1, r.top() + border, SYSTEM_BUTTON_WIDTH,
                           r.height() - 2 * border);
            break;
          case SC_ComboBoxEditField:
            if (combo->editable) {
              result = QRect(r.left() + border + 1, r.top() + border, r.width() - 2 * border - 1 - SYSTEM_BUTTON_WIDTH,
                             r.height() - 2 * border);
            } else {
              // Inset so the focus highlight leaves a margin, as in Win32
              result = QRect(r.left() + border + 2, r.top() + border + 2,
                             r.width() - 2 * border - 2 - SYSTEM_BUTTON_WIDTH, r.height() - 2 * border - 4);
            }
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
        // Arrow buttons shrink when the bar is too short for them, as Win32 does
        const int button = qMin(thickness, length / 2);
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
            const int available = (horizontal ? r.width() : r.height()) - SLIDER_THUMB_LENGTH;
            const int position = sliderPositionFromValue(slider->minimum, slider->maximum, slider->sliderPosition,
                                                         available, slider->upsideDown);
            return horizontal
                       ? QRect(r.left() + position, r.top() + (r.height() - SLIDER_THUMB_THICKNESS) / 2,
                               SLIDER_THUMB_LENGTH, SLIDER_THUMB_THICKNESS)
                       : QRect(r.left() + (r.width() - SLIDER_THUMB_THICKNESS) / 2, r.top() + position,
                               SLIDER_THUMB_THICKNESS, SLIDER_THUMB_LENGTH);
          }
          case SC_SliderTickmarks:
            return r;
          default:
            break;
        }
      }
      break;

    case CC_GroupBox:
      if (const QStyleOptionGroupBox* group = qstyleoption_cast<const QStyleOptionGroupBox*>(option)) {
        // Title on the frame line, 8px in, the way QCommonStyle and Win32 lay it out
        const QFontMetrics& metrics = group->fontMetrics;
        const bool hasTitle = !group->text.isEmpty() || (group->subControls & SC_GroupBoxCheckBox);
        const int titleHeight = hasTitle ? metrics.height() : 0;
        switch (subControl) {
          case SC_GroupBoxFrame:
          case SC_GroupBoxContents: {
            QRect frame = group->rect;
            frame.setTop(group->rect.top() + titleHeight / 2);
            if (subControl == SC_GroupBoxFrame) {
              return frame;
            }
            const int frameWidth = (group->features & QStyleOptionFrame::Flat) ? 0 : FRAME_WIDTH;
            return frame.adjusted(frameWidth, frameWidth + titleHeight - titleHeight / 2, -frameWidth, -frameWidth);
          }
          case SC_GroupBoxCheckBox:
          case SC_GroupBoxLabel: {
            const int margin = (group->features & QStyleOptionFrame::Flat) ? 0 : 8;
            const int textWidth = metrics.size(Qt::TextShowMnemonic, group->text + QLatin1Char(' ')).width();
            const int indicatorWidth = proxy()->pixelMetric(PM_IndicatorWidth, group, widget);
            const int indicatorSpace = proxy()->pixelMetric(PM_CheckBoxLabelSpacing, group, widget) - 1;
            const bool hasCheckBox = group->subControls & SC_GroupBoxCheckBox;
            const int checkWidth = hasCheckBox ? indicatorWidth + indicatorSpace : 0;
            QRect bounds = group->rect.adjusted(margin, 0, -margin, 0);
            bounds.setHeight(metrics.height());
            const QRect total =
                alignedRect(group->direction, group->textAlignment, QSize(textWidth + checkWidth, metrics.height()),
                            bounds);
            const bool leftToRight = group->direction == Qt::LeftToRight;
            if (subControl == SC_GroupBoxCheckBox) {
              if (!hasCheckBox) {
                return QRect();
              }
              const int indicatorHeight = proxy()->pixelMetric(PM_IndicatorHeight, group, widget);
              const int left = leftToRight ? total.left() : total.right() - indicatorWidth + 1;
              const int top = total.top() + qMax(0, metrics.height() - indicatorHeight) / 2;
              return QRect(left, top, indicatorWidth, indicatorHeight);
            }
            const int left = leftToRight ? total.left() + checkWidth : total.left();
            return QRect(left, total.top(), total.width() - checkWidth, total.height());
          }
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

QRect Windows10Style::subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget) const {
  switch (element) {
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
            qMax(metrics.horizontalAdvance(QStringLiteral("100%")), metrics.horizontalAdvance(bar->text)) + 6;
        const QRect result = element == SE_ProgressBarLabel
                                 ? QRect(r.right() - labelWidth + 1, r.top(), labelWidth, r.height())
                                 : QRect(r.left(), r.top(), r.width() - labelWidth, r.height());
        return visualRect(bar->direction, r, result);
      }
      break;

    case SE_TabWidgetTabBar:
      if (const QStyleOptionTabWidgetFrame* frame = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>(option)) {
        // Leave room for the selected tab, which is 2px wider than its slot
        QRect r = QProxyStyle::subElementRect(element, option, widget);
        const QRect bounds = frame->rect;
        if (isVerticalTab(frame->shape)) {
          r.translate(0, TAB_RAISE);
          r.setBottom(qMin(r.bottom(), bounds.bottom() - TAB_RAISE));
        } else if (frame->direction == Qt::RightToLeft) {
          r.translate(-TAB_RAISE, 0);
          r.setLeft(qMax(r.left(), bounds.left() + TAB_RAISE));
        } else {
          r.translate(TAB_RAISE, 0);
          r.setRight(qMin(r.right(), bounds.right() - TAB_RAISE));
        }
        return r;
      }
      break;

    case SE_HeaderArrow:
      if (const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader*>(option)) {
        // Explorer puts the sort chevron above the title, centered
        const QRect r = header->rect;
        if (header->orientation == Qt::Horizontal) {
          return QRect(r.center().x() - 4, r.top(), 9, 6);
        }
        return QRect(r.left(), r.center().y() - 4, 6, 9);
      }
      break;

    case SE_HeaderLabel:
      if (const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader*>(option)) {
        // The sort chevron sits above the text, so the label keeps its full width
        const int margin = proxy()->pixelMetric(PM_HeaderMargin, header, widget);
        return header->rect.adjusted(margin, 0, -margin, 0);
      }
      break;

    default:
      break;
  }
  return QProxyStyle::subElementRect(element, option, widget);
}

QSize Windows10Style::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size,
                                       const QWidget* widget) const {
  switch (type) {
    case CT_PushButton:
      if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
        // 75x23 is the standard dialog button (50x14 dialog units)
        QSize result(size.width() + 16, qMax(size.height() + 8, CONTROL_HEIGHT));
        if (!button->text.isEmpty()) {
          result.setWidth(qMax(result.width(), BUTTON_MIN_WIDTH));
        }
        return result;
      }
      break;

    case CT_LineEdit:
      if (const QStyleOptionFrame* frame = qstyleoption_cast<const QStyleOptionFrame*>(option)) {
        if (frame->lineWidth > 0) {
          const QSize base = QProxyStyle::sizeFromContents(type, option, size, widget);
          return QSize(base.width(), qMax(base.height(), CONTROL_HEIGHT));
        }
      }
      break;

    case CT_SpinBox:
      if (const QStyleOptionSpinBox* spin = qstyleoption_cast<const QStyleOptionSpinBox*>(option)) {
        const int border = spin->frame ? FRAME_WIDTH : 0;
        const int buttons = spin->buttonSymbols != QAbstractSpinBox::NoButtons ? SYSTEM_BUTTON_WIDTH : 2;
        return QSize(size.width() + 2 * border + 1 + buttons, qMax(size.height() + 2 * border, CONTROL_HEIGHT));
      }
      break;

    case CT_ComboBox:
      if (const QStyleOptionComboBox* combo = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
        const int border = combo->frame ? FRAME_WIDTH : 0;
        return QSize(size.width() + 2 * border + 6 + SYSTEM_BUTTON_WIDTH,
                     qMax(size.height() + 2 * border + 4, CONTROL_HEIGHT));
      }
      break;

    case CT_MenuItem:
      if (const QStyleOptionMenuItem* item = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
        const QFontMetrics metrics(item->font);
        if (item->menuItemType == QStyleOptionMenuItem::Separator) {
          return QSize(size.width(), item->text.isEmpty() ? 7 : metrics.height() + 6);
        }
        int width = MENU_TEXT_OFFSET + size.width() + MENU_ARROW_COLUMN;
        if (item->text.contains(QLatin1Char('\t'))) {
          // QMenu adds the shortcut column width itself
          width += MENU_SHORTCUT_GAP;
        }
        return QSize(width, qMax(MENU_ITEM_HEIGHT, metrics.height() + 7));
      }
      break;

    case CT_MenuBarItem:
      return QProxyStyle::sizeFromContents(type, option, size, widget) + QSize(4, 2);

    case CT_ItemViewItem:
      return QProxyStyle::sizeFromContents(type, option, size, widget) + QSize(0, 4);

    case CT_HeaderSection:
      if (const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader*>(option)) {
        const QSize base = QProxyStyle::sizeFromContents(type, option, size, widget);
        return header->orientation == Qt::Horizontal ? base + QSize(0, 4) : base;
      }
      break;

    default:
      break;
  }
  return QProxyStyle::sizeFromContents(type, option, size, widget);
}

int Windows10Style::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const {
  switch (metric) {
    case PM_DefaultFrameWidth:
    case PM_SpinBoxFrameWidth:
    case PM_ComboBoxFrameWidth:
    case PM_MenuPanelWidth:
      return FRAME_WIDTH;
    case PM_ButtonDefaultIndicator:
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
    case PM_TabBarTabShiftHorizontal:
    case PM_MenuBarPanelWidth:
    case PM_MenuBarHMargin:
    case PM_MenuBarVMargin:
    case PM_MenuBarItemSpacing:
      return 0;
    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
      return INDICATOR_SIZE;
    case PM_CheckBoxLabelSpacing:
    case PM_RadioButtonLabelSpacing:
    case PM_ToolTipLabelFrameWidth:
    case PM_FocusFrameHMargin:
      return 4;
    case PM_ScrollBarExtent:
    case PM_ScrollBarSliderMin:
      return SYSTEM_BUTTON_WIDTH;
    case PM_SliderThickness:
      return SLIDER_THUMB_THICKNESS + 3;
    case PM_SliderLength:
      return SLIDER_THUMB_LENGTH;
    case PM_SliderControlThickness:
      return SLIDER_THUMB_THICKNESS;
    case PM_SliderTickmarkOffset:
      return 4;
    case PM_MenuHMargin:
    case PM_MenuVMargin:
    case PM_TabBarTabShiftVertical:
      return 2;
    case PM_SmallIconSize:
      return 16;
    case PM_TabBarTabHSpace:
      return 12;
    case PM_TabBarTabVSpace:
      return 8;
    case PM_TabBarBaseOverlap:
      // The selected tab covers the pane's top border, merging into it
      return 1;
    case PM_HeaderMargin:
      return 6;
    default:
      break;
  }
  return QProxyStyle::pixelMetric(metric, option, widget);
}

int Windows10Style::styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget,
                              QStyleHintReturn* returnData) const {
  switch (hint) {
    case SH_DialogButtonBox_ButtonsHaveIcons:
    case SH_EtchDisabledText:
    case SH_DitherDisabledText:
    case SH_ItemView_ShowDecorationSelected:
    // Win32 drop-down lists are plain list boxes below the control, not menus
    case SH_ComboBox_Popup:
      return 0;
    case SH_ComboBox_ListMouseTracking:
    case SH_MenuBar_AltKeyNavigation:
    case SH_Menu_AllowActiveAndDisabled:
      return 1;
    case SH_DialogButtonLayout:
      return QDialogButtonBox::WinLayout;
    case SH_LineEdit_PasswordCharacter:
      return 0x25CF;
    case SH_Header_ArrowAlignment:
      return int(Qt::AlignTop | Qt::AlignHCenter);
    case SH_GroupBox_TextLabelVerticalAlignment:
      return int(Qt::AlignVCenter);
    case SH_GroupBox_TextLabelColor:
      if (option) {
        return int(option->palette.color(QPalette::WindowText).rgba());
      }
      break;
    case SH_FormLayoutLabelAlignment:
      return int(Qt::AlignLeft | Qt::AlignVCenter);
    case SH_Table_GridLineColor:
      if (option) {
        return int((isDarkPalette(option->palette) ? rgb(0x333333) : rgb(0xF0F0F0)).rgba());
      }
      break;
    default:
      break;
  }
  return QProxyStyle::styleHint(hint, option, widget, returnData);
}

QPixmap Windows10Style::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap& pixmap,
                                            const QStyleOption* option) const {
  // Explorer selection is pale; the default accent wash would muddy icons
  if (iconMode == QIcon::Selected) {
    return pixmap;
  }
  return QProxyStyle::generatedIconPixmap(iconMode, pixmap, option);
}

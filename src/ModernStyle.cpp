#include "ModernStyle.h"

#include "StyleHelpers.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractScrollArea>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QFrame>
#include <QGroupBox>
#include <QLineEdit>
#include <QListView>
#include <QPainter>
#include <QProgressBar>
#include <QSlider>
#include <QSplitterHandle>
#include <QStyleFactory>
#include <QStyleHints>
#include <QStyleOption>
#include <QTabBar>
#include <QTime>
#include <QTimer>
#include <QToolTip>

using namespace StyleHelpers;

namespace {

constexpr int CONTROL_HEIGHT = 30;
constexpr int FRAME_WIDTH = 2;
constexpr int FIELD_PADDING = 8;
constexpr int SPIN_BUTTON_WIDTH = 20;
constexpr int COMBO_ARROW_WIDTH = 26;
// Small enough that square viewports inside a 2px frame stay within the border
constexpr int VIEW_FRAME_RADIUS = 3;
constexpr int SCROLLBAR_EXTENT = 12;
constexpr int SLIDER_HANDLE_SIZE = 18;
constexpr int SLIDER_GROOVE_THICKNESS = 4;
constexpr int PROGRESS_THICKNESS = 6;
constexpr int INDICATOR_SIZE = 18;
constexpr int ANIMATION_INTERVAL_MS = 16;
constexpr int BUSY_CYCLE_MS = 1600;

QColor defaultAccent() {
  return QColor(0x35, 0x84, 0xE4);
}

QColor outlineColor(const QStyleOption* option) {
  const QColor window = option->palette.color(QPalette::Window);
  return mix(window, option->palette.color(QPalette::WindowText), isDarkColor(window) ? 0.2 : 0.16);
}

QColor strongOutlineColor(const QStyleOption* option) {
  const QColor window = option->palette.color(QPalette::Window);
  return mix(window, option->palette.color(QPalette::WindowText), isDarkColor(window) ? 0.34 : 0.3);
}

QColor hoverOverlay(const QStyleOption* option, qreal strength = 1.0) {
  const qreal alpha = isDarkPalette(option->palette) ? 0.08 : 0.05;
  return withAlpha(option->palette.color(QPalette::WindowText), qMin(1.0, alpha * strength));
}

QColor accentHover(const QColor& accent, bool dark) {
  return dark ? accent.lighter(112) : accent.darker(108);
}

QColor accentPressed(const QColor& accent) {
  return accent.darker(118);
}

QRectF progressTrack(const QRect& rect, bool horizontal) {
  const QRectF bounds(rect);
  if (horizontal) {
    return QRectF(bounds.left(), bounds.center().y() - PROGRESS_THICKNESS / 2.0, bounds.width(), PROGRESS_THICKNESS);
  }
  return QRectF(bounds.center().x() - PROGRESS_THICKNESS / 2.0, bounds.top(), PROGRESS_THICKNESS, bounds.height());
}

bool wantsHover(const QWidget* widget) {
  return qobject_cast<const QAbstractButton*>(widget) || qobject_cast<const QComboBox*>(widget) ||
         qobject_cast<const QAbstractSpinBox*>(widget) || qobject_cast<const QAbstractSlider*>(widget) ||
         qobject_cast<const QTabBar*>(widget) || qobject_cast<const QLineEdit*>(widget) ||
         qobject_cast<const QSplitterHandle*>(widget) || qobject_cast<const QGroupBox*>(widget);
}

} // namespace

ModernStyle::ModernStyle(Theme theme, const QColor& accentColor)
    : QProxyStyle(QStyleFactory::create(QStringLiteral("Fusion"))),
      m_theme(theme),
      m_accentColor(accentColor.isValid() ? accentColor : defaultAccent()),
      m_cornerRadius(6),
      m_systemIsDark(false),
      m_pAnimationTimer(new QTimer(this)) {
  setObjectName(QStringLiteral("Modern"));

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

ModernStyle::Theme ModernStyle::theme() const {
  return m_theme;
}

void ModernStyle::setTheme(Theme theme) {
  if (m_theme == theme) {
    return;
  }
  m_theme = theme;
  applyPalette();
}

QColor ModernStyle::accentColor() const {
  return m_accentColor;
}

void ModernStyle::setAccentColor(const QColor& color) {
  const QColor accent = color.isValid() ? color : defaultAccent();
  if (m_accentColor == accent) {
    return;
  }
  m_accentColor = accent;
  applyPalette(true);
}

int ModernStyle::cornerRadius() const {
  return m_cornerRadius;
}

void ModernStyle::setCornerRadius(int radius) {
  m_cornerRadius = qMax(0, radius);
  if (qApp) {
    const QWidgetList widgets = QApplication::allWidgets();
    for (QWidget* widget : widgets) {
      widget->update();
    }
  }
}

bool ModernStyle::isDark() const {
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

void ModernStyle::applyPalette(bool accentChanged) {
  if (!qApp || QApplication::style() != this) {
    return;
  }
  // Keep roles the application set itself; only colors that came from this style change
  const QPalette next = standardPalette();
  const QPalette palette = mergeCustomRoles(next, m_appliedPalette, QApplication::palette(), accentChanged);
  m_appliedPalette = next;
  QApplication::setPalette(palette);
}

void ModernStyle::scheduleAnimationFrame(const QWidget* widget) const {
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

QPalette ModernStyle::standardPalette() const {
  const bool dark = isDark();
  const QColor accent = m_accentColor;

  QColor window, windowText, base, alternateBase, button, text, placeholder;
  QColor light, midlight, mid, shadow, darkShade, toolTipBase, toolTipText;
  QColor disabledText, disabledBase, disabledButton, disabledHighlight;

  if (dark) {
    window = QColor(0x1E, 0x1E, 0x21);
    windowText = QColor(0xE8, 0xE8, 0xEA);
    base = QColor(0x2A, 0x2A, 0x2E);
    alternateBase = QColor(0x26, 0x26, 0x2A);
    button = QColor(0x32, 0x32, 0x37);
    text = windowText;
    placeholder = QColor(0x8A, 0x8A, 0x90);
    light = QColor(0x48, 0x48, 0x4E);
    midlight = QColor(0x3A, 0x3A, 0x40);
    mid = QColor(0x4A, 0x4A, 0x50);
    darkShade = QColor(0x14, 0x14, 0x16);
    shadow = QColor(0, 0, 0, 0x80);
    toolTipBase = QColor(0x3A, 0x3A, 0x40);
    toolTipText = QColor(0xF0, 0xF0, 0xF2);
    disabledText = QColor(0x6A, 0x6A, 0x70);
    disabledBase = QColor(0x25, 0x25, 0x28);
    disabledButton = QColor(0x2A, 0x2A, 0x2E);
    disabledHighlight = QColor(0x48, 0x48, 0x4E);
  } else {
    window = QColor(0xF3, 0xF3, 0xF5);
    windowText = QColor(0x1C, 0x1C, 0x1E);
    base = QColor(0xFF, 0xFF, 0xFF);
    alternateBase = QColor(0xF6, 0xF6, 0xF8);
    button = QColor(0xFF, 0xFF, 0xFF);
    text = windowText;
    placeholder = QColor(0x8A, 0x8A, 0x8E);
    light = QColor(0xFF, 0xFF, 0xFF);
    midlight = QColor(0xE8, 0xE8, 0xEC);
    mid = QColor(0xC8, 0xC8, 0xCE);
    darkShade = QColor(0x9A, 0x9A, 0xA0);
    shadow = QColor(0, 0, 0, 0x33);
    toolTipBase = QColor(0x2C, 0x2C, 0x2E);
    toolTipText = QColor(0xF5, 0xF5, 0xF7);
    disabledText = QColor(0xA0, 0xA0, 0xA6);
    disabledBase = QColor(0xF7, 0xF7, 0xF8);
    disabledButton = QColor(0xF0, 0xF0, 0xF2);
    disabledHighlight = QColor(0xC8, 0xC8, 0xCE);
  }

  QPalette palette;
  palette.setColor(QPalette::All, QPalette::Window, window);
  palette.setColor(QPalette::All, QPalette::WindowText, windowText);
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
  palette.setColor(QPalette::All, QPalette::HighlightedText, QColor(0xFF, 0xFF, 0xFF));
  palette.setColor(QPalette::All, QPalette::Link, dark ? accent.lighter(130) : accent);
  palette.setColor(QPalette::All, QPalette::LinkVisited, dark ? accent.lighter(110) : accent.darker(130));
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
  palette.setColor(QPalette::Disabled, QPalette::Highlight, disabledHighlight);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
  palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, disabledText);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
  palette.setColor(QPalette::Disabled, QPalette::Accent, disabledHighlight);
#endif
  return palette;
}

void ModernStyle::polish(QPalette& palette) {
  palette = standardPalette();
  m_appliedPalette = palette;
}

void ModernStyle::polish(QApplication* application) {
  QProxyStyle::polish(application);
  // Tooltips keep a palette of their own; follow the application's from now on
  QToolTip::setPalette(QApplication::palette());
  application->installEventFilter(this);
}

void ModernStyle::unpolish(QApplication* application) {
  application->removeEventFilter(this);
  QProxyStyle::unpolish(application);
}

bool ModernStyle::eventFilter(QObject* watched, QEvent* event) {
  if (event->type() == QEvent::ApplicationPaletteChange && watched == qApp) {
    QToolTip::setPalette(QApplication::palette());
  }
  return QProxyStyle::eventFilter(watched, event);
}

void ModernStyle::polish(QWidget* widget) {
  QProxyStyle::polish(widget);
  if (wantsHover(widget)) {
    widget->setAttribute(Qt::WA_Hover, true);
  }
  if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(widget)) {
    view->viewport()->setAttribute(Qt::WA_Hover, true);
  }
}

void ModernStyle::unpolish(QWidget* widget) {
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

void ModernStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter,
                                const QWidget* widget) const {
  switch (element) {
    case PE_FrameFocusRect:
    case PE_FrameDefaultButton:
    case PE_FrameStatusBarItem:
    case PE_PanelMenuBar:
    case PE_PanelScrollAreaCorner:
      // Focus is shown on the controls themselves; the rest are intentionally flat
      return;

    case PE_PanelButtonCommand: {
      const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option);
      const bool isDefault = button && (button->features & QStyleOptionButton::DefaultButton);
      const bool isFlat = button && (button->features & QStyleOptionButton::Flat);
      drawButtonPanel(option, painter, isDefault, isFlat);
      return;
    }

    case PE_PanelButtonTool:
    case PE_PanelButtonBevel:
    case PE_IndicatorButtonDropDown:
      drawToolPanel(option, painter);
      return;

    case PE_PanelLineEdit: {
      const QStyleOptionFrame* frame = qstyleoption_cast<const QStyleOptionFrame*>(option);
      if (frame && frame->lineWidth <= 0) {
        // Frameless edits live inside spin boxes, combos and item editors
        painter->fillRect(option->rect, option->palette.brush(colorGroup(option), QPalette::Base));
        return;
      }
      drawFieldFrame(option, painter, true);
      return;
    }

    case PE_FrameLineEdit:
      drawFieldFrame(option, painter, false);
      return;

    case PE_IndicatorCheckBox:
    case PE_IndicatorItemViewItemCheck:
      drawCheckIndicator(option, painter, false);
      return;

    case PE_IndicatorRadioButton:
      drawCheckIndicator(option, painter, true);
      return;

    case PE_Frame: {
      const QColor fill = viewportColor(widget);
      fillRoundedRect(painter, option->rect, VIEW_FRAME_RADIUS, fill.isValid() ? QBrush(fill) : QBrush(Qt::NoBrush),
                      outlineColor(option));
      return;
    }

    case PE_FrameGroupBox: {
      const QStyleOptionFrame* frame = qstyleoption_cast<const QStyleOptionFrame*>(option);
      if (frame && (frame->features & QStyleOptionFrame::Flat)) {
        painter->fillRect(QRect(option->rect.left(), option->rect.top(), option->rect.width(), 1),
                          outlineColor(option));
        return;
      }
      const qreal fillAlpha = isDarkPalette(option->palette) ? 0.03 : 0.02;
      fillRoundedRect(painter, option->rect, m_cornerRadius,
                      withAlpha(option->palette.color(QPalette::WindowText), fillAlpha), outlineColor(option));
      return;
    }

    case PE_FrameTabWidget: {
      const QColor window = option->palette.color(QPalette::Window);
      const QColor base = option->palette.color(QPalette::Base);
      const QColor fill = isDarkColor(window) ? mix(window, base, 0.6) : base;
      fillRoundedRect(painter, option->rect, m_cornerRadius, fill, outlineColor(option));
      return;
    }

    case PE_FrameTabBarBase:
      if (const QStyleOptionTabBarBase* tabBase = qstyleoption_cast<const QStyleOptionTabBarBase*>(option)) {
        const QRect r = tabBase->rect;
        QRect line;
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
            line = QRect(r.left(), r.bottom(), r.width(), 1);
            break;
        }
        painter->fillRect(line, outlineColor(option));
        return;
      }
      break;

    case PE_PanelMenu:
      painter->fillRect(option->rect, option->palette.color(QPalette::Base));
      return;

    case PE_FrameMenu:
      painter->save();
      painter->setPen(outlineColor(option));
      painter->setBrush(Qt::NoBrush);
      painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
      painter->restore();
      return;

    case PE_PanelTipLabel: {
      const QColor base = option->palette.color(QPalette::ToolTipBase);
      painter->fillRect(option->rect, base);
      painter->save();
      painter->setPen(mix(base, option->palette.color(QPalette::ToolTipText), 0.2));
      painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
      painter->restore();
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
      const QColor textColor = paletteColor(option, QPalette::Text);
      const bool hovered = option->state & State_MouseOver;
      drawChevron(painter, option->rect, arrow,
                  mix(textColor, option->palette.color(QPalette::Base), hovered ? 0.1 : 0.4));
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
      const qreal size = qMin<qreal>(8.0, qMin(option->rect.width(), option->rect.height()) * 0.7);
      drawChevron(painter, option->rect, arrow, paletteColor(option, QPalette::ButtonText), size);
      return;
    }

    case PE_IndicatorHeaderArrow:
      if (const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader*>(option)) {
        const QColor color = mix(paletteColor(option, QPalette::ButtonText), option->palette.color(QPalette::Base), 0.3);
        if (header->sortIndicator & QStyleOptionHeader::SortUp) {
          drawChevron(painter, option->rect, Qt::UpArrow, color);
        } else if (header->sortIndicator & QStyleOptionHeader::SortDown) {
          drawChevron(painter, option->rect, Qt::DownArrow, color);
        }
        return;
      }
      break;

    case PE_IndicatorToolBarSeparator: {
      const QRect r = option->rect;
      if (option->state & State_Horizontal) {
        painter->fillRect(QRect(r.center().x(), r.top() + 4, 1, r.height() - 8), outlineColor(option));
      } else {
        painter->fillRect(QRect(r.left() + 4, r.center().y(), r.width() - 8, 1), outlineColor(option));
      }
      return;
    }

    default:
      break;
  }
  QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void ModernStyle::drawButtonPanel(const QStyleOption* option, QPainter* painter, bool isDefault, bool isFlat) const {
  const bool enabled = option->state & State_Enabled;
  const bool hovered = enabled && (option->state & State_MouseOver);
  const bool pressed = enabled && (option->state & State_Sunken);
  const bool checked = option->state & State_On;
  const bool focused = enabled && hasKeyboardFocus(option);
  const bool dark = isDarkPalette(option->palette);
  const QColor accent = option->palette.color(QPalette::Highlight);
  const QColor button = paletteColor(option, QPalette::Button);
  const QColor buttonText = paletteColor(option, QPalette::ButtonText);
  const QRectF rect(option->rect);

  if (isFlat && !hovered && !pressed && !checked) {
    if (focused) {
      fillRoundedRect(painter, rect, m_cornerRadius, Qt::NoBrush, accent, 2.0);
    }
    return;
  }

  QColor fill;
  QColor border;
  if (isDefault && enabled) {
    fill = pressed ? accentPressed(accent) : hovered ? accentHover(accent, dark) : accent;
  } else if (checked) {
    fill = mix(button, accent, pressed ? 0.4 : hovered ? 0.3 : 0.22);
    border = mix(button, accent, 0.6);
  } else {
    fill = button;
    if (pressed) {
      fill = mix(button, buttonText, dark ? 0.14 : 0.09);
    } else if (hovered) {
      fill = mix(button, buttonText, dark ? 0.07 : 0.04);
    }
    if (!isFlat) {
      border = hovered ? strongOutlineColor(option) : outlineColor(option);
    }
  }
  fillRoundedRect(painter, rect, m_cornerRadius, fill, border);

  if (focused) {
    const QColor ring = isDefault ? withAlpha(option->palette.color(QPalette::HighlightedText), 0.7) : accent;
    fillRoundedRect(painter, rect, m_cornerRadius, Qt::NoBrush, ring, 2.0);
  }
}

void ModernStyle::drawToolPanel(const QStyleOption* option, QPainter* painter) const {
  if (!(option->state & State_AutoRaise)) {
    drawButtonPanel(option, painter, false, false);
    return;
  }
  const bool enabled = option->state & State_Enabled;
  const bool hovered = enabled && (option->state & State_MouseOver);
  const bool pressed = enabled && (option->state & State_Sunken);
  const bool checked = option->state & State_On;
  const QColor accent = option->palette.color(QPalette::Highlight);

  QColor fill;
  if (pressed) {
    fill = hoverOverlay(option, 2.2);
  } else if (checked) {
    fill = withAlpha(accent, (isDarkPalette(option->palette) ? 0.3 : 0.18) + (hovered ? 0.08 : 0.0));
  } else if (hovered) {
    fill = hoverOverlay(option);
  } else {
    return;
  }
  fillRoundedRect(painter, option->rect, m_cornerRadius, fill);
}

void ModernStyle::drawFieldFrame(const QStyleOption* option, QPainter* painter, bool fill) const {
  const bool enabled = option->state & State_Enabled;
  const bool hovered = enabled && (option->state & State_MouseOver);
  const bool focused = enabled && (option->state & State_HasFocus);
  const bool readOnly = option->state & State_ReadOnly;

  QColor base = paletteColor(option, QPalette::Base);
  if (readOnly) {
    base = mix(base, option->palette.color(QPalette::Window), 0.5);
  }
  QColor border = outlineColor(option);
  if (focused) {
    border = option->palette.color(QPalette::Highlight);
  } else if (hovered) {
    border = strongOutlineColor(option);
  }
  fillRoundedRect(painter, option->rect, m_cornerRadius, fill ? QBrush(base) : QBrush(Qt::NoBrush), border,
                  focused ? 2.0 : 1.0);
}

void ModernStyle::drawCheckIndicator(const QStyleOption* option, QPainter* painter, bool isRadio) const {
  const qreal size = qMin(option->rect.width(), option->rect.height());
  QRectF box(0, 0, size, size);
  box.moveCenter(QRectF(option->rect).center());

  const bool enabled = option->state & State_Enabled;
  const bool hovered = enabled && (option->state & State_MouseOver);
  const bool pressed = enabled && (option->state & State_Sunken);
  const bool checked = option->state & State_On;
  const bool partial = !isRadio && (option->state & State_NoChange);
  const bool dark = isDarkPalette(option->palette);
  const QColor accent = option->palette.color(QPalette::Highlight);
  const QColor window = option->palette.color(QPalette::Window);
  const QColor text = option->palette.color(QPalette::WindowText);
  const qreal radius = isRadio ? size / 2.0 : qBound<qreal>(2.0, m_cornerRadius - 2.0, size / 3.0);

  if (checked || partial) {
    QColor fill = mix(window, text, 0.25);
    if (enabled) {
      fill = pressed ? accentPressed(accent) : hovered ? accentHover(accent, dark) : accent;
    }
    fillRoundedRect(painter, box, radius, fill);

    const QColor mark = enabled ? option->palette.color(QPalette::HighlightedText) : window;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    if (isRadio) {
      const qreal dot = size * 0.4;
      painter->setPen(Qt::NoPen);
      painter->setBrush(mark);
      painter->drawEllipse(QRectF(box.center().x() - dot / 2.0, box.center().y() - dot / 2.0, dot, dot));
    } else {
      QPen pen(mark, qMax<qreal>(1.5, size / 9.0));
      pen.setCapStyle(Qt::RoundCap);
      pen.setJoinStyle(Qt::RoundJoin);
      painter->setPen(pen);
      painter->setBrush(Qt::NoBrush);
      if (partial && !checked) {
        painter->drawLine(QPointF(box.left() + size * 0.28, box.center().y()),
                          QPointF(box.right() - size * 0.28, box.center().y()));
      } else {
        const QPointF points[3] = {
          QPointF(box.left() + size * 0.25, box.top() + size * 0.52),
          QPointF(box.left() + size * 0.43, box.top() + size * 0.70),
          QPointF(box.left() + size * 0.76, box.top() + size * 0.33),
        };
        painter->drawPolyline(points, 3);
      }
    }
    painter->restore();
  } else {
    QColor fill = paletteColor(option, QPalette::Base);
    if (pressed) {
      fill = mix(fill, text, 0.08);
    }
    QColor border = mix(window, text, dark ? 0.4 : 0.34);
    if (!enabled) {
      border = outlineColor(option);
    } else if (hovered) {
      border = accent;
    }
    fillRoundedRect(painter, box, radius, fill, border);
  }

  if (enabled && hasKeyboardFocus(option)) {
    const QRectF ring = box.adjusted(-3, -3, 3, 3);
    fillRoundedRect(painter, ring, isRadio ? ring.width() / 2.0 : radius + 3, Qt::NoBrush, withAlpha(accent, 0.6), 2.0);
  }
}

void ModernStyle::drawItemViewBackground(const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
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

  const bool enabled = item->state & State_Enabled;
  const bool selected = item->state & State_Selected;
  const bool hovered = enabled && (item->state & State_MouseOver);
  if (!selected && !hovered) {
    return;
  }

  const bool dark = isDarkPalette(item->palette);
  QColor color;
  if (selected) {
    const QColor accent = item->palette.color(enabled ? QPalette::Active : QPalette::Disabled, QPalette::Highlight);
    const bool active = item->state & State_Active;
    qreal alpha = dark ? 0.38 : 0.22;
    if (!active) {
      alpha *= 0.65;
    }
    if (hovered) {
      alpha += 0.06;
    }
    color = withAlpha(accent, alpha);
  } else {
    color = hoverOverlay(item);
  }

  QStyleOptionViewItem::ViewItemPosition position = item->viewItemPosition;
  if (position == QStyleOptionViewItem::Invalid && qobject_cast<const QListView*>(widget)) {
    position = QStyleOptionViewItem::OnlyOne;
  }
  if (position == QStyleOptionViewItem::Invalid) {
    // Tables: plain cells keep the grid readable
    painter->fillRect(item->rect, color);
    return;
  }
  if (item->direction == Qt::RightToLeft) {
    if (position == QStyleOptionViewItem::Beginning) {
      position = QStyleOptionViewItem::End;
    } else if (position == QStyleOptionViewItem::End) {
      position = QStyleOptionViewItem::Beginning;
    }
  }

  // Round only the outer corners so a multi-column row reads as one pill
  const bool roundLeft = position == QStyleOptionViewItem::Beginning || position == QStyleOptionViewItem::OnlyOne;
  const bool roundRight = position == QStyleOptionViewItem::End || position == QStyleOptionViewItem::OnlyOne;
  const QRect clip = item->rect.adjusted(roundLeft ? 2 : 0, 1, roundRight ? -2 : 0, -1);
  const qreal radius = qMax(2, m_cornerRadius - 2);
  QRectF shape(clip);
  if (!roundLeft) {
    shape.adjust(-radius - 1, 0, 0, 0);
  }
  if (!roundRight) {
    shape.adjust(0, 0, radius + 1, 0);
  }
  painter->save();
  painter->setClipRect(clip, Qt::IntersectClip);
  fillRoundedRect(painter, shape, radius, color);
  painter->restore();
}

// ---------------------------------------------------------------------------
// Controls
// ---------------------------------------------------------------------------

void ModernStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter,
                              const QWidget* widget) const {
  switch (element) {
    case CE_PushButtonBevel:
      if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
        proxy()->drawPrimitive(PE_PanelButtonCommand, button, painter, widget);
        if (button->features & QStyleOptionButton::HasMenu) {
          const bool accentText = (button->features & QStyleOptionButton::DefaultButton) && (button->state & State_Enabled);
          const QColor color = accentText ? button->palette.color(QPalette::HighlightedText)
                                          : paletteColor(button, QPalette::ButtonText);
          const int indicatorWidth = proxy()->pixelMetric(PM_MenuButtonIndicator, button, widget);
          const QRect arrowRect(button->rect.right() - indicatorWidth - 6, button->rect.top(), indicatorWidth,
                                button->rect.height());
          drawChevron(painter, visualRect(button->direction, button->rect, arrowRect), Qt::DownArrow, color);
        }
        return;
      }
      break;

    case CE_PushButtonLabel:
      if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
        if ((button->features & QStyleOptionButton::DefaultButton) && (button->state & State_Enabled)) {
          QStyleOptionButton copy(*button);
          setEnabledColor(copy.palette, QPalette::ButtonText, button->palette.color(QPalette::HighlightedText));
          QProxyStyle::drawControl(element, &copy, painter, widget);
          return;
        }
      }
      break;

    case CE_TabBarTabShape:
      drawTabShape(option, painter);
      return;

    case CE_TabBarTabLabel:
      if (const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
        if (!(tab->state & State_Selected)) {
          QStyleOptionTab copy(*tab);
          const QColor dimmed = mix(tab->palette.color(QPalette::WindowText), tab->palette.color(QPalette::Window), 0.3);
          setEnabledColor(copy.palette, QPalette::WindowText, dimmed);
          setEnabledColor(copy.palette, QPalette::ButtonText, dimmed);
          QProxyStyle::drawControl(element, &copy, painter, widget);
          return;
        }
      }
      break;

    case CE_HeaderSection:
      drawHeaderSection(option, painter);
      return;

    case CE_HeaderEmptyArea: {
      painter->fillRect(option->rect, option->palette.color(QPalette::Base));
      const QRect r = option->rect;
      if (option->state & State_Horizontal) {
        painter->fillRect(QRect(r.left(), r.bottom(), r.width(), 1), outlineColor(option));
      } else {
        painter->fillRect(QRect(r.right(), r.top(), 1, r.height()), outlineColor(option));
      }
      return;
    }

    case CE_ItemViewItem:
      if (const QStyleOptionViewItem* item = qstyleoption_cast<const QStyleOptionViewItem*>(option)) {
        if (item->state & State_Selected) {
          // Selection is a soft tint, so selected text keeps its normal color
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
        if (item->menuItemType == QStyleOptionMenuItem::Separator && item->text.isEmpty()) {
          const QRect r = item->rect;
          painter->fillRect(QRect(r.left() + 8, r.center().y(), r.width() - 16, 1), outlineColor(item));
          return;
        }
        QStyleOptionMenuItem copy(*item);
        if ((item->state & State_Selected) && (item->state & State_Enabled)) {
          fillRoundedRect(painter, QRectF(item->rect).adjusted(4, 1, -4, -1), qMax(2, m_cornerRadius - 2),
                          hoverOverlay(item, 1.6));
          copy.state &= ~State_Selected;
        }
        QProxyStyle::drawControl(element, &copy, painter, widget);
        return;
      }
      break;

    case CE_MenuBarEmptyArea:
      painter->fillRect(option->rect, option->palette.color(QPalette::Window));
      return;

    case CE_MenuBarItem:
      if (const QStyleOptionMenuItem* item = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
        const bool enabled = item->state & State_Enabled;
        painter->fillRect(item->rect, item->palette.color(QPalette::Window));
        if (enabled && (item->state & (State_Selected | State_Sunken))) {
          fillRoundedRect(painter, QRectF(item->rect).adjusted(1, 2, -1, -2), qMax(2, m_cornerRadius - 2),
                          hoverOverlay(item, (item->state & State_Sunken) ? 2.2 : 1.4));
        }
        int flags = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
        if (!proxy()->styleHint(SH_UnderlineShortcut, item, widget)) {
          flags |= Qt::TextHideMnemonic;
        }
        if (item->text.isEmpty() && !item->icon.isNull()) {
          const int iconSize = proxy()->pixelMetric(PM_SmallIconSize, item, widget);
          const QPixmap pixmap = item->icon.pixmap(iconSize, enabled ? QIcon::Normal : QIcon::Disabled);
          proxy()->drawItemPixmap(painter, item->rect, flags, pixmap);
        } else {
          proxy()->drawItemText(painter, item->rect, flags, item->palette, enabled, item->text, QPalette::ButtonText);
        }
        return;
      }
      break;

    case CE_ToolBar:
      // Toolbars blend into the window background
      return;

    case CE_Splitter:
      if ((option->state & State_Enabled) && (option->state & State_MouseOver)) {
        const QRect r = option->rect;
        const QColor color = withAlpha(option->palette.color(QPalette::Highlight), 0.7);
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
          painter->fillRect(QRect(r.left(), r.center().y(), r.width(), 1), outlineColor(frame));
          return;
        }
        if (frame->frameShape == QFrame::VLine) {
          painter->fillRect(QRect(r.center().x(), r.top(), 1, r.height()), outlineColor(frame));
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
        const QColor window = bar->palette.color(QPalette::Window);
        const QColor track = mix(window, bar->palette.color(QPalette::WindowText), isDarkColor(window) ? 0.18 : 0.12);
        fillRoundedRect(painter, progressTrack(bar->rect, isHorizontal(bar)), PROGRESS_THICKNESS / 2.0, track);
        return;
      }
      break;

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

void ModernStyle::drawTabShape(const QStyleOption* option, QPainter* painter) const {
  const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option);
  if (!tab) {
    return;
  }
  const QRect r = tab->rect;
  const bool selected = tab->state & State_Selected;
  const bool hovered = (tab->state & State_Enabled) && (tab->state & State_MouseOver);
  const bool vertical = isVerticalTab(tab->shape);

  if (hovered && !selected) {
    const QRectF pill = vertical ? QRectF(r).adjusted(4, 2, -4, -2) : QRectF(r).adjusted(2, 4, -2, -4);
    fillRoundedRect(painter, pill, m_cornerRadius, hoverOverlay(tab));
  }
  if (!selected) {
    return;
  }

  const qreal thickness = 3.0;
  const qreal inset = qMin(12, (vertical ? r.height() : r.width()) / 4);
  QRectF bar;
  switch (tab->shape) {
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
      bar = QRectF(r.left() + inset, r.top(), r.width() - 2 * inset, thickness);
      break;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
      bar = QRectF(r.right() + 1 - thickness, r.top() + inset, thickness, r.height() - 2 * inset);
      break;
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
      bar = QRectF(r.left(), r.top() + inset, thickness, r.height() - 2 * inset);
      break;
    default:
      bar = QRectF(r.left() + inset, r.bottom() + 1 - thickness, r.width() - 2 * inset, thickness);
      break;
  }
  fillRoundedRect(painter, bar, thickness / 2.0, paletteColor(tab, QPalette::Highlight));
}

void ModernStyle::drawHeaderSection(const QStyleOption* option, QPainter* painter) const {
  const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader*>(option);
  if (!header) {
    return;
  }
  const QRect r = header->rect;
  const bool enabled = header->state & State_Enabled;
  painter->fillRect(r, header->palette.color(QPalette::Base));
  if (enabled && (header->state & State_Sunken)) {
    painter->fillRect(r, hoverOverlay(header, 2.0));
  } else if (enabled && (header->state & State_MouseOver)) {
    painter->fillRect(r, hoverOverlay(header));
  }

  const QColor line = outlineColor(header);
  const bool isLast = header->position == QStyleOptionHeader::End ||
                      header->position == QStyleOptionHeader::OnlyOneSection;
  if (header->orientation == Qt::Horizontal) {
    painter->fillRect(QRect(r.left(), r.bottom(), r.width(), 1), line);
    if (!isLast) {
      const int x = header->direction == Qt::RightToLeft ? r.left() : r.right();
      painter->fillRect(QRect(x, r.top() + r.height() / 4, 1, r.height() / 2), line);
    }
  } else {
    const int x = header->direction == Qt::RightToLeft ? r.left() : r.right();
    painter->fillRect(QRect(x, r.top(), 1, r.height()), line);
  }
}

void ModernStyle::drawProgressContents(const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionProgressBar* bar = qstyleoption_cast<const QStyleOptionProgressBar*>(option);
  if (!bar) {
    return;
  }
  const bool horizontal = isHorizontal(bar);
  const QRectF track = progressTrack(bar->rect, horizontal);
  const qreal length = horizontal ? track.width() : track.height();
  const qreal thickness = horizontal ? track.height() : track.width();

  // Vertical bars fill bottom-up, horizontal ones follow the layout direction
  bool reverse = horizontal ? bar->direction == Qt::RightToLeft : true;
  if (bar->invertedAppearance) {
    reverse = !reverse;
  }

  qreal start = 0.0;
  qreal extent = 0.0;
  const qint64 range = qint64(bar->maximum) - bar->minimum;
  if (range <= 0) {
    const qreal segment = length * 0.3;
    const qreal phase = (QTime::currentTime().msecsSinceStartOfDay() % BUSY_CYCLE_MS) / qreal(BUSY_CYCLE_MS);
    start = -segment + phase * (length + segment);
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
  fillRoundedRect(painter, chunk, thickness / 2.0, paletteColor(bar, QPalette::Highlight));
}

// ---------------------------------------------------------------------------
// Complex controls
// ---------------------------------------------------------------------------

void ModernStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter,
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

void ModernStyle::drawSpinBox(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionSpinBox* spin = qstyleoption_cast<const QStyleOptionSpinBox*>(option);
  if (spin->frame) {
    drawFieldFrame(spin, painter, true);
  } else {
    painter->fillRect(spin->rect, spin->palette.brush(colorGroup(spin), QPalette::Base));
  }
  if (spin->buttonSymbols == QAbstractSpinBox::NoButtons) {
    return;
  }

  const bool enabled = spin->state & State_Enabled;
  for (SubControl subControl : {SC_SpinBoxUp, SC_SpinBoxDown}) {
    const bool isUp = subControl == SC_SpinBoxUp;
    const QRect r = proxy()->subControlRect(CC_SpinBox, spin, subControl, widget);
    const bool stepEnabled =
        enabled && (spin->stepEnabled & (isUp ? QAbstractSpinBox::StepUpEnabled : QAbstractSpinBox::StepDownEnabled));
    const bool active = stepEnabled && (spin->activeSubControls & subControl);
    if (active && (spin->state & State_Sunken)) {
      fillRoundedRect(painter, QRectF(r).adjusted(1, 1, -1, -1), qMax(2, m_cornerRadius - 3), hoverOverlay(spin, 2.2));
    } else if (active && (spin->state & State_MouseOver)) {
      fillRoundedRect(painter, QRectF(r).adjusted(1, 1, -1, -1), qMax(2, m_cornerRadius - 3), hoverOverlay(spin, 1.2));
    }

    const QColor color = spin->palette.color(stepEnabled ? QPalette::Active : QPalette::Disabled, QPalette::Text);
    if (spin->buttonSymbols == QAbstractSpinBox::PlusMinus) {
      painter->save();
      painter->setRenderHint(QPainter::Antialiasing, true);
      QPen pen(color, 1.5);
      pen.setCapStyle(Qt::RoundCap);
      painter->setPen(pen);
      const QPointF center = QRectF(r).center();
      painter->drawLine(center - QPointF(3.5, 0), center + QPointF(3.5, 0));
      if (isUp) {
        painter->drawLine(center - QPointF(0, 3.5), center + QPointF(0, 3.5));
      }
      painter->restore();
    } else {
      drawChevron(painter, r, isUp ? Qt::UpArrow : Qt::DownArrow, color, 7.0);
    }
  }
}

void ModernStyle::drawComboBox(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionComboBox* combo = qstyleoption_cast<const QStyleOptionComboBox*>(option);
  if (combo->editable) {
    if (combo->frame) {
      drawFieldFrame(combo, painter, true);
    } else {
      painter->fillRect(combo->rect, combo->palette.brush(colorGroup(combo), QPalette::Base));
    }
  } else if (combo->frame) {
    // State_On only means "popup open" here, which should not look checked
    QStyleOptionComboBox panel(*combo);
    panel.state &= ~State_On;
    drawButtonPanel(&panel, painter, false, false);
  } else if ((combo->state & State_Enabled) && (combo->state & State_MouseOver)) {
    fillRoundedRect(painter, combo->rect, m_cornerRadius, hoverOverlay(combo));
  }

  if (combo->subControls & SC_ComboBoxArrow) {
    const QRect arrow = proxy()->subControlRect(CC_ComboBox, combo, SC_ComboBoxArrow, widget);
    const QColor color = paletteColor(combo, combo->editable ? QPalette::Text : QPalette::ButtonText);
    drawChevron(painter, arrow, Qt::DownArrow, color);
  }
}

void ModernStyle::drawScrollBar(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionSlider* bar = qstyleoption_cast<const QStyleOptionSlider*>(option);
  const bool horizontal = bar->orientation == Qt::Horizontal;
  const bool enabled = bar->state & State_Enabled;
  const bool hovered = enabled && (bar->state & State_MouseOver);
  const bool sliderActive = bar->activeSubControls & SC_ScrollBarSlider;
  const bool sliderPressed = sliderActive && (bar->state & State_Sunken);
  const bool sliderHovered = hovered && sliderActive;
  const QColor window = bar->palette.color(QPalette::Window);
  const QColor text = bar->palette.color(QPalette::WindowText);

  const QRectF groove(proxy()->subControlRect(CC_ScrollBar, bar, SC_ScrollBarGroove, widget));
  const qreal thickness = (hovered || sliderPressed) ? 8.0 : 5.0;
  const auto centered = [&](const QRectF& r) {
    return horizontal ? QRectF(r.left(), groove.center().y() - thickness / 2.0, r.width(), thickness)
                      : QRectF(groove.center().x() - thickness / 2.0, r.top(), thickness, r.height());
  };

  if (hovered || sliderPressed) {
    const QRectF track = centered(horizontal ? groove.adjusted(2, 0, -2, 0) : groove.adjusted(0, 2, 0, -2));
    fillRoundedRect(painter, track, thickness / 2.0, withAlpha(text, 0.06));
  }

  if (bar->maximum == bar->minimum || !(bar->subControls & SC_ScrollBarSlider)) {
    return;
  }
  const QRectF slider(proxy()->subControlRect(CC_ScrollBar, bar, SC_ScrollBarSlider, widget));
  const QRectF handle = centered(horizontal ? slider.adjusted(2, 0, -2, 0) : slider.adjusted(0, 2, 0, -2));
  qreal amount = 0.3;
  if (!enabled) {
    amount = 0.15;
  } else if (sliderPressed) {
    amount = 0.6;
  } else if (sliderHovered) {
    amount = 0.48;
  } else if (hovered) {
    amount = 0.38;
  }
  fillRoundedRect(painter, handle, thickness / 2.0, mix(window, text, amount));
}

void ModernStyle::drawSlider(const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const {
  const QStyleOptionSlider* slider = qstyleoption_cast<const QStyleOptionSlider*>(option);
  const bool horizontal = slider->orientation == Qt::Horizontal;
  const bool enabled = slider->state & State_Enabled;
  const bool dark = isDarkPalette(slider->palette);
  const QColor window = slider->palette.color(QPalette::Window);
  const QColor text = slider->palette.color(QPalette::WindowText);
  const QColor accent = enabled ? slider->palette.color(QPalette::Highlight) : mix(window, text, 0.3);
  const QRect groove = proxy()->subControlRect(CC_Slider, slider, SC_SliderGroove, widget);
  const QRect handle = proxy()->subControlRect(CC_Slider, slider, SC_SliderHandle, widget);
  const QPointF handleCenter = QRectF(handle).center();

  if (slider->subControls & SC_SliderGroove) {
    const qreal radius = SLIDER_GROOVE_THICKNESS / 2.0;
    fillRoundedRect(painter, groove, radius, mix(window, text, dark ? 0.22 : 0.16));

    // The filled part runs from the minimum to the handle
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
    const int available = (horizontal ? r.width() : r.height()) - SLIDER_HANDLE_SIZE;
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
    painter->setPen(mix(window, text, 0.35));
    for (qint64 value = slider->minimum; value <= slider->maximum; value += interval) {
      const int position = sliderPositionFromValue(slider->minimum, slider->maximum, int(value), available,
                                                   slider->upsideDown) + SLIDER_HANDLE_SIZE / 2;
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
    const QRectF knob(handle);
    const bool handleActive = slider->activeSubControls & SC_SliderHandle;
    const bool pressed = enabled && handleActive && (slider->state & State_Sunken);
    const bool hovered = enabled && handleActive && (slider->state & State_MouseOver);
    const QColor knobFill = dark ? mix(slider->palette.color(QPalette::Button), text, 0.12)
                                 : slider->palette.color(QPalette::Base);
    fillRoundedRect(painter, knob, knob.width() / 2.0, knobFill, outlineColor(slider));

    qreal dotScale = 0.5;
    if (pressed) {
      dotScale = 0.42;
    } else if (hovered) {
      dotScale = 0.6;
    }
    const qreal dot = knob.width() * dotScale;
    const QRectF dotRect(knob.center().x() - dot / 2.0, knob.center().y() - dot / 2.0, dot, dot);
    fillRoundedRect(painter, dotRect, dot / 2.0, accent);

    if (enabled && hasKeyboardFocus(slider)) {
      fillRoundedRect(painter, knob, knob.width() / 2.0, Qt::NoBrush, withAlpha(accent, 0.6), 2.0);
    }
  }
}

// ---------------------------------------------------------------------------
// Geometry
// ---------------------------------------------------------------------------

QRect ModernStyle::subControlRect(ComplexControl control, const QStyleOptionComplex* option, SubControl subControl,
                                  const QWidget* widget) const {
  switch (control) {
    case CC_SpinBox:
      if (const QStyleOptionSpinBox* spin = qstyleoption_cast<const QStyleOptionSpinBox*>(option)) {
        const QRect r = spin->rect;
        const int frameWidth = spin->frame ? FRAME_WIDTH : 0;
        const bool hasButtons = spin->buttonSymbols != QAbstractSpinBox::NoButtons;
        const int buttonWidth = hasButtons ? SPIN_BUTTON_WIDTH : 0;
        // Keep the square inner edit clear of the rounded corners
        const int padding = qMax(FIELD_PADDING, m_cornerRadius);
        const int innerHeight = r.height() - 2 * frameWidth;
        const int buttonX = r.right() - frameWidth - buttonWidth + 1;
        QRect result;
        switch (subControl) {
          case SC_SpinBoxFrame:
            return r;
          case SC_SpinBoxEditField:
            result = QRect(r.left() + frameWidth + padding, r.top() + frameWidth,
                           r.width() - 2 * frameWidth - padding - (hasButtons ? buttonWidth : padding), innerHeight);
            break;
          case SC_SpinBoxUp:
            if (!hasButtons) {
              return QRect();
            }
            result = QRect(buttonX, r.top() + frameWidth, buttonWidth, innerHeight / 2);
            break;
          case SC_SpinBoxDown:
            if (!hasButtons) {
              return QRect();
            }
            result = QRect(buttonX, r.top() + frameWidth + innerHeight / 2, buttonWidth, innerHeight - innerHeight / 2);
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
        const int frameWidth = combo->frame ? FRAME_WIDTH : 0;
        const int padding = qMax(FIELD_PADDING, m_cornerRadius);
        QRect result;
        switch (subControl) {
          case SC_ComboBoxFrame:
            return r;
          case SC_ComboBoxArrow:
            result = QRect(r.right() - frameWidth - COMBO_ARROW_WIDTH + 1, r.top() + frameWidth, COMBO_ARROW_WIDTH,
                           r.height() - 2 * frameWidth);
            break;
          case SC_ComboBoxEditField:
            result = QRect(r.left() + frameWidth + padding, r.top() + frameWidth,
                           r.width() - 2 * frameWidth - padding - COMBO_ARROW_WIDTH, r.height() - 2 * frameWidth);
            break;
          default:
            return QProxyStyle::subControlRect(control, option, subControl, widget);
        }
        return visualRect(combo->direction, r, result);
      }
      break;

    case CC_ScrollBar:
      if (const QStyleOptionSlider* bar = qstyleoption_cast<const QStyleOptionSlider*>(option)) {
        // No arrow buttons: the groove spans the whole bar
        const QRect r = bar->rect;
        const bool horizontal = bar->orientation == Qt::Horizontal;
        const int length = horizontal ? r.width() : r.height();
        const qint64 range = qint64(bar->maximum) - bar->minimum;
        int sliderLength = length;
        if (range > 0) {
          const int minimum = proxy()->pixelMetric(PM_ScrollBarSliderMin, bar, widget);
          sliderLength = int(qint64(length) * bar->pageStep / (range + bar->pageStep));
          sliderLength = qMin(length, qMax(minimum, sliderLength));
        }
        const int sliderStart = sliderPositionFromValue(bar->minimum, bar->maximum, bar->sliderPosition,
                                                        length - sliderLength, bar->upsideDown);
        QRect result;
        switch (subControl) {
          case SC_ScrollBarGroove:
            return r;
          case SC_ScrollBarSlider:
            result = horizontal ? QRect(r.left() + sliderStart, r.top(), sliderLength, r.height())
                                : QRect(r.left(), r.top() + sliderStart, r.width(), sliderLength);
            break;
          case SC_ScrollBarSubPage:
            result = horizontal ? QRect(r.left(), r.top(), sliderStart, r.height())
                                : QRect(r.left(), r.top(), r.width(), sliderStart);
            break;
          case SC_ScrollBarAddPage: {
            const int end = sliderStart + sliderLength;
            result = horizontal ? QRect(r.left() + end, r.top(), length - end, r.height())
                                : QRect(r.left(), r.top() + end, r.width(), length - end);
            break;
          }
          case SC_ScrollBarAddLine:
          case SC_ScrollBarSubLine:
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
            return horizontal ? QRect(r.left(), r.top() + (r.height() - SLIDER_GROOVE_THICKNESS) / 2, r.width(),
                                      SLIDER_GROOVE_THICKNESS)
                              : QRect(r.left() + (r.width() - SLIDER_GROOVE_THICKNESS) / 2, r.top(),
                                      SLIDER_GROOVE_THICKNESS, r.height());
          case SC_SliderHandle: {
            const int available = (horizontal ? r.width() : r.height()) - SLIDER_HANDLE_SIZE;
            const int position = sliderPositionFromValue(slider->minimum, slider->maximum, slider->sliderPosition,
                                                         available, slider->upsideDown);
            return horizontal ? QRect(r.left() + position, r.top() + (r.height() - SLIDER_HANDLE_SIZE) / 2,
                                      SLIDER_HANDLE_SIZE, SLIDER_HANDLE_SIZE)
                              : QRect(r.left() + (r.width() - SLIDER_HANDLE_SIZE) / 2, r.top() + position,
                                      SLIDER_HANDLE_SIZE, SLIDER_HANDLE_SIZE);
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

QRect ModernStyle::subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget) const {
  switch (element) {
    case SE_LineEditContents:
      if (const QStyleOptionFrame* frame = qstyleoption_cast<const QStyleOptionFrame*>(option)) {
        if (frame->lineWidth > 0) {
          const int width = frame->lineWidth;
          return frame->rect.adjusted(width + 6, width, -(width + 6), -width);
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
            qMax(metrics.horizontalAdvance(QStringLiteral("100%")), metrics.horizontalAdvance(bar->text)) + 8;
        const QRect result = element == SE_ProgressBarLabel
                                 ? QRect(r.right() - labelWidth + 1, r.top(), labelWidth, r.height())
                                 : QRect(r.left(), r.top(), r.width() - labelWidth, r.height());
        return visualRect(bar->direction, r, result);
      }
      break;

    default:
      break;
  }
  return QProxyStyle::subElementRect(element, option, widget);
}

QSize ModernStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size,
                                    const QWidget* widget) const {
  switch (type) {
    case CT_PushButton:
      if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
        QSize result(size.width() + 32, qMax(size.height() + 10, CONTROL_HEIGHT));
        if (!button->text.isEmpty()) {
          result.setWidth(qMax(result.width(), 80));
        }
        return result;
      }
      break;

    case CT_ToolButton:
      return QProxyStyle::sizeFromContents(type, option, size, widget) + QSize(4, 4);

    case CT_LineEdit:
      if (const QStyleOptionFrame* frame = qstyleoption_cast<const QStyleOptionFrame*>(option)) {
        if (frame->lineWidth > 0) {
          const QSize base = QProxyStyle::sizeFromContents(type, option, size, widget);
          return QSize(base.width() + 12, qMax(base.height(), CONTROL_HEIGHT));
        }
      }
      break;

    case CT_SpinBox:
      if (const QStyleOptionSpinBox* spin = qstyleoption_cast<const QStyleOptionSpinBox*>(option)) {
        const int frameWidth = spin->frame ? FRAME_WIDTH : 0;
        const int padding = qMax(FIELD_PADDING, m_cornerRadius);
        const int trailing = spin->buttonSymbols != QAbstractSpinBox::NoButtons ? SPIN_BUTTON_WIDTH : padding;
        return QSize(size.width() + 2 * frameWidth + padding + trailing,
                     qMax(size.height() + 2 * frameWidth, CONTROL_HEIGHT));
      }
      break;

    case CT_ComboBox:
      if (const QStyleOptionComboBox* combo = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
        const int frameWidth = combo->frame ? FRAME_WIDTH : 0;
        const int padding = qMax(FIELD_PADDING, m_cornerRadius);
        return QSize(size.width() + 2 * frameWidth + padding + COMBO_ARROW_WIDTH + 4,
                     qMax(size.height() + 2 * frameWidth + 4, CONTROL_HEIGHT));
      }
      break;

    case CT_MenuItem:
      if (const QStyleOptionMenuItem* item = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
        QSize result = QProxyStyle::sizeFromContents(type, option, size, widget);
        if (item->menuItemType == QStyleOptionMenuItem::Separator && item->text.isEmpty()) {
          result.setHeight(9);
        } else {
          result += QSize(8, 6);
        }
        return result;
      }
      break;

    case CT_MenuBarItem:
      return QProxyStyle::sizeFromContents(type, option, size, widget) + QSize(8, 4);

    case CT_ItemViewItem:
      return QProxyStyle::sizeFromContents(type, option, size, widget) + QSize(0, 6);

    case CT_HeaderSection:
      if (const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader*>(option)) {
        // Vertical headers set the row height of tables; keep those compact
        const QSize extra = header->orientation == Qt::Horizontal ? QSize(0, 6) : QSize(4, 0);
        return QProxyStyle::sizeFromContents(type, option, size, widget) + extra;
      }
      break;

    default:
      break;
  }
  return QProxyStyle::sizeFromContents(type, option, size, widget);
}

int ModernStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const {
  switch (metric) {
    case PM_DefaultFrameWidth:
    case PM_SpinBoxFrameWidth:
    case PM_ComboBoxFrameWidth:
      return FRAME_WIDTH;
    case PM_ButtonDefaultIndicator:
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
    case PM_TabBarTabShiftHorizontal:
    case PM_TabBarTabShiftVertical:
    case PM_MenuHMargin:
    case PM_MenuBarPanelWidth:
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
      return 8;
    case PM_ScrollBarExtent:
      return SCROLLBAR_EXTENT;
    case PM_ScrollBarSliderMin:
      return 32;
    case PM_SliderThickness:
      return SLIDER_HANDLE_SIZE + 4;
    case PM_SliderLength:
    case PM_SliderControlThickness:
      return SLIDER_HANDLE_SIZE;
    case PM_SliderTickmarkOffset:
      return 5;
    case PM_MenuVMargin:
    case PM_MenuBarHMargin:
    case PM_ToolTipLabelFrameWidth:
    case PM_ToolBarItemSpacing:
      return 4;
    case PM_MenuPanelWidth:
      return 1;
    case PM_MenuBarItemSpacing:
    case PM_MenuBarVMargin:
      return 2;
    case PM_HeaderMargin:
      return 8;
    case PM_FocusFrameHMargin:
      // Also the text margin inside item view cells
      return 4;
    case PM_TabBarTabHSpace:
      return 28;
    case PM_TabBarTabVSpace:
      return 14;
    case PM_TabBarBaseOverlap:
      // Lets the selected-tab indicator sit on the pane border
      return 1;
    default:
      break;
  }
  return QProxyStyle::pixelMetric(metric, option, widget);
}

int ModernStyle::styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget,
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
        return int(outlineColor(option).rgba());
      }
      break;
    default:
      break;
  }
  return QProxyStyle::styleHint(hint, option, widget, returnData);
}

QPixmap ModernStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap& pixmap, const QStyleOption* option) const {
  // Selection is a soft tint already; the default accent wash would muddy icons
  if (iconMode == QIcon::Selected) {
    return pixmap;
  }
  return QProxyStyle::generatedIconPixmap(iconMode, pixmap, option);
}

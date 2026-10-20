#pragma once

// Painting utilities shared by the styles in this library

#include <QAbstractScrollArea>
#include <QColor>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QTabBar>

namespace StyleHelpers {

inline QColor mix(const QColor& from, const QColor& to, qreal amount) {
  const qreal inverse = 1.0 - amount;
  return QColor::fromRgbF(from.redF() * inverse + to.redF() * amount,
                          from.greenF() * inverse + to.greenF() * amount,
                          from.blueF() * inverse + to.blueF() * amount,
                          from.alphaF() * inverse + to.alphaF() * amount);
}

inline QColor withAlpha(QColor color, qreal alpha) {
  color.setAlphaF(alpha);
  return color;
}

inline bool isDarkColor(const QColor& color) {
  return qGray(color.rgb()) < 128;
}

inline bool isDarkPalette(const QPalette& palette) {
  return isDarkColor(palette.color(QPalette::Window));
}

inline QPalette::ColorGroup colorGroup(const QStyleOption* option) {
  if (!(option->state & QStyle::State_Enabled)) {
    return QPalette::Disabled;
  }
  return (option->state & QStyle::State_Active) ? QPalette::Active : QPalette::Inactive;
}

inline QColor paletteColor(const QStyleOption* option, QPalette::ColorRole role) {
  return option->palette.color(colorGroup(option), role);
}

// Sets a role for the enabled color groups only, keeping the disabled look intact
inline void setEnabledColor(QPalette& palette, QPalette::ColorRole role, const QColor& color) {
  palette.setColor(QPalette::Active, role, color);
  palette.setColor(QPalette::Inactive, role, color);
}

// A hover or pressed shade of an arbitrary color: nudged toward black on light colors, white on dark ones
inline QColor shade(const QColor& color, qreal amount) {
  return mix(color, isDarkColor(color) ? QColor(Qt::white) : QColor(Qt::black), amount);
}

/*
  The option's color for role when it differs from the style's own palette, meaning the
  application or widget palette overrides it; an invalid color otherwise. Enabled controls
  compare the Active group, disabled ones the Disabled group.
*/
inline QColor overriddenColor(const QStyleOption* option, QPalette::ColorRole role, const QPalette& standard) {
  const QPalette::ColorGroup group = (option->state & QStyle::State_Enabled) ? QPalette::Active : QPalette::Disabled;
  const QColor color = option->palette.color(group, role);
  return color.rgba() == standard.color(group, role).rgba() ? QColor() : color;
}

inline QColor overriddenOr(const QStyleOption* option, QPalette::ColorRole role, const QPalette& standard,
                           const QColor& fallback) {
  const QColor color = overriddenColor(option, role, standard);
  return color.isValid() ? color : fallback;
}

// Whether actual keeps the window and text colors of a style's own palette. Compares the
// Active group: a disabled widget's palette reports its disabled text color by default.
inline bool hasStandardNeutrals(const QPalette& actual, const QPalette& standard) {
  return actual.color(QPalette::Active, QPalette::Window).rgba() ==
             standard.color(QPalette::Active, QPalette::Window).rgba() &&
         actual.color(QPalette::Active, QPalette::WindowText).rgba() ==
             standard.color(QPalette::Active, QPalette::WindowText).rgba();
}

/*
  Moves a grey picked for a style's own palette (standard) into another palette (actual).
  The grey keeps its relative position between the window and text colors, so contrast
  steps survive and the result takes on the new palette's tint. Colored values, such as
  accent tints, are returned unchanged; alpha is kept.
*/
inline QColor retint(const QColor& color, const QPalette& standard, const QPalette& actual) {
  const QColor hsv = color.toHsv();
  if (hsv.hsvSaturationF() > 0.02) {
    return color;
  }
  const QColor fromWindow = standard.color(QPalette::Active, QPalette::Window);
  const int fromWindowGray = qGray(fromWindow.rgb());
  const int span = qGray(standard.color(QPalette::Active, QPalette::WindowText).rgb()) - fromWindowGray;
  if (span == 0) {
    return color;
  }
  // Can fall slightly outside 0..1, for greys lighter than the window in a light theme
  const qreal amount = qreal(qGray(color.rgb()) - fromWindowGray) / span;
  const QColor toWindow = actual.color(QPalette::Active, QPalette::Window);
  const QColor toText = actual.color(QPalette::Active, QPalette::WindowText);
  const auto channel = [amount](qreal from, qreal to) { return qBound<qreal>(0.0, from + (to - from) * amount, 1.0); };
  return QColor::fromRgbF(channel(toWindow.redF(), toText.redF()), channel(toWindow.greenF(), toText.greenF()),
                          channel(toWindow.blueF(), toText.blueF()), color.alphaF());
}

inline bool isAccentRole(QPalette::ColorRole role) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
  if (role == QPalette::Accent) {
    return true;
  }
#endif
  return role == QPalette::Highlight || role == QPalette::Link || role == QPalette::LinkVisited;
}

/*
  The palette to install when a style's own palette changes from previous to next. Roles
  in current that differ from previous were set by the application, so they're kept.
  When the accent itself changed, the accent-derived roles take next's value regardless.
*/
inline QPalette mergeCustomRoles(const QPalette& next, const QPalette& previous, const QPalette& current,
                                 bool accentChanged) {
  QPalette merged = next;
  for (int group = 0; group < QPalette::NColorGroups; group++) {
    const QPalette::ColorGroup colorGroup = QPalette::ColorGroup(group);
    for (int role = 0; role < QPalette::NColorRoles; role++) {
      const QPalette::ColorRole colorRole = QPalette::ColorRole(role);
      if (colorRole == QPalette::NoRole || (accentChanged && isAccentRole(colorRole))) {
        continue;
      }
      const QColor color = current.color(colorGroup, colorRole);
      if (color.rgba() != previous.color(colorGroup, colorRole).rgba()) {
        merged.setColor(colorGroup, colorRole, color);
      }
    }
  }
  return merged;
}

inline bool hasKeyboardFocus(const QStyleOption* option) {
  return (option->state & QStyle::State_HasFocus) && (option->state & QStyle::State_KeyboardFocusChange);
}

inline void fillRoundedRect(QPainter* painter, const QRectF& rect, qreal radius, const QBrush& fill,
                            const QColor& border = QColor(), qreal borderWidth = 1.0) {
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, true);
  QRectF shape = rect;
  qreal shapeRadius = radius;
  if (border.isValid() && border.alpha() > 0) {
    // Inset by half the pen width so the stroke stays crisp and inside rect
    const qreal inset = borderWidth / 2.0;
    shape.adjust(inset, inset, -inset, -inset);
    shapeRadius = qMax<qreal>(0.0, radius - inset);
    painter->setPen(QPen(border, borderWidth));
  } else {
    painter->setPen(Qt::NoPen);
  }
  painter->setBrush(fill);
  painter->drawRoundedRect(shape, shapeRadius, shapeRadius);
  painter->restore();
}

inline void drawChevron(QPainter* painter, const QRectF& rect, Qt::ArrowType direction, const QColor& color,
                        qreal size = 8.0, qreal penWidth = 1.5) {
  const QPointF center = rect.center();
  const qreal half = size / 2.0;
  const qreal depth = size / 4.0;
  QPointF points[3];
  switch (direction) {
    case Qt::UpArrow:
      points[0] = center + QPointF(-half, depth);
      points[1] = center + QPointF(0, -depth);
      points[2] = center + QPointF(half, depth);
      break;
    case Qt::DownArrow:
      points[0] = center + QPointF(-half, -depth);
      points[1] = center + QPointF(0, depth);
      points[2] = center + QPointF(half, -depth);
      break;
    case Qt::LeftArrow:
      points[0] = center + QPointF(depth, -half);
      points[1] = center + QPointF(-depth, 0);
      points[2] = center + QPointF(depth, half);
      break;
    case Qt::RightArrow:
      points[0] = center + QPointF(-depth, -half);
      points[1] = center + QPointF(depth, 0);
      points[2] = center + QPointF(-depth, half);
      break;
    default:
      return;
  }
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, true);
  QPen pen(color, penWidth);
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::RoundJoin);
  painter->setPen(pen);
  painter->setBrush(Qt::NoBrush);
  painter->drawPolyline(points, 3);
  painter->restore();
}

inline bool isHorizontal(const QStyleOptionProgressBar* bar) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  return bar->orientation == Qt::Horizontal;
#else
  return bar->state & QStyle::State_Horizontal;
#endif
}

inline bool hasProgressLabel(const QStyleOptionProgressBar* bar) {
  const bool busy = bar->minimum == bar->maximum;
  return bar->textVisible && !busy && isHorizontal(bar);
}

inline bool isVerticalTab(QTabBar::Shape shape) {
  return shape == QTabBar::RoundedWest || shape == QTabBar::RoundedEast || shape == QTabBar::TriangularWest ||
         shape == QTabBar::TriangularEast;
}

// Background of the viewport inside a scroll area, so its frame can match it
inline QColor viewportColor(const QWidget* widget) {
  const QAbstractScrollArea* area = qobject_cast<const QAbstractScrollArea*>(widget);
  if (!area || !area->viewport()) {
    return QColor();
  }
  const QWidget* viewport = area->viewport();
  if (!viewport->autoFillBackground() || viewport->backgroundRole() == QPalette::NoRole) {
    return QColor();
  }
  return viewport->palette().color(viewport->backgroundRole());
}

// Small solid triangle, as on scroll bar and spin buttons
inline void drawCaret(QPainter* painter, const QRectF& rect, Qt::ArrowType direction, const QColor& color, qreal size) {
  const QPointF center = rect.center();
  const qreal half = size / 2.0;
  const qreal depth = size * 0.3;
  QPolygonF triangle;
  switch (direction) {
    case Qt::UpArrow:
      triangle << center + QPointF(-half, depth) << center + QPointF(half, depth) << center + QPointF(0, -depth);
      break;
    case Qt::DownArrow:
      triangle << center + QPointF(-half, -depth) << center + QPointF(half, -depth) << center + QPointF(0, depth);
      break;
    case Qt::LeftArrow:
      triangle << center + QPointF(depth, -half) << center + QPointF(depth, half) << center + QPointF(-depth, 0);
      break;
    case Qt::RightArrow:
      triangle << center + QPointF(-depth, -half) << center + QPointF(-depth, half) << center + QPointF(depth, 0);
      break;
    default:
      return;
  }
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, true);
  QPen pen(color, 1.0);
  pen.setJoinStyle(Qt::RoundJoin);
  painter->setPen(pen);
  painter->setBrush(color);
  painter->drawPolygon(triangle);
  painter->restore();
}

// Maps a tab drawn as if on a north tab bar (open side at the bottom) onto its real shape
inline QTransform tabTransform(const QStyleOptionTab* tab, QSizeF* localSize) {
  const QRect r = tab->rect;
  switch (tab->shape) {
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
      *localSize = QSizeF(r.width(), r.height());
      return QTransform(1, 0, 0, -1, r.left(), r.top() + r.height());
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
      *localSize = QSizeF(r.height(), r.width());
      return QTransform(0, 1, 1, 0, r.left(), r.top());
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
      *localSize = QSizeF(r.height(), r.width());
      return QTransform(0, 1, -1, 0, r.left() + r.width(), r.top());
    default:
      *localSize = QSizeF(r.width(), r.height());
      return QTransform::fromTranslate(r.left(), r.top());
  }
}

} // namespace StyleHelpers

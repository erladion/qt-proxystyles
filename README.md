# ModernStyle

Three `QProxyStyle`s for Qt Widgets that work with **Qt 5.12+ and Qt 6**. All three are built on
Fusion and redraw buttons, inputs, check boxes and radio buttons, sliders, progress bars, scroll bars, tabs,
menus, headers and item-view selections. Each ships its own light and dark palettes with a
configurable accent color.

- **`ModernStyle`**: a neutral flat, rounded look.
- **`WinUIStyle`**: mimics WinUI 3 / Windows 11 (Fluent Design). It uses WinUI's theme colors,
  translucent control fills, borders with a darker bottom edge (lighter top edge in dark mode),
  accent primary buttons, text fields with an accent underline on focus, inline spin buttons,
  browser-style tabs, pill selection indicators, thin scroll bars that expand on hover, and
  rounded menus and tooltips.
- **`Windows10Style`**: mimics the Windows 10 desktop (Win32) controls seen in Explorer and classic
  dialogs, on any platform. Square controls with thin borders and pale-blue hover and pressed
  states, 2px accent border on the default button, Explorer-style list and tree selection, green
  progress bars, blue trackbar thumbs, 17px scroll bars with arrow buttons, raised tabs, and group
  boxes with the title set into the frame line. The dark theme approximates Explorer's dark mode.

## Build

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64
cmake --build build
./build/modernstyle-demo                            # widget gallery, ModernStyle
./build/modernstyle-demo --style-name winui         # widget gallery, WinUIStyle
./build/modernstyle-demo --style-name windows10     # widget gallery, Windows10Style
```

By default CMake uses Qt6 if it can find it, and Qt5 otherwise. To force a version, pass
`-DMODERNSTYLE_QT_MAJOR_VERSION=5` (or `6`).

| Option | Default | |
|---|---|---|
| `MODERNSTYLE_BUILD_PLUGIN` | `ON` | Builds a `QStylePlugin` into `build/plugins/styles` |
| `MODERNSTYLE_BUILD_DEMO` | `ON` | Builds the `modernstyle-demo` gallery |

## Use it in your application

```cmake
add_subdirectory(modernstyle)
target_link_libraries(myapp PRIVATE ModernStyle::ModernStyle)
```

```cpp
#include "ModernStyle.h"

QApplication app(argc, argv);
auto* style = new ModernStyle(ModernStyle::Theme::System, QColor("#3584e4"));
QApplication::setStyle(style);   // the application takes ownership

// Later, at runtime:
style->setTheme(ModernStyle::Theme::Dark);
style->setAccentColor(QColor("#8b5cf6"));
style->setCornerRadius(8);
```

`WinUIStyle` has the same theme and accent API. Pass the *base* accent color (Windows' default is
`#0078D4`); the style derives the darker and lighter shades WinUI uses in each theme. On Windows
with Qt 6.6+, it uses the system accent color by default.

```cpp
#include "WinUIStyle.h"

QApplication::setStyle(new WinUIStyle(WinUIStyle::Theme::System));
QApplication::setFont(WinUIStyle::recommendedFont());   // optional: Segoe UI Variable, 14px
```

`Windows10Style` has the same API. Win32 controls ignore the system accent color, so its accent
defaults to Windows 10's `#0078D7`; the hover and selection blues are derived from it.

```cpp
#include "Windows10Style.h"

QApplication::setStyle(new Windows10Style(Windows10Style::Theme::Light));
QApplication::setFont(Windows10Style::recommendedFont());   // optional: Segoe UI 9pt
```

## Use it in apps you don't build yourself

Point Qt at the plugin and select the style by name:

```sh
QT_PLUGIN_PATH=/path/to/build/plugins someapp -style modern      # or modern-light / modern-dark
QT_PLUGIN_PATH=/path/to/build/plugins someapp -style winui       # or winui-light / winui-dark
QT_PLUGIN_PATH=/path/to/build/plugins someapp -style windows10   # or windows10-light / windows10-dark
```

The plugin only loads into an app built against the same major Qt version (and a compatible
minor version) as the plugin.

## Notes

- **Palette.** Setting a style installs its light or dark palette as the application palette.
  After that, `QApplication::setPalette()` and `QWidget::setPalette()` work as usual: the styles
  paint with the colors you set, for example a red `Base` on one line edit to flag invalid input.
  `setTheme()` and `setAccentColor()` replace only the colors that still match the style's own
  palette, so roles you set yourself survive a theme change. `setAccentColor()` also replaces
  `Highlight` and the link colors.
  - `ModernStyle` derives everything from the palette, including hover, pressed and border shades.
  - `WinUIStyle` and `Windows10Style` keep their exact Windows colors for every role you leave
    alone. A role you change replaces them, with hover and pressed shades derived from your color:

    | Role | Used for |
    |---|---|
    | `Window` | Window background; also picks the light or dark color set. Menu bar in `Windows10Style` |
    | `Base` | Text fields, spin boxes, editable combos, check boxes, item views, headers, menus and popups |
    | `Button` / `ButtonText` | Push buttons, fixed-list combos and framed tool buttons, and their text; menu bar text |
    | `Text` / `WindowText` | Text in fields, views and menus / labels, check boxes, group boxes, tabs |
    | `Highlight` / `HighlightedText` | Accent: default buttons, checked controls, focus, selection |
    | `ToolTipBase` / `ToolTipText` | Tooltips |

    Borders, glyphs, scroll bars, sliders, progress bars, tab pages, group boxes and hover
    highlights always keep the Windows colors.
- **System theme.** On Qt 6.5+, `Theme::System` follows the OS color scheme and updates live when it
  changes. On older Qt versions it picks light or dark once, from the palette in use at startup.
- **Menus and tooltips.** `ModernStyle` and `Windows10Style` keep them square. `WinUIStyle` rounds
  them using translucent windows. On X11 this is off by default, because without a compositor the corners
  render black. Call `setTranslucentPopups(true)` before the popups are first shown to enable it.
- **No Mica or acrylic.** `WinUIStyle` uses solid colors where WinUI shows the desktop through the
  window or a menu, and it draws no drop shadows under menus.
- **Windows 10 behavior.** Besides the look, `Windows10Style` switches a few behaviors to match
  Windows: dialog buttons use the Windows order (OK before Cancel), combo boxes open a plain list
  below the control, and focus rectangles only appear after keyboard navigation.
- **No transition animations.** State changes are instant, apart from the animated busy progress bar.

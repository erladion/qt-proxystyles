#include "ModernStyle.h"
#include "WinUIStyle.h"
#include "Windows10Style.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCommandLineParser>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <functional>

namespace {

QIcon swatchIcon(const QColor& color) {
  QPixmap pixmap(32, 32);
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(Qt::NoPen);
  painter.setBrush(color);
  painter.drawEllipse(QRectF(4, 4, 24, 24));
  return QIcon(pixmap);
}

QWidget* createButtonsGroup() {
  QGroupBox* group = new QGroupBox(QStringLiteral("Buttons"));
  QGridLayout* layout = new QGridLayout(group);

  QPushButton* primary = new QPushButton(QStringLiteral("Primary"));
  primary->setDefault(true);
  QPushButton* normal = new QPushButton(QStringLiteral("Secondary"));
  QPushButton* checkable = new QPushButton(QStringLiteral("Toggle"));
  checkable->setCheckable(true);
  checkable->setChecked(true);
  QPushButton* flat = new QPushButton(QStringLiteral("Flat"));
  flat->setFlat(true);
  QPushButton* disabled = new QPushButton(QStringLiteral("Disabled"));
  disabled->setEnabled(false);

  QPushButton* withMenu = new QPushButton(QStringLiteral("Menu"));
  QMenu* buttonMenu = new QMenu(withMenu);
  buttonMenu->addAction(QStringLiteral("First"));
  buttonMenu->addAction(QStringLiteral("Second"));
  withMenu->setMenu(buttonMenu);

  layout->addWidget(primary, 0, 0);
  layout->addWidget(normal, 0, 1);
  layout->addWidget(checkable, 0, 2);
  layout->addWidget(flat, 1, 0);
  layout->addWidget(disabled, 1, 1);
  layout->addWidget(withMenu, 1, 2);
  return group;
}

QWidget* createInputsGroup() {
  QGroupBox* group = new QGroupBox(QStringLiteral("Inputs"));
  QFormLayout* layout = new QFormLayout(group);

  QLineEdit* name = new QLineEdit;
  name->setPlaceholderText(QStringLiteral("Your name"));
  QLineEdit* password = new QLineEdit(QStringLiteral("secret"));
  password->setEchoMode(QLineEdit::Password);
  password->setClearButtonEnabled(true);
  QLineEdit* disabled = new QLineEdit(QStringLiteral("Disabled"));
  disabled->setEnabled(false);

  QSpinBox* spin = new QSpinBox;
  spin->setRange(0, 100);
  spin->setValue(42);
  QDoubleSpinBox* doubleSpin = new QDoubleSpinBox;
  doubleSpin->setValue(3.14);
  QDateEdit* date = new QDateEdit(QDate::currentDate());
  date->setCalendarPopup(true);

  QComboBox* combo = new QComboBox;
  combo->addItems({QStringLiteral("Apple"), QStringLiteral("Banana"), QStringLiteral("Cherry")});
  QComboBox* editable = new QComboBox;
  editable->setEditable(true);
  editable->addItems({QStringLiteral("Editable"), QStringLiteral("Combo")});

  layout->addRow(QStringLiteral("Name"), name);
  layout->addRow(QStringLiteral("Password"), password);
  layout->addRow(QStringLiteral("Disabled"), disabled);
  layout->addRow(QStringLiteral("Integer"), spin);
  layout->addRow(QStringLiteral("Double"), doubleSpin);
  layout->addRow(QStringLiteral("Date"), date);
  layout->addRow(QStringLiteral("Fruit"), combo);
  layout->addRow(QStringLiteral("Editable"), editable);
  return group;
}

QWidget* createSelectionGroup() {
  QGroupBox* group = new QGroupBox(QStringLiteral("Selection"));
  QGridLayout* layout = new QGridLayout(group);

  QCheckBox* checked = new QCheckBox(QStringLiteral("Checked"));
  checked->setChecked(true);
  QCheckBox* unchecked = new QCheckBox(QStringLiteral("Unchecked"));
  QCheckBox* partial = new QCheckBox(QStringLiteral("Partial"));
  partial->setTristate(true);
  partial->setCheckState(Qt::PartiallyChecked);
  QCheckBox* disabled = new QCheckBox(QStringLiteral("Disabled"));
  disabled->setChecked(true);
  disabled->setEnabled(false);

  QRadioButton* first = new QRadioButton(QStringLiteral("Option A"));
  first->setChecked(true);
  QRadioButton* second = new QRadioButton(QStringLiteral("Option B"));
  QRadioButton* third = new QRadioButton(QStringLiteral("Option C"));
  third->setEnabled(false);

  layout->addWidget(checked, 0, 0);
  layout->addWidget(unchecked, 1, 0);
  layout->addWidget(partial, 2, 0);
  layout->addWidget(disabled, 3, 0);
  layout->addWidget(first, 0, 1);
  layout->addWidget(second, 1, 1);
  layout->addWidget(third, 2, 1);
  return group;
}

QWidget* createRangesGroup() {
  QGroupBox* group = new QGroupBox(QStringLiteral("Ranges"));
  QVBoxLayout* layout = new QVBoxLayout(group);

  QSlider* slider = new QSlider(Qt::Horizontal);
  slider->setValue(60);
  QSlider* ticks = new QSlider(Qt::Horizontal);
  ticks->setRange(0, 10);
  ticks->setValue(3);
  ticks->setTickPosition(QSlider::TicksBelow);

  QProgressBar* progress = new QProgressBar;
  progress->setValue(65);
  QObject::connect(slider, &QSlider::valueChanged, progress, &QProgressBar::setValue);
  QProgressBar* busy = new QProgressBar;
  busy->setRange(0, 0);

  layout->addWidget(slider);
  layout->addWidget(ticks);
  layout->addWidget(progress);
  layout->addWidget(busy);
  layout->addStretch();
  return group;
}

QWidget* createControlsPage() {
  QWidget* page = new QWidget;
  QGridLayout* layout = new QGridLayout(page);
  layout->addWidget(createButtonsGroup(), 0, 0);
  layout->addWidget(createSelectionGroup(), 1, 0);
  layout->addWidget(createRangesGroup(), 2, 0);
  layout->addWidget(createInputsGroup(), 0, 1, 3, 1);
  return page;
}

QWidget* createViewsPage() {
  QSplitter* splitter = new QSplitter;

  QTreeWidget* tree = new QTreeWidget;
  tree->setHeaderLabels({QStringLiteral("Name"), QStringLiteral("Size"), QStringLiteral("Kind")});
  for (int i = 0; i < 4; i++) {
    QTreeWidgetItem* folder =
        new QTreeWidgetItem(tree, {QStringLiteral("Folder %1").arg(i + 1), QString(), QStringLiteral("Folder")});
    for (int j = 0; j < 3; j++) {
      new QTreeWidgetItem(folder, {QStringLiteral("File %1.txt").arg(j + 1), QStringLiteral("%1 KB").arg((j + 1) * 12),
                                   QStringLiteral("Text")});
    }
  }
  tree->setColumnWidth(0, 160);
  tree->expandToDepth(0);
  tree->setCurrentItem(tree->topLevelItem(0)->child(1));
  tree->header()->setSortIndicatorShown(true);

  QListWidget* list = new QListWidget;
  for (int i = 0; i < 30; i++) {
    list->addItem(QStringLiteral("List item %1").arg(i + 1));
  }
  list->setCurrentRow(2);

  QTableWidget* table = new QTableWidget(12, 3);
  table->setHorizontalHeaderLabels({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")});
  for (int row = 0; row < table->rowCount(); row++) {
    for (int column = 0; column < table->columnCount(); column++) {
      table->setItem(row, column, new QTableWidgetItem(QString::number(row * column)));
    }
  }
  table->setAlternatingRowColors(true);

  splitter->addWidget(tree);
  splitter->addWidget(list);
  splitter->addWidget(table);
  return splitter;
}

QWidget* createTextPage() {
  QTextEdit* edit = new QTextEdit;
  edit->setHtml(QStringLiteral("<h2>ModernStyle</h2><p>A flat, rounded QProxyStyle for Qt 5 and Qt 6.</p>"
                               "<p>Try switching the theme and accent color from the toolbar.</p>"));
  return edit;
}

// Theme index follows the styles' Theme enums: System, Light, Dark
struct StyleControls {
  int m_theme;
  std::function<void(int)> m_setTheme;
  std::function<void(const QColor&)> m_setAccent;
  QList<QColor> m_accents;
};

class Gallery : public QMainWindow {
public:
  explicit Gallery(const StyleControls& controls) : m_controls(controls) {
    setWindowTitle(QStringLiteral("ModernStyle Gallery"));

    QMenu* fileMenu = menuBar()->addMenu(QStringLiteral("&File"));
    fileMenu->addAction(QStringLiteral("&New"))->setShortcut(QKeySequence::New);
    fileMenu->addAction(QStringLiteral("&Open..."))->setShortcut(QKeySequence::Open);
    QMenu* recent = fileMenu->addMenu(QStringLiteral("Open &Recent"));
    recent->addAction(QStringLiteral("report.txt"));
    recent->addAction(QStringLiteral("notes.md"));
    fileMenu->addSeparator();
    QAction* autosave = fileMenu->addAction(QStringLiteral("&Autosave"));
    autosave->setCheckable(true);
    autosave->setChecked(true);
    fileMenu->addSeparator();
    QAction* quit = fileMenu->addAction(QStringLiteral("&Quit"));
    quit->setShortcut(QKeySequence::Quit);
    connect(quit, &QAction::triggered, this, &QWidget::close);
    menuBar()->addMenu(QStringLiteral("&Edit"))->addAction(QStringLiteral("&Undo"));
    menuBar()->addMenu(QStringLiteral("&View"))->addAction(QStringLiteral("&Zoom In"));

    QToolBar* toolBar = addToolBar(QStringLiteral("Appearance"));
    toolBar->setMovable(false);
    toolBar->addWidget(new QLabel(QStringLiteral(" Theme ")));
    QComboBox* theme = new QComboBox;
    theme->addItems({QStringLiteral("System"), QStringLiteral("Light"), QStringLiteral("Dark")});
    theme->setCurrentIndex(controls.m_theme);
    connect(theme, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) { m_controls.m_setTheme(index); });
    toolBar->addWidget(theme);
    toolBar->addSeparator();

    for (const QColor& accent : controls.m_accents) {
      QAction* action = toolBar->addAction(swatchIcon(accent), accent.name());
      connect(action, &QAction::triggered, this, [this, accent] { m_controls.m_setAccent(accent); });
    }

    QTabWidget* tabs = new QTabWidget;
    tabs->addTab(createControlsPage(), QStringLiteral("Controls"));
    tabs->addTab(createViewsPage(), QStringLiteral("Views"));
    tabs->addTab(createTextPage(), QStringLiteral("Text"));
    setCentralWidget(tabs);
    m_pTabs = tabs;

    statusBar()->showMessage(QStringLiteral("Ready"));
  }

  QTabWidget* tabs() const { return m_pTabs; }

private:
  StyleControls m_controls;
  QTabWidget* m_pTabs = nullptr;
};

} // namespace

int main(int argc, char* argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
  QApplication app(argc, argv);

  QCommandLineParser parser;
  parser.addHelpOption();
  QCommandLineOption themeOption(QStringLiteral("theme"), QStringLiteral("system, light or dark."),
                                 QStringLiteral("theme"), QStringLiteral("system"));
  QCommandLineOption styleOption(QStringLiteral("style-name"), QStringLiteral("modern, winui or windows10."),
                                 QStringLiteral("name"), QStringLiteral("modern"));
  QCommandLineOption screenshotOption(QStringLiteral("screenshot"),
                                      QStringLiteral("Save each gallery tab as <prefix>-<n>.png and exit."),
                                      QStringLiteral("prefix"));
  parser.addOption(themeOption);
  parser.addOption(styleOption);
  parser.addOption(screenshotOption);
  parser.process(app);

  StyleControls controls;
  controls.m_theme = 0;
  const QString themeName = parser.value(themeOption).toLower();
  if (themeName == QLatin1String("light")) {
    controls.m_theme = 1;
  } else if (themeName == QLatin1String("dark")) {
    controls.m_theme = 2;
  }

  const QString styleName = parser.value(styleOption).toLower();
  if (styleName == QLatin1String("windows10")) {
    Windows10Style* style = new Windows10Style(Windows10Style::Theme(controls.m_theme));
    QApplication::setStyle(style);
    QApplication::setFont(Windows10Style::recommendedFont());
    controls.m_setTheme = [style](int theme) { style->setTheme(Windows10Style::Theme(theme)); };
    controls.m_setAccent = [style](const QColor& accent) { style->setAccentColor(accent); };
    controls.m_accents = {QColor(0x00, 0x78, 0xD7), QColor(0x88, 0x17, 0x98), QColor(0x10, 0x7C, 0x10),
                          QColor(0xCA, 0x50, 0x10), QColor(0xE8, 0x11, 0x23)};
  } else if (styleName == QLatin1String("winui")) {
    WinUIStyle* style = new WinUIStyle(WinUIStyle::Theme(controls.m_theme));
    QApplication::setStyle(style);
    QApplication::setFont(WinUIStyle::recommendedFont());
    controls.m_setTheme = [style](int theme) { style->setTheme(WinUIStyle::Theme(theme)); };
    controls.m_setAccent = [style](const QColor& accent) { style->setAccentColor(accent); };
    controls.m_accents = {QColor(0x00, 0x78, 0xD4), QColor(0x88, 0x17, 0x98), QColor(0x10, 0x7C, 0x10),
                          QColor(0xCA, 0x50, 0x10), QColor(0xE8, 0x11, 0x23)};
  } else {
    ModernStyle* style = new ModernStyle(ModernStyle::Theme(controls.m_theme));
    QApplication::setStyle(style);
    controls.m_setTheme = [style](int theme) { style->setTheme(ModernStyle::Theme(theme)); };
    controls.m_setAccent = [style](const QColor& accent) { style->setAccentColor(accent); };
    controls.m_accents = {QColor(0x35, 0x84, 0xE4), QColor(0x8B, 0x5C, 0xF6), QColor(0x10, 0xB9, 0x81),
                          QColor(0xF5, 0x9E, 0x0B), QColor(0xEF, 0x44, 0x44)};
  }

  Gallery gallery(controls);
  gallery.resize(980, 680);
  gallery.show();

  if (parser.isSet(screenshotOption)) {
    const QString prefix = parser.value(screenshotOption);
    QTimer::singleShot(300, &gallery, [&gallery, prefix] {
      for (int i = 0; i < gallery.tabs()->count(); i++) {
        gallery.tabs()->setCurrentIndex(i);
        gallery.grab().save(QStringLiteral("%1-%2.png").arg(prefix).arg(i));
      }
      QApplication::quit();
    });
  }
  return app.exec();
}

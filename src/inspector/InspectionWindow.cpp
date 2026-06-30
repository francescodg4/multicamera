#include "inspector/InspectionWindow.hpp"

#include "Icons.hpp"
#include "ThemeRegistry.hpp"
#include "WidgetStyle.hpp"
#include "inspector/CameraGrid.hpp"
#include "inspector/CameraPipeline.hpp"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QStatusBar>

namespace {

QPushButton* transportButton(const QString& text, const QString& tip)
{
    auto* button = new QPushButton(text);
    button->setToolTip(tip);
    button->setFocusPolicy(Qt::NoFocus); // keys stay with the window's shortcuts (Space, E, Q...)
    return button;
}

} // namespace

InspectionWindow::InspectionWindow(const QList<CameraSource>& sources, QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Multicamera inspection"));

    for (const CameraSource& source : sources) {
        auto* camera = new CameraPipeline(source, this);
        connect(camera, &CameraPipeline::errorOccurred, this, [this](const QString& message) { statusBar()->showMessage(message, 10000); });
        m_cameras << camera;
    }

    createActions();
    createMenus();

    // ---- control room over the board (each camera carries its own controls)
    auto* central = new QWidget;
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(10);
    m_grid = new CameraGrid(m_cameras);
    m_controlRoom = controlBox();
    m_controlRoom->setObjectName(QStringLiteral("controlRoom"));
    layout->addWidget(m_controlRoom);
    layout->addWidget(m_grid, 1);
    setCentralWidget(central);

    // a single camera gets the whole window: the controls for every camera step aside
    connect(m_grid, &CameraGrid::maximizedChanged, this, [this](int index) {
        m_controlRoom->setVisible(index < 0);
        m_maximizeAction->setChecked(index >= 0);
    });

    connect(m_grid, &CameraGrid::cameraClicked, this, &InspectionWindow::select);
    connect(m_grid, &CameraGrid::cameraDoubleClicked, this, [this](int index) {
        select(index);
        toggleMaximized(index);
    });

    statusBar()->setSizeGripEnabled(false);
    auto* engine = new QLabel(tr("%1  ·  %n camera(s)", nullptr, int(m_cameras.size())).arg(CameraPipeline::version()));
    statusBar()->addPermanentWidget(engine);

    select(-1);
    resize(1440, 960);
}

CameraPipeline* InspectionWindow::current() const
{
    return m_current >= 0 ? m_cameras[m_current] : nullptr;
}

// ---- actions & menus ----------------------------------------------------------------------------

void InspectionWindow::createActions()
{
    m_playAllAction = new QAction(tr("&Play all"), this);
    m_playAllAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_P));
    connect(m_playAllAction, &QAction::triggered, this, &InspectionWindow::playAll);
    m_pauseAllAction = new QAction(tr("P&ause all"), this);
    m_pauseAllAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P));
    connect(m_pauseAllAction, &QAction::triggered, this, &InspectionWindow::pauseAll);
    m_stopAllAction = new QAction(tr("&Stop all"), this);
    m_stopAllAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Period));
    connect(m_stopAllAction, &QAction::triggered, this, &InspectionWindow::stopAll);

    // on the camera under inspection
    m_playAction = new QAction(tr("&Play / pause"), this);
    m_playAction->setShortcut(QKeySequence(Qt::Key_Space));
    connect(m_playAction, &QAction::triggered, this, [this] {
        if (CameraPipeline* camera = current()) {
            camera->togglePlay();
        }
    });
    m_stopAction = new QAction(tr("&Stop"), this);
    m_stopAction->setShortcut(QKeySequence(Qt::Key_S));
    connect(m_stopAction, &QAction::triggered, this, [this] {
        if (CameraPipeline* camera = current()) {
            camera->stop();
        }
    });
    m_nextAction = new QAction(tr("&Next frame"), this);
    m_nextAction->setShortcut(QKeySequence(Qt::Key_E));
    m_nextAction->setAutoRepeat(true);
    connect(m_nextAction, &QAction::triggered, this, [this] {
        if (CameraPipeline* camera = current()) {
            camera->stepForward();
        }
    });
    m_previousAction = new QAction(tr("P&revious frame"), this);
    m_previousAction->setShortcut(QKeySequence(Qt::Key_Q));
    m_previousAction->setAutoRepeat(true);
    connect(m_previousAction, &QAction::triggered, this, [this] {
        if (CameraPipeline* camera = current()) {
            camera->stepBackward();
        }
    });
    m_startAction = new QAction(tr("Go to s&tart"), this);
    m_startAction->setShortcut(QKeySequence(Qt::Key_Home));
    connect(m_startAction, &QAction::triggered, this, [this] {
        if (CameraPipeline* camera = current()) {
            camera->seek(0);
        }
    });
    m_maximizeAction = new QAction(tr("&Maximise camera"), this);
    m_maximizeAction->setShortcut(QKeySequence(Qt::Key_F));
    m_maximizeAction->setCheckable(true);
    connect(m_maximizeAction, &QAction::triggered, this, [this] { toggleMaximized(m_current); });
}

void InspectionWindow::createMenus()
{
    QMenu* file = menuBar()->addMenu(tr("&File"));
    QAction* quit = file->addAction(tr("&Quit"));
    quit->setShortcut(QKeySequence::Quit);
    connect(quit, &QAction::triggered, qApp, &QApplication::closeAllWindows);

    QMenu* cameras = menuBar()->addMenu(tr("&Cameras"));
    cameras->addAction(m_playAllAction);
    cameras->addAction(m_pauseAllAction);
    cameras->addAction(m_stopAllAction);
    cameras->addSeparator();
    QMenu* inspect = cameras->addMenu(tr("&Inspect"));
    for (int i = 0; i < m_cameras.size(); ++i) {
        const CameraSource& source = m_cameras[i]->source();
        QAction* action = inspect->addAction(QStringLiteral("%1 — %2").arg(source.name, source.label));
        if (i < 9) {
            action->setShortcut(QKeySequence(Qt::Key_1 + i));
        }
        connect(action, &QAction::triggered, this, [this, i] { select(i); });
    }
    QAction* none = inspect->addAction(tr("&None"));
    none->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(none, &QAction::triggered, this, [this] {
        m_grid->setMaximized(-1);
        select(-1);
    });

    QMenu* playback = menuBar()->addMenu(tr("&Playback"));
    playback->addAction(m_playAction);
    playback->addAction(m_stopAction);
    playback->addSeparator();
    playback->addAction(m_previousAction);
    playback->addAction(m_nextAction);
    playback->addAction(m_startAction);

    QMenu* view = menuBar()->addMenu(tr("&View"));
    view->addAction(m_maximizeAction);

    QMenu* interfaces = menuBar()->addMenu(tr("&Interface"));
    m_themeActions = new QActionGroup(this);
    int n = 0;
    for (const ThemeEntry& theme : Themes::all()) {
        QAction* action = interfaces->addAction(theme.name);
        action->setCheckable(true);
        action->setData(theme.id);
        action->setShortcut(QKeySequence(Qt::CTRL | (Qt::Key_1 + n++)));
        m_themeActions->addAction(action);
        connect(action, &QAction::triggered, this, [this, id = theme.id] { chooseTheme(id); });
    }
    interfaces->addSeparator();
    m_darkAction = interfaces->addAction(tr("&Dark mode"));
    m_darkAction->setCheckable(true);
    m_darkAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    connect(m_darkAction, &QAction::triggered, this, &InspectionWindow::chooseDark);

    QMenu* help = menuBar()->addMenu(tr("&Help"));
    connect(help->addAction(tr("&Keyboard")), &QAction::triggered, this, [this] {
        QMessageBox::information(this, tr("Keyboard"),
            tr("<table cellspacing=4>"
               "<tr><td><b>1</b> … <b>9</b></td><td>inspect a camera (or click it)</td></tr>"
               "<tr><td><b>Esc</b></td><td>inspect none, show every camera</td></tr>"
               "<tr><td><b>Space</b></td><td>play / pause</td></tr>"
               "<tr><td><b>E</b></td><td>next frame (hold to repeat)</td></tr>"
               "<tr><td><b>Q</b></td><td>previous frame</td></tr>"
               "<tr><td><b>Home</b></td><td>go to the start</td></tr>"
               "<tr><td><b>S</b></td><td>stop</td></tr>"
               "<tr><td><b>F</b></td><td>maximise the camera (or double click it)</td></tr>"
               "<tr><td><b>Ctrl+P</b> / <b>Ctrl+Shift+P</b> / <b>Ctrl+.</b></td><td>play / pause / stop all</td></tr>"
               "<tr><td><b>Ctrl+1</b> … <b>Ctrl+3</b></td><td>interface design</td></tr>"
               "<tr><td><b>Ctrl+D</b></td><td>dark / light mode</td></tr>"
               "</table>"));
    });
    connect(help->addAction(tr("&About")), &QAction::triggered, this, [this] {
        QMessageBox::about(this, tr("Multicamera inspection"),
            tr("<p>An array of cameras on a GStreamer backbone: every camera is a pipeline decoding "
               "into an appsink.</p><p>Click a camera to inspect it: its controls show under its picture. "
               "Play it, move along its time bar, and step through it picture by picture with <b>E</b> "
               "(and <b>Q</b> back).</p><p>%1</p>")
                .arg(CameraPipeline::version()));
    });
}

// ---- control room -------------------------------------------------------------------------------

QWidget* InspectionWindow::controlBox()
{
    auto* box = new QGroupBox(tr("Control room"));

    m_playAllButton = transportButton(tr("Play all"), tr("Start every camera (Ctrl+P)"));
    connect(m_playAllButton, &QPushButton::clicked, this, &InspectionWindow::playAll);
    m_pauseAllButton = transportButton(tr("Pause all"), tr("Freeze every camera (Ctrl+Shift+P)"));
    connect(m_pauseAllButton, &QPushButton::clicked, this, &InspectionWindow::pauseAll);
    m_stopAllButton = transportButton(tr("Stop all"), tr("Stop every camera (Ctrl+.)"));
    connect(m_stopAllButton, &QPushButton::clicked, this, &InspectionWindow::stopAll);

    m_layout = new QComboBox;
    m_layout->addItem(tr("Automatic"), 0);
    for (int c = 1; c <= 4; ++c) {
        m_layout->addItem(tr("%n column(s)", nullptr, c), c);
    }
    m_layout->setToolTip(tr("Columns of the camera board"));
    connect(m_layout, &QComboBox::currentIndexChanged, this, [this] { m_grid->setColumns(m_layout->currentData().toInt()); });

    m_loop = new QCheckBox(tr("Loop"));
    m_loop->setChecked(true);
    m_loop->setToolTip(tr("At the end, a camera starts again (otherwise it pauses on its last picture)"));
    connect(m_loop, &QCheckBox::toggled, this, [this](bool on) {
        for (CameraPipeline* camera : std::as_const(m_cameras)) {
            camera->setLoop(on);
        }
    });

    m_themeSelector = new QComboBox;
    for (const ThemeEntry& theme : Themes::all()) {
        m_themeSelector->addItem(theme.name, theme.id);
        m_themeSelector->setItemData(m_themeSelector->count() - 1, theme.description, Qt::ToolTipRole);
    }
    m_themeSelector->setToolTip(tr("Interface design (Ctrl+1, 2, 3)"));
    connect(m_themeSelector, &QComboBox::currentIndexChanged, this, [this] { chooseTheme(m_themeSelector->currentData().toString()); });

    m_darkBox = new QCheckBox(tr("Dark"));
    m_darkBox->setToolTip(tr("Dark or light mode, for the designs that have both (Ctrl+D)"));
    connect(m_darkBox, &QCheckBox::toggled, this, &InspectionWindow::chooseDark);

    auto* row = new QHBoxLayout(box);
    row->addWidget(m_playAllButton);
    row->addWidget(m_pauseAllButton);
    row->addWidget(m_stopAllButton);
    row->addSpacing(16);
    row->addWidget(new QLabel(tr("Layout")));
    row->addWidget(m_layout);
    row->addSpacing(8);
    row->addWidget(m_loop);
    row->addStretch(1);
    row->addWidget(new QLabel(tr("Design")));
    row->addWidget(m_themeSelector);
    row->addWidget(m_darkBox);
    return box;
}

void InspectionWindow::playAll()
{
    for (CameraPipeline* camera : std::as_const(m_cameras)) {
        camera->play();
    }
}

void InspectionWindow::pauseAll()
{
    for (CameraPipeline* camera : std::as_const(m_cameras)) {
        if (camera->state() == CameraPipeline::State::Playing) {
            camera->pause();
        }
    }
}

void InspectionWindow::stopAll()
{
    for (CameraPipeline* camera : std::as_const(m_cameras)) {
        camera->stop();
    }
}

void InspectionWindow::toggleMaximized(int index)
{
    if (index < 0) {
        return;
    }
    m_grid->setMaximized(m_grid->maximized() == index ? -1 : index);
}

// ---- inspection ---------------------------------------------------------------------------------

void InspectionWindow::select(int index)
{
    m_current = index >= 0 && index < m_cameras.size() ? index : -1;
    m_grid->setSelected(m_current); // the selected card shows its controls, the others hide theirs
    if (m_grid->maximized() >= 0 && m_current >= 0 && m_grid->maximized() != m_current) {
        m_grid->setMaximized(m_current); // the maximised view follows the selection
    }

    CameraPipeline* camera = current();
    for (QAction* action : { m_playAction, m_stopAction, m_previousAction, m_nextAction, m_startAction, m_maximizeAction }) {
        action->setEnabled(camera);
    }
    if (camera) {
        statusBar()->showMessage(tr("Inspecting %1: Space plays or pauses, E steps one frame forward, Q one back").arg(camera->source().name), 8000);
    }
}

// ---- interface ----------------------------------------------------------------------------------

void InspectionWindow::setTheme(const QString& id, bool dark)
{
    const ThemeEntry* theme = Themes::find(id);
    if (!theme || !Themes::apply(id, dark)) {
        return;
    }
    m_theme = id;
    m_dark = dark;
    {
        const QSignalBlocker block(m_themeSelector);
        m_themeSelector->setCurrentIndex(m_themeSelector->findData(id));
    }
    for (QAction* action : m_themeActions->actions()) {
        action->setChecked(action->data().toString() == id);
    }
    {
        const QSignalBlocker block(m_darkBox);
        m_darkBox->setChecked(dark);
    }
    m_darkAction->setChecked(dark);
    m_darkBox->setEnabled(theme->modes);
    m_darkAction->setEnabled(theme->modes);
    statusBar()->showMessage(theme->modes ? tr("Interface: %1, %2 mode").arg(theme->name, dark ? tr("dark") : tr("light")) : tr("Interface: %1").arg(theme->name), 4000);
    tintIcons();
    update();
}

void InspectionWindow::chooseTheme(const QString& id)
{
    setTheme(id, m_dark);
    Themes::save(id);
}

void InspectionWindow::chooseDark(bool dark)
{
    setTheme(m_theme, dark);
    Themes::saveDark(dark);
}

void InspectionWindow::tintIcons()
{
    const QColor ink = QApplication::palette().color(QPalette::ButtonText);
    m_playAllButton->setIcon(Icons::icon(Icon::Play, ink));
    m_pauseAllButton->setIcon(Icons::icon(Icon::Pause, ink));
    m_stopAllButton->setIcon(Icons::icon(Icon::Stop, ink));
}

void InspectionWindow::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    if (const auto* style = qobject_cast<const WidgetStyle*>(this->style())) {
        style->window(p, this, rect());
    } else {
        p.fillRect(rect(), palette().window());
    }
}

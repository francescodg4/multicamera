#pragma once

#include <QPixmap>
#include <QWidget>

class CameraControls;
class CameraPipeline;

/// One camera on the board: its pictures on the screen of a card drawn by the interface design,
/// with a status lamp, the time and picture number of what is shown. Click selects the camera,
/// double click maximises it.
///
/// The selected camera shows its player controls over the bottom of the card; the screen
/// shrinks to make room for them, so they never cover the picture.
class CameraTile : public QWidget {
    Q_OBJECT
public:
    /// Camera number @p index (from 0) of the board.
    CameraTile(CameraPipeline* camera, int index, QWidget* parent = nullptr);

    CameraPipeline* camera() const { return m_camera; }
    CameraControls* controls() const { return m_controls; }

    bool isSelected() const { return m_selected; }
    /// Selects the camera: its controls show, and hide again when it is deselected.
    void setSelected(bool selected);
    /// Whether a selected camera wears the design's selection mark (not when it is alone on the board).
    void setMarkVisible(bool visible);
    /// Draws the card again, e.g. for a new selection mark colour.
    void refreshChrome();

    /// The display on the card, where the pictures go.
    QRect screenRect() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void clicked();
    void doubleClicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void changeEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    QRect cardRect() const; ///< the card: the whole tile, so neighbouring cards touch
    QRect controlsRect() const; ///< the bottom band of the card, for the controls
    void layoutControls();
    void paintChrome(); ///< card, display and selection mark, cached in m_chrome
    void paintOverlay(QPainter& p, const QRect& screen);

    CameraPipeline* m_camera;
    CameraControls* m_controls;
    int m_index; ///< place on the board: designs may colour each camera's card
    bool m_selected = false;
    bool m_markVisible = true;
    QPixmap m_chrome;
};

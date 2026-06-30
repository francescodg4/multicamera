#pragma once

#include <QList>
#include <QWidget>

class CameraPipeline;
class CameraTile;

/// The board of cameras: their tiles in a grid (automatic or with a chosen number of columns),
/// one of them selected, or one maximised over the whole board.
class CameraGrid : public QWidget {
    Q_OBJECT
public:
    explicit CameraGrid(const QList<CameraPipeline*>& cameras, QWidget* parent = nullptr);

    const QList<CameraTile*>& tiles() const { return m_tiles; }

    int columns() const { return m_columns; }
    /// Columns of the grid; 0 picks a near-square grid for the number of cameras.
    void setColumns(int columns);

    int selected() const { return m_selected; }
    void setSelected(int index); ///< -1: none

    int maximized() const { return m_maximized; }
    void setMaximized(int index); ///< -1: every camera

signals:
    void cameraClicked(int index);
    void cameraDoubleClicked(int index);
    void maximizedChanged(int index); ///< -1: back to every camera

private:
    void relayout();

    QList<CameraTile*> m_tiles;
    int m_columns = 0;
    int m_selected = -1;
    int m_maximized = -1;
};

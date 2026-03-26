#include "inspector/CameraGrid.hpp"

#include "inspector/CameraTile.hpp"

#include <QGridLayout>

#include <cmath>

CameraGrid::CameraGrid(const QList<CameraPipeline*>& cameras, QWidget* parent)
    : QWidget(parent)
{
    for (int i = 0; i < cameras.size(); ++i) {
        auto* tile = new CameraTile(cameras[i], this);
        connect(tile, &CameraTile::clicked, this, [this, i] { emit cameraClicked(i); });
        connect(tile, &CameraTile::doubleClicked, this, [this, i] { emit cameraDoubleClicked(i); });
        m_tiles << tile;
    }
    relayout();
}

void CameraGrid::setColumns(int columns)
{
    if (m_columns != columns) {
        m_columns = std::max(0, columns);
        relayout();
    }
}

void CameraGrid::setSelected(int index)
{
    m_selected = index >= 0 && index < m_tiles.size() ? index : -1;
    for (int i = 0; i < m_tiles.size(); ++i) {
        m_tiles[i]->setSelected(i == m_selected);
    }
}

void CameraGrid::setMaximized(int index)
{
    index = index >= 0 && index < m_tiles.size() ? index : -1;
    if (m_maximized != index) {
        m_maximized = index;
        relayout();
    }
}

void CameraGrid::relayout()
{
    delete layout(); // the tiles stay: they are children of the board, not of the layout
    auto* grid = new QGridLayout(this);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(4); // the tiles keep room round their cards for the selection mark

    if (m_maximized >= 0) {
        for (int i = 0; i < m_tiles.size(); ++i) {
            m_tiles[i]->setVisible(i == m_maximized);
        }
        grid->addWidget(m_tiles[m_maximized], 0, 0);
        return;
    }

    const int count = int(m_tiles.size());
    const int columns = m_columns > 0 ? m_columns : std::max(1, int(std::ceil(std::sqrt(double(count)))));
    for (int i = 0; i < count; ++i) {
        grid->addWidget(m_tiles[i], i / columns, i % columns);
        m_tiles[i]->setVisible(true);
    }
    for (int c = 0; c < columns; ++c) {
        grid->setColumnStretch(c, 1);
    }
    for (int r = 0; r < (count + columns - 1) / columns; ++r) {
        grid->setRowStretch(r, 1);
    }
}

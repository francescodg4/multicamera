#include "Icons.hpp"

#include <QGuiApplication>
#include <QPainter>

#include <algorithm>

namespace Icons {

QPainterPath svgPath(QStringView d)
{
    QPainterPath path;
    QPointF cur, start, lastCtrl;
    QChar cmd, prev;
    qsizetype i = 0;
    const qsizetype n = d.size();

    auto skip = [&] {
        while (i < n && (d[i].isSpace() || d[i] == u',')) {
            ++i;
        }
    };
    auto num = [&]() -> qreal {
        skip();
        const qsizetype s = i;
        if (i < n && (d[i] == u'-' || d[i] == u'+')) {
            ++i;
        }
        bool dot = false;
        while (i < n && (d[i].isDigit() || (d[i] == u'.' && !dot))) {
            dot = dot || d[i] == u'.';
            ++i;
        }
        return d.mid(s, i - s).toDouble();
    };
    auto pt = [&]() {
        const qreal x = num();
        const qreal y = num();
        return QPointF(x, y);
    };

    for (;;) {
        skip();
        if (i >= n) {
            break;
        }
        if (d[i].isLetter()) {
            cmd = d[i++];
        } else if (cmd.toUpper() == u'Z' || cmd.isNull()) {
            break; // stray number: malformed data
        } else if (cmd == u'M') {
            cmd = u'L'; // coordinates after a moveto are implicit linetos
        } else if (cmd == u'm') {
            cmd = u'l';
        }
        const QPointF o = cmd.isLower() ? cur : QPointF();
        const QChar c = cmd.toUpper();
        if (c == u'M') {
            cur = start = o + pt();
            path.moveTo(cur);
        } else if (c == u'L') {
            cur = o + pt();
            path.lineTo(cur);
        } else if (c == u'H') {
            cur.setX(o.x() + num());
            path.lineTo(cur);
        } else if (c == u'V') {
            cur.setY(o.y() + num());
            path.lineTo(cur);
        } else if (c == u'C' || c == u'S') {
            const QPointF c1 = c == u'C' ? o + pt() : (prev == u'C' || prev == u'S' ? 2 * cur - lastCtrl : cur);
            const QPointF c2 = o + pt();
            const QPointF e = o + pt();
            path.cubicTo(c1, c2, e);
            lastCtrl = c2;
            cur = e;
        } else if (c == u'Q' || c == u'T') {
            const QPointF c1 = c == u'Q' ? o + pt() : (prev == u'Q' || prev == u'T' ? 2 * cur - lastCtrl : cur);
            const QPointF e = o + pt();
            path.quadTo(c1, e);
            lastCtrl = c1;
            cur = e;
        } else if (c == u'Z') {
            path.closeSubpath();
            cur = start;
        } else {
            break; // unsupported command (e.g. arcs)
        }
        prev = c;
    }
    return path;
}

namespace {

    QColor hex(const char* name, qreal alpha = 1.0)
    {
        QColor c = QColor::fromString(QLatin1String(name));
        c.setAlphaF(float(alpha));
        return c;
    }

    QPainterPath circle(qreal cx, qreal cy, qreal r)
    {
        QPainterPath p;
        p.addEllipse(QPointF(cx, cy), r, r);
        return p;
    }

    QPainterPath rect(qreal x, qreal y, qreal w, qreal h, qreal radius = 0)
    {
        QPainterPath p;
        p.addRoundedRect(QRectF(x, y, w, h), radius, radius);
        return p;
    }

    /// Small drawing vocabulary shared by the icon definitions.
    struct Canvas {
        QPainter& p;
        QColor c; ///< the icon colour passed by the caller

        void fill(const QPainterPath& path, const QColor& col) { p.fillPath(path, col); }
        void fill(QStringView d, const QColor& col) { fill(svgPath(d), col); }
        void stroke(const QPainterPath& path, const QColor& col, qreal w)
        {
            p.strokePath(path, QPen(col, w, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        }
        void stroke(QStringView d, const QColor& col, qreal w) { stroke(svgPath(d), col, w); }
    };

    struct Definition {
        QSizeF box; ///< view box of the path data
        void (*draw)(Canvas&);
    };

    Definition definition(Icon icon)
    {
        switch (icon) {
        // ---- tab bar -----------------------------------------------------------------------
        case Icon::Person:
            return { { 24, 24 }, [](Canvas& c) {
                        c.fill(circle(12, 7.5, 4), c.c);
                        c.fill(u"M3.5 21c.6-5 4-7.5 8.5-7.5s7.9 2.5 8.5 7.5z", c.c);
                    } };
        case Icon::Wrench:
            return { { 24, 24 }, [](Canvas& c) {
                        const QPainterPath slot = QTransform().translate(16.5, 7.5).rotate(45).map(rect(-1.7, -9, 3.4, 9));
                        c.fill(circle(16.5, 7.5, 5.5).subtracted(slot), c.c);
                        c.stroke(u"M5 19L13 11", c.c, 4.2);
                    } };
        case Icon::Pin:
            return { { 24, 24 }, [](Canvas& c) {
                        const QPainterPath pin = svgPath(u"M12 2C8.1 2 5 5.1 5 9c0 5 7 13 7 13s7-8 7-13c0-3.9-3.1-7-7-7Z");
                        c.fill(pin.subtracted(circle(12, 9, 2.6)), c.c);
                    } };
        case Icon::Image:
            return { { 24, 24 }, [](Canvas& c) {
                        c.fill(rect(3, 4, 18, 16).subtracted(rect(5, 6, 14, 12)), c.c);
                        c.fill(u"M5 18L10 12.5L13 15.5L16 11.5L19 15.5V18Z", c.c);
                        c.fill(circle(9, 9.5, 1.7), c.c);
                    } };
        case Icon::Doc:
            return { { 24, 24 }, [](Canvas& c) {
                        QPainterPath page = rect(4, 3, 16, 18, 1.5);
                        for (const qreal y : { 6.5, 10.0, 13.5 }) {
                            page = page.subtracted(rect(7, y, 10, 1.5));
                        }
                        c.fill(page.subtracted(rect(7, 17, 6, 1.5)), c.c);
                    } };
        case Icon::Book:
            return { { 24, 24 }, [](Canvas& c) {
                        c.fill(u"M2 5c3-1.5 5.5-1.5 8 .6V20C7.5 18 5 18 2 19.5zM22 5c-3-1.5-5.5-1.5-8 .6V20c2.5-2 5-2 8-.5z", c.c);
                    } };

        // ---- interface glyphs ----------------------------------------------------------------
        case Icon::Eye:
            return { { 24, 24 }, [](Canvas& c) {
                        c.stroke(u"M1.5 12S5.5 5 12 5s10.5 7 10.5 7-4 7-10.5 7S1.5 12 1.5 12z", c.c, 1.8);
                        c.fill(circle(12, 12, 3.2), c.c);
                    } };
        case Icon::EyeOff:
            return { { 24, 24 }, [](Canvas& c) {
                        c.stroke(u"M1.5 12S5.5 5 12 5s10.5 7 10.5 7-4 7-10.5 7S1.5 12 1.5 12z", c.c, 1.8);
                        c.fill(circle(12, 12, 3.2), c.c);
                        c.stroke(u"M3 3L21 21", c.c, 2);
                    } };
        case Icon::Play:
            return { { 24, 24 }, [](Canvas& c) { c.stroke(u"M8 5v14l11-7z", c.c, 1.8); } };
        case Icon::Pause:
            return { { 24, 24 }, [](Canvas& c) {
                        c.fill(rect(6.5, 5, 4, 14, 1.2), c.c);
                        c.fill(rect(13.5, 5, 4, 14, 1.2), c.c);
                    } };
        case Icon::Stop:
            return { { 24, 24 }, [](Canvas& c) { c.fill(rect(7, 7, 10, 10, 1.5), c.c); } };
        case Icon::FramePrevious:
            return { { 24, 24 }, [](Canvas& c) {
                        c.stroke(u"M18 6v12l-9-6z", c.c, 1.8);
                        c.fill(rect(5, 6, 2.6, 12, 1), c.c);
                    } };
        case Icon::FrameNext:
            return { { 24, 24 }, [](Canvas& c) {
                        c.stroke(u"M6 6v12l9-6z", c.c, 1.8);
                        c.fill(rect(16.4, 6, 2.6, 12, 1), c.c);
                    } };
        case Icon::ChevronRight:
            return { { 24, 24 }, [](Canvas& c) { c.stroke(u"M9 5l7 7-7 7", c.c, 2.4); } };
        case Icon::ChevronLeft:
            return { { 24, 24 }, [](Canvas& c) { c.stroke(u"M15 5l-7 7 7 7", c.c, 2.4); } };
        case Icon::Close:
            return { { 24, 24 }, [](Canvas& c) { c.stroke(u"M6 6l12 12M18 6L6 18", c.c, 2.4); } };
        case Icon::Plus:
            return { { 24, 24 }, [](Canvas& c) { c.stroke(u"M12 5v14M5 12h14", c.c, 2.4); } };
        case Icon::Trash:
            return { { 24, 24 }, [](Canvas& c) { c.stroke(u"M4 7h16M9 7V4h6v3M6 7l1 13h10l1-13", c.c, 1.8); } };
        case Icon::LogDoc:
            return { { 24, 32 }, [](Canvas& c) {
                        c.stroke(u"M2 1h14l6 6v24H2z", c.c, 1.6);
                        c.stroke(u"M5 9h8M5 13h14M5 17h14M5 21h14M5 25h10", c.c, 1.6);
                    } };

        // ---- items -----------------------------------------------------------------------------
        case Icon::Titanium:
            return { { 64, 64 }, [](Canvas& c) {
                        c.fill(u"M14 36l8-14 12 2 6 12-8 12-14-2z", hex("#1c1f26"));
                        c.fill(u"M34 24l12-6 10 10-4 12-12-4z", hex("#272b34"));
                        c.fill(u"M22 22l12 2-6 8z", hex("#5b6475"));
                        c.fill(u"M46 18l10 10-8-2z", hex("#6c7689"));
                        c.fill(u"M26 46l6-10 8 0-8 12z", hex("#3a404d"));
                    } };
        case Icon::Ingot:
            return { { 64, 64 }, [](Canvas& c) {
                        c.fill(u"M8 38l30-10 18 8-30 10z", hex("#8d98a8"));
                        c.fill(u"M8 38v8l18 8v-8z", hex("#5d6776"));
                        c.fill(u"M26 46v8l30-10v-8z", hex("#737e8e"));
                        c.fill(u"M14 30l30-10 14 6-30 10z", hex("#b6c0cc"));
                        c.fill(u"M14 30v6l14 6v-6z", hex("#7c8696"));
                        c.fill(u"M28 36v6l30-10v-6z", hex("#96a1b0"));
                    } };
        case Icon::Mesh:
            return { { 64, 64 }, [](Canvas& c) {
                        c.fill(rect(10, 20, 44, 24, 12), hex("#3b4250"));
                        c.fill(u"M50 20C53.3 20 56 25.4 56 32S53.3 44 50 44 44 38.6 44 32 46.7 20 50 20Z", hex("#5d6778"));
                        c.stroke(u"M16 21v22M22 20v24M28 20v24M34 20v24M40 20v24", hex("#8d97a8"), 1.5);
                    } };
        case Icon::Glass:
            return { { 64, 64 }, [](Canvas& c) {
                        c.fill(u"M14 40l10-18 10 20z", hex("#2a6fd6"));
                        c.fill(u"M28 26l14-10 8 18-14 10z", hex("#3fa0ff"));
                        c.fill(u"M36 44l14-6 4 10-14 4z", hex("#1c4fa8"));
                        c.fill(u"M28 26l14-10 2 6z", hex("#b8e4ff"));
                    } };
        case Icon::Bottle:
            return { { 64, 64 }, [](Canvas& c) {
                        c.fill(rect(27, 8, 10, 8), hex("#d8dde4"));
                        c.fill(u"M26 16h12v8l8 8v24H18V32l8-8z", hex("#e6f2ff", 0.85));
                        c.fill(u"M19 38h26v17H19z", c.c);
                        c.fill(rect(24, 36, 16, 12), hex("#ffd23a"));
                        c.fill(u"M32 39l5 7h-10z", hex("#1d1d1d"));
                    } };
        case Icon::Drop:
            return { { 64, 64 }, [](Canvas& c) {
                        c.fill(u"M32 8C24 24 18 32 18 40C18 47.7 24.3 54 32 54S46 47.7 46 40C46 32 40 24 32 8Z", hex("#f6d125"));
                        c.stroke(u"M26 38C26 42 28.5 45 32 46", hex("#fff6b0"), 3);
                    } };
        case Icon::Fiber:
            return { { 64, 64 }, [](Canvas& c) {
                        c.fill(u"M8 40L48 16l8 8-40 24z", hex("#c4a15a"));
                        c.fill(u"M48 16l8 8-4 2-8-8z", hex("#e8cc86"));
                        c.fill(u"M8 40l8 8 4-2-8-8z", hex("#8b6c33"));
                    } };
        case Icon::Aerogel:
            return { { 64, 64 }, [](Canvas& c) {
                        c.fill(u"M8 34l24-14 24 14-24 14z", hex("#bfe7ff", 0.9));
                        c.fill(u"M8 34v4l24 14v-4z", hex("#7fb9e0"));
                        c.fill(u"M32 48v4l24-14v-4z", hex("#a4d4f2"));
                    } };
        case Icon::Scanner:
            return { { 64, 64 }, [](Canvas& c) {
                        c.fill(u"M16 22h26l10 6v8l-10 6H16z", hex("#e9eef3"));
                        c.fill(rect(20, 40, 10, 16, 3), hex("#cfd6de"));
                        c.fill(u"M52 28l8-4v16l-8-4z", hex("#3fdcff"));
                        c.fill(rect(22, 27, 16, 10, 2), hex("#3fa9ff"));
                    } };
        case Icon::Flashlight:
            return { { 64, 64 }, [](Canvas& c) {
                        c.fill(rect(26, 8, 12, 40, 6), hex("#e4e8ee"));
                        c.fill(rect(24, 44, 16, 12, 3), hex("#bfc6cf"));
                        c.fill(rect(30, 16, 4, 22, 2), hex("#f07b2a"));
                    } };
        case Icon::Repair:
            return { { 64, 64 }, [](Canvas& c) {
                        c.fill(u"M10 24h30l8 6-8 6H10z", hex("#e3e8ee"));
                        c.fill(rect(18, 34, 10, 16, 3), hex("#c3cad3"));
                        c.fill(u"M48 26l10-4v16l-10-4z", hex("#8a96a6"));
                        c.fill(u"M2 30l8-3v6z", hex("#57e1ff"));
                        c.fill(circle(4, 22, 3), hex("#57e1ff"));
                        c.fill(circle(4, 38, 3), hex("#57e1ff"));
                    } };
        case Icon::Knife:
            return { { 64, 64 }, [](Canvas& c) {
                        c.fill(u"M10 50l26-26 6 6-26 26z", hex("#262a30"));
                        c.fill(u"M36 24L54 8l2 2-16 20z", hex("#dfe6ee"));
                    } };
        case Icon::Battery:
            return { { 64, 64 }, [](Canvas& c) {
                        c.fill(rect(20, 10, 24, 44, 4), hex("#3a3f48"));
                        c.fill(rect(26, 6, 12, 6, 2), hex("#6b7280"));
                        c.fill(rect(24, 30, 16, 20, 2), hex("#42e87c"));
                    } };
        case Icon::Fabricator:
            return { { 64, 64 }, [](Canvas& c) {
                        QPainterPath base, deck;
                        base.addEllipse(QPointF(32, 50), 26, 8);
                        deck.addEllipse(QPointF(32, 46), 22, 6);
                        c.fill(base, hex("#e56a26"));
                        c.fill(deck, hex("#c9d1db"));
                        c.fill(rect(18, 16, 4, 30), hex("#b7c0cc"));
                        c.fill(rect(42, 16, 4, 30), hex("#b7c0cc"));
                        c.fill(rect(18, 14, 28, 6), hex("#dfe5ec"));
                        c.fill(rect(28, 4, 8, 12, 3), hex("#4fd3ff"));
                        c.fill(rect(26, 24, 12, 10), hex("#8995a6"));
                    } };

        // ---- equipment --------------------------------------------------------------------------
        case Icon::Diver:
            return { { 64, 96 }, [](Canvas& c) {
                        c.fill(circle(32, 12, 9), hex("#2e3440"));
                        c.fill(u"M20 24h24l4 30-6 2-2-16-2 22 2 30h-7l-1-26-1 26h-7l2-30-2-22-2 16-6-2z", hex("#343b48"));
                        c.fill(rect(26, 26, 12, 18, 2), hex("#495568"));
                        c.fill(rect(28, 46, 8, 10), hex("#3fa0ff"));
                    } };
        case Icon::Mask:
            return { { 64, 64 }, [](Canvas& c) {
                        c.stroke(u"M10 26c0-6 8-10 22-10s22 4 22 10v8c0 6-6 10-12 10-5 0-6-5-10-5s-5 5-10 5c-6 0-12-4-12-10z", c.c, 3);
                    } };
        case Icon::Chip:
            return { { 64, 64 }, [](Canvas& c) {
                        c.stroke(rect(18, 18, 28, 28, 3), c.c, 3);
                        c.stroke(u"M24 12v6M32 12v6M40 12v6M24 46v6M32 46v6M40 46v6M12 24h6M12 32h6M12 40h6M46 24h6M46 32h6M46 40h6", c.c, 2.5);
                    } };
        case Icon::Tank:
            return { { 64, 64 }, [](Canvas& c) {
                        c.stroke(rect(22, 12, 20, 44, 10), c.c, 3);
                        c.stroke(u"M28 12V6h8v6", c.c, 3);
                    } };
        case Icon::Gloves:
            return { { 64, 64 }, [](Canvas& c) {
                        c.stroke(u"M18 54V30l-4-10 5-1 5 9V12h5v14V9h5v17V12h5v16l4-6 5 2-7 16v14z", c.c, 2.6);
                    } };
        case Icon::Fins:
            return { { 64, 64 }, [](Canvas& c) {
                        c.fill(u"M14 10h12v20l-6 26c-8 0-10-6-8-12z", hex("#e8a27c"));
                        c.fill(u"M36 10h12l2 34c2 6 0 12-8 12l-6-26z", hex("#e8a27c"));
                        c.fill(u"M16 10h8v12h-8zM38 10h8v12h-8z", hex("#f1f3f6"));
                        c.stroke(u"M14 44c2 6 6 8 10 6M50 44c-2 6-6 8-10 6", hex("#c0643d"), 3);
                    } };
        case Icon::Compass:
            return { { 64, 64 }, [](Canvas& c) {
                        QFont font = c.p.font();
                        font.setBold(true);
                        font.setPixelSize(18);
                        c.p.setFont(font);
                        c.p.setPen(Qt::white);
                        c.p.drawText(QRectF(0, 10, 64, 24), Qt::AlignCenter, QStringLiteral("N"));
                        c.fill(u"M8 40q24-10 48 0q-24 8-48 0z", hex("#6ce3a8"));
                    } };

        // ---- pings ------------------------------------------------------------------------------
        case Icon::Lifepod:
            return { { 48, 48 }, [](Canvas& c) {
                        const QPainterPath pod = svgPath(u"M8 34c0-16 7-26 16-26s16 10 16 26z");
                        c.fill(pod.subtracted(circle(24, 16, 3)).subtracted(rect(13, 22, 22, 3)), c.c);
                        c.fill(rect(4, 34, 40, 6, 3), c.c);
                    } };
        case Icon::Seamoth:
            return { { 48, 48 }, [](Canvas& c) {
                        c.stroke(circle(24, 22, 9), c.c, 3);
                        c.stroke(u"M24 13V6M6 30h12M30 30h12", c.c, 3);
                        c.stroke(circle(10, 34, 5), c.c, 3);
                        c.stroke(circle(38, 34, 5), c.c, 3);
                    } };
        case Icon::Signal:
            return { { 48, 48 }, [](Canvas& c) {
                        c.fill(circle(24, 24, 4), c.c);
                        QPainterPath waves;
                        for (const qreal r : { 11.0, 19.0 }) {
                            const QRectF box(24 - r, 24 - r, 2 * r, 2 * r);
                            for (const qreal startAngle : { -45.0, 135.0 }) {
                                waves.arcMoveTo(box, startAngle);
                                waves.arcTo(box, startAngle, 90);
                            }
                        }
                        c.stroke(waves, c.c, 3);
                    } };
        case Icon::Beacon:
            return { { 48, 48 }, [](Canvas& c) {
                        c.stroke(u"M18 42l6-30 6 30z", c.c, 3);
                        c.fill(circle(24, 10, 4), c.c);
                    } };

        // ---- theme selection ----------------------------------------------------------------------
        case Icon::Palette:
            return { { 24, 24 }, [](Canvas& c) {
                        QPainterPath palette = svgPath(u"M12 3C7 3 3 6.6 3 11c0 4.1 3.4 7.4 7.6 7.4h1.2c1 0 1.6.8 1.6 1.6 0 1 .7 1.9 1.9 1.9C18.8 21.9 21 17.2 21 12c0-5-4-9-9-9Z");
                        for (const QPointF dot : { QPointF(7.5, 10.5), QPointF(10.2, 6.8), QPointF(14.6, 6.8), QPointF(17.4, 10.6) }) {
                            palette = palette.subtracted(circle(dot.x(), dot.y(), 1.5));
                        }
                        c.fill(palette, c.c);
                    } };
        }
        return { { 1, 1 }, [](Canvas&) {} };
    }

} // namespace

void paint(QPainter& painter, Icon icon, const QRectF& rect, const QColor& color)
{
    const Definition def = definition(icon);
    const qreal scale = std::min(rect.width() / def.box.width(), rect.height() / def.box.height());
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(rect.center());
    painter.scale(scale, scale);
    painter.translate(-def.box.width() / 2, -def.box.height() / 2);
    Canvas canvas { painter, color };
    def.draw(canvas);
    painter.restore();
}

QPixmap pixmap(Icon icon, const QSize& size, const QColor& color)
{
    const qreal dpr = qGuiApp ? qGuiApp->devicePixelRatio() : 1.0;
    QPixmap pm(size * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    paint(p, icon, QRectF(QPointF(), QSizeF(size)), color);
    return pm;
}

QIcon icon(Icon icon, const QColor& color)
{
    QColor disabled = color;
    disabled.setAlphaF(0.3f);
    QIcon result;
    result.addPixmap(pixmap(icon, { 48, 48 }, color));
    result.addPixmap(pixmap(icon, { 48, 48 }, disabled), QIcon::Disabled);
    return result;
}

} // namespace Icons

#ifndef VERTICALLABEL_H
#define VERTICALLABEL_H

#include <QLabel>
#include <QPainter>
#include <QTransform>

class VerticalLabel : public QLabel
/*
    A QLabel that is vertical, rotated 90 degrees clockwise or counter-clockwise.
 */
{
    Q_OBJECT
public:
    explicit VerticalLabel(const QString& text = "", QWidget* parent = nullptr, bool clockwise = false)
        : QLabel(text, parent), m_clockwise(clockwise)
    {
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::TextAntialiasing);

        painter.save();

        // Move origin to bottom-left corner and rotate counter-clockwise
        if (m_clockwise)
        {
            painter.translate(width(), 0);
            painter.rotate(90);
        }
        else
        {
            painter.translate(0, height());
            painter.rotate(-90);
        }

        // Create rotated rect (swap width and height)
        QRect rotatedRect(0, 0, height(), width());

        // Draw text centered in rotated coordinates
        painter.drawText(rotatedRect, Qt::AlignCenter, text());

        painter.restore();
    }


    QSize sizeHint() const override
    {
        QFontMetrics fm(font());
        QSize textSize = fm.size(Qt::TextSingleLine, text());
        return QSize(textSize.height(), textSize.width());
    }

    QSize minimumSizeHint() const override
    {
        return sizeHint();
    }
    bool m_clockwise = true;
};

#endif // VERTICALLABEL_H

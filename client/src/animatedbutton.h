#ifndef ANIMATEDBUTTON_H
#define ANIMATEDBUTTON_H

#include <QPushButton>
#include <QPropertyAnimation>
#include <QGraphicsEffect>

class AnimatedButton : public QPushButton
{
    Q_OBJECT

public:
    explicit AnimatedButton(const QString &text, QWidget *parent = nullptr);
    ~AnimatedButton();

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QPropertyAnimation *m_hoverAnimation;
    QPropertyAnimation *m_pressAnimation;
    QGraphicsDropShadowEffect *m_shadowEffect;
    QSize m_baseSize;
};

#endif

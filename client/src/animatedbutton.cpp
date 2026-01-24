#include "animatedbutton.h"
#include <QEnterEvent>
#include <QMouseEvent>
#include <QEasingCurve>
#include <QGraphicsDropShadowEffect>

AnimatedButton::AnimatedButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    setStyleSheet(
        "QPushButton {"
        "    background-color: #0d7377;"
        "    color: #ffffff;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 10px 20px;"
        "    font-weight: bold;"
        "    font-size: 12px;"
        "    min-width: 120px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #14a085;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #0a5d61;"
        "}"
    );

    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setBlurRadius(10);
    m_shadowEffect->setColor(QColor(13, 115, 119, 150));
    m_shadowEffect->setOffset(0, 3);
    setGraphicsEffect(m_shadowEffect);

    m_baseSize = size();
    
    m_hoverAnimation = new QPropertyAnimation(this, "geometry", this);
    m_hoverAnimation->setDuration(200);
    m_hoverAnimation->setEasingCurve(QEasingCurve::OutCubic);

    m_pressAnimation = new QPropertyAnimation(this, "geometry", this);
    m_pressAnimation->setDuration(100);
    m_pressAnimation->setEasingCurve(QEasingCurve::InOutQuad);
}

AnimatedButton::~AnimatedButton()
{
}

void AnimatedButton::enterEvent(QEnterEvent *event)
{
    QPushButton::enterEvent(event);
    
    QRect currentRect = geometry();
    QRect targetRect = currentRect;
    targetRect.adjust(-2, -2, 2, 2);
    
    m_hoverAnimation->stop();
    m_hoverAnimation->setStartValue(currentRect);
    m_hoverAnimation->setEndValue(targetRect);
    m_hoverAnimation->start();
    
    m_shadowEffect->setColor(QColor(20, 160, 133, 200));
    m_shadowEffect->setBlurRadius(15);
}

void AnimatedButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    
    QRect currentRect = geometry();
    QRect targetRect = currentRect;
    targetRect.adjust(2, 2, -2, -2);
    
    m_hoverAnimation->stop();
    m_hoverAnimation->setStartValue(currentRect);
    m_hoverAnimation->setEndValue(targetRect);
    m_hoverAnimation->start();
    
    m_shadowEffect->setColor(QColor(13, 115, 119, 150));
    m_shadowEffect->setBlurRadius(10);
}

void AnimatedButton::mousePressEvent(QMouseEvent *event)
{
    QPushButton::mousePressEvent(event);
    
    QRect currentRect = geometry();
    QRect targetRect = currentRect;
    targetRect.adjust(1, 1, -1, -1);
    
    m_pressAnimation->stop();
    m_pressAnimation->setStartValue(currentRect);
    m_pressAnimation->setEndValue(targetRect);
    m_pressAnimation->start();
}

void AnimatedButton::mouseReleaseEvent(QMouseEvent *event)
{
    QPushButton::mouseReleaseEvent(event);
    
    QRect currentRect = geometry();
    QRect targetRect = currentRect;
    targetRect.adjust(-1, -1, 1, 1);
    
    m_pressAnimation->stop();
    m_pressAnimation->setStartValue(currentRect);
    m_pressAnimation->setEndValue(targetRect);
    m_pressAnimation->start();
}

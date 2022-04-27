/*
 * Copyright (C) 2020 Chris Holland <zrenfire@gmail.com>
 * Copyright (C) 2018 Vlad Zagorodniy <vladzzag@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// own
#include "AppMenuButtonGroup.h"
#include "InternalSettings.h"

// KDecoration
#include <KDecoration2/Decoration>
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationButton>
#include <KDecoration2/DecorationButtonGroup>

// Qt
#include <QHoverEvent>
#include <QMouseEvent>
#include <QRectF>
#include <QSharedPointer>
#include <QWheelEvent>
#include <QVariant>

// X11
#include <xcb/xcb.h>


namespace Material
{

class Button;
class TextButton;
class MenuOverflowButton;

//* exception
enum ExceptionMask
{
    None = 0,
    BorderSize = 1<<4
};

//* metrics
enum Metrics
{

    //* corner radius (pixels)
    Frame_FrameRadius = 3,

    //* titlebar metrics, in units of small spacing
    TitleBar_TopMargin = 2,
    TitleBar_BottomMargin = 2,
    TitleBar_SideMargin = 2,
    TitleBar_ButtonSpacing = 2,

    // shadow dimensions (pixels)
    Shadow_Overlap = 3,

};


class Decoration : public KDecoration2::Decoration
{
    Q_OBJECT

public:
    Decoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    ~Decoration() override;

    QRect titleBarRect() const;
    QRect centerRect() const;

    void paint(QPainter *painter, const QRect &repaintRegion) override;

public slots:
    void init() override;
    void reconfigure();

protected:
    void hoverEnterEvent(QHoverEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void onSectionUnderMouseChanged(const Qt::WindowFrameSection value);

private:
    void updateBorders();
    void updateResizeBorders();
    void updateTitleBar();
    void updateTitleBarHoverState();
    void setButtonGroupHeight(KDecoration2::DecorationButtonGroup *buttonGroup, int buttonHeight);
    void updateButtonHeight();
    void updateButtonsGeometry();
    void setButtonGroupAnimation(KDecoration2::DecorationButtonGroup *buttonGroup, bool enabled, int duration);
    void updateButtonAnimation();
    void updateShadow();

    bool menuAlwaysShow() const;
    bool animationsEnabled() const;
    int animationsDuration() const;
    int buttonPadding() const;
    int titleBarHeight() const;
    int appMenuCaptionSpacing() const;
    int captionMinWidth() const;

    int bottomBorderSize() const;
    int sideBorderSize() const;

    bool leftBorderVisible() const;
    bool rightBorderVisible() const;
    bool topBorderVisible() const;
    bool bottomBorderVisible() const;

    bool titleBarIsHovered() const;
    int getTextWidth(const QString text, bool showMnemonic = false) const;
    QPoint windowPos() const;

    void initDragMove(const QPoint pos);
    void resetDragMove();
    bool dragMoveTick(const QPoint pos);
    void sendMoveEvent(const QPoint pos);

    QColor borderColor() const;
    QColor titleBarBackgroundColor() const;
    QColor titleBarForegroundColor() const;

    void paintTitleBar(QPainter *painter, const QRect &repaintRegion);
    void createShadow();
    void paintFrameBackground(QPainter *painter, const QRect &repaintRegion) const;
    void paintTitleBarBackground(QPainter *painter, const QRect &repaintRegion) const;
    void paintCaption(QPainter *painter, const QRect &repaintRegion) const;
    void paintButtons(QPainter *painter, const QRect &repaintRegion) const;
    void paintOutline(QPainter *painter, const QRect &repaintRegion) const;

    //* caption height
    int captionHeight() const;

    //* return the rect in which caption will be drawn
    QPair<QRect,Qt::Alignment> captionRect() const;

    //* button height
    int buttonHeight() const;

    //*@name maximization modes
    //@{
    inline bool isMaximized() const;
    inline bool isMaximizedHorizontally() const;
    inline bool isMaximizedVertically() const;

    inline bool isLeftEdge() const;
    inline bool isRightEdge() const;
    inline bool isTopEdge() const;
    inline bool isBottomEdge() const;

    inline bool hideTitleBar() const;
    //@}

    //*@name colors
    //@{
    QColor titleBarColor() const;
    QColor outlineColor() const;
    QColor fontColor() const;
    //@}

    //*@name border size
    //@{
    int borderSize(bool bottom = false) const;
    inline bool hasBorders() const;
    inline bool hasNoBorders() const;
    inline bool hasNoSideBorders() const;
    //@}

    //*@name color customization
    //@{
    inline bool opaqueTitleBar() const;
    inline bool flatTitleBar() const;
    inline int titleBarAlpha() const;
    //@}

    KDecoration2::DecorationButtonGroup *m_leftButtons;
    KDecoration2::DecorationButtonGroup *m_rightButtons;
    AppMenuButtonGroup *m_menuButtons;

    QSharedPointer<InternalSettings> m_internalSettings;

    QPoint m_pressedPoint;
    xcb_atom_t m_moveResizeAtom = 0;

    friend class AppMenuButtonGroup;
    friend class Button;
    friend class AppIconButton;
    friend class AppMenuButton;
    friend class TextButton;
    // friend class MenuOverflowButton;
    //* active state change animation
    QVariantAnimation *m_animation;

    //* active state change opacity
    qreal m_opacity = 0;
};

bool Decoration::hasBorders() const
{
    if( m_internalSettings && m_internalSettings->mask() & BorderSize ) return m_internalSettings->borderSize() > InternalSettings::BorderNoSides;
    else return settings()->borderSize() > KDecoration2::BorderSize::NoSides;
}

bool Decoration::hasNoBorders() const
{
    if( m_internalSettings && m_internalSettings->mask() & BorderSize ) return m_internalSettings->borderSize() == InternalSettings::BorderNone;
    else return settings()->borderSize() == KDecoration2::BorderSize::None;
}

bool Decoration::hasNoSideBorders() const
{
    if( m_internalSettings && m_internalSettings->mask() & BorderSize ) return m_internalSettings->borderSize() == InternalSettings::BorderNoSides;
    else return settings()->borderSize() == KDecoration2::BorderSize::NoSides;
}

bool Decoration::isMaximized() const
{ return client().data()->isMaximized() && !m_internalSettings->drawBorderOnMaximizedWindows(); }

bool Decoration::isMaximizedHorizontally() const
{ return client().data()->isMaximizedHorizontally() && !m_internalSettings->drawBorderOnMaximizedWindows(); }

bool Decoration::isMaximizedVertically() const
{ return client().data()->isMaximizedVertically() && !m_internalSettings->drawBorderOnMaximizedWindows(); }

bool Decoration::isLeftEdge() const
{ return (client().data()->isMaximizedHorizontally() || client().data()->adjacentScreenEdges().testFlag( Qt::LeftEdge ) ) && !m_internalSettings->drawBorderOnMaximizedWindows(); }

bool Decoration::isRightEdge() const
{ return (client().data()->isMaximizedHorizontally() || client().data()->adjacentScreenEdges().testFlag( Qt::RightEdge ) ) && !m_internalSettings->drawBorderOnMaximizedWindows(); }

bool Decoration::isTopEdge() const
{ return (client().data()->isMaximizedVertically() || client().data()->adjacentScreenEdges().testFlag( Qt::TopEdge ) ) && !m_internalSettings->drawBorderOnMaximizedWindows(); }

bool Decoration::isBottomEdge() const
{ return (client().data()->isMaximizedVertically() || client().data()->adjacentScreenEdges().testFlag( Qt::BottomEdge ) ) && !m_internalSettings->drawBorderOnMaximizedWindows(); }

bool Decoration::hideTitleBar() const
{ return m_internalSettings->hideTitleBar() && !client().data()->isShaded(); }

bool Decoration::opaqueTitleBar() const
{ return m_internalSettings->opaqueTitleBar(); }

bool Decoration::flatTitleBar() const
{ return m_internalSettings->flatTitleBar(); }

int Decoration::titleBarAlpha() const
{
    if (m_internalSettings->opaqueTitleBar())
        return 255;
    int a = m_internalSettings->opacityOverride() > -1 ? m_internalSettings->opacityOverride()
                                                        : m_internalSettings->backgroundOpacity();
    a =  qBound(0, a, 100);
    return qRound(static_cast<qreal>(a) * static_cast<qreal>(2.55));
}

} // namespace Material

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

// own
#include "Button.h"
#include "Material.h"
#include "Decoration.h"

#include "AppIconButton.h"
#include "ApplicationMenuButton.h"
#include "OnAllDesktopsButton.h"
#include "ContextHelpButton.h"
#include "ShadeButton.h"
#include "KeepAboveButton.h"
#include "KeepBelowButton.h"
#include "CloseButton.h"
#include "MaximizeButton.h"
#include "MinimizeButton.h"

// KDecoration
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/Decoration>
#include <KDecoration2/DecorationButton>

// KF
#include <KColorUtils>

// Qt
#include <QDebug>
#include <QMargins>
#include <QPainter>
#include <QVariantAnimation>
#include <QtMath> // qFloor


namespace Material
{

Button::Button(KDecoration2::DecorationButtonType type, Decoration *decoration, QObject *parent)
    : DecorationButton(type, decoration, parent)
    , m_animationEnabled(true)
    , m_animation(new QVariantAnimation(this))
    , m_opacity(1)
    , m_animation2(new QVariantAnimation(this))
    , m_transitionValue(0)
    , m_padding(new QMargins())
    , m_isGtkButton(false)
{
    connect(this, &Button::hoveredChanged, this,
        [this](bool hovered) {
            updateAnimationState(hovered);
            update();
        });

    if (QCoreApplication::applicationName() == QStringLiteral("kded5")) {
        // See: https://github.com/Zren/material-decoration/issues/22
        // kde-gtk-config has a kded5 module which renders the buttons to svgs for gtk.
        m_isGtkButton = true;
    }

    // Animation based on SierraBreezeEnhanced
    // https://github.com/kupiqu/SierraBreezeEnhanced/blob/master/breezebutton.cpp#L45
    m_animationEnabled = decoration->animationsEnabled();
    m_animation->setDuration(decoration->animationsDuration());
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        setTransitionValue(value.toReal());
    });
    connect(this, &Button::transitionValueChanged, this, [this]() {
        update();
    });

    connect(this, &Button::opacityChanged, this, [this]() {
        update();
    });

    // setup animation
    // It is important start and end value are of the same type, hence 0.0 and not just 0
    m_animation2->setStartValue( 0.0 );
    m_animation2->setEndValue( 1.0 );
    m_animation2->setEasingCurve( QEasingCurve::InOutQuad );
    connect(m_animation2, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            setOpacity2(value.toReal());
    });
    connect(decoration->client().data(), SIGNAL(iconChanged(QIcon)), this, SLOT(update()));
    connect(decoration->settings().data(), &KDecoration2::DecorationSettings::reconfigured, this, &Button::reconfigure2);
    connect( this, &KDecoration2::DecorationButton::hoveredChanged, this, &Button::updateAnimationState2 );

    reconfigure2();
    setHeight(decoration->titleBarHeight());

    auto *decoratedClient = decoration->client().toStrongRef().data();

    switch (type) {
    case KDecoration2::DecorationButtonType::Menu:
        AppIconButton::init(this, decoratedClient);
        break;

    case KDecoration2::DecorationButtonType::ApplicationMenu:
        ApplicationMenuButton::init(this, decoratedClient);
        break;

    case KDecoration2::DecorationButtonType::OnAllDesktops:
        OnAllDesktopsButton::init(this, decoratedClient);
        break;

    case KDecoration2::DecorationButtonType::ContextHelp:
        ContextHelpButton::init(this, decoratedClient);
        break;

    case KDecoration2::DecorationButtonType::Shade:
        ShadeButton::init(this, decoratedClient);
        break;

    case KDecoration2::DecorationButtonType::KeepAbove:
        KeepAboveButton::init(this, decoratedClient);
        break;

    case KDecoration2::DecorationButtonType::KeepBelow:
        KeepBelowButton::init(this, decoratedClient);
        break;

    case KDecoration2::DecorationButtonType::Close:
        CloseButton::init(this, decoratedClient);
        break;

    case KDecoration2::DecorationButtonType::Maximize:
        MaximizeButton::init(this, decoratedClient);
        break;

    case KDecoration2::DecorationButtonType::Minimize:
        MinimizeButton::init(this, decoratedClient);
        break;

    default:
        break;
    }
}

Button::~Button()
{
}

KDecoration2::DecorationButton* Button::create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
{
    auto deco = qobject_cast<Decoration*>(decoration);
    if (!deco) {
        return nullptr;
    }

    switch (type) {
    case KDecoration2::DecorationButtonType::Menu:
    // case KDecoration2::DecorationButtonType::ApplicationMenu:
    case KDecoration2::DecorationButtonType::OnAllDesktops:
    case KDecoration2::DecorationButtonType::ContextHelp:
    case KDecoration2::DecorationButtonType::Shade:
    case KDecoration2::DecorationButtonType::KeepAbove:
    case KDecoration2::DecorationButtonType::KeepBelow:
    case KDecoration2::DecorationButtonType::Close:
    case KDecoration2::DecorationButtonType::Maximize:
    case KDecoration2::DecorationButtonType::Minimize:
        return new Button(type, deco, parent);

    default:
        return nullptr;
    }
}

Button::Button(QObject *parent, const QVariantList &args)
    : Button(args.at(0).value<KDecoration2::DecorationButtonType>(), args.at(1).value<Decoration*>(), parent)
{
    m_flag = FlagStandalone;
    //! icon size must return to !valid because it was altered from the default constructor,
    //! in Standalone mode the button is not using the decoration metrics but its geometry
    m_iconSize = QSize(-1, -1);
}

//__________________________________________________________________
void Button::drawIconBreezeEnhanced( QPainter *painter )
{

    // translate from offset
    if( m_flag == FlagFirstInList ) painter->translate( m_offset );
    else painter->translate( 0, m_offset.y() );

    if( !m_iconSize.isValid() ) m_iconSize = geometry().size().toSize();

    if (type() == KDecoration2::DecorationButtonType::Menu) {
        const QRectF iconRect( geometry().topLeft(), m_iconSize );
        decoration()->client().data()->icon().paint(painter, iconRect.toRect());
        return;
    }


    painter->setRenderHints( QPainter::Antialiasing );

    /*
    scale painter so that its window matches QRect( -1, -1, 20, 20 )
    this makes all further rendering and scaling simpler
    all further rendering is preformed inside QRect( 0, 0, 18, 18 )
    */
    painter->translate( geometry().topLeft() );

    const qreal width( m_iconSize.width() );
    painter->scale( width/20, width/20 );
    painter->translate( 1, 1 );

    // render background
    const QColor backgroundColor( this->backgroundColor() );

    auto d = qobject_cast<Decoration*>( decoration() );
    bool isInactive(d && !d->client().data()->isActive()
                    && !isHovered() && !isPressed()
                    && m_animation2->state() != QAbstractAnimation::Running);
    QColor inactiveCol(Qt::gray);
    if (isInactive)
    {
        int gray = qGray(d->titleBarColor().rgb());
        if (gray <= 200) {
            gray += 55;
            gray = qMax(gray, 115);
        }
        else gray -= 45;
        inactiveCol = QColor(gray, gray, gray);
    }

    const bool macOSBtn(!d || d->m_internalSettings->buttonType() == InternalSettings::ButtonMacOS);

    // render mark
    const QColor foregroundColor( this->foregroundColor(inactiveCol) );
    if( foregroundColor.isValid() )
    {

        // setup painter
        QPen pen( foregroundColor );
        pen.setCapStyle( Qt::RoundCap );
        pen.setJoinStyle( Qt::MiterJoin );
        pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

        switch( type() )
        {

            case KDecoration2::DecorationButtonType::Close:
            {
                if (macOSBtn) {
                    QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                    if (d && qGray(d->titleBarColor().rgb()) > 100)
                    {
                        grad.setColorAt(0, isInactive ? inactiveCol
                                                      : QColor(255, 92, 87));
                        grad.setColorAt(1, isInactive ? inactiveCol
                                                      : QColor(233, 84, 79));
                    }
                    else
                    {
                        grad.setColorAt(0, isInactive ? inactiveCol
                                                      : QColor(250, 100, 102));
                        grad.setColorAt(1, isInactive ? inactiveCol
                                                      : QColor(230, 92, 94));
                    }
                    painter->setBrush( QBrush(grad) );
                    painter->setPen( Qt::NoPen );
                    painter->drawEllipse( QRectF( 2, 2, 14, 14 ) );
                    if( backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        qreal r = static_cast<qreal>(7)
                                  + (isPressed() ? 0.0
                                      : static_cast<qreal>(2) * m_animation2->currentValue().toReal());
                        QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                        painter->drawEllipse( c, r, r );
                    }
                }
                else {
                    if( backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
                    }
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    painter->drawLine( QPointF( 5, 5 ), QPointF( 13, 13 ) );
                    painter->drawLine( QPointF( 5, 13 ), QPointF( 13, 5 ) );
                }
                break;
            }

            case KDecoration2::DecorationButtonType::Maximize:
            {
                if (macOSBtn) {
                    QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                    if (d && qGray(d->titleBarColor().rgb()) > 100)
                    {
                        grad.setColorAt(0, isChecked() ? isInactive ? inactiveCol
                                                                    : QColor(67, 198, 176)
                                                        : isInactive ? inactiveCol
                                                                    : QColor(40, 211, 63));
                        grad.setColorAt(1, isChecked() ? isInactive ? inactiveCol
                                                                    : QColor(60, 178, 159)
                                                        : isInactive ? inactiveCol
                                                                    : QColor(36, 191, 57));
                    }
                    else
                    {
                        grad.setColorAt(0, isChecked() ? isInactive ? inactiveCol
                                                                    : QColor(67, 198, 176)
                                                        : isInactive ? inactiveCol
                                                                    : QColor(124, 198, 67));
                        grad.setColorAt(1, isChecked() ? isInactive ? inactiveCol
                                                                    : QColor(60, 178, 159)
                                                        : isInactive ? inactiveCol
                                                                    : QColor(111, 178, 60));
                    }
                    painter->setBrush( QBrush(grad) );
                    painter->setPen( Qt::NoPen );
                    painter->drawEllipse( QRectF( 2, 2, 14, 14 ) );
                    if( backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        qreal r = static_cast<qreal>(7)
                                  + (isPressed() ? 0.0
                                      : static_cast<qreal>(2) * m_animation2->currentValue().toReal());
                        QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                        painter->drawEllipse( c, r, r );
                    }
                }
                else {
                    if( backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
                    }

                    if (isHovered())
                        pen.setWidthF( 1.2*qMax((qreal)1.0, 20/width ) );
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    painter->drawPolyline(QPolygonF()
                                            << QPointF(5, 8) << QPointF(5, 13) << QPointF(10, 13));
                    if (isChecked())
                        painter->drawRect(QRectF(8.0, 5.0, 5.0, 5.0));
                    else {
                        painter->drawPolyline(QPolygonF()
                                              << QPointF(8, 5) << QPointF(13, 5) << QPointF(13, 10));
                    }

                    if (isHovered())
                        pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                }
                break;
            }

            case KDecoration2::DecorationButtonType::Minimize:
            {
                if (macOSBtn) {
                    QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                    if (d && qGray(d->titleBarColor().rgb()) > 100)
                    { // yellow isn't good with light backgrounds
                        grad.setColorAt(0, isInactive ? inactiveCol
                                                      : QColor(243, 176, 43));
                        grad.setColorAt(1, isInactive ? inactiveCol
                                                      : QColor(223, 162, 39));
                    }
                    else
                    {
                        grad.setColorAt(0, isInactive ? inactiveCol
                                                      : QColor(237, 198, 81));
                        grad.setColorAt(1, isInactive ? inactiveCol
                                                      : QColor(217, 181, 74));
                    }
                    painter->setBrush( QBrush(grad) );
                    painter->setPen( Qt::NoPen );
                    painter->drawEllipse( QRectF( 2, 2, 14, 14 ) );
                    if( backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        qreal r = static_cast<qreal>(7)
                                  + (isPressed() ? 0.0
                                      : static_cast<qreal>(2) * m_animation2->currentValue().toReal());
                        QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                        painter->drawEllipse( c, r, r );
                    }
                }
                else {
                    if( backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
                    }

                    if (isHovered())
                        pen.setWidthF( 1.2*qMax((qreal)1.0, 20/width ) );
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    painter->drawLine( QPointF( 4, 9 ), QPointF( 14, 9 ) );

                    if (isHovered())
                        pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                }
                break;
            }

            case KDecoration2::DecorationButtonType::OnAllDesktops:
            {

                if (macOSBtn && !isPressed()) {
                    QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                    if (d && qGray(d->titleBarColor().rgb()) > 100)
                    { // yellow isn't good with light backgrounds
                        grad.setColorAt(0, isInactive ? inactiveCol
                                                      : QColor(103, 149, 210));
                        grad.setColorAt(1, isInactive ? inactiveCol
                                                      : QColor(93, 135, 190));
                    }
                    else
                    {
                        grad.setColorAt(0, isInactive ? inactiveCol
                                                      : QColor(135, 166, 220));
                        grad.setColorAt(1, isInactive ? inactiveCol
                                                      : QColor(122, 151, 200));
                    }
                    painter->setBrush( QBrush(grad) );
                    painter->setPen( Qt::NoPen );
                    if (isChecked())
                        painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
                    else {
                        painter->drawEllipse( QRectF( 2, 2, 14, 14 ) );
                        if( backgroundColor.isValid() )
                        {
                            painter->setPen( Qt::NoPen );
                            painter->setBrush( backgroundColor );
                            qreal r = static_cast<qreal>(7)
                                      + static_cast<qreal>(2) * m_animation2->currentValue().toReal();
                            QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                            painter->drawEllipse( c, r, r );
                        }
                    }
                }
                if (!macOSBtn || isPressed() || isHovered() || isChecked()) {
                    painter->setPen( Qt::NoPen );
                    if( (!macOSBtn  || isPressed()) && backgroundColor.isValid() )
                    {
                        painter->setBrush( backgroundColor );
                        painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
                    }
                    painter->setBrush( foregroundColor );

                    if (macOSBtn)
                        painter->drawEllipse( QRectF( 6, 6, 6, 6 ) );
                    else {
                        if( isChecked()) {

                            // outer ring
                            painter->drawEllipse( QRectF( 3, 3, 12, 12 ) );

                            // center dot
                            QColor backgroundColor( this->backgroundColor() );
                            if( !backgroundColor.isValid() && d ) backgroundColor = d->titleBarColor();

                            if( backgroundColor.isValid() )
                            {
                                painter->setBrush( backgroundColor );
                                painter->drawEllipse( QRectF( 8, 8, 2, 2 ) );
                            }

                        } else {

                            painter->drawPolygon( QPolygonF()
                                << QPointF( 6.5, 8.5 )
                                << QPointF( 12, 3 )
                                << QPointF( 15, 6 )
                                << QPointF( 9.5, 11.5 ) );

                            painter->setPen( pen );
                            painter->drawLine( QPointF( 5.5, 7.5 ), QPointF( 10.5, 12.5 ) );
                            painter->drawLine( QPointF( 12, 6 ), QPointF( 4.5, 13.5 ) );
                        }
                    }
                }
                break;
            }

            case KDecoration2::DecorationButtonType::Shade:
            {
                if (macOSBtn && !isPressed()) {
                    QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                    if (d && qGray(d->titleBarColor().rgb()) > 100)
                    { // yellow isn't good with light backgrounds
                        grad.setColorAt(0, isInactive ? inactiveCol
                                                      : QColor(103, 149, 210));
                        grad.setColorAt(1, isInactive ? inactiveCol
                                                      : QColor(93, 135, 190));
                    }
                    else
                    {
                        grad.setColorAt(0, isInactive ? inactiveCol
                                                      : QColor(135, 166, 220));
                        grad.setColorAt(1, isInactive ? inactiveCol
                                                      : QColor(122, 151, 200));
                    }
                    painter->setBrush( QBrush(grad) );
                    painter->setPen( Qt::NoPen );
                    if (isChecked())
                        painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
                    else {
                        painter->drawEllipse( QRectF( 2, 2, 14, 14 ) );
                        if( backgroundColor.isValid() )
                        {
                            painter->setPen( Qt::NoPen );
                            painter->setBrush( backgroundColor );
                            qreal r = static_cast<qreal>(7)
                                      + static_cast<qreal>(2) * m_animation2->currentValue().toReal();
                            QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                            painter->drawEllipse( c, r, r );
                        }
                    }
                }
                if (!macOSBtn || isPressed() || isHovered() || isChecked()) {
                    if( (!macOSBtn  || isPressed()) && backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
                    }
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    painter->drawLine( 5, 6, 13, 6 );
                    if (isChecked()) {
                        painter->drawPolyline( QPolygonF()
                            << QPointF( 5, 9 )
                            << QPointF( 9, 13 )
                            << QPointF( 13, 9 ) );

                    } else {
                        painter->drawPolyline( QPolygonF()
                            << QPointF( 5, 13 )
                            << QPointF( 9, 9 )
                            << QPointF( 13, 13 ) );
                    }
                }

                break;

            }

            case KDecoration2::DecorationButtonType::KeepBelow:
            {
                if (macOSBtn && !isPressed()) {
                    QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                    if (d && qGray(d->titleBarColor().rgb()) > 100)
                    { // yellow isn't good with light backgrounds
                        grad.setColorAt(0, isInactive ? inactiveCol
                                                      : QColor(103, 149, 210));
                        grad.setColorAt(1, isInactive ? inactiveCol
                                                      : QColor(93, 135, 190));
                    }
                    else
                    {
                        grad.setColorAt(0, isInactive ? inactiveCol
                                                      : QColor(135, 166, 220));
                        grad.setColorAt(1, isInactive ? inactiveCol
                                                      : QColor(122, 151, 200));
                    }
                    painter->setBrush( QBrush(grad) );
                    painter->setPen( Qt::NoPen );
                    if (isChecked())
                        painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
                    else {
                        painter->drawEllipse( QRectF( 2, 2, 14, 14 ) );
                        if( backgroundColor.isValid() )
                        {
                            painter->setPen( Qt::NoPen );
                            painter->setBrush( backgroundColor );
                            qreal r = static_cast<qreal>(7)
                                      + static_cast<qreal>(2) * m_animation2->currentValue().toReal();
                            QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                            painter->drawEllipse( c, r, r );
                        }
                    }
                }
                if (!macOSBtn || isPressed() || isHovered() || isChecked()) {
                    if( (!macOSBtn  || isPressed()) && backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
                    }
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    if (macOSBtn) {
                        painter->drawPolyline( QPolygonF()
                            << QPointF( 6, 6 )
                            << QPointF( 9, 9 )
                            << QPointF( 12, 6 ) );

                        painter->drawPolyline( QPolygonF()
                            << QPointF( 6, 10 )
                            << QPointF( 9, 13 )
                            << QPointF( 12, 10 ) );
                    }
                    else {
                        painter->drawPolyline( QPolygonF()
                            << QPointF( 5, 5 )
                            << QPointF( 9, 9 )
                            << QPointF( 13, 5 ) );

                        painter->drawPolyline( QPolygonF()
                            << QPointF( 5, 9 )
                            << QPointF( 9, 13 )
                            << QPointF( 13, 9 ) );
                    }
                }
                break;

            }

            case KDecoration2::DecorationButtonType::KeepAbove:
            {
                if (macOSBtn && !isPressed()) {
                    QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                    if (d && qGray(d->titleBarColor().rgb()) > 100)
                    { // yellow isn't good with light backgrounds
                        grad.setColorAt(0, isInactive ? inactiveCol
                                                      : QColor(103, 149, 210));
                        grad.setColorAt(1, isInactive ? inactiveCol
                                                      : QColor(93, 135, 190));
                    }
                    else
                    {
                        grad.setColorAt(0, isInactive ? inactiveCol
                                                      : QColor(135, 166, 220));
                        grad.setColorAt(1, isInactive ? inactiveCol
                                                      : QColor(122, 151, 200));
                    }
                    painter->setBrush( QBrush(grad) );
                    painter->setPen( Qt::NoPen );
                    if (isChecked())
                        painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
                    else {
                        painter->drawEllipse( QRectF( 2, 2, 14, 14 ) );
                        if( backgroundColor.isValid() )
                        {
                            painter->setPen( Qt::NoPen );
                            painter->setBrush( backgroundColor );
                            qreal r = static_cast<qreal>(7)
                                      + static_cast<qreal>(2) * m_animation2->currentValue().toReal();
                            QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                            painter->drawEllipse( c, r, r );
                        }
                    }
                }
                if (!macOSBtn || isPressed() || isHovered() || isChecked()) {
                    if( (!macOSBtn  || isPressed()) && backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
                    }
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    if (macOSBtn) {
                        painter->drawPolyline( QPolygonF()
                            << QPointF( 6, 8 )
                            << QPointF( 9, 5 )
                            << QPointF( 12, 8 ) );

                        painter->drawPolyline( QPolygonF()
                            << QPointF( 6, 12 )
                            << QPointF( 9, 9 )
                            << QPointF( 12, 12 ) );
                    }
                    else {
                        painter->drawPolyline( QPolygonF()
                            << QPointF( 5, 9 )
                            << QPointF( 9, 5 )
                            << QPointF( 13, 9 ) );

                        painter->drawPolyline( QPolygonF()
                            << QPointF( 5, 13 )
                            << QPointF( 9, 9 )
                            << QPointF( 13, 13 ) );
                    }
                }
                break;
            }


            case KDecoration2::DecorationButtonType::ApplicationMenu:
            {
                if (macOSBtn && !isPressed()) {
                    QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                    if (d && qGray(d->titleBarColor().rgb()) > 100)
                    { // yellow isn't good with light backgrounds
                        grad.setColorAt(0, isInactive ? inactiveCol
                                                      : QColor(230, 129, 67));
                        grad.setColorAt(1, isInactive ? inactiveCol
                                                      : QColor(210, 118, 61));
                    }
                    else
                    {
                        grad.setColorAt(0, isInactive ? inactiveCol
                                                      : QColor(250, 145, 100));
                        grad.setColorAt(1, isInactive ? inactiveCol
                                                      : QColor(230, 131, 92));
                    }
                    painter->setBrush( QBrush(grad) );
                    painter->setPen( Qt::NoPen );
                    painter->drawEllipse( QRectF( 2, 2, 14, 14 ) );
                    if( backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        qreal r = static_cast<qreal>(7)
                                  + static_cast<qreal>(2) * m_animation2->currentValue().toReal();
                        QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                        painter->drawEllipse( c, r, r );
                    }
                }
                if (!macOSBtn || isPressed() || isHovered()) {
                    if( (!macOSBtn  || isPressed()) && backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
                    }
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    if (macOSBtn) {
                        painter->drawLine( QPointF( 4.5, 6 ), QPointF( 13.5, 6 ) );
                        painter->drawLine( QPointF( 4.5, 9 ), QPointF( 13.5, 9 ) );
                        painter->drawLine( QPointF( 4.5, 12 ), QPointF( 13.5, 12 ) );
                    }
                    else {
                        painter->drawLine( QPointF( 3.5, 5 ), QPointF( 14.5, 5 ) );
                        painter->drawLine( QPointF( 3.5, 9 ), QPointF( 14.5, 9 ) );
                        painter->drawLine( QPointF( 3.5, 13 ), QPointF( 14.5, 13 ) );
                    }
                }
                break;
            }

            case KDecoration2::DecorationButtonType::ContextHelp:
            {
                if (macOSBtn && !isPressed()) {
                    QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                    if (d && qGray(d->titleBarColor().rgb()) > 100)
                    { // yellow isn't good with light backgrounds
                        grad.setColorAt(0, isInactive ? inactiveCol
                                                      : QColor(103, 149, 210));
                        grad.setColorAt(1, isInactive ? inactiveCol
                                                      : QColor(93, 135, 190));
                    }
                    else
                    {
                        grad.setColorAt(0, isInactive ? inactiveCol
                                                      : QColor(135, 166, 220));
                        grad.setColorAt(1, isInactive ? inactiveCol
                                                      : QColor(122, 151, 200));
                    }
                    painter->setBrush( QBrush(grad) );
                    painter->setPen( Qt::NoPen );
                    painter->drawEllipse( QRectF( 2, 2, 14, 14 ) );
                    if( backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        qreal r = static_cast<qreal>(7)
                                  + static_cast<qreal>(2) * m_animation2->currentValue().toReal();
                        QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                        painter->drawEllipse( c, r, r );
                    }
                }
                if (!macOSBtn || isPressed() || isHovered()) {
                    if( (!macOSBtn  || isPressed()) && backgroundColor.isValid() )
                    {
                        painter->setPen( Qt::NoPen );
                        painter->setBrush( backgroundColor );
                        painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
                    }
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    QPainterPath path;
                    path.moveTo( 5, 6 );
                    path.arcTo( QRectF( 5, 3.5, 8, 5 ), 180, -180 );
                    path.cubicTo( QPointF(12.5, 9.5), QPointF( 9, 7.5 ), QPointF( 9, 11.5 ) );
                    painter->drawPath( path );

                    painter->drawPoint( 9, 15 );
                }

                break;
            }
            default:
                break;
        }

    }
}


//__________________________________________________________________
void Button::drawIconMaterial( QPainter *painter )
{
    // Buttons are coded assuming 24 units in size.
    const QRectF buttonRect = geometry();
    const QRectF contentRect = contentArea();

    const qreal iconScale = contentRect.height()/24;
    int iconSize;
    if (m_isGtkButton) {
        // See: https://github.com/Zren/material-decoration/issues/22
        // kde-gtk-config has a kded5 module which renders the buttons to svgs for gtk.

        // The svgs are 50x50, located at ~/.config/gtk-3.0/assets/
        // They are usually scaled down to just 18x18 when drawn in gtk headerbars.
        // The Gtk theme already has a fairly large amount of padding, as
        // the Breeze theme doesn't currently follow fitt's law. So use less padding
        // around the icon so that the icon is not a very tiny 8px.

        // 15% top/bottom padding, 70% leftover for the icon.
        // 24 = 3.5 topPadding + 17 icon + 3.5 bottomPadding
        // 17/24 * 18 = 12.75
        iconSize = qRound(iconScale * 17);
    } else {
        // 30% top/bottom padding, 40% leftover for the icon.
        // 24 = 7 topPadding + 10 icon + 7 bottomPadding
        iconSize = qRound(iconScale * 10);
    }
    QRectF iconRect = QRectF(0, 0, iconSize, iconSize);
    iconRect.moveCenter(contentRect.center().toPoint());

    const qreal gridUnit = iconRect.height()/10;

    painter->save();

    painter->setRenderHints(QPainter::Antialiasing, false);

    // Opacity
    painter->setOpacity(m_opacity);

    // Background.
    painter->setPen(Qt::NoPen);
    painter->setBrush(backgroundColor());
    painter->drawRect(buttonRect);

    // Foreground.
    setPenWidth(painter, gridUnit, 1);
    painter->setBrush(Qt::NoBrush);


    // Icon
    switch (type()) {
    case KDecoration2::DecorationButtonType::Menu:
        AppIconButton::paintIcon(this, painter, iconRect, gridUnit);
        break;

    case KDecoration2::DecorationButtonType::ApplicationMenu:
        ApplicationMenuButton::paintIcon(this, painter, iconRect, gridUnit);
        break;

    case KDecoration2::DecorationButtonType::OnAllDesktops:
        OnAllDesktopsButton::paintIcon(this, painter, iconRect, gridUnit);
        break;

    case KDecoration2::DecorationButtonType::ContextHelp:
        ContextHelpButton::paintIcon(this, painter, iconRect, gridUnit);
        break;

    case KDecoration2::DecorationButtonType::Shade:
        ShadeButton::paintIcon(this, painter, iconRect, gridUnit);
        break;

    case KDecoration2::DecorationButtonType::KeepAbove:
        KeepAboveButton::paintIcon(this, painter, iconRect, gridUnit);
        break;

    case KDecoration2::DecorationButtonType::KeepBelow:
        KeepBelowButton::paintIcon(this, painter, iconRect, gridUnit);
        break;

    case KDecoration2::DecorationButtonType::Close:
        CloseButton::paintIcon(this, painter, iconRect, gridUnit);
        break;

    case KDecoration2::DecorationButtonType::Maximize:
        MaximizeButton::paintIcon(this, painter, iconRect, gridUnit);
        break;

    case KDecoration2::DecorationButtonType::Minimize:
        MinimizeButton::paintIcon(this, painter, iconRect, gridUnit);
        break;

    default:
        paintIcon(painter, iconRect, gridUnit);
        break;
    }

}

void Button::paint(QPainter *painter, const QRect &repaintRegion)
{
    Q_UNUSED(repaintRegion)

    if (!decoration()) return;

    painter->save();

    // Button Type
    if ( isAppMenu() ) {
      drawIconMaterial( painter );
    } else {
        auto d = qobject_cast<Decoration*>( decoration() );
        switch (d->m_internalSettings->buttonType()) {
          case InternalSettings::ButtonMacOS:
          case InternalSettings::ButtonBreeze:
              drawIconBreezeEnhanced( painter );
              break;

          case InternalSettings::ButtonMaterial:
              drawIconMaterial( painter );
              break;

          default:
              drawIconMaterial( painter );
              break;
        }
    }


    painter->restore();
}

void Button::paintIcon(QPainter *painter, const QRectF &iconRect, const qreal gridUnit)
{
    Q_UNUSED(painter)
    Q_UNUSED(iconRect)
    Q_UNUSED(gridUnit)
}

void Button::updateSize(int contentWidth, int contentHeight)
{
    const QSize size(
        m_padding->left() + contentWidth + m_padding->right(),
        m_padding->top() + contentHeight + m_padding->bottom()
    );
    setGeometry(QRect(QPoint(0, 0), size));
}

void Button::setHeight(int buttonHeight)
{
  const auto *d = qobject_cast<Decoration *>(decoration());
  if ( !d || d->m_internalSettings->buttonType() == InternalSettings::ButtonMaterial || isAppMenu() ) {
    // For simplicity, don't count the 1.33:1 scaling in the left/right padding.
    // The left/right padding is mainly for the border offset alignment.
    updateSize(qRound(buttonHeight * 1.33), buttonHeight);
  } else {
    // setup default geometry
    const int height = d->buttonHeight();
    updateSize(height, height);
  }
}

QColor Button::foregroundColor() const
{
    const auto *deco = qobject_cast<Decoration *>(decoration());
    if (!deco) {
        return {};
    }

    //--- Checked
    if (isChecked() && type() != KDecoration2::DecorationButtonType::Maximize) {
        QColor activeColor = KColorUtils::mix(
            deco->titleBarBackgroundColor(),
            deco->titleBarForegroundColor(),
            0.2);

        if (isPressed() || isHovered()) {
            return KColorUtils::mix(
                activeColor,
                deco->titleBarBackgroundColor(),
                m_transitionValue);
        }
        return activeColor;
    }

    //--- Normal
    QColor normalColor = KColorUtils::mix(
        deco->titleBarBackgroundColor(),
        deco->titleBarForegroundColor(),
        0.8);

    if (isPressed() || isHovered()) {
        return KColorUtils::mix(
            normalColor,
            deco->titleBarForegroundColor(),
            m_transitionValue);
    }

    return normalColor;
}


qreal Button::iconLineWidth(const qreal gridUnit) const
{
    return PenWidth::Symbol * qMax(1.0, gridUnit);
}

void Button::setPenWidth(QPainter *painter, const qreal gridUnit, const qreal scale)
{
    QPen pen(foregroundColor());
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setWidthF(iconLineWidth(gridUnit) * scale);
    painter->setPen(pen);
}

QColor Button::backgroundColor() const
{
    const auto *deco = qobject_cast<Decoration *>(decoration());
    if (!deco) {
        return {};
    }
    if ( deco->m_internalSettings->buttonType() != InternalSettings::ButtonMaterial && ! isAppMenu() ) {
            if( isPressed() ) {

                QColor col;
                if( type() == KDecoration2::DecorationButtonType::Close )
                {
                    if (qGray(deco->titleBarColor().rgb()) > 100)
                        col = QColor(254, 73, 66);
                    else
                        col = QColor(240, 77, 80);
                }
                else if( type() == KDecoration2::DecorationButtonType::Maximize)
                {
                    if (qGray(deco->titleBarColor().rgb()) > 100)
                        col = isChecked() ? QColor(0, 188, 154) : QColor(7, 201, 33);
                    else
                        col = isChecked() ? QColor(0, 188, 154) : QColor(101, 188, 34);
                }
                else if( type() == KDecoration2::DecorationButtonType::Minimize )
                {
                    if (qGray(deco->titleBarColor().rgb()) > 100)
                        col = QColor(233, 160, 13);
                    else
                        col = QColor(227, 185, 59);
                }
                else if( type() == KDecoration2::DecorationButtonType::ApplicationMenu ) {
                    if (qGray(deco->titleBarColor().rgb()) > 100)
                        col = QColor(220, 124, 64);
                    else
                        col = QColor(240, 139, 96);
                }
                else {
                    if (qGray(deco->titleBarColor().rgb()) > 100)
                        col = QColor(83, 121, 170);
                    else
                        col = QColor(110, 136, 180);
                }
                if (col.isValid())
                    return col;
                else return KColorUtils::mix( deco->titleBarColor(), deco->fontColor(), 0.3 );

            } else if( m_animation2->state() == QAbstractAnimation::Running ) {

                QColor col;
                if( type() == KDecoration2::DecorationButtonType::Close )
                {
                    if (qGray(deco->titleBarColor().rgb()) > 100)
                        col = QColor(254, 95, 87);
                    else
                        col = QColor(240, 96, 97);
                }
                else if( type() == KDecoration2::DecorationButtonType::Maximize)
                {
                    if (qGray(deco->titleBarColor().rgb()) > 100)
                        col = isChecked() ? QColor(64, 188, 168) : QColor(39, 201, 63);
                    else
                        col = isChecked() ? QColor(64, 188, 168) : QColor(116, 188, 64);
                }
                else if( type() == KDecoration2::DecorationButtonType::Minimize )
                {
                    if (qGray(deco->titleBarColor().rgb()) > 100)
                        col = QColor(233, 172, 41);
                    else
                        col = QColor(227, 191, 78);
                }
                else if( type() == KDecoration2::DecorationButtonType::ApplicationMenu ) {
                    if (qGray(deco->titleBarColor().rgb()) > 100)
                        col = QColor(220, 124, 64);
                    else
                        col = QColor(240, 139, 96);
                }
                else {
                    if (qGray(deco->titleBarColor().rgb()) > 100)
                        col = QColor(98, 141, 200);
                    else
                        col = QColor(128, 157, 210);
                }
                if (col.isValid())
                    return col;
                else {

                    col = deco->fontColor();
                    col.setAlpha( col.alpha()*m_opacity2 );
                    return col;

                }

            } else if( isHovered() ) {

                QColor col;
                if( type() == KDecoration2::DecorationButtonType::Close )
                {
                    if (qGray(deco->titleBarColor().rgb()) > 100)
                        col = QColor(254, 95, 87);
                    else
                        col = QColor(240, 96, 97);
                }
                else if( type() == KDecoration2::DecorationButtonType::Maximize)
                {
                    if (qGray(deco->titleBarColor().rgb()) > 100)
                        col = isChecked() ? QColor(64, 188, 168) : QColor(39, 201, 63);
                    else
                        col = isChecked() ? QColor(64, 188, 168) : QColor(116, 188, 64);
                }
                else if( type() == KDecoration2::DecorationButtonType::Minimize )
                {
                    if (qGray(deco->titleBarColor().rgb()) > 100)
                        col = QColor(233, 172, 41);
                    else
                        col = QColor(227, 191, 78);
                }
                else if( type() == KDecoration2::DecorationButtonType::ApplicationMenu ) {
                    if (qGray(deco->titleBarColor().rgb()) > 100)
                        col = QColor(220, 124, 64);
                    else
                        col = QColor(240, 139, 96);
                }
                else {
                    if (qGray(deco->titleBarColor().rgb()) > 100)
                        col = QColor(98, 141, 200);
                    else
                        col = QColor(128, 157, 210);
                }
                if (col.isValid())
                    return col;
                else return deco->fontColor();

            } else {

                return QColor();

            }
        }
        else {
            //--- CloseButton
            if (type() == KDecoration2::DecorationButtonType::Close) {
                auto *decoratedClient = deco->client().toStrongRef().data();
                const QColor hoveredColor = decoratedClient->color(
                    KDecoration2::ColorGroup::Warning,
                    KDecoration2::ColorRole::Foreground
                );
                QColor normalColor = QColor(hoveredColor);
                normalColor.setAlphaF(0);

                if (isPressed()) {
                    const QColor pressedColor = decoratedClient->color(
                        KDecoration2::ColorGroup::Warning,
                        KDecoration2::ColorRole::Foreground
                    ).lighter();
                    return KColorUtils::mix(normalColor, pressedColor, m_transitionValue);
                }

                if (isHovered()) {
                    return KColorUtils::mix(normalColor, hoveredColor, m_transitionValue);
                }
            }

            //--- Checked
            if (isChecked() && type() != KDecoration2::DecorationButtonType::Maximize) {
                const QColor normalColor = deco->titleBarForegroundColor();

                if (isPressed()) {
                    const QColor pressedColor = KColorUtils::mix(
                        deco->titleBarBackgroundColor(),
                        deco->titleBarForegroundColor(),
                        0.7);
                    return KColorUtils::mix(normalColor, pressedColor, m_transitionValue);
                }
                if (isHovered()) {
                    const QColor hoveredColor = KColorUtils::mix(
                        deco->titleBarBackgroundColor(),
                        deco->titleBarForegroundColor(),
                        0.8);
                    return KColorUtils::mix(normalColor, hoveredColor, m_transitionValue);
                }
                return normalColor;
            }

            //--- Normal
            const QColor hoveredColor = KColorUtils::mix(
                deco->titleBarBackgroundColor(),
                deco->titleBarForegroundColor(),
                0.2);
            QColor normalColor = QColor(hoveredColor);
            normalColor.setAlphaF(0);

            if (isPressed()) {
                const QColor pressedColor = KColorUtils::mix(
                    deco->titleBarBackgroundColor(),
                    deco->titleBarForegroundColor(),
                    0.3);
                return KColorUtils::mix(normalColor, pressedColor, m_transitionValue);
            }
            if (isHovered()) {
                return KColorUtils::mix(normalColor, hoveredColor, m_transitionValue);
            }
            return normalColor;
        }
}
//__________________________________________________________________
QColor Button::foregroundColor(const QColor& inactiveCol) const
{
    auto d = qobject_cast<Decoration*>( decoration() );
    const bool macOSBtn(!d || d->m_internalSettings->buttonType() == InternalSettings::ButtonMacOS);
    if(macOSBtn && ! isAppMenu() ) {
        QColor col;
        if (d && !d->client().data()->isActive()
            && !isHovered() && !isPressed()
            && m_animation2->state() != QAbstractAnimation::Running)
        {
            int v = qGray(inactiveCol.rgb());
            if (v > 127) v -= 127;
            else v += 128;
            col = QColor(v, v, v);
        }
        else
        {
            if (d && qGray(d->titleBarColor().rgb()) > 100)
                col = QColor(250, 250, 250);
            else
                col = QColor(40, 40, 40);
        }
        return col;
    }
    else if( !d ) {

        return QColor();

    } else if( isPressed() ) {

        return d->titleBarColor();

    /*} else if( type() == KDecoration2::DecorationButtonType::Close && d->m_internalSettings->outlineCloseButton() ) {

        return d->titleBarColor();*/

    } else if( ( type() == KDecoration2::DecorationButtonType::KeepBelow || type() == KDecoration2::DecorationButtonType::KeepAbove ) && isChecked() ) {

        return d->titleBarColor();

    } else if( m_animation2->state() == QAbstractAnimation::Running ) { // TODO check this if this could mess up the text menu

        return KColorUtils::mix( d->fontColor(), d->titleBarColor(), m_opacity2 );

    } else if( isHovered() ) {

        return d->titleBarColor();

    } else {

        return d->fontColor();

    }

}

QColor Button::backgroundColorMaterial() const
{
    const auto *deco = qobject_cast<Decoration *>(decoration());
    if (!deco) {
        return {};
    }

    //--- CloseButton
    if (type() == KDecoration2::DecorationButtonType::Close) {
        auto *decoratedClient = deco->client().toStrongRef().data();
        const QColor hoveredColor = decoratedClient->color(
            KDecoration2::ColorGroup::Warning,
            KDecoration2::ColorRole::Foreground
        );
        QColor normalColor = QColor(hoveredColor);
        normalColor.setAlphaF(0);

        if (isPressed()) {
            const QColor pressedColor = decoratedClient->color(
                KDecoration2::ColorGroup::Warning,
                KDecoration2::ColorRole::Foreground
            ).lighter();
            return KColorUtils::mix(normalColor, pressedColor, m_transitionValue);
        }

        if (isHovered()) {
            return KColorUtils::mix(normalColor, hoveredColor, m_transitionValue);
        }
    }

    //--- Checked
    if (isChecked() && type() != KDecoration2::DecorationButtonType::Maximize) {
        const QColor normalColor = deco->titleBarForegroundColor();

        if (isPressed()) {
            const QColor pressedColor = KColorUtils::mix(
                deco->titleBarBackgroundColor(),
                deco->titleBarForegroundColor(),
                0.7);
            return KColorUtils::mix(normalColor, pressedColor, m_transitionValue);
        }
        if (isHovered()) {
            const QColor hoveredColor = KColorUtils::mix(
                deco->titleBarBackgroundColor(),
                deco->titleBarForegroundColor(),
                0.8);
            return KColorUtils::mix(normalColor, hoveredColor, m_transitionValue);
        }
        return normalColor;
    }

    //--- Normal
    const QColor hoveredColor = KColorUtils::mix(
        deco->titleBarBackgroundColor(),
        deco->titleBarForegroundColor(),
        0.2);
    QColor normalColor = QColor(hoveredColor);
    normalColor.setAlphaF(0);

    if (isPressed()) {
        const QColor pressedColor = KColorUtils::mix(
            deco->titleBarBackgroundColor(),
            deco->titleBarForegroundColor(),
            0.3);
        return KColorUtils::mix(normalColor, pressedColor, m_transitionValue);
    }
    if (isHovered()) {
        return KColorUtils::mix(normalColor, hoveredColor, m_transitionValue);
    }
    return normalColor;
}

QColor Button::foregroundColorMaterial() const
{
    const auto *deco = qobject_cast<Decoration *>(decoration());
    if (!deco) {
        return {};
    }

    //--- Checked
    if (isChecked() && type() != KDecoration2::DecorationButtonType::Maximize) {
        QColor activeColor = KColorUtils::mix(
            deco->titleBarBackgroundColor(),
            deco->titleBarForegroundColor(),
            0.2);

        if (isPressed() || isHovered()) {
            return KColorUtils::mix(
                activeColor,
                deco->titleBarBackgroundColor(),
                m_transitionValue);
        }
        return activeColor;
    }

    //--- Normal
    QColor normalColor = KColorUtils::mix(
        deco->titleBarBackgroundColor(),
        deco->titleBarForegroundColor(),
        0.8);

    if (isPressed() || isHovered()) {
        return KColorUtils::mix(
            normalColor,
            deco->titleBarForegroundColor(),
            m_transitionValue);
    }

    return normalColor;
}

QRectF Button::contentArea() const
{
    return geometry().adjusted(
        m_padding->left(),
        m_padding->top(),
        -m_padding->right(),
        -m_padding->bottom()
    );
}

bool Button::animationEnabled() const
{
    return m_animationEnabled;
}

void Button::setAnimationEnabled(bool value)
{
    if (m_animationEnabled != value) {
        m_animationEnabled = value;
        emit animationEnabledChanged();
    }
}

int Button::animationDuration() const
{
    return m_animation->duration();
}

void Button::setAnimationDuration(int value)
{
    if (m_animation->duration() != value) {
        m_animation->setDuration(value);
        emit animationDurationChanged();
    }
}

qreal Button::opacity() const
{
    return m_opacity;
}

void Button::setOpacity(qreal value)
{
    if (m_opacity != value) {
        m_opacity = value;
        emit opacityChanged();
    }
}

qreal Button::transitionValue() const
{
    return m_transitionValue;
}

void Button::setTransitionValue(qreal value)
{
    if (m_transitionValue != value) {
        m_transitionValue = value;
        emit transitionValueChanged(value);
    }
}

QMargins* Button::padding()
{
    return m_padding;
}

//________________________________________________________________
void Button::reconfigure2()
{

    // animation
    auto d = qobject_cast<Decoration*>(decoration());
    if( d )  m_animation2->setDuration( d->m_internalSettings->animationsDuration() );

}

void Button::updateAnimationState2(bool hovered)
{
    QAbstractAnimation::Direction dir = hovered ? QAbstractAnimation::Forward : QAbstractAnimation::Backward;
    if (m_animation2->state() == QAbstractAnimation::Running && m_animation2->direction() != dir) {
        m_animation2->stop();
    }
    m_animation2->setDirection(dir);
    if (m_animation2->state() != QAbstractAnimation::Running) {
        m_animation2->start();
    }
}

void Button::updateAnimationState(bool hovered)
{
    if (m_animationEnabled) {
        QAbstractAnimation::Direction dir = hovered ? QAbstractAnimation::Forward : QAbstractAnimation::Backward;
        if (m_animation->state() == QAbstractAnimation::Running && m_animation->direction() != dir) {
            m_animation->stop();
        }
        m_animation->setDirection(dir);
        if (m_animation->state() != QAbstractAnimation::Running) {
            m_animation->start();
        }
    } else {
        setTransitionValue(1);
    }
}

} // namespace Material

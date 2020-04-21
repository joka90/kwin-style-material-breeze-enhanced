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
#include "CommonButton.h"
#include "Decoration.h"

// KDecoration
#include <KDecoration2/DecoratedClient>

// Qt
#include <QPainter>

namespace Material
{

class AppIconButton
{

public:
    static void init(CommonButton *button, KDecoration2::DecoratedClient *decoratedClient) {
        QObject::connect(decoratedClient, &KDecoration2::DecoratedClient::iconChanged,
            button, [button] {
                button->update();
            }
        );
    }
    static void paintIcon(CommonButton *button, QPainter *painter, const QRectF &iconRect) {
        Q_UNUSED(iconRect)

        const QRectF buttonRect = button->geometry();
        QRectF appIconRect = QRectF(0, 0, 16, 16);
        appIconRect.moveCenter(buttonRect.center().toPoint());

        auto *decoratedClient = button->decoration()->client().toStrongRef().data();
        decoratedClient->icon().paint(painter, appIconRect.toRect());
    }
};

} // namespace Material

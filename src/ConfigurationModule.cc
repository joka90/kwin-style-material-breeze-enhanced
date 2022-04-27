/*
 * Copyright (C) 2020 Chris Holland <zrenfire@gmail.com>
 * Copyright (C) 2012 Martin Gräßlin <mgraesslin@kde.org>
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
#include "ConfigurationModule.h"
#include "Material.h"
#include "InternalSettings.h"

// KF
#include <KColorButton>
#include <KConfigSkeleton>
#include <KCoreConfigSkeleton>
#include <KCModule>
#include <KLocalizedString>
#include <KSharedConfig>

// KDecoration
#include <KDecoration2/DecorationButton>

// Qt
#include <QDebug>

// QWidget
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QFontComboBox>

namespace Material
{


ConfigurationModule::ConfigurationModule(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_titleAlignment(InternalSettings::AlignCenterFullWidth)
    , m_buttonSize(InternalSettings::ButtonDefault)
    , m_shadowSize(InternalSettings::ShadowVeryLarge)
    , m_buttonType(InternalSettings::ButtonMaterial)
{
    init();
}

void ConfigurationModule::init()
{
    KCoreConfigSkeleton *skel = new KCoreConfigSkeleton(KSharedConfig::openConfig(s_configFilename), this);
    skel->setCurrentGroup(QStringLiteral("Windeco"));

    // See fr.po for the messages we can reuse from breeze:
    // https://websvn.kde.org/*checkout*/trunk/l10n-kf5/fr/messages/breeze/breeze_kwin_deco.po

    //--- Tabs
    QTabWidget *tabWidget = new QTabWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tabWidget);
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    //--- General
    QWidget *generalTab = new QWidget(tabWidget);
    tabWidget->addTab(generalTab, i18nd("breeze_kwin_deco", "General"));
    QFormLayout *generalForm = new QFormLayout(generalTab);
    generalTab->setLayout(generalForm);

    QComboBox *titleAlignment = new QComboBox(generalTab);
    titleAlignment->addItem(i18nd("breeze_kwin_deco", "Left"));
    titleAlignment->addItem(i18nd("breeze_kwin_deco", "Center"));
    titleAlignment->addItem(i18nd("breeze_kwin_deco", "Center (Full Width)"));
    titleAlignment->addItem(i18nd("breeze_kwin_deco", "Right"));
    titleAlignment->setObjectName(QStringLiteral("kcfg_TitleAlignment"));
    generalForm->addRow(i18nd("breeze_kwin_deco", "Tit&le alignment:"), titleAlignment);

    QFontComboBox *fontComboBox = new QFontComboBox(generalTab);
    fontComboBox->setObjectName(QStringLiteral("kcfg_TitleBarFont"));
    generalForm->addRow(i18nd("breeze_kwin_deco", "Font:"), fontComboBox);

    QSpinBox *fontSizeSpinBox = new QSpinBox(generalTab);
    fontSizeSpinBox->setMinimum(5);
    fontSizeSpinBox->setMaximum(50);
    fontSizeSpinBox->setSuffix(i18nd("breeze_kwin_deco", " pt"));
    fontSizeSpinBox->setObjectName(QStringLiteral("kcfg_TitleBarFontSize"));
    generalForm->addRow(i18nd("breeze_kwin_deco", "Size:"), fontSizeSpinBox);

    QComboBox *buttonType = new QComboBox(generalTab);
    buttonType->addItem(i18ndc("breeze_kwin_deco", "@item:inlistbox Button type:", "Material"));
    buttonType->addItem(i18ndc("breeze_kwin_deco", "@item:inlistbox Button type:", "MacOS"));
    buttonType->addItem(i18ndc("breeze_kwin_deco", "@item:inlistbox Button type:", "Breeze"));
    buttonType->setObjectName(QStringLiteral("kcfg_ButtonType"));
    generalForm->addRow(i18nd("breeze_kwin_deco", "Button type:"), buttonType);

    QComboBox *buttonSizes = new QComboBox(generalTab);
    buttonSizes->addItem(i18nd("breeze_kwin_deco", "Tiny"));
    buttonSizes->addItem(i18ndc("breeze_kwin_deco", "@item:inlistbox Button size:", "Small"));
    buttonSizes->addItem(i18ndc("breeze_kwin_deco", "@item:inlistbox Button size:", "Medium"));
    buttonSizes->addItem(i18ndc("breeze_kwin_deco", "@item:inlistbox Button size:", "Large"));
    buttonSizes->addItem(i18ndc("breeze_kwin_deco", "@item:inlistbox Button size:", "Very Large"));
    buttonSizes->setObjectName(QStringLiteral("kcfg_ButtonSize"));
    generalForm->addRow(i18nd("breeze_kwin_deco", "B&utton size:"), buttonSizes);

    QSpinBox *buttonSpacing = new QSpinBox(generalTab);
    buttonSpacing->setMinimum(0);
    buttonSpacing->setMaximum(100);
    buttonSpacing->setSuffix(i18nd("breeze_kwin_deco", " px"));
    buttonSpacing->setObjectName(QStringLiteral("kcfg_ButtonSpacing"));
    generalForm->addRow(i18nd("breeze_kwin_deco", "Button Spacing:"), buttonSpacing);

    QCheckBox *flatTitleBar = new QCheckBox(generalTab);
    flatTitleBar->setText(i18nd("breeze_kwin_deco", "Flat Title Bar"));
    flatTitleBar->setObjectName(QStringLiteral("kcfg_FlatTitleBar"));
    generalForm->addRow(QStringLiteral(""), flatTitleBar);

    QCheckBox *drawBackgroundGradient = new QCheckBox(generalTab);
    drawBackgroundGradient->setText(i18nd("breeze_kwin_deco", "Draw Background Gradient"));
    drawBackgroundGradient->setObjectName(QStringLiteral("kcfg_DrawBackgroundGradient"));
    generalForm->addRow(QStringLiteral(""), drawBackgroundGradient);

    QSpinBox *backgroundGradientIntensity = new QSpinBox(generalTab);
    backgroundGradientIntensity->setMinimum(0);
    backgroundGradientIntensity->setMaximum(100);
    backgroundGradientIntensity->setSuffix(i18nd("breeze_kwin_deco", " %"));
    backgroundGradientIntensity->setObjectName(QStringLiteral("kcfg_BackgroundGradientIntensity"));
    generalForm->addRow(i18nd("breeze_kwin_deco", "Background Gradient Intensity:"), backgroundGradientIntensity);

    QCheckBox *opaqueTitleBar = new QCheckBox(generalTab);
    opaqueTitleBar->setText(i18nd("breeze_kwin_deco", "Opaque Title Bar"));
    opaqueTitleBar->setObjectName(QStringLiteral("kcfg_OpaqueTitleBar"));
    generalForm->addRow(QStringLiteral(""), opaqueTitleBar);

    QDoubleSpinBox *activeOpacity = new QDoubleSpinBox(generalTab);
    activeOpacity->setMinimum(0.0);
    activeOpacity->setMaximum(1.0);
    activeOpacity->setSingleStep(0.05);
    activeOpacity->setObjectName(QStringLiteral("kcfg_ActiveOpacity"));
    generalForm->addRow(i18n("Active Opacity:"), activeOpacity);

    QDoubleSpinBox *inactiveOpacity = new QDoubleSpinBox(generalTab);
    inactiveOpacity->setMinimum(0.0);
    inactiveOpacity->setMaximum(1.0);
    inactiveOpacity->setSingleStep(0.05);
    inactiveOpacity->setObjectName(QStringLiteral("kcfg_InactiveOpacity"));
    generalForm->addRow(i18n("Inactive Opacity:"), inactiveOpacity);


    //--- Menu
    QWidget *menuTab = new QWidget(tabWidget);
    tabWidget->addTab(menuTab, i18n("Menu"));
    QFormLayout *menuForm = new QFormLayout(menuTab);
    menuTab->setLayout(menuForm);

    QLabel *menuLabel = new QLabel(menuTab);
    menuLabel->setText(i18n("To enable the Locally Integrated Menus in the titlebar:\nSystem Settings > Window Decorations > Titlebar Buttons Tab\nDrag the 'Application Menu' button to bar."));
    menuForm->addRow(QStringLiteral(""), menuLabel);

    QRadioButton *menuAlwaysShow = new QRadioButton(menuTab);
    menuAlwaysShow->setText(i18n("Always Show Menu"));
    menuAlwaysShow->setObjectName(QStringLiteral("kcfg_MenuAlwaysShow"));
    menuForm->addRow(QStringLiteral(""), menuAlwaysShow);

    // Since there's no easy way to bind this to !MenuAlwaysShow, we
    // workaround this by marking the button as checked on init.
    // When the config is loaded:
    // * If menuAlwaysShow is toggled true, this will be toggled false.
    // * If menuAlwaysShow is left false, then this remains true.
    QRadioButton *menuRevealOnHover = new QRadioButton(menuTab);
    menuRevealOnHover->setText(i18n("Reveal Menu on Hover"));
    menuRevealOnHover->setChecked(true);
    menuForm->addRow(QStringLiteral(""), menuRevealOnHover);

    QButtonGroup *menuAlwaysShowGroup = new QButtonGroup(menuTab);
    menuAlwaysShowGroup->addButton(menuAlwaysShow);
    menuAlwaysShowGroup->addButton(menuRevealOnHover);


    //--- Animations
    QWidget *animationsTab = new QWidget(tabWidget);
    tabWidget->addTab(animationsTab, i18nd("breeze_kwin_deco", "Animations"));
    QFormLayout *animationsForm = new QFormLayout(animationsTab);
    animationsTab->setLayout(animationsForm);

    QCheckBox *animationsEnabled = new QCheckBox(animationsTab);
    animationsEnabled->setText(i18nd("breeze_kwin_deco", "Enable animations"));
    animationsEnabled->setObjectName(QStringLiteral("kcfg_AnimationsEnabled"));
    animationsForm->addRow(QStringLiteral(""), animationsEnabled);

    QSpinBox *animationsDuration = new QSpinBox(animationsTab);
    animationsDuration->setMinimum(0);
    animationsDuration->setMaximum(INT_MAX);
    animationsDuration->setSuffix(i18nd("breeze_kwin_deco", " ms"));
    animationsDuration->setObjectName(QStringLiteral("kcfg_AnimationsDuration"));
    animationsForm->addRow(i18nd("breeze_kwin_deco", "Animations:"), animationsDuration);


    //--- Shadows
    QWidget *shadowTab = new QWidget(tabWidget);
    tabWidget->addTab(shadowTab, i18nd("breeze_kwin_deco", "Shadows"));
    QFormLayout *shadowForm = new QFormLayout(shadowTab);
    shadowTab->setLayout(shadowForm);

    QComboBox *shadowSizes = new QComboBox(shadowTab);
    shadowSizes->addItem(i18ndc("breeze_kwin_deco", "@item:inlistbox Shadow size:", "None"));
    shadowSizes->addItem(i18ndc("breeze_kwin_deco", "@item:inlistbox Shadow size:", "Small"));
    shadowSizes->addItem(i18ndc("breeze_kwin_deco", "@item:inlistbox Shadow size:", "Medium"));
    shadowSizes->addItem(i18ndc("breeze_kwin_deco", "@item:inlistbox Shadow size:", "Large"));
    shadowSizes->addItem(i18ndc("breeze_kwin_deco", "@item:inlistbox Shadow size:", "Very Large"));
    shadowSizes->setObjectName(QStringLiteral("kcfg_ShadowSize"));
    shadowForm->addRow(i18nd("breeze_kwin_deco", "Si&ze:"), shadowSizes);

    QSpinBox *shadowStrength = new QSpinBox(shadowTab);
    shadowStrength->setMinimum(25);
    shadowStrength->setMaximum(255);
    // shadowStrength->setSuffix(i18nd("breeze_kwin_deco", "%"));
    shadowStrength->setObjectName(QStringLiteral("kcfg_ShadowStrength"));
    shadowForm->addRow(i18ndc("breeze_kwin_deco", "strength of the shadow (from transparent to opaque)", "S&trength:"), shadowStrength);

    KColorButton *shadowColor = new KColorButton(shadowTab);
    shadowColor->setObjectName(QStringLiteral("kcfg_ShadowColor"));
    shadowForm->addRow(i18nd("breeze_kwin_deco", "Color:"), shadowColor);

    //--- Config Bindings
    skel->addItemInt(
        QStringLiteral("TitleAlignment"),
        m_titleAlignment,
        InternalSettings::AlignCenterFullWidth,
        QStringLiteral("TitleAlignment")
    );
    skel->addItemString( // TODO addItemProperty https://api.kde.org/frameworks/kconfig/html/classKCoreConfigSkeleton.html
        QStringLiteral("TitleBarFont"),
        m_titleBarFont,
        "Sans,11,-1,5,50,0,0,0,0,0",
        QStringLiteral("TitleBarFont")
    );
    skel->addItemInt(
        QStringLiteral("TitleBarFontSize"),
        m_titleBarFontSize,
        11,
        QStringLiteral("TitleBarFontSize")
    );
    skel->addItemInt(
        QStringLiteral("ButtonType"),
        m_buttonType,
        InternalSettings::ButtonMaterial,
        QStringLiteral("ButtonType")
    );
    skel->addItemInt(
        QStringLiteral("ButtonSize"),
        m_buttonSize,
        InternalSettings::ButtonDefault,
        QStringLiteral("ButtonSize")
    );
    skel->addItemInt(
        QStringLiteral("ButtonSpacing"),
        m_buttonSpacing,
        6,
        QStringLiteral("ButtonSpacing")
    );
    skel->addItemBool(
        QStringLiteral("DrawBackgroundGradient"),
        m_drawBackgroundGradient,
        true,
        QStringLiteral("DrawBackgroundGradient")
    );
    skel->addItemInt(
        QStringLiteral("BackgroundGradientIntensity"),
        m_backgroundGradientIntensity,
        20,
        QStringLiteral("BackgroundGradientIntensity")
    );
    skel->addItemBool(
        QStringLiteral("FlatTitleBar"),
        m_flatTitleBar,
        false,
        QStringLiteral("FlatTitleBar")
    );
    skel->addItemBool(
        QStringLiteral("OpaqueTitleBar"),
        m_opaqueTitleBar,
        false,
        QStringLiteral("OpaqueTitleBar")
    );
    skel->addItemBool(
        QStringLiteral("AnimationsEnabled"),
        m_animationsEnabled,
        true,
        QStringLiteral("AnimationsEnabled")
    );
    skel->addItemDouble(
        QStringLiteral("ActiveOpacity"),
        m_activeOpacity,
        0.75,
        QStringLiteral("ActiveOpacity")
    );
    skel->addItemDouble(
        QStringLiteral("InactiveOpacity"),
        m_inactiveOpacity,
        0.85,
        QStringLiteral("InactiveOpacity")
    );
    skel->addItemBool(
        QStringLiteral("MenuAlwaysShow"),
        m_menuAlwaysShow,
        true,
        QStringLiteral("MenuAlwaysShow")
    );
    skel->addItemBool(
        QStringLiteral("AnimationsEnabled"),
        m_animationsEnabled,
        true,
        QStringLiteral("AnimationsEnabled")
    );
    skel->addItemInt(
        QStringLiteral("AnimationsDuration"),
        m_animationsDuration,
        150,
        QStringLiteral("AnimationsDuration")
    );
    skel->addItemInt(
        QStringLiteral("ShadowSize"),
        m_shadowSize,
        InternalSettings::ShadowVeryLarge,
        QStringLiteral("ShadowSize")
    );
    skel->addItemInt(
        QStringLiteral("ShadowStrength"),
        m_shadowStrength,
        255,
        QStringLiteral("ShadowStrength")
    );
    skel->addItem(new KConfigSkeleton::ItemColor(
        skel->currentGroup(),
        QStringLiteral("ShadowColor"),
        m_shadowColor,
        QColor(33, 33, 33)
    ), QStringLiteral("ShadowColor"));

    //---
    addConfig(skel, this);
}

} // namespace Material

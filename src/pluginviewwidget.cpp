/*
#
# Natron Plug-in Manager
#
# Copyright (c) Ole-André Rodlie. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>
#
*/

#include "pluginviewwidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFile>
#include <QIcon>
#include <QPixmap>
#include <QRegExp>

PluginViewWidget::PluginViewWidget(QWidget *parent,
                                   Plugins *plugins,
                                   QSize iconSize)
    : QWidget(parent)
    , _plugins(plugins)
    , _goBackButton(nullptr)
    , _pluginIconLabel(nullptr)
    , _pluginTitleLabel(nullptr)
    , _pluginGroupLabel(nullptr)
    , _pluginDescBrowser(nullptr)
    , _iconSize(iconSize)
    , _installButton(nullptr)
    , _removeButton(nullptr)
    , _updateButton(nullptr)
{
    setObjectName("PluginViewWidget");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QWidget *headerWidget = new QWidget(this);
    headerWidget->setSizePolicy(QSizePolicy::Expanding,
                                QSizePolicy::Fixed);

    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);

    _goBackButton = new QPushButton(tr("❮"), this);
    _goBackButton->setSizePolicy(QSizePolicy::Fixed,
                                 QSizePolicy::Expanding);
    _goBackButton->setObjectName("GoBackButton");
    connect(_goBackButton,
            SIGNAL(released()),
            this,
            SLOT(handleGoBackButton()));

    _pluginIconLabel = new QLabel(this);
    _pluginIconLabel->setMinimumSize(_iconSize);
    _pluginIconLabel->setMaximumSize(_iconSize);

    _pluginIconLabel->setPixmap(QIcon(QString(DEFAULT_ICON)).pixmap(_iconSize).scaled(iconSize,
                                                                                      Qt::KeepAspectRatio,
                                                                                      Qt::SmoothTransformation));

    QWidget *pluginHeaderWidget = new QWidget(this);
    pluginHeaderWidget->setObjectName("PluginViewHeaderWidget");
    QVBoxLayout *pluginHeaderLayout = new QVBoxLayout(pluginHeaderWidget);

    _pluginTitleLabel = new QLabel(tr("Title"), this);
    _pluginTitleLabel->setObjectName("PluginViewTitleLabel");

    _pluginGroupLabel = new QLabel(tr("Group"), this);
    _pluginGroupLabel->setObjectName("PluginViewGroupLabel");

    _installButton = new QPushButton(tr("Install"), this);
    _removeButton = new QPushButton(tr("Remove"), this);
    _updateButton = new QPushButton(tr("Update"), this);

    _installButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _removeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _updateButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    _installButton->setProperty("InstallButton", true);
    _removeButton->setProperty("RemoveButton", true);
    _updateButton->setProperty("UpdateButton", true);

    _installButton->setEnabled(false);
    _removeButton->setEnabled(false);
    _updateButton->setEnabled(false);

    _installButton->setHidden(true);
    _removeButton->setHidden(true);
    _updateButton->setHidden(true);

    connect(_installButton,
            SIGNAL(released()),
            this,
            SLOT(handleInstallButtonReleased()));
    connect(_removeButton,
            SIGNAL(released()),
            this,
            SLOT(handleRemoveButtonReleased()));
    connect(_updateButton,
            SIGNAL(released()),
            this,
            SLOT(handleUpdateButtonReleased()));

    QWidget *pluginButtonsWidget = new QWidget(this);
    pluginButtonsWidget->setObjectName("PluginViewButtonsWidget");
    pluginButtonsWidget->setSizePolicy(QSizePolicy::Fixed,
                                       QSizePolicy::Expanding);
    QVBoxLayout *pluginButtonsLayout = new QVBoxLayout(pluginButtonsWidget);

    pluginButtonsLayout->addStretch();
    pluginButtonsLayout->addWidget(_installButton);
    pluginButtonsLayout->addWidget(_updateButton);
    pluginButtonsLayout->addWidget(_removeButton);
    pluginButtonsLayout->addStretch();

    pluginHeaderLayout->addStretch();
    pluginHeaderLayout->addWidget(_pluginTitleLabel);
    pluginHeaderLayout->addWidget(_pluginGroupLabel);
    pluginHeaderLayout->addStretch();

    headerLayout->addWidget(_goBackButton);
    headerLayout->addWidget(_pluginIconLabel);
    headerLayout->addWidget(pluginHeaderWidget);
    headerLayout->addWidget(pluginButtonsWidget);

    _pluginDescBrowser = new QTextBrowser(this);
    _pluginDescBrowser->setObjectName("PluginViewBrowser");
    _pluginDescBrowser->setOpenLinks(true);
    _pluginDescBrowser->setOpenExternalLinks(true);
    _pluginDescBrowser->setReadOnly(true);

    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(_pluginDescBrowser);
}

void PluginViewWidget::showPlugin(const QString &id)
{
    if (!_plugins || id.isEmpty()) { return; }
    Plugins::PluginSpecs plugin = _plugins->getPlugin(id);
    if (!_plugins->isValidPlugin(plugin)) { return; }
    _id = id;

    _pluginTitleLabel->setText(plugin.label);
    _pluginGroupLabel->setText(plugin.group);

    _pluginIconLabel->setPixmap(QIcon(QString(DEFAULT_ICON)).pixmap(_iconSize).scaled(_iconSize,
                                                                                      Qt::KeepAspectRatio,
                                                                                      Qt::SmoothTransformation));
    QString pluginIconPath = QString("%1/%2").arg(plugin.path, plugin.icon);
    if (!plugin.icon.isEmpty() && QFile::exists(pluginIconPath)) {
        QPixmap pluginPixmap = QIcon(pluginIconPath).pixmap(_iconSize).scaled(_iconSize,
                                                                             Qt::KeepAspectRatio,
                                                                             Qt::SmoothTransformation);
        if (!pluginPixmap.isNull()) { _pluginIconLabel->setPixmap(pluginPixmap); }
    }

    QString desc = plugin.desc.replace("\\n", "<br>").replace("\\", "").simplified();
    if (desc.isEmpty()) {
        desc = QString("<p>%1.</p>").arg(tr("No description available"));
    }

    desc = desc.replace(QRegExp("((?:https?|ftp)://\\S+)"),
                        "<a href=\"\\1\">\\1</a>");

    _pluginDescBrowser->setHtml(desc);

    if (_plugins->hasAvailablePlugin(plugin.id)) {
        setPluginStatus(plugin.id, Plugins::NATRON_PLUGIN_TYPE_AVAILABLE);
    } else if (_plugins->hasInstalledPlugin(plugin.id)) {
        setPluginStatus(plugin.id, Plugins::NATRON_PLUGIN_TYPE_INSTALLED);
    }
}

void PluginViewWidget::setPluginStatus(const QString &id,
                                       int type)
{
    Plugins::PluginSpecs plugin = _plugins->getPlugin(id);
    if (!_plugins->isValidPlugin(plugin)) { return; }
    if (plugin.id != id || id != _id) { return; }
    switch(type) {
    case Plugins::NATRON_PLUGIN_TYPE_AVAILABLE:
        _installButton->setEnabled(true);
        _installButton->setHidden(false);
        _removeButton->setEnabled(false);
        _removeButton->setHidden(true);
        _updateButton->setEnabled(false);
        _updateButton->setHidden(true);
        break;
    case Plugins::NATRON_PLUGIN_TYPE_INSTALLED:
        _installButton->setEnabled(false);
        _installButton->setHidden(true);
        _removeButton->setEnabled(true);
        _removeButton->setHidden(false);
        _updateButton->setEnabled(false);
        _updateButton->setHidden(true);
        break;
    case Plugins::NATRON_PLUGIN_TYPE_UPDATE:
        _installButton->setEnabled(false);
        _installButton->setHidden(true);
        _removeButton->setEnabled(true);
        _removeButton->setHidden(false);
        _updateButton->setEnabled(true);
        _updateButton->setHidden(false);
        break;
    default:
        _installButton->setEnabled(false);
        _installButton->setHidden(true);
        _removeButton->setEnabled(false);
        _removeButton->setHidden(true);
        _updateButton->setEnabled(false);
        _updateButton->setHidden(true);
        break;
    }
}

void PluginViewWidget::handleGoBackButton()
{
    emit goBack();
}

void PluginViewWidget::handleInstallButtonReleased()
{
    emit installPlugin(_id);
}

void PluginViewWidget::handleRemoveButtonReleased()
{
    emit removePlugin(_id);
}

void PluginViewWidget::handleUpdateButtonReleased()
{
    emit updatePlugin(_id);
}
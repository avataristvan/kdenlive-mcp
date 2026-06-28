// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "ProjectCapability.h"
#include "core.h"
#include "doc/kdenlivedoc.h"
#include "kdenlivesettings.h"
#include "mainwindow.h"
#include "project/projectmanager.h"
#include "timeline2/view/timelinewidget.h"
#include <QUrl>

ProjectCapability::ProjectCapability(MainWindow *window)
    : m_window(window)
{
}

QString ProjectCapability::newProject(const QString &)
{
    pCore->projectManager()->newFile(false);
    if (pCore->currentDoc()) {
        return pCore->currentDoc()->url().fileName();
    }
    return {};
}

bool ProjectCapability::openProject(const QString &filePath)
{
    if (filePath.isEmpty()) return false;
    pCore->projectManager()->openFile(QUrl::fromLocalFile(filePath));
    return pCore->currentDoc() != nullptr;
}

bool ProjectCapability::saveProject()
{
    if (!pCore->currentDoc()) return false;
    return pCore->projectManager()->saveFile();
}

bool ProjectCapability::saveProjectAs(const QString &filePath)
{
    if (!pCore->currentDoc() || filePath.isEmpty()) return false;
    return pCore->projectManager()->saveFileAs(filePath);
}

QString ProjectCapability::getProjectName()
{
    if (!pCore->currentDoc()) return {};
    return pCore->currentDoc()->url().fileName();
}

QString ProjectCapability::getProjectPath()
{
    if (!pCore->currentDoc()) return {};
    return pCore->currentDoc()->url().toLocalFile();
}

double ProjectCapability::getProjectFps()
{
    return pCore->getCurrentFps();
}

int ProjectCapability::getProjectResolutionWidth()
{
    if (!pCore->currentDoc()) return 0;
    return pCore->currentDoc()->width();
}

int ProjectCapability::getProjectResolutionHeight()
{
    if (!pCore->currentDoc()) return 0;
    return pCore->currentDoc()->height();
}

QString ProjectCapability::getProjectProperty(const QString &key)
{
    if (!pCore->currentDoc()) return {};
    return pCore->currentDoc()->getDocumentProperty(key);
}

bool ProjectCapability::setProjectProperty(const QString &key, const QString &value)
{
    if (!pCore->currentDoc()) return false;
    pCore->currentDoc()->setDocumentProperty(key, value);
    return true;
}

int ProjectCapability::getProjectDuration()
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return 0;
    return tl->model()->duration();
}

QString ProjectCapability::getProjectColorSpace()
{
    if (!pCore->currentDoc()) return {};
    return pCore->currentDoc()->getDocumentProperty(QStringLiteral("color_space"), QStringLiteral("709"));
}

bool ProjectCapability::setProjectColorSpace(const QString &colorSpace)
{
    if (!pCore->currentDoc()) return false;
    pCore->currentDoc()->setDocumentProperty(QStringLiteral("color_space"), colorSpace);
    return true;
}

int ProjectCapability::getProjectAudioSampleRate()
{
    if (!pCore->currentDoc()) return 0;
    return KdenliveSettings::audiocapturesamplerate();
}

// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "PlaybackCapability.h"
#include "bin/model/markerlistmodel.hpp"
#include "core.h"
#include "definitions.h"
#include "doc/kdenlivedoc.h"
#include "mainwindow.h"
#include "monitor/monitor.h"
#include "monitor/monitormanager.h"
#include "monitor/monitorproxy.h"
#include "profiles/profilemodel.hpp"
#include "profiles/profilerepository.hpp"
#include "timeline2/model/timelinemodel.hpp"
#include "timeline2/view/timelinecontroller.h"
#include "timeline2/view/timelinewidget.h"

PlaybackCapability::PlaybackCapability(MainWindow *window)
    : m_window(window)
{
}

// ── Render ────────────────────────────────────────────────────────────────────

bool PlaybackCapability::renderWithParams(const QString &, const QString &, int, int, const QStringList &, const QStringList &)
{
    // Not implemented: RenderPresetRepository API is not yet public
    return false;
}

QStringList PlaybackCapability::getRenderPresets()
{
    // Not implemented: RenderPresetRepository API is not yet public
    return {};
}

QVariantList PlaybackCapability::getRenderJobs()
{
    // Not implemented: render job list API is not yet public
    return {};
}

bool PlaybackCapability::abortRenderJob(const QString &outputPath)
{
    if (outputPath.isEmpty()) return false;
    m_window->abortRenderJob(outputPath);
    return true;
}

// ── Project Profile ───────────────────────────────────────────────────────────

bool PlaybackCapability::setProjectProfile(int width, int height, int fpsNum, int fpsDen)
{
    if (!pCore->currentDoc()) return false;
    if (width <= 0 || height <= 0 || fpsNum <= 0 || fpsDen <= 0) return false;

    std::unique_ptr<ProfileParam> newProfile(new ProfileParam(
        width, height, fpsNum, fpsDen,
        width, height,  // display aspect ratio (square pixels assumed)
        1, 1,           // sample aspect ratio
        709,            // colorspace (ITU-R 709)
        false           // progressive
    ));
    newProfile->m_description = QStringLiteral("%1x%2 %3/%4fps").arg(width).arg(height).arg(fpsNum).arg(fpsDen);

    QString matchingPath = ProfileRepository::get()->findMatchingProfile(newProfile.get());
    if (matchingPath.isEmpty()) {
        matchingPath = ProfileRepository::get()->saveProfile(newProfile.get());
    }
    if (matchingPath.isEmpty()) return false;

    pCore->setCurrentProfile(matchingPath);
    pCore->currentDoc()->resetProfile(true);
    return true;
}

// ── Copy/Cut/Paste ────────────────────────────────────────────────────────────

int PlaybackCapability::copyClips()
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->controller()) return -1;
    return tl->controller()->copyItem();
}

bool PlaybackCapability::cutClips()
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->controller()) return false;
    tl->controller()->cutItem();
    return true;
}

bool PlaybackCapability::pasteClips(int position, int trackId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->controller()) return false;
    return tl->controller()->pasteItem(position, trackId);
}

// ── Playback ──────────────────────────────────────────────────────────────────

void PlaybackCapability::seek(int frame)
{
    if (!pCore->monitorManager()) return;
    auto *monitor = pCore->monitorManager()->projectMonitor();
    if (monitor) {
        monitor->slotSeek(frame);
    }
}

int PlaybackCapability::getPosition()
{
    if (!pCore->monitorManager()) return -1;
    auto *monitor = pCore->monitorManager()->projectMonitor();
    if (monitor) {
        return monitor->position();
    }
    return -1;
}

void PlaybackCapability::play()
{
    if (pCore->monitorManager()) {
        pCore->monitorManager()->slotPlay();
    }
}

void PlaybackCapability::pause()
{
    if (pCore->monitorManager()) {
        pCore->monitorManager()->slotPause();
    }
}

bool PlaybackCapability::setPlaybackSpeed(double speed)
{
    if (!pCore->monitorManager()) return false;
    auto *monitor = pCore->monitorManager()->projectMonitor();
    if (!monitor) return false;
    if (qFuzzyIsNull(speed)) {
        monitor->slotPlay();
        return true;
    }
    if (speed > 0) {
        monitor->slotForward(speed);
    } else {
        monitor->slotRewind(-speed);
    }
    return true;
}

double PlaybackCapability::getPlaybackSpeed()
{
    if (!pCore->monitorManager()) return 0.0;
    auto *monitor = pCore->monitorManager()->projectMonitor();
    if (!monitor) return 0.0;
    auto *proxy = monitor->getControllerProxy();
    if (!proxy) return 0.0;
    return proxy->property("speed").toDouble();
}

// ── Navigation ────────────────────────────────────────────────────────────────

int PlaybackCapability::goToNextMarker()
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return -1;

    auto guideModel = tl->model()->getGuideModel();
    if (!guideModel) return -1;

    int currentPos = pCore->getMonitorPosition();
    const auto guides = guideModel->getAllMarkers();
    for (const auto &marker : guides) {
        int frame = marker.time().frames(pCore->getCurrentFps());
        if (frame > currentPos) {
            pCore->seekMonitor(Kdenlive::ProjectMonitor, frame);
            return frame;
        }
    }
    return -1;
}

int PlaybackCapability::goToPreviousMarker()
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return -1;

    auto guideModel = tl->model()->getGuideModel();
    if (!guideModel) return -1;

    int currentPos = pCore->getMonitorPosition();
    int prevFrame = -1;
    for (const auto &marker : guideModel->getAllMarkers()) {
        int frame = marker.time().frames(pCore->getCurrentFps());
        if (frame < currentPos) {
            prevFrame = frame;
        } else {
            break;
        }
    }
    if (prevFrame >= 0) {
        pCore->seekMonitor(Kdenlive::ProjectMonitor, prevFrame);
    }
    return prevFrame;
}

int PlaybackCapability::goToNextEdit()
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return -1;

    int currentPos = pCore->getMonitorPosition();
    int nextEdit = -1;
    auto model = tl->model();

    QList<int> allTracks;
    allTracks.append(model->getTracksIds(false));
    allTracks.append(model->getTracksIds(true));
    for (int tid : allTracks) {
        std::unordered_set<int> clipIds = model->getItemsInRange(tid, 0, -1, false);
        for (int cid : clipIds) {
            if (!model->isClip(cid)) continue;
            int pos = model->getClipPosition(cid);
            int endPos = pos + model->getClipPlaytime(cid);
            if (pos > currentPos && (nextEdit < 0 || pos < nextEdit)) {
                nextEdit = pos;
            }
            if (endPos > currentPos && (nextEdit < 0 || endPos < nextEdit)) {
                nextEdit = endPos;
            }
        }
    }
    if (nextEdit >= 0) {
        pCore->seekMonitor(Kdenlive::ProjectMonitor, nextEdit);
    }
    return nextEdit;
}

int PlaybackCapability::goToPreviousEdit()
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return -1;

    int currentPos = pCore->getMonitorPosition();
    int prevEdit = -1;
    auto model = tl->model();

    QList<int> allTracks;
    allTracks.append(model->getTracksIds(false));
    allTracks.append(model->getTracksIds(true));
    for (int tid : allTracks) {
        std::unordered_set<int> clipIds = model->getItemsInRange(tid, 0, -1, false);
        for (int cid : clipIds) {
            if (!model->isClip(cid)) continue;
            int pos = model->getClipPosition(cid);
            int endPos = pos + model->getClipPlaytime(cid);
            if (pos < currentPos && pos > prevEdit) {
                prevEdit = pos;
            }
            if (endPos < currentPos && endPos > prevEdit) {
                prevEdit = endPos;
            }
        }
    }
    if (prevEdit >= 0) {
        pCore->seekMonitor(Kdenlive::ProjectMonitor, prevEdit);
    }
    return prevEdit;
}

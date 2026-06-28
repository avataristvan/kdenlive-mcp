// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "MarkersCapability.h"
#include "bin/model/markerlistmodel.hpp"
#include "bin/projectclip.h"
#include "bin/projectitemmodel.h"
#include "core.h"
#include "definitions.h"
#include "mainwindow.h"
#include "timeline2/model/timelinemodel.hpp"
#include "timeline2/view/timelinewidget.h"
#include "utils/gentime.h"

MarkersCapability::MarkersCapability(MainWindow *window)
    : m_window(window)
{
}

// ── Timeline Guides ──────────────────────────────────────────────────────────

bool MarkersCapability::addGuide(int frame, const QString &comment, int category)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;

    auto guideModel = tl->model()->getGuideModel();
    if (!guideModel) return false;

    GenTime pos(frame, pCore->getCurrentFps());
    return guideModel->addMarker(pos, comment, category);
}

QVariantList MarkersCapability::getGuides()
{
    QVariantList result;
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return result;

    auto guideModel = tl->model()->getGuideModel();
    if (!guideModel) return result;

    const double fps = pCore->getCurrentFps();
    for (const CommentedTime &marker : guideModel->getAllMarkers()) {
        QVariantMap m;
        m[QStringLiteral("frame")] = marker.time().frames(fps);
        m[QStringLiteral("comment")] = marker.comment();
        m[QStringLiteral("category")] = marker.markerType();
        result.append(m);
    }
    return result;
}

bool MarkersCapability::deleteGuide(int frame)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;

    auto guideModel = tl->model()->getGuideModel();
    if (!guideModel) return false;

    GenTime pos(frame, pCore->getCurrentFps());
    return guideModel->removeMarker(pos);
}

bool MarkersCapability::deleteGuidesByCategory(int category)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;

    auto guideModel = tl->model()->getGuideModel();
    if (!guideModel) return false;

    bool anyDeleted = false;
    for (const CommentedTime &marker : guideModel->getAllMarkers()) {
        if (marker.markerType() == category) {
            if (guideModel->removeMarker(marker.time())) {
                anyDeleted = true;
            }
        }
    }
    return anyDeleted;
}

// ── Clip-level Markers ───────────────────────────────────────────────────────

bool MarkersCapability::addClipMarker(const QString &binId, int frame, const QString &comment, int category)
{
    auto clip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!clip) return false;

    auto markerModel = clip->markerModel();
    if (!markerModel) return false;

    GenTime pos(frame, pCore->getCurrentFps());
    return markerModel->addMarker(pos, comment, category);
}

QVariantList MarkersCapability::getClipMarkers(const QString &binId)
{
    QVariantList result;
    auto clip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!clip) return result;

    auto markerModel = clip->markerModel();
    if (!markerModel) return result;

    const double fps = pCore->getCurrentFps();
    for (const CommentedTime &marker : markerModel->getAllMarkers()) {
        QVariantMap m;
        m[QStringLiteral("frame")] = marker.time().frames(fps);
        m[QStringLiteral("comment")] = marker.comment();
        m[QStringLiteral("category")] = marker.markerType();
        result.append(m);
    }
    return result;
}

bool MarkersCapability::deleteClipMarker(const QString &binId, int frame)
{
    auto clip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!clip) return false;

    auto markerModel = clip->markerModel();
    if (!markerModel) return false;

    GenTime pos(frame, pCore->getCurrentFps());
    return markerModel->removeMarker(pos);
}

bool MarkersCapability::deleteClipMarkersByCategory(const QString &binId, int category)
{
    auto clip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!clip) return false;

    auto markerModel = clip->markerModel();
    if (!markerModel) return false;

    bool anyDeleted = false;
    for (const CommentedTime &marker : markerModel->getAllMarkers()) {
        if (marker.markerType() == category) {
            if (markerModel->removeMarker(marker.time())) {
                anyDeleted = true;
            }
        }
    }
    return anyDeleted;
}

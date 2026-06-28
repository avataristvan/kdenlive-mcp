// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "TimelineCapability.h"
#include "bin/abstractprojectitem.h"
#include "bin/bin.h"
#include "bin/projectclip.h"
#include "bin/projectitemmodel.h"
#include "core.h"
#include "mainwindow.h"
#include "timeline2/model/timelinefunctions.hpp"
#include "timeline2/model/timelineitemmodel.hpp"
#include "timeline2/model/timelinemodel.hpp"
#include "timeline2/view/timelinecontroller.h"
#include "timeline2/view/timelinewidget.h"
#include <KLocalizedString>

TimelineCapability::TimelineCapability(MainWindow *window)
    : m_window(window)
{
}

int TimelineCapability::getTrackCount(const QString &trackType)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return 0;

    QPair<int, int> counts = tl->model()->getAVtracksCount();
    if (trackType == QStringLiteral("audio")) {
        return counts.first;
    }
    return counts.second;
}

QVariantMap TimelineCapability::getTrackInfo(int trackIndex)
{
    QVariantMap info;
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return info;

    auto model = tl->model();
    QList<int> trackIds = model->getTracksIds(false);
    QList<int> audioIds = model->getTracksIds(true);
    QList<int> allIds;
    allIds.append(trackIds);
    allIds.append(audioIds);

    if (trackIndex < 0 || trackIndex >= allIds.size()) return info;

    int trackId = allIds.at(trackIndex);
    info[QStringLiteral("id")] = trackId;
    info[QStringLiteral("name")] = model->getTrackTagById(trackId);
    info[QStringLiteral("audio")] = model->isAudioTrack(trackId);
    info[QStringLiteral("position")] = model->getTrackPosition(trackId);
    return info;
}

QVariantList TimelineCapability::getAllTracksInfo()
{
    QVariantList result;
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return result;

    auto model = tl->model();
    QList<int> videoIds = model->getTracksIds(false);
    QList<int> audioIds = model->getTracksIds(true);

    for (int trackId : videoIds) {
        QVariantMap info;
        info[QStringLiteral("id")] = trackId;
        info[QStringLiteral("name")] = model->getTrackTagById(trackId);
        info[QStringLiteral("audio")] = false;
        info[QStringLiteral("position")] = model->getTrackPosition(trackId);
        int hide = model->getTrackProperty(trackId, QStringLiteral("hide")).toInt();
        info[QStringLiteral("mute")] = bool(hide & 2);
        info[QStringLiteral("hidden")] = bool(hide & 1);
        result.append(info);
    }
    for (int trackId : audioIds) {
        QVariantMap info;
        info[QStringLiteral("id")] = trackId;
        info[QStringLiteral("name")] = model->getTrackTagById(trackId);
        info[QStringLiteral("audio")] = true;
        info[QStringLiteral("position")] = model->getTrackPosition(trackId);
        int hide = model->getTrackProperty(trackId, QStringLiteral("hide")).toInt();
        info[QStringLiteral("mute")] = bool(hide & 2);
        info[QStringLiteral("hidden")] = bool(hide & 1);
        result.append(info);
    }
    return result;
}

int TimelineCapability::addTrack(const QString &name, bool audioTrack)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return -1;

    int newId = -1;
    int pos = tl->model()->getTracksCount();
    bool success = tl->model()->requestTrackInsertion(pos, newId, name, audioTrack);
    return success ? newId : -1;
}

bool TimelineCapability::deleteTrack(int trackId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    return tl->model()->requestTrackDeletion(trackId);
}

bool TimelineCapability::insertSpace(int trackId, int position, int duration, bool allTracks)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model() || !tl->controller()) return false;
    if (duration <= 0) return false;

    int targetTrack = allTracks ? -1 : trackId;
    int cid = tl->controller()->requestSpacerStartOperation(targetTrack, position);
    if (cid == -1) return false;

    int start = tl->model()->getItemPosition(cid);
    return tl->controller()->requestSpacerEndOperation(cid, start, start + duration, targetTrack, {}, -1);
}

bool TimelineCapability::removeSpace(int trackId, int position, bool allTracks)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    return TimelineFunctions::requestDeleteBlankAt(tl->model(), trackId, position, allTracks);
}

int TimelineCapability::insertClip(const QString &binClipId, int trackId, int position)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return -1;

    int newClipId = -1;
    bool success = tl->model()->requestClipInsertion(binClipId, trackId, position, newClipId,
                                                     true,  // logUndo
                                                     true,  // refreshView
                                                     false  // useTargets
    );
    return success ? newClipId : -1;
}

QVariantList TimelineCapability::insertClipsSequentially(const QStringList &binClipIds, int trackId, int startPosition)
{
    QVariantList result;
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return result;

    int position = startPosition;
    for (const QString &binId : binClipIds) {
        int newClipId = -1;
        bool success = tl->model()->requestClipInsertion(binId, trackId, position, newClipId, true, true, false);
        if (success && newClipId >= 0) {
            result.append(newClipId);
            position += tl->model()->getClipPlaytime(newClipId);
        } else {
            result.append(-1);
        }
    }
    return result;
}

bool TimelineCapability::moveClip(int clipId, int trackId, int position)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;

    return tl->model()->requestClipMove(clipId, trackId, position,
                                        true,  // moveMirrorTracks
                                        true,  // updateView
                                        true); // logUndo
}

int TimelineCapability::resizeClip(int clipId, int newDuration, bool fromRight)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return -1;

    return tl->model()->requestItemResize(clipId, newDuration, fromRight, true);
}

bool TimelineCapability::deleteTimelineClip(int clipId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;

    return tl->model()->requestItemDeletion(clipId, true);
}

QVariantList TimelineCapability::getClipsOnTrack(int trackId)
{
    QVariantList result;
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return result;

    auto model = tl->model();
    std::unordered_set<int> clipIds = model->getItemsInRange(trackId, 0, -1, false);
    for (int cid : clipIds) {
        if (!model->isClip(cid)) continue;
        QVariantMap info;
        info[QStringLiteral("id")] = cid;
        info[QStringLiteral("position")] = model->getClipPosition(cid);
        info[QStringLiteral("duration")] = model->getClipPlaytime(cid);
        info[QStringLiteral("trackId")] = model->getClipTrackId(cid);
        info[QStringLiteral("in")] = model->getClipIn(cid);
        QString binId = model->getClipBinId(cid);
        info[QStringLiteral("binId")] = binId;
        if (!binId.isEmpty() && pCore->bin()) {
            info[QStringLiteral("name")] = pCore->bin()->getBinClipName(binId);
        }
        auto clip = pCore->projectItemModel()->getClipByBinID(binId);
        if (clip) {
            info[QStringLiteral("url")] = clip->clipUrl();
        }
        result.append(info);
    }
    return result;
}

QVariantMap TimelineCapability::getTimelineClipInfo(int clipId)
{
    QVariantMap info;
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return info;

    auto model = tl->model();
    if (!model->isClip(clipId)) return info;

    info[QStringLiteral("id")] = clipId;
    info[QStringLiteral("position")] = model->getClipPosition(clipId);
    info[QStringLiteral("duration")] = model->getClipPlaytime(clipId);
    info[QStringLiteral("trackId")] = model->getClipTrackId(clipId);
    auto inOut = model->getClipInOut(clipId);
    info[QStringLiteral("in")] = inOut.first;
    info[QStringLiteral("out")] = inOut.second;
    QString binId = model->getClipBinId(clipId);
    info[QStringLiteral("binId")] = binId;
    if (!binId.isEmpty() && pCore->bin()) {
        info[QStringLiteral("name")] = pCore->bin()->getBinClipName(binId);
    }
    auto clip = pCore->projectItemModel()->getClipByBinID(binId);
    if (clip) {
        info[QStringLiteral("url")] = clip->clipUrl();
        info[QStringLiteral("maxDuration")] = (int)clip->frameDuration();
    }
    return info;
}

bool TimelineCapability::slipClip(int clipId, int offset)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->controller()) return false;
    auto model = tl->model();
    if (!model || !model->isClip(clipId)) return false;

    QList<int> prevSel = tl->controller()->selection();
    tl->controller()->selectItems({clipId});
    model->requestSlipSelection(offset, true);
    if (prevSel.isEmpty()) {
        tl->controller()->selectItems(QList<int>());
    } else {
        tl->controller()->selectItems(prevSel);
    }
    return true;
}

bool TimelineCapability::cutClip(int clipId, int position)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    return TimelineFunctions::requestClipCut(tl->model(), clipId, position);
}

bool TimelineCapability::rippleDelete(int clipId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    int position = tl->model()->getClipPosition(clipId);
    int duration = tl->model()->getClipPlaytime(clipId);

    Fun undo = []() { return true; };
    Fun redo = []() { return true; };

    bool deleted = tl->model()->requestItemDeletion(clipId, undo, redo);
    if (!deleted) return false;

    QPoint zone(position, position + duration);
    TimelineFunctions::removeSpace(tl->model(), zone, undo, redo);
    pCore->pushUndo(undo, redo, i18n("Ripple delete"));
    return true;
}

bool TimelineCapability::rippleTrim(int clipId, int delta, bool fromRight)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    int position = tl->model()->getClipPosition(clipId);
    int duration = tl->model()->getClipPlaytime(clipId);
    int newDuration = duration + delta;
    if (newDuration < 1) return false;

    Fun undo = []() { return true; };
    Fun redo = []() { return true; };

    bool ok = tl->model()->requestItemResize(clipId, newDuration, fromRight, false, undo, redo);
    if (!ok) return false;

    if (delta < 0) {
        int gapStart = position + newDuration;
        if (!fromRight) {
            int newPosition = tl->model()->getClipPosition(clipId);
            gapStart = newPosition + newDuration;
        }
        QPoint zone(gapStart, gapStart + (-delta));
        TimelineFunctions::removeSpace(tl->model(), zone, undo, redo);
    } else if (delta > 0) {
        int spaceStart = position + duration;
        if (!fromRight) {
            spaceStart = position;
        }
        QPoint zone(spaceStart, spaceStart + delta);
        TimelineFunctions::requestInsertSpace(tl->model(), zone, undo, redo);
    }

    pCore->pushUndo(undo, redo, i18n("Ripple trim"));
    return true;
}

bool TimelineCapability::rollEdit(int clipId, int delta)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    auto model = tl->model();
    int trackId = model->getClipTrackId(clipId);
    int position = model->getClipPosition(clipId);
    int duration = model->getClipPlaytime(clipId);
    int clipEnd = position + duration;

    int nextClipId = model->getClipByPosition(trackId, clipEnd);
    if (nextClipId < 0 || !model->isClip(nextClipId)) return false;

    int nextDuration = model->getClipPlaytime(nextClipId);
    int newDuration = duration + delta;
    int newNextDuration = nextDuration - delta;
    if (newDuration < 1 || newNextDuration < 1) return false;

    Fun undo = []() { return true; };
    Fun redo = []() { return true; };

    bool ok1 = model->requestItemResize(clipId, newDuration, true, false, undo, redo);
    if (!ok1) return false;

    bool ok2 = model->requestItemResize(nextClipId, newNextDuration, false, false, undo, redo);
    if (!ok2) {
        undo();
        return false;
    }

    pCore->pushUndo(undo, redo, i18n("Roll edit"));
    return true;
}

bool TimelineCapability::slideEdit(int clipId, int delta)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    auto model = tl->model();
    int trackId = model->getClipTrackId(clipId);
    int position = model->getClipPosition(clipId);
    int duration = model->getClipPlaytime(clipId);

    int prevClipId = (position > 0) ? model->getClipByPosition(trackId, position - 1) : -1;
    int nextClipId = model->getClipByPosition(trackId, position + duration);

    if (prevClipId < 0 && nextClipId < 0) return false;

    if (prevClipId >= 0 && model->isClip(prevClipId)) {
        int prevDur = model->getClipPlaytime(prevClipId);
        if (prevDur + delta < 1) return false;
    }
    if (nextClipId >= 0 && model->isClip(nextClipId)) {
        int nextDur = model->getClipPlaytime(nextClipId);
        if (nextDur - delta < 1) return false;
    }

    Fun undo = []() { return true; };
    Fun redo = []() { return true; };

    if (prevClipId >= 0 && model->isClip(prevClipId)) {
        int newPrevDur = model->getClipPlaytime(prevClipId) + delta;
        bool ok = model->requestItemResize(prevClipId, newPrevDur, true, false, undo, redo);
        if (!ok) return false;
    }

    auto moveResult = model->requestClipMove(clipId, trackId, position + delta, true, true, false, true, undo, redo);
    if (moveResult != TimelineModel::MoveSuccess) {
        undo();
        return false;
    }

    if (nextClipId >= 0 && model->isClip(nextClipId)) {
        int newNextDur = model->getClipPlaytime(nextClipId) - delta;
        bool ok = model->requestItemResize(nextClipId, newNextDur, false, false, undo, redo);
        if (!ok) {
            undo();
            return false;
        }
    }

    pCore->pushUndo(undo, redo, i18n("Slide edit"));
    return true;
}

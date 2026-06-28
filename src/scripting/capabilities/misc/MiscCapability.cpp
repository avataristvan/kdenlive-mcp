// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "MiscCapability.h"
#include "bin/projectclip.h"
#include "bin/projectitemmodel.h"
#include "core.h"
#include "definitions.h"
#include "doc/docundostack.hpp"
#include "doc/kdenlivedoc.h"
#include "jobs/abstracttask.h"
#include "mainwindow.h"
#include "timeline2/model/timelinemodel.hpp"
#include "timeline2/view/timelinecontroller.h"
#include "timeline2/view/timelinewidget.h"

#include <QFile>

MiscCapability::MiscCapability(MainWindow *window)
    : m_window(window)
{
}

// ── Undo/Redo ─────────────────────────────────────────────────────────────────

bool MiscCapability::undo(int steps)
{
    auto stack = pCore->undoStack();
    if (!stack) return false;
    int done = 0;
    for (int i = 0; i < steps; ++i) {
        if (!stack->canUndo()) break;
        stack->undo();
        ++done;
    }
    return done > 0;
}

bool MiscCapability::redo(int steps)
{
    auto stack = pCore->undoStack();
    if (!stack) return false;
    int done = 0;
    for (int i = 0; i < steps; ++i) {
        if (!stack->canRedo()) break;
        stack->redo();
        ++done;
    }
    return done > 0;
}

QString MiscCapability::undoStatus()
{
    auto stack = pCore->undoStack();
    if (!stack) {
        return QStringLiteral("can_undo=false;can_redo=false;undo_text=;redo_text=;index=0;count=0");
    }
    return QStringLiteral("can_undo=%1;can_redo=%2;undo_text=%3;redo_text=%4;index=%5;count=%6")
        .arg(stack->canUndo() ? QStringLiteral("true") : QStringLiteral("false"),
             stack->canRedo() ? QStringLiteral("true") : QStringLiteral("false"),
             stack->undoText(), stack->redoText(),
             QString::number(stack->index()), QString::number(stack->count()));
}

// ── Relink ────────────────────────────────────────────────────────────────────

bool MiscCapability::relinkBinClip(const QString &binId, const QString &newFilePath)
{
    if (!QFile::exists(newFilePath)) return false;
    auto clip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!clip) return false;
    clip->setProducerProperty(QStringLiteral("resource"), newFilePath);
    clip->setProducerProperty(QStringLiteral("kdenlive:originalurl"), newFilePath);
    clip->reloadProducer(false, false, false);
    return true;
}

// ── Groups ────────────────────────────────────────────────────────────────────

int MiscCapability::groupClips(const QList<int> &itemIds)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return -1;

    std::unordered_set<int> ids(itemIds.begin(), itemIds.end());
    if (static_cast<int>(ids.size()) < 2) return -1;

    return tl->model()->requestClipsGroup(ids, true, GroupType::Normal);
}

bool MiscCapability::ungroupClips(int itemId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isInGroup(itemId)) return false;

    return tl->model()->requestClipUngroup(itemId, true);
}

QVariantMap MiscCapability::getGroupInfo(int itemId)
{
    QVariantMap result;
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return result;

    auto model = tl->model();

    bool inGroup = model->isInGroup(itemId);
    result[QStringLiteral("isInGroup")] = inGroup;
    result[QStringLiteral("isGroup")] = model->isGroup(itemId);

    if (!inGroup) {
        result[QStringLiteral("rootId")] = itemId;
        result[QStringLiteral("groupType")] = QStringLiteral("Leaf");
        result[QStringLiteral("members")] = QVariantList();
        return result;
    }

    int rootId = model->getGroupRootId(itemId);
    result[QStringLiteral("rootId")] = rootId;
    result[QStringLiteral("groupType")] = groupTypeToStr(model->getGroupType(rootId));

    std::unordered_set<int> leaves = model->getGroupElements(itemId);
    QVariantList memberList;
    memberList.reserve(int(leaves.size()));
    for (int leaf : leaves) {
        QVariantMap member;
        member[QStringLiteral("id")] = leaf;
        if (model->isClip(leaf)) {
            member[QStringLiteral("type")] = QStringLiteral("clip");
            member[QStringLiteral("trackId")] = model->getClipTrackId(leaf);
            member[QStringLiteral("position")] = model->getClipPosition(leaf);
        } else if (model->isComposition(leaf)) {
            member[QStringLiteral("type")] = QStringLiteral("composition");
            member[QStringLiteral("trackId")] = model->getCompositionTrackId(leaf);
            member[QStringLiteral("position")] = model->getCompositionPosition(leaf);
        }
        memberList.append(member);
    }
    result[QStringLiteral("members")] = memberList;

    return result;
}

bool MiscCapability::removeFromGroup(int itemId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isInGroup(itemId)) return false;

    return tl->model()->requestRemoveFromGroup(itemId, true);
}

// ── Proxy ─────────────────────────────────────────────────────────────────────

QVariantMap MiscCapability::getClipProxyStatus(const QString &binId)
{
    QVariantMap result;
    if (!pCore->currentDoc()) return result;

    auto clip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!clip) return result;

    result[QStringLiteral("supportsProxy")] = clip->supportsProxy();
    result[QStringLiteral("hasProxy")] = clip->hasProxy();
    result[QStringLiteral("proxyPath")] = clip->getProducerProperty(QStringLiteral("kdenlive:proxy"));
    result[QStringLiteral("originalUrl")] = clip->getProducerProperty(QStringLiteral("kdenlive:originalurl"));
    ObjectId oid(KdenliveObjectType::BinClip, binId.toInt(), QUuid());
    result[QStringLiteral("isGenerating")] = pCore->taskManager.hasPendingJob(oid, AbstractTask::PROXYJOB);
    return result;
}

bool MiscCapability::setClipProxy(const QString &binId, bool enabled)
{
    if (!pCore->currentDoc()) return false;
    auto clip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!clip) return false;
    if (!clip->supportsProxy()) return false;

    QList<std::shared_ptr<ProjectClip>> clipList{clip};
    pCore->currentDoc()->slotProxyCurrentItem(enabled, clipList);
    return true;
}

bool MiscCapability::deleteClipProxy(const QString &binId)
{
    if (!pCore->currentDoc()) return false;
    auto clip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!clip) return false;
    if (!clip->supportsProxy()) return false;

    clip->deleteProxy(true);
    return true;
}

bool MiscCapability::rebuildClipProxy(const QString &binId)
{
    if (!pCore->currentDoc()) return false;
    auto clip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!clip) return false;
    if (!clip->supportsProxy()) return false;

    QList<std::shared_ptr<ProjectClip>> clipList{clip};
    pCore->currentDoc()->slotProxyCurrentItem(true, clipList, true);
    return true;
}

// ── Selection ─────────────────────────────────────────────────────────────────

QVariantList MiscCapability::getSelection()
{
    QVariantList result;
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->controller()) return result;
    const QList<int> sel = tl->controller()->selection();
    for (int id : sel) {
        result.append(id);
    }
    return result;
}

bool MiscCapability::setSelection(const QList<int> &ids)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->controller()) return false;
    tl->controller()->selectItems(ids);
    return true;
}

bool MiscCapability::addToSelection(int itemId, bool clear)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->controller()) return false;
    auto model = tl->controller()->getModel();
    if (!model) return false;
    model->requestAddToSelection(itemId, clear);
    return true;
}

bool MiscCapability::clearSelection()
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->controller()) return false;
    auto model = tl->controller()->getModel();
    if (!model) return false;
    return model->requestClearSelection();
}

bool MiscCapability::selectAll()
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->controller()) return false;
    tl->controller()->selectAll();
    return true;
}

bool MiscCapability::selectCurrentTrack()
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->controller()) return false;
    tl->controller()->selectCurrentTrack();
    return true;
}

bool MiscCapability::selectItems(const QList<int> &trackIds, int startFrame, int endFrame)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->controller()) return false;
    QVariantList tracks;
    for (int tid : trackIds) {
        tracks.append(tid);
    }
    tl->controller()->selectItems(tracks, startFrame, endFrame, false, true, true);
    return true;
}

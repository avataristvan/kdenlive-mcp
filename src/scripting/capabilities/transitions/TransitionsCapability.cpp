// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "TransitionsCapability.h"
#include "assets/model/assetparametermodel.hpp"
#include "core.h"
#include "mainwindow.h"
#include "timeline2/model/timelineitemmodel.hpp"
#include "timeline2/view/timelinewidget.h"
#include "transitions/transitionsrepository.hpp"
#include <KLocalizedString>

TransitionsCapability::TransitionsCapability(MainWindow *window)
    : m_window(window)
{
}

bool TransitionsCapability::addMix(int clipIdA, int clipIdB, int durationFrames)
{
    Q_UNUSED(clipIdA)
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;

    auto model = tl->model();
    int trackId = model->getClipTrackId(clipIdB);
    int posB = model->getClipPosition(clipIdB);
    std::pair<int, int> clipIds = {clipIdA, clipIdB};
    std::pair<int, int> mixDurations = {durationFrames / 2, durationFrames - durationFrames / 2};

    Fun undo = []() { return true; };
    Fun redo = []() { return true; };
    bool success = model->requestClipMix(QStringLiteral("luma"), clipIds, mixDurations, trackId, posB, true, true, true, undo, redo, false);
    if (success) {
        pCore->pushUndo(undo, redo, i18n("Add mix"));
    }
    return success;
}

int TransitionsCapability::addComposition(const QString &transitionId, int trackId, int position, int duration)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return -1;

    int newId = -1;
    bool success = tl->model()->requestCompositionInsertion(transitionId, trackId, position, duration, nullptr, newId, true);
    return success ? newId : -1;
}

bool TransitionsCapability::removeMix(int clipId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    return tl->model()->requestItemDeletion(clipId, true);
}

QVariantList TransitionsCapability::getAvailableTransitions()
{
    QVariantList result;
    auto allTransitions = TransitionsRepository::get()->getNames();
    for (const auto &pair : allTransitions) {
        QVariantMap info;
        info[QStringLiteral("id")] = pair.first;
        info[QStringLiteral("name")] = pair.second;
        result.append(info);
    }
    return result;
}

QVariantMap TransitionsCapability::getMixParams(int clipId)
{
    QVariantMap result;
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return result;
    if (!tl->model()->isClip(clipId)) return result;

    auto model = tl->model();
    int trackId = model->getClipTrackId(clipId);
    int mixDuration = model->getMixDuration(clipId);

    result[QStringLiteral("clipId")] = clipId;
    result[QStringLiteral("trackId")] = trackId;
    result[QStringLiteral("duration")] = mixDuration;

    if (mixDuration > 0) {
        std::pair<int, int> mixInOut = model->getMixInOut(clipId);
        int mixCut = model->getMixCutPos(clipId);
        result[QStringLiteral("mixIn")] = mixInOut.first;
        result[QStringLiteral("mixOut")] = mixInOut.second;
        result[QStringLiteral("mixCut")] = mixCut;
    }

    return result;
}

bool TransitionsCapability::setMixDuration(int clipId, int newDuration)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;
    if (newDuration < 1) return false;

    auto model = tl->model();
    int currentDuration = model->getMixDuration(clipId);
    if (currentDuration <= 0) return false;

    model->requestResizeMix(clipId, newDuration, MixAlignment::AlignCenter);
    return model->getMixDuration(clipId) > 0;
}

QVariantList TransitionsCapability::getCompositions()
{
    QVariantList result;
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return result;

    auto model = tl->model();
    const QList<int> ids = model->getCompositionIds();
    for (int compoId : ids) {
        QVariantMap info;
        info[QStringLiteral("id")] = compoId;
        info[QStringLiteral("trackId")] = model->getCompositionTrackId(compoId);
        info[QStringLiteral("position")] = model->getCompositionPosition(compoId);
        info[QStringLiteral("duration")] = model->getCompositionPlaytime(compoId);
        auto paramModel = model->getCompositionParameterModel(compoId);
        if (paramModel) {
            info[QStringLiteral("type")] = paramModel->getAssetId();
        }
        result.append(info);
    }
    return result;
}

QVariantMap TransitionsCapability::getCompositionInfo(int compoId)
{
    QVariantMap result;
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return result;
    if (!tl->model()->isComposition(compoId)) return result;

    auto model = tl->model();
    result[QStringLiteral("id")] = compoId;
    result[QStringLiteral("trackId")] = model->getCompositionTrackId(compoId);
    result[QStringLiteral("position")] = model->getCompositionPosition(compoId);
    result[QStringLiteral("duration")] = model->getCompositionPlaytime(compoId);
    auto paramModel = model->getCompositionParameterModel(compoId);
    if (paramModel) {
        result[QStringLiteral("type")] = paramModel->getAssetId();
    }
    return result;
}

bool TransitionsCapability::moveComposition(int compoId, int trackId, int position)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isComposition(compoId)) return false;
    return tl->model()->requestCompositionMove(compoId, trackId, position, true, true);
}

int TransitionsCapability::resizeComposition(int compoId, int newDuration, bool fromRight)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return -1;
    if (!tl->model()->isComposition(compoId)) return -1;
    return tl->model()->requestItemResize(compoId, newDuration, fromRight, true);
}

bool TransitionsCapability::deleteComposition(int compoId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isComposition(compoId)) return false;
    return tl->model()->requestItemDeletion(compoId, true);
}

QVariantList TransitionsCapability::getCompositionTypes()
{
    QVariantList result;
    auto allTransitions = TransitionsRepository::get()->getNames();
    for (const auto &pair : allTransitions) {
        QVariantMap info;
        info[QStringLiteral("id")] = pair.first;
        info[QStringLiteral("name")] = pair.second;
        result.append(info);
    }
    return result;
}

bool TransitionsCapability::setCompositionParam(int compoId, const QString &paramName, const QString &paramValue)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isComposition(compoId)) return false;

    auto paramModel = tl->model()->getCompositionParameterModel(compoId);
    if (!paramModel) return false;
    paramModel->setParameter(paramName, paramValue, true);
    return true;
}

QString TransitionsCapability::getCompositionParam(int compoId, const QString &paramName)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return {};
    if (!tl->model()->isComposition(compoId)) return {};

    auto paramModel = tl->model()->getCompositionParameterModel(compoId);
    if (!paramModel) return {};
    return paramModel->getParam(paramName);
}

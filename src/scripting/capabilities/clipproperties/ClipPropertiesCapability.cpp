// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "ClipPropertiesCapability.h"
#include "assets/keyframes/model/keyframemodellist.hpp"
#include "assets/model/assetparametermodel.hpp"
#include "bin/abstractprojectitem.h"
#include "bin/projectclip.h"
#include "bin/projectitemmodel.h"
#include "core.h"
#include "effects/effectstack/model/effectitemmodel.hpp"
#include "effects/effectstack/model/effectstackmodel.hpp"
#include "kdenlivesettings.h"
#include "mainwindow.h"
#include "timeline2/model/timelinefunctions.hpp"
#include "timeline2/model/timelineitemmodel.hpp"
#include "timeline2/view/timelinewidget.h"
#include <KLocalizedString>

ClipPropertiesCapability::ClipPropertiesCapability(MainWindow *window)
    : m_window(window)
{
}

// ── Clip Transform ────────────────────────────────────────────────────────────

QVariantList ClipPropertiesCapability::getClipTransformKeyframes(int clipId)
{
    QVariantList result;
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return result;
    if (!tl->model()->isClip(clipId)) return result;

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    if (!stack || !stack->hasFilter(QStringLiteral("qtblend"))) return result;
    auto asset = stack->getAssetModelById(QStringLiteral("qtblend"));
    if (!asset) return result;

    auto kfModelList = asset->getKeyframeModel();
    if (!kfModelList) return result;
    auto *kfModel = kfModelList->getKeyModel();
    if (!kfModel) return result;

    for (int i = 0; i < kfModel->rowCount(); ++i) {
        QModelIndex idx = kfModel->index(i, 0);
        QVariantMap kf;
        kf[QStringLiteral("frame")] = kfModel->data(idx, Qt::UserRole);
        kf[QStringLiteral("value")] = kfModel->data(idx, Qt::DisplayRole);
        result.append(kf);
    }
    return result;
}

bool ClipPropertiesCapability::setClipTransform(int clipId, int frame, int x, int y, int width, int height, double opacity)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    if (!stack) return false;

    if (!stack->hasFilter(QStringLiteral("qtblend"))) {
        stack->appendEffect(QStringLiteral("qtblend"), false);
    }
    auto asset = stack->getAssetModelById(QStringLiteral("qtblend"));
    if (!asset) return false;

    int intOpacity = qBound(0, static_cast<int>(opacity * 100.0), 100);

    QString existingValue;
    for (int i = 0; i < asset->rowCount(); ++i) {
        QModelIndex idx = asset->index(i, 0);
        QString paramName = asset->data(idx, AssetParameterModel::NameRole).toString();
        if (paramName == QLatin1String("rect")) {
            existingValue = asset->data(idx, AssetParameterModel::ValueRole).toString();
            break;
        }
    }

    QMap<int, QString> keyframes;
    if (!existingValue.isEmpty() && existingValue.contains(QLatin1Char('='))) {
        const QStringList parts = existingValue.split(QLatin1Char(';'), Qt::SkipEmptyParts);
        for (const QString &part : parts) {
            int eqPos = part.indexOf(QLatin1Char('='));
            if (eqPos > 0) {
                bool ok;
                int kfFrame = part.left(eqPos).toInt(&ok);
                if (ok) {
                    keyframes[kfFrame] = part.mid(eqPos + 1);
                }
            }
        }
    }

    keyframes[frame] = QStringLiteral("%1 %2 %3 %4 %5").arg(x).arg(y).arg(width).arg(height).arg(intOpacity);

    QStringList result;
    for (auto it = keyframes.constBegin(); it != keyframes.constEnd(); ++it) {
        result.append(QStringLiteral("%1=%2").arg(it.key()).arg(it.value()));
    }
    asset->setParameter(QStringLiteral("rect"), result.join(QLatin1Char(';')), true);
    return true;
}

bool ClipPropertiesCapability::removeClipTransformKeyframe(int clipId, int frame)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    if (!stack || !stack->hasFilter(QStringLiteral("qtblend"))) return false;
    auto asset = stack->getAssetModelById(QStringLiteral("qtblend"));
    if (!asset) return false;

    auto kfModelList = asset->getKeyframeModel();
    if (!kfModelList) return false;

    GenTime pos(frame, pCore->getCurrentFps());
    return kfModelList->removeKeyframe(pos);
}

// ── Clip Properties ───────────────────────────────────────────────────────────

double ClipPropertiesCapability::getClipOpacity(int clipId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return -1.0;
    if (!tl->model()->isClip(clipId)) return -1.0;

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    if (!stack) return 1.0;

    for (int i = 0; i < stack->rowCount(); ++i) {
        auto effect = std::static_pointer_cast<EffectItemModel>(stack->getEffectStackRow(i));
        if (effect && effect->getAssetId() == QLatin1String("qtblend")) {
            QString val = effect->filter().get("opacity");
            return val.isEmpty() ? 1.0 : val.toDouble() / 100.0;
        }
    }
    return 1.0;
}

bool ClipPropertiesCapability::setClipOpacity(int clipId, double opacity)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    if (!stack) return false;

    for (int i = 0; i < stack->rowCount(); ++i) {
        auto effect = std::static_pointer_cast<EffectItemModel>(stack->getEffectStackRow(i));
        if (effect && effect->getAssetId() == QLatin1String("qtblend")) {
            int intOpacity = qBound(0, static_cast<int>(opacity * 100.0), 100);
            effect->filter().set("opacity", intOpacity);
            return true;
        }
    }

    int intOpacity = qBound(0, static_cast<int>(opacity * 100.0), 100);
    QMap<QString, QString> params;
    params[QStringLiteral("opacity")] = QString::number(intOpacity);
    return stack->appendEffect(QStringLiteral("qtblend"), false, params);
}

bool ClipPropertiesCapability::isClipEnabled(int clipId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    auto [state, type] = tl->model()->getClipState(clipId);
    return state != PlaylistState::Disabled;
}

bool ClipPropertiesCapability::setClipEnabled(int clipId, bool enabled)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    PlaylistState::ClipState newState;
    if (enabled) {
        int tid = tl->model()->getClipTrackId(clipId);
        bool isAudio = tl->model()->isAudioTrack(tid);
        newState = isAudio ? PlaylistState::AudioOnly : PlaylistState::VideoOnly;
    } else {
        newState = PlaylistState::Disabled;
    }

    Fun undo = []() { return true; };
    Fun redo = []() { return true; };
    bool success = TimelineFunctions::changeClipState(tl->model(), clipId, newState, undo, redo);
    if (success) {
        pCore->pushUndo(undo, redo, enabled ? i18n("Enable clip") : i18n("Disable clip"));
    }
    return success;
}

QString ClipPropertiesCapability::getClipColor(int clipId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return QString();
    if (!tl->model()->isClip(clipId)) return QString();

    if (!KdenliveSettings::tagsintimeline()) return QString();
    const QString binId = tl->model()->getClipBinId(clipId);
    auto binClip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!binClip) return QString();
    return binClip->tags();
}

bool ClipPropertiesCapability::setClipColor(int clipId, const QString &colorTag)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    const QString binId = tl->model()->getClipBinId(clipId);
    auto binClip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!binClip) return false;
    binClip->setTags(colorTag);
    return true;
}

// ── Track Properties ──────────────────────────────────────────────────────────

QString ClipPropertiesCapability::getTrackName(int trackId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return QString();
    return tl->model()->getTrackProperty(trackId, QStringLiteral("kdenlive:track_name")).toString();
}

bool ClipPropertiesCapability::setTrackName(int trackId, const QString &name)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    tl->model()->setTrackProperty(trackId, QStringLiteral("kdenlive:track_name"), name);
    return true;
}

int ClipPropertiesCapability::getTrackColor(int trackId)
{
    // Kdenlive does not store per-track colors as properties; track colors are palette-derived in QML.
    Q_UNUSED(trackId);
    return -1;
}

bool ClipPropertiesCapability::setTrackColor(int trackId, int color)
{
    // Kdenlive does not support per-track custom colors; track colors are theme-derived in QML.
    Q_UNUSED(trackId);
    Q_UNUSED(color);
    return false;
}

bool ClipPropertiesCapability::getTrackSolo(int trackId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    return tl->model()->getTrackProperty(trackId, QStringLiteral("kdenlive:solo")).toInt() != 0;
}

bool ClipPropertiesCapability::setTrackSolo(int trackId, bool solo)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    tl->model()->setTrackProperty(trackId, QStringLiteral("kdenlive:solo"), solo ? QStringLiteral("1") : QStringLiteral("0"));
    return true;
}

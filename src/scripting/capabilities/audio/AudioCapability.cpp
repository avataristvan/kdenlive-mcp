// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "AudioCapability.h"
#include "bin/projectclip.h"
#include "bin/projectitemmodel.h"
#include "core.h"
#include "effects/effectstack/model/effectitemmodel.hpp"
#include "effects/effectstack/model/effectstackmodel.hpp"
#include "mainwindow.h"
#include "timeline2/model/timelinefunctions.hpp"
#include "timeline2/model/timelineitemmodel.hpp"
#include "timeline2/view/timelinewidget.h"

#include <cmath>

AudioCapability::AudioCapability(MainWindow *window)
    : m_window(window)
{
}

bool AudioCapability::splitAudio(int clipId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    auto model = tl->model();
    if (!model->isClip(clipId)) return false;

    QList<int> audioTracks = model->getActiveAudioTrackIndexes();
    if (audioTracks.isEmpty()) return false;

    return TimelineFunctions::requestSplitAudio(model, clipId, audioTracks);
}

bool AudioCapability::setClipVolume(int clipId, double dB)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    auto model = tl->model();
    if (!model->isClip(clipId)) return false;

    auto stack = model->getClipEffectStackModel(clipId);
    if (!stack) return false;

    for (int i = 0; i < stack->rowCount(); ++i) {
        auto effect = std::static_pointer_cast<EffectItemModel>(stack->getEffectStackRow(i));
        if (effect && effect->getAssetId() == QLatin1String("volume") && effect->isBuiltIn()) {
            effect->filter().set("level", dB);
            effect->filter().set("disable", 0);
            return true;
        }
    }

    QMap<QString, QString> params;
    params[QStringLiteral("level")] = QString::number(dB);
    return stack->appendEffect(QStringLiteral("volume"), false, params);
}

double AudioCapability::getClipVolume(int clipId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return 0.0;
    auto model = tl->model();
    if (!model->isClip(clipId)) return 0.0;

    auto stack = model->getClipEffectStackModel(clipId);
    if (!stack) return 0.0;

    for (int i = 0; i < stack->rowCount(); ++i) {
        auto effect = std::static_pointer_cast<EffectItemModel>(stack->getEffectStackRow(i));
        if (effect && effect->getAssetId() == QLatin1String("volume")) {
            if (effect->filter().get_int("disable") == 1) return 0.0;
            return effect->filter().get_double("level");
        }
    }
    return 0.0;
}

bool AudioCapability::setAudioFade(int clipId, int fadeInFrames, int fadeOutFrames)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    auto model = tl->model();
    if (!model->isClip(clipId)) return false;

    auto stack = model->getClipEffectStackModel(clipId);
    if (!stack) return false;

    bool ok = true;
    if (fadeInFrames >= 0) {
        ok = stack->adjustFadeLength(fadeInFrames, true, true, false, true) && ok;
    }
    if (fadeOutFrames >= 0) {
        ok = stack->adjustFadeLength(fadeOutFrames, false, true, false, true) && ok;
    }
    return ok;
}

bool AudioCapability::setClipPan(int clipId, double pan)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    auto model = tl->model();
    if (!model->isClip(clipId)) return false;

    auto stack = model->getClipEffectStackModel(clipId);
    if (!stack) return false;

    if (!stack->hasFilter(QStringLiteral("audiopan"))) {
        stack->appendEffect(QStringLiteral("audiopan"), false);
    }
    auto asset = stack->getAssetModelById(QStringLiteral("audiopan"));
    if (!asset) return false;

    // Pan range: -100 (full left) to +100 (full right); normalized to 0.0–1.0 for MLT
    double normalized = qBound(0.0, (pan + 100.0) / 200.0, 1.0);
    asset->setParameter(QStringLiteral("start"), QString::number(normalized, 'f', 3), true);
    return true;
}

double AudioCapability::getClipPan(int clipId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return 0.0;
    auto model = tl->model();
    if (!model->isClip(clipId)) return 0.0;

    auto stack = model->getClipEffectStackModel(clipId);
    if (!stack || !stack->hasFilter(QStringLiteral("audiopan"))) return 0.0;

    auto asset = stack->getAssetModelById(QStringLiteral("audiopan"));
    if (!asset) return 0.0;

    QString val = asset->getParam(QStringLiteral("start"));
    if (val.isEmpty()) return 0.0;
    return (val.toDouble() * 200.0) - 100.0;
}

bool AudioCapability::setTrackMute(int trackId, bool mute)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    auto model = tl->model();

    int currentHide = model->getTrackProperty(trackId, QStringLiteral("hide")).toInt();
    int newHide = mute ? (currentHide | 2) : (currentHide & ~2);
    model->setTrackProperty(trackId, QStringLiteral("hide"), QString::number(newHide));
    return true;
}

bool AudioCapability::getTrackMute(int trackId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    return tl->model()->getTrackProperty(trackId, QStringLiteral("hide")).toInt() & 2;
}

bool AudioCapability::setTrackLocked(int trackId, bool locked)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    tl->model()->setTrackLockedState(trackId, locked);
    return true;
}

bool AudioCapability::getTrackLocked(int trackId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    return tl->model()->trackIsLocked(trackId);
}

bool AudioCapability::setTrackHidden(int trackId, bool hidden)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    auto model = tl->model();

    int currentHide = model->getTrackProperty(trackId, QStringLiteral("hide")).toInt();
    int newHide = hidden ? (currentHide | 1) : (currentHide & ~1);
    model->setTrackProperty(trackId, QStringLiteral("hide"), QString::number(newHide));
    return true;
}

bool AudioCapability::getTrackHidden(int trackId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    return tl->model()->getTrackProperty(trackId, QStringLiteral("hide")).toInt() & 1;
}

QVariantList AudioCapability::getAudioLevels(const QString &binId, int stream, int downsample, int mode)
{
    QVariantList result;
    auto clip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!clip) return result;

    QVector<int16_t> levels = clip->audioFrameCache(stream);
    if (levels.isEmpty()) return result;

    int16_t maxVal = clip->getAudioMax(stream);
    if (maxVal <= 0) maxVal = 1;

    int step = qMax(1, downsample);
    double normMax = double(maxVal);

    for (int i = 0; i < levels.size(); i += step) {
        int end = qMin(i + step, levels.size());
        if (mode == 1) {
            // RMS mode
            double sumSq = 0.0;
            int count = 0;
            for (int j = i; j < end; ++j) {
                double v = double(levels[j]) / normMax;
                sumSq += v * v;
                ++count;
            }
            result.append(count > 0 ? std::sqrt(sumSq / count) : 0.0);
        } else {
            // Peak mode (default)
            int16_t peak = 0;
            for (int j = i; j < end; ++j) {
                peak = qMax(peak, qAbs(levels[j]));
            }
            result.append(double(peak) / normMax);
        }
    }
    return result;
}

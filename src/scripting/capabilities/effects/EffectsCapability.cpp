// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "EffectsCapability.h"
#include "core.h"
#include "effects/effectsrepository.hpp"
#include "effects/effectstack/model/effectitemmodel.hpp"
#include "effects/effectstack/model/effectstackmodel.hpp"
#include "assets/keyframes/model/keyframemodellist.hpp"
#include "assets/keyframes/model/keyframemodel.hpp"
#include "mainwindow.h"
#include "timeline2/model/timelineitemmodel.hpp"
#include "timeline2/view/timelinewidget.h"
#include "utils/gentime.h"
#include <KLocalizedString>
#include <QDomDocument>

EffectsCapability::EffectsCapability(MainWindow *window)
    : m_window(window)
{
}

// ── Effects ───────────────────────────────────────────────────────────────

QVariantList EffectsCapability::getAvailableEffects()
{
    QVariantList result;
    auto allEffects = EffectsRepository::get()->getNames();
    for (const auto &pair : allEffects) {
        QVariantMap info;
        info[QStringLiteral("id")] = pair.first;
        info[QStringLiteral("name")] = pair.second;
        info[QStringLiteral("type")] = EffectsRepository::get()->isAudioEffect(pair.first) ? QStringLiteral("audio") : QStringLiteral("video");
        result.append(info);
    }
    return result;
}

bool EffectsCapability::addClipEffect(int clipId, const QString &effectId, const QStringList &paramKeys, const QStringList &paramValues)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    QMap<QString, QString> params;
    int count = qMin(paramKeys.size(), paramValues.size());
    for (int i = 0; i < count; ++i) {
        params.insert(paramKeys.at(i), paramValues.at(i));
    }

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    if (!stack) return false;
    return stack->appendEffect(effectId, false, params);
}

bool EffectsCapability::removeClipEffect(int clipId, const QString &effectId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    if (!stack || !stack->hasFilter(effectId)) return false;
    Fun undo = []() { return true; };
    Fun redo = []() { return true; };
    QString effectName;
    stack->removeEffectWithUndo(effectId, effectName, -1, undo, redo);
    return !effectName.isEmpty();
}

QString EffectsCapability::getClipEffects(int clipId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return {};
    if (!tl->model()->isClip(clipId)) return {};

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    if (!stack) return {};
    return stack->effectNames();
}

bool EffectsCapability::setEffectParam(int clipId, const QString &effectId, const QString &paramName, const QString &paramValue)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    if (!stack) return false;
    auto asset = stack->getAssetModelById(effectId);
    if (!asset) return false;
    asset->setParameter(paramName, paramValue, true);
    return true;
}

QString EffectsCapability::getEffectParam(int clipId, const QString &effectId, const QString &paramName)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return {};
    if (!tl->model()->isClip(clipId)) return {};

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    if (!stack) return {};
    auto asset = stack->getAssetModelById(effectId);
    if (!asset) return {};
    return asset->getParam(paramName);
}

bool EffectsCapability::setEffectExpression(int clipId, const QString &effectId, const QString &paramName, const QString &expression, double baseValue)
{
    // Requires expression infrastructure (setExpression / setExpressionBaseValue) not present in vanilla build.
    Q_UNUSED(clipId)
    Q_UNUSED(effectId)
    Q_UNUSED(paramName)
    Q_UNUSED(expression)
    Q_UNUSED(baseValue)
    return false;
}

bool EffectsCapability::clearEffectExpression(int clipId, const QString &effectId, const QString &paramName)
{
    // Requires expression infrastructure (clearExpression) not present in vanilla build.
    Q_UNUSED(clipId)
    Q_UNUSED(effectId)
    Q_UNUSED(paramName)
    return false;
}

QString EffectsCapability::copyClipEffects(int clipId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return {};
    if (!tl->model()->isClip(clipId)) return {};

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    if (!stack) return {};

    QDomDocument doc;
    QDomElement xml = stack->toXml(doc);
    doc.appendChild(xml);
    return doc.toString(-1);
}

bool EffectsCapability::pasteClipEffects(int targetClipId, const QString &effectsXml)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(targetClipId)) return false;

    auto stack = tl->model()->getClipEffectStackModel(targetClipId);
    if (!stack) return false;

    QDomDocument doc;
    if (!doc.setContent(effectsXml)) return false;

    Fun undo = []() { return true; };
    Fun redo = []() { return true; };
    bool result = stack->fromXml(doc.documentElement(), undo, redo);
    if (result) {
        pCore->pushUndo(undo, redo, i18n("Paste effects"));
    }
    return result;
}

// ── Effect Keyframes by index ─────────────────────────────────────────────

static std::shared_ptr<KeyframeModelList> getEffectKeyframeModelAt(const std::shared_ptr<EffectStackModel> &stack, int effectIndex)
{
    if (!stack || effectIndex < 0 || effectIndex >= stack->rowCount()) return nullptr;
    auto effect = std::static_pointer_cast<EffectItemModel>(stack->getEffectStackRow(effectIndex));
    if (!effect) return nullptr;
    return effect->getKeyframeModel();
}

static QString keyframeTypeName(int type)
{
    switch (type) {
    case mlt_keyframe_linear:
        return QStringLiteral("linear");
    case mlt_keyframe_discrete:
        return QStringLiteral("discrete");
    case mlt_keyframe_smooth_natural:
        return QStringLiteral("smooth");
    case mlt_keyframe_bounce_in:
        return QStringLiteral("bounce_in");
    case mlt_keyframe_bounce_out:
        return QStringLiteral("bounce_out");
    case mlt_keyframe_cubic_in:
        return QStringLiteral("cubic_in");
    case mlt_keyframe_cubic_out:
        return QStringLiteral("cubic_out");
    case mlt_keyframe_exponential_in:
        return QStringLiteral("exponential_in");
    case mlt_keyframe_exponential_out:
        return QStringLiteral("exponential_out");
    case mlt_keyframe_circular_in:
        return QStringLiteral("circular_in");
    case mlt_keyframe_circular_out:
        return QStringLiteral("circular_out");
    case mlt_keyframe_elastic_in:
        return QStringLiteral("elastic_in");
    case mlt_keyframe_elastic_out:
        return QStringLiteral("elastic_out");
    default:
        return QStringLiteral("linear");
    }
}

QVariantList EffectsCapability::getEffectKeyframes(int clipId, int effectIndex)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return {};
    if (!tl->model()->isClip(clipId)) return {};

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    auto listModel = getEffectKeyframeModelAt(stack, effectIndex);
    if (!listModel) return {};

    double fps = pCore->getCurrentFps();
    KeyframeModel *kfModel = listModel->getKeyModel();
    if (!kfModel) return {};

    int count = kfModel->rowCount();
    QVariantList result;
    for (int i = 0; i < count; ++i) {
        GenTime pos = kfModel->getPosAtIndex(i);
        bool ok = false;
        Keyframe kf = kfModel->getKeyframe(pos, &ok);
        if (!ok) continue;
        QVariantMap m;
        m[QStringLiteral("frame")] = pos.frames(fps);
        m[QStringLiteral("type")] = keyframeTypeName(static_cast<int>(kf.second));
        m[QStringLiteral("value")] = kfModel->getInterpolatedValue(pos).toString();
        result.append(m);
    }
    return result;
}

bool EffectsCapability::addEffectKeyframe(int clipId, int effectIndex, int frame, double normalizedValue, int keyframeType)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    auto listModel = getEffectKeyframeModelAt(stack, effectIndex);
    if (!listModel) return false;

    if (keyframeType >= 0) {
        double fps = pCore->getCurrentFps();
        return listModel->addKeyframe(GenTime(frame, fps), static_cast<KeyframeType::KeyframeEnum>(keyframeType));
    }
    return listModel->addKeyframe(frame, normalizedValue);
}

bool EffectsCapability::removeEffectKeyframe(int clipId, int effectIndex, int frame)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    auto listModel = getEffectKeyframeModelAt(stack, effectIndex);
    if (!listModel) return false;

    double fps = pCore->getCurrentFps();
    return listModel->removeKeyframe(GenTime(frame, fps));
}

bool EffectsCapability::updateEffectKeyframe(int clipId, int effectIndex, int oldFrame, int newFrame, double normalizedValue)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    auto listModel = getEffectKeyframeModelAt(stack, effectIndex);
    if (!listModel) return false;

    double fps = pCore->getCurrentFps();
    QVariant val = (normalizedValue >= 0) ? QVariant(normalizedValue) : QVariant();
    return listModel->updateKeyframe(GenTime(oldFrame, fps), GenTime(newFrame, fps), val);
}

// ── Effect Keyframes by parameter name ───────────────────────────────────

static std::pair<std::shared_ptr<KeyframeModelList>, KeyframeModel *> getEffectKeyframeByParam(
    const std::shared_ptr<EffectStackModel> &stack, const QString &effectId, const QString &paramName)
{
    if (!stack) return {nullptr, nullptr};
    int row = stack->effectRow(effectId);
    if (row < 0) return {nullptr, nullptr};
    auto effect = std::static_pointer_cast<EffectItemModel>(stack->getEffectStackRow(row));
    if (!effect) return {nullptr, nullptr};
    auto listModel = effect->getKeyframeModel();
    if (!listModel) return {nullptr, nullptr};
    if (paramName.isEmpty()) {
        return {listModel, listModel->getKeyModel()};
    }
    QModelIndex paramIndex = effect->getParamIndexFromName(paramName);
    if (!paramIndex.isValid()) return {nullptr, nullptr};
    KeyframeModel *kfModel = listModel->getKeyModel(QPersistentModelIndex(paramIndex));
    if (!kfModel) return {nullptr, nullptr};
    return {listModel, kfModel};
}

QVariantList EffectsCapability::getEffectKeyframesByParam(int clipId, const QString &effectId, const QString &paramName)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return {};
    if (!tl->model()->isClip(clipId)) return {};

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    auto [listModel, kfModel] = getEffectKeyframeByParam(stack, effectId, paramName);
    if (!kfModel) return {};

    double fps = pCore->getCurrentFps();
    int count = kfModel->rowCount();
    QVariantList result;
    for (int i = 0; i < count; ++i) {
        GenTime pos = kfModel->getPosAtIndex(i);
        bool ok = false;
        Keyframe kf = kfModel->getKeyframe(pos, &ok);
        if (!ok) continue;
        QVariantMap m;
        m[QStringLiteral("frame")] = pos.frames(fps);
        m[QStringLiteral("type")] = keyframeTypeName(static_cast<int>(kf.second));
        m[QStringLiteral("value")] = kfModel->getInterpolatedValue(pos).toString();
        result.append(m);
    }
    return result;
}

bool EffectsCapability::addEffectKeyframeByParam(int clipId, const QString &effectId, const QString &paramName,
                                                  int frame, const QString &value, int keyframeType)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    auto [listModel, kfModel] = getEffectKeyframeByParam(stack, effectId, paramName);
    if (!kfModel || !listModel) return false;

    double fps = pCore->getCurrentFps();
    GenTime pos(frame, fps);
    auto type = (keyframeType >= 0) ? static_cast<KeyframeType::KeyframeEnum>(keyframeType) : KeyframeType::Linear;
    if (!listModel->addKeyframe(pos, type)) return false;
    if (!value.isEmpty()) {
        kfModel->updateKeyframe(pos, QVariant(value));
    }
    return true;
}

bool EffectsCapability::removeEffectKeyframeByParam(int clipId, const QString &effectId, const QString &paramName, int frame)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    auto [listModel, kfModel] = getEffectKeyframeByParam(stack, effectId, paramName);
    if (!kfModel || !listModel) return false;

    double fps = pCore->getCurrentFps();
    return listModel->removeKeyframe(GenTime(frame, fps));
}

// ── Time Remap ────────────────────────────────────────────────────────────

bool EffectsCapability::enableTimeRemap(int clipId, bool enable)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;
    return tl->model()->requestClipTimeRemap(clipId, enable);
}

QVariantMap EffectsCapability::getTimeRemap(int clipId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return {};
    if (!tl->model()->isClip(clipId)) return {};

    QVariantMap result;
    bool hasRemap = tl->model()->getClipHasTimeRemap(clipId);
    result[QStringLiteral("enabled")] = hasRemap;

    if (hasRemap) {
        QMap<QString, QString> vals = tl->model()->getClipRemapValues(clipId);
        result[QStringLiteral("time_map")] = vals.value(QStringLiteral("time_map"));
        result[QStringLiteral("pitch")] = vals.value(QStringLiteral("pitch"), QStringLiteral("0")).toInt();
        result[QStringLiteral("image_mode")] = vals.value(QStringLiteral("image_mode"), QStringLiteral("nearest"));
    }
    return result;
}

bool EffectsCapability::setTimeRemap(int clipId, const QString &timeMap, int pitch, const QString &imageMode)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;
    if (!tl->model()->getClipHasTimeRemap(clipId)) return false;

    if (!timeMap.isEmpty()) {
        tl->model()->setClipRemapValue(clipId, QStringLiteral("time_map"), timeMap);
    }
    tl->model()->setClipRemapValue(clipId, QStringLiteral("pitch"), QString::number(pitch));
    if (!imageMode.isEmpty()) {
        tl->model()->setClipRemapValue(clipId, QStringLiteral("image_mode"), imageMode);
    }
    return true;
}

// ── Clip Speed ────────────────────────────────────────────────────────────

bool EffectsCapability::setClipSpeed(int clipId, double speed, bool pitchCompensate)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;
    return tl->model()->requestClipTimeWarp(clipId, speed, pitchCompensate, true);
}

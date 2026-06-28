// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#pragma once

#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

class MainWindow;

class EffectsCapability
{
public:
    explicit EffectsCapability(MainWindow *window);

    // Effects
    QVariantList getAvailableEffects();
    bool addClipEffect(int clipId, const QString &effectId, const QStringList &paramKeys, const QStringList &paramValues);
    bool removeClipEffect(int clipId, const QString &effectId);
    QString getClipEffects(int clipId);
    bool setEffectParam(int clipId, const QString &effectId, const QString &paramName, const QString &paramValue);
    QString getEffectParam(int clipId, const QString &effectId, const QString &paramName);
    bool setEffectExpression(int clipId, const QString &effectId, const QString &paramName, const QString &expression, double baseValue);
    bool clearEffectExpression(int clipId, const QString &effectId, const QString &paramName);
    QString copyClipEffects(int clipId);
    bool pasteClipEffects(int targetClipId, const QString &effectsXml);

    // Clip Speed
    bool setClipSpeed(int clipId, double speed, bool pitchCompensate);

    // Effect Keyframes by index
    QVariantList getEffectKeyframes(int clipId, int effectIndex);
    bool addEffectKeyframe(int clipId, int effectIndex, int frame, double normalizedValue, int keyframeType);
    bool removeEffectKeyframe(int clipId, int effectIndex, int frame);
    bool updateEffectKeyframe(int clipId, int effectIndex, int oldFrame, int newFrame, double normalizedValue);

    // Effect Keyframes by parameter name
    QVariantList getEffectKeyframesByParam(int clipId, const QString &effectId, const QString &paramName);
    bool addEffectKeyframeByParam(int clipId, const QString &effectId, const QString &paramName, int frame, const QString &value, int keyframeType);
    bool removeEffectKeyframeByParam(int clipId, const QString &effectId, const QString &paramName, int frame);

    // Time Remap
    bool enableTimeRemap(int clipId, bool enable);
    QVariantMap getTimeRemap(int clipId);
    bool setTimeRemap(int clipId, const QString &timeMap, int pitch, const QString &imageMode);

private:
    MainWindow *m_window;
};

// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#pragma once

#include <QString>
#include <QVariantList>
#include <QVariantMap>

class MainWindow;

class TransitionsCapability
{
public:
    explicit TransitionsCapability(MainWindow *window);

    bool addMix(int clipIdA, int clipIdB, int durationFrames);
    int addComposition(const QString &transitionId, int trackId, int position, int duration);
    bool removeMix(int clipId);
    QVariantList getAvailableTransitions();
    QVariantMap getMixParams(int clipId);
    bool setMixDuration(int clipId, int newDuration);
    QVariantList getCompositions();
    QVariantMap getCompositionInfo(int compoId);
    bool moveComposition(int compoId, int trackId, int position);
    int resizeComposition(int compoId, int newDuration, bool fromRight);
    bool deleteComposition(int compoId);
    QVariantList getCompositionTypes();
    bool setCompositionParam(int compoId, const QString &paramName, const QString &paramValue);
    QString getCompositionParam(int compoId, const QString &paramName);

private:
    MainWindow *m_window;
};

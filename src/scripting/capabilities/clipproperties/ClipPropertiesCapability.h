// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#pragma once

#include <QString>
#include <QVariantList>

class MainWindow;

class ClipPropertiesCapability
{
public:
    explicit ClipPropertiesCapability(MainWindow *window);

    // Clip Transform
    QVariantList getClipTransformKeyframes(int clipId);
    bool setClipTransform(int clipId, int frame, int x, int y, int width, int height, double opacity);
    bool removeClipTransformKeyframe(int clipId, int frame);

    // Clip Properties
    double getClipOpacity(int clipId);
    bool setClipOpacity(int clipId, double opacity);
    bool isClipEnabled(int clipId);
    bool setClipEnabled(int clipId, bool enabled);
    QString getClipColor(int clipId);
    bool setClipColor(int clipId, const QString &colorTag);

    // Track Properties
    QString getTrackName(int trackId);
    bool setTrackName(int trackId, const QString &name);
    int getTrackColor(int trackId);
    bool setTrackColor(int trackId, int color);
    bool getTrackSolo(int trackId);
    bool setTrackSolo(int trackId, bool solo);

private:
    MainWindow *m_window;
};

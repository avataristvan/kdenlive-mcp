// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#pragma once

#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

class MainWindow;

class TimelineCapability
{
public:
    explicit TimelineCapability(MainWindow *window);

    int getTrackCount(const QString &trackType);
    QVariantMap getTrackInfo(int trackIndex);
    QVariantList getAllTracksInfo();
    int addTrack(const QString &name, bool audioTrack);
    bool deleteTrack(int trackId);
    bool insertSpace(int trackId, int position, int duration, bool allTracks);
    bool removeSpace(int trackId, int position, bool allTracks);
    int insertClip(const QString &binClipId, int trackId, int position);
    QVariantList insertClipsSequentially(const QStringList &binClipIds, int trackId, int startPosition);
    bool moveClip(int clipId, int trackId, int position);
    int resizeClip(int clipId, int newDuration, bool fromRight);
    bool deleteTimelineClip(int clipId);
    QVariantList getClipsOnTrack(int trackId);
    QVariantMap getTimelineClipInfo(int clipId);
    bool slipClip(int clipId, int offset);
    bool cutClip(int clipId, int position);
    bool rippleDelete(int clipId);
    bool rippleTrim(int clipId, int delta, bool fromRight);
    bool rollEdit(int clipId, int delta);
    bool slideEdit(int clipId, int delta);

private:
    MainWindow *m_window;
};

// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#pragma once

#include <QString>
#include <QVariantList>

class MainWindow;

class AudioCapability
{
public:
    explicit AudioCapability(MainWindow *window);

    bool splitAudio(int clipId);
    bool setClipVolume(int clipId, double dB);
    double getClipVolume(int clipId);
    bool setAudioFade(int clipId, int fadeInFrames, int fadeOutFrames);
    bool setClipPan(int clipId, double pan);
    double getClipPan(int clipId);
    bool setTrackMute(int trackId, bool mute);
    bool getTrackMute(int trackId);
    bool setTrackLocked(int trackId, bool locked);
    bool getTrackLocked(int trackId);
    bool setTrackHidden(int trackId, bool hidden);
    bool getTrackHidden(int trackId);
    QVariantList getAudioLevels(const QString &binId, int stream, int downsample, int mode);

private:
    MainWindow *m_window;
};

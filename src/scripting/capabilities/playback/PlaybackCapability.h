// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#pragma once

#include <QString>
#include <QStringList>
#include <QVariantList>

class MainWindow;

class PlaybackCapability
{
public:
    explicit PlaybackCapability(MainWindow *window);

    // Render
    bool renderWithParams(const QString &outputFile, const QString &presetName, int inFrame, int outFrame,
                          const QStringList &paramKeys, const QStringList &paramValues);
    QStringList getRenderPresets();
    QVariantList getRenderJobs();
    bool abortRenderJob(const QString &outputPath);

    // Project Profile
    bool setProjectProfile(int width, int height, int fpsNum, int fpsDen);

    // Copy/Cut/Paste
    int copyClips();
    bool cutClips();
    bool pasteClips(int position, int trackId);

    // Playback
    void seek(int frame);
    int getPosition();
    void play();
    void pause();
    bool setPlaybackSpeed(double speed);
    double getPlaybackSpeed();

    // Navigation
    int goToNextMarker();
    int goToPreviousMarker();
    int goToNextEdit();
    int goToPreviousEdit();

private:
    MainWindow *m_window;
};

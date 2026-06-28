// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#pragma once

#include <QString>
#include <QStringList>
#include <QVariantList>

class MainWindow;

class SubtitlesCapability
{
public:
    explicit SubtitlesCapability(MainWindow *window);

    // Subtitles
    QVariantList getSubtitles();
    int addSubtitle(int startFrame, int endFrame, const QString &text, int layer);
    bool editSubtitle(int subtitleId, const QString &newText);
    bool moveSubtitle(int subtitleId, int newStartFrame);
    bool resizeSubtitle(int subtitleId, int newDuration, bool fromRight);
    bool importSubtitle(const QString &filePath, int offset, const QString &encoding);
    bool deleteSubtitle(int subtitleId);
    bool exportSubtitles(const QString &filePath);

    // Speech Recognition
    bool speechRecognition();

    // Subtitle Styles
    QVariantList getSubtitleStyles(bool global);
    bool setSubtitleStyle(const QString &name, const QStringList &keys, const QStringList &values, bool global);
    bool deleteSubtitleStyle(const QString &name, bool global);
    bool setSubtitleStyleName(int subtitleId, const QString &styleName);

private:
    MainWindow *m_window;
};

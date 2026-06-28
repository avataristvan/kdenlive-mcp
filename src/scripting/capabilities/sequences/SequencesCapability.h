// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#pragma once

#include <QString>
#include <QVariantList>
#include <QVariantMap>

class MainWindow;

class SequencesCapability
{
public:
    explicit SequencesCapability(MainWindow *window);

    bool fillFrame(int clipId);
    QString renderBinFrame(const QString &binId, int frame, int width, int height, const QString &outputPath);
    QString renderTimelineFrame(int frame, int width, int height, const QString &outputPath);
    QString captureWindow(int maxSize, const QString &outputPath);

    QString createSequence(const QString &name, int audioTracks, int videoTracks, const QString &parentFolder);
    QVariantList getSequences();
    QVariantMap getActiveSequence();
    bool setActiveSequence(const QString &uuid);

    QVariantMap getZone();
    bool setZone(int inFrame, int outFrame);
    bool setZoneIn(int inFrame);
    bool setZoneOut(int outFrame);
    bool extractZone(int inFrame, int outFrame, bool liftOnly);

private:
    MainWindow *m_window;
};

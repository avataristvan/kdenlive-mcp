// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#pragma once

#include <QString>
#include <QStringList>
#include <QVariantMap>

class MainWindow;

class BinCapability
{
public:
    explicit BinCapability(MainWindow *window);

    QStringList importMedia(const QStringList &filePaths, const QString &folderId);
    QString createFolder(const QString &name, const QString &parentId);
    QStringList getAllClipIds();
    QStringList getFolderClipIds(const QString &folderId);
    QVariantMap getClipProperties(const QString &binId);
    bool deleteBinClip(const QString &binId);
    QString createTitleClip(const QString &titleXml, int durationFrames,
                            const QString &clipName, const QString &parentFolderId);
    QString getTitleXml(const QString &binId);
    bool setTitleXml(const QString &binId, const QString &newXml);
    bool renameBinClip(const QString &binId, const QString &newName);
    bool moveBinClip(const QString &binId, const QString &targetFolderId);
    QVariantMap getClipMetadata(const QString &binId);

private:
    MainWindow *m_window;
};

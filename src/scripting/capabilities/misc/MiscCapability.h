// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#pragma once

#include <QList>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class MainWindow;

class MiscCapability
{
public:
    explicit MiscCapability(MainWindow *window);

    // Undo/Redo
    bool undo(int steps);
    bool redo(int steps);
    QString undoStatus();

    // Relink
    bool relinkBinClip(const QString &binId, const QString &newFilePath);

    // Groups
    int groupClips(const QList<int> &itemIds);
    bool ungroupClips(int itemId);
    QVariantMap getGroupInfo(int itemId);
    bool removeFromGroup(int itemId);

    // Proxy
    QVariantMap getClipProxyStatus(const QString &binId);
    bool setClipProxy(const QString &binId, bool enabled);
    bool deleteClipProxy(const QString &binId);
    bool rebuildClipProxy(const QString &binId);

    // Selection
    QVariantList getSelection();
    bool setSelection(const QList<int> &ids);
    bool addToSelection(int itemId, bool clear);
    bool clearSelection();
    bool selectAll();
    bool selectCurrentTrack();
    bool selectItems(const QList<int> &trackIds, int startFrame, int endFrame);

private:
    MainWindow *m_window;
};

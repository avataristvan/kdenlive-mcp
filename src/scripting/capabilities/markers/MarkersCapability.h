// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#pragma once

#include <QString>
#include <QVariantList>

class MainWindow;

class MarkersCapability
{
public:
    explicit MarkersCapability(MainWindow *window);

    bool addGuide(int frame, const QString &comment, int category);
    QVariantList getGuides();
    bool deleteGuide(int frame);
    bool deleteGuidesByCategory(int category);

    bool addClipMarker(const QString &binId, int frame, const QString &comment, int category);
    QVariantList getClipMarkers(const QString &binId);
    bool deleteClipMarker(const QString &binId, int frame);
    bool deleteClipMarkersByCategory(const QString &binId, int category);

private:
    MainWindow *m_window;
};

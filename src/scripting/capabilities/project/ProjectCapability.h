// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#pragma once

#include <QString>

class MainWindow;

class ProjectCapability
{
public:
    explicit ProjectCapability(MainWindow *window);

    QString newProject(const QString &name);
    bool openProject(const QString &filePath);
    bool saveProject();
    bool saveProjectAs(const QString &filePath);
    QString getProjectName();
    QString getProjectPath();
    double getProjectFps();
    int getProjectResolutionWidth();
    int getProjectResolutionHeight();
    QString getProjectProperty(const QString &key);
    bool setProjectProperty(const QString &key, const QString &value);
    int getProjectDuration();
    QString getProjectColorSpace();
    bool setProjectColorSpace(const QString &colorSpace);
    int getProjectAudioSampleRate();

private:
    MainWindow *m_window;
};

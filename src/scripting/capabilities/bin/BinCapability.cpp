// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "BinCapability.h"
#include "bin/abstractprojectitem.h"
#include "bin/bin.h"
#include "bin/bincommands.h"
#include "bin/clipcreator.hpp"
#include "bin/projectclip.h"
#include "bin/projectfolder.h"
#include "bin/projectitemmodel.h"
#include "core.h"
#include "mainwindow.h"
#include <KLocalizedString>
#include <QFileInfo>
#include <QUndoCommand>
#include <QUrl>

BinCapability::BinCapability(MainWindow *window)
    : m_window(window)
{
}

QStringList BinCapability::importMedia(const QStringList &filePaths, const QString &folderId)
{
    QStringList binIds;
    if (!pCore->currentDoc()) return binIds;

    auto model = pCore->projectItemModel();
    QString folder = folderId;
    if (folder == QStringLiteral("-1")) {
        folder = model->getRootFolder()->clipId();
    }

    for (const QString &path : filePaths) {
        QStringList existing = model->getClipByUrl(QFileInfo(path));
        if (!existing.isEmpty()) {
            binIds.append(existing.first());
            continue;
        }
        ClipCreator::createClipFromFile(path, folder, model);
        QStringList newIds = model->getClipByUrl(QFileInfo(path));
        if (!newIds.isEmpty()) {
            binIds.append(newIds.first());
        }
    }
    return binIds;
}

QString BinCapability::createFolder(const QString &name, const QString &parentId)
{
    if (!pCore->currentDoc()) return {};

    auto model = pCore->projectItemModel();
    QString parent = parentId;
    if (parent == QStringLiteral("-1")) {
        parent = model->getRootFolder()->clipId();
    }

    QString newId;
    Fun undo = []() { return true; };
    Fun redo = []() { return true; };
    if (model->requestAddFolder(newId, name, parent, undo, redo)) {
        pCore->pushUndo(undo, redo, i18n("Create folder"));
        return newId;
    }
    return {};
}

QStringList BinCapability::getAllClipIds()
{
    if (!pCore->currentDoc()) return {};

    auto model = pCore->projectItemModel();
    std::vector<QString> ids = model->getAllClipIds();
    QStringList result;
    for (const QString &id : ids) {
        result.append(id);
    }
    return result;
}

QStringList BinCapability::getFolderClipIds(const QString &folderId)
{
    if (!pCore->currentDoc()) return {};

    auto model = pCore->projectItemModel();
    auto folder = model->getFolderByBinId(folderId);
    if (!folder) return {};

    QStringList result;
    int count = folder->childCount();
    for (int i = 0; i < count; i++) {
        auto child = std::static_pointer_cast<AbstractProjectItem>(folder->child(i));
        if (child && child->itemType() == AbstractProjectItem::ClipItem) {
            result.append(child->clipId());
        }
    }
    return result;
}

QVariantMap BinCapability::getClipProperties(const QString &binId)
{
    QVariantMap props;
    if (!pCore->currentDoc()) return props;

    auto clip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!clip) return props;

    props[QStringLiteral("name")] = clip->getData(AbstractProjectItem::DataName);
    props[QStringLiteral("type")] = clip->getData(AbstractProjectItem::ClipType);
    props[QStringLiteral("url")] = clip->clipUrl();
    props[QStringLiteral("id")] = binId;

    // Duration: convert cached SMPTE timecode "HH:MM:SS:FF" or "HH:MM:SS;FF" to frames
    QString durationStr = clip->getData(AbstractProjectItem::DataDuration).toString();
    int fps = qRound(pCore->getCurrentFps());
    if (!durationStr.isEmpty() && fps > 0) {
        QString normalized = durationStr.replace(QLatin1Char(';'), QLatin1Char(':'));
        QStringList parts = normalized.split(QLatin1Char(':'));
        if (parts.size() == 4) {
            props[QStringLiteral("duration")] = parts[0].toInt() * 3600 * fps + parts[1].toInt() * 60 * fps + parts[2].toInt() * fps + parts[3].toInt();
        } else {
            props[QStringLiteral("duration")] = 0;
        }
    } else {
        props[QStringLiteral("duration")] = 0;
    }

    return props;
}

bool BinCapability::deleteBinClip(const QString &binId)
{
    if (!pCore->currentDoc()) return false;

    auto model = pCore->projectItemModel();
    auto clip = model->getClipByBinID(binId);
    if (!clip) return false;

    Fun undo = []() { return true; };
    Fun redo = []() { return true; };
    bool result = model->requestBinClipDeletion(clip, undo, redo);
    if (result) {
        pCore->pushUndo(undo, redo, i18n("Delete clip"));
    }
    return result;
}

QString BinCapability::createTitleClip(const QString &titleXml, int durationFrames,
                                       const QString &clipName, const QString &parentFolderId)
{
    if (!pCore->currentDoc()) return QStringLiteral("-1");

    auto model = pCore->projectItemModel();
    QString folder = parentFolderId;
    if (folder == QStringLiteral("-1")) folder = model->getRootFolder()->clipId();

    std::unordered_map<QString, QString> properties;
    properties[QStringLiteral("xmldata")] = titleXml;

    return ClipCreator::createTitleClip(properties, durationFrames,
                                        clipName.isEmpty() ? i18n("Title clip") : clipName,
                                        folder, model);
}

QString BinCapability::getTitleXml(const QString &binId)
{
    if (!pCore->currentDoc()) return {};
    auto clip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!clip) return {};
    return clip->getProducerProperty(QStringLiteral("xmldata"));
}

bool BinCapability::setTitleXml(const QString &binId, const QString &newXml)
{
    if (!pCore->currentDoc()) return false;
    auto clip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!clip) return false;
    clip->setProducerProperty(QStringLiteral("xmldata"), newXml);
    clip->reloadProducer(false, false, false);
    return true;
}

bool BinCapability::renameBinClip(const QString &binId, const QString &newName)
{
    if (!pCore->currentDoc()) return false;
    if (newName.isEmpty()) return false;
    auto clip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!clip) return false;
    return clip->rename(newName, 0);
}

bool BinCapability::moveBinClip(const QString &binId, const QString &targetFolderId)
{
    if (!pCore->currentDoc()) return false;
    auto item = pCore->projectItemModel()->getItemByBinId(binId);
    if (!item) return false;
    auto currentParent = item->parent();
    if (!currentParent) return false;
    QString oldParentId = std::static_pointer_cast<AbstractProjectItem>(currentParent)->clipId();
    if (oldParentId == targetFolderId) return true;
    auto targetFolder = pCore->projectItemModel()->getFolderByBinId(targetFolderId);
    if (!targetFolder) return false;
    QMap<QString, std::pair<QString, QString>> idsMap;
    idsMap.insert(binId, {targetFolderId, oldParentId});
    auto *moveCommand = new QUndoCommand();
    new MoveBinClipCommand(pCore->bin(), idsMap, moveCommand);
    pCore->pushUndo(moveCommand);
    return true;
}

QVariantMap BinCapability::getClipMetadata(const QString &binId)
{
    QVariantMap meta;
    if (!pCore->currentDoc()) return meta;

    auto clip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!clip) return meta;

    meta[QStringLiteral("id")] = binId;
    meta[QStringLiteral("name")] = clip->getData(AbstractProjectItem::DataName);
    meta[QStringLiteral("type")] = clip->getData(AbstractProjectItem::ClipType);
    meta[QStringLiteral("url")] = clip->clipUrl();

    QFileInfo fileInfo(clip->clipUrl());
    if (fileInfo.exists()) {
        meta[QStringLiteral("fileSize")] = fileInfo.size();
        meta[QStringLiteral("fileName")] = fileInfo.fileName();
    }

    meta[QStringLiteral("videoCodec")] = clip->getProducerProperty(QStringLiteral("meta.media.0.codec.name"));
    meta[QStringLiteral("audioCodec")] = clip->getProducerProperty(QStringLiteral("meta.media.1.codec.name"));
    meta[QStringLiteral("width")] = clip->getProducerProperty(QStringLiteral("meta.media.width"));
    meta[QStringLiteral("height")] = clip->getProducerProperty(QStringLiteral("meta.media.height"));
    meta[QStringLiteral("frameRate")] = clip->getProducerProperty(QStringLiteral("meta.media.frame_rate_num"));
    meta[QStringLiteral("sampleRate")] = clip->getProducerProperty(QStringLiteral("meta.media.1.codec.sample_rate"));
    meta[QStringLiteral("channels")] = clip->getProducerProperty(QStringLiteral("meta.media.1.codec.channels"));

    // Duration from cached SMPTE timecode (safe, no producer lock)
    QString durationStr = clip->getData(AbstractProjectItem::DataDuration).toString();
    int fps = qRound(pCore->getCurrentFps());
    if (!durationStr.isEmpty() && fps > 0) {
        QString normalized = durationStr.replace(QLatin1Char(';'), QLatin1Char(':'));
        QStringList parts = normalized.split(QLatin1Char(':'));
        if (parts.size() == 4) {
            meta[QStringLiteral("durationFrames")] = parts[0].toInt() * 3600 * fps + parts[1].toInt() * 60 * fps + parts[2].toInt() * fps + parts[3].toInt();
        }
    }
    meta[QStringLiteral("durationTimecode")] = durationStr;

    return meta;
}

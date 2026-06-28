// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "SequencesCapability.h"
#include "bin/clipcreator.hpp"
#include "bin/projectclip.h"
#include "bin/projectitemmodel.h"
#include "core.h"
#include "definitions.h"
#include "doc/kdenlivedoc.h"
#include "doc/kthumb.h"
#include "effects/effectstack/model/effectstackmodel.hpp"
#include "kdenlivesettings.h"
#include "mainwindow.h"
#include "profiles/profilemodel.hpp"
#include "project/projectmanager.h"
#include "timeline2/model/timelinemodel.hpp"
#include "timeline2/view/timelinecontroller.h"
#include "timeline2/view/timelinewidget.h"
#include "undohelper.hpp"
#include <KLocalizedString>
#include <QImage>
#include <QPixmap>

SequencesCapability::SequencesCapability(MainWindow *window)
    : m_window(window)
{
}

// ── Fill Frame ────────────────────────────────────────────────────────────────

bool SequencesCapability::fillFrame(int clipId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!tl->model()->isClip(clipId)) return false;

    const QString binId = tl->model()->getClipBinId(clipId);
    auto binClip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!binClip) return false;

    const QSize srcSize = binClip->frameSize();
    const int srcW = srcSize.width();
    const int srcH = srcSize.height();
    if (srcW <= 0 || srcH <= 0) return false;

    const int projW = pCore->getCurrentProfile()->width();
    const int projH = pCore->getCurrentProfile()->height();

    if (srcW == projW && srcH == projH) return true;

    // Scale-to-fill: uniform scale so the smaller dimension fills the frame
    const double scale = qMax(double(projW) / srcW, double(projH) / srcH);
    const int w = qRound(srcW * scale);
    const int h = qRound(srcH * scale);
    const int x = (projW - w) / 2;
    const int y = (projH - h) / 2;

    const QString rect = QStringLiteral("%1 %2 %3 %4 1").arg(x).arg(y).arg(w).arg(h);

    stringMap params;
    params.insert(QStringLiteral("rect"), rect);
    params.insert(QStringLiteral("distort"), QStringLiteral("1"));

    auto stack = tl->model()->getClipEffectStackModel(clipId);
    if (!stack) return false;
    return stack->appendEffect(QStringLiteral("qtblend"), false, params);
}

// ── Render Frames ─────────────────────────────────────────────────────────────

QString SequencesCapability::renderBinFrame(const QString &binId, int frame, int width, int height, const QString &outputPath)
{
    auto clip = pCore->projectItemModel()->getClipByBinID(binId);
    if (!clip) {
        qWarning() << "renderBinFrame: clip not found:" << binId;
        return QString();
    }

    const QString sourceUrl = clip->url();
    if (sourceUrl.isEmpty()) {
        qWarning() << "renderBinFrame: clip has no source URL";
        return QString();
    }

    Mlt::Producer producer(pCore->getProjectProfile(), sourceUrl.toUtf8().constData());
    if (!producer.is_valid()) {
        qWarning() << "renderBinFrame: invalid producer for" << sourceUrl;
        return QString();
    }

    QImage img = KThumb::getFrame(&producer, frame, width, height);
    if (img.isNull()) {
        qWarning() << "renderBinFrame: got null image";
        return QString();
    }

    if (img.width() != width || img.height() != height) {
        img = img.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    if (!img.save(outputPath, "JPEG", 90)) {
        qWarning() << "renderBinFrame: failed to save" << outputPath;
        return QString();
    }

    return outputPath;
}

QString SequencesCapability::renderTimelineFrame(int frame, int width, int height, const QString &outputPath)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) {
        qWarning() << "renderTimelineFrame: no active timeline";
        return QString();
    }

    Mlt::Tractor *tractor = tl->model()->tractor();
    if (!tractor || !tractor->is_valid()) {
        qWarning() << "renderTimelineFrame: invalid tractor";
        return QString();
    }

    QImage img = KThumb::getFrame(tractor, frame, width, height);
    if (img.isNull()) {
        qWarning() << "renderTimelineFrame: got null image";
        return QString();
    }

    if (img.width() != width || img.height() != height) {
        img = img.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    if (!img.save(outputPath, "JPEG", 90)) {
        qWarning() << "renderTimelineFrame: failed to save" << outputPath;
        return QString();
    }

    return outputPath;
}

QString SequencesCapability::captureWindow(int maxSize, const QString &outputPath)
{
    QPixmap pixmap = m_window->grab();
    if (pixmap.isNull()) {
        qWarning() << "captureWindow: grab() returned null";
        return QString();
    }

    QImage img = pixmap.toImage();
    if (img.isNull()) {
        qWarning() << "captureWindow: toImage() returned null";
        return QString();
    }

    if (maxSize > 0 && (img.width() > maxSize || img.height() > maxSize)) {
        img = img.scaled(maxSize, maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    if (!img.save(outputPath, "JPEG", 90)) {
        qWarning() << "captureWindow: failed to save" << outputPath;
        return QString();
    }

    return outputPath;
}

// ── Sequences ─────────────────────────────────────────────────────────────────

QString SequencesCapability::createSequence(const QString &name, int audioTracks, int videoTracks, const QString &parentFolder)
{
    if (!pCore->currentDoc()) return QStringLiteral("-1");

    int aTracks = (audioTracks <= 0) ? KdenliveSettings::audiotracks() : audioTracks;
    int vTracks = (videoTracks <= 0) ? KdenliveSettings::videotracks() : videoTracks;

    Fun undo = []() { return true; };
    Fun redo = []() { return true; };

    QString binId = ClipCreator::createPlaylistClipWithUndo(name, {aTracks, vTracks}, parentFolder, pCore->projectItemModel(), undo, redo);

    if (binId != QStringLiteral("-1")) {
        pCore->pushUndo(undo, redo, i18n("Create sequence"));
    }
    return binId;
}

QVariantList SequencesCapability::getSequences()
{
    KdenliveDoc *project = pCore->currentDoc();
    if (!project) return {};

    QList<QUuid> uuids = project->getTimelinesUuids();
    QUuid activeUuid;
    auto *tl = m_window->getCurrentTimeline();
    if (tl) {
        activeUuid = tl->getUuid();
    }

    QVariantList result;
    for (const QUuid &uuid : uuids) {
        auto model = project->getTimeline(uuid, true);
        if (!model) continue;
        QVariantMap m;
        m[QStringLiteral("uuid")] = uuid.toString(QUuid::WithoutBraces);
        m[QStringLiteral("name")] = model->tractor() ? QString(model->tractor()->get("kdenlive:clipname")) : QString();
        m[QStringLiteral("duration")] = model->duration();
        m[QStringLiteral("tracks")] = model->getTracksCount();
        m[QStringLiteral("active")] = (uuid == activeUuid);
        result.append(m);
    }
    return result;
}

QVariantMap SequencesCapability::getActiveSequence()
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return {};

    KdenliveDoc *project = pCore->currentDoc();
    if (!project) return {};

    QUuid uuid = tl->getUuid();
    auto model = tl->model();

    QVariantMap m;
    m[QStringLiteral("uuid")] = uuid.toString(QUuid::WithoutBraces);
    m[QStringLiteral("name")] = model->tractor() ? QString(model->tractor()->get("kdenlive:clipname")) : QString();
    m[QStringLiteral("duration")] = model->duration();
    m[QStringLiteral("tracks")] = model->getTracksCount();
    return m;
}

bool SequencesCapability::setActiveSequence(const QString &uuid)
{
    QUuid targetUuid = QUuid::fromString(uuid);
    if (targetUuid.isNull()) return false;

    if (m_window->raiseTimeline(targetUuid)) {
        return true;
    }

    KdenliveDoc *project = pCore->currentDoc();
    if (!project) return false;

    auto binModel = pCore->projectItemModel();
    std::vector<QString> allIds = binModel->getAllClipIds();
    for (const QString &binId : allIds) {
        auto clip = binModel->getClipByBinID(binId);
        if (clip && clip->getSequenceUuid() == targetUuid) {
            return pCore->projectManager()->openTimeline(binId, -1, targetUuid, -1);
        }
    }
    return false;
}

// ── Zones ─────────────────────────────────────────────────────────────────────

QVariantMap SequencesCapability::getZone()
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->controller()) return {};

    QVariantMap m;
    m[QStringLiteral("zoneIn")] = tl->controller()->zoneIn();
    m[QStringLiteral("zoneOut")] = tl->controller()->zoneOut();
    return m;
}

bool SequencesCapability::setZone(int inFrame, int outFrame)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->controller()) return false;
    tl->controller()->setZone(QPoint(inFrame, outFrame), true);
    return true;
}

bool SequencesCapability::setZoneIn(int inFrame)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->controller()) return false;
    tl->controller()->setZoneIn(inFrame);
    return true;
}

bool SequencesCapability::setZoneOut(int outFrame)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->controller()) return false;
    tl->controller()->setZoneOut(outFrame);
    return true;
}

bool SequencesCapability::extractZone(int inFrame, int outFrame, bool liftOnly)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->controller()) return false;
    tl->controller()->extractZone(QPoint(inFrame, outFrame), liftOnly);
    return true;
}

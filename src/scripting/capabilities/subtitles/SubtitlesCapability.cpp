// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "SubtitlesCapability.h"
#include "bin/model/subtitlemodel.hpp"
#include "core.h"
#include "definitions.h"
#include "dialogs/subtitleedit.h"
#include "doc/kdenlivedoc.h"
#include "kdenlivesettings.h"
#include "mainwindow.h"
#include "timeline2/model/timelineitemmodel.hpp"
#include "timeline2/view/timelinewidget.h"
#include "undohelper.hpp"
#include "utils/gentime.h"

#include <KLocalizedString>
#include <QFile>

SubtitlesCapability::SubtitlesCapability(MainWindow *window)
    : m_window(window)
{
}

// ── Subtitles ────────────────────────────────────────────────────────────────

QVariantList SubtitlesCapability::getSubtitles()
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return {};
    if (!tl->model()->hasSubtitleModel()) return {};
    auto subModel = tl->model()->getSubtitleModel();
    double fps = pCore->getCurrentFps();
    auto allSubs = subModel->getAllSubtitles();
    QVariantList result;
    for (const auto &sub : allSubs) {
        int layer = sub.first.first;
        GenTime start = sub.first.second;
        const SubtitleEvent &event = sub.second;
        QVariantMap m;
        m[QStringLiteral("id")] = subModel->getIdForStartPos(layer, start);
        m[QStringLiteral("layer")] = layer;
        m[QStringLiteral("startFrame")] = start.frames(fps);
        m[QStringLiteral("endFrame")] = event.endTime().frames(fps);
        m[QStringLiteral("text")] = event.text();
        m[QStringLiteral("styleName")] = event.styleName();
        result.append(m);
    }
    return result;
}

int SubtitlesCapability::addSubtitle(int startFrame, int endFrame, const QString &text, int layer)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return -1;
    bool created = false;
    if (!tl->model()->hasSubtitleModel()) {
        tl->model()->createSubtitleModel();
        created = true;
    }
    auto subModel = tl->model()->getSubtitleModel();
    if (created) {
        pCore->subtitleWidget()->setModel(subModel);
        tl->connectSubtitleModel(true);
        KdenliveSettings::setShowSubtitles(true);
    }
    double fps = pCore->getCurrentFps();
    GenTime startPos(startFrame, fps);
    GenTime endPos(endFrame, fps);
    SubtitleEvent event(true, endPos, subModel->getLayerDefaultStyle(layer), QString(), 0, 0, 0, QString(), text);
    Fun undo = []() { return true; };
    Fun redo = []() { return true; };
    if (subModel->addSubtitle({layer, startPos}, event, undo, redo)) {
        pCore->pushUndo(undo, redo, i18n("Add subtitle"));
        Q_EMIT subModel->layoutChanged();
        return subModel->getIdForStartPos(layer, startPos);
    }
    return -1;
}

bool SubtitlesCapability::editSubtitle(int subtitleId, const QString &newText)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model() || !tl->model()->hasSubtitleModel()) return false;
    return tl->model()->getSubtitleModel()->editSubtitle(subtitleId, newText);
}

bool SubtitlesCapability::moveSubtitle(int subtitleId, int newStartFrame)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model() || !tl->model()->hasSubtitleModel()) return false;
    auto subModel = tl->model()->getSubtitleModel();
    double fps = pCore->getCurrentFps();
    GenTime newPos(newStartFrame, fps);
    int layer = subModel->getLayerForId(subtitleId);
    return subModel->moveSubtitle(subtitleId, layer, newPos, true, true);
}

bool SubtitlesCapability::resizeSubtitle(int subtitleId, int newDuration, bool fromRight)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model() || !tl->model()->hasSubtitleModel()) return false;
    return tl->model()->getSubtitleModel()->requestResize(subtitleId, newDuration, fromRight);
}

bool SubtitlesCapability::importSubtitle(const QString &filePath, int offset, const QString &encoding)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return false;
    if (!QFile::exists(filePath)) return false;
    bool created = false;
    if (!tl->model()->hasSubtitleModel()) {
        tl->model()->createSubtitleModel();
        created = true;
    }
    auto subModel = tl->model()->getSubtitleModel();
    if (created) {
        pCore->subtitleWidget()->setModel(subModel);
        tl->connectSubtitleModel(true);
        KdenliveSettings::setShowSubtitles(true);
    }
    double fps = pCore->getCurrentFps();
    QByteArray enc = encoding.isEmpty() ? QByteArrayLiteral("UTF-8") : encoding.toUtf8();
    subModel->importSubtitle(filePath, offset, true, fps, fps, enc);
    return true;
}

bool SubtitlesCapability::deleteSubtitle(int subtitleId)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model() || !tl->model()->hasSubtitleModel()) return false;
    return tl->model()->getSubtitleModel()->removeSubtitle(subtitleId);
}

bool SubtitlesCapability::exportSubtitles(const QString &filePath)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model() || !tl->model()->hasSubtitleModel()) return false;
    auto subModel = tl->model()->getSubtitleModel();
    int ix = pCore->currentDoc()->getSequenceProperty(tl->model()->uuid(), QStringLiteral("kdenlive:activeSubtitleIndex"), QStringLiteral("0")).toInt();
    subModel->copySubtitle(filePath, ix, false, false);
    return QFile::exists(filePath);
}

// ── Speech Recognition ────────────────────────────────────────────────────────

bool SubtitlesCapability::speechRecognition()
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->controller()) return false;
    tl->controller()->subtitleSpeechRecognition();
    return true;
}

// ── Subtitle Styles ───────────────────────────────────────────────────────────

static QVariantMap subtitleStyleToMap(const QString &name, const SubtitleStyle &s)
{
    QVariantMap m;
    m[QStringLiteral("name")] = name;
    m[QStringLiteral("fontName")] = s.fontName();
    m[QStringLiteral("fontSize")] = s.fontSize();
    m[QStringLiteral("primaryColour")] = s.primaryColour().name(QColor::HexArgb);
    m[QStringLiteral("secondaryColour")] = s.secondaryColour().name(QColor::HexArgb);
    m[QStringLiteral("outlineColour")] = s.outlineColour().name(QColor::HexArgb);
    m[QStringLiteral("backColour")] = s.backColour().name(QColor::HexArgb);
    m[QStringLiteral("bold")] = s.bold();
    m[QStringLiteral("italic")] = s.italic();
    m[QStringLiteral("underline")] = s.underline();
    m[QStringLiteral("strikeOut")] = s.strikeOut();
    m[QStringLiteral("scaleX")] = s.scaleX();
    m[QStringLiteral("scaleY")] = s.scaleY();
    m[QStringLiteral("spacing")] = s.spacing();
    m[QStringLiteral("angle")] = s.angle();
    m[QStringLiteral("borderStyle")] = s.borderStyle();
    m[QStringLiteral("outline")] = s.outline();
    m[QStringLiteral("shadow")] = s.shadow();
    m[QStringLiteral("alignment")] = s.alignment();
    m[QStringLiteral("marginL")] = s.marginL();
    m[QStringLiteral("marginR")] = s.marginR();
    m[QStringLiteral("marginV")] = s.marginV();
    return m;
}

static void applyStyleOverrides(SubtitleStyle &style, const QStringList &keys, const QStringList &values)
{
    int count = qMin(keys.size(), values.size());
    for (int i = 0; i < count; ++i) {
        const QString &k = keys.at(i);
        const QString &v = values.at(i);
        if (k == QLatin1String("fontName"))
            style.setFontName(v);
        else if (k == QLatin1String("fontSize"))
            style.setFontSize(v.toDouble());
        else if (k == QLatin1String("primaryColour"))
            style.setPrimaryColour(QColor(v));
        else if (k == QLatin1String("secondaryColour"))
            style.setSecondaryColour(QColor(v));
        else if (k == QLatin1String("outlineColour"))
            style.setOutlineColour(QColor(v));
        else if (k == QLatin1String("backColour"))
            style.setBackColour(QColor(v));
        else if (k == QLatin1String("bold"))
            style.setBold(v == QLatin1String("true") || v == QLatin1String("1"));
        else if (k == QLatin1String("italic"))
            style.setItalic(v == QLatin1String("true") || v == QLatin1String("1"));
        else if (k == QLatin1String("underline"))
            style.setUnderline(v == QLatin1String("true") || v == QLatin1String("1"));
        else if (k == QLatin1String("strikeOut"))
            style.setStrikeOut(v == QLatin1String("true") || v == QLatin1String("1"));
        else if (k == QLatin1String("scaleX"))
            style.setScaleX(v.toDouble());
        else if (k == QLatin1String("scaleY"))
            style.setScaleY(v.toDouble());
        else if (k == QLatin1String("spacing"))
            style.setSpacing(v.toDouble());
        else if (k == QLatin1String("angle"))
            style.setAngle(v.toDouble());
        else if (k == QLatin1String("borderStyle"))
            style.setBorderStyle(v.toInt());
        else if (k == QLatin1String("outline"))
            style.setOutline(v.toDouble());
        else if (k == QLatin1String("shadow"))
            style.setShadow(v.toDouble());
        else if (k == QLatin1String("alignment"))
            style.setAlignment(v.toInt());
        else if (k == QLatin1String("marginL"))
            style.setMarginL(v.toInt());
        else if (k == QLatin1String("marginR"))
            style.setMarginR(v.toInt());
        else if (k == QLatin1String("marginV"))
            style.setMarginV(v.toInt());
    }
}

QVariantList SubtitlesCapability::getSubtitleStyles(bool global)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model()) return {};
    if (!tl->model()->hasSubtitleModel()) return {};
    auto subModel = tl->model()->getSubtitleModel();
    const auto &styles = subModel->getAllSubtitleStyles(global);
    QVariantList result;
    for (const auto &pair : styles) {
        result.append(subtitleStyleToMap(pair.first, pair.second));
    }
    return result;
}

bool SubtitlesCapability::setSubtitleStyle(const QString &name, const QStringList &keys, const QStringList &values, bool global)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model() || !tl->model()->hasSubtitleModel()) return false;
    auto subModel = tl->model()->getSubtitleModel();
    const auto &styles = subModel->getAllSubtitleStyles(global);
    SubtitleStyle style;
    auto it = styles.find(name);
    if (it != styles.end()) {
        style = it->second;
    }
    applyStyleOverrides(style, keys, values);
    subModel->setSubtitleStyle(name, style, global);
    Q_EMIT subModel->layoutChanged();
    return true;
}

bool SubtitlesCapability::deleteSubtitleStyle(const QString &name, bool global)
{
    if (name == QLatin1String("Default")) return false;
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model() || !tl->model()->hasSubtitleModel()) return false;
    auto subModel = tl->model()->getSubtitleModel();
    subModel->deleteSubtitleStyle(name, global);
    Q_EMIT subModel->layoutChanged();
    return true;
}

bool SubtitlesCapability::setSubtitleStyleName(int subtitleId, const QString &styleName)
{
    auto *tl = m_window->getCurrentTimeline();
    if (!tl || !tl->model() || !tl->model()->hasSubtitleModel()) return false;
    auto subModel = tl->model()->getSubtitleModel();
    subModel->setStyleName(subtitleId, styleName);
    Q_EMIT subModel->layoutChanged();
    return true;
}

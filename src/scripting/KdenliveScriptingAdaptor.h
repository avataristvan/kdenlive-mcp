// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#pragma once

#include <QDBusAbstractAdaptor>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <memory>

class MainWindow;
class ProjectCapability;
class BinCapability;
class TimelineCapability;
class TransitionsCapability;
class EffectsCapability;
class AudioCapability;
class ClipPropertiesCapability;
class MarkersCapability;
class PlaybackCapability;
class SequencesCapability;
class SubtitlesCapability;
class MiscCapability;

/**
 * @brief D-Bus scripting adaptor for Kdenlive.
 *
 * Exposes all scripting operations via the org.kde.kdenlive.scripting D-Bus
 * interface without touching mainwindow.h. All access to internal state goes
 * through pCore and window()->getCurrentTimeline().
 */
class KdenliveScriptingAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kdenlive.scripting")

public:
    explicit KdenliveScriptingAdaptor(MainWindow *parent);
    ~KdenliveScriptingAdaptor();

    MainWindow *window() const;

public Q_SLOTS:
    // ── Project Management ────────────────────────────────────────────────
    QString scriptNewProject(const QString &name);
    bool scriptOpenProject(const QString &filePath);
    bool scriptSaveProject();
    bool scriptSaveProjectAs(const QString &filePath);
    QString scriptGetProjectName();
    QString scriptGetProjectPath();
    double scriptGetProjectFps();
    int scriptGetProjectResolutionWidth();
    int scriptGetProjectResolutionHeight();
    QString scriptGetProjectProperty(const QString &key);
    bool scriptSetProjectProperty(const QString &key, const QString &value);
    int scriptGetProjectDuration();
    QString scriptGetProjectColorSpace();
    bool scriptSetProjectColorSpace(const QString &colorSpace);
    int scriptGetProjectAudioSampleRate();

    // ── Media Pool (Bin) ──────────────────────────────────────────────────
    QStringList scriptImportMedia(const QStringList &filePaths, const QString &folderId = QStringLiteral("-1"));
    QString scriptCreateFolder(const QString &name, const QString &parentId = QStringLiteral("-1"));
    QStringList scriptGetAllClipIds();
    QStringList scriptGetFolderClipIds(const QString &folderId);
    QVariantMap scriptGetClipProperties(const QString &binId);
    bool scriptDeleteBinClip(const QString &binId);
    QString scriptCreateTitleClip(const QString &titleXml, int durationFrames,
                                  const QString &clipName = QStringLiteral("Title clip"),
                                  const QString &parentFolderId = QStringLiteral("-1"));
    QString scriptGetTitleXml(const QString &binId);
    bool scriptSetTitleXml(const QString &binId, const QString &newXml);
    bool scriptRenameBinClip(const QString &binId, const QString &newName);
    bool scriptMoveBinClip(const QString &binId, const QString &targetFolderId);
    QVariantMap scriptGetClipMetadata(const QString &binId);

    // ── Timeline Tracks ───────────────────────────────────────────────────
    int scriptGetTrackCount(const QString &trackType);
    QVariantMap scriptGetTrackInfo(int trackIndex);
    QVariantList scriptGetAllTracksInfo();
    int scriptAddTrack(const QString &name, bool audioTrack);
    bool scriptDeleteTrack(int trackId);
    bool scriptInsertSpace(int trackId, int position, int duration, bool allTracks);
    bool scriptRemoveSpace(int trackId, int position, bool allTracks);
    int scriptInsertClip(const QString &binClipId, int trackId, int position);
    QVariantList scriptInsertClipsSequentially(const QStringList &binClipIds, int trackId, int startPosition);
    bool scriptMoveClip(int clipId, int trackId, int position);
    int scriptResizeClip(int clipId, int newDuration, bool fromRight);
    bool scriptDeleteTimelineClip(int clipId);
    QVariantList scriptGetClipsOnTrack(int trackId);
    QVariantMap scriptGetTimelineClipInfo(int clipId);
    bool scriptSlipClip(int clipId, int offset);
    bool scriptCutClip(int clipId, int position);

    // ── Ripple / Roll / Slide ─────────────────────────────────────────────
    bool scriptRippleDelete(int clipId);
    bool scriptRippleTrim(int clipId, int delta, bool fromRight);
    bool scriptRollEdit(int clipId, int delta);
    bool scriptSlideEdit(int clipId, int delta);

    // ── Transitions & Mixes ───────────────────────────────────────────────
    bool scriptAddMix(int clipIdA, int clipIdB, int durationFrames);
    int scriptAddComposition(const QString &transitionId, int trackId, int position, int duration);
    bool scriptRemoveMix(int clipId);
    QVariantList scriptGetAvailableTransitions();
    QVariantMap scriptGetMixParams(int clipId);
    bool scriptSetMixDuration(int clipId, int newDuration);

    // ── Compositions ──────────────────────────────────────────────────────
    QVariantList scriptGetCompositions();
    QVariantMap scriptGetCompositionInfo(int compoId);
    bool scriptMoveComposition(int compoId, int trackId, int position);
    int scriptResizeComposition(int compoId, int newDuration, bool fromRight);
    bool scriptDeleteComposition(int compoId);
    QVariantList scriptGetCompositionTypes();
    bool scriptSetCompositionParam(int compoId, const QString &paramName, const QString &paramValue);
    QString scriptGetCompositionParam(int compoId, const QString &paramName);

    // ── Effects ───────────────────────────────────────────────────────────
    QVariantList scriptGetAvailableEffects();
    bool scriptAddClipEffect(int clipId, const QString &effectId, const QStringList &paramKeys, const QStringList &paramValues);
    bool scriptRemoveClipEffect(int clipId, const QString &effectId);
    QString scriptGetClipEffects(int clipId);
    bool scriptSetEffectParam(int clipId, const QString &effectId, const QString &paramName, const QString &paramValue);
    QString scriptGetEffectParam(int clipId, const QString &effectId, const QString &paramName);
    bool scriptSetEffectExpression(int clipId, const QString &effectId, const QString &paramName, const QString &expression, double baseValue);
    bool scriptClearEffectExpression(int clipId, const QString &effectId, const QString &paramName);
    QString scriptCopyClipEffects(int clipId);
    bool scriptPasteClipEffects(int targetClipId, const QString &effectsXml);

    // ── Clip Speed ────────────────────────────────────────────────────────
    bool scriptSetClipSpeed(int clipId, double speed, bool pitchCompensate);

    // ── Effect Keyframes (by index) ───────────────────────────────────────
    QVariantList scriptGetEffectKeyframes(int clipId, int effectIndex);
    bool scriptAddEffectKeyframe(int clipId, int effectIndex, int frame, double normalizedValue, int keyframeType);
    bool scriptRemoveEffectKeyframe(int clipId, int effectIndex, int frame);
    bool scriptUpdateEffectKeyframe(int clipId, int effectIndex, int oldFrame, int newFrame, double normalizedValue);

    // ── Effect Keyframes (by parameter name) ──────────────────────────────
    QVariantList scriptGetEffectKeyframesByParam(int clipId, const QString &effectId, const QString &paramName);
    bool scriptAddEffectKeyframeByParam(int clipId, const QString &effectId, const QString &paramName,
                                        int frame, const QString &value, int keyframeType);
    bool scriptRemoveEffectKeyframeByParam(int clipId, const QString &effectId, const QString &paramName, int frame);

    // ── Time Remap ────────────────────────────────────────────────────────
    bool scriptEnableTimeRemap(int clipId, bool enable);
    QVariantMap scriptGetTimeRemap(int clipId);
    bool scriptSetTimeRemap(int clipId, const QString &timeMap, int pitch, const QString &imageMode);

    // ── Clip Transform ────────────────────────────────────────────────────
    QVariantList scriptGetClipTransformKeyframes(int clipId);
    bool scriptSetClipTransform(int clipId, int frame, int x, int y, int width, int height, double opacity);
    bool scriptRemoveClipTransformKeyframe(int clipId, int frame);

    // ── Clip Properties ───────────────────────────────────────────────────
    double scriptGetClipOpacity(int clipId);
    bool scriptSetClipOpacity(int clipId, double opacity);
    bool scriptIsClipEnabled(int clipId);
    bool scriptSetClipEnabled(int clipId, bool enabled);
    QString scriptGetClipColor(int clipId);
    bool scriptSetClipColor(int clipId, const QString &colorTag);

    // ── Track Properties ──────────────────────────────────────────────────
    QString scriptGetTrackName(int trackId);
    bool scriptSetTrackName(int trackId, const QString &name);
    int scriptGetTrackColor(int trackId);
    bool scriptSetTrackColor(int trackId, int color);
    bool scriptGetTrackSolo(int trackId);
    bool scriptSetTrackSolo(int trackId, bool solo);

    // ── Audio ─────────────────────────────────────────────────────────────
    bool scriptSplitAudio(int clipId);
    bool scriptSetClipVolume(int clipId, double dB);
    double scriptGetClipVolume(int clipId);
    bool scriptSetAudioFade(int clipId, int fadeInFrames, int fadeOutFrames);
    bool scriptSetClipPan(int clipId, double pan);
    double scriptGetClipPan(int clipId);
    bool scriptSetTrackMute(int trackId, bool mute);
    bool scriptGetTrackMute(int trackId);
    bool scriptSetTrackLocked(int trackId, bool locked);
    bool scriptGetTrackLocked(int trackId);
    bool scriptSetTrackHidden(int trackId, bool hidden);
    bool scriptGetTrackHidden(int trackId);
    QVariantList scriptGetAudioLevels(const QString &binId, int stream, int downsample, int mode = 0);

    // ── Markers & Guides ──────────────────────────────────────────────────
    bool scriptAddGuide(int frame, const QString &comment, int category);
    QVariantList scriptGetGuides();
    bool scriptDeleteGuide(int frame);
    bool scriptDeleteGuidesByCategory(int category);

    // ── Clip-level Markers ────────────────────────────────────────────────
    bool scriptAddClipMarker(const QString &binId, int frame, const QString &comment, int category);
    QVariantList scriptGetClipMarkers(const QString &binId);
    bool scriptDeleteClipMarker(const QString &binId, int frame);
    bool scriptDeleteClipMarkersByCategory(const QString &binId, int category);

    // ── Render ────────────────────────────────────────────────────────────
    bool scriptRenderWithParams(const QString &outputFile, const QString &presetName,
                                int inFrame, int outFrame,
                                const QStringList &paramKeys, const QStringList &paramValues);
    QStringList scriptGetRenderPresets();
    QVariantList scriptGetRenderJobs();
    bool scriptAbortRenderJob(const QString &outputPath);

    // ── Project Profile ───────────────────────────────────────────────────
    bool scriptSetProjectProfile(int width, int height, int fpsNum, int fpsDen);

    // ── Copy / Cut / Paste ────────────────────────────────────────────────
    int scriptCopyClips();
    bool scriptCutClips();
    bool scriptPasteClips(int position, int trackId);

    // ── Playback & Monitor ────────────────────────────────────────────────
    void scriptSeek(int frame);
    int scriptGetPosition();
    void scriptPlay();
    void scriptPause();
    bool scriptSetPlaybackSpeed(double speed);
    double scriptGetPlaybackSpeed();

    // ── Timeline Navigation ───────────────────────────────────────────────
    int scriptGoToNextMarker();
    int scriptGoToPreviousMarker();
    int scriptGoToNextEdit();
    int scriptGoToPreviousEdit();

    // ── Sequences ─────────────────────────────────────────────────────────
    QString scriptCreateSequence(const QString &name, int audioTracks, int videoTracks,
                                 const QString &parentFolder);
    QVariantList scriptGetSequences();
    QVariantMap scriptGetActiveSequence();
    bool scriptSetActiveSequence(const QString &uuid);

    // ── Zones ─────────────────────────────────────────────────────────────
    QVariantMap scriptGetZone();
    bool scriptSetZone(int inFrame, int outFrame);
    bool scriptSetZoneIn(int inFrame);
    bool scriptSetZoneOut(int outFrame);
    bool scriptExtractZone(int inFrame, int outFrame, bool liftOnly);

    // ── Fill Frame ────────────────────────────────────────────────────────
    bool scriptFillFrame(int clipId);

    // ── Rendering Frames ──────────────────────────────────────────────────
    QString scriptRenderBinFrame(const QString &binId, int frame, int width, int height, const QString &outputPath);
    QString scriptRenderTimelineFrame(int frame, int width, int height, const QString &outputPath);
    QString scriptCaptureWindow(int maxSize, const QString &outputPath);

    // ── Subtitles ─────────────────────────────────────────────────────────
    QVariantList scriptGetSubtitles();
    int scriptAddSubtitle(int startFrame, int endFrame, const QString &text, int layer = 0);
    bool scriptEditSubtitle(int subtitleId, const QString &newText);
    bool scriptMoveSubtitle(int subtitleId, int newStartFrame);
    bool scriptResizeSubtitle(int subtitleId, int newDuration, bool fromRight);
    bool scriptImportSubtitle(const QString &filePath, int offset, const QString &encoding);
    bool scriptDeleteSubtitle(int subtitleId);
    bool scriptExportSubtitles(const QString &filePath);

    // ── Speech Recognition (stub) ─────────────────────────────────────────
    bool scriptSpeechRecognition();

    // ── Subtitle Styles ───────────────────────────────────────────────────
    QVariantList scriptGetSubtitleStyles(bool global);
    bool scriptSetSubtitleStyle(const QString &name, const QStringList &keys, const QStringList &values, bool global);
    bool scriptDeleteSubtitleStyle(const QString &name, bool global);
    bool scriptSetSubtitleStyleName(int subtitleId, const QString &styleName);

    // ── Groups ────────────────────────────────────────────────────────────
    int scriptGroupClips(const QList<int> &itemIds);
    bool scriptUngroupClips(int itemId);
    QVariantMap scriptGetGroupInfo(int itemId);
    bool scriptRemoveFromGroup(int itemId);

    // ── Proxy Clips ───────────────────────────────────────────────────────
    QVariantMap scriptGetClipProxyStatus(const QString &binId);
    bool scriptSetClipProxy(const QString &binId, bool enabled);
    bool scriptDeleteClipProxy(const QString &binId);
    bool scriptRebuildClipProxy(const QString &binId);

    // ── Relink ────────────────────────────────────────────────────────────
    bool scriptRelinkBinClip(const QString &binId, const QString &newFilePath);

    // ── Undo / Redo ───────────────────────────────────────────────────────
    bool scriptUndo(int steps = 1);
    bool scriptRedo(int steps = 1);
    QString scriptUndoStatus();

    // ── Selection ─────────────────────────────────────────────────────────
    QVariantList scriptGetSelection();
    bool scriptSetSelection(const QList<int> &ids);
    bool scriptAddToSelection(int itemId, bool clear);
    bool scriptClearSelection();
    bool scriptSelectAll();
    bool scriptSelectCurrentTrack();
    bool scriptSelectItems(const QList<int> &trackIds, int startFrame, int endFrame);

private:
    std::unique_ptr<ProjectCapability> m_project;
    std::unique_ptr<BinCapability> m_bin;
    std::unique_ptr<TimelineCapability> m_timeline;
    std::unique_ptr<TransitionsCapability> m_transitions;
    std::unique_ptr<EffectsCapability> m_effects;
    std::unique_ptr<AudioCapability> m_audio;
    std::unique_ptr<ClipPropertiesCapability> m_clipProperties;
    std::unique_ptr<MarkersCapability> m_markers;
    std::unique_ptr<PlaybackCapability> m_playback;
    std::unique_ptr<SequencesCapability> m_sequences;
    std::unique_ptr<SubtitlesCapability> m_subtitles;
    std::unique_ptr<MiscCapability> m_misc;
};

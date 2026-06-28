// SPDX-FileCopyrightText: 2024 Istvan Lovas
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "KdenliveScriptingAdaptor.h"
#include "capabilities/bin/BinCapability.h"
#include "capabilities/project/ProjectCapability.h"
#include "capabilities/timeline/TimelineCapability.h"
#include "capabilities/transitions/TransitionsCapability.h"
#include "capabilities/effects/EffectsCapability.h"
#include "capabilities/audio/AudioCapability.h"
#include "capabilities/clipproperties/ClipPropertiesCapability.h"
#include "capabilities/markers/MarkersCapability.h"
#include "capabilities/playback/PlaybackCapability.h"
#include "capabilities/sequences/SequencesCapability.h"
#include "capabilities/subtitles/SubtitlesCapability.h"
#include "capabilities/misc/MiscCapability.h"
#include "mainwindow.h"

KdenliveScriptingAdaptor::KdenliveScriptingAdaptor(MainWindow *parent)
    : QDBusAbstractAdaptor(parent)
    , m_project(std::make_unique<ProjectCapability>(parent))
    , m_bin(std::make_unique<BinCapability>(parent))
    , m_timeline(std::make_unique<TimelineCapability>(parent))
    , m_transitions(std::make_unique<TransitionsCapability>(parent))
    , m_effects(std::make_unique<EffectsCapability>(parent))
    , m_audio(std::make_unique<AudioCapability>(parent))
    , m_clipProperties(std::make_unique<ClipPropertiesCapability>(parent))
    , m_markers(std::make_unique<MarkersCapability>(parent))
    , m_playback(std::make_unique<PlaybackCapability>(parent))
    , m_sequences(std::make_unique<SequencesCapability>(parent))
    , m_subtitles(std::make_unique<SubtitlesCapability>(parent))
    , m_misc(std::make_unique<MiscCapability>(parent))
{
    setAutoRelaySignals(false);
}

KdenliveScriptingAdaptor::~KdenliveScriptingAdaptor() = default;

MainWindow *KdenliveScriptingAdaptor::window() const
{
    return static_cast<MainWindow *>(parent());
}

// ── Project Management ────────────────────────────────────────────────────
QString KdenliveScriptingAdaptor::scriptNewProject(const QString &name) { return m_project->newProject(name); }
bool KdenliveScriptingAdaptor::scriptOpenProject(const QString &filePath) { return m_project->openProject(filePath); }
bool KdenliveScriptingAdaptor::scriptSaveProject() { return m_project->saveProject(); }
bool KdenliveScriptingAdaptor::scriptSaveProjectAs(const QString &filePath) { return m_project->saveProjectAs(filePath); }
QString KdenliveScriptingAdaptor::scriptGetProjectName() { return m_project->getProjectName(); }
QString KdenliveScriptingAdaptor::scriptGetProjectPath() { return m_project->getProjectPath(); }
double KdenliveScriptingAdaptor::scriptGetProjectFps() { return m_project->getProjectFps(); }
int KdenliveScriptingAdaptor::scriptGetProjectResolutionWidth() { return m_project->getProjectResolutionWidth(); }
int KdenliveScriptingAdaptor::scriptGetProjectResolutionHeight() { return m_project->getProjectResolutionHeight(); }
QString KdenliveScriptingAdaptor::scriptGetProjectProperty(const QString &key) { return m_project->getProjectProperty(key); }
bool KdenliveScriptingAdaptor::scriptSetProjectProperty(const QString &key, const QString &value) { return m_project->setProjectProperty(key, value); }
int KdenliveScriptingAdaptor::scriptGetProjectDuration() { return m_project->getProjectDuration(); }
QString KdenliveScriptingAdaptor::scriptGetProjectColorSpace() { return m_project->getProjectColorSpace(); }
bool KdenliveScriptingAdaptor::scriptSetProjectColorSpace(const QString &colorSpace) { return m_project->setProjectColorSpace(colorSpace); }
int KdenliveScriptingAdaptor::scriptGetProjectAudioSampleRate() { return m_project->getProjectAudioSampleRate(); }

// ── Media Pool (Bin) ──────────────────────────────────────────────────────
QStringList KdenliveScriptingAdaptor::scriptImportMedia(const QStringList &filePaths, const QString &folderId) { return m_bin->importMedia(filePaths, folderId); }
QString KdenliveScriptingAdaptor::scriptCreateFolder(const QString &name, const QString &parentId) { return m_bin->createFolder(name, parentId); }
QStringList KdenliveScriptingAdaptor::scriptGetAllClipIds() { return m_bin->getAllClipIds(); }
QStringList KdenliveScriptingAdaptor::scriptGetFolderClipIds(const QString &folderId) { return m_bin->getFolderClipIds(folderId); }
QVariantMap KdenliveScriptingAdaptor::scriptGetClipProperties(const QString &binId) { return m_bin->getClipProperties(binId); }
bool KdenliveScriptingAdaptor::scriptDeleteBinClip(const QString &binId) { return m_bin->deleteBinClip(binId); }
QString KdenliveScriptingAdaptor::scriptCreateTitleClip(const QString &titleXml, int durationFrames, const QString &clipName, const QString &parentFolderId) { return m_bin->createTitleClip(titleXml, durationFrames, clipName, parentFolderId); }
QString KdenliveScriptingAdaptor::scriptGetTitleXml(const QString &binId) { return m_bin->getTitleXml(binId); }
bool KdenliveScriptingAdaptor::scriptSetTitleXml(const QString &binId, const QString &newXml) { return m_bin->setTitleXml(binId, newXml); }
bool KdenliveScriptingAdaptor::scriptRenameBinClip(const QString &binId, const QString &newName) { return m_bin->renameBinClip(binId, newName); }
bool KdenliveScriptingAdaptor::scriptMoveBinClip(const QString &binId, const QString &targetFolderId) { return m_bin->moveBinClip(binId, targetFolderId); }
QVariantMap KdenliveScriptingAdaptor::scriptGetClipMetadata(const QString &binId) { return m_bin->getClipMetadata(binId); }

// ── Timeline Tracks ───────────────────────────────────────────────────────
int KdenliveScriptingAdaptor::scriptGetTrackCount(const QString &trackType) { return m_timeline->getTrackCount(trackType); }
QVariantMap KdenliveScriptingAdaptor::scriptGetTrackInfo(int trackIndex) { return m_timeline->getTrackInfo(trackIndex); }
QVariantList KdenliveScriptingAdaptor::scriptGetAllTracksInfo() { return m_timeline->getAllTracksInfo(); }
int KdenliveScriptingAdaptor::scriptAddTrack(const QString &name, bool audioTrack) { return m_timeline->addTrack(name, audioTrack); }
bool KdenliveScriptingAdaptor::scriptDeleteTrack(int trackId) { return m_timeline->deleteTrack(trackId); }
bool KdenliveScriptingAdaptor::scriptInsertSpace(int trackId, int position, int duration, bool allTracks) { return m_timeline->insertSpace(trackId, position, duration, allTracks); }
bool KdenliveScriptingAdaptor::scriptRemoveSpace(int trackId, int position, bool allTracks) { return m_timeline->removeSpace(trackId, position, allTracks); }
int KdenliveScriptingAdaptor::scriptInsertClip(const QString &binClipId, int trackId, int position) { return m_timeline->insertClip(binClipId, trackId, position); }
QVariantList KdenliveScriptingAdaptor::scriptInsertClipsSequentially(const QStringList &binClipIds, int trackId, int startPosition) { return m_timeline->insertClipsSequentially(binClipIds, trackId, startPosition); }
bool KdenliveScriptingAdaptor::scriptMoveClip(int clipId, int trackId, int position) { return m_timeline->moveClip(clipId, trackId, position); }
int KdenliveScriptingAdaptor::scriptResizeClip(int clipId, int newDuration, bool fromRight) { return m_timeline->resizeClip(clipId, newDuration, fromRight); }
bool KdenliveScriptingAdaptor::scriptDeleteTimelineClip(int clipId) { return m_timeline->deleteTimelineClip(clipId); }
QVariantList KdenliveScriptingAdaptor::scriptGetClipsOnTrack(int trackId) { return m_timeline->getClipsOnTrack(trackId); }
QVariantMap KdenliveScriptingAdaptor::scriptGetTimelineClipInfo(int clipId) { return m_timeline->getTimelineClipInfo(clipId); }
bool KdenliveScriptingAdaptor::scriptSlipClip(int clipId, int offset) { return m_timeline->slipClip(clipId, offset); }
bool KdenliveScriptingAdaptor::scriptCutClip(int clipId, int position) { return m_timeline->cutClip(clipId, position); }

// ── Ripple / Roll / Slide ─────────────────────────────────────────────────
bool KdenliveScriptingAdaptor::scriptRippleDelete(int clipId) { return m_timeline->rippleDelete(clipId); }
bool KdenliveScriptingAdaptor::scriptRippleTrim(int clipId, int delta, bool fromRight) { return m_timeline->rippleTrim(clipId, delta, fromRight); }
bool KdenliveScriptingAdaptor::scriptRollEdit(int clipId, int delta) { return m_timeline->rollEdit(clipId, delta); }
bool KdenliveScriptingAdaptor::scriptSlideEdit(int clipId, int delta) { return m_timeline->slideEdit(clipId, delta); }

// ── Transitions & Mixes ───────────────────────────────────────────────────
bool KdenliveScriptingAdaptor::scriptAddMix(int clipIdA, int clipIdB, int durationFrames) { return m_transitions->addMix(clipIdA, clipIdB, durationFrames); }
int KdenliveScriptingAdaptor::scriptAddComposition(const QString &transitionId, int trackId, int position, int duration) { return m_transitions->addComposition(transitionId, trackId, position, duration); }
bool KdenliveScriptingAdaptor::scriptRemoveMix(int clipId) { return m_transitions->removeMix(clipId); }
QVariantList KdenliveScriptingAdaptor::scriptGetAvailableTransitions() { return m_transitions->getAvailableTransitions(); }
QVariantMap KdenliveScriptingAdaptor::scriptGetMixParams(int clipId) { return m_transitions->getMixParams(clipId); }
bool KdenliveScriptingAdaptor::scriptSetMixDuration(int clipId, int newDuration) { return m_transitions->setMixDuration(clipId, newDuration); }

// ── Compositions ──────────────────────────────────────────────────────────
QVariantList KdenliveScriptingAdaptor::scriptGetCompositions() { return m_transitions->getCompositions(); }
QVariantMap KdenliveScriptingAdaptor::scriptGetCompositionInfo(int compoId) { return m_transitions->getCompositionInfo(compoId); }
bool KdenliveScriptingAdaptor::scriptMoveComposition(int compoId, int trackId, int position) { return m_transitions->moveComposition(compoId, trackId, position); }
int KdenliveScriptingAdaptor::scriptResizeComposition(int compoId, int newDuration, bool fromRight) { return m_transitions->resizeComposition(compoId, newDuration, fromRight); }
bool KdenliveScriptingAdaptor::scriptDeleteComposition(int compoId) { return m_transitions->deleteComposition(compoId); }
QVariantList KdenliveScriptingAdaptor::scriptGetCompositionTypes() { return m_transitions->getCompositionTypes(); }
bool KdenliveScriptingAdaptor::scriptSetCompositionParam(int compoId, const QString &paramName, const QString &paramValue) { return m_transitions->setCompositionParam(compoId, paramName, paramValue); }
QString KdenliveScriptingAdaptor::scriptGetCompositionParam(int compoId, const QString &paramName) { return m_transitions->getCompositionParam(compoId, paramName); }

// ── Effects ───────────────────────────────────────────────────────────────
QVariantList KdenliveScriptingAdaptor::scriptGetAvailableEffects() { return m_effects->getAvailableEffects(); }
bool KdenliveScriptingAdaptor::scriptAddClipEffect(int clipId, const QString &effectId, const QStringList &paramKeys, const QStringList &paramValues) { return m_effects->addClipEffect(clipId, effectId, paramKeys, paramValues); }
bool KdenliveScriptingAdaptor::scriptRemoveClipEffect(int clipId, const QString &effectId) { return m_effects->removeClipEffect(clipId, effectId); }
QString KdenliveScriptingAdaptor::scriptGetClipEffects(int clipId) { return m_effects->getClipEffects(clipId); }
bool KdenliveScriptingAdaptor::scriptSetEffectParam(int clipId, const QString &effectId, const QString &paramName, const QString &paramValue) { return m_effects->setEffectParam(clipId, effectId, paramName, paramValue); }
QString KdenliveScriptingAdaptor::scriptGetEffectParam(int clipId, const QString &effectId, const QString &paramName) { return m_effects->getEffectParam(clipId, effectId, paramName); }
bool KdenliveScriptingAdaptor::scriptSetEffectExpression(int clipId, const QString &effectId, const QString &paramName, const QString &expression, double baseValue) { return m_effects->setEffectExpression(clipId, effectId, paramName, expression, baseValue); }
bool KdenliveScriptingAdaptor::scriptClearEffectExpression(int clipId, const QString &effectId, const QString &paramName) { return m_effects->clearEffectExpression(clipId, effectId, paramName); }
QString KdenliveScriptingAdaptor::scriptCopyClipEffects(int clipId) { return m_effects->copyClipEffects(clipId); }
bool KdenliveScriptingAdaptor::scriptPasteClipEffects(int targetClipId, const QString &effectsXml) { return m_effects->pasteClipEffects(targetClipId, effectsXml); }

// ── Clip Speed ────────────────────────────────────────────────────────────
bool KdenliveScriptingAdaptor::scriptSetClipSpeed(int clipId, double speed, bool pitchCompensate) { return m_effects->setClipSpeed(clipId, speed, pitchCompensate); }

// ── Effect Keyframes by index ────────────────────────────────────────────
QVariantList KdenliveScriptingAdaptor::scriptGetEffectKeyframes(int clipId, int effectIndex) { return m_effects->getEffectKeyframes(clipId, effectIndex); }
bool KdenliveScriptingAdaptor::scriptAddEffectKeyframe(int clipId, int effectIndex, int frame, double normalizedValue, int keyframeType) { return m_effects->addEffectKeyframe(clipId, effectIndex, frame, normalizedValue, keyframeType); }
bool KdenliveScriptingAdaptor::scriptRemoveEffectKeyframe(int clipId, int effectIndex, int frame) { return m_effects->removeEffectKeyframe(clipId, effectIndex, frame); }
bool KdenliveScriptingAdaptor::scriptUpdateEffectKeyframe(int clipId, int effectIndex, int oldFrame, int newFrame, double normalizedValue) { return m_effects->updateEffectKeyframe(clipId, effectIndex, oldFrame, newFrame, normalizedValue); }

// ── Effect Keyframes by param ────────────────────────────────────────────
QVariantList KdenliveScriptingAdaptor::scriptGetEffectKeyframesByParam(int clipId, const QString &effectId, const QString &paramName) { return m_effects->getEffectKeyframesByParam(clipId, effectId, paramName); }
bool KdenliveScriptingAdaptor::scriptAddEffectKeyframeByParam(int clipId, const QString &effectId, const QString &paramName, int frame, const QString &value, int keyframeType) { return m_effects->addEffectKeyframeByParam(clipId, effectId, paramName, frame, value, keyframeType); }
bool KdenliveScriptingAdaptor::scriptRemoveEffectKeyframeByParam(int clipId, const QString &effectId, const QString &paramName, int frame) { return m_effects->removeEffectKeyframeByParam(clipId, effectId, paramName, frame); }

// ── Time Remap ────────────────────────────────────────────────────────────
bool KdenliveScriptingAdaptor::scriptEnableTimeRemap(int clipId, bool enable) { return m_effects->enableTimeRemap(clipId, enable); }
QVariantMap KdenliveScriptingAdaptor::scriptGetTimeRemap(int clipId) { return m_effects->getTimeRemap(clipId); }
bool KdenliveScriptingAdaptor::scriptSetTimeRemap(int clipId, const QString &timeMap, int pitch, const QString &imageMode) { return m_effects->setTimeRemap(clipId, timeMap, pitch, imageMode); }

// ── Clip Transform ────────────────────────────────────────────────────────
QVariantList KdenliveScriptingAdaptor::scriptGetClipTransformKeyframes(int clipId) { return m_clipProperties->getClipTransformKeyframes(clipId); }
bool KdenliveScriptingAdaptor::scriptSetClipTransform(int clipId, int frame, int x, int y, int width, int height, double opacity) { return m_clipProperties->setClipTransform(clipId, frame, x, y, width, height, opacity); }
bool KdenliveScriptingAdaptor::scriptRemoveClipTransformKeyframe(int clipId, int frame) { return m_clipProperties->removeClipTransformKeyframe(clipId, frame); }

// ── Clip Properties ───────────────────────────────────────────────────────
double KdenliveScriptingAdaptor::scriptGetClipOpacity(int clipId) { return m_clipProperties->getClipOpacity(clipId); }
bool KdenliveScriptingAdaptor::scriptSetClipOpacity(int clipId, double opacity) { return m_clipProperties->setClipOpacity(clipId, opacity); }
bool KdenliveScriptingAdaptor::scriptIsClipEnabled(int clipId) { return m_clipProperties->isClipEnabled(clipId); }
bool KdenliveScriptingAdaptor::scriptSetClipEnabled(int clipId, bool enabled) { return m_clipProperties->setClipEnabled(clipId, enabled); }
QString KdenliveScriptingAdaptor::scriptGetClipColor(int clipId) { return m_clipProperties->getClipColor(clipId); }
bool KdenliveScriptingAdaptor::scriptSetClipColor(int clipId, const QString &colorTag) { return m_clipProperties->setClipColor(clipId, colorTag); }

// ── Track Properties ──────────────────────────────────────────────────────
QString KdenliveScriptingAdaptor::scriptGetTrackName(int trackId) { return m_clipProperties->getTrackName(trackId); }
bool KdenliveScriptingAdaptor::scriptSetTrackName(int trackId, const QString &name) { return m_clipProperties->setTrackName(trackId, name); }
int KdenliveScriptingAdaptor::scriptGetTrackColor(int trackId) { return m_clipProperties->getTrackColor(trackId); }
bool KdenliveScriptingAdaptor::scriptSetTrackColor(int trackId, int color) { return m_clipProperties->setTrackColor(trackId, color); }
bool KdenliveScriptingAdaptor::scriptGetTrackSolo(int trackId) { return m_clipProperties->getTrackSolo(trackId); }
bool KdenliveScriptingAdaptor::scriptSetTrackSolo(int trackId, bool solo) { return m_clipProperties->setTrackSolo(trackId, solo); }

// ── Audio ─────────────────────────────────────────────────────────────────
bool KdenliveScriptingAdaptor::scriptSplitAudio(int clipId) { return m_audio->splitAudio(clipId); }
bool KdenliveScriptingAdaptor::scriptSetClipVolume(int clipId, double dB) { return m_audio->setClipVolume(clipId, dB); }
double KdenliveScriptingAdaptor::scriptGetClipVolume(int clipId) { return m_audio->getClipVolume(clipId); }
bool KdenliveScriptingAdaptor::scriptSetAudioFade(int clipId, int fadeInFrames, int fadeOutFrames) { return m_audio->setAudioFade(clipId, fadeInFrames, fadeOutFrames); }
bool KdenliveScriptingAdaptor::scriptSetClipPan(int clipId, double pan) { return m_audio->setClipPan(clipId, pan); }
double KdenliveScriptingAdaptor::scriptGetClipPan(int clipId) { return m_audio->getClipPan(clipId); }
bool KdenliveScriptingAdaptor::scriptSetTrackMute(int trackId, bool mute) { return m_audio->setTrackMute(trackId, mute); }
bool KdenliveScriptingAdaptor::scriptGetTrackMute(int trackId) { return m_audio->getTrackMute(trackId); }
bool KdenliveScriptingAdaptor::scriptSetTrackLocked(int trackId, bool locked) { return m_audio->setTrackLocked(trackId, locked); }
bool KdenliveScriptingAdaptor::scriptGetTrackLocked(int trackId) { return m_audio->getTrackLocked(trackId); }
bool KdenliveScriptingAdaptor::scriptSetTrackHidden(int trackId, bool hidden) { return m_audio->setTrackHidden(trackId, hidden); }
bool KdenliveScriptingAdaptor::scriptGetTrackHidden(int trackId) { return m_audio->getTrackHidden(trackId); }
QVariantList KdenliveScriptingAdaptor::scriptGetAudioLevels(const QString &binId, int stream, int downsample, int mode) { return m_audio->getAudioLevels(binId, stream, downsample, mode); }

// ── Markers & Guides ──────────────────────────────────────────────────────
bool KdenliveScriptingAdaptor::scriptAddGuide(int frame, const QString &comment, int category) { return m_markers->addGuide(frame, comment, category); }
QVariantList KdenliveScriptingAdaptor::scriptGetGuides() { return m_markers->getGuides(); }
bool KdenliveScriptingAdaptor::scriptDeleteGuide(int frame) { return m_markers->deleteGuide(frame); }
bool KdenliveScriptingAdaptor::scriptDeleteGuidesByCategory(int category) { return m_markers->deleteGuidesByCategory(category); }

// ── Clip-level Markers ────────────────────────────────────────────────────
bool KdenliveScriptingAdaptor::scriptAddClipMarker(const QString &binId, int frame, const QString &comment, int category) { return m_markers->addClipMarker(binId, frame, comment, category); }
QVariantList KdenliveScriptingAdaptor::scriptGetClipMarkers(const QString &binId) { return m_markers->getClipMarkers(binId); }
bool KdenliveScriptingAdaptor::scriptDeleteClipMarker(const QString &binId, int frame) { return m_markers->deleteClipMarker(binId, frame); }
bool KdenliveScriptingAdaptor::scriptDeleteClipMarkersByCategory(const QString &binId, int category) { return m_markers->deleteClipMarkersByCategory(binId, category); }

// ── Render ────────────────────────────────────────────────────────────────
bool KdenliveScriptingAdaptor::scriptRenderWithParams(const QString &outputFile, const QString &presetName, int inFrame, int outFrame, const QStringList &paramKeys, const QStringList &paramValues) { return m_playback->renderWithParams(outputFile, presetName, inFrame, outFrame, paramKeys, paramValues); }
QStringList KdenliveScriptingAdaptor::scriptGetRenderPresets() { return m_playback->getRenderPresets(); }
QVariantList KdenliveScriptingAdaptor::scriptGetRenderJobs() { return m_playback->getRenderJobs(); }
bool KdenliveScriptingAdaptor::scriptAbortRenderJob(const QString &outputPath) { return m_playback->abortRenderJob(outputPath); }

// ── Project Profile ───────────────────────────────────────────────────────
bool KdenliveScriptingAdaptor::scriptSetProjectProfile(int width, int height, int fpsNum, int fpsDen) { return m_playback->setProjectProfile(width, height, fpsNum, fpsDen); }

// ── Copy / Cut / Paste ────────────────────────────────────────────────────
int KdenliveScriptingAdaptor::scriptCopyClips() { return m_playback->copyClips(); }
bool KdenliveScriptingAdaptor::scriptCutClips() { return m_playback->cutClips(); }
bool KdenliveScriptingAdaptor::scriptPasteClips(int position, int trackId) { return m_playback->pasteClips(position, trackId); }

// ── Playback & Monitor ────────────────────────────────────────────────────
void KdenliveScriptingAdaptor::scriptSeek(int frame) { m_playback->seek(frame); }
int KdenliveScriptingAdaptor::scriptGetPosition() { return m_playback->getPosition(); }
void KdenliveScriptingAdaptor::scriptPlay() { m_playback->play(); }
void KdenliveScriptingAdaptor::scriptPause() { m_playback->pause(); }
bool KdenliveScriptingAdaptor::scriptSetPlaybackSpeed(double speed) { return m_playback->setPlaybackSpeed(speed); }
double KdenliveScriptingAdaptor::scriptGetPlaybackSpeed() { return m_playback->getPlaybackSpeed(); }

// ── Timeline Navigation ───────────────────────────────────────────────────
int KdenliveScriptingAdaptor::scriptGoToNextMarker() { return m_playback->goToNextMarker(); }
int KdenliveScriptingAdaptor::scriptGoToPreviousMarker() { return m_playback->goToPreviousMarker(); }
int KdenliveScriptingAdaptor::scriptGoToNextEdit() { return m_playback->goToNextEdit(); }
int KdenliveScriptingAdaptor::scriptGoToPreviousEdit() { return m_playback->goToPreviousEdit(); }

// ── Sequences ─────────────────────────────────────────────────────────────
QString KdenliveScriptingAdaptor::scriptCreateSequence(const QString &name, int audioTracks, int videoTracks, const QString &parentFolder) { return m_sequences->createSequence(name, audioTracks, videoTracks, parentFolder); }
QVariantList KdenliveScriptingAdaptor::scriptGetSequences() { return m_sequences->getSequences(); }
QVariantMap KdenliveScriptingAdaptor::scriptGetActiveSequence() { return m_sequences->getActiveSequence(); }
bool KdenliveScriptingAdaptor::scriptSetActiveSequence(const QString &uuid) { return m_sequences->setActiveSequence(uuid); }

// ── Zones ─────────────────────────────────────────────────────────────────
QVariantMap KdenliveScriptingAdaptor::scriptGetZone() { return m_sequences->getZone(); }
bool KdenliveScriptingAdaptor::scriptSetZone(int inFrame, int outFrame) { return m_sequences->setZone(inFrame, outFrame); }
bool KdenliveScriptingAdaptor::scriptSetZoneIn(int inFrame) { return m_sequences->setZoneIn(inFrame); }
bool KdenliveScriptingAdaptor::scriptSetZoneOut(int outFrame) { return m_sequences->setZoneOut(outFrame); }
bool KdenliveScriptingAdaptor::scriptExtractZone(int inFrame, int outFrame, bool liftOnly) { return m_sequences->extractZone(inFrame, outFrame, liftOnly); }

// ── Fill Frame ────────────────────────────────────────────────────────────
bool KdenliveScriptingAdaptor::scriptFillFrame(int clipId) { return m_sequences->fillFrame(clipId); }

// ── Rendering Frames ──────────────────────────────────────────────────────
QString KdenliveScriptingAdaptor::scriptRenderBinFrame(const QString &binId, int frame, int width, int height, const QString &outputPath) { return m_sequences->renderBinFrame(binId, frame, width, height, outputPath); }
QString KdenliveScriptingAdaptor::scriptRenderTimelineFrame(int frame, int width, int height, const QString &outputPath) { return m_sequences->renderTimelineFrame(frame, width, height, outputPath); }
QString KdenliveScriptingAdaptor::scriptCaptureWindow(int maxSize, const QString &outputPath) { return m_sequences->captureWindow(maxSize, outputPath); }

// ── Subtitles ─────────────────────────────────────────────────────────────
QVariantList KdenliveScriptingAdaptor::scriptGetSubtitles() { return m_subtitles->getSubtitles(); }
int KdenliveScriptingAdaptor::scriptAddSubtitle(int startFrame, int endFrame, const QString &text, int layer) { return m_subtitles->addSubtitle(startFrame, endFrame, text, layer); }
bool KdenliveScriptingAdaptor::scriptEditSubtitle(int subtitleId, const QString &newText) { return m_subtitles->editSubtitle(subtitleId, newText); }
bool KdenliveScriptingAdaptor::scriptMoveSubtitle(int subtitleId, int newStartFrame) { return m_subtitles->moveSubtitle(subtitleId, newStartFrame); }
bool KdenliveScriptingAdaptor::scriptResizeSubtitle(int subtitleId, int newDuration, bool fromRight) { return m_subtitles->resizeSubtitle(subtitleId, newDuration, fromRight); }
bool KdenliveScriptingAdaptor::scriptImportSubtitle(const QString &filePath, int offset, const QString &encoding) { return m_subtitles->importSubtitle(filePath, offset, encoding); }
bool KdenliveScriptingAdaptor::scriptDeleteSubtitle(int subtitleId) { return m_subtitles->deleteSubtitle(subtitleId); }
bool KdenliveScriptingAdaptor::scriptExportSubtitles(const QString &filePath) { return m_subtitles->exportSubtitles(filePath); }

// ── Speech Recognition ─────────────────────────────────────────────────────
bool KdenliveScriptingAdaptor::scriptSpeechRecognition() { return m_subtitles->speechRecognition(); }

// ── Subtitle Styles ───────────────────────────────────────────────────────
QVariantList KdenliveScriptingAdaptor::scriptGetSubtitleStyles(bool global) { return m_subtitles->getSubtitleStyles(global); }
bool KdenliveScriptingAdaptor::scriptSetSubtitleStyle(const QString &name, const QStringList &keys, const QStringList &values, bool global) { return m_subtitles->setSubtitleStyle(name, keys, values, global); }
bool KdenliveScriptingAdaptor::scriptDeleteSubtitleStyle(const QString &name, bool global) { return m_subtitles->deleteSubtitleStyle(name, global); }
bool KdenliveScriptingAdaptor::scriptSetSubtitleStyleName(int subtitleId, const QString &styleName) { return m_subtitles->setSubtitleStyleName(subtitleId, styleName); }

// ── Groups ────────────────────────────────────────────────────────────────
int KdenliveScriptingAdaptor::scriptGroupClips(const QList<int> &itemIds) { return m_misc->groupClips(itemIds); }
bool KdenliveScriptingAdaptor::scriptUngroupClips(int itemId) { return m_misc->ungroupClips(itemId); }
QVariantMap KdenliveScriptingAdaptor::scriptGetGroupInfo(int itemId) { return m_misc->getGroupInfo(itemId); }
bool KdenliveScriptingAdaptor::scriptRemoveFromGroup(int itemId) { return m_misc->removeFromGroup(itemId); }

// ── Proxy Clips ───────────────────────────────────────────────────────────
QVariantMap KdenliveScriptingAdaptor::scriptGetClipProxyStatus(const QString &binId) { return m_misc->getClipProxyStatus(binId); }
bool KdenliveScriptingAdaptor::scriptSetClipProxy(const QString &binId, bool enabled) { return m_misc->setClipProxy(binId, enabled); }
bool KdenliveScriptingAdaptor::scriptDeleteClipProxy(const QString &binId) { return m_misc->deleteClipProxy(binId); }
bool KdenliveScriptingAdaptor::scriptRebuildClipProxy(const QString &binId) { return m_misc->rebuildClipProxy(binId); }

// ── Relink ────────────────────────────────────────────────────────────────
bool KdenliveScriptingAdaptor::scriptRelinkBinClip(const QString &binId, const QString &newFilePath) { return m_misc->relinkBinClip(binId, newFilePath); }

// ── Undo / Redo ───────────────────────────────────────────────────────────
bool KdenliveScriptingAdaptor::scriptUndo(int steps) { return m_misc->undo(steps); }
bool KdenliveScriptingAdaptor::scriptRedo(int steps) { return m_misc->redo(steps); }
QString KdenliveScriptingAdaptor::scriptUndoStatus() { return m_misc->undoStatus(); }

// ── Selection ─────────────────────────────────────────────────────────────
QVariantList KdenliveScriptingAdaptor::scriptGetSelection() { return m_misc->getSelection(); }
bool KdenliveScriptingAdaptor::scriptSetSelection(const QList<int> &ids) { return m_misc->setSelection(ids); }
bool KdenliveScriptingAdaptor::scriptAddToSelection(int itemId, bool clear) { return m_misc->addToSelection(itemId, clear); }
bool KdenliveScriptingAdaptor::scriptClearSelection() { return m_misc->clearSelection(); }
bool KdenliveScriptingAdaptor::scriptSelectAll() { return m_misc->selectAll(); }
bool KdenliveScriptingAdaptor::scriptSelectCurrentTrack() { return m_misc->selectCurrentTrack(); }
bool KdenliveScriptingAdaptor::scriptSelectItems(const QList<int> &trackIds, int startFrame, int endFrame) { return m_misc->selectItems(trackIds, startFrame, endFrame); }

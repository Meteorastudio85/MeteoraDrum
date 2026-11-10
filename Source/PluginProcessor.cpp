#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

using namespace juce;

namespace
{
const char* const kPieceIds[md::NumPieces]   = { "kick", "snare", "tom", "floor", "hihat", "crashl", "crashr", "ride" };
const char* const kPieceNames[md::NumPieces] = { "Kick", "Snare", "Tom", "Floor", "Hi-Hat", "Crash 1", "Crash 2", "Ride" };

// valori di default dei canali
const float kDefaultLevel[md::NumPieces] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, -4.f };
const float kDefaultPan[md::NumPieces]   = { 0.f, -0.08f, -0.2f, 0.5f, -0.55f, 0.f, 0.f, 0.55f };

String dbText (float v, int)   { return v <= -59.9f ? String ("-inf") : String (v, 1) + " dB"; }
String pctText (float v, int)  { return String (roundToInt (v * 100.f)) + " %"; }
String panText (float v, int)
{
    int p = roundToInt (v * 100.f);
    if (p == 0) return "C";
    return p < 0 ? "L " + String (-p) : "R " + String (p);
}
String stText (float v, int)   { return (v > 0 ? "+" : "") + String (v, 1) + " st"; }
String hzText (float v, int)   { return String (roundToInt (v)) + " Hz"; }
String secText (float v, int)  { return String (v, 2) + " s"; }

std::unique_ptr<AudioParameterFloat> fparam (const String& id, const String& name, NormalisableRange<float> r, float def,
                                             std::function<String (float, int)> fmt)
{
    return std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, r, def,
                                                  AudioParameterFloatAttributes().withStringFromValueFunction (std::move (fmt)));
}

std::unique_ptr<AudioParameterBool> bparam (const String& id, const String& name, bool def)
{
    return std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, name, def);
}

NormalisableRange<float> faderRange (float minDb, float maxDb)
{
    NormalisableRange<float> r (minDb, maxDb, 0.1f);
    r.setSkewForCentre (-12.f);
    return r;
}

// ---- Preset di mix -----------------------------------------------------------
struct PresetValue { const char* id; float value; };
struct MixPreset { const char* name; const char* desc; std::vector<PresetValue> values; };

// KIT PRESET: scelta dei suoni (A/B), microfoni e mix
const std::vector<MixPreset>& presets()
{
    static const std::vector<MixPreset> list =
    {
        { "Meteora Default", "nastro 180 cm \xc2\xb7 tom, floor, crash 1, hi-hat B \xc2\xb7 room corta",
          { { "oh_ribbon", 1.f }, { "oh_spacing", 1.f }, { "hh_mic_x", 0.22f }, { "hh_mic_h", 1.f },
            { "set_tom", 1.f }, { "set_floor", 1.f }, { "set_crash1", 1.f }, { "set_hihat", 1.f },
            { "master_eq", 1.f }, { "master_bass", 1.f }, { "master_treble", -1.5f }, { "master_width", 0.85f },
            { "tom_decay", 0.85f }, { "floor_decay", 0.85f }, { "kick_comp", 0.35f }, { "snare_comp", 0.35f },
            { "kick_bleed", 0.75f }, { "oh_locut", 25.f }, { "oh_width", 0.85f }, { "oh_air", 0.f },
            { "room_size", 0.2f }, { "room_decay", 0.45f }, { "room_comp", 0.4f }, { "room_locut", 50.f },
            { "snare_room", 0.5f }, { "hihat_room", 0.25f }, { "hihat_pan", -0.22f },
            { "snare_level", -5.5f }, { "hihat_level", -5.5f }, { "room_level", -17.f } } },
        { "Studio Naturale", "set A \xc2\xb7 condensatori 110 cm \xc2\xb7 room media", {} },
        { "Warm Indie", "crash B \xc2\xb7 nastro 90 cm \xc2\xb7 room calda e compressa",
          { { "set_crash1", 1.f }, { "set_crash2", 1.f }, { "oh_ribbon", 1.f }, { "oh_spacing", 0.36f },
            { "room_level", -4.f }, { "room_size", 0.45f }, { "room_comp", 0.6f }, { "kick_comp", 0.3f }, { "snare_comp", 0.3f },
            { "master_eq", 1.f }, { "master_bass", 1.5f }, { "master_treble", -1.f } } },
        { "Vintage Tape '72", "kick, snare, tom B \xc2\xb7 nastro stretti \xc2\xb7 treble -1,5 dB",
          { { "set_kick", 1.f }, { "set_snare", 1.f }, { "set_tom", 1.f }, { "oh_ribbon", 1.f }, { "oh_spacing", 0.21f },
            { "master_eq", 1.f }, { "master_treble", -1.5f }, { "master_bass", 1.f }, { "oh_air", 0.f }, { "oh_width", 0.7f },
            { "room_size", 0.35f }, { "room_comp", 0.85f }, { "room_level", -5.f }, { "tom_decay", 0.85f }, { "floor_decay", 0.85f },
            { "kick_comp", 0.35f }, { "snare_comp", 0.35f }, { "master_width", 0.85f } } },
        { "Big Rock Room", "set A \xc2\xb7 condensatori larghi 150 cm \xc2\xb7 big room",
          { { "room_type", 1.f }, { "oh_spacing", 0.79f }, { "room_level", -1.f }, { "room_size", 0.85f }, { "room_decay", 1.6f }, { "room_comp", 0.75f },
            { "oh_level", -2.f }, { "oh_width", 1.25f }, { "kick_room", 1.2f }, { "snare_room", 1.3f }, { "tom_room", 1.2f },
            { "floor_room", 1.2f }, { "kick_comp", 0.4f }, { "snare_comp", 0.4f } } },
        { "Dry Punch", "kick B \xc2\xb7 condensatori 80 cm \xc2\xb7 comp alto \xc2\xb7 poca room",
          { { "set_kick", 1.f }, { "oh_spacing", 0.29f }, { "kick_comp", 0.55f }, { "snare_comp", 0.45f }, { "tom_comp", 0.35f },
            { "floor_comp", 0.35f }, { "kick_level", 1.5f }, { "snare_level", 1.f }, { "room_level", -14.f }, { "room_comp", 0.95f },
            { "oh_locut", 220.f }, { "oh_level", -5.f }, { "master_limit", 0.35f } } },
        { "Garage", "tutto B \xc2\xb7 nastro 100 cm \xc2\xb7 room schiacciata",
          { { "set_kick", 1.f }, { "set_snare", 1.f }, { "set_tom", 1.f }, { "set_floor", 1.f }, { "set_hihat", 1.f },
            { "set_crash1", 1.f }, { "set_crash2", 1.f }, { "oh_ribbon", 1.f }, { "oh_spacing", 0.43f }, { "room_comp", 0.95f },
            { "room_size", 0.4f }, { "room_level", -3.f }, { "master_limit", 0.3f }, { "kick_bleed", 0.5f }, { "snare_bleed", 0.5f } } },
        { "Half Time Heavy", "kick, floor B \xc2\xb7 condensatori larghi \xc2\xb7 big room \xc2\xb7 limit",
          { { "room_type", 1.f }, { "set_kick", 1.f }, { "set_floor", 1.f }, { "oh_spacing", 0.86f }, { "master_limit", 0.45f }, { "kick_comp", 0.45f },
            { "snare_comp", 0.4f }, { "room_level", -4.f }, { "room_size", 0.7f }, { "room_decay", 1.3f },
            { "master_eq", 1.f }, { "master_bass", 2.f } } },
        { "Ballad", "set A \xc2\xb7 condensatori 130 cm \xc2\xb7 velocity morbida",
          { { "oh_spacing", 0.64f }, { "room_decay", 1.8f }, { "room_size", 0.7f }, { "room_level", -6.f }, { "velcurve", 0.f },
            { "kick_comp", 0.2f }, { "snare_comp", 0.2f }, { "oh_air", 3.f } } },
    };
    return list;
}
} // namespace

String MeteoraDrumProcessor::pieceId (int piece) { return kPieceIds[jlimit (0, md::NumPieces - 1, piece)]; }

AudioProcessorValueTreeState::ParameterLayout MeteoraDrumProcessor::createLayout()
{
    AudioProcessorValueTreeState::ParameterLayout layout;

    for (int p : kStripPieces)
    {
        const String id = kPieceIds[p], nm = kPieceNames[p];
        layout.add (fparam (id + "_level", nm + " Level", faderRange (-60.f, 12.f), kDefaultLevel[p], dbText));
        layout.add (fparam (id + "_pan",   nm + " Pan",   { -1.f, 1.f, 0.01f }, kDefaultPan[p], panText));
        layout.add (fparam (id + "_tune",  nm + " Tune",  { -12.f, 12.f, 0.1f }, 0.f, stText));
        layout.add (fparam (id + "_decay", nm + " Decay", { 0.05f, 1.f, 0.01f }, 1.f, pctText));
        layout.add (fparam (id + "_comp",  nm + " Comp",  { 0.f, 1.f, 0.01f }, 0.f, pctText));
        if (hasOHSend (p))
            layout.add (fparam (id + "_oh", nm + " OH Send", { 0.f, 1.5f, 0.01f }, 1.f, pctText));
        layout.add (fparam (id + "_room",  nm + " Room Send", { 0.f, 1.5f, 0.01f }, 1.f, pctText));
        if (hasBleed (p))
            layout.add (fparam (id + "_bleed", nm + (p == md::Kick ? " Bleed (Snare)" : " Bleed (Kick)"), { 0.f, 1.f, 0.01f }, 0.3f, pctText));
        layout.add (bparam (id + "_mute",  nm + " Mute", false));
        layout.add (bparam (id + "_solo",  nm + " Solo", false));
    }

    NormalisableRange<float> cut (20.f, 500.f, 1.f); cut.setSkewForCentre (120.f);

    layout.add (fparam ("oh_level", "Overheads Level", faderRange (-60.f, 12.f), -3.f, dbText));
    layout.add (bparam ("oh_mute", "Overheads Mute", false));
    layout.add (bparam ("oh_solo", "Overheads Solo", false));
    layout.add (fparam ("oh_width", "Overheads Width", { 0.f, 2.f, 0.01f }, 1.f, pctText));
    layout.add (fparam ("oh_locut", "Overheads Low Cut", cut, 120.f, hzText));
    layout.add (fparam ("oh_air", "Overheads Air", { 0.f, 10.f, 0.1f }, 2.f, dbText));

    layout.add (fparam ("room_level", "Room Level", faderRange (-60.f, 12.f), -6.f, dbText));
    layout.add (bparam ("room_mute", "Room Mute", false));
    layout.add (bparam ("room_solo", "Room Solo", false));
    layout.add (fparam ("room_size", "Room Size", { 0.f, 1.f, 0.01f }, 0.5f, pctText));
    layout.add (fparam ("room_decay", "Room Decay", { 0.2f, 3.f, 0.01f }, 0.9f, secText));
    layout.add (fparam ("room_comp", "Room Comp", { 0.f, 1.f, 0.01f }, 0.5f, pctText));
    layout.add (fparam ("room_locut", "Room Low Cut", cut, 80.f, hzText));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "room_type", 1 }, "Room Type", StringArray { "Studio", "Big Room" }, 0));
    layout.add (bparam ("phase_align", "Phase Align", true));

    layout.add (fparam ("master", "Master Volume", faderRange (-60.f, 6.f), 0.f, dbText));
    layout.add (bparam ("master_mute", "Master Mute", false));
    layout.add (bparam ("master_eq", "Master EQ On", false));
    layout.add (fparam ("master_bass", "Master Bass", { -12.f, 12.f, 0.1f }, 0.f, dbText));
    layout.add (fparam ("master_treble", "Master Treble", { -12.f, 12.f, 0.1f }, 0.f, dbText));
    layout.add (fparam ("master_width", "Master Width", { 0.f, 2.f, 0.01f }, 1.f, pctText));
    layout.add (fparam ("master_limit", "Master Limit", { 0.f, 1.f, 0.01f }, 0.f, pctText));

    // set A/B e microfoni (schermata Kit)
    const char* setIds[md::NumSetGroups]   = { "set_kick", "set_snare", "set_tom", "set_floor", "set_hihat", "set_crash1", "set_crash2" };
    const char* setNames[md::NumSetGroups] = { "Kick Set B", "Snare Set B", "Tom Set B", "Floor Set B", "Hi-Hat Set B", "Crash 1 Set B", "Crash 2 Set B" };
    for (int i = 0; i < md::NumSetGroups; ++i) layout.add (bparam (setIds[i], setNames[i], false));
    layout.add (fparam ("oh_spacing", "Overheads Spacing", { 0.f, 1.f, 0.01f }, 0.5f,
                        [] (float v, int) { return String (roundToInt (40.f + 140.f * v)) + " cm"; }));
    layout.add (bparam ("oh_ribbon", "Overheads Ribbon", false));
    layout.add (fparam ("hh_mic_x", "Hi-Hat Mic Position", { 0.f, 1.f, 0.01f }, 0.5f,
                        [] (float v, int) { return v < 0.2f ? String ("bordo") : v > 0.8f ? String ("campana") : String ("tra campana e bordo"); }));
    layout.add (fparam ("hh_mic_h", "Hi-Hat Mic Height", { 0.f, 1.f, 0.01f }, 0.25f,
                        [] (float v, int) { return String (roundToInt (3.f + 12.f * v)) + " cm"; }));

    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "velcurve", 1 }, "Velocity Curve",
                                                        StringArray { "Morbida", "Normale", "Dura" }, 1));
    layout.add (bparam ("hhcc4", "Hi-Hat Pedal CC4", true));
    return layout;
}

MeteoraDrumProcessor::MeteoraDrumProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "MeteoraDrum", createLayout())
{
    for (int p : kStripPieces)
    {
        const String id = kPieceIds[p];
        auto& c = chp[p];
        c.level = apvts.getRawParameterValue (id + "_level");
        c.pan   = apvts.getRawParameterValue (id + "_pan");
        c.tune  = apvts.getRawParameterValue (id + "_tune");
        c.decay = apvts.getRawParameterValue (id + "_decay");
        c.comp  = apvts.getRawParameterValue (id + "_comp");
        c.oh    = hasOHSend (p) ? apvts.getRawParameterValue (id + "_oh") : nullptr;
        c.room  = apvts.getRawParameterValue (id + "_room");
        c.bleed = hasBleed (p) ? apvts.getRawParameterValue (id + "_bleed") : nullptr;
        c.mute  = apvts.getRawParameterValue (id + "_mute");
        c.solo  = apvts.getRawParameterValue (id + "_solo");
    }
    const char* ohIds[6]     = { "oh_level", "oh_mute", "oh_solo", "oh_width", "oh_locut", "oh_air" };
    const char* roomIds[7]   = { "room_level", "room_mute", "room_solo", "room_size", "room_decay", "room_comp", "room_locut" };
    const char* masterIds[7] = { "master", "master_mute", "master_eq", "master_bass", "master_treble", "master_width", "master_limit" };
    for (int i = 0; i < 6; ++i) ohP[i] = apvts.getRawParameterValue (ohIds[i]);
    for (int i = 0; i < 7; ++i) roomP[i] = apvts.getRawParameterValue (roomIds[i]);
    for (int i = 0; i < 7; ++i) masterP[i] = apvts.getRawParameterValue (masterIds[i]);
    const char* setIds[md::NumSetGroups] = { "set_kick", "set_snare", "set_tom", "set_floor", "set_hihat", "set_crash1", "set_crash2" };
    for (int i = 0; i < md::NumSetGroups; ++i) setP[i] = apvts.getRawParameterValue (setIds[i]);
    ohSpacingP = apvts.getRawParameterValue ("oh_spacing");
    ohRibbonP  = apvts.getRawParameterValue ("oh_ribbon");
    hhMicXP    = apvts.getRawParameterValue ("hh_mic_x");
    hhMicHP    = apvts.getRawParameterValue ("hh_mic_h");
    roomTypeP   = apvts.getRawParameterValue ("room_type");
    phaseAlignP = apvts.getRawParameterValue ("phase_align");
    velCurveP = apvts.getRawParameterValue ("velcurve");
    hhCC4P    = apvts.getRawParameterValue ("hhcc4");

    loadSamples();
    loadGrooves();
    setCurrentProgram (0);   // all'apertura: Meteora Default
}

String MeteoraDrumProcessor::getPresetDescription (int index)
{
    return isPositiveAndBelow (index, (int) presets().size()) ? String (CharPointer_UTF8 (presets()[(size_t) index].desc)) : String();
}

// ---- groove -------------------------------------------------------------------
void MeteoraDrumProcessor::loadGrooves()
{
    grooves.clear();
    grooves.resize ((size_t) kNumGrooves);
    for (int i = 0; i < kNumGrooves; ++i)
    {
        const auto& info = kGrooves[i];
        auto& gd = grooves[(size_t) i];
        gd.lengthPpq = 4.0 * info.bars;
        int packSize = 0;
        const char* pack = BinaryData::getNamedResource ("grooves_pack", packSize);
        if (pack == nullptr || info.offset + info.size > packSize) continue;
        MemoryInputStream in (pack + info.offset, (size_t) info.size, false);
        MidiFile mf;
        if (! mf.readFrom (in)) continue;
        const double ppq = (double) jmax (1, (int) mf.getTimeFormat());
        const double msPerTick = 60000.0 / ((double) info.bpm * ppq);
        for (int t = 0; t < mf.getNumTracks(); ++t)
            for (auto* ev : *mf.getTrack (t))
            {
                const auto& m = ev->message;
                if (! m.isNoteOn()) continue;
                const double tick = m.getTimeStamp();
                // il tocco umano è salvato come spostamento in ms dalla griglia (16mi o terzine di 8vi)
                const double g16 = std::round (tick / (ppq / 4.0)) * (ppq / 4.0);
                const double g3  = std::round (tick / (ppq / 3.0)) * (ppq / 3.0);
                const double grid = std::abs (tick - g16) <= std::abs (tick - g3) ? g16 : g3;
                gd.events.push_back ({ grid / ppq, (float) ((tick - grid) * msPerTick), m.getNoteNumber(), (int) m.getVelocity(), 0.f });
            }
        // velocity media per nota: con UMANIZZA a 0 la dinamica si appiattisce verso questa media
        std::map<int, std::pair<double, int>> acc;
        for (auto& e : gd.events) { acc[e.note].first += e.velocity; acc[e.note].second++; }
        for (auto& e : gd.events) e.meanVel = (float) (acc[e.note].first / acc[e.note].second);
        std::sort (gd.events.begin(), gd.events.end(), [] (const GrooveEvent& a, const GrooveEvent& b) { return a.gridPpq < b.gridPpq; });
    }
}

void MeteoraDrumProcessor::runGroovePlayer (MidiBuffer& midi, int n)
{
    const int req = grooveRequest.exchange (-1);
    if (req >= 0 && req < (int) grooves.size()) { playIndex = req; needsStart = true; }
    else if (req == -2) playIndex = -1;

    double bpm = (double) grooveBpm.load();
    bool hostPlaying = false;
    double hostPpq = 0.0;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm()) { hostBpm.store ((float) *b); if (grooveSync.load()) bpm = *b; }
            if (auto p = pos->getPpqPosition()) hostPpq = *p;
            hostPlaying = pos->getIsPlaying();
        }

    if (playIndex < 0) { grooveCurrent.store (-1); groovePos.store (-1.f); wasHostPlaying = false; return; }

    const auto& g = grooves[(size_t) playIndex];
    const bool loop = grooveLoop.load();
    const float hum = grooveHumanize.load();
    const bool synced = grooveSync.load() && hostPlaying;
    const double ppqPerSample = jmax (20.0, bpm) / 60.0 / getSampleRate();

    double start;
    if (synced)
    {
        // con la DAW in riproduzione parte all'inizio della battuta successiva
        if (needsStart || ! wasHostPlaying) { startPpq = std::ceil (hostPpq / 4.0 - 1.0e-6) * 4.0; needsStart = false; }
        start = hostPpq - startPpq;
    }
    else
    {
        if (needsStart || wasHostPlaying) { localPpq = 0.0; needsStart = false; }
        start = localPpq;
    }
    wasHostPlaying = synced;
    const double end = start + (double) n * ppqPerSample;

    const int kFirst = loop ? (int) std::floor (start / g.lengthPpq) : 0;
    const int kLast  = loop ? (int) std::floor (end / g.lengthPpq) + 1 : 0;
    for (int k = jmax (0, kFirst - 1); k <= kLast; ++k)
    {
        const double base = (double) k * g.lengthPpq;
        for (const auto& e : g.events)
        {
            // il tocco umano resta in millisecondi a qualsiasi tempo
            const double t = base + e.gridPpq + (double) (e.offsetMs * hum) * bpm / 60000.0;
            if (t < start || t >= end || t < 0.0) continue;
            const int pos = jlimit (0, n - 1, (int) ((t - start) / ppqPerSample));
            const int vel = roundToInt (e.meanVel + ((float) e.velocity - e.meanVel) * hum);
            midi.addEvent (MidiMessage::noteOn (10, e.note, (uint8) jlimit (1, 127, vel)), pos);
        }
    }

    if (! synced) localPpq = end;
    if (! loop && start > g.lengthPpq + 0.5) { playIndex = -1; grooveCurrent.store (-1); groovePos.store (-1.f); return; }
    grooveCurrent.store (playIndex);
    groovePos.store (start < 0.0 ? -1.f : (float) std::fmod (start, g.lengthPpq));
}

void MeteoraDrumProcessor::loadSamples()
{
    WavAudioFormat wav;
    numSoundsLoaded = 0;

    for (int s = 0; s < md::NumSlots; ++s)
    {
        const auto& info = md::kSlotInfo[s];
        auto& smp = engine.samples[(size_t) s];
        smp.piece = info.piece;
        smp.baseGainDb = info.baseGainDb;

        int size = 0;
        const char* data = BinaryData::getNamedResource (info.resource, size);
        if (data == nullptr || size <= 0) continue;

        auto* stream = new MemoryInputStream (data, (size_t) size, false);
        std::unique_ptr<AudioFormatReader> reader (wav.createReaderFor (stream, true));
        if (reader == nullptr) continue;

        const int len = (int) reader->lengthInSamples;
        AudioBuffer<float> buf (1, len);
        reader->read (&buf, 0, len, 0, true, false);

        smp.data.assign (buf.getReadPointer (0), buf.getReadPointer (0) + len);
        smp.sampleRate = reader->sampleRate;
        ++numSoundsLoaded;
    }
}

bool MeteoraDrumProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == AudioChannelSet::stereo();
}

void MeteoraDrumProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, jmax (samplesPerBlock, 512));
    loadMeasurer.reset (sampleRate, samplesPerBlock);
    keyboardState.reset();
    updateEngineParams();
}

void MeteoraDrumProcessor::updateEngineParams()
{
    auto db = [] (std::atomic<float>* a) { const float v = a->load(); return v <= -59.9f ? -100.f : v; };

    md::EngineParams p;
    for (int i = 0; i < md::NumPieces; ++i)
    {
        auto& c = chp[i];
        auto& d = p.ch[i];
        if (c.level == nullptr) continue;   // crash: niente canale, valori fissi
        d.levelDb  = db (c.level);
        d.pan      = c.pan->load();
        d.tune     = c.tune->load();
        d.decay    = c.decay->load();
        d.comp     = c.comp->load();
        d.ohSend   = c.oh != nullptr ? c.oh->load() : 1.f;
        d.roomSend = c.room->load();
        d.bleed    = c.bleed != nullptr ? c.bleed->load() : 0.f;
        d.mute     = c.mute->load() > 0.5f;
        d.solo     = c.solo->load() > 0.5f;
    }
    p.oh.levelDb = db (ohP[0]);
    p.oh.mute = ohP[1]->load() > 0.5f;  p.oh.solo = ohP[2]->load() > 0.5f;
    p.oh.p1 = ohP[3]->load(); p.oh.p2 = ohP[4]->load(); p.oh.p3 = ohP[5]->load();

    p.room.levelDb = db (roomP[0]);
    p.room.mute = roomP[1]->load() > 0.5f;  p.room.solo = roomP[2]->load() > 0.5f;
    p.room.p1 = roomP[3]->load(); p.room.p2 = roomP[4]->load(); p.room.p3 = roomP[5]->load(); p.room.p4 = roomP[6]->load();

    p.masterDb    = db (masterP[0]);
    p.masterMute  = masterP[1]->load() > 0.5f;
    p.eqOn        = masterP[2]->load() > 0.5f;
    p.bassDb      = masterP[3]->load();
    p.trebleDb    = masterP[4]->load();
    p.masterWidth = masterP[5]->load();
    p.limit       = masterP[6]->load();

    for (int i = 0; i < md::NumSetGroups; ++i) p.setB[i] = setP[i]->load() > 0.5f;
    p.ohSpacing = ohSpacingP->load();
    p.ohRibbon  = ohRibbonP->load() > 0.5f ? 1.f : 0.f;
    p.hhMicX    = hhMicXP->load();
    p.hhMicH    = hhMicHP->load();
    p.roomType   = roundToInt (roomTypeP->load());
    p.phaseAlign = phaseAlignP->load() > 0.5f;
    p.velCurve   = roundToInt (velCurveP->load());
    p.hhPedalCC4 = hhCC4P->load() > 0.5f;
    engine.setParams (p);
}

void MeteoraDrumProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    AudioProcessLoadMeasurer::ScopedTimer timer (loadMeasurer, n);

    // clic sul kit nell'interfaccia e anteprima dei groove
    keyboardState.processNextMidiBuffer (midi, 0, n, true);
    runGroovePlayer (midi, n);
    updateEngineParams();

    buffer.clear();
    if (buffer.getNumChannels() < 2) return;
    float* L = buffer.getWritePointer (0);
    float* R = buffer.getWritePointer (1);

    int pos = 0;
    for (const auto meta : midi)
    {
        const auto msg = meta.getMessage();
        const int at = jlimit (0, n, meta.samplePosition);
        if (at > pos) { engine.render (L + pos, R + pos, at - pos); pos = at; }

        if (msg.isNoteOn())                                   engine.noteOn (msg.getNoteNumber(), msg.getVelocity());
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())  engine.allNotesOff();
        else if (msg.isController())                          engine.controller (msg.getControllerNumber(), msg.getControllerValue());
        else if (msg.isAftertouch())                          engine.polyPressure (msg.getNoteNumber(), msg.getAfterTouchValue());
    }
    if (pos < n) engine.render (L + pos, R + pos, n - pos);
}

// ---- Preset ----------------------------------------------------------------
int MeteoraDrumProcessor::getNumPrograms() { return (int) presets().size(); }

const String MeteoraDrumProcessor::getProgramName (int index)
{
    return isPositiveAndBelow (index, (int) presets().size()) ? String (presets()[(size_t) index].name) : String();
}

void MeteoraDrumProcessor::setCurrentProgram (int index)
{
    if (! isPositiveAndBelow (index, (int) presets().size())) return;
    currentProgram = index;

    // si parte sempre dal mix naturale, mute e solo esclusi
    for (auto* p : getParameters())
        if (auto* rp = dynamic_cast<RangedAudioParameter*> (p))
        {
            const String pid = rp->paramID;
            // i kit preset impostano suoni, microfoni e mix (mute, solo e pedale CC4 restano come sono)
            if (pid.endsWith ("_mute") || pid.endsWith ("_solo") || pid == "hhcc4" || pid == "phase_align")
                continue;
            rp->setValueNotifyingHost (rp->getDefaultValue());
        }

    for (const auto& v : presets()[(size_t) index].values)
        if (auto* rp = apvts.getParameter (v.id))
            rp->setValueNotifyingHost (rp->convertTo0to1 (v.value));
}

// ---- Stato -----------------------------------------------------------------
void MeteoraDrumProcessor::getStateInformation (MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void MeteoraDrumProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto tree = ValueTree::fromXml (*xml);
            currentProgram = jlimit (0, getNumPrograms() - 1, (int) tree.getProperty ("program", 0));
            apvts.replaceState (tree);
        }
}

MemoryBlock MeteoraDrumProcessor::buildGrooveMidi (int index) const
{
    // file MIDI del groove con l'umanizzazione scelta (al tempo consigliato del pattern)
    MemoryBlock out;
    if (! isPositiveAndBelow (index, (int) grooves.size())) return out;
    const auto& info = kGrooves[index];
    const auto& gd = grooves[(size_t) index];
    const float hum = grooveHumanize.load();
    const double ppq = 480.0;
    MidiMessageSequence seq;
    auto name = MidiMessage::textMetaEvent (3, String (info.style) + " - " + info.name + " - " + info.desc);
    name.setTimeStamp (0); seq.addEvent (name);
    auto tempo = MidiMessage::tempoMetaEvent (roundToInt (60000000.0 / info.bpm));
    tempo.setTimeStamp (0); seq.addEvent (tempo);
    auto ts = MidiMessage::timeSignatureMetaEvent (4, 4);
    ts.setTimeStamp (0); seq.addEvent (ts);
    for (const auto& e : gd.events)
    {
        const double t = jmax (0.0, e.gridPpq * ppq + (double) (e.offsetMs * hum) * info.bpm / 60000.0 * ppq);
        const int vel = jlimit (1, 127, roundToInt (e.meanVel + ((float) e.velocity - e.meanVel) * hum));
        auto on = MidiMessage::noteOn (10, e.note, (uint8) vel);   on.setTimeStamp (std::round (t));
        auto off = MidiMessage::noteOff (10, e.note);             off.setTimeStamp (std::round (t) + 60.0);
        seq.addEvent (on); seq.addEvent (off);
    }
    auto end = MidiMessage::endOfTrack(); end.setTimeStamp (gd.lengthPpq * ppq); seq.addEvent (end);
    seq.updateMatchedPairs();
    MidiFile mf; mf.setTicksPerQuarterNote (480); mf.addTrack (seq);
    {
        MemoryOutputStream os (out, false);
        mf.writeTo (os);
    }   // lo stream chiuso riporta il blocco alla dimensione esatta
    return out;
}

AudioProcessorEditor* MeteoraDrumProcessor::createEditor() { return new MeteoraDrumEditor (*this); }

AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MeteoraDrumProcessor(); }

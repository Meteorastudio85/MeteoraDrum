#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "DrumEngine.h"
#include "GrooveIndex.h"
#include <map>

// Canali con una striscia nel mixer (i crash si sentono solo da overheads e room)
constexpr int kNumInstrumentStrips = 6;
constexpr int kStripPieces[kNumInstrumentStrips] = { md::Kick, md::Snare, md::Tom, md::Floor, md::HiHat, md::Ride };

class MeteoraDrumProcessor : public juce::AudioProcessor
{
public:
    MeteoraDrumProcessor();
    ~MeteoraDrumProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Meteora Drum"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 6.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    static juce::String getPresetDescription (int index);

    // ---- groove ---------------------------------------------------------------
    struct GrooveEvent { double gridPpq; float offsetMs; int note; int velocity; float meanVel; };
    struct GrooveData  { std::vector<GrooveEvent> events; double lengthPpq = 8.0; };
    std::vector<GrooveData> grooves;                 // letti una volta all'avvio, poi solo in lettura
    void playGroove (int index) { grooveRequest.store (index); }
    void stopGroove()           { grooveRequest.store (-2); }
    std::atomic<bool>  grooveLoop { true }, grooveSync { true };
    std::atomic<float> grooveBpm { 120.f }, groovePos { -1.f }, hostBpm { 0.f };
    std::atomic<int>   grooveCurrent { -1 };
    std::atomic<float> grooveHumanize { 1.f };    // 0 = a griglia, 1 = come suonato, 1.5 = più sciolto
    juce::MemoryBlock buildGrooveMidi (int index) const;
    static juce::String pieceId (int piece);
    static bool hasOHSend (int piece)  { return piece == md::Kick || piece == md::Snare || piece == md::Tom || piece == md::Floor; }
    static bool hasBleed (int piece)   { return piece == md::Kick || piece == md::Snare; }

    juce::AudioProcessorValueTreeState apvts;
    md::Engine engine;
    juce::MidiKeyboardState keyboardState;
    juce::AudioProcessLoadMeasurer loadMeasurer;
    int numSoundsLoaded = 0;

private:
    void loadSamples();
    void loadGrooves();
    void runGroovePlayer (juce::MidiBuffer& midi, int numSamples);

    std::atomic<int> grooveRequest { -1 };
    int playIndex = -1;
    bool needsStart = false, wasHostPlaying = false;
    double localPpq = 0.0, startPpq = 0.0;
    void updateEngineParams();

    struct ChannelPtrs
    {
        std::atomic<float>* level = nullptr; std::atomic<float>* pan = nullptr;   std::atomic<float>* tune = nullptr;
        std::atomic<float>* decay = nullptr; std::atomic<float>* comp = nullptr;  std::atomic<float>* oh = nullptr;
        std::atomic<float>* room = nullptr;  std::atomic<float>* bleed = nullptr; std::atomic<float>* mute = nullptr;
        std::atomic<float>* solo = nullptr;
    };
    ChannelPtrs chp[md::NumPieces];
    std::atomic<float>* ohP[6] {};     // level, mute, solo, width, locut, air
    std::atomic<float>* roomP[7] {};   // level, mute, solo, size, decay, comp, locut
    std::atomic<float>* masterP[7] {}; // level, mute, eq, bass, treble, width, limit
    std::atomic<float>* setP[md::NumSetGroups] {};
    std::atomic<float>* ohSpacingP = nullptr; std::atomic<float>* ohRibbonP = nullptr;
    std::atomic<float>* hhMicXP = nullptr;    std::atomic<float>* hhMicHP = nullptr;
    std::atomic<float>* roomTypeP = nullptr; std::atomic<float>* phaseAlignP = nullptr;
    std::atomic<float>* velCurveP = nullptr;
    std::atomic<float>* hhCC4P = nullptr;

    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeteoraDrumProcessor)
};

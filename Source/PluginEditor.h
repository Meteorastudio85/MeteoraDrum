#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

// -----------------------------------------------------------------------------
class MeteoraLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MeteoraLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float sliderPos,
                           float minPos, float maxPos, juce::Slider::SliderStyle, juce::Slider&) override;
    int getSliderThumbRadius (juce::Slider&) override;

    void drawComboBox (juce::Graphics&, int w, int h, bool isDown, int bx, int by, int bw, int bh, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    juce::Font getPopupMenuFont() override;
    void drawPopupMenuBackground (juce::Graphics&, int width, int height) override;

    static juce::Font font (float size, bool bold = true);

    juce::Image faderCap;
};

// -----------------------------------------------------------------------------
//  Elementi grafici
// -----------------------------------------------------------------------------
class SpriteButton : public juce::Button
{
public:
    SpriteButton (const juce::String& name, juce::Image onImage, juce::Image offImage);
    void paintButton (juce::Graphics&, bool isOver, bool isDown) override;

private:
    juce::Image on, off;
};

class HotspotButton : public juce::Button
{
public:
    HotspotButton() : juce::Button ({}) {}
    void paintButton (juce::Graphics&, bool isOver, bool isDown) override;
};

class TabButton : public juce::Button
{
public:
    explicit TabButton (const juce::String& text) : juce::Button (text) {}
    void paintButton (juce::Graphics&, bool isOver, bool isDown) override;
    bool active = false;
};

// Selettore a segmenti collegato a un parametro (A/B, condensatore/nastro)
class SegmentedSwitch : public juce::Component, public juce::SettableTooltipClient
{
public:
    SegmentedSwitch (juce::RangedAudioParameter& param, juce::StringArray labels);
    float fontSize = 11.f;
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    juce::RangedAudioParameter& param;
    juce::StringArray labels;
    juce::ParameterAttachment attachment;
    int selected = 0;
};

// Ago di un VU (la faccia è nella grafica)
class VUNeedle : public juce::Component
{
public:
    void setPivot (juce::Point<float> p) { pivot = p; }
    void push (float rms);
    void paint (juce::Graphics&) override;

private:
    juce::Point<float> pivot;
    float needle = 0.f;
};

// -----------------------------------------------------------------------------
//  Schermata KIT
// -----------------------------------------------------------------------------
class KitPage : public juce::Component
{
public:
    explicit KitPage (MeteoraDrumProcessor&);
    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;
    void tick();

private:
    struct Zone { int piece; int note; float cx, cy, rx, ry; };
    int zoneAt (juce::Point<float>) const;
    juce::Point<float> micTip() const;
    bool nearMic (juce::Point<float>) const;
    void moveMic (juce::Point<float>);

    MeteoraDrumProcessor& proc;
    juce::Image kitLdc, kitRibbon, micSprite, micShadow;
    std::vector<Zone> zones;
    float flash[md::NumPieces] {};
    int lastHits[md::NumPieces] {};
    int heldNote = -1, hover = -1;
    bool draggingMic = false, micHover = false;
    float ribbonMix = 0.f;
    juce::RangedAudioParameter* micX = nullptr;
    juce::RangedAudioParameter* micH = nullptr;

    juce::Slider spacing;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> spacingAtt;
    std::unique_ptr<SegmentedSwitch> micType;
    std::vector<std::unique_ptr<SegmentedSwitch>> sets;
};

// -----------------------------------------------------------------------------
//  Schermata MIXER
// -----------------------------------------------------------------------------
class MixerPage : public juce::Component
{
public:
    explicit MixerPage (MeteoraDrumProcessor&);
    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override {}
    void mouseDown (const juce::MouseEvent&) override;
    void tick();

private:
    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;

    struct KnobUI   { juce::Slider slider; std::unique_ptr<SliderAtt> att; };
    struct StripUI
    {
        int meterIndex = 0, piece = -1, lastHits = 0;
        float led = 0.f;
        VUNeedle vu;
        juce::Slider fader;
        std::unique_ptr<SliderAtt> faderAtt;
        std::unique_ptr<SpriteButton> muteBtn, soloBtn;
        std::unique_ptr<ButtonAtt> muteAtt, soloAtt;
    };

    MeteoraDrumProcessor& proc;
    juce::Image background;
    std::unique_ptr<SegmentedSwitch> roomType;
    std::vector<std::unique_ptr<KnobUI>> knobs;
    std::vector<std::unique_ptr<StripUI>> strips;
};

// -----------------------------------------------------------------------------
//  Schermata GROOVES
// -----------------------------------------------------------------------------
class GroovesPage : public juce::Component
{
public:
    explicit GroovesPage (MeteoraDrumProcessor&);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    void tick();

private:
    void rebuildList();
    void select (int grooveIndex);
    void dragGroove (int grooveIndex);
    int  rowAt (juce::Point<float>) const;
    float displayBpm() const;

    MeteoraDrumProcessor& proc;
    juce::Image background;
    juce::Slider humanize;
    juce::String style { "ROCK" }, section { "STROFA" };
    std::vector<int> list;               // indici dei groove filtrati
    int scroll = 0, selected = 0;
    int pressedRow = -1, dragSource = -1;
    bool dragStarted = false, draggingBpm = false, draggingScroll = false;
    float bpmAtPress = 120.f;
    float lastPos = -1.f; int lastCurrent = -1;
};

// Selettore dei kit preset (menu a tendina disegnato)
class PresetSelector : public juce::Component, public juce::SettableTooltipClient
{
public:
    explicit PresetSelector (MeteoraDrumProcessor& p) : proc (p) { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void refresh() { repaint(); }

private:
    MeteoraDrumProcessor& proc;
};

// -----------------------------------------------------------------------------
class MainContent : public juce::Component
{
public:
    static constexpr int W = 1184, H = 972, Header = 60;

    explicit MainContent (MeteoraDrumProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void tick();
    void showOptions();

private:
    void showPage (int page);   // 0 kit, 1 mixer, 2 grooves
    void setParam (const juce::String& id, float value);

    MeteoraDrumProcessor& proc;
    juce::Image header;
    KitPage kit;
    MixerPage mixer;
    GroovesPage grooves;
    TabButton kitTab { "KIT" }, mixerTab { "MIXER" }, groovesTab { "GROOVES" };
    PresetSelector presetSel;
    HotspotButton prevBtn, nextBtn, logoBtn;
    int lastProgram = -1;
};

// -----------------------------------------------------------------------------
class MeteoraDrumEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit MeteoraDrumEditor (MeteoraDrumProcessor&);
    ~MeteoraDrumEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override { content.tick(); }

    MeteoraLookAndFeel lnf;
    MainContent content;
    juce::TooltipWindow tooltips { &content, 700 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeteoraDrumEditor)
};

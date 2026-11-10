#include "PluginEditor.h"
#include "BinaryData.h"

using namespace juce;

// =============================================================================
//  Geometria delle due schermate (coordinate delle immagini)
// =============================================================================
namespace Geo
{
// --- schermata KIT (1184 x 912) ---
constexpr float micZoneX0 = 116.f, micZoneX1 = 214.f;    // dal bordo alla campana
constexpr float micZoneY0 = 282.f, micZoneY1 = 326.f;    // da 15 cm a 3 cm dal piatto
constexpr float micPoleX = 54.f, micPoleY = 268.f;       // asta da cui parte la giraffa
constexpr float micTipAnchorX = 98.93f, micTipAnchorY = 107.03f;   // punta della capsula nello sprite
constexpr float micDX = 37.86f, micDY = 54.06f;          // dalla punta al supporto (66 px a 55°)
constexpr int panelY = 818;

// --- schermata MIXER (1184 x 912) ---
struct KnobPos { const char* id; float x, y, r; bool darkPointer; };
constexpr KnobPos knobs[] =
{
    { "kick_tune", 103.2f, 176.7f, 12.5f, false },
    { "kick_decay", 103.2f, 245.4f, 16.4f, false },
    { "kick_comp", 103.5f, 318.0f, 15.4f, false },
    { "kick_pan", 78.7f, 394.9f, 12.5f, false },
    { "kick_bleed", 132.1f, 395.3f, 13.4f, false },
    { "kick_oh", 77.5f, 468.0f, 12.5f, true },
    { "kick_room", 131.0f, 468.0f, 12.6f, false },
    { "snare_tune", 219.4f, 175.8f, 13.8f, false },
    { "snare_decay", 219.5f, 246.1f, 16.2f, false },
    { "snare_comp", 218.9f, 317.9f, 15.6f, false },
    { "snare_pan", 193.2f, 395.0f, 13.0f, false },
    { "snare_bleed", 245.3f, 394.6f, 13.8f, false },
    { "snare_oh", 193.0f, 468.0f, 12.5f, true },
    { "snare_room", 246.7f, 468.0f, 12.1f, false },
    { "tom_tune", 336.6f, 174.9f, 14.4f, false },
    { "tom_decay", 334.8f, 246.2f, 16.5f, false },
    { "tom_comp", 334.9f, 318.2f, 15.2f, false },
    { "tom_pan", 335.9f, 391.0f, 15.1f, false },
    { "tom_oh", 310.0f, 468.0f, 12.5f, true },
    { "tom_room", 363.6f, 468.3f, 12.6f, false },
    { "floor_tune", 451.5f, 173.7f, 14.2f, false },
    { "floor_decay", 450.2f, 246.5f, 16.0f, false },
    { "floor_comp", 450.6f, 318.4f, 15.2f, false },
    { "floor_pan", 451.3f, 391.2f, 14.9f, false },
    { "floor_oh", 424.5f, 468.0f, 12.5f, true },
    { "floor_room", 478.0f, 468.1f, 12.5f, false },
    { "hihat_tune", 568.1f, 173.5f, 14.2f, false },
    { "hihat_decay", 567.2f, 246.5f, 15.9f, false },
    { "hihat_comp", 567.1f, 318.4f, 15.0f, false },
    { "hihat_pan", 568.7f, 390.8f, 14.9f, false },
    { "hihat_room", 567.8f, 463.6f, 14.6f, false },
    { "ride_tune", 687.2f, 173.3f, 13.9f, false },
    { "ride_decay", 686.5f, 246.6f, 15.6f, false },
    { "ride_comp", 686.0f, 318.3f, 14.8f, false },
    { "ride_pan", 686.2f, 390.6f, 14.5f, false },
    { "ride_room", 686.1f, 463.4f, 14.9f, false },
    { "oh_width", 816.4f, 173.3f, 13.5f, false },
    { "oh_locut", 815.1f, 247.0f, 15.6f, false },
    { "oh_air", 814.5f, 318.5f, 15.6f, false },
    { "room_size", 962.7f, 173.3f, 14.1f, false },
    { "room_decay", 961.8f, 246.7f, 15.2f, false },
    { "room_comp", 961.9f, 318.8f, 14.9f, false },
    { "room_locut", 960.3f, 391.8f, 14.6f, false },
    { "master_bass", 1082.9f, 173.8f, 15.6f, false },
    { "master_treble", 1082.9f, 246.5f, 15.0f, false },
    { "master_width", 1080.0f, 390.0f, 14.5f, true },
    { "master_limit", 1080.0f, 462.0f, 14.5f, false }
};
constexpr int numStrips = 9;
constexpr float faderX[numStrips]  = { 109.f, 223.f, 338.f, 452.f, 567.f, 684.f, 810.f, 951.5f, 1078.f };
constexpr float faderTop = 605.f, faderBottom = 840.5f;   // corsa del centro del tappo: 632.5 .. 813
constexpr float muteX[numStrips]   = { 90.5f, 204.5f, 319.f, 434.5f, 550.f, 667.f, 793.5f, 935.f, 1060.5f };
constexpr float soloX[numStrips]   = { 90.5f, 204.5f, 318.5f, 434.5f, 550.f, 667.5f, 793.5f, 934.5f, 1058.f };
constexpr float muteY = 510.5f, soloY = 554.f;
constexpr float pivotX[numStrips]  = { 109.f, 223.5f, 337.5f, 453.f, 567.3f, 686.5f, 814.8f, 959.5f, 1078.3f };
constexpr float pivotY[numStrips]  = { 75.f, 75.f, 75.f, 75.f, 74.5f, 75.f, 74.5f, 75.f, 74.5f };
constexpr float ledX[6] = { 108.8f, 220.7f, 336.9f, 451.8f, 568.2f, 684.8f };
constexpr float ledY[6] = { 128.9f, 130.3f, 130.f, 130.5f, 130.f, 130.1f };
}

namespace Col
{
const Colour cream  { 0xfff1e8d2 };
const Colour amber  { 0xffeb9637 };
const Colour amberText { 0xffffbe6e };
const Colour panelBg { 0xd8120e0b };
const Colour panelLine { 0xdc966e3c };
const Colour label  { 0xffe1d7c3 };
const Colour soft   { 0xffc8bea9 };
}

static Image loadImage (const char* name)
{
    int size = 0;
    if (const char* data = BinaryData::getNamedResource (name, size))
        return ImageCache::getFromMemory (data, size);
    return {};
}

// =============================================================================
//  Look and feel
// =============================================================================
MeteoraLookAndFeel::MeteoraLookAndFeel()
{
    faderCap = loadImage ("mx_fader_cap_png");
    setColour (PopupMenu::backgroundColourId, Colour (0xff232323));
    setColour (PopupMenu::textColourId, Col::cream);
    setColour (PopupMenu::headerTextColourId, Col::amberText);
    setColour (PopupMenu::highlightedBackgroundColourId, Colour (0xff5a3d22));
    setColour (PopupMenu::highlightedTextColourId, Colours::white);
    setColour (ComboBox::textColourId, Col::cream);
    setColour (BubbleComponent::backgroundColourId, Colour (0xf0202020));
    setColour (BubbleComponent::outlineColourId, Colour (0xff6b5a44));
    setColour (TooltipWindow::backgroundColourId, Colour (0xf0202020));
    setColour (TooltipWindow::textColourId, Col::cream);
    setColour (TooltipWindow::outlineColourId, Colour (0xff6b5a44));
    setColour (AlertWindow::backgroundColourId, Colour (0xff232323));
    setColour (AlertWindow::textColourId, Col::cream);
    setColour (TextButton::buttonColourId, Colour (0xff3a3a3a));
    setColour (TextButton::textColourOffId, Col::cream);
}

Font MeteoraLookAndFeel::font (float size, bool bold)
{
    return Font (FontOptions (size, bold ? Font::bold : Font::plain));
}

int MeteoraLookAndFeel::getSliderThumbRadius (Slider& s)
{
    return s.getProperties().contains ("mixerFader") ? 27 : 9;
}

void MeteoraLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos,
                                           float startAngle, float endAngle, Slider& s)
{
    // il cappuccio è nella grafica: qui si disegna solo l'indice
    const auto c = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).getCentre();
    const float r = (float) s.getProperties().getWithDefault ("capR", 12.f);
    const bool dark = (bool) s.getProperties().getWithDefault ("darkPointer", false);
    const float ang = startAngle + pos * (endAngle - startAngle);
    const auto p1 = c.getPointOnCircumference (r * 0.18f, ang), p2 = c.getPointOnCircumference (r * 0.90f, ang);
    const float wd = jmax (1.5f, r * 0.12f);
    g.setColour (Colours::black.withAlpha (dark ? 0.15f : 0.35f));
    g.drawLine (Line<float> (p1.translated (0.6f, 0.8f), p2.translated (0.6f, 0.8f)), wd);
    g.setColour (dark ? Colour (0xff2e2a26) : Colour (0xfff5f1e8));
    g.drawLine (Line<float> (p1, p2), wd);
}

void MeteoraLookAndFeel::drawLinearSlider (Graphics& g, int x, int y, int w, int h, float sliderPos,
                                           float, float, Slider::SliderStyle style, Slider& s)
{
    if (s.getProperties().contains ("mixerFader"))
    {
        if (! faderCap.isValid()) return;
        const float cx = (float) x + (float) w * 0.5f;
        const float cw = (float) faderCap.getWidth(), ch = (float) faderCap.getHeight();
        g.setColour (Colours::black.withAlpha (0.28f));
        g.fillRoundedRectangle (cx - cw * 0.5f + 2.f, sliderPos - ch * 0.5f + 4.f, cw, ch, 3.f);
        g.setOpacity (1.f);   // drawImage usa l'opacità dell'ultimo colore: senza questa riga il tappo era trasparente
        g.drawImage (faderCap, Rectangle<float> (cx - cw * 0.5f, sliderPos - ch * 0.5f, cw, ch));
        return;
    }
    if (style == Slider::LinearHorizontal)   // cursore della distanza degli overhead
    {
        const float cy = (float) y + (float) h * 0.5f;
        g.setColour (Colour (0xff080605));
        g.fillRoundedRectangle ((float) x, cy - 3.f, (float) w, 6.f, 3.f);
        g.setColour (Col::amber);
        g.fillRoundedRectangle ((float) x, cy - 3.f, sliderPos - (float) x, 6.f, 3.f);
        g.setColour (Colour (0xff968773));
        for (int i = 0; i <= 8; ++i)
        {
            const float xx = (float) x + (float) w * (float) i / 8.f;
            g.drawLine (xx, cy + 8.f, xx, cy + 12.f, 1.f);
        }
        Rectangle<float> th (sliderPos - 7.f, cy - 11.f, 14.f, 22.f);
        g.setColour (Colour (0xffe1e1dc));
        g.fillRoundedRectangle (th, 3.f);
        g.setColour (Colour (0xff141414));
        g.drawRoundedRectangle (th, 3.f, 1.f);
        g.setColour (Colour (0xff282828));
        g.drawLine (sliderPos, cy - 7.f, sliderPos, cy + 7.f, 2.f);
    }
}

void MeteoraLookAndFeel::drawComboBox (Graphics&, int, int, bool, int, int, int, int, ComboBox&) {}
Font MeteoraLookAndFeel::getComboBoxFont (ComboBox&) { return font (16.f, false); }
Font MeteoraLookAndFeel::getPopupMenuFont() { return font (15.f, false); }

void MeteoraLookAndFeel::drawPopupMenuBackground (Graphics& g, int width, int height)
{
    g.fillAll (Colour (0xff1e1b18));
    g.setColour (Colour (0xe6966e3c));
    g.drawRect (0, 0, width, height, 1);
}

void MeteoraLookAndFeel::positionComboBoxText (ComboBox& box, Label& label)
{
    label.setBounds (6, 0, box.getWidth() - 34, box.getHeight());
    label.setFont (getComboBoxFont (box));
    label.setJustificationType (Justification::centred);
}

// =============================================================================
//  Pulsanti e selettori
// =============================================================================
SpriteButton::SpriteButton (const String& name, Image onImage, Image offImage)
    : Button (name), on (std::move (onImage)), off (std::move (offImage))
{
    setClickingTogglesState (true);
}

void SpriteButton::paintButton (Graphics& g, bool isOver, bool isDown)
{
    const auto& img = getToggleState() ? on : off;
    if (img.isValid()) g.drawImage (img, getLocalBounds().toFloat());
    if (isDown)      { g.setColour (Colours::black.withAlpha (0.2f));  g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (3.f), 3.f); }
    else if (isOver) { g.setColour (Colours::white.withAlpha (0.07f)); g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (3.f), 3.f); }
}

void HotspotButton::paintButton (Graphics& g, bool isOver, bool isDown)
{
    if (isDown)      { g.setColour (Colours::black.withAlpha (0.25f)); g.fillRoundedRectangle (getLocalBounds().toFloat(), 4.f); }
    else if (isOver) { g.setColour (Colours::white.withAlpha (0.08f)); g.fillRoundedRectangle (getLocalBounds().toFloat(), 4.f); }
}

void TabButton::paintButton (Graphics& g, bool isOver, bool isDown)
{
    auto r = getLocalBounds().toFloat().reduced (1.f);
    ColourGradient cg = active ? ColourGradient (Colour (0xffffaa3c), 0, r.getY(), Colour (0xffc36e1e), 0, r.getBottom(), false)
                               : ColourGradient (Colour (0xff464646), 0, r.getY(), Colour (0xff282828), 0, r.getBottom(), false);
    g.setGradientFill (cg);
    g.fillRoundedRectangle (r, 4.f);
    if (isOver && ! active) { g.setColour (Colours::white.withAlpha (0.08f)); g.fillRoundedRectangle (r, 4.f); }
    if (isDown) { g.setColour (Colours::black.withAlpha (0.2f)); g.fillRoundedRectangle (r, 4.f); }
    g.setColour (Colour (0xff141414));
    g.drawRoundedRectangle (r, 4.f, 2.f);
    g.setColour (active ? Colour (0xff28160a) : Colour (0xffe1d7c3));
    g.setFont (MeteoraLookAndFeel::font (15.f, true));
    g.drawText (getButtonText(), getLocalBounds(), Justification::centred);
}

SegmentedSwitch::SegmentedSwitch (RangedAudioParameter& p, StringArray l)
    : param (p), labels (std::move (l)),
      attachment (p, [this] (float v) { selected = roundToInt (v); repaint(); }, nullptr)
{
    attachment.sendInitialUpdate();
    setMouseCursor (MouseCursor::PointingHandCursor);
}

void SegmentedSwitch::paint (Graphics& g)
{
    const int n = labels.size();
    const float sw = (float) getWidth() / (float) n;
    for (int i = 0; i < n; ++i)
    {
        Rectangle<float> r ((float) i * sw + 1.f, 0.f, sw - 2.f, (float) getHeight());
        const bool a = (i == selected);
        g.setColour (a ? Col::amber : Colour (0xff342e28));
        g.fillRoundedRectangle (r, 4.f);
        g.setColour (Colour (0xff0a0806));
        g.drawRoundedRectangle (r, 4.f, 1.f);
        g.setColour (a ? Colour (0xff28160a) : Colour (0xffdcd2be));
        g.setFont (MeteoraLookAndFeel::font (fontSize, true));
        g.drawText (labels[i], r, Justification::centred);
    }
}

void SegmentedSwitch::mouseDown (const MouseEvent& e)
{
    const int n = labels.size();
    const int i = jlimit (0, n - 1, (int) (e.position.x / ((float) getWidth() / (float) n)));
    attachment.setValueAsCompleteGesture ((float) i);
}

// =============================================================================
//  VU
// =============================================================================
void VUNeedle::push (float rms)
{
    const float vuDb = Decibels::gainToDecibels (rms, -80.f) + 18.f;     // 0 VU = -18 dBFS RMS
    const float lin = std::pow (10.f, vuDb / 20.f);
    const float lo = 0.1f, hi = std::pow (10.f, 3.f / 20.f);
    const float target = jlimit (0.f, 1.04f, (lin - lo) / (hi - lo));
    const float k = target > needle ? 0.30f : 0.11f;
    const float next = needle + (target - needle) * k;
    if (std::abs (next - needle) > 0.0005f) { needle = next; repaint(); }
}

void VUNeedle::paint (Graphics& g)
{
    const float ang = degreesToRadians (-54.f + 104.f * needle);
    const auto p1 = pivot.getPointOnCircumference (10.f, ang), p2 = pivot.getPointOnCircumference (34.f, ang);
    g.setColour (Colours::black.withAlpha (0.18f));
    g.drawLine (Line<float> (p1.translated (1.f, 1.f), p2.translated (1.f, 1.f)), 1.2f);
    g.setColour (Colour (0xff1e1914));
    g.drawLine (Line<float> (p1, p2), 1.3f);
}

// =============================================================================
//  Schermata KIT
// =============================================================================
KitPage::KitPage (MeteoraDrumProcessor& p) : proc (p)
{
    kitLdc    = loadImage ("kit_ldc_jpg");
    kitRibbon = loadImage ("kit_ribbon_jpg");
    micSprite = loadImage ("hh_mic_png");
    if (micSprite.isValid())
    {
        micShadow = micSprite.createCopy();
        Image::BitmapData bd (micShadow, Image::BitmapData::readWrite);
        for (int yy = 0; yy < micShadow.getHeight(); ++yy)
            for (int xx = 0; xx < micShadow.getWidth(); ++xx)
            {
                const auto c = bd.getPixelColour (xx, yy);
                bd.setPixelColour (xx, yy, Colours::black.withAlpha (c.getFloatAlpha() * 0.32f));
            }
    }

    zones = {
        { md::HiHat,  42, 222.f, 340.f, 112.f, 22.f },
        { md::CrashL, 49, 333.f, 213.f, 123.f, 45.f },
        { md::CrashR, 57, 760.f, 327.f, 134.f, 60.f },
        { md::Ride,   51, 961.f, 232.f, 154.f, 72.f },
        { md::Tom,    48, 509.f, 401.f,  82.f, 88.f },
        { md::Snare,  38, 433.f, 518.f, 111.f, 66.f },
        { md::Floor,  43, 877.f, 586.f, 107.f, 118.f },
        { md::Kick,   36, 621.f, 657.f, 164.f, 157.f },
    };

    micX = p.apvts.getParameter ("hh_mic_x");
    micH = p.apvts.getParameter ("hh_mic_h");

    spacing.setSliderStyle (Slider::LinearHorizontal);
    spacing.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
    spacing.setPopupDisplayEnabled (true, true, this);
    spacing.setDoubleClickReturnValue (true, 0.5);
    spacing.setTooltip ("Distanza tra i due overhead");
    addAndMakeVisible (spacing);
    spacingAtt = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "oh_spacing", spacing);
    spacing.onValueChange = [this] { repaint (0, Geo::panelY, 600, 94); };

    micType = std::make_unique<SegmentedSwitch> (*p.apvts.getParameter ("oh_ribbon"), StringArray { "CONDENSATORE", "NASTRO" });
    micType->setTooltip ("Tipo di microfoni overhead");
    addAndMakeVisible (*micType);

    const char* ids[] = { "set_kick", "set_snare", "set_tom", "set_floor", "set_crash1", "set_crash2", "set_hihat" };
    for (auto* id : ids)
    {
        sets.push_back (std::make_unique<SegmentedSwitch> (*p.apvts.getParameter (id), StringArray { "A", "B" }));
        addAndMakeVisible (*sets.back());
    }
    ribbonMix = p.apvts.getRawParameterValue ("oh_ribbon")->load() > 0.5f ? 1.f : 0.f;
}

void KitPage::resized()
{
    const int py0 = Geo::panelY;
    spacing.setBounds (79, py0 + 30, 202, 24);
    micType->setBounds (360, py0 + 30, 210, 24);
    for (int i = 0; i < (int) sets.size(); ++i)
    {
        const int r = i < 4 ? 0 : 1, c = i < 4 ? i : i - 4;
        sets[(size_t) i]->setBounds (608 + c * 140 + 60, py0 + 32 + r * 28, 66, 20);
    }
}

Point<float> KitPage::micTip() const
{
    const float x = micX != nullptr ? micX->convertFrom0to1 (micX->getValue()) : 0.5f;
    const float h = micH != nullptr ? micH->convertFrom0to1 (micH->getValue()) : 0.25f;
    return { Geo::micZoneX0 + (Geo::micZoneX1 - Geo::micZoneX0) * x, Geo::micZoneY1 - (Geo::micZoneY1 - Geo::micZoneY0) * h };
}

bool KitPage::nearMic (Point<float> p) const
{
    const auto tip = micTip();
    const Point<float> mid (tip.x - Geo::micDX * 0.5f, tip.y - Geo::micDY * 0.5f);
    return p.getDistanceFrom (tip) < 16.f || p.getDistanceFrom (mid) < 16.f
        || Rectangle<float> (Geo::micZoneX0 - 4.f, Geo::micZoneY0 - 4.f, Geo::micZoneX1 - Geo::micZoneX0 + 8.f, Geo::micZoneY1 - Geo::micZoneY0 + 8.f).contains (p);
}

void KitPage::moveMic (Point<float> p)
{
    const float x = jlimit (0.f, 1.f, (p.x - Geo::micZoneX0) / (Geo::micZoneX1 - Geo::micZoneX0));
    const float h = jlimit (0.f, 1.f, (Geo::micZoneY1 - p.y) / (Geo::micZoneY1 - Geo::micZoneY0));
    if (micX != nullptr) micX->setValueNotifyingHost (micX->convertTo0to1 (x));
    if (micH != nullptr) micH->setValueNotifyingHost (micH->convertTo0to1 (h));
    repaint();
}

int KitPage::zoneAt (Point<float> pt) const
{
    for (int i = 0; i < (int) zones.size(); ++i)
    {
        const auto& z = zones[(size_t) i];
        const float dx = (pt.x - z.cx) / z.rx, dy = (pt.y - z.cy) / z.ry;
        if (dx * dx + dy * dy <= 1.f) return i;
    }
    return -1;
}

void KitPage::paint (Graphics& g)
{
    // foto: condensatore e nastro in dissolvenza
    if (kitLdc.isValid() && ribbonMix < 0.999f) g.drawImageAt (kitLdc, 0, 0);
    if (kitRibbon.isValid() && ribbonMix > 0.001f)
    {
        g.setOpacity (ribbonMix);
        g.drawImageAt (kitRibbon, 0, 0);
        g.setOpacity (1.f);
    }

    // bagliori dei colpi
    for (auto& z : zones)
    {
        const float f = flash[z.piece];
        if (f <= 0.01f) continue;
        Rectangle<float> r (z.cx - z.rx - 6.f, z.cy - z.ry - 6.f, 2 * z.rx + 12.f, 2 * z.ry + 12.f);
        ColourGradient glow (Colour (0xfffff2d0).withAlpha (0.45f * f), r.getCentreX(), r.getCentreY(),
                             Colour (0xfffff2d0).withAlpha (0.f), r.getRight(), r.getCentreY(), true);
        g.setGradientFill (glow);
        g.fillEllipse (r);
    }
    if (hover >= 0 && ! micHover)
    {
        const auto& z = zones[(size_t) hover];
        g.setColour (Col::cream.withAlpha (0.45f));
        g.drawEllipse (z.cx - z.rx, z.cy - z.ry, 2 * z.rx, 2 * z.ry, 1.5f);
    }

    // microfono dell'hi-hat (asta a giraffa, cavo, microfono)
    const auto tip = micTip();
    const Point<float> tail (tip.x - Geo::micDX, tip.y - Geo::micDY);
    const Point<float> clip (tail.x - 4.f, tail.y - 3.f);
    Path cable;
    cable.startNewSubPath (tail.translated (-2.f, -2.f));
    cable.cubicTo (tail.translated (-14.f, 6.f), tail.translated (-20.f, 30.f), tail.translated (-24.f, 70.f));
    g.setColour (Colour (0xff121110));
    g.strokePath (cable, PathStrokeType (2.f));
    g.setColour (Colours::black.withAlpha (0.25f));
    g.drawLine (Geo::micPoleX + 3.f, Geo::micPoleY + 5.f, clip.x + 3.f, clip.y + 5.f, 4.f);
    g.setColour (Colour (0xff2e2c2a));
    g.drawLine (Geo::micPoleX, Geo::micPoleY, clip.x, clip.y, 4.f);
    g.setColour (Colour (0xff78736c));
    g.drawLine (Geo::micPoleX, Geo::micPoleY - 1.f, clip.x, clip.y - 1.f, 1.f);
    g.setColour (Colour (0xff282624));
    g.fillRoundedRectangle (Geo::micPoleX - 6.f, Geo::micPoleY - 6.f, 12.f, 12.f, 2.f);
    g.fillEllipse (clip.x - 5.f, clip.y - 5.f, 10.f, 10.f);
    if (micShadow.isValid())
        g.drawImageAt (micShadow, roundToInt (tip.x - Geo::micTipAnchorX) + 3, roundToInt (tip.y - Geo::micTipAnchorY) + 5);
    if (micSprite.isValid())
        g.drawImageAt (micSprite, roundToInt (tip.x - Geo::micTipAnchorX), roundToInt (tip.y - Geo::micTipAnchorY));

    // zona di spostamento del microfono
    {
        const float a = (micHover || draggingMic) ? 0.95f : 0.55f;
        Path zone; zone.addRectangle (Geo::micZoneX0, Geo::micZoneY0, Geo::micZoneX1 - Geo::micZoneX0, Geo::micZoneY1 - Geo::micZoneY0);
        Path dashed; const float dash[] = { 6.f, 6.f };
        PathStrokeType (2.f).createDashedStroke (dashed, zone, dash, 2);
        g.setColour (Col::amber.withAlpha (a));
        g.fillPath (dashed);
        Path arrows;
        arrows.addTriangle (110.f, 304.f, 117.f, 299.f, 117.f, 309.f);
        arrows.addTriangle (220.f, 304.f, 213.f, 299.f, 213.f, 309.f);
        g.fillPath (arrows);
    }
    // etichetta del microfono
    {
        Rectangle<float> r (90.f, 374.f, 160.f, 40.f);
        g.setColour (Colour (0xc8140f0b)); g.fillRoundedRectangle (r, 6.f);
        g.setColour (Col::amber.withAlpha (0.7f)); g.drawRoundedRectangle (r, 6.f, 1.f);
        g.setColour (Col::amberText); g.setFont (MeteoraLookAndFeel::font (11.f, true));
        g.drawText (String (CharPointer_UTF8 ("MIC HI-HAT  \xc2\xb7  trascina")), r.withHeight (22.f).translated (0.f, 2.f), Justification::centred);
        const String where = micX != nullptr ? micX->getCurrentValueAsText() : String();
        const String dist  = micH != nullptr ? micH->getCurrentValueAsText() : String();
        g.setColour (Colour (0xffd7cdb9)); g.setFont (MeteoraLookAndFeel::font (10.f, true));
        g.drawText (where + String (CharPointer_UTF8 (" \xc2\xb7 ")) + dist, r.withTrimmedTop (20.f), Justification::centred);
    }
    // etichette degli overhead
    struct OhLabel { float x, y; const char* t; };
    const OhLabel ohLabels[] = { { 322.f, 112.f, "OH L" }, { 898.f, 114.f, "OH R" } };
    for (const auto& lb : ohLabels)
    {
        const float x = lb.x, y = lb.y; const char* t = lb.t;
        Rectangle<float> r (x - 22.f, y - 10.f, 44.f, 20.f);
        g.setColour (Colour (0xbe140f0b)); g.fillRoundedRectangle (r, 5.f);
        g.setColour (Col::amber.withAlpha (0.67f)); g.drawRoundedRectangle (r, 5.f, 1.f);
        g.setColour (Col::amberText); g.setFont (MeteoraLookAndFeel::font (12.f, true));
        g.drawText (t, r, Justification::centred);
    }

    // pannello inferiore
    const float py0 = (float) Geo::panelY;
    Rectangle<float> panel (10.f, py0, (float) getWidth() - 20.f, (float) getHeight() - py0 - 8.f);
    g.setColour (Col::panelBg);   g.fillRoundedRectangle (panel, 10.f);
    g.setColour (Col::panelLine); g.drawRoundedRectangle (panel, 10.f, 1.f);
    g.drawLine (592.f, py0 + 12.f, 592.f, panel.getBottom() - 12.f, 1.f);

    auto title = [&] (float x, const String& t) { g.setColour (Col::amberText); g.setFont (MeteoraLookAndFeel::font (13.f, true)); g.drawText (t, Rectangle<float> (x, py0 + 8.f, 200.f, 16.f), Justification::centredLeft); };
    auto small = [&] (Rectangle<float> r, const String& t, Justification j, Colour c) { g.setColour (c); g.setFont (MeteoraLookAndFeel::font (11.f, true)); g.drawText (t, r, j); };

    title (30.f, "OVERHEADS");
    small ({ 30.f, py0 + 34.f, 50.f, 16.f }, "STRETTI", Justification::centredLeft, Col::label);
    small ({ 280.f, py0 + 34.f, 50.f, 16.f }, "LARGHI", Justification::centredRight, Col::label);
    small ({ 80.f, py0 + 60.f, 200.f, 16.f }, "distanza " + spacing.getTextFromValue (spacing.getValue()), Justification::centred, Col::soft);
    const bool rib = proc.apvts.getRawParameterValue ("oh_ribbon")->load() > 0.5f;
    small ({ 360.f, py0 + 60.f, 210.f, 16.f }, rib ? "nastro: caldo, figura 8" : "diaframma largo: presenza", Justification::centred, Col::soft);

    title (608.f, "SET SUONI");
    small ({ 690.f, py0 + 8.f, 300.f, 16.f }, String (CharPointer_UTF8 ("A = originale   \xc2\xb7   B = nuovo")), Justification::centredLeft, Col::soft);
    const char* names[] = { "KICK", "SNARE", "TOM", "FLOOR", "CRASH 1", "CRASH 2", "HI-HAT" };
    for (int i = 0; i < 7; ++i)
    {
        const int r = i < 4 ? 0 : 1, c = i < 4 ? i : i - 4;
        small ({ 608.f + (float) c * 140.f, py0 + 32.f + (float) r * 28.f, 60.f, 20.f }, names[i], Justification::centredLeft, Col::label);
    }
}

void KitPage::paintOverChildren (Graphics&) {}

void KitPage::mouseDown (const MouseEvent& e)
{
    if (e.mods.isPopupMenu()) { if (auto* mc = findParentComponentOfClass<MainContent>()) mc->showOptions(); return; }
    if (e.position.y >= (float) Geo::panelY) return;
    if (nearMic (e.position))
    {
        draggingMic = true;
        if (micX) micX->beginChangeGesture();
        if (micH) micH->beginChangeGesture();
        moveMic (e.position);
        return;
    }
    const int i = zoneAt (e.position);
    if (i < 0) return;
    const auto& z = zones[(size_t) i];
    int note = z.note;
    if (z.piece == md::HiHat && e.mods.isShiftDown()) note = 46;
    if (z.piece == md::Snare && e.mods.isAltDown())   note = 40;   // rimshot
    proc.keyboardState.noteOn (1, note, e.mods.isCommandDown() ? 0.35f : 0.85f);
    heldNote = note;
}

void KitPage::mouseDrag (const MouseEvent& e)
{
    if (draggingMic) moveMic (e.position);
}

void KitPage::mouseUp (const MouseEvent&)
{
    if (draggingMic)
    {
        draggingMic = false;
        if (micX) micX->endChangeGesture();
        if (micH) micH->endChangeGesture();
        repaint();
    }
    if (heldNote >= 0) proc.keyboardState.noteOff (1, heldNote, 0.f);
    heldNote = -1;
}

void KitPage::mouseMove (const MouseEvent& e)
{
    const bool m = e.position.y < (float) Geo::panelY && nearMic (e.position);
    const int i = (m || e.position.y >= (float) Geo::panelY) ? -1 : zoneAt (e.position);
    if (m != micHover || i != hover)
    {
        micHover = m; hover = i;
        setMouseCursor (m ? MouseCursor::DraggingHandCursor : (i >= 0 ? MouseCursor::PointingHandCursor : MouseCursor::NormalCursor));
        repaint();
    }
}

void KitPage::mouseExit (const MouseEvent&)
{
    if (hover != -1 || micHover) { hover = -1; micHover = false; repaint(); }
}

void KitPage::tick()
{
    bool dirty = false;
    for (int p = 0; p < md::NumPieces; ++p)
    {
        const int hits = proc.engine.hitCounter[p].load();
        if (hits != lastHits[p]) { lastHits[p] = hits; flash[p] = 1.f; dirty = true; }
        else if (flash[p] > 0.f) { flash[p] *= 0.78f; if (flash[p] < 0.01f) flash[p] = 0.f; dirty = true; }
    }
    const float target = proc.apvts.getRawParameterValue ("oh_ribbon")->load() > 0.5f ? 1.f : 0.f;
    if (std::abs (target - ribbonMix) > 0.001f)
    {
        ribbonMix += jlimit (-0.15f, 0.15f, target - ribbonMix);
        dirty = true;
    }
    if (dirty) repaint();
}

// =============================================================================
//  Schermata MIXER
// =============================================================================
MixerPage::MixerPage (MeteoraDrumProcessor& p) : proc (p)
{
    background = loadImage ("mixer_bg_jpg");

    for (const auto& kp : Geo::knobs)
    {
        auto k = std::make_unique<KnobUI>();
        auto& sl = k->slider;
        sl.setSliderStyle (Slider::RotaryVerticalDrag);
        sl.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        sl.setRotaryParameters (MathConstants<float>::pi * 1.25f, MathConstants<float>::pi * 2.75f, true);
        sl.setMouseDragSensitivity (160);
        sl.setPopupDisplayEnabled (true, true, getParentComponent() != nullptr ? getParentComponent() : this);
        sl.getProperties().set ("capR", kp.r);
        sl.getProperties().set ("darkPointer", kp.darkPointer);
        if (auto* prm = p.apvts.getParameter (kp.id))
        {
            sl.setDoubleClickReturnValue (true, prm->convertFrom0to1 (prm->getDefaultValue()));
            sl.setTooltip (prm->getName (64));
        }
        const float s = kp.r + 5.f;
        sl.setBounds (Rectangle<float> (kp.x - s, kp.y - s, 2 * s, 2 * s).getSmallestIntegerContainer());
        addAndMakeVisible (sl);
        k->att = std::make_unique<SliderAtt> (p.apvts, kp.id, sl);
        knobs.push_back (std::move (k));
    }

    // tipo di room: studio di registrazione o big room (nello spazio libero del canale ROOM)
    roomType = std::make_unique<SegmentedSwitch> (*p.apvts.getParameter ("room_type"), StringArray { "STUDIO", "BIG ROOM" });
    roomType->setTooltip ("Tipo di stanza: studio di registrazione o sala grande");
    roomType->setBounds (899, 452, 116, 24);
    roomType->fontSize = 9.5f;
    addAndMakeVisible (*roomType);

    const Image muteOn = loadImage ("mx_mute_on_png"), muteOff = loadImage ("mx_mute_off_png");
    const Image soloOn = loadImage ("mx_solo_on_png"), soloOff = loadImage ("mx_solo_off_png");
    const Image eqOn   = loadImage ("mx_eq_on_png"),   eqOff   = loadImage ("mx_eq_off_png");
    const int meterIdx[Geo::numStrips] = { md::Kick, md::Snare, md::Tom, md::Floor, md::HiHat, md::Ride, md::MeterOH, md::MeterRoom, md::MeterMaster };

    for (int s = 0; s < Geo::numStrips; ++s)
    {
        auto st = std::make_unique<StripUI>();
        st->meterIndex = meterIdx[s];
        st->piece = s < kNumInstrumentStrips ? kStripPieces[s] : -1;

        String faderId, muteId, soloId;
        if (s < kNumInstrumentStrips)
        {
            const String id = MeteoraDrumProcessor::pieceId (kStripPieces[s]);
            faderId = id + "_level"; muteId = id + "_mute"; soloId = id + "_solo";
        }
        else if (s == 6) { faderId = "oh_level";   muteId = "oh_mute";     soloId = "oh_solo"; }
        else if (s == 7) { faderId = "room_level"; muteId = "room_mute";   soloId = "room_solo"; }
        else             { faderId = "master";     muteId = "master_mute"; soloId = "master_eq"; }

        auto& f = st->fader;
        f.setSliderStyle (Slider::LinearVertical);
        f.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        f.getProperties().set ("mixerFader", true);
        f.setPopupDisplayEnabled (true, true, this);
        if (auto* prm = p.apvts.getParameter (faderId))
        {
            f.setDoubleClickReturnValue (true, prm->convertFrom0to1 (prm->getDefaultValue()));
            f.setTooltip (prm->getName (64));
        }
        f.setBounds (Rectangle<float> (Geo::faderX[s] - 17.f, Geo::faderTop, 34.f, Geo::faderBottom - Geo::faderTop).toNearestInt());
        addAndMakeVisible (f);
        st->faderAtt = std::make_unique<SliderAtt> (p.apvts, faderId, f);

        st->muteBtn = std::make_unique<SpriteButton> ("Mute", muteOn, muteOff);
        st->soloBtn = s == 8 ? std::make_unique<SpriteButton> ("EQ On", eqOn, eqOff)
                             : std::make_unique<SpriteButton> ("Solo", soloOn, soloOff);
        st->muteBtn->setTooltip ("Mute");
        st->soloBtn->setTooltip (s == 8 ? "EQ master (Bass / Treble)" : "Solo");
        st->muteBtn->setBounds (Rectangle<float> (Geo::muteX[s] - 2.5f, Geo::muteY, 41.f, 30.f).toNearestInt());
        st->soloBtn->setBounds (Rectangle<float> (Geo::soloX[s] - (s == 8 ? 0.f : 2.5f), Geo::soloY, 42.f, 30.f).toNearestInt());
        for (auto* b : { st->muteBtn.get(), st->soloBtn.get() })
        {
            addAndMakeVisible (*b);
            b->onStateChange = [this] { repaint(); };
        }
        st->muteAtt = std::make_unique<ButtonAtt> (p.apvts, muteId, *st->muteBtn);
        st->soloAtt = std::make_unique<ButtonAtt> (p.apvts, soloId, *st->soloBtn);

        const Rectangle<float> vuR (Geo::pivotX[s] - 42.f, Geo::pivotY[s] - 42.f, 84.f, 50.f);
        st->vu.setBounds (vuR.toNearestInt());
        st->vu.setPivot ({ Geo::pivotX[s] - (float) st->vu.getX(), Geo::pivotY[s] - (float) st->vu.getY() });
        st->vu.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (st->vu);
        strips.push_back (std::move (st));
    }
}

void MixerPage::paint (Graphics& g)
{
    if (background.isValid()) g.drawImageAt (background, 0, 0);
    else g.fillAll (Colour (0xff8c8882));

    // alone dei pulsanti accesi
    for (auto& st : strips)
        for (auto* b : { st->muteBtn.get(), st->soloBtn.get() })
            if (b->getToggleState())
            {
                const auto r = b->getBounds().toFloat().expanded (12.f, 10.f);
                const bool eq = b->getName() == "EQ On";
                const Colour c = eq ? Colour (0xff8cc8ff) : (b->getName() == "Mute" ? Colour (0xffff7a3c) : Colour (0xffffb43c));
                ColourGradient glow (c.withAlpha (0.35f), r.getCentreX(), r.getCentreY(), c.withAlpha (0.f), r.getRight(), r.getCentreY(), true);
                g.setGradientFill (glow);
                g.fillEllipse (r);
            }
}

void MixerPage::paintOverChildren (Graphics& g)
{
    g.setColour (Colour (0xff231e1a));
    g.setFont (MeteoraLookAndFeel::font (13.f, true));
    g.drawText ("TIPO", Rectangle<float> (901.f, 430.f, 110.f, 18.f), Justification::centredLeft);

    for (int s = 0; s < 6; ++s)
    {
        const float led = strips[(size_t) s]->led;
        const float x = Geo::ledX[s], y = Geo::ledY[s];
        if (led > 0.02f)
        {
            ColourGradient glow (Colour (0xffffa03c).withAlpha (0.55f * led), x, y, Colour (0xffffa03c).withAlpha (0.f), x + 14.f, y, true);
            g.setGradientFill (glow);
            g.fillEllipse (x - 14.f, y - 14.f, 28.f, 28.f);
        }
        g.setColour (Colour (0xff2a2622)); g.fillEllipse (x - 6.5f, y - 6.5f, 13.f, 13.f);
        g.setColour (Colour (0xff6e6860)); g.drawEllipse (x - 6.5f, y - 6.5f, 13.f, 13.f, 0.8f);
        const Colour lens = Colour (0xff3c1a10).interpolatedWith (Colour (0xffffb04a), led);
        ColourGradient lg (lens.brighter (0.4f), x - 1.5f, y - 1.5f, lens.darker (0.4f), x + 4.f, y + 4.f, true);
        g.setGradientFill (lg);
        g.fillEllipse (x - 4.2f, y - 4.2f, 8.4f, 8.4f);
    }
}

void MixerPage::mouseDown (const MouseEvent& e)
{
    if (e.mods.isPopupMenu()) if (auto* mc = findParentComponentOfClass<MainContent>()) mc->showOptions();
}

void MixerPage::tick()
{
    for (int s = 0; s < (int) strips.size(); ++s)
    {
        auto& st = *strips[(size_t) s];
        st.vu.push (proc.engine.vu[st.meterIndex].exchange (0.f));
        proc.engine.meter[st.meterIndex].exchange (0.f);
        if (s < 6)
        {
            const int hits = proc.engine.hitCounter[st.piece].load();
            float nl = st.led * 0.80f;
            if (hits != st.lastHits) { st.lastHits = hits; nl = 1.f; }
            if (nl < 0.01f) nl = 0.f;
            if (std::abs (nl - st.led) > 0.005f)
            {
                st.led = nl;
                repaint (Rectangle<float> (Geo::ledX[s] - 16.f, Geo::ledY[s] - 16.f, 32.f, 32.f).getSmallestIntegerContainer());
            }
        }
    }
}

// =============================================================================
//  Selettore dei kit preset
// =============================================================================
namespace
{
struct PresetHeaderItem : public PopupMenu::CustomComponent
{
    PresetHeaderItem() : PopupMenu::CustomComponent (false) {}
    void getIdealSize (int& w, int& h) override { w = 380; h = 26; }
    void paint (Graphics& g) override
    {
        g.setColour (Col::amberText);
        g.setFont (MeteoraLookAndFeel::font (11.f, true));
        g.drawText (String (CharPointer_UTF8 ("KIT PRESET  \xc2\xb7  suoni, microfoni e mix")), getLocalBounds().withTrimmedLeft (14), Justification::centredLeft);
    }
};

struct PresetItem : public PopupMenu::CustomComponent
{
    PresetItem (String n, String d, bool a) : name (std::move (n)), desc (std::move (d)), active (a) {}
    void getIdealSize (int& w, int& h) override { w = 380; h = 46; }
    void paint (Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (6.f, 2.f);
        if (isItemHighlighted() || active)
        {
            g.setColour (isItemHighlighted() ? Colour (0xff6a4826) : Colour (0xff5a3d22));
            g.fillRoundedRectangle (r, 6.f);
        }
        g.setColour (active ? Colour (0xfffff0dc) : Col::cream);
        g.setFont (MeteoraLookAndFeel::font (14.f, true));
        g.drawText (name, r.withTrimmedLeft (10.f).withHeight (24.f).translated (0.f, 2.f), Justification::centredLeft);
        g.setColour (active ? Colour (0xffd7c8b4) : Col::soft);
        g.setFont (MeteoraLookAndFeel::font (11.f, false));
        g.drawText (desc, r.withTrimmedLeft (10.f).withTrimmedTop (22.f), Justification::centredLeft);
        if (active)
        {
            g.setColour (Col::amberText);
            g.setFont (MeteoraLookAndFeel::font (15.f, true));
            g.drawText (String (CharPointer_UTF8 ("\xe2\x9c\x93")), r.removeFromRight (30.f), Justification::centred);
        }
    }
    String name, desc; bool active;
};
}

void PresetSelector::paint (Graphics& g)
{
    g.setColour (Col::cream);
    g.setFont (MeteoraLookAndFeel::font (15.f, false));
    g.drawText (proc.getProgramName (proc.getCurrentProgram()), getLocalBounds().withTrimmedLeft (6).withTrimmedRight (34), Justification::centred);
}

void PresetSelector::mouseDown (const MouseEvent&)
{
    PopupMenu m;
    m.addCustomItem (999, std::make_unique<PresetHeaderItem>(), nullptr);
    for (int i = 0; i < proc.getNumPrograms(); ++i)
        m.addCustomItem (i + 1, std::make_unique<PresetItem> (proc.getProgramName (i), MeteoraDrumProcessor::getPresetDescription (i),
                                                              i == proc.getCurrentProgram()), nullptr);
    m.showMenuAsync (PopupMenu::Options().withTargetComponent (this).withMinimumWidth (380),
                     [this] (int r)
                     {
                         if (r >= 1 && r <= proc.getNumPrograms()) { proc.setCurrentProgram (r - 1); repaint(); }
                     });
}

// =============================================================================
//  Schermata GROOVES
// =============================================================================
namespace GG
{
constexpr int listX0 = 462, listX1 = 1140, listY0 = 88, rowH = 46, visibleRows = 7;
constexpr float gx0 = 130.f, gx1 = 1150.f, gy0 = 486.f, rh = 36.f;
const char* const styles[]   = { "ROCK", "ROCK HALF TIME" };
const char* const sections[] = { "STROFA", "PRE-RITORNELLO", "RITORNELLO", "FILL" };
const char* const rowNames[] = { "CRASH", "RIDE", "HH APERTO", "HH SEMIAPERTO", "HH CHIUSO", "PEDALE HH", "TOM", "FLOOR", "SNARE", "KICK" };
inline int rowForNote (int n)
{
    switch (n)
    {
        case 49: case 57: case 52: case 55: return 0;
        case 51: case 53: case 59: return 1;
        case 27: case 28: case 46: return 2;
        case 25: case 26: return 3;
        case 22: case 23: case 24: case 42: return 4;
        case 44: return 5;
        case 48: case 50: case 45: case 47: return 6;
        case 41: case 43: return 7;
        case 37: case 38: case 40: return 8;
        case 35: case 36: return 9;
        default: return -1;
    }
}
inline Rectangle<float> styleRect (int i)   { return { 30.f, 90.f + (float) i * 50.f, 166.f, 38.f }; }
inline Rectangle<float> sectionRect (int i) { return { 236.f, 90.f + (float) i * 50.f, 186.f, 38.f }; }
inline Rectangle<float> rowRect (int i)     { return { (float) listX0, (float) (listY0 + i * rowH), (float) (listX1 - listX0), 38.f }; }
const Rectangle<float> playBtn (760.f, 444.f, 80.f, 24.f), stopBtn (846.f, 444.f, 80.f, 24.f), loopBtn (932.f, 444.f, 56.f, 24.f),
                       syncBtn (994.f, 444.f, 90.f, 24.f), bpmBox (1090.f, 444.f, 66.f, 24.f), dragBar (32.f, 856.f, 1118.f, 36.f),
                       scrollTrack (1148.f, 88.f, 8.f, 318.f), grid (gx0, gy0, gx1 - gx0, rh * 10.f);
}

GroovesPage::GroovesPage (MeteoraDrumProcessor& p) : proc (p)
{
    background = loadImage ("grooves_bg_jpg");
    humanize.setSliderStyle (Slider::LinearHorizontal);
    humanize.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
    humanize.setRange (0.0, 150.0, 1.0);
    humanize.setValue (proc.grooveHumanize.load() * 100.0, dontSendNotification);
    humanize.setDoubleClickReturnValue (true, 100.0);
    humanize.setTooltip ("Umanizzazione dei groove: 0% a griglia, 100% come suonati, 150% piu' sciolti");
    humanize.onValueChange = [this] { proc.grooveHumanize.store ((float) humanize.getValue() / 100.f); repaint(); };
    humanize.setBounds (26, 232, 174, 24);
    addAndMakeVisible (humanize);
    rebuildList();
    if (! list.empty()) select (list.front());
}

void GroovesPage::rebuildList()
{
    list.clear();
    for (int i = 0; i < kNumGrooves; ++i)
        if (style == kGrooves[i].style && section == kGrooves[i].section) list.push_back (i);
    scroll = 0;
}

void GroovesPage::select (int idx)
{
    selected = idx;
    if (! proc.grooveSync.load()) proc.grooveBpm.store ((float) kGrooves[idx].bpm);
    repaint();
}

float GroovesPage::displayBpm() const
{
    const float h = proc.hostBpm.load();
    return (proc.grooveSync.load() && h > 0.f) ? h : proc.grooveBpm.load();
}

int GroovesPage::rowAt (Point<float> p) const
{
    for (int i = 0; i < GG::visibleRows; ++i)
        if (GG::rowRect (i).contains (p) && scroll + i < (int) list.size()) return scroll + i;
    return -1;
}

void GroovesPage::paint (Graphics& g)
{
    if (background.isValid()) g.drawImageAt (background, 0, 0);
    else g.fillAll (Colour (0xff1a1512));

    auto panel = [&] (Rectangle<float> r, const char* title)
    {
        g.setColour (Colour (0xe114100c)); g.fillRoundedRectangle (r, 10.f);
        g.setColour (Col::panelLine);      g.drawRoundedRectangle (r, 10.f, 1.f);
        if (title != nullptr)
        {
            g.setColour (Col::amberText); g.setFont (MeteoraLookAndFeel::font (13.f, true));
            g.drawText (title, Rectangle<float> (r.getX() + 16.f, r.getY() + 8.f, 200.f, 20.f), Justification::centredLeft);
        }
    };
    auto button = [&] (Rectangle<float> r, const String& t, bool on, float fontSize = 11.f, Justification j = Justification::centred)
    {
        g.setColour (on ? Col::amber : Colour (0xff302a24)); g.fillRoundedRectangle (r, 6.f);
        g.setColour (Colour (0xff0a0806)); g.drawRoundedRectangle (r, 6.f, 1.f);
        g.setColour (on ? Colour (0xff28160a) : Col::label); g.setFont (MeteoraLookAndFeel::font (fontSize, true));
        g.drawText (t, j == Justification::centred ? r : r.withTrimmedLeft (14.f).withTrimmedRight (14.f), j);
    };

    g.setColour (Col::amberText); g.setFont (MeteoraLookAndFeel::font (22.f, true));
    g.drawText ("GROOVES", 24, 18, 130, 26, Justification::centredLeft);
    g.setColour (Col::soft); g.setFont (MeteoraLookAndFeel::font (12.f, false));
    g.drawText (String (kNumGrooves) + String (CharPointer_UTF8 (" pattern MIDI rock suonati con dinamica e tempo umani  \xc2\xb7  ascoltali con il kit e trascinali nella DAW")),
                150, 18, 900, 26, Justification::centredLeft);

    // stile e sezione
    panel ({ 16.f, 56.f, 194.f, 364.f }, "STILE");
    for (int i = 0; i < 2; ++i)
    {
        int n = 0; for (int k = 0; k < kNumGrooves; ++k) if (String (kGrooves[k].style) == GG::styles[i]) ++n;
        const bool on = style == GG::styles[i];
        button (GG::styleRect (i), GG::styles[i], on, 12.f, Justification::centredLeft);
        g.setColour (on ? Colour (0xff28160a) : Col::soft); g.setFont (MeteoraLookAndFeel::font (11.f, true));
        g.drawText (String (n), GG::styleRect (i).withTrimmedRight (12.f), Justification::centredRight);
    }
    // UMANIZZA
    g.setColour (Col::panelLine); g.drawLine (30.f, 200.f, 196.f, 200.f, 1.f);
    g.setColour (Col::amberText); g.setFont (MeteoraLookAndFeel::font (13.f, true));
    g.drawText ("UMANIZZA", Rectangle<float> (32.f, 208.f, 160.f, 20.f), Justification::centredLeft);
    g.setColour (Col::label); g.setFont (MeteoraLookAndFeel::font (13.f, true));
    g.drawText (String (roundToInt (humanize.getValue())) + " %", Rectangle<float> (120.f, 208.f, 76.f, 20.f), Justification::centredRight);
    g.setColour (Col::soft); g.setFont (MeteoraLookAndFeel::font (10.f, false));
    g.drawText ("0% = a griglia, dinamica piatta", Rectangle<float> (32.f, 266.f, 170.f, 14.f), Justification::centredLeft);
    g.drawText ("100% = come suonato", Rectangle<float> (32.f, 282.f, 170.f, 14.f), Justification::centredLeft);
    g.drawText (String::fromUTF8 ("150% = pi\xc3\xb9 sciolto"), Rectangle<float> (32.f, 298.f, 170.f, 14.f), Justification::centredLeft);
    g.drawText ("vale per ascolto e trascinamento", Rectangle<float> (32.f, 322.f, 170.f, 14.f), Justification::centredLeft);

    panel ({ 222.f, 56.f, 214.f, 364.f }, "SEZIONE");
    for (int i = 0; i < 4; ++i)
    {
        int n = 0; for (int k = 0; k < kNumGrooves; ++k) if (style == kGrooves[k].style && String (kGrooves[k].section) == GG::sections[i]) ++n;
        const bool on = section == GG::sections[i];
        button (GG::sectionRect (i), GG::sections[i], on, 13.f, Justification::centredLeft);
        g.setColour (on ? Colour (0xff28160a) : Col::soft); g.setFont (MeteoraLookAndFeel::font (12.f, true));
        g.drawText (String (n), GG::sectionRect (i).withTrimmedRight (14.f), Justification::centredRight);
    }

    // lista dei pattern
    panel ({ 448.f, 56.f, 720.f, 364.f }, "PATTERN");
    g.setColour (Col::soft); g.setFont (MeteoraLookAndFeel::font (10.f, true));
    g.drawText ("TEMPO", 776, 66, 60, 16, Justification::centred);
    g.drawText ("ENERGIA", 850, 66, 60, 16, Justification::centred);
    g.drawText ("TRASCINA", 1050, 66, 60, 16, Justification::centred);
    const int playing = proc.grooveCurrent.load();
    for (int i = 0; i < GG::visibleRows && scroll + i < (int) list.size(); ++i)
    {
        const int idx = list[(size_t) (scroll + i)];
        const auto& info = kGrooves[idx];
        const auto r = GG::rowRect (i);
        const bool sel = idx == selected, isPlaying = idx == playing;
        g.setColour (sel ? Colour (0xff46321e) : Colour (0xff28231e)); g.fillRoundedRectangle (r, 6.f);
        g.setColour (sel ? Col::amber : Colour (0xff0a0806)); g.drawRoundedRectangle (r, 6.f, sel ? 2.f : 1.f);
        // pulsante play / pausa
        const Point<float> c (r.getX() + 18.f, r.getCentreY());
        g.setColour (isPlaying ? Col::amber : Colour (0xff463e36)); g.fillEllipse (c.x - 11.f, c.y - 11.f, 22.f, 22.f);
        if (isPlaying)
        {
            g.setColour (Colour (0xff28160a));
            g.fillRect (c.x - 4.f, c.y - 5.f, 3.f, 10.f); g.fillRect (c.x + 1.f, c.y - 5.f, 3.f, 10.f);
        }
        else
        {
            Path tri; tri.addTriangle (c.x - 3.f, c.y - 6.f, c.x - 3.f, c.y + 6.f, c.x + 6.f, c.y);
            g.setColour (Col::label); g.fillPath (tri);
        }
        g.setColour (sel ? Colour (0xffffecd2) : Col::label); g.setFont (MeteoraLookAndFeel::font (13.f, true));
        g.drawText (info.name, Rectangle<float> (r.getX() + 40.f, r.getY() + 3.f, 270.f, 18.f), Justification::centredLeft);
        g.setColour (sel ? Colour (0xffd7c8b4) : Col::soft); g.setFont (MeteoraLookAndFeel::font (10.f, false));
        g.drawText (info.desc, Rectangle<float> (r.getX() + 40.f, r.getY() + 20.f, 260.f, 16.f), Justification::centredLeft, true);
        g.setColour (Col::label); g.setFont (MeteoraLookAndFeel::font (11.f, false));
        g.drawText (String (info.bpm) + " bpm", Rectangle<float> (776.f, r.getY(), 60.f, r.getHeight()), Justification::centred);
        for (int e = 0; e < 5; ++e)
        {
            const float hgt = 6.f + 3.f * (float) e;
            g.setColour (e < info.energy ? Col::amber : Colour (0xff463e36));
            g.fillRect (852.f + 12.f * (float) e, r.getY() + 27.f - hgt, 8.f, hgt);
        }
        const Rectangle<float> midi (1066.f, r.getY() + 7.f, 28.f, 24.f);
        g.setColour (sel ? Col::amberText : Col::soft); g.drawRoundedRectangle (midi, 4.f, 1.f);
        g.setFont (MeteoraLookAndFeel::font (8.f, true)); g.drawText ("MIDI", midi, Justification::centred);
    }
    // barra di scorrimento
    g.setColour (Colour (0xff1e1a16)); g.fillRoundedRectangle (GG::scrollTrack, 4.f);
    if ((int) list.size() > GG::visibleRows)
    {
        const float h = GG::scrollTrack.getHeight() * (float) GG::visibleRows / (float) list.size();
        const float y = GG::scrollTrack.getY() + GG::scrollTrack.getHeight() * (float) scroll / (float) list.size();
        g.setColour (Col::amber); g.fillRoundedRectangle (GG::scrollTrack.getX(), y, 8.f, h, 4.f);
    }

    // griglia del pattern selezionato
    panel ({ 16.f, 434.f, 1152.f, 466.f }, nullptr);
    const auto& info = kGrooves[selected];
    g.setColour (Col::amberText); g.setFont (MeteoraLookAndFeel::font (13.f, true));
    g.drawText (String (info.style) + String (CharPointer_UTF8 ("  \xc2\xb7  ")) + info.name + String (CharPointer_UTF8 ("  \xc2\xb7  ")) + info.desc,
                Rectangle<float> (32.f, 444.f, 720.f, 18.f), Justification::centredLeft, true);
    if (String (info.hh).isNotEmpty())
    {
        g.setColour (Col::soft); g.setFont (MeteoraLookAndFeel::font (10.f, false));
        g.drawText (String ("hi-hat: ") + info.hh, Rectangle<float> (32.f, 462.f, 720.f, 14.f), Justification::centredLeft, true);
    }
    const bool isPlay = playing == selected;
    button (GG::playBtn, String (CharPointer_UTF8 ("\xe2\x96\xb6  PLAY")), isPlay);
    button (GG::stopBtn, String (CharPointer_UTF8 ("\xe2\x96\xa0  STOP")), false);
    button (GG::loopBtn, "LOOP", proc.grooveLoop.load());
    button (GG::syncBtn, "SYNC HOST", proc.grooveSync.load());
    g.setColour (Colour (0xff0a0806)); g.fillRoundedRectangle (GG::bpmBox, 5.f);
    g.setColour (Colour (0xff5a5046)); g.drawRoundedRectangle (GG::bpmBox, 5.f, 1.f);
    g.setColour (Col::amberText); g.setFont (MeteoraLookAndFeel::font (11.f, true));
    g.drawText (String (roundToInt (displayBpm())) + " bpm", GG::bpmBox, Justification::centred);

    const auto& gd = proc.grooves[(size_t) selected];
    const int steps = 16 * info.bars;
    const float cw = (GG::gx1 - GG::gx0) / 32.f;
    for (int r = 0; r < 10; ++r)
    {
        const float y = GG::gy0 + (float) r * GG::rh;
        g.setColour (Col::label); g.setFont (MeteoraLookAndFeel::font (10.f, true));
        g.drawText (GG::rowNames[r], Rectangle<float> (32.f, y, 96.f, GG::rh), Justification::centredLeft);
        for (int st = 0; st < 32; ++st)
        {
            const float x = GG::gx0 + (float) st * cw;
            g.setColour (st >= steps ? Colour (0xff181512) : ((st / 4) % 2 == 0 ? Colour (0xff2a2520) : Colour (0xff221e1a)));
            g.fillRoundedRectangle (x + 1.5f, y + 3.f, cw - 3.f, GG::rh - 6.f, 3.f);
        }
    }
    const float bpm = displayBpm();
    const float hum = proc.grooveHumanize.load();
    for (const auto& e : gd.events)
    {
        const int r = GG::rowForNote (e.note);
        if (r < 0) continue;
        const float pos16 = (float) (e.gridPpq * 4.0) + e.offsetMs * hum * bpm / 60000.f * 4.f;   // posizione reale, umana
        const float x = GG::gx0 + pos16 * cw, y = GG::gy0 + (float) r * GG::rh;
        const float vel = e.meanVel + ((float) e.velocity - e.meanVel) * hum;
        const float a = 0.35f + 0.65f * jlimit (1.f, 127.f, vel) / 127.f;
        g.setColour (r == 5 ? Colour (0xffaa8c64) : Col::amber.withMultipliedBrightness (a).withAlpha (1.f));
        g.fillRoundedRectangle (x + 3.f, y + 6.f, cw - 6.f, GG::rh - 12.f, 3.f);
        const float hh = (GG::rh - 16.f) * jlimit (1.f, 127.f, vel) / 127.f;
        g.setColour (Colour (0xb4ffecc8)); g.fillRect (x + 5.f, y + GG::rh - 8.f - hh, 2.f, hh);
    }
    g.setColour (Col::soft); g.setFont (MeteoraLookAndFeel::font (9.f, true));
    for (int st = 0; st < 32; st += 4)
        g.drawText (String (st / 16 + 1) + "." + String ((st % 16) / 4 + 1), Rectangle<float> (GG::gx0 + (float) st * cw + 3.f, GG::gy0 - 16.f, 40.f, 14.f), Justification::bottomLeft);
    const float ph = proc.groovePos.load();
    if (isPlay && ph >= 0.f)
    {
        const float x = GG::gx0 + ph * 4.f * cw;
        g.setColour (Colour (0xe6fff0dc)); g.drawLine (x, GG::gy0 - 2.f, x, GG::gy0 + 10.f * GG::rh, 2.f);
    }
    // zona di trascinamento
    g.setColour (Colour (0xff1e1a16)); g.fillRoundedRectangle (GG::dragBar, 8.f);
    g.setColour (Col::amber.withAlpha (0.8f)); g.drawRoundedRectangle (GG::dragBar, 8.f, 1.f);
    g.setColour (Col::amberText); g.setFont (MeteoraLookAndFeel::font (12.f, true));
    g.drawText (String (CharPointer_UTF8 ("\xe2\x87\xa2   Trascina il pattern (o l'icona MIDI di una riga) su una traccia della DAW")), GG::dragBar, Justification::centred);
}

void GroovesPage::mouseDown (const MouseEvent& e)
{
    const auto p = e.position;
    dragStarted = false; pressedRow = -1; dragSource = -1; draggingBpm = draggingScroll = false;
    if (e.mods.isPopupMenu()) { if (auto* mc = findParentComponentOfClass<MainContent>()) mc->showOptions(); return; }

    for (int i = 0; i < 2; ++i) if (GG::styleRect (i).contains (p)) { style = GG::styles[i]; rebuildList(); if (! list.empty()) select (list.front()); return; }
    for (int i = 0; i < 4; ++i) if (GG::sectionRect (i).contains (p)) { section = GG::sections[i]; rebuildList(); if (! list.empty()) select (list.front()); return; }

    const int row = rowAt (p);
    if (row >= 0)
    {
        const int idx = list[(size_t) row];
        const auto r = GG::rowRect (row - scroll);
        if (p.x < r.getX() + 34.f)   // pulsante play della riga
        {
            if (proc.grooveCurrent.load() == idx) proc.stopGroove();
            else { select (idx); proc.playGroove (idx); }
        }
        else select (idx);
        pressedRow = idx;
        return;
    }
    if (GG::scrollTrack.expanded (6.f, 0.f).contains (p)) { draggingScroll = true; mouseDrag (e); return; }
    if (GG::playBtn.contains (p)) { proc.playGroove (selected); return; }
    if (GG::stopBtn.contains (p)) { proc.stopGroove(); return; }
    if (GG::loopBtn.contains (p)) { proc.grooveLoop.store (! proc.grooveLoop.load()); repaint(); return; }
    if (GG::syncBtn.contains (p)) { proc.grooveSync.store (! proc.grooveSync.load()); repaint(); return; }
    if (GG::bpmBox.contains (p))
    {
        if (e.getNumberOfClicks() > 1) proc.grooveBpm.store ((float) kGrooves[selected].bpm);
        else { draggingBpm = true; bpmAtPress = proc.grooveBpm.load(); }
        repaint(); return;
    }
    if (GG::dragBar.contains (p)) { dragSource = selected; return; }
    if (GG::grid.contains (p)) proc.playGroove (selected);
}

void GroovesPage::mouseDrag (const MouseEvent& e)
{
    if (draggingBpm)
    {
        proc.grooveBpm.store (jlimit (40.f, 240.f, bpmAtPress - (float) e.getDistanceFromDragStartY() * 0.25f));
        repaint (GG::bpmBox.toNearestInt().expanded (2));
        return;
    }
    if (draggingScroll)
    {
        const int maxScroll = jmax (0, (int) list.size() - GG::visibleRows);
        const float t = (e.position.y - GG::scrollTrack.getY()) / GG::scrollTrack.getHeight();
        scroll = jlimit (0, maxScroll, roundToInt (t * (float) list.size() - GG::visibleRows * 0.5f));
        repaint(); return;
    }
    const int src = dragSource >= 0 ? dragSource : pressedRow;
    if (src >= 0 && ! dragStarted && e.getDistanceFromDragStart() > 6)
    {
        dragStarted = true;
        dragGroove (src);
    }
}

void GroovesPage::mouseUp (const MouseEvent&)
{
    draggingBpm = draggingScroll = false; pressedRow = dragSource = -1;
}

void GroovesPage::mouseWheelMove (const MouseEvent&, const MouseWheelDetails& w)
{
    const int maxScroll = jmax (0, (int) list.size() - GG::visibleRows);
    scroll = jlimit (0, maxScroll, scroll - (w.deltaY > 0 ? 1 : (w.deltaY < 0 ? -1 : 0)));
    repaint();
}

void GroovesPage::dragGroove (int idx)
{
    const auto& info = kGrooves[idx];
    const auto midiData = proc.buildGrooveMidi (idx);
    if (midiData.getSize() == 0) return;
    auto dir = File::getSpecialLocation (File::tempDirectory).getChildFile ("Meteora Drum Grooves");
    dir.createDirectory();
    const String niceStyle = String (info.style) == "ROCK" ? "Rock" : "Rock Half Time";
    auto file = dir.getChildFile (File::createLegalFileName ("Meteora " + niceStyle + " - " + info.name + ".mid"));
    if (! file.replaceWithData (midiData.getData(), midiData.getSize())) return;
    StringArray files;
    files.add (file.getFullPathName());
    DragAndDropContainer::performExternalDragDropOfFiles (files, false, this);
}

void GroovesPage::tick()
{
    const float pos = proc.groovePos.load();
    const int cur = proc.grooveCurrent.load();
    if (std::abs (pos - lastPos) > 0.001f || cur != lastCurrent) { lastPos = pos; lastCurrent = cur; repaint(); }
}

// =============================================================================
//  Contenuto principale
// =============================================================================
MainContent::MainContent (MeteoraDrumProcessor& p) : proc (p), kit (p), mixer (p), grooves (p), presetSel (p)
{
    header = loadImage ("header_png");
    addAndMakeVisible (kit);
    addChildComponent (mixer);
    addChildComponent (grooves);

    kitTab.onClick     = [this] { showPage (0); };
    mixerTab.onClick   = [this] { showPage (1); };
    groovesTab.onClick = [this] { showPage (2); };
    for (auto* t : { &kitTab, &mixerTab, &groovesTab }) addAndMakeVisible (*t);

    presetSel.setTooltip ("Kit preset: suoni, microfoni e mix");
    addAndMakeVisible (presetSel);
    prevBtn.onClick = [this] { const int n = proc.getNumPrograms(); proc.setCurrentProgram ((proc.getCurrentProgram() + n - 1) % n); presetSel.refresh(); };
    nextBtn.onClick = [this] { const int n = proc.getNumPrograms(); proc.setCurrentProgram ((proc.getCurrentProgram() + 1) % n); presetSel.refresh(); };
    prevBtn.setTooltip ("Preset precedente");
    nextBtn.setTooltip ("Preset successivo");
    logoBtn.setTooltip ("Opzioni");
    logoBtn.onClick = [this] { showOptions(); };
    addAndMakeVisible (prevBtn);
    addAndMakeVisible (nextBtn);
    addAndMakeVisible (logoBtn);

    showPage (0);
}

void MainContent::showPage (int page)
{
    kit.setVisible (page == 0);
    mixer.setVisible (page == 1);
    grooves.setVisible (page == 2);
    kitTab.active = page == 0; mixerTab.active = page == 1; groovesTab.active = page == 2;
    for (auto* t : { &kitTab, &mixerTab, &groovesTab }) t->repaint();
}

void MainContent::resized()
{
    for (auto* c : { (Component*) &kit, (Component*) &mixer, (Component*) &grooves }) c->setBounds (0, Header, W, H - Header);
    kitTab.setBounds (30, 15, 96, 30);
    mixerTab.setBounds (136, 15, 96, 30);
    groovesTab.setBounds (242, 15, 96, 30);
    prevBtn.setBounds (835, 15, 26, 30);
    presetSel.setBounds (870, 15, 250, 30);
    nextBtn.setBounds (1128, 15, 26, 30);
    logoBtn.setBounds (W / 2 - 28, 3, 56, 54);
}

void MainContent::paint (Graphics& g)
{
    if (header.isValid()) g.drawImageAt (header, 0, 0);
    else g.fillAll (Colour (0xffa0a0a0));
    g.setColour (Colour (0xff3c3a37));
    g.setFont (MeteoraLookAndFeel::font (10.f, true));
    g.drawText ("KIT PRESET", 720, 15, 108, 30, Justification::centredRight);
}

void MainContent::mouseDown (const MouseEvent& e)
{
    if (e.mods.isPopupMenu()) showOptions();
}

void MainContent::tick()
{
    if (kit.isVisible()) kit.tick();
    if (grooves.isVisible()) grooves.tick();
    if (mixer.isVisible()) mixer.tick();
    else
    {
        // svuoto comunque i meter, così al ritorno sul mixer non saltano
        for (auto& m : proc.engine.vu) m.exchange (0.f);
        for (auto& m : proc.engine.meter) m.exchange (0.f);
    }
    if (proc.getCurrentProgram() != lastProgram) { lastProgram = proc.getCurrentProgram(); presetSel.refresh(); }
}

void MainContent::setParam (const String& id, float value)
{
    if (auto* prm = proc.apvts.getParameter (id))
        prm->setValueNotifyingHost (prm->convertTo0to1 (value));
}

void MainContent::showOptions()
{
    const int curve = roundToInt (proc.apvts.getRawParameterValue ("velcurve")->load());
    const bool cc4 = proc.apvts.getRawParameterValue ("hhcc4")->load() > 0.5f;

    PopupMenu vel;
    vel.addItem (1, "Morbida", true, curve == 0);
    vel.addItem (2, "Normale", true, curve == 1);
    vel.addItem (3, "Dura", true, curve == 2);

    PopupMenu m;
    m.addSectionHeader ("METEORA DRUM 1.0");
    m.addSubMenu ("Curva velocity", vel);
    m.addItem (10, "Pedale hi-hat via CC4", true, cc4);
    const bool align = proc.apvts.getRawParameterValue ("phase_align")->load() > 0.5f;
    m.addItem (11, "Allineamento di fase dei microfoni", true, align);
    m.addSeparator();
    m.addItem (20, "Ripristina mix naturale");
    m.addItem (30, "Mappa MIDI...");

    m.showMenuAsync (PopupMenu::Options().withTargetComponent (&logoBtn), [this, cc4] (int r)
    {
        if (r >= 1 && r <= 3) setParam ("velcurve", (float) (r - 1));
        else if (r == 10)     setParam ("hhcc4", cc4 ? 0.f : 1.f);
        else if (r == 11)     setParam ("phase_align", proc.apvts.getRawParameterValue ("phase_align")->load() > 0.5f ? 0.f : 1.f);
        else if (r == 20)     proc.setCurrentProgram (0);
        else if (r == 30)
        {
            AlertWindow::showMessageBoxAsync (MessageBoxIconType::InfoIcon, "Mappa MIDI - Meteora Drum",
                "36 / 35   Kick\n"
                "38   Snare   |   40   Rimshot   |   37   Side stick\n"
                "48 / 50 / 45 / 47   Tom\n"
                "43 / 41   Floor tom\n"
                "42   Hi-hat chiuso   |   46   Hi-hat aperto   |   44   Pedale\n"
                "22 - 28   Hi-hat per apertura (22 chiuso stretto ... 28 tutto aperto)\n"
                "21   Foot splash\n"
                "49 / 52   Crash 1   |   57 / 55   Crash 2   (solo overheads e room)\n"
                "51 / 53 / 59   Ride\n\n"
                "CC4: pedale hi-hat (0 = aperto, 127 = chiuso)\n"
                "Aftertouch polifonico sui piatti = choke\n\n"
                "Sulla foto: clic = suona, Ctrl/Cmd+clic = colpo leggero,\n"
                "Shift+clic sull'hi-hat = aperto, Alt+clic sul rullante = rimshot", "OK", this);
        }
    });
}

// =============================================================================
//  Editor
// =============================================================================
MeteoraDrumEditor::MeteoraDrumEditor (MeteoraDrumProcessor& p)
    : AudioProcessorEditor (&p), content (p)
{
    content.setLookAndFeel (&lnf);
    content.setBounds (0, 0, MainContent::W, MainContent::H);
    addAndMakeVisible (content);

    setResizable (true, true);
    setResizeLimits (MainContent::W / 2, MainContent::H / 2, (MainContent::W * 3) / 2, (MainContent::H * 3) / 2);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio ((double) MainContent::W / (double) MainContent::H);
    setSize ((MainContent::W * 80) / 100, (MainContent::H * 80) / 100);
    startTimerHz (30);
}

MeteoraDrumEditor::~MeteoraDrumEditor()
{
    stopTimer();
    content.setLookAndFeel (nullptr);
}

void MeteoraDrumEditor::paint (Graphics& g) { g.fillAll (Colour (0xff121212)); }

void MeteoraDrumEditor::resized()
{
    content.setTransform (AffineTransform::scale ((float) getWidth() / (float) MainContent::W));
}

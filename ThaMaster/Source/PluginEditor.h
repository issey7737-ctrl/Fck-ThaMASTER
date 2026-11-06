#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace tmui
{
using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
using ButtonAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;
using ComboAtt  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

namespace col
{
    const juce::Colour bg      { 0xff0e1014 };
    const juce::Colour panel   { 0xff171a21 };
    const juce::Colour panelHi { 0xff1f232c };
    const juce::Colour line    { 0xff2b303b };
    const juce::Colour text    { 0xffecebe6 };
    const juce::Colour dim     { 0xff8a8f9c };
    const juce::Colour accent  { 0xfff2c14e };      // jaune miel
    const juce::Colour cyan    { 0xff4dd6c4 };
    const juce::Colour pink    { 0xffff6b9a };
    const juce::Colour violet  { 0xff9b8cff };
    const juce::Colour orange  { 0xffff9a4d };
    const juce::Colour green   { 0xff62d26f };
    const juce::Colour red     { 0xffff4d5a };
}

class Look : public juce::LookAndFeel_V4
{
public:
    Look();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool over, bool down) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool down, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::Font (juce::FontOptions (12.0f, juce::Font::bold)); }
    juce::Label* createSliderTextBox (juce::Slider&) override;
};

// bouton rotatif avec pastille de valeur ; la couleur vient de la propriete "tint"
class Knob : public juce::Component
{
public:
    Knob (ThaMasterProcessor&, int paramIndex, juce::Colour tint, bool small = false);
    void paint (juce::Graphics&) override;
    void resized() override;
    juce::Slider slider;
private:
    juce::String label;
    juce::Colour tint;
    bool small_;
    std::unique_ptr<SliderAtt> att;
};

class Toggle : public juce::TextButton
{
public:
    Toggle (ThaMasterProcessor&, int paramIndex, const juce::String& text, juce::Colour tint);
    juce::Colour tint;
private:
    std::unique_ptr<ButtonAtt> att;
};

class Sticker : public juce::Component
{
public:
    Sticker (const juce::String& t, juce::Colour c) : text (t), colour (c) { setInterceptsMouseClicks (false, false); }
    void paint (juce::Graphics&) override;
    juce::String text; juce::Colour colour;
};

class SpectrumView : public juce::Component
{
public:
    explicit SpectrumView (ThaMasterProcessor& p) : proc (p) {}
    void paint (juce::Graphics&) override;
private:
    ThaMasterProcessor& proc;
};

class VMeter : public juce::Component       // barre verticale de reduction de gain (0..range dB, vers le bas) ou niveau
{
public:
    VMeter (float range, juce::Colour c, bool fromBottom = false) : range_ (range), colour (c), bottom (fromBottom) {}
    void set (float v) { value = v; repaint(); }
    void paint (juce::Graphics&) override;
private:
    float range_, value = 0; juce::Colour colour; bool bottom;
};

class StereoView : public juce::Component
{
public:
    explicit StereoView (ThaMasterProcessor& p) : proc (p) {}
    void paint (juce::Graphics&) override;
private:
    ThaMasterProcessor& proc;
};

class ThaMasterEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit ThaMasterEditor (ThaMasterProcessor&);
    ~ThaMasterEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    void drawBox (juce::Graphics&, juce::Rectangle<int>, const juce::String& title, juce::Colour c);

    ThaMasterProcessor& proc;
    Look look;

    Knob kCeil, kIn, kSpec, kGlue, kDetail, kColor, kWidth, kBass, kAir, kClipG, kLimG, kOut;
    Toggle tBypass, tMatch, tSpec, tComp, tAna, tSt, tEq, tClip, tLim, tLM, tTP;
    juce::TextButton bAuto { "AUTO" }, bAutoIn { "AUTO IN" }, bA { "A" }, bB { "B" }, bAtoB { "A>B" }, bPrev { "<" }, bNext { ">" };
    juce::ComboBox cbType, cbMon, cbTarget, cbPreset;
    std::unique_ptr<ComboAtt> aType, aMon, aTarget;
    Sticker sSpec { "SPECTRAL", col::cyan }, sComp { "COMP", col::pink }, sAna { "ANALOG", col::orange }, sSt { "STEREO", col::violet },
            sEq { "EQ", col::green }, sClip { "CLIP", col::red }, sLim { "LIMITER", col::accent };
    SpectrumView spectrum { proc };
    StereoView stereoView { proc };
    VMeter grL { 10, col::pink }, grM { 10, col::pink }, grH { 10, col::pink }, clipM { 10, col::red }, limM { 10, col::accent }, outL { 60, col::green, true }, outR { 60, col::green, true };

    juce::String sLufsS = "--", sLufsI = "--", sPeak = "--", sDyn = "--", sDynWord = "", sStereoWord = "";
    juce::Rectangle<int> rSpec, rComp, rAna, rSt, rEq, rClip, rLim, rStrip, rTop;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThaMasterEditor)
};
} // namespace tmui

using ThaMasterEditor = tmui::ThaMasterEditor;

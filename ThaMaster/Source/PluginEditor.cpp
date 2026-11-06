#include "PluginEditor.h"

using namespace tmx;
using namespace tmui;

static juce::Font fnt (float h, bool bold = false) { return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain)); }

//==============================================================================
Look::Look()
{
    setColour (juce::ComboBox::textColourId, col::text);
    setColour (juce::PopupMenu::backgroundColourId, col::panel);
    setColour (juce::PopupMenu::textColourId, col::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, col::panelHi);
    setColour (juce::PopupMenu::highlightedTextColourId, col::accent);
}

void Look::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
{
    const auto tint = juce::Colour ((juce::uint32) (int) s.getProperties().getWithDefault ("tint", (int) col::accent.getARGB()));
    const auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.0f);
    const float d = std::min (b.getWidth(), b.getHeight()), r = d * 0.5f;
    const auto c = b.getCentre();
    // corps
    g.setGradientFill (juce::ColourGradient (col::panelHi.brighter (0.15f), c.x, c.y - r, col::panel.darker (0.3f), c.x, c.y + r, false));
    g.fillEllipse (c.x - r * 0.78f, c.y - r * 0.78f, r * 1.56f, r * 1.56f);
    g.setColour (col::line); g.drawEllipse (c.x - r * 0.78f, c.y - r * 0.78f, r * 1.56f, r * 1.56f, 1.2f);
    // arc
    const float ar = r * 0.93f, th = 3.2f;
    juce::Path t; t.addCentredArc (c.x, c.y, ar, ar, 0, a0, a1, true);
    g.setColour (col::line); g.strokePath (t, juce::PathStrokeType (th, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    const float ang = a0 + pos * (a1 - a0);
    juce::Path v; v.addCentredArc (c.x, c.y, ar, ar, 0, a0, ang, true);
    g.setColour (tint); g.strokePath (v, juce::PathStrokeType (th, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    // repere
    const float pr = r * 0.78f;
    g.setColour (col::text);
    g.drawLine (c.x + std::sin (ang) * pr * 0.35f, c.y - std::cos (ang) * pr * 0.35f, c.x + std::sin (ang) * pr * 0.88f, c.y - std::cos (ang) * pr * 0.88f, 2.4f);
}

void Look::drawButtonBackground (juce::Graphics& g, juce::Button& bt, const juce::Colour&, bool over, bool down)
{
    auto b = bt.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = bt.getToggleState();
    auto tint = juce::Colour ((juce::uint32) (int) bt.getProperties().getWithDefault ("tint", (int) col::accent.getARGB()));
    if (bt.getProperties().contains ("round"))
    {
        const float prog = (float) bt.getProperties().getWithDefault ("prog", 0.0);
        const bool busy = (bool) bt.getProperties().getWithDefault ("busy", false);
        const auto c = b.getCentre(); const float r = std::min (b.getWidth(), b.getHeight()) * 0.5f - 2.0f;
        g.setColour (busy ? col::panelHi : (down ? col::red.darker (0.3f) : (over ? col::red.brighter (0.15f) : col::red)));
        g.fillEllipse (c.x - r, c.y - r, 2 * r, 2 * r);
        g.setColour (juce::Colours::white.withAlpha (0.25f)); g.drawEllipse (c.x - r, c.y - r, 2 * r, 2 * r, 1.5f);
        if (busy) { juce::Path p; p.addCentredArc (c.x, c.y, r, r, 0, 0, juce::MathConstants<float>::twoPi * prog, true); g.setColour (col::accent); g.strokePath (p, juce::PathStrokeType (4.0f)); }
        return;
    }
    g.setColour (on ? tint : (down ? col::line : col::panelHi));
    g.fillRoundedRectangle (b, 5.0f);
    g.setColour (on ? tint.brighter (0.3f) : (over ? col::dim : col::line));
    g.drawRoundedRectangle (b, 5.0f, 1.0f);
}

void Look::drawButtonText (juce::Graphics& g, juce::TextButton& bt, bool, bool)
{
    const bool on = bt.getToggleState();
    const bool round = bt.getProperties().contains ("round");
    g.setColour (round ? juce::Colours::white : (on ? juce::Colour (0xff101216) : col::text));
    g.setFont (fnt (round ? 14.0f : 11.0f, true));
    g.drawText (bt.getButtonText(), bt.getLocalBounds(), juce::Justification::centred);
}

void Look::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    auto b = juce::Rectangle<float> (0.f, 0.f, (float) w, (float) h).reduced (0.5f);
    g.setColour (col::panelHi); g.fillRoundedRectangle (b, 5.0f);
    g.setColour (col::line); g.drawRoundedRectangle (b, 5.0f, 1.0f);
    juce::Path p; const float cx = (float) w - 12.0f, cy = (float) h * 0.5f;
    p.addTriangle (cx - 4, cy - 2, cx + 4, cy - 2, cx, cy + 3);
    g.setColour (col::dim); g.fillPath (p);
}

juce::Label* Look::createSliderTextBox (juce::Slider&) { return new juce::Label(); }

//==============================================================================
Knob::Knob (ThaMasterProcessor& p, int idx, juce::Colour t, bool small) : tint (t), small_ (small)
{
    const auto& d = kDefs[idx];
    label = d.label;
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
    slider.setTextValueSuffix (d.unit);
    slider.setNumDecimalPlacesToDisplay (d.step < 1.0f ? 1 : 0);
    slider.setDoubleClickReturnValue (true, d.def);
    slider.getProperties().set ("tint", (int) tint.getARGB());
    slider.onValueChange = [this] { repaint(); };
    addAndMakeVisible (slider);
    att = std::make_unique<SliderAtt> (p.apvts, d.id, slider);
}
void Knob::resized()
{
    auto b = getLocalBounds();
    b.removeFromTop (small_ ? 11 : 13);
    b.removeFromBottom (small_ ? 16 : 18);
    slider.setBounds (b);
}
void Knob::paint (juce::Graphics& g)
{
    auto b = getLocalBounds();
    g.setColour (col::dim); g.setFont (fnt (small_ ? 9.0f : 10.0f, true));
    g.drawText (label, b.removeFromTop (small_ ? 11 : 13), juce::Justification::centred);
    auto pill = b.removeFromBottom (small_ ? 16 : 18).withSizeKeepingCentre (std::min (b.getWidth(), small_ ? 52 : 58), small_ ? 14 : 16);
    g.setColour (col::bg); g.fillRoundedRectangle (pill.toFloat(), 7.0f);
    g.setColour (tint.withAlpha (0.7f)); g.drawRoundedRectangle (pill.toFloat(), 7.0f, 1.0f);
    g.setColour (col::text); g.setFont (fnt (small_ ? 9.5f : 10.5f, true));
    g.drawText (slider.getTextFromValue (slider.getValue()), pill, juce::Justification::centred);
}

Toggle::Toggle (ThaMasterProcessor& p, int idx, const juce::String& text, juce::Colour t) : juce::TextButton (text), tint (t)
{
    setClickingTogglesState (true);
    getProperties().set ("tint", (int) tint.getARGB());
    att = std::make_unique<ButtonAtt> (p.apvts, kDefs[idx].id, *this);
}

void Sticker::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (colour); g.fillRoundedRectangle (b, 4.0f);
    g.setColour (juce::Colour (0xff101216)); g.setFont (fnt (11.0f, true));
    g.drawText (text, getLocalBounds(), juce::Justification::centred);
}

void VMeter::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour (col::bg); g.fillRoundedRectangle (b, 3.0f);
    const float f = juce::jlimit (0.f, 1.f, bottom ? value / range_ : value / range_);
    auto fill = b.reduced (2.0f);
    if (bottom) fill = fill.withTop (fill.getBottom() - fill.getHeight() * f);
    else        fill = fill.withHeight (fill.getHeight() * f);
    g.setColour (colour); g.fillRoundedRectangle (fill, 2.0f);
    g.setColour (col::line); g.drawRoundedRectangle (b, 3.0f, 1.0f);
}

//==============================================================================
void SpectrumView::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour (col::bg); g.fillRoundedRectangle (b, 6.0f);
    g.setColour (col::line); g.drawRoundedRectangle (b, 6.0f, 1.0f);
    auto a = b.reduced (6.0f, 6.0f);
    auto xOf = [&] (double f) { return a.getX() + a.getWidth() * (float) (std::log (f / 20.0) / std::log (1000.0)); };
    const float mid = a.getCentreY(), sc = a.getHeight() * 0.5f / 8.0f;
    g.setColour (col::line.withAlpha (0.6f));
    for (double f : { 50.0, 100.0, 200.0, 500.0, 1000.0, 2000.0, 5000.0, 10000.0 }) g.drawVerticalLine ((int) xOf (f), a.getY(), a.getBottom());
    g.drawHorizontalLine ((int) mid, a.getX(), a.getRight());
    g.setColour (col::dim); g.setFont (fnt (8.5f));
    for (auto fl : { std::pair<double, const char*> { 100, "100" }, { 1000, "1k" }, { 10000, "10k" } }) g.drawText (fl.second, (int) xOf (fl.first) + 2, (int) a.getBottom() - 11, 26, 10, juce::Justification::left);
    // ecart mesure (points) puis courbe de correction (remplie)
    auto& sp = proc.engine.spec;
    static const double fc[Spectral::kNB] = { 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500, 16000 };
    float mean = 0; int n = 0;
    for (int i = 0; i < Spectral::kNB; ++i) { const float v = sp.devOutBands[(size_t) i].load(); if (fc[i] >= 200 && fc[i] <= 5000) { mean += v; ++n; } }
    mean /= (float) std::max (1, n);
    g.setColour (col::dim);
    for (int i = 0; i < Spectral::kNB; ++i)
    {
        const float v = juce::jlimit (-8.f, 8.f, sp.devOutBands[(size_t) i].load());
        (void) mean;
        g.fillEllipse (xOf (fc[i]) - 2.0f, mid - v * sc - 2.0f, 4.0f, 4.0f);
    }
    juce::Path line;
    for (int q = 0; q < Spectral::kCurve; ++q)
    {
        const float db = juce::jlimit (-8.f, 8.f, sp.curveOut[(size_t) q].load());
        const float x = xOf (sp.curveF[(size_t) q]), y = mid - db * sc;
        if (q == 0) line.startNewSubPath (x, y); else line.lineTo (x, y);
    }
    juce::Path fill (line); fill.lineTo (a.getRight(), mid); fill.lineTo (a.getX(), mid); fill.closeSubPath();
    g.setColour (col::cyan.withAlpha (0.18f)); g.fillPath (fill);
    g.setColour (col::cyan); g.strokePath (line, juce::PathStrokeType (2.0f));
    g.setColour (col::dim); g.setFont (fnt (8.5f));
    g.drawText ("ecart mesure (points) / correction (courbe)", a.toNearestInt().removeFromTop (11), juce::Justification::topRight);
}

void StereoView::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour (col::bg); g.fillRoundedRectangle (b, 5.0f);
    g.setColour (col::line); g.drawRoundedRectangle (b, 5.0f, 1.0f);
    const float c = proc.engine.corr.load();     // -1..1
    auto a = b.reduced (6.0f, 0.0f);
    const float cy = b.getCentreY(), w = a.getWidth();
    g.setColour (col::line); g.drawHorizontalLine ((int) cy, a.getX(), a.getRight());
    g.drawVerticalLine ((int) (a.getX() + w * 0.5f), b.getY() + 3, b.getBottom() - 3);
    const float x = a.getX() + w * (0.5f + 0.5f * c);
    g.setColour (c > 0.1f ? col::violet : col::red);
    g.fillRoundedRectangle (x - 5.0f, cy - 6.0f, 10.0f, 12.0f, 3.0f);
    g.setColour (col::dim); g.setFont (fnt (8.5f, true));
    g.drawText ("L/R", b.toNearestInt().removeFromLeft (22), juce::Justification::centred);
    g.drawText ("MONO", b.toNearestInt().removeFromRight (30), juce::Justification::centred);
}

//==============================================================================
ThaMasterEditor::ThaMasterEditor (ThaMasterProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      kCeil (p, limCeil, col::accent, true), kIn (p, inGain, col::cyan, true), kSpec (p, specAmt, col::cyan), kGlue (p, compGlue, col::pink), kDetail (p, compDetail, col::pink, true),
      kColor (p, anaColor, col::orange), kWidth (p, stWidth, col::violet), kBass (p, eqBass, col::green), kAir (p, eqAir, col::green),
      kClipG (p, clipGain, col::red), kLimG (p, limGain, col::accent), kOut (p, outGain, col::cyan, true),
      tBypass (p, bypass, "BYPASS", col::red), tMatch (p, matchOn, "MATCH", col::cyan), tSpec (p, specOn, "ON", col::cyan), tComp (p, compOn, "ON", col::pink), tAna (p, anaOn, "ON", col::orange),
      tSt (p, stOn, "ON", col::violet), tEq (p, eqOn, "ON", col::green), tClip (p, clipOn, "ON", col::red), tLim (p, limOn, "ON", col::accent),
      tLM (p, stLM, "LM", col::violet), tTP (p, limTP, "TP", col::accent)
{
    setLookAndFeel (&look);
    setSize (560, 630);
    for (auto* c : std::initializer_list<juce::Component*> { &kCeil, &kIn, &kSpec, &kGlue, &kDetail, &kColor, &kWidth, &kBass, &kAir, &kClipG, &kLimG, &kOut,
                                                              &tBypass, &tMatch, &tSpec, &tComp, &tAna, &tSt, &tEq, &tClip, &tLim, &tLM, &tTP,
                                                              &bAuto, &bAutoIn, &bA, &bB, &bAtoB, &bPrev, &bNext, &cbType, &cbMon, &cbTarget, &cbPreset,
                                                              &sSpec, &sComp, &sAna, &sSt, &sEq, &sClip, &sLim, &spectrum, &stereoView,
                                                              &grL, &grM, &grH, &clipM, &limM, &outL, &outR })
        addAndMakeVisible (c);

    for (auto id : { kDefs[anaType].unit }) { juce::StringArray s; s.addTokens (id, "|", ""); for (int i = 0; i < s.size(); ++i) cbType.addItem (s[i], i + 1); }
    { juce::StringArray s; s.addTokens (kDefs[monitor].unit, "|", ""); for (int i = 0; i < s.size(); ++i) cbMon.addItem ("Monitor: " + s[i], i + 1); }
    { juce::StringArray s; s.addTokens (kDefs[target].unit, "|", ""); for (int i = 0; i < s.size(); ++i) cbTarget.addItem (s[i], i + 1); }
    aType = std::make_unique<ComboAtt> (p.apvts, kDefs[anaType].id, cbType);
    aMon = std::make_unique<ComboAtt> (p.apvts, kDefs[monitor].id, cbMon);
    aTarget = std::make_unique<ComboAtt> (p.apvts, kDefs[target].id, cbTarget);
    for (int i = 0; i < ThaMasterProcessor::presetNames().size(); ++i) cbPreset.addItem (ThaMasterProcessor::presetNames()[i], i + 1);
    cbPreset.setSelectedId (proc.getCurrentPreset() + 1, juce::dontSendNotification);
    cbPreset.onChange = [this] { proc.applyPreset (cbPreset.getSelectedId() - 1); };
    bPrev.onClick = [this] { const int i = std::max (0, proc.getCurrentPreset() - 1); proc.applyPreset (i); cbPreset.setSelectedId (i + 1, juce::dontSendNotification); };
    bNext.onClick = [this] { const int i = std::min (ThaMasterProcessor::presetNames().size() - 1, proc.getCurrentPreset() + 1); proc.applyPreset (i); cbPreset.setSelectedId (i + 1, juce::dontSendNotification); };
    bA.onClick = [this] { if (proc.getAB() != 0) proc.switchAB(); };
    bB.onClick = [this] { if (proc.getAB() != 1) proc.switchAB(); };
    bAtoB.onClick = [this] { proc.copyToOther(); };
    bAuto.getProperties().set ("round", true);
    bAuto.onClick = [this] { if (proc.getAutoState() != 0) proc.cancelAuto(); else proc.startAuto(); };
    bAutoIn.onClick = [this] { if (proc.getAutoState() != 0) proc.cancelAuto(); else proc.startAutoInput(); };
    bA.setClickingTogglesState (false); bB.setClickingTogglesState (false);
    startTimerHz (30);
}

ThaMasterEditor::~ThaMasterEditor() { stopTimer(); setLookAndFeel (nullptr); }

void ThaMasterEditor::drawBox (juce::Graphics& g, juce::Rectangle<int> r, const juce::String&, juce::Colour c)
{
    g.setColour (col::panel); g.fillRoundedRectangle (r.toFloat(), 8.0f);
    g.setColour (c.withAlpha (0.35f)); g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 8.0f, 1.0f);
}

void ThaMasterEditor::paint (juce::Graphics& g)
{
    g.fillAll (col::bg);
    g.setColour (col::panel); g.fillRect (rTop);
    g.setColour (col::line); g.drawHorizontalLine (rTop.getBottom(), 0.f, (float) getWidth());
    drawBox (g, rSpec, "", col::cyan); drawBox (g, rComp, "", col::pink); drawBox (g, rAna, "", col::orange); drawBox (g, rSt, "", col::violet);
    drawBox (g, rEq, "", col::green); drawBox (g, rClip, "", col::red); drawBox (g, rLim, "", col::accent);
    // bandeau de mesure
    g.setColour (col::panel); g.fillRoundedRectangle (rStrip.toFloat(), 8.0f);
    g.setColour (col::line); g.drawRoundedRectangle (rStrip.toFloat().reduced (0.5f), 8.0f, 1.0f);
    const int cw = (rStrip.getWidth() - 150) / 5;
    auto cell = [&] (int i, const juce::String& title, const juce::String& val, const juce::String& word, juce::Colour c)
    {
        auto r = juce::Rectangle<int> (rStrip.getX() + 150 + i * cw, rStrip.getY() + 6, cw, rStrip.getHeight() - 12);
        g.setColour (col::dim); g.setFont (fnt (9.5f, true)); g.drawText (title, r.removeFromTop (14), juce::Justification::centred);
        g.setColour (c); g.setFont (fnt (17.0f, true)); g.drawText (val, r.removeFromTop (24), juce::Justification::centred);
        g.setColour (col::dim); g.setFont (fnt (9.5f, true)); g.drawText (word, r, juce::Justification::centred);
    };
    cell (0, "LUFS S", sLufsS, "short-term", col::text);
    cell (1, "LUFS I", sLufsI, "integrated", col::accent);
    cell (2, "PEAK", sPeak, "true peak", col::text);
    cell (3, "DYNAMICS", sDyn, sDynWord, col::cyan);
    cell (4, "STEREO", sStereoWord, "image", col::violet);
    g.setColour (col::dim); g.setFont (fnt (9.0f, true));
    for (int i = 0; i < 3; ++i) g.drawText (i == 0 ? "LOW" : i == 1 ? "MID" : "HIGH", rComp.getX() + 160 + i * 28, rComp.getBottom() - 16, 32, 12, juce::Justification::centred);
    // titre
    g.setColour (col::text); g.setFont (fnt (11.0f, true));
    g.drawText ("Fck ThaMASTER", getLocalBounds().removeFromBottom (14).reduced (10, 0), juce::Justification::centredRight);
}

void ThaMasterEditor::resized()
{
    const int W = getWidth();
    rTop   = { 0, 0, W, 78 };
    rSpec  = { 10, 86, W - 20, 150 };
    rComp  = { 10, 244, 270, 142 };
    rAna   = { 288, 244, W - 298, 66 };
    rSt    = { 288, 316, W - 298, 70 };
    rEq    = { 10, 394, 170, 120 };
    rClip  = { 188, 394, 170, 120 };
    rLim   = { 366, 394, W - 376, 120 };
    rStrip = { 10, 522, W - 20, 74 };

    // barre du haut
    kIn.setBounds (8, 4, 56, 70);
    bAutoIn.setBounds (70, 8, 70, 22);
    bA.setBounds (70, 36, 24, 22); bB.setBounds (97, 36, 24, 22); bAtoB.setBounds (124, 36, 40, 22);
    bPrev.setBounds (172, 8, 22, 22); cbPreset.setBounds (196, 8, 140, 22); bNext.setBounds (338, 8, 22, 22);
    cbMon.setBounds (172, 36, 150, 22);
    tBypass.setBounds (366, 8, 60, 22); tMatch.setBounds (366, 36, 60, 22);
    kOut.setBounds (428, 4, 52, 70);
    bAuto.setBounds (W - 62, 6, 52, 52); cbTarget.setBounds (W - 72, 58, 66, 17);

    // spectral
    sSpec.setBounds (rSpec.getX() + 8, rSpec.getY() + 8, 80, 18);
    tSpec.setBounds (rSpec.getX() + 8, rSpec.getY() + 32, 42, 20);
    kSpec.setBounds (rSpec.getX() + 6, rSpec.getY() + 58, 84, 88);
    spectrum.setBounds (rSpec.getX() + 98, rSpec.getY() + 8, rSpec.getWidth() - 106, rSpec.getHeight() - 16);

    // comp
    sComp.setBounds (rComp.getX() + 8, rComp.getY() + 8, 70, 18);
    tComp.setBounds (rComp.getX() + 86, rComp.getY() + 8, 42, 18);
    kGlue.setBounds (rComp.getX() + 6, rComp.getY() + 34, 84, 102);
    kDetail.setBounds (rComp.getX() + 92, rComp.getY() + 40, 66, 84);
    { const int x = rComp.getX() + 168, y = rComp.getY() + 14, h = rComp.getHeight() - 28;
      grL.setBounds (x, y, 16, h - 12); grM.setBounds (x + 28, y, 16, h - 12); grH.setBounds (x + 56, y, 16, h - 12); }

    // analog
    sAna.setBounds (rAna.getX() + 8, rAna.getY() + 8, 66, 18);
    tAna.setBounds (rAna.getX() + 8, rAna.getY() + 36, 42, 22);
    kColor.setBounds (rAna.getX() + 56, rAna.getY() + 2, 70, 62);
    cbType.setBounds (rAna.getX() + 134, rAna.getY() + 22, rAna.getWidth() - 142, 22);

    // stereo
    sSt.setBounds (rSt.getX() + 8, rSt.getY() + 8, 66, 18);
    tSt.setBounds (rSt.getX() + 8, rSt.getY() + 40, 42, 22);
    kWidth.setBounds (rSt.getX() + 56, rSt.getY() + 2, 70, 66);
    tLM.setBounds (rSt.getX() + 134, rSt.getY() + 8, 40, 22);
    stereoView.setBounds (rSt.getX() + 134, rSt.getY() + 38, rSt.getWidth() - 142, 24);

    // eq
    sEq.setBounds (rEq.getX() + 8, rEq.getY() + 8, 50, 18);
    tEq.setBounds (rEq.getX() + 64, rEq.getY() + 8, 42, 18);
    kBass.setBounds (rEq.getX() + 6, rEq.getY() + 32, 76, 84);
    kAir.setBounds (rEq.getX() + 88, rEq.getY() + 32, 76, 84);

    // clip
    sClip.setBounds (rClip.getX() + 8, rClip.getY() + 8, 50, 18);
    tClip.setBounds (rClip.getX() + 64, rClip.getY() + 8, 42, 18);
    kClipG.setBounds (rClip.getX() + 10, rClip.getY() + 32, 84, 84);
    clipM.setBounds (rClip.getX() + 112, rClip.getY() + 12, 16, rClip.getHeight() - 24);

    // limiter
    sLim.setBounds (rLim.getX() + 8, rLim.getY() + 8, 66, 18);
    tLim.setBounds (rLim.getX() + 80, rLim.getY() + 8, 36, 18);
    tTP.setBounds (rLim.getX() + 120, rLim.getY() + 8, 36, 18);
    kLimG.setBounds (rLim.getX() + 6, rLim.getY() + 32, 70, 84);
    kCeil.setBounds (rLim.getX() + 80, rLim.getY() + 34, 54, 80);
    limM.setBounds (rLim.getRight() - 24, rLim.getY() + 32, 14, rLim.getHeight() - 42);

    // bandeau : mini analyseur de niveau
    outL.setBounds (rStrip.getX() + 14, rStrip.getY() + 10, 12, rStrip.getHeight() - 20);
    outR.setBounds (rStrip.getX() + 30, rStrip.getY() + 10, 12, rStrip.getHeight() - 20);
}

void ThaMasterEditor::timerCallback()
{
    auto& e = proc.engine;
    grL.set (e.glue.grOut[0].load()); grM.set (e.glue.grOut[1].load()); grH.set (e.glue.grOut[2].load());
    limM.set (e.lim.grOut.load());
    clipM.set (e.clipAct.load());
    outL.set (juce::jlimit (0.f, 60.f, 60.f + (float) tmx::linToDb ((double) e.peakOutL.load())));
    outR.set (juce::jlimit (0.f, 60.f, 60.f + (float) tmx::linToDb ((double) e.peakOutR.load())));

    const double s = e.outShort(), in = e.outIntegrated(), tp = (double) e.tpOut.load();
    auto fmt = [] (double v, int d = 1) { return v < -100 ? juce::String ("--") : juce::String (v, d); };
    sLufsS = fmt (s); sLufsI = fmt (in);
    sPeak = tp < -100 ? juce::String ("--") : juce::String (tp, 1) + " dB";
    if (s > -60 && tp > -100)
    {
        const double plr = tp - s;
        sDyn = juce::String (plr, 1);
        sDynWord = plr > 14 ? "OPEN" : plr > 11 ? "PUNCHY" : plr > 8.5 ? "LOUD" : plr > 6.5 ? "DENSE" : "CRUSHED";
    }
    else { sDyn = "--"; sDynWord = ""; }
    const float c = e.corr.load();
    sStereoWord = c > 0.97f ? "MONO" : c > 0.6f ? "NARROW" : c > 0.2f ? "WIDE" : c > -0.1f ? "VERY WIDE" : "PHASE!";

    const int a = proc.getAutoState();
    bAuto.getProperties().set ("busy", a != 0);
    bAuto.getProperties().set ("prog", proc.getAutoProgress());
    bAuto.setButtonText (a == 1 ? "..." : a == 2 ? "FIX" : "AUTO");
    bAutoIn.setButtonText (a == 3 ? "..." : "AUTO IN");
    bAuto.repaint();
    bA.setToggleState (proc.getAB() == 0, juce::dontSendNotification);
    bB.setToggleState (proc.getAB() == 1, juce::dontSendNotification);
    spectrum.repaint(); stereoView.repaint();
    repaint (rStrip);
}

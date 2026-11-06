#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "TmDsp.h"

class ThaMasterProcessor : public juce::AudioProcessor, private juce::AsyncUpdater
{
public:
    ThaMasterProcessor();
    ~ThaMasterProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Fck ThaMASTER"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    int getNumPrograms() override { return presetNames().size(); }
    int getCurrentProgram() override { return currentPreset; }
    void setCurrentProgram (int i) override { applyPreset (i); }
    const juce::String getProgramName (int i) override { return presetNames()[i]; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static const juce::StringArray& presetNames();
    void applyPreset (int index);
    int getCurrentPreset() const noexcept { return currentPreset; }

    // A/B
    int  getAB() const noexcept { return abActive; }
    void switchAB();
    void copyToOther();

    // AUTO : 0 repos, 1 analyse entree, 2 verification sortie, 3 auto-input
    void startAuto();
    void startAutoInput();
    void cancelAuto() { autoState.store (0); }
    int  getAutoState() const noexcept { return autoState.load(); }
    float getAutoProgress() const noexcept { return autoProg.load(); }

    float getParam (int idx) const { return raw[(size_t) idx]->load(); }

    juce::AudioProcessorValueTreeState apvts;
    tmx::Engine engine;

private:
    void handleAsyncUpdate() override;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void captureSlot (int s);
    void loadSlot (int s);
    void setParamNorm (int idx, float plainValue);

    std::array<std::atomic<float>*, tmx::Count> raw {};
    std::array<float, tmx::Count> cur {};
    std::array<std::array<float, tmx::Count>, 2> slots {};
    std::array<bool, 2> slotValid { false, false };
    int abActive = 0, currentPreset = 0;
    double sr = 48000;

    std::atomic<int> autoState { 0 };
    std::atomic<float> autoProg { 0.f };
    std::atomic<bool> resetReq { false };
    double autoTime = 0, autoNeeded = 8.0, activeTime = 0;
    std::atomic<int> pendingKind { 0 };           // 1 : appliquer resultat phase 1, 2 : phase 2, 3 : auto-input
    std::atomic<float> pendA { 0.f }, pendB { 0.f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThaMasterProcessor)
};

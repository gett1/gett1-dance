#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "Motion.h"
#include "Meter.h"
#include <limits>

class OverlayController;

class DanceProcessor final : public juce::AudioProcessor, private juce::AsyncUpdater {
public:
    DanceProcessor();
    ~DanceProcessor() override;
    const juce::String getName() const override { return "gett1 Dance"; }
    void prepareToPlay(double, int) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0; }
    int getNumPrograms() override { return dance::presetCount; }
    int getCurrentProgram() override;
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int i) override { return dance::names[juce::jlimit(0,dance::presetCount-1,i)]; }
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;
    juce::AudioProcessorValueTreeState state;
    struct Clock {
        double ppq = 0, bpm = 120, stamp = 0;
        bool playing = false, valid = false;
        dance::Meter meter;
        double barStartPpq = std::numeric_limits<double>::quiet_NaN();
    };
    Clock clock() const noexcept;
private:
    void handleAsyncUpdate() override;
    // Lifetime belongs to the plugin instance, never to its disposable editor.
    std::unique_ptr<OverlayController> overlay;
    void updateClock(int);
    double sampleRate=44100, fallbackPpq=0;
    std::atomic<unsigned> revision{0};
    std::atomic<double> ppq{0}, bpm{120}, stamp{0};
    std::atomic<int> numerator{4}, denominator{4};
    std::atomic<double> barStartPpq{std::numeric_limits<double>::quiet_NaN()};
    std::atomic<bool> playing{false}, valid{false};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DanceProcessor)
};

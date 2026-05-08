#pragma once

#include "PluginProcessor.h"

class ErbeyVerbyAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                             private juce::Timer
{
public:
    explicit ErbeyVerbyAudioProcessorEditor (ErbeyVerbyAudioProcessor&);
    ~ErbeyVerbyAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    struct KnobLookAndFeel final : public juce::LookAndFeel_V4
    {
        void drawRotarySlider (juce::Graphics&,
                               int x,
                               int y,
                               int width,
                               int height,
                               float sliderPosProportional,
                               float rotaryStartAngle,
                               float rotaryEndAngle,
                               juce::Slider&) override;
    };

    struct ModSlider final : public juce::Slider
    {
        bool hasScript = false;
        bool scriptActive = false;
        std::function<void()> onDotClicked;
        std::function<void()> onDotDoubleClicked;
        std::function<void()> onManualDragStarted;
        void mouseDown (const juce::MouseEvent&) override;
    };

    struct Control
    {
        ModSlider slider;
        juce::Label label;
        juce::String parameterId;
        std::unique_ptr<SliderAttachment> attachment;
    };

    void setupControl (Control& control, const juce::String& name, const juce::String& parameterId);
    void openFabricScriptEditor (const juce::String& parameterId, const juce::String& parameterName);
    void timerCallback() override;
    void updateScriptIndicator (Control& control);
    void updateModulatedSlider (Control& control);
    float getDisplayedPaths() const;

    ErbeyVerbyAudioProcessor& audioProcessor;
    KnobLookAndFeel knobLookAndFeel;
    juce::Label title;
    juce::Label subtitle;
    float displayedPaths = 0.0f;

    Control paths;
    Control size;
    Control coupling;
    Control skew;
    Control feedback;
    Control damping;
    Control freeze;
    Control octaveUp;
    Control octaveDown;
    Control mix;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ErbeyVerbyAudioProcessorEditor)
};

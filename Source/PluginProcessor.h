#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

class ErbeyVerbyAudioProcessor final : public juce::AudioProcessor
{
public:
    ErbeyVerbyAudioProcessor();
    ~ErbeyVerbyAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int) override;
    const juce::String getProgramName (int) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void setFabricScriptForParameter (const juce::String& parameterId, const juce::String& script);
    juce::String getFabricScriptForParameter (const juce::String& parameterId) const;
    void setFabricScriptActive (const juce::String& parameterId, bool active);
    bool isFabricScriptActive (const juce::String& parameterId) const;
    float getCurrentModulatedParameterValue (const juce::String& parameterId) const;
    bool parameterHasFabricScript (const juce::String& parameterId) const;

    juce::AudioProcessorValueTreeState parameters;

private:
    static constexpr int maxPaths = 16;
    static constexpr float maxDelaySeconds = 12.0f;

    struct DelayPath
    {
        std::vector<float> buffer;
        int writePosition = 0;

        void prepare (int size);
        void clear();
        float read (float delaySamples) const;
        void write (float sample);
    };

    struct OnePole
    {
        float z = 0.0f;
        float process (float input, float coefficient) noexcept
        {
            z += coefficient * (input - z);
            return z;
        }
        void reset() noexcept { z = 0.0f; }
    };

    struct PitchDelayLine
    {
        DelayPath delay;
        float phase = 0.0f;

        void prepare (int size);
        void reset();
        float process (float input, float pitchRatio, float sweepSamples);
    };

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    static float skewValue (float centre, float skew, bool invert);
    static float activeWeight (float pathCount, int pathIndex);
    static int parameterIndexForId (const juce::String& parameterId);
    static float normaliseParameterValue (int parameterIndex, float value);
    static float denormaliseParameterValue (int parameterIndex, float normalised);

    struct ModStage
    {
        float target = 0.0f;
        int samples = 1;
        bool smooth = true;
    };

    struct ParameterModulation
    {
        juce::String script;
        std::vector<ModStage> stages;
        bool loop = true;
        bool active = false;
        int stage = 0;
        int sample = 0;
        float start = 0.0f;
        float current = 0.0f;
        bool initialised = false;

        void setScript (const juce::String& newScript, double sampleRate);
        float process (float baseNormalised);
    };

    float processModulatedParameter (int parameterIndex, float baseValue);
    void applyPreset (int index);

    std::array<DelayPath, maxPaths> leftPaths;
    std::array<DelayPath, maxPaths> rightPaths;
    std::array<OnePole, maxPaths> leftReadFilters;
    std::array<OnePole, maxPaths> rightReadFilters;
    std::array<OnePole, maxPaths> leftDampers;
    std::array<OnePole, maxPaths> rightDampers;
    PitchDelayLine leftOctaveUp;
    PitchDelayLine rightOctaveUp;
    PitchDelayLine leftOctaveDown;
    PitchDelayLine rightOctaveDown;
    OnePole leftOctaveUpFilter;
    OnePole rightOctaveUpFilter;
    OnePole leftOctaveDownFilter;
    OnePole rightOctaveDownFilter;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> pathsSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> sizeSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> couplingSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> skewSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> feedbackSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> dampSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> octaveUpSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> octaveDownSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;

    std::array<ParameterModulation, 9> parameterModulations;
    std::array<std::atomic<float>, 9> currentModulatedValues;
    mutable juce::CriticalSection modulationLock;
    double currentSampleRate = 44100.0;
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ErbeyVerbyAudioProcessor)
};

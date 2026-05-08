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
    juce::String validateFabricScript (const juce::String& script) const;
    juce::String getFabricScriptForParameter (const juce::String& parameterId) const;
    void setFabricScriptDepth (const juce::String& parameterId, float depth);
    float getFabricScriptDepth (const juce::String& parameterId) const;
    void setFabricScriptActive (const juce::String& parameterId, bool active);
    bool isFabricScriptActive (const juce::String& parameterId) const;
    float getCurrentModulatedParameterValue (const juce::String& parameterId) const;
    bool parameterHasFabricScript (const juce::String& parameterId) const;
    float getVisualLevelLeft() const noexcept;
    float getVisualLevelRight() const noexcept;
    float getVisualStereoWidth() const noexcept;
    float getVisualLimiterReduction() const noexcept;

    juce::AudioProcessorValueTreeState parameters;

private:
    static constexpr int maxPaths = 32;
    static constexpr float maxDelaySeconds = 18.0f;

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

    struct HighPass
    {
        float low = 0.0f;
        float process (float input, float coefficient) noexcept
        {
            low += coefficient * (input - low);
            return input - low;
        }
        void reset() noexcept { low = 0.0f; }
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
    static float sizeDivisionToBeats (float division);
    static float softLimit (float sample, float& reduction) noexcept;
    static int parameterIndexForId (const juce::String& parameterId);
    static float normaliseParameterValue (int parameterIndex, float value);
    static float denormaliseParameterValue (int parameterIndex, float normalised);

    struct ModStage
    {
        enum class Type
        {
            ramp,
            hold,
            sine,
            random,
            wander
        };

        Type type = Type::ramp;
        float target = 0.0f;
        float minimum = 0.0f;
        float maximum = 1.0f;
        int samples = 1;
        bool smooth = true;
    };

    struct ParameterModulation
    {
        juce::String script;
        std::vector<ModStage> stages;
        bool loop = true;
        bool active = false;
        float depth = 1.0f;
        int stage = 0;
        int sample = 0;
        float start = 0.0f;
        float current = 0.0f;
        bool initialised = false;
        uint32_t randomState = 0x12345678u;

        juce::String setScript (const juce::String& newScript, double sampleRate);
        float process (float baseNormalised);

    private:
        float nextRandom();
    };

    float processModulatedParameter (int parameterIndex, float baseValue);
    void applyPreset (int index);

    std::array<DelayPath, maxPaths> leftPaths;
    std::array<DelayPath, maxPaths> rightPaths;
    std::array<OnePole, maxPaths> leftReadFilters;
    std::array<OnePole, maxPaths> rightReadFilters;
    std::array<OnePole, maxPaths> leftDampers;
    std::array<OnePole, maxPaths> rightDampers;
    std::array<HighPass, maxPaths> leftFeedbackHighPass;
    std::array<HighPass, maxPaths> rightFeedbackHighPass;
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
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> freezeSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> lowCutSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> airSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> sizeSyncSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> sizeDivisionSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputSmoothed;

    std::array<ParameterModulation, 15> parameterModulations;
    std::array<std::atomic<float>, 15> currentModulatedValues;
    std::atomic<float> visualLevelLeft { 0.0f };
    std::atomic<float> visualLevelRight { 0.0f };
    std::atomic<float> visualStereoWidth { 0.0f };
    std::atomic<float> visualLimiterReduction { 0.0f };
    mutable juce::CriticalSection modulationLock;
    double currentSampleRate = 44100.0;
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ErbeyVerbyAudioProcessor)
};

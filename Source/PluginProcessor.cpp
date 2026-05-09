#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr int numDelayPaths = 32;
constexpr std::array<const char*, 18> parameterIds {
    "paths", "size", "coupling", "skew", "feedback", "damping", "mix", "octaveUp", "octaveDown", "freeze", "lowCut", "air", "sizeSync", "sizeDivision", "output", "spread", "matrix", "earlyLate"
};
constexpr std::array<float, numDelayPaths> pathRatios {
    1.000f, 1.037f, 0.971f, 1.019f,
    0.947f, 1.061f, 0.983f, 1.029f,
    0.959f, 1.047f, 0.991f, 1.013f,
    0.937f, 1.071f, 0.977f, 1.033f,
    1.011f, 0.929f, 1.083f, 0.989f,
    1.052f, 0.963f, 1.024f, 0.941f,
    1.067f, 0.981f, 1.039f, 0.953f,
    1.091f, 0.973f, 1.017f, 0.933f
};
constexpr std::array<float, numDelayPaths> pathDisorder {
    -0.42f, 0.31f, -0.18f, 0.55f,
    -0.61f, 0.14f, 0.73f, -0.36f,
    0.23f, -0.78f, 0.48f, -0.09f,
    0.66f, -0.52f, 0.08f, -0.70f,
    0.39f, -0.24f, 0.82f, -0.47f,
    0.17f, 0.58f, -0.64f, 0.28f,
    -0.12f, 0.76f, -0.33f, 0.44f,
    -0.86f, 0.06f, 0.62f, -0.57f
};

struct Preset
{
    const char* name;
    std::array<float, 18> values;
    std::array<const char*, 18> scripts;
    std::array<bool, 18> active;
};

constexpr std::array<Preset, 12> presets {{
    {
        "Clockwork Bloom",
        { 18.0f, 0.52f, 0.70f, 0.24f, 0.66f, 0.34f, 0.70f, 0.18f, 0.00f, 0.0f, 0.26f, 0.62f, 1.0f, 8.0f, 0.86f, 0.34f, 0.42f, 0.56f },
        {
            "modulator paths\n  mode loop\n  stage 1 to 18% for 1s curve smooth\n  stage 2 to 100% for 2s curve smooth\n  stage 3 to 42% for 500ms curve linear\nend\n",
            "",
            "modulator coupling\n  mode loop\n  stage 1 sine 18% 92% for 4s\nend\n",
            "modulator skew\n  mode loop\n  stage 1 sine 20% 80% for 3s\nend\n",
            "modulator feedback\n  mode loop\n  stage 1 sine 48% 76% for 6s\nend\n",
            "", "", "", "", "",
            "modulator lowCut\n  mode loop\n  stage 1 sine 12% 45% for 8s\nend\n",
            "modulator air\n  mode loop\n  stage 1 sine 42% 82% for 5s\nend\n",
            "",
            "modulator sizeDivision\n  mode loop\n  stage 1 to 67% for 4s curve linear\n  stage 2 to 83% for 2s curve smooth\n  stage 3 to 50% for 1s curve linear\nend\n",
            "",
            "modulator spread\n  mode loop\n  stage 1 sine 10% 62% for 5s\nend\n"
        },
        { true, false, true, true, true, false, false, false, false, false, true, true, false, true, false, true }
    },
    {
        "Frozen Glass Plain",
        { 32.0f, 0.92f, 0.95f, -0.18f, 0.80f, 0.12f, 0.92f, 0.48f, 0.12f, 0.72f, 0.34f, 0.86f, 0.0f, 8.0f, 0.78f, 0.18f, 0.64f, 0.82f },
        {
            "modulator paths\n  mode loop\n  stage 1 to 100% for 9s curve smooth\n  stage 2 to 62% for 1s curve linear\nend\n",
            "modulator size\n  mode loop\n  stage 1 to 88% for 6s curve smooth\n  stage 2 to 100% for 6s curve smooth\nend\n",
            "modulator coupling\n  mode loop\n  stage 1 sine 62% 100% for 11s\nend\n",
            "modulator skew\n  mode loop\n  stage 1 sine 28% 72% for 13s\nend\n",
            "", "",
            "modulator mix\n  mode loop\n  stage 1 to 100% for 5s curve smooth\n  stage 2 to 64% for 900ms curve linear\nend\n",
            "modulator octaveUp\n  mode loop\n  stage 1 sine 8% 72% for 10s\nend\n",
            "",
            "modulator freeze\n  mode loop\n  stage 1 to 0% for 1s curve linear\n  stage 2 to 96% for 8s curve smooth\n  stage 3 hold for 3s\nend\n",
            "", "", "", "", ""
        },
        { true, true, true, true, false, false, true, true, false, true, false, false, false, false, false }
    },
    {
        "Dub Nebula",
        { 3.0f, 0.80f, 0.22f, -0.42f, 0.82f, 0.24f, 0.82f, 0.00f, 0.32f, 0.18f, 0.42f, 0.46f, 1.0f, 8.0f, 0.82f, 0.08f, 0.18f, 0.38f },
        {
            "modulator paths\n  mode loop\n  stage 1 to 0% for 3s curve smooth\n  stage 2 to 18% for 1s curve linear\nend\n",
            "",
            "modulator coupling\n  mode loop\n  stage 1 to 0% for 2s curve linear\n  stage 2 to 88% for 700ms curve smooth\n  stage 3 to 12% for 3s curve smooth\nend\n",
            "modulator skew\n  mode loop\n  stage 1 sine 8% 92% for 6s\nend\n",
            "modulator feedback\n  mode loop\n  stage 1 to 55% for 1s curve smooth\n  stage 2 to 86% for 2s curve smooth\n  stage 3 to 40% for 700ms curve linear\nend\n",
            "",
            "modulator mix\n  mode loop\n  stage 1 to 42% for 600ms curve linear\n  stage 2 to 96% for 1200ms curve smooth\n  stage 3 to 55% for 900ms curve smooth\nend\n",
            "", 
            "modulator octaveDown\n  mode loop\n  stage 1 sine 0% 58% for 5s\nend\n",
            "modulator freeze\n  mode loop\n  stage 1 hold for 2500ms\n  stage 2 to 48% for 400ms curve smooth\n  stage 3 to 0% for 1300ms curve smooth\nend\n",
            "", "", "", "", ""
        },
        { true, false, true, true, true, false, true, false, true, true, false, false, false, false, false }
    },
    {
        "Micro Swarm Room",
        { 32.0f, 0.035f, 0.86f, 0.00f, 0.62f, 0.72f, 0.74f, 0.24f, 0.18f, 0.0f, 0.78f, 0.76f, 0.0f, 8.0f, 0.88f, 0.86f, 0.82f, 0.28f },
        {
            "modulator paths\n  mode loop\n  stage 1 random 45% 100% for 140ms curve linear\nend\n",
            "modulator size\n  mode loop\n  stage 1 random 0% 12% for 220ms curve smooth\nend\n",
            "",
            "modulator skew\n  mode loop\n  stage 1 random 0% 100% for 420ms curve smooth\nend\n",
            "modulator feedback\n  mode loop\n  stage 1 random 30% 70% for 300ms curve smooth\nend\n",
            "modulator damping\n  mode loop\n  stage 1 sine 45% 92% for 3s\nend\n",
            "", 
            "modulator octaveUp\n  mode loop\n  stage 1 random 0% 45% for 520ms curve smooth\nend\n",
            "modulator octaveDown\n  mode loop\n  stage 1 random 0% 35% for 650ms curve linear\nend\n",
            "", 
            "modulator lowCut\n  mode loop\n  stage 1 random 45% 100% for 600ms curve smooth\nend\n",
            "modulator air\n  mode loop\n  stage 1 random 35% 100% for 400ms curve linear\nend\n",
            "", "", "",
            "modulator spread\n  mode loop\n  stage 1 random 45% 100% for 350ms curve smooth\nend\n"
        },
        { true, true, false, true, true, true, false, true, true, false, true, true, false, false, false, true }
    },
    {
        "Shimmer Freeze Gate",
        { 24.0f, 0.64f, 0.98f, 0.36f, 0.72f, 0.20f, 0.90f, 0.86f, 0.00f, 0.42f, 0.30f, 0.88f, 0.65f, 8.0f, 0.74f, 0.28f, 0.58f, 0.74f },
        {
            "modulator paths\n  mode loop\n  stage 1 to 50% for 1s curve smooth\n  stage 2 to 100% for 1s curve smooth\nend\n",
            "",
            "modulator coupling\n  mode loop\n  stage 1 sine 35% 100% for 3s\nend\n",
            "modulator skew\n  mode loop\n  stage 1 sine 10% 90% for 1500ms\nend\n",
            "", "",
            "modulator mix\n  mode loop\n  stage 1 sine 55% 100% for 4s\nend\n",
            "modulator octaveUp\n  mode loop\n  stage 1 sine 12% 100% for 2200ms\nend\n",
            "",
            "modulator freeze\n  mode loop\n  stage 1 to 0% for 800ms curve linear\n  stage 2 to 88% for 600ms curve smooth\n  stage 3 hold for 900ms\n  stage 4 to 5% for 700ms curve smooth\nend\n",
            "",
            "modulator air\n  mode loop\n  stage 1 sine 62% 100% for 5s\nend\n",
            "modulator sizeSync\n  mode loop\n  stage 1 sine 20% 100% for 8s\nend\n",
            "", ""
        },
        { true, false, true, true, false, false, true, true, false, true, false, true, true, false, false }
    },
    {
        "Subterranean Orbit",
        { 6.0f, 0.76f, 1.00f, -0.88f, 0.76f, 0.18f, 0.86f, 0.00f, 0.92f, 0.16f, 0.18f, 0.32f, 0.45f, 10.0f, 0.76f, 0.14f, 0.30f, 0.42f },
        {
            "modulator paths\n  mode loop\n  stage 1 to 8% for 1600ms curve linear\n  stage 2 to 45% for 4s curve smooth\nend\n",
            "modulator size\n  mode loop\n  stage 1 sine 45% 100% for 9s\nend\n",
            "modulator coupling\n  mode loop\n  stage 1 sine 70% 100% for 7s\nend\n",
            "modulator skew\n  mode loop\n  stage 1 sine 0% 100% for 5s\nend\n",
            "modulator feedback\n  mode loop\n  stage 1 sine 52% 82% for 8s\nend\n",
            "",
            "", "",
            "modulator octaveDown\n  mode loop\n  stage 1 sine 35% 100% for 6s\nend\n",
            "modulator freeze\n  mode loop\n  stage 1 random 0% 38% for 2s curve smooth\nend\n",
            "modulator lowCut\n  mode loop\n  stage 1 sine 4% 32% for 12s\nend\n",
            "", "", "", ""
        },
        { true, true, true, true, true, false, false, false, true, true, true, false, false, false, false }
    },
    {
        "Broken Clock Room",
        { 13.0f, 0.38f, 0.64f, 0.62f, 0.58f, 0.48f, 0.72f, 0.22f, 0.10f, 0.0f, 0.52f, 0.70f, 1.0f, 6.0f, 0.84f, 0.74f, 0.92f, 0.22f },
        {
            "modulator paths\n  mode loop\n  stage 1 random 8% 100% for 300ms curve linear\n  stage 2 hold for 200ms\nend\n",
            "",
            "modulator coupling\n  mode loop\n  stage 1 random 0% 100% for 900ms curve smooth\nend\n",
            "modulator skew\n  mode loop\n  stage 1 random 0% 100% for 700ms curve linear\nend\n",
            "modulator feedback\n  mode loop\n  stage 1 random 32% 78% for 800ms curve smooth\nend\n",
            "modulator damping\n  mode loop\n  stage 1 random 18% 88% for 1300ms curve smooth\nend\n",
            "modulator mix\n  mode loop\n  stage 1 random 48% 95% for 500ms curve smooth\nend\n",
            "", "",
            "",
            "modulator lowCut\n  mode loop\n  stage 1 random 15% 82% for 1s curve smooth\nend\n",
            "modulator air\n  mode loop\n  stage 1 random 20% 96% for 1200ms curve smooth\nend\n",
            "",
            "modulator sizeDivision\n  mode loop\n  stage 1 random 17% 92% for 700ms curve linear\nend\n",
            "",
            "modulator spread\n  mode loop\n  stage 1 random 0% 100% for 450ms curve linear\nend\n"
        },
        { true, false, true, true, true, true, true, false, false, false, true, true, false, true, false, true }
    },
    {
        "Cathedral Afterimage",
        { 32.0f, 1.0f, 1.0f, 0.08f, 0.86f, 0.08f, 0.95f, 0.62f, 0.28f, 0.58f, 0.38f, 0.52f, 0.15f, 12.0f, 0.72f, 0.22f, 0.70f, 0.90f },
        {
            "modulator paths\n  mode loop\n  stage 1 to 100% for 12s curve smooth\n  stage 2 to 72% for 1s curve linear\nend\n",
            "modulator size\n  mode loop\n  stage 1 to 100% for 14s curve smooth\n  stage 2 to 72% for 2s curve smooth\nend\n",
            "modulator coupling\n  mode loop\n  stage 1 sine 65% 100% for 13s\nend\n",
            "modulator skew\n  mode loop\n  stage 1 sine 40% 60% for 17s\nend\n",
            "modulator feedback\n  mode loop\n  stage 1 sine 68% 88% for 19s\nend\n",
            "modulator damping\n  mode loop\n  stage 1 sine 0% 35% for 15s\nend\n",
            "",
            "modulator octaveUp\n  mode loop\n  stage 1 sine 0% 72% for 16s\nend\n",
            "",
            "modulator freeze\n  mode loop\n  stage 1 to 0% for 2s curve smooth\n  stage 2 to 85% for 9s curve smooth\n  stage 3 hold for 5s\nend\n",
            "", "", "", "", ""
        },
        { true, true, true, true, true, true, false, true, false, true, false, false, false, false, false }
    },
    {
        "Stereo Knife Weather",
        { 20.0f, 0.58f, 0.72f, 1.00f, 0.70f, 0.30f, 0.84f, 0.16f, 0.22f, 0.0f, 0.50f, 0.72f, 0.35f, 7.0f, 0.82f, 0.58f, 0.88f, 0.48f },
        {
            "modulator paths\n  mode loop\n  stage 1 sine 30% 100% for 5s\nend\n",
            "modulator size\n  mode loop\n  stage 1 wander 35% 80% for 1200ms curve smooth\nend\n",
            "modulator coupling\n  mode loop\n  stage 1 sine 0% 100% for 2200ms\nend\n",
            "modulator skew\n  mode loop\n  stage 1 sine 0% 100% for 1400ms\nend\n",
            "",
            "modulator damping\n  mode loop\n  stage 1 sine 12% 78% for 3s\nend\n",
            "modulator mix\n  mode loop\n  stage 1 sine 45% 100% for 6s\nend\n",
            "modulator octaveUp\n  mode loop\n  stage 1 random 0% 38% for 1500ms curve smooth\nend\n",
            "modulator octaveDown\n  mode loop\n  stage 1 random 0% 42% for 1700ms curve smooth\nend\n",
            "",
            "", "", "", "", "",
            "modulator spread\n  mode loop\n  stage 1 sine 12% 92% for 3s\nend\n"
        },
        { true, true, true, true, false, true, true, true, true, false, false, false, false, false, false, true }
    },
    {
        "Bass Safe Furnace",
        { 11.0f, 0.50f, 1.0f, 0.16f, 0.72f, 0.22f, 1.0f, 1.0f, 0.88f, 0.0f, 0.88f, 0.44f, 0.0f, 8.0f, 0.68f, 0.46f, 0.52f, 0.34f },
        {
            "modulator paths\n  mode loop\n  stage 1 random 20% 100% for 900ms curve smooth\nend\n",
            "modulator size\n  mode loop\n  stage 1 sine 8% 70% for 4s\nend\n",
            "modulator coupling\n  mode loop\n  stage 1 sine 30% 100% for 5s\nend\n",
            "modulator skew\n  mode loop\n  stage 1 random 15% 85% for 1300ms curve smooth\nend\n",
            "modulator feedback\n  mode loop\n  stage 1 random 28% 76% for 900ms curve smooth\nend\n",
            "modulator damping\n  mode loop\n  stage 1 sine 8% 70% for 3200ms\nend\n",
            "",
            "modulator octaveUp\n  mode loop\n  stage 1 sine 0% 100% for 1400ms\nend\n",
            "modulator octaveDown\n  mode loop\n  stage 1 sine 15% 100% for 1900ms\nend\n",
            "",
            "modulator lowCut\n  mode loop\n  stage 1 sine 62% 100% for 5s\nend\n",
            "modulator air\n  mode loop\n  stage 1 sine 18% 72% for 6s\nend\n",
            "", "", ""
        },
        { true, true, true, true, true, true, false, true, true, false, true, true, false, false, false }
    },
    {
        "Granular Tide Pool",
        { 29.0f, 0.16f, 0.58f, -0.30f, 0.64f, 0.62f, 0.86f, 0.38f, 0.28f, 0.12f, 0.72f, 0.82f, 0.0f, 8.0f, 0.84f, 0.92f, 0.96f, 0.20f },
        {
            "modulator paths\n  mode loop\n  stage 1 wander 30% 100% for 180ms curve linear\nend\n",
            "modulator size\n  mode loop\n  stage 1 wander 0% 42% for 240ms curve smooth\nend\n",
            "modulator coupling\n  mode loop\n  stage 1 random 0% 100% for 500ms curve smooth\nend\n",
            "modulator skew\n  mode loop\n  stage 1 random 0% 100% for 360ms curve linear\nend\n",
            "modulator feedback\n  mode loop\n  stage 1 wander 30% 78% for 500ms curve smooth\nend\n",
            "modulator damping\n  mode loop\n  stage 1 random 35% 95% for 700ms curve smooth\nend\n",
            "modulator mix\n  mode loop\n  stage 1 random 60% 100% for 380ms curve smooth\nend\n",
            "modulator octaveUp\n  mode loop\n  stage 1 random 0% 76% for 600ms curve smooth\nend\n",
            "modulator octaveDown\n  mode loop\n  stage 1 random 0% 62% for 750ms curve smooth\nend\n",
            "modulator freeze\n  mode loop\n  stage 1 random 0% 28% for 900ms curve smooth\nend\n",
            "modulator lowCut\n  mode loop\n  stage 1 random 40% 100% for 1s curve smooth\nend\n",
            "modulator air\n  mode loop\n  stage 1 random 25% 100% for 900ms curve smooth\nend\n",
            "", "", "",
            "modulator spread\n  mode loop\n  stage 1 wander 20% 100% for 280ms curve smooth\nend\n"
        },
        { true, true, true, true, true, true, true, true, true, true, true, true, false, false, false, true }
    },
    {
        "Manual Wide Hall",
        { 18.0f, 0.70f, 0.72f, 0.00f, 0.62f, 0.32f, 0.58f, 0.00f, 0.00f, 0.0f, 0.34f, 0.66f, 0.0f, 8.0f, 0.92f, 0.24f, 0.36f, 0.62f },
        {
            "",
            "",
            "modulator coupling\n  mode loop\n  stage 1 sine 35% 92% for 9s\nend\n",
            "modulator skew\n  mode loop\n  stage 1 sine 35% 65% for 12s\nend\n",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "modulator air\n  mode loop\n  stage 1 sine 50% 80% for 10s\nend\n",
            "", "", ""
        },
        { false, false, true, true, false, false, false, false, false, false, false, true, false, false, false }
    }
}};

float parseTimeToSeconds (const juce::String& token)
{
    const auto text = token.trim().toLowerCase();
    if (text.endsWith ("ms"))
        return text.dropLastCharacters (2).getFloatValue() * 0.001f;
    if (text.endsWithChar ('s'))
        return text.dropLastCharacters (1).getFloatValue();

    return text.getFloatValue();
}

float parseLevel (const juce::String& token)
{
    const auto text = token.trim();
    if (text.endsWithChar ('%'))
        return juce::jlimit (0.0f, 1.0f, text.dropLastCharacters (1).getFloatValue() * 0.01f);

    return juce::jlimit (0.0f, 1.0f, text.getFloatValue());
}

bool looksLikeNumberOrPercent (const juce::String& token)
{
    const auto text = token.trim();
    if (text.isEmpty())
        return false;

    const auto numeric = text.endsWithChar ('%') ? text.dropLastCharacters (1) : text;
    return numeric.containsOnly ("0123456789.+-");
}
}

void ErbeyVerbyAudioProcessor::DelayPath::prepare (int size)
{
    buffer.assign ((size_t) size, 0.0f);
    writePosition = 0;
}

void ErbeyVerbyAudioProcessor::DelayPath::clear()
{
    std::fill (buffer.begin(), buffer.end(), 0.0f);
    writePosition = 0;
}

float ErbeyVerbyAudioProcessor::DelayPath::read (float delaySamples) const
{
    if (buffer.empty())
        return 0.0f;

    const auto size = (int) buffer.size();
    auto readPosition = (float) writePosition - delaySamples;

    while (readPosition < 0.0f)
        readPosition += (float) size;

    const auto indexA = ((int) readPosition) % size;
    const auto indexB = (indexA + 1) % size;
    const auto fraction = readPosition - std::floor (readPosition);

    return juce::jmap (fraction, buffer[(size_t) indexA], buffer[(size_t) indexB]);
}

void ErbeyVerbyAudioProcessor::DelayPath::write (float sample)
{
    if (buffer.empty())
        return;

    buffer[(size_t) writePosition] = sample;
    writePosition = (writePosition + 1) % (int) buffer.size();
}

void ErbeyVerbyAudioProcessor::PitchDelayLine::prepare (int size)
{
    delay.prepare (size);
    phase = 0.0f;
}

void ErbeyVerbyAudioProcessor::PitchDelayLine::reset()
{
    delay.clear();
    phase = 0.0f;
}

float ErbeyVerbyAudioProcessor::PitchDelayLine::process (float input, float pitchRatio, float sweepSamples)
{
    const auto sweep = juce::jmax (32.0f, sweepSamples);
    const auto phaseB = std::fmod (phase + 0.5f, 1.0f);
    const auto rising = pitchRatio < 1.0f;
    const auto delayA = rising ? phase * sweep : (1.0f - phase) * sweep;
    const auto delayB = rising ? phaseB * sweep : (1.0f - phaseB) * sweep;
    const auto windowA = std::sin (phase * juce::MathConstants<float>::pi);
    const auto windowB = std::sin (phaseB * juce::MathConstants<float>::pi);
    const auto normaliser = 1.0f / juce::jmax (0.001f, windowA + windowB);
    const auto shifted = (delay.read (delayA + 64.0f) * windowA + delay.read (delayB + 64.0f) * windowB) * normaliser;

    delay.write (input);
    phase += std::abs (1.0f - pitchRatio) / sweep;
    if (phase >= 1.0f)
        phase -= std::floor (phase);

    return shifted;
}

juce::String ErbeyVerbyAudioProcessor::ParameterModulation::setScript (const juce::String& newScript, double sampleRate)
{
    script = newScript;
    stages.clear();
    loop = true;
    active = false;
    stage = 0;
    sample = 0;
    initialised = false;

    juce::StringArray lines;
    lines.addLines (newScript);

    for (int lineIndex = 0; lineIndex < lines.size(); ++lineIndex)
    {
        auto line = lines[lineIndex].upToFirstOccurrenceOf ("#", false, false).trim();
        if (line.isEmpty())
            continue;

        juce::StringArray tokens;
        tokens.addTokens (line, " \t", "\"'");
        tokens.removeEmptyStrings();

        if (tokens.size() == 0)
            continue;

        const auto lineError = [&] (const juce::String& message)
        {
            return "Line " + juce::String (lineIndex + 1) + ": " + message;
        };

        if (tokens[0].equalsIgnoreCase ("modulator") || tokens[0].equalsIgnoreCase ("end"))
            continue;

        if (tokens[0].equalsIgnoreCase ("mode") && tokens.size() > 1)
        {
            loop = ! tokens[1].equalsIgnoreCase ("one_shot");
            continue;
        }

        const auto isStageLine = tokens[0].equalsIgnoreCase ("stage");
        const auto commandIndex = isStageLine ? 2 : 0;
        if (isStageLine && tokens.size() < 3)
            return lineError ("stage needs a command, for example 'stage 1 to 80% for 1s'.");

        if (commandIndex >= tokens.size())
            return lineError ("missing command.");

        const auto command = tokens[commandIndex].toLowerCase();
        if (! (command == "to" || command == "hold" || command == "random" || command == "sine" || command == "wander"))
            return lineError ("unknown command '" + tokens[commandIndex] + "'. Use to, hold, random, sine, or wander.");

        ModStage next;
        next.target = stages.empty() ? 1.0f : stages.back().target;
        next.minimum = 0.0f;
        next.maximum = 1.0f;
        next.samples = (int) std::round (0.25 * sampleRate);
        next.smooth = true;
        int optionStart = commandIndex + 1;

        if (command == "to")
        {
            if (optionStart >= tokens.size())
                return lineError ("'to' needs a target value.");

            if (tokens[optionStart].equalsIgnoreCase ("random"))
            {
                if (optionStart + 2 >= tokens.size())
                    return lineError ("'to random' needs minimum and maximum values.");

                next.type = ModStage::Type::random;
                next.minimum = parseLevel (tokens[optionStart + 1]);
                next.maximum = parseLevel (tokens[optionStart + 2]);
                optionStart += 3;
            }
            else
            {
                if (! looksLikeNumberOrPercent (tokens[optionStart]))
                    return lineError ("target value must be 0..1 or a percentage.");

                next.type = ModStage::Type::ramp;
                next.target = parseLevel (tokens[optionStart]);
                optionStart += 1;
            }
        }
        else if (command == "hold")
        {
            next.type = ModStage::Type::hold;
        }
        else
        {
            if (optionStart + 1 >= tokens.size())
                return lineError ("'" + command + "' needs minimum and maximum values.");

            if (! looksLikeNumberOrPercent (tokens[optionStart]) || ! looksLikeNumberOrPercent (tokens[optionStart + 1]))
                return lineError ("'" + command + "' values must be 0..1 or percentages.");

            next.type = command == "random" ? ModStage::Type::random
                      : command == "sine"   ? ModStage::Type::sine
                                            : ModStage::Type::wander;
            next.minimum = parseLevel (tokens[optionStart]);
            next.maximum = parseLevel (tokens[optionStart + 1]);
            optionStart += 2;
        }

        if (next.minimum > next.maximum)
            std::swap (next.minimum, next.maximum);

        for (int i = optionStart; i < tokens.size(); ++i)
        {
            if ((tokens[i].equalsIgnoreCase ("for") || tokens[i].equalsIgnoreCase ("in")) && i + 1 < tokens.size())
                next.samples = juce::jmax (1, (int) std::round (parseTimeToSeconds (tokens[++i]) * sampleRate));
            else if (tokens[i].equalsIgnoreCase ("curve") && i + 1 < tokens.size())
                next.smooth = ! tokens[++i].equalsIgnoreCase ("linear");
            else
                return lineError ("unexpected token '" + tokens[i] + "'.");
        }

        stages.push_back (next);
    }

    active = ! stages.empty();
    return {};
}

float ErbeyVerbyAudioProcessor::ParameterModulation::nextRandom()
{
    randomState = randomState * 1664525u + 1013904223u;
    return (float) ((randomState >> 8) & 0x00ffffffu) / (float) 0x00ffffffu;
}

float ErbeyVerbyAudioProcessor::ParameterModulation::process (float baseNormalised)
{
    if (stages.empty() || ! active)
        return baseNormalised;

    if (! initialised)
    {
        current = baseNormalised;
        start = baseNormalised;
        initialised = true;
    }

    auto& currentStage = stages[(size_t) stage];
    if (sample == 0)
    {
        if (currentStage.type == ModStage::Type::random || currentStage.type == ModStage::Type::wander)
            currentStage.target = juce::jmap (nextRandom(), currentStage.minimum, currentStage.maximum);
        else if (currentStage.type == ModStage::Type::hold)
            currentStage.target = start;
    }

    const auto progress = juce::jlimit (0.0f, 1.0f, (float) sample / (float) juce::jmax (1, currentStage.samples));
    const auto shaped = currentStage.smooth ? progress * progress * (3.0f - 2.0f * progress) : progress;

    if (currentStage.type == ModStage::Type::sine)
    {
        const auto phase = progress * juce::MathConstants<float>::twoPi;
        const auto wave = 0.5f + 0.5f * std::sin (phase - juce::MathConstants<float>::halfPi);
        current = juce::jmap (wave, currentStage.minimum, currentStage.maximum);
    }
    else
    {
        current = start + (currentStage.target - start) * shaped;
    }

    if (++sample >= currentStage.samples)
    {
        current = currentStage.target;
        sample = 0;
        start = current;

        if (++stage >= (int) stages.size())
        {
            if (loop)
                stage = 0;
            else
                stage = (int) stages.size() - 1;
        }
    }

    const auto modulated = juce::jlimit (0.0f, 1.0f, current);
    return juce::jlimit (0.0f, 1.0f, baseNormalised + (modulated - baseNormalised) * depth);
}

ErbeyVerbyAudioProcessor::ErbeyVerbyAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    for (auto& value : currentModulatedValues)
        value.store (0.0f);
}

juce::AudioProcessorValueTreeState::ParameterLayout ErbeyVerbyAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("paths", "Paths",
        juce::NormalisableRange<float> (1.0f, (float) maxPaths, 0.001f), 5.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("size", "Size",
        juce::NormalisableRange<float> (0.03f, 1.0f, 0.0001f, 0.45f), 0.32f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("coupling", "Coupling",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.45f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("skew", "Skew",
        juce::NormalisableRange<float> (-1.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("feedback", "Feedback",
        juce::NormalisableRange<float> (0.0f, 0.96f, 0.001f), 0.62f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("damping", "Damping",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.38f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("octaveUp", "Octave Up",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("octaveDown", "Octave Down",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("mix", "Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.35f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("freeze", "Freeze",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lowCut", "Low Cut",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.20f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("air", "Air",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.55f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("sizeSync", "Sync",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("sizeDivision", "Div",
        juce::NormalisableRange<float> (0.0f, 12.0f, 1.0f), 8.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("output", "Output",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("spread", "Spread",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.28f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("matrix", "Matrix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.42f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("earlyLate", "Early/Late",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.55f));

    return { params.begin(), params.end() };
}

bool ErbeyVerbyAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    return mainOut == juce::AudioChannelSet::mono() || mainOut == juce::AudioChannelSet::stereo();
}

int ErbeyVerbyAudioProcessor::getNumPrograms()
{
    return (int) presets.size();
}

int ErbeyVerbyAudioProcessor::getCurrentProgram()
{
    return currentProgram;
}

void ErbeyVerbyAudioProcessor::setCurrentProgram (int index)
{
    applyPreset (juce::jlimit (0, getNumPrograms() - 1, index));
}

const juce::String ErbeyVerbyAudioProcessor::getProgramName (int index)
{
    if (! juce::isPositiveAndBelow (index, (int) presets.size()))
        return {};

    return presets[(size_t) index].name;
}

void ErbeyVerbyAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    const auto bufferSize = (int) std::ceil (sampleRate * maxDelaySeconds) + 8;

    for (auto& path : leftPaths)  path.prepare (bufferSize);
    for (auto& path : rightPaths) path.prepare (bufferSize);
    for (auto& filter : leftReadFilters)  filter.reset();
    for (auto& filter : rightReadFilters) filter.reset();
    for (auto& damper : leftDampers)  damper.reset();
    for (auto& damper : rightDampers) damper.reset();
    for (auto& filter : leftFeedbackHighPass)  filter.reset();
    for (auto& filter : rightFeedbackHighPass) filter.reset();
    for (auto* pitchDelay : { &leftOctaveUp, &rightOctaveUp, &leftOctaveDown, &rightOctaveDown })
    {
        pitchDelay->prepare (bufferSize);
        pitchDelay->reset();
    }
    for (auto* filter : { &leftOctaveUpFilter, &rightOctaveUpFilter, &leftOctaveDownFilter, &rightOctaveDownFilter })
        filter->reset();

    for (auto* smoother : { &pathsSmoothed, &sizeSmoothed, &couplingSmoothed, &skewSmoothed,
                            &feedbackSmoothed, &dampSmoothed, &octaveUpSmoothed, &octaveDownSmoothed, &mixSmoothed,
                            &freezeSmoothed, &lowCutSmoothed, &airSmoothed, &sizeSyncSmoothed, &sizeDivisionSmoothed,
                            &outputSmoothed, &spreadSmoothed, &matrixSmoothed, &earlyLateSmoothed })
        smoother->reset (sampleRate, 0.035);

    pathsSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("paths"));
    sizeSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("size"));
    couplingSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("coupling"));
    skewSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("skew"));
    feedbackSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("feedback"));
    dampSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("damping"));
    octaveUpSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("octaveUp"));
    octaveDownSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("octaveDown"));
    mixSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("mix"));
    freezeSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("freeze"));
    lowCutSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("lowCut"));
    airSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("air"));
    sizeSyncSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("sizeSync"));
    sizeDivisionSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("sizeDivision"));
    outputSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("output"));
    spreadSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("spread"));
    matrixSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("matrix"));
    earlyLateSmoothed.setCurrentAndTargetValue (*parameters.getRawParameterValue ("earlyLate"));
    visualLevelLeft.store (0.0f, std::memory_order_relaxed);
    visualLevelRight.store (0.0f, std::memory_order_relaxed);
    visualStereoWidth.store (0.0f, std::memory_order_relaxed);
    visualLimiterReduction.store (0.0f, std::memory_order_relaxed);
}

float ErbeyVerbyAudioProcessor::skewValue (float centre, float skew, bool invert)
{
    const auto signedSkew = invert ? -skew : skew;
    return juce::jlimit (0.02f, 1.25f, centre * std::pow (2.0f, signedSkew));
}

float ErbeyVerbyAudioProcessor::activeWeight (float pathCount, int pathIndex)
{
    const auto oneBased = (float) pathIndex + 1.0f;
    const auto linear = juce::jlimit (0.0f, 1.0f, pathCount - oneBased + 1.0f);
    return 0.5f - 0.5f * std::cos (linear * juce::MathConstants<float>::pi);
}

float ErbeyVerbyAudioProcessor::sizeDivisionToBeats (float division)
{
    static constexpr std::array<float, 13> divisions {
        0.0625f, 0.083333f, 0.125f, 0.166667f, 0.25f, 0.333333f, 0.5f,
        0.666667f, 1.0f, 1.5f, 2.0f, 3.0f, 4.0f
    };

    const auto index = juce::jlimit (0, (int) divisions.size() - 1, (int) std::round (division));
    return divisions[(size_t) index];
}

float ErbeyVerbyAudioProcessor::softLimit (float sample, float& reduction) noexcept
{
    const auto sign = sample < 0.0f ? -1.0f : 1.0f;
    const auto magnitude = std::abs (sample);
    if (magnitude <= 0.92f)
        return sample;

    const auto limitedMagnitude = 0.92f + (1.0f - std::exp ((0.92f - magnitude) * 4.0f)) * 0.075f;
    const auto limited = sign * juce::jmin (0.995f, limitedMagnitude);
    reduction = juce::jmax (reduction, magnitude - std::abs (limited));
    return limited;
}

int ErbeyVerbyAudioProcessor::parameterIndexForId (const juce::String& parameterId)
{
    for (int i = 0; i < (int) parameterIds.size(); ++i)
        if (parameterId == parameterIds[(size_t) i])
            return i;

    return -1;
}

float ErbeyVerbyAudioProcessor::normaliseParameterValue (int parameterIndex, float value)
{
    switch (parameterIndex)
    {
        case 0: return juce::jmap (value, 1.0f, (float) maxPaths, 0.0f, 1.0f);
        case 1: return juce::jmap (value, 0.03f, 1.0f, 0.0f, 1.0f);
        case 4: return juce::jmap (value, 0.0f, 0.96f, 0.0f, 1.0f);
        case 3: return juce::jmap (value, -1.0f, 1.0f, 0.0f, 1.0f);
        case 13: return juce::jmap (value, 0.0f, 12.0f, 0.0f, 1.0f);
        default: return juce::jlimit (0.0f, 1.0f, value);
    }
}

float ErbeyVerbyAudioProcessor::denormaliseParameterValue (int parameterIndex, float normalised)
{
    const auto value = juce::jlimit (0.0f, 1.0f, normalised);

    switch (parameterIndex)
    {
        case 0: return juce::jmap (value, 1.0f, (float) maxPaths);
        case 1: return juce::jmap (value, 0.03f, 1.0f);
        case 4: return juce::jmap (value, 0.0f, 0.96f);
        case 3: return juce::jmap (value, -1.0f, 1.0f);
        case 13: return std::round (juce::jmap (value, 0.0f, 12.0f));
        default: return value;
    }
}

float ErbeyVerbyAudioProcessor::processModulatedParameter (int parameterIndex, float baseValue)
{
    const juce::ScopedLock lock (modulationLock);
    if (! juce::isPositiveAndBelow (parameterIndex, (int) parameterModulations.size()))
        return baseValue;

    const auto baseNormalised = normaliseParameterValue (parameterIndex, baseValue);
    const auto modulatedValue = denormaliseParameterValue (parameterIndex, parameterModulations[(size_t) parameterIndex].process (baseNormalised));
    currentModulatedValues[(size_t) parameterIndex].store (modulatedValue, std::memory_order_relaxed);
    return modulatedValue;
}

void ErbeyVerbyAudioProcessor::applyPreset (int index)
{
    if (! juce::isPositiveAndBelow (index, (int) presets.size()))
        return;

    currentProgram = index;
    const auto& preset = presets[(size_t) index];

    for (int i = 0; i < (int) parameterIds.size(); ++i)
        if (auto* parameter = parameters.getParameter (parameterIds[(size_t) i]))
        {
            const auto normalisedValue = normaliseParameterValue (i, preset.values[(size_t) i]);
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost (normalisedValue);
            parameter->endChangeGesture();
            currentModulatedValues[(size_t) i].store (preset.values[(size_t) i], std::memory_order_relaxed);
        }

    {
        const juce::ScopedLock lock (modulationLock);
        for (int i = 0; i < (int) parameterIds.size(); ++i)
        {
            auto& modulation = parameterModulations[(size_t) i];
            modulation.setScript (preset.scripts[(size_t) i] == nullptr ? "" : preset.scripts[(size_t) i], currentSampleRate);
            modulation.depth = 1.0f;
            modulation.active = preset.active[(size_t) i] && ! modulation.stages.empty();
        }
    }
}

void ErbeyVerbyAudioProcessor::setFabricScriptForParameter (const juce::String& parameterId, const juce::String& script)
{
    const auto index = parameterIndexForId (parameterId);
    if (! juce::isPositiveAndBelow (index, (int) parameterModulations.size()))
        return;

    ParameterModulation parsed;
    if (parsed.setScript (script, currentSampleRate).isNotEmpty())
        return;

    const juce::ScopedLock lock (modulationLock);
    parsed.depth = parameterModulations[(size_t) index].depth;
    parameterModulations[(size_t) index] = std::move (parsed);
}

juce::String ErbeyVerbyAudioProcessor::validateFabricScript (const juce::String& script) const
{
    ParameterModulation parsed;
    return parsed.setScript (script, currentSampleRate);
}

juce::String ErbeyVerbyAudioProcessor::getFabricScriptForParameter (const juce::String& parameterId) const
{
    const auto index = parameterIndexForId (parameterId);
    if (! juce::isPositiveAndBelow (index, (int) parameterModulations.size()))
        return {};

    const juce::ScopedLock lock (modulationLock);
    return parameterModulations[(size_t) index].script;
}

void ErbeyVerbyAudioProcessor::setFabricScriptDepth (const juce::String& parameterId, float depth)
{
    const auto index = parameterIndexForId (parameterId);
    if (! juce::isPositiveAndBelow (index, (int) parameterModulations.size()))
        return;

    const juce::ScopedLock lock (modulationLock);
    parameterModulations[(size_t) index].depth = juce::jlimit (0.0f, 1.0f, depth);
}

float ErbeyVerbyAudioProcessor::getFabricScriptDepth (const juce::String& parameterId) const
{
    const auto index = parameterIndexForId (parameterId);
    if (! juce::isPositiveAndBelow (index, (int) parameterModulations.size()))
        return 1.0f;

    const juce::ScopedLock lock (modulationLock);
    return parameterModulations[(size_t) index].depth;
}

void ErbeyVerbyAudioProcessor::setFabricScriptActive (const juce::String& parameterId, bool active)
{
    const auto index = parameterIndexForId (parameterId);
    if (! juce::isPositiveAndBelow (index, (int) parameterModulations.size()))
        return;

    const juce::ScopedLock lock (modulationLock);
    auto& modulation = parameterModulations[(size_t) index];
    modulation.active = active && ! modulation.stages.empty();
    if (modulation.active)
    {
        modulation.stage = 0;
        modulation.sample = 0;
        modulation.initialised = false;
    }
}

bool ErbeyVerbyAudioProcessor::isFabricScriptActive (const juce::String& parameterId) const
{
    const auto index = parameterIndexForId (parameterId);
    if (! juce::isPositiveAndBelow (index, (int) parameterModulations.size()))
        return false;

    const juce::ScopedLock lock (modulationLock);
    return parameterModulations[(size_t) index].active && ! parameterModulations[(size_t) index].stages.empty();
}

float ErbeyVerbyAudioProcessor::getCurrentModulatedParameterValue (const juce::String& parameterId) const
{
    const auto index = parameterIndexForId (parameterId);
    if (! juce::isPositiveAndBelow (index, (int) currentModulatedValues.size()))
        return 0.0f;

    return currentModulatedValues[(size_t) index].load (std::memory_order_relaxed);
}

bool ErbeyVerbyAudioProcessor::parameterHasFabricScript (const juce::String& parameterId) const
{
    const auto index = parameterIndexForId (parameterId);
    if (! juce::isPositiveAndBelow (index, (int) parameterModulations.size()))
        return false;

    const juce::ScopedLock lock (modulationLock);
    return ! parameterModulations[(size_t) index].stages.empty();
}

float ErbeyVerbyAudioProcessor::getVisualLevelLeft() const noexcept
{
    return visualLevelLeft.load (std::memory_order_relaxed);
}

float ErbeyVerbyAudioProcessor::getVisualLevelRight() const noexcept
{
    return visualLevelRight.load (std::memory_order_relaxed);
}

float ErbeyVerbyAudioProcessor::getVisualStereoWidth() const noexcept
{
    return visualStereoWidth.load (std::memory_order_relaxed);
}

float ErbeyVerbyAudioProcessor::getVisualLimiterReduction() const noexcept
{
    return visualLimiterReduction.load (std::memory_order_relaxed);
}

void ErbeyVerbyAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalInputChannels = getTotalNumInputChannels();
    const auto totalOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalInputChannels; i < totalOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    pathsSmoothed.setTargetValue (*parameters.getRawParameterValue ("paths"));
    sizeSmoothed.setTargetValue (*parameters.getRawParameterValue ("size"));
    couplingSmoothed.setTargetValue (*parameters.getRawParameterValue ("coupling"));
    skewSmoothed.setTargetValue (*parameters.getRawParameterValue ("skew"));
    feedbackSmoothed.setTargetValue (*parameters.getRawParameterValue ("feedback"));
    dampSmoothed.setTargetValue (*parameters.getRawParameterValue ("damping"));
    octaveUpSmoothed.setTargetValue (*parameters.getRawParameterValue ("octaveUp"));
    octaveDownSmoothed.setTargetValue (*parameters.getRawParameterValue ("octaveDown"));
    mixSmoothed.setTargetValue (*parameters.getRawParameterValue ("mix"));
    freezeSmoothed.setTargetValue (*parameters.getRawParameterValue ("freeze"));
    lowCutSmoothed.setTargetValue (*parameters.getRawParameterValue ("lowCut"));
    airSmoothed.setTargetValue (*parameters.getRawParameterValue ("air"));
    sizeSyncSmoothed.setTargetValue (*parameters.getRawParameterValue ("sizeSync"));
    sizeDivisionSmoothed.setTargetValue (*parameters.getRawParameterValue ("sizeDivision"));
    outputSmoothed.setTargetValue (*parameters.getRawParameterValue ("output"));
    spreadSmoothed.setTargetValue (*parameters.getRawParameterValue ("spread"));
    matrixSmoothed.setTargetValue (*parameters.getRawParameterValue ("matrix"));
    earlyLateSmoothed.setTargetValue (*parameters.getRawParameterValue ("earlyLate"));

    auto* left = buffer.getWritePointer (0);
    auto* right = totalOutputChannels > 1 ? buffer.getWritePointer (1) : left;
    const auto hasStereoInput = totalInputChannels > 1;
    auto hostBpm = 120.0f;
    if (auto* hostPlayHead = getPlayHead())
        if (auto position = hostPlayHead->getPosition())
            if (auto bpm = position->getBpm())
                hostBpm = juce::jlimit (20.0f, 320.0f, (float) *bpm);
    const auto beatSeconds = 60.0f / hostBpm;
    auto energyL = 0.0f;
    auto energyR = 0.0f;
    auto midEnergy = 0.0f;
    auto sideEnergy = 0.0f;
    auto limiterReduction = 0.0f;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto pathCount = processModulatedParameter (0, pathsSmoothed.getNextValue());
        const auto size = processModulatedParameter (1, sizeSmoothed.getNextValue());
        const auto coupling = processModulatedParameter (2, couplingSmoothed.getNextValue());
        const auto skew = processModulatedParameter (3, skewSmoothed.getNextValue());
        const auto feedback = processModulatedParameter (4, feedbackSmoothed.getNextValue());
        const auto damping = processModulatedParameter (5, dampSmoothed.getNextValue());
        const auto mix = processModulatedParameter (6, mixSmoothed.getNextValue());
        const auto octaveUp = processModulatedParameter (7, octaveUpSmoothed.getNextValue());
        const auto octaveDown = processModulatedParameter (8, octaveDownSmoothed.getNextValue());
        const auto freeze = processModulatedParameter (9, freezeSmoothed.getNextValue());
        const auto lowCut = processModulatedParameter (10, lowCutSmoothed.getNextValue());
        const auto air = processModulatedParameter (11, airSmoothed.getNextValue());
        const auto sizeSync = processModulatedParameter (12, sizeSyncSmoothed.getNextValue());
        const auto sizeDivision = processModulatedParameter (13, sizeDivisionSmoothed.getNextValue());
        const auto output = processModulatedParameter (14, outputSmoothed.getNextValue());
        const auto spread = processModulatedParameter (15, spreadSmoothed.getNextValue());
        const auto matrix = processModulatedParameter (16, matrixSmoothed.getNextValue());
        const auto earlyLate = processModulatedParameter (17, earlyLateSmoothed.getNextValue());
        const auto activePaths = juce::jmax (1.0f, pathCount);
        const auto sizeNormalised = juce::jlimit (0.0f, 1.0f, (size - 0.03f) / 0.97f);
        const auto smallSpaceDensity = 1.0f - sizeNormalised;
        const auto diffusion = juce::jlimit (0.0f, 1.0f,
            juce::jmax ((activePaths - 1.0f) / 9.0f, 0.42f * smallSpaceDensity * smallSpaceDensity));
        const auto freezeShape = freeze * freeze;
        const auto inputCapture = 1.0f - freezeShape;
        const auto freezeFeedback = 0.955f + 0.03f * sizeNormalised;
        const auto effectiveFeedback = juce::jmap (freezeShape,
            juce::jlimit (0.0f, 0.88f, feedback * juce::jmap (diffusion, 0.92f, 0.72f)),
            freezeFeedback);

        const auto dryL = left[sample];
        const auto dryR = hasStereoInput ? right[sample] : dryL;

        const auto leftSize = skewValue (size, skew, false);
        const auto rightSize = skewValue (size, skew, true);
        const auto minimumDelaySamples = juce::jmap (smallSpaceDensity, 0.006f, 0.0015f) * (float) currentSampleRate;
        const auto topEndExtension = 1.0f + std::pow (sizeNormalised, 5.0f);
        const auto freeLeftSpacingSeconds = 0.0025f + std::pow (leftSize, 1.55f) * 0.233f * topEndExtension;
        const auto freeRightSpacingSeconds = 0.0025f + std::pow (rightSize, 1.55f) * 0.233f * topEndExtension;
        const auto syncShape = sizeSync * sizeSync;
        const auto syncedSpacingSeconds = beatSeconds * sizeDivisionToBeats (sizeDivision);
        const auto leftSyncSkew = std::pow (2.0f, skew * 0.5f);
        const auto rightSyncSkew = std::pow (2.0f, -skew * 0.5f);
        const auto leftSpacingSeconds = juce::jlimit (0.0015f, maxDelaySeconds * 0.35f,
            juce::jmap (syncShape, freeLeftSpacingSeconds, syncedSpacingSeconds * leftSyncSkew));
        const auto rightSpacingSeconds = juce::jlimit (0.0015f, maxDelaySeconds * 0.35f,
            juce::jmap (syncShape, freeRightSpacingSeconds, syncedSpacingSeconds * rightSyncSkew));
        const auto leftSpacingSamples = leftSpacingSeconds * (float) currentSampleRate;
        const auto rightSpacingSamples = rightSpacingSeconds * (float) currentSampleRate;
        const auto dampCoefficient = juce::jmap (damping, 0.62f, 0.045f);
        const auto sizeAbsorption = juce::jmap (size, 0.03f, 1.0f, 0.54f, 0.16f);
        const auto airLift = juce::jmap (air, 1.32f, 0.62f);
        const auto readDampCoefficient = juce::jlimit (0.018f, 0.72f, juce::jmin (dampCoefficient, sizeAbsorption) * airLift);
        const auto feedbackToneCoefficient = juce::jlimit (0.018f, 0.78f, dampCoefficient * juce::jmap (air, 1.18f, 0.70f));
        const auto lowCutHz = 18.0f + std::pow (lowCut, 2.2f) * 620.0f;
        const auto lowCutCoefficient = juce::jlimit (0.00002f, 0.25f,
            1.0f - std::exp (-juce::MathConstants<float>::twoPi * lowCutHz / (float) currentSampleRate));

        float sumL = 0.0f;
        float sumR = 0.0f;
        float signedSumL = 0.0f;
        float signedSumR = 0.0f;
        float earlySumL = 0.0f;
        float earlySumR = 0.0f;
        float lateSumL = 0.0f;
        float lateSumR = 0.0f;
        float weightSum = 0.0f;
        float weightPowerSum = 0.0f;
        float earlyPowerSum = 0.0f;
        float latePowerSum = 0.0f;
        std::array<float, maxPaths> pathOutL {};
        std::array<float, maxPaths> pathOutR {};

        for (int path = 0; path < maxPaths; ++path)
        {
            const auto weight = activeWeight (pathCount, path);
            const auto disorder = spread * spread;
            const auto pathShape = juce::jmap (disorder, 1.0f, 1.0f + 0.018f * (float) ((path * 7) % 5));
            const auto linearSpread = (float) path + 1.0f;
            const auto compactSpread = std::sqrt (linearSpread);
            const auto pathSpreadBase = compactSpread * smallSpaceDensity + linearSpread * (1.0f - smallSpaceDensity);
            const auto pathOffset = pathDisorder[(size_t) path] * disorder * (0.22f + 0.78f * sizeNormalised);
            const auto pathSpread = juce::jmax (0.30f, pathSpreadBase + pathOffset * juce::jmax (1.0f, std::sqrt (linearSpread)));
            const auto leftRatio = juce::jmap (disorder, 1.0f, pathRatios[(size_t) path]);
            const auto rightRatio = juce::jmap (disorder, 1.0f, 2.0f - pathRatios[(size_t) path]);
            const auto leftDelay = juce::jlimit (8.0f, (float) currentSampleRate * maxDelaySeconds * 0.92f,
                minimumDelaySamples + leftSpacingSamples * pathSpread * leftRatio * pathShape);
            const auto rightDelay = juce::jlimit (8.0f, (float) currentSampleRate * maxDelaySeconds * 0.92f,
                minimumDelaySamples + rightSpacingSamples * pathSpread * rightRatio / pathShape);
            pathOutL[(size_t) path] = leftReadFilters[(size_t) path].process (leftPaths[(size_t) path].read (leftDelay), readDampCoefficient);
            pathOutR[(size_t) path] = rightReadFilters[(size_t) path].process (rightPaths[(size_t) path].read (rightDelay), readDampCoefficient);

            sumL += weight * pathOutL[(size_t) path];
            sumR += weight * pathOutR[(size_t) path];
            const auto phase = ((path & 1) == 0) ? 1.0f : -1.0f;
            signedSumL += phase * weight * pathOutL[(size_t) path];
            signedSumR += phase * weight * pathOutR[(size_t) path];
            const auto pathPosition = (float) path / juce::jmax (1.0f, activePaths - 1.0f);
            const auto earlyWeight = weight * std::pow (1.0f - juce::jlimit (0.0f, 1.0f, pathPosition), 1.25f);
            const auto lateWeight = weight * std::pow (juce::jlimit (0.0f, 1.0f, pathPosition), 0.70f);
            earlySumL += earlyWeight * pathOutL[(size_t) path];
            earlySumR += earlyWeight * pathOutR[(size_t) path];
            lateSumL += lateWeight * pathOutL[(size_t) path];
            lateSumR += lateWeight * pathOutR[(size_t) path];
            weightSum += weight;
            weightPowerSum += weight * weight;
            earlyPowerSum += earlyWeight * earlyWeight;
            latePowerSum += lateWeight * lateWeight;
        }

        const auto loudnessCompensation = 0.72f + 0.28f * std::sqrt (juce::jlimit (0.0f, 1.0f, (activePaths - 1.0f) / ((float) maxPaths - 1.0f)));
        const auto normaliser = loudnessCompensation / std::sqrt (juce::jmax (1.0f, weightPowerSum));
        const auto earlyNormaliser = 0.78f / std::sqrt (juce::jmax (1.0f, earlyPowerSum));
        const auto lateNormaliser = 0.92f / std::sqrt (juce::jmax (1.0f, latePowerSum));
        const auto inverseWeightSum = 1.0f / juce::jmax (1.0f, weightSum);
        const auto matrixDiffuse = juce::jlimit (0.0f, 1.0f, matrix * 1.45f);
        const auto matrixOrbit = juce::jlimit (0.0f, 1.0f, matrix * 1.35f - 0.18f);
        const auto matrixScatter = juce::jlimit (0.0f, 1.0f, matrix * 1.60f - 0.54f);
        const auto matrixFeedbackTrim = 1.0f - 0.16f * matrixScatter - 0.08f * matrixOrbit;
        const auto earlyAmount = juce::jmap (diffusion, 0.16f, 0.045f);
        const auto inputBloomL = inputCapture * earlyAmount * (dryL + coupling * dryR);
        const auto inputBloomR = inputCapture * earlyAmount * (dryR + coupling * dryL);
        const auto earlyBusL = earlySumL * earlyNormaliser + inputBloomL * 1.18f;
        const auto earlyBusR = earlySumR * earlyNormaliser + inputBloomR * 1.18f;
        const auto lateDiffuseL = ((1.0f - diffusion) * lateSumL + diffusion * signedSumL) * lateNormaliser + inputBloomL * 0.54f;
        const auto lateDiffuseR = ((1.0f - diffusion) * lateSumR + diffusion * signedSumR) * lateNormaliser + inputBloomR * 0.54f;
        const auto fullCloudL = ((1.0f - diffusion) * sumL + diffusion * signedSumL) * normaliser + inputBloomL;
        const auto fullCloudR = ((1.0f - diffusion) * sumR + diffusion * signedSumR) * normaliser + inputBloomR;
        const auto lateShape = earlyLate * earlyLate * (3.0f - 2.0f * earlyLate);
        const auto wetBusL = juce::jmap (lateShape, earlyBusL, juce::jmap (matrixDiffuse, lateDiffuseL, fullCloudL));
        const auto wetBusR = juce::jmap (lateShape, earlyBusR, juce::jmap (matrixDiffuse, lateDiffuseR, fullCloudR));
        const auto pitchInputL = std::tanh (wetBusL + 0.25f * dryL);
        const auto pitchInputR = std::tanh (wetBusR + 0.25f * dryR);
        const auto pitchSweepSamples = juce::jlimit (0.035f, 0.18f, 0.045f + size * 0.11f) * (float) currentSampleRate;
        const auto octaveUpL = leftOctaveUpFilter.process (leftOctaveUp.process (pitchInputL, 2.0f, pitchSweepSamples), 0.28f);
        const auto octaveUpR = rightOctaveUpFilter.process (rightOctaveUp.process (pitchInputR, 2.0f, pitchSweepSamples * 1.07f), 0.28f);
        const auto octaveDownL = leftOctaveDownFilter.process (leftOctaveDown.process (pitchInputL, 0.5f, pitchSweepSamples * 1.43f), 0.16f);
        const auto octaveDownR = rightOctaveDownFilter.process (rightOctaveDown.process (pitchInputR, 0.5f, pitchSweepSamples * 1.51f), 0.16f);
        const auto pitchedWetL = wetBusL + 0.55f * octaveUp * octaveUpL + 0.70f * octaveDown * octaveDownL;
        const auto pitchedWetR = wetBusR + 0.55f * octaveUp * octaveUpR + 0.70f * octaveDown * octaveDownR;
        const auto wetL = std::tanh (1.65f * pitchedWetL);
        const auto wetR = std::tanh (1.65f * pitchedWetR);
        const auto meanL = sumL * inverseWeightSum;
        const auto meanR = sumR * inverseWeightSum;

        for (int path = 0; path < maxPaths; ++path)
        {
            const auto weight = activeWeight (pathCount, path);
            const auto previous = (path + maxPaths - 1) % maxPaths;
            const auto next = (path + 1) % maxPaths;
            const auto phase = ((path & 1) == 0) ? 1.0f : -1.0f;
            const auto clusterL = 0.50f * pathOutL[(size_t) path]
                                + 0.30f * pathOutL[(size_t) previous]
                                + 0.20f * pathOutL[(size_t) next];
            const auto clusterR = 0.50f * pathOutR[(size_t) path]
                                + 0.30f * pathOutR[(size_t) next]
                                + 0.20f * pathOutR[(size_t) previous];
            const auto orbitL = ((path & 1) == 0 ? pathOutL[(size_t) previous] : -pathOutR[(size_t) next]);
            const auto orbitR = ((path & 1) == 0 ? pathOutR[(size_t) next] : -pathOutL[(size_t) previous]);
            const auto scatteredL = 2.0f * meanL - pathOutL[(size_t) path];
            const auto scatteredR = 2.0f * meanR - pathOutR[(size_t) path];
            const auto matrixedL = juce::jmap (matrixOrbit, clusterL, orbitL);
            const auto matrixedR = juce::jmap (matrixOrbit, clusterR, orbitR);
            const auto diffuseL = juce::jmap (matrixScatter, matrixedL, phase * scatteredL);
            const auto diffuseR = juce::jmap (matrixScatter, matrixedR, -phase * scatteredR);
            const auto localFeedbackL = juce::jmap (matrixDiffuse * diffusion, pathOutL[(size_t) path], diffuseL);
            const auto localFeedbackR = juce::jmap (matrixDiffuse * diffusion, pathOutR[(size_t) path], diffuseR);
            const auto coupledFeedbackL = (1.0f - coupling) * localFeedbackL + coupling * localFeedbackR;
            const auto coupledFeedbackR = (1.0f - coupling) * localFeedbackR + coupling * localFeedbackL;
            const auto pathFeedbackL = leftFeedbackHighPass[(size_t) path].process (
                leftDampers[(size_t) path].process (coupledFeedbackL, feedbackToneCoefficient), lowCutCoefficient);
            const auto pathFeedbackR = rightFeedbackHighPass[(size_t) path].process (
                rightDampers[(size_t) path].process (coupledFeedbackR, feedbackToneCoefficient), lowCutCoefficient);
            const auto feedbackDepth = matrixFeedbackTrim * juce::jmap (lateShape, 0.72f, 1.08f);
            const auto excitation = weight * juce::jmap (lateShape, 0.60f, 0.34f) / std::sqrt (juce::jmax (1.0f, weightPowerSum));

            leftPaths[(size_t) path].write (inputCapture * excitation * (dryL + 0.18f * coupling * dryR)
                                            + effectiveFeedback * feedbackDepth * weight * pathFeedbackL);
            rightPaths[(size_t) path].write (inputCapture * excitation * (dryR + 0.18f * coupling * dryL)
                                             + effectiveFeedback * feedbackDepth * weight * pathFeedbackR);
        }

        const auto dryGain = std::cos (mix * juce::MathConstants<float>::halfPi);
        const auto wetGain = std::sin (mix * juce::MathConstants<float>::halfPi);
        auto outL = (dryL * dryGain + wetL * wetGain) * output;
        auto outR = (dryR * dryGain + wetR * wetGain) * output;
        outL = softLimit (outL, limiterReduction);
        outR = softLimit (outR, limiterReduction);
        left[sample] = outL;
        right[sample] = outR;

        energyL += outL * outL;
        energyR += outR * outR;
        const auto mid = (outL + outR) * 0.5f;
        const auto side = (outL - outR) * 0.5f;
        midEnergy += mid * mid;
        sideEnergy += side * side;
    }

    const auto inverseSamples = 1.0f / (float) juce::jmax (1, buffer.getNumSamples());
    const auto rmsL = std::sqrt (energyL * inverseSamples);
    const auto rmsR = std::sqrt (energyR * inverseSamples);
    const auto width = std::sqrt (sideEnergy / juce::jmax (1.0e-8f, midEnergy + sideEnergy));
    visualLevelLeft.store (juce::jlimit (0.0f, 1.0f, rmsL * 2.4f), std::memory_order_relaxed);
    visualLevelRight.store (juce::jlimit (0.0f, 1.0f, rmsR * 2.4f), std::memory_order_relaxed);
    visualStereoWidth.store (juce::jlimit (0.0f, 1.0f, width * 1.8f), std::memory_order_relaxed);
    visualLimiterReduction.store (juce::jlimit (0.0f, 1.0f, limiterReduction * 3.0f), std::memory_order_relaxed);
}

void ErbeyVerbyAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    {
        const juce::ScopedLock lock (modulationLock);
        for (int i = 0; i < (int) parameterIds.size(); ++i)
        {
            state.setProperty ("fabricScript_" + juce::String (parameterIds[(size_t) i]),
                               parameterModulations[(size_t) i].script,
                               nullptr);
            state.setProperty ("fabricScriptActive_" + juce::String (parameterIds[(size_t) i]),
                               parameterModulations[(size_t) i].active,
                               nullptr);
            state.setProperty ("fabricScriptDepth_" + juce::String (parameterIds[(size_t) i]),
                               parameterModulations[(size_t) i].depth,
                               nullptr);
        }
    }

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void ErbeyVerbyAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
            for (int i = 0; i < (int) parameterIds.size(); ++i)
            {
                setFabricScriptForParameter (parameterIds[(size_t) i],
                    parameters.state.getProperty ("fabricScript_" + juce::String (parameterIds[(size_t) i])).toString());
                setFabricScriptActive (parameterIds[(size_t) i],
                    (bool) parameters.state.getProperty ("fabricScriptActive_" + juce::String (parameterIds[(size_t) i]), true));
                setFabricScriptDepth (parameterIds[(size_t) i],
                    (float) parameters.state.getProperty ("fabricScriptDepth_" + juce::String (parameterIds[(size_t) i]), 1.0f));
            }
        }
}

juce::AudioProcessorEditor* ErbeyVerbyAudioProcessor::createEditor()
{
    return new ErbeyVerbyAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ErbeyVerbyAudioProcessor();
}

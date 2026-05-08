#include "PluginEditor.h"

void ErbeyVerbyAudioProcessorEditor::KnobLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                                                        int x,
                                                                        int y,
                                                                        int width,
                                                                        int height,
                                                                        float sliderPosProportional,
                                                                        float rotaryStartAngle,
                                                                        float rotaryEndAngle,
                                                                        juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (4.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.46f;
    const auto centre = bounds.getCentre();
    const auto lineWidth = juce::jmax (4.0f, radius * 0.16f);
    const auto arcRadius = radius - lineWidth * 0.5f;
    const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    juce::ColourGradient body (juce::Colour (0xff313a42), centre.x, centre.y - radius * 0.15f,
                               juce::Colour (0xff07090b), centre.x, centre.y + radius, true);
    body.addColour (0.32, juce::Colour (0xff1a2229));
    body.addColour (0.78, juce::Colour (0xff0d1115));
    g.setGradientFill (body);
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    g.setColour (juce::Colour (0x44e9f8ff));
    g.drawEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.0f);

    g.setColour (juce::Colour (0x22ffffff));
    g.drawEllipse (centre.x - radius * 0.72f, centre.y - radius * 0.72f, radius * 1.44f, radius * 1.44f, 1.2f);
    g.setColour (juce::Colour (0x24000000));
    g.drawEllipse (centre.x - radius * 0.86f, centre.y - radius * 0.86f, radius * 1.72f, radius * 1.72f, 2.0f);

    juce::Path backgroundArc;
    backgroundArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                 rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (slider.findColour (juce::Slider::rotarySliderOutlineColourId));
    g.strokePath (backgroundArc, juce::PathStrokeType (lineWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                            rotaryStartAngle, angle, true);
    g.setColour (slider.findColour (juce::Slider::rotarySliderFillColourId));
    g.strokePath (valueArc, juce::PathStrokeType (lineWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const auto thumbRadius = lineWidth * 0.64f;
    const auto thumbPoint = centre.getPointOnCircumference (arcRadius, angle);
    g.setColour (slider.findColour (juce::Slider::thumbColourId));
    g.fillEllipse (thumbPoint.x - thumbRadius, thumbPoint.y - thumbRadius, thumbRadius * 2.0f, thumbRadius * 2.0f);

    const auto dotRadius = juce::jmax (4.0f, lineWidth * 0.48f);
    const auto* modSlider = dynamic_cast<ErbeyVerbyAudioProcessorEditor::ModSlider*> (&slider);
    const auto hasScript = modSlider != nullptr && modSlider->hasScript;
    const auto scriptActive = modSlider != nullptr && modSlider->scriptActive;
    const auto dotColour = scriptActive ? juce::Colour (0xff42c7ff)
                                        : (hasScript ? juce::Colour (0xff276ea7) : juce::Colour (0xff193449));
    const auto glowAlpha = scriptActive ? 0x66 : (hasScript ? 0x30 : 0x14);

    g.setColour (dotColour.withAlpha ((juce::uint8) glowAlpha));
    g.fillEllipse (centre.x - dotRadius * 3.1f, centre.y - dotRadius * 3.1f, dotRadius * 6.2f, dotRadius * 6.2f);
    g.setColour (dotColour.brighter (scriptActive ? 0.18f : 0.02f));
    g.fillEllipse (centre.x - dotRadius, centre.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
    g.setColour (scriptActive ? juce::Colour (0xccffffff) : juce::Colour (0x55ffffff));
    g.fillEllipse (centre.x - dotRadius * 0.40f, centre.y - dotRadius * 0.48f, dotRadius * 0.58f, dotRadius * 0.42f);
    g.setColour (juce::Colour (0x66000000));
    g.drawEllipse (centre.x - dotRadius, centre.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f, 1.0f);
}

void ErbeyVerbyAudioProcessorEditor::ModSlider::mouseDown (const juce::MouseEvent& event)
{
    const auto bounds = getLocalBounds().toFloat().reduced (4.0f);
    const auto centre = bounds.getCentre();
    const auto dotRadius = juce::jmax (10.0f, juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.055f);

    if (event.position.getDistanceFrom (centre) <= dotRadius * 1.6f && onDotClicked != nullptr)
    {
        if (event.getNumberOfClicks() >= 2 && onDotDoubleClicked != nullptr)
            onDotDoubleClicked();
        else
            onDotClicked();

        return;
    }

    if (onManualDragStarted != nullptr)
        onManualDragStarted();

    juce::Slider::mouseDown (event);
}

namespace
{
class FabricScriptComponent final : public juce::Component
{
public:
    FabricScriptComponent (ErbeyVerbyAudioProcessor& processorIn,
                           juce::String parameterIdIn,
                           juce::String parameterNameIn)
        : processor (processorIn),
          parameterId (std::move (parameterIdIn)),
          parameterName (std::move (parameterNameIn))
    {
        title.setText ("Fabric envelope: " + parameterName, juce::dontSendNotification);
        title.setFont (juce::FontOptions (18.0f, juce::Font::bold));
        title.setColour (juce::Label::textColourId, juce::Colour (0xfff4f7fb));
        addAndMakeVisible (title);

        script.setMultiLine (true);
        script.setReturnKeyStartsNewLine (true);
        script.setScrollbarsShown (true);
        script.setFont (juce::FontOptions (14.0f));
        script.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0c1116));
        script.setColour (juce::TextEditor::textColourId, juce::Colour (0xffeef5ff));
        script.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff26323d));
        script.setText (processor.getFabricScriptForParameter (parameterId).isEmpty()
            ? defaultScript()
            : processor.getFabricScriptForParameter (parameterId));
        addAndMakeVisible (script);

        help.setText ("Commands: to 80%, random 20% 80%, hold, sine 10% 90%, wander 20% 80%.",
                      juce::dontSendNotification);
        help.setFont (juce::FontOptions (13.0f));
        help.setColour (juce::Label::textColourId, juce::Colour (0xff8f9aa5));
        help.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (help);

        depthLabel.setText ("Depth", juce::dontSendNotification);
        depthLabel.setFont (juce::FontOptions (13.0f));
        depthLabel.setColour (juce::Label::textColourId, juce::Colour (0xffb8c4ce));
        depthLabel.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (depthLabel);

        depth.setSliderStyle (juce::Slider::LinearHorizontal);
        depth.setTextBoxStyle (juce::Slider::TextBoxRight, false, 62, 20);
        depth.setRange (0.0, 1.0, 0.001);
        depth.setValue (processor.getFabricScriptDepth (parameterId), juce::dontSendNotification);
        depth.setColour (juce::Slider::trackColourId, juce::Colour (0xff42c7ff));
        depth.setColour (juce::Slider::backgroundColourId, juce::Colour (0xff24313b));
        depth.setColour (juce::Slider::thumbColourId, juce::Colour (0xffeef5ff));
        depth.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffdce7ef));
        depth.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        depth.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (depth);

        error.setFont (juce::FontOptions (13.0f, juce::Font::bold));
        error.setColour (juce::Label::textColourId, juce::Colour (0xffff6f61));
        error.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (error);

        apply.setButtonText ("Apply");
        apply.onClick = [this]
        {
            const auto validationError = processor.validateFabricScript (script.getText());
            if (validationError.isNotEmpty())
            {
                error.setText (validationError, juce::dontSendNotification);
                return;
            }

            processor.setFabricScriptForParameter (parameterId, script.getText());
            processor.setFabricScriptDepth (parameterId, (float) depth.getValue());
            if (auto* window = findParentComponentOfClass<juce::DialogWindow>())
                window->exitModalState (0);
        };
        addAndMakeVisible (apply);

        clear.setButtonText ("Clear");
        clear.onClick = [this]
        {
            processor.setFabricScriptForParameter (parameterId, {});
            if (auto* window = findParentComponentOfClass<juce::DialogWindow>())
                window->exitModalState (0);
        };
        addAndMakeVisible (clear);

        cancel.setButtonText ("Cancel");
        cancel.onClick = [this]
        {
            if (auto* window = findParentComponentOfClass<juce::DialogWindow>())
                window->exitModalState (0);
        };
        addAndMakeVisible (cancel);

        setSize (620, 470);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (18);
        title.setBounds (area.removeFromTop (30));
        help.setBounds (area.removeFromBottom (30));
        error.setBounds (area.removeFromBottom (28));
        auto buttons = area.removeFromBottom (42);
        apply.setBounds (buttons.removeFromRight (94).reduced (4));
        clear.setBounds (buttons.removeFromRight (94).reduced (4));
        cancel.setBounds (buttons.removeFromRight (94).reduced (4));
        auto depthArea = area.removeFromBottom (34);
        depthLabel.setBounds (depthArea.removeFromLeft (58));
        depth.setBounds (depthArea.reduced (0, 4));
        script.setBounds (area.reduced (0, 8));
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xff0e1419));
    }

private:
    juce::String defaultScript() const
    {
        return "modulator " + parameterId + "\n"
               "  mode loop\n"
               "  stage 1 random 20% 80% for 700ms curve smooth\n"
               "  stage 2 hold for 300ms\n"
               "  stage 3 sine 10% 90% for 2s\n"
               "  stage 4 wander 25% 75% for 900ms curve smooth\n"
               "end\n";
    }

    ErbeyVerbyAudioProcessor& processor;
    juce::String parameterId;
    juce::String parameterName;
    juce::Label title;
    juce::Label help;
    juce::Label depthLabel;
    juce::Label error;
    juce::TextEditor script;
    juce::Slider depth;
    juce::TextButton apply;
    juce::TextButton clear;
    juce::TextButton cancel;
};
}

ErbeyVerbyAudioProcessorEditor::ErbeyVerbyAudioProcessorEditor (ErbeyVerbyAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    title.setText ("MMVE", juce::dontSendNotification);
    title.setJustificationType (juce::Justification::centredLeft);
    title.setFont (juce::FontOptions (26.0f, juce::Font::plain));
    title.setColour (juce::Label::textColourId, juce::Colour (0xfff5f8fb));
    addAndMakeVisible (title);

    subtitle.setText ("morphing space echo", juce::dontSendNotification);
    subtitle.setJustificationType (juce::Justification::centredRight);
    subtitle.setFont (juce::FontOptions (13.0f));
    subtitle.setColour (juce::Label::textColourId, juce::Colour (0xff8d9ba7));
    addAndMakeVisible (subtitle);

    setupControl (paths, "Paths", "paths");
    setupControl (size, "Size", "size");
    setupControl (sizeSync, "Sync", "sizeSync");
    setupControl (sizeDivision, "Div", "sizeDivision");
    setupControl (coupling, "Coupling", "coupling");
    setupControl (skew, "Skew", "skew");
    setupControl (freeze, "Freeze", "freeze");
    setupControl (feedback, "Feedback", "feedback");
    setupControl (damping, "Damping", "damping");
    setupControl (lowCut, "Low Cut", "lowCut");
    setupControl (air, "Air", "air");
    setupControl (octaveUp, "Oct Up", "octaveUp");
    setupControl (octaveDown, "Oct Down", "octaveDown");
    setupControl (mix, "Mix", "mix");
    setupControl (output, "Output", "output");

    setSize (960, 500);
    startTimerHz (30);
}

void ErbeyVerbyAudioProcessorEditor::setupControl (Control& control, const juce::String& name, const juce::String& parameterId)
{
    control.parameterId = parameterId;
    control.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    control.slider.setLookAndFeel (&knobLookAndFeel);
    control.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 78, 20);
    control.slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff66d9ff));
    control.slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff202a33));
    control.slider.setColour (juce::Slider::thumbColourId, juce::Colour (0xfff6f8fb));
    control.slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffdce7ef));
    control.slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    control.slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (control.slider);
    control.slider.onDotClicked = [this, parameterId]
    {
        audioProcessor.setFabricScriptActive (parameterId, ! audioProcessor.isFabricScriptActive (parameterId));
    };
    control.slider.onDotDoubleClicked = [this, parameterId, name] { openFabricScriptEditor (parameterId, name); };
    control.slider.onManualDragStarted = [this, parameterId] { audioProcessor.setFabricScriptActive (parameterId, false); };

    control.label.setText (name, juce::dontSendNotification);
    control.label.setJustificationType (juce::Justification::centred);
    control.label.setFont (juce::FontOptions (13.5f, juce::Font::plain));
    control.label.setColour (juce::Label::textColourId, juce::Colour (0xffc5d0d9));
    addAndMakeVisible (control.label);

    control.attachment = std::make_unique<SliderAttachment> (audioProcessor.parameters, parameterId, control.slider);
}

void ErbeyVerbyAudioProcessorEditor::openFabricScriptEditor (const juce::String& parameterId, const juce::String& parameterName)
{
    juce::DialogWindow::LaunchOptions options;
    options.dialogTitle = "Fabric Script";
    options.dialogBackgroundColour = juce::Colour (0xff20262b);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    options.content.setOwned (new FabricScriptComponent (audioProcessor, parameterId, parameterName));
    options.launchAsync();
}

void ErbeyVerbyAudioProcessorEditor::timerCallback()
{
    const auto nextDisplayedPaths = getDisplayedPaths();
    if (std::abs (nextDisplayedPaths - displayedPaths) > 0.01f)
    {
        displayedPaths = nextDisplayedPaths;
        repaint();
    }

    updateScriptIndicator (paths);
    updateScriptIndicator (size);
    updateScriptIndicator (sizeSync);
    updateScriptIndicator (sizeDivision);
    updateScriptIndicator (coupling);
    updateScriptIndicator (skew);
    updateScriptIndicator (freeze);
    updateScriptIndicator (feedback);
    updateScriptIndicator (damping);
    updateScriptIndicator (lowCut);
    updateScriptIndicator (air);
    updateScriptIndicator (mix);
    updateScriptIndicator (output);
    updateScriptIndicator (octaveUp);
    updateScriptIndicator (octaveDown);

    updateModulatedSlider (paths);
    updateModulatedSlider (size);
    updateModulatedSlider (sizeSync);
    updateModulatedSlider (sizeDivision);
    updateModulatedSlider (coupling);
    updateModulatedSlider (skew);
    updateModulatedSlider (freeze);
    updateModulatedSlider (feedback);
    updateModulatedSlider (damping);
    updateModulatedSlider (lowCut);
    updateModulatedSlider (air);
    updateModulatedSlider (mix);
    updateModulatedSlider (output);
    updateModulatedSlider (octaveUp);
    updateModulatedSlider (octaveDown);
    updateVisualMeters();
}

void ErbeyVerbyAudioProcessorEditor::updateScriptIndicator (Control& control)
{
    if (control.parameterId.isEmpty())
        return;

    const auto hasScript = audioProcessor.parameterHasFabricScript (control.parameterId);
    const auto scriptActive = audioProcessor.isFabricScriptActive (control.parameterId);

    if (control.slider.hasScript != hasScript || control.slider.scriptActive != scriptActive)
    {
        control.slider.hasScript = hasScript;
        control.slider.scriptActive = scriptActive;
        control.slider.repaint();
    }
}

void ErbeyVerbyAudioProcessorEditor::updateModulatedSlider (Control& control)
{
    if (control.parameterId.isEmpty() || ! audioProcessor.isFabricScriptActive (control.parameterId))
        return;

    if (control.slider.isMouseButtonDown())
        return;

    const auto value = audioProcessor.getCurrentModulatedParameterValue (control.parameterId);
    control.slider.setValue (value, juce::dontSendNotification);
}

float ErbeyVerbyAudioProcessorEditor::getDisplayedPaths() const
{
    if (audioProcessor.isFabricScriptActive ("paths"))
        return audioProcessor.getCurrentModulatedParameterValue ("paths");

    return (float) paths.slider.getValue();
}

void ErbeyVerbyAudioProcessorEditor::updateVisualMeters()
{
    const auto follow = 0.28f;
    const auto decay = 0.92f;
    const auto update = [follow, decay] (float current, float target)
    {
        return target > current ? current + (target - current) * follow
                                : current * decay + target * (1.0f - decay);
    };

    const auto nextLeft = update (displayedLevelLeft, audioProcessor.getVisualLevelLeft());
    const auto nextRight = update (displayedLevelRight, audioProcessor.getVisualLevelRight());
    const auto nextWidth = update (displayedStereoWidth, audioProcessor.getVisualStereoWidth());
    const auto nextLimiter = update (displayedLimiter, audioProcessor.getVisualLimiterReduction());

    if (std::abs (nextLeft - displayedLevelLeft) > 0.002f
        || std::abs (nextRight - displayedLevelRight) > 0.002f
        || std::abs (nextWidth - displayedStereoWidth) > 0.002f
        || std::abs (nextLimiter - displayedLimiter) > 0.002f)
    {
        displayedLevelLeft = nextLeft;
        displayedLevelRight = nextRight;
        displayedStereoWidth = nextWidth;
        displayedLimiter = nextLimiter;
        repaint();
    }
}

void ErbeyVerbyAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff070a0d));

    auto outer = getLocalBounds().toFloat();
    juce::ColourGradient background (juce::Colour (0xff111b24), outer.getTopLeft(),
                                     juce::Colour (0xff030405), outer.getBottomRight(), false);
    background.addColour (0.40, juce::Colour (0xff0b1218));
    background.addColour (0.78, juce::Colour (0xff08090b));
    g.setGradientFill (background);
    g.fillRect (outer);

    auto halo = outer.reduced (80.0f, 30.0f);
    juce::ColourGradient sheen (juce::Colour (0x223ed0ff), halo.getTopLeft(),
                                juce::Colours::transparentBlack, halo.getBottomRight(), true);
    g.setGradientFill (sheen);
    g.fillEllipse (halo.withY (halo.getY() - 80.0f));

    auto panel = getLocalBounds().toFloat().reduced (18.0f);
    juce::ColourGradient glass (juce::Colour (0xcc26333d), panel.getTopLeft(),
                                juce::Colour (0xcc080a0c), panel.getBottomRight(), false);
    glass.addColour (0.40, juce::Colour (0xcc141b21));
    glass.addColour (0.82, juce::Colour (0xcc0a0d10));
    g.setGradientFill (glass);
    g.fillRoundedRectangle (panel, 8.0f);

    auto gloss = panel.reduced (1.0f).withTrimmedBottom (panel.getHeight() * 0.58f);
    juce::ColourGradient glossGradient (juce::Colour (0x30ffffff), gloss.getTopLeft(),
                                        juce::Colours::transparentWhite, gloss.getBottomLeft(), false);
    g.setGradientFill (glossGradient);
    g.fillRoundedRectangle (gloss, 8.0f);

    g.setColour (juce::Colour (0x55d7ecff));
    g.drawRoundedRectangle (panel, 8.0f, 1.0f);

    g.setColour (juce::Colour (0x26000000));
    g.drawRoundedRectangle (panel.reduced (1.5f), 7.0f, 1.2f);

    g.setColour (juce::Colour (0x16000000));
    g.fillRoundedRectangle (panel.withY (panel.getBottom() - 58.0f).withHeight (40.0f), 8.0f);

    const auto dotCount = 32;
    const auto active = juce::jlimit (1.0f, (float) dotCount, displayedPaths <= 0.0f ? getDisplayedPaths() : displayedPaths);
    const auto stripWidth = 248.0f;
    const auto spacing = stripWidth / (float) (dotCount - 1);
    const auto y = panel.getY() + 43.0f;
    const auto startX = panel.getCentreX() - stripWidth * 0.5f;

    for (int i = 0; i < dotCount; ++i)
    {
        const auto weight = juce::jlimit (0.0f, 1.0f, active - (float) i);
        const auto radius = 1.8f + weight * 1.6f;
        const auto centre = juce::Point<float> (startX + spacing * (float) i, y);
        const auto colour = juce::Colour (0xff42c7ff).withAlpha (0.14f + weight * 0.76f);

        if (weight > 0.0f)
        {
            g.setColour (colour.withAlpha (0.18f * weight));
            g.fillEllipse (centre.x - radius * 2.2f, centre.y - radius * 2.2f, radius * 4.4f, radius * 4.4f);
        }

        g.setColour (colour);
        g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
    }

    auto meter = juce::Rectangle<float> (panel.getCentreX() - 132.0f, panel.getY() + 58.0f, 264.0f, 18.0f);
    g.setColour (juce::Colour (0x28000000));
    g.fillRoundedRectangle (meter, 5.0f);
    g.setColour (juce::Colour (0x33d7ecff));
    g.drawRoundedRectangle (meter, 5.0f, 0.8f);

    auto meterInner = meter.reduced (5.0f, 5.0f);
    auto leftMeter = meterInner.removeFromLeft ((meterInner.getWidth() - 4.0f) * 0.5f);
    meterInner.removeFromLeft (4.0f);
    auto rightMeter = meterInner;
    auto centreLine = meter.withWidth (1.0f).withCentre (meter.getCentre());
    g.setColour (juce::Colour (0x44d7ecff));
    g.fillRect (centreLine);

    const auto leftWidth = leftMeter.getWidth() * juce::jlimit (0.0f, 1.0f, displayedLevelLeft);
    const auto rightWidth = rightMeter.getWidth() * juce::jlimit (0.0f, 1.0f, displayedLevelRight);
    g.setColour (juce::Colour (0xff42c7ff).withAlpha (0.22f + displayedLevelLeft * 0.60f));
    g.fillRoundedRectangle (leftMeter.withX (leftMeter.getRight() - leftWidth).withWidth (leftWidth), 3.0f);
    g.setColour (juce::Colour (0xff42c7ff).withAlpha (0.22f + displayedLevelRight * 0.60f));
    g.fillRoundedRectangle (rightMeter.withWidth (rightWidth), 3.0f);

    const auto widthX = meter.getCentreX() + juce::jmap (juce::jlimit (0.0f, 1.0f, displayedStereoWidth), -54.0f, 54.0f);
    g.setColour (juce::Colour (0xffeef5ff).withAlpha (0.42f + displayedStereoWidth * 0.45f));
    g.fillEllipse (widthX - 3.0f, meter.getCentreY() - 3.0f, 6.0f, 6.0f);

    if (displayedLimiter > 0.001f)
    {
        g.setColour (juce::Colour (0xffff6f61).withAlpha (0.18f + displayedLimiter * 0.62f));
        g.fillRoundedRectangle (meter.withTrimmedLeft (meter.getWidth() * (1.0f - displayedLimiter)), 5.0f);
    }
}

void ErbeyVerbyAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (38, 30);
    auto header = area.removeFromTop (48);
    title.setBounds (header.removeFromLeft (260));
    subtitle.setBounds (header);

    area.removeFromTop (12);

    auto upper = area.removeFromTop (168);
    auto lower = area.removeFromTop (168);

    const auto layoutControl = [] (Control& control, juce::Rectangle<int> bounds)
    {
        control.label.setBounds (bounds.removeFromTop (24));
        control.slider.setBounds (bounds);
    };

    const auto upperWidth = upper.getWidth() / 7;
    layoutControl (paths, upper.removeFromLeft (upperWidth).reduced (8));
    layoutControl (size, upper.removeFromLeft (upperWidth).reduced (8));
    layoutControl (sizeSync, upper.removeFromLeft (upperWidth).reduced (8));
    layoutControl (sizeDivision, upper.removeFromLeft (upperWidth).reduced (8));
    layoutControl (coupling, upper.removeFromLeft (upperWidth).reduced (8));
    layoutControl (skew, upper.removeFromLeft (upperWidth).reduced (8));
    layoutControl (freeze, upper.reduced (8));

    const auto lowerWidth = lower.getWidth() / 8;
    layoutControl (feedback, lower.removeFromLeft (lowerWidth).reduced (8));
    layoutControl (damping, lower.removeFromLeft (lowerWidth).reduced (8));
    layoutControl (lowCut, lower.removeFromLeft (lowerWidth).reduced (8));
    layoutControl (air, lower.removeFromLeft (lowerWidth).reduced (8));
    layoutControl (mix, lower.removeFromLeft (lowerWidth).reduced (8));
    layoutControl (output, lower.removeFromLeft (lowerWidth).reduced (8));
    layoutControl (octaveUp, lower.removeFromLeft (lowerWidth).reduced (8));
    layoutControl (octaveDown, lower.reduced (8));
}

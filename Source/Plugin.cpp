#include "Plugin.h"
#include "Cat.h"
#include "OverlayLifetime.h"
#include "HostForeground.h"
#include "Theme.h"

#if JUCE_MAC
 #include "MacHostWindow.h"
#endif

static juce::AudioProcessorValueTreeState::ParameterLayout parameters() {
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"dance",1},"Balletto",
          juce::StringArray{"Head Nod","Trap Bounce","Two Step","House","Wave","Spin","Running Man","Shuffle","Griddy","Floss","Charleston","Heel Toe"},1));
    l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"rate",1},"Velocita",
          juce::StringArray{"1/2","1x","2x"},1));
    l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"size",1},"Dimensione",
          juce::NormalisableRange<float>(.5f,1.6f,.01f),1.f));
    l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"overlay",1},"Mascot Overlay",false));
    return l;
}
void DanceProcessor::prepareToPlay(double sr,int) { sampleRate=sr; fallbackPpq=0; }
void DanceProcessor::releaseResources() { playing=false; valid=false; }
bool DanceProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
    return l.getMainInputChannelSet()==l.getMainOutputChannelSet()
        && (l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo()
         || l.getMainOutputChannelSet()==juce::AudioChannelSet::mono());
}
void DanceProcessor::updateClock(int samples) {
    Clock c;
    c.bpm=bpm.load(std::memory_order_relaxed);
    if(auto* h=getPlayHead()) if(auto position=h->getPosition()) {
        c.valid=true; c.playing=position->getIsPlaying();
        if(auto b=position->getBpm()) if(std::isfinite(*b)&&*b>0) c.bpm=*b;
        if (const auto signature = position->getTimeSignature())
            c.meter = dance::Meter::fromHost(signature->numerator, signature->denominator);
        if (const auto barStart = position->getPpqPositionOfLastBarStart())
            c.barStartPpq = *barStart;
        if(auto q=position->getPpqPosition()) {
            c.ppq=std::isfinite(*q)?*q:fallbackPpq;
        } else if(auto t=position->getTimeInSamples()) {
            c.ppq=(double)*t/sampleRate*c.bpm/60.0;
        } else c.ppq=fallbackPpq;
    }
    fallbackPpq=c.ppq+(c.playing ? samples/sampleRate*c.bpm/60.0 : 0.0);
    // All fields atomic: a bounded seqlock read on the UI cannot race or block audio.
    revision.fetch_add(1,std::memory_order_seq_cst);
    ppq.store(c.ppq); bpm.store(c.bpm); playing.store(c.playing); valid.store(c.valid);
    numerator.store(c.meter.numerator);
    denominator.store(c.meter.denominator);
    barStartPpq.store(c.barStartPpq);
    stamp.store(juce::Time::getMillisecondCounterHiRes());
    revision.fetch_add(1,std::memory_order_seq_cst);
}
DanceProcessor::Clock DanceProcessor::clock() const noexcept {
    Clock c;
    for(int tries=0;tries<3;++tries) {
        auto start=revision.load();
        if(start&1u) continue;
        c={ppq.load(),bpm.load(),stamp.load(),playing.load(),valid.load(),
           {numerator.load(), denominator.load()},barStartPpq.load()};
        if(start==revision.load()) return c;
    }
    c.valid=false; c.playing=false; return c;
}
void DanceProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&) { updateClock(b.getNumSamples()); }
void DanceProcessor::processBlockBypassed(juce::AudioBuffer<float>& b,juce::MidiBuffer&) { updateClock(b.getNumSamples()); }
int DanceProcessor::getCurrentProgram() { return (int)state.getRawParameterValue("dance")->load(); }
void DanceProcessor::setCurrentProgram(int i) {
    auto* p=state.getParameter("dance");
    p->beginChangeGesture(); p->setValueNotifyingHost(p->convertTo0to1((float)juce::jlimit(0,dance::presetCount-1,i))); p->endChangeGesture();
}
void DanceProcessor::getStateInformation(juce::MemoryBlock& dest) {
    if(auto xml=state.copyState().createXml()) copyXmlToBinary(*xml,dest);
}
void DanceProcessor::setStateInformation(const void* data,int size) {
    if(auto xml=getXmlFromBinary(data,size)) if(xml->hasTagName(state.state.getType()))
        state.replaceState(juce::ValueTree::fromXml(*xml));
}

class CatView : public juce::Component {
public:
    dance::Pose pose;
    CatView() { setOpaque(false); setWantsKeyboardFocus(false); }
    void paint(juce::Graphics& g) override { drawCat(g,getLocalBounds().toFloat(),pose); }
};
class FloatingCat final : public CatView {
public:
    FloatingCat() {
        setName("gett1 Dance - trascina il gattino");
        setSize(280,330);
        addToDesktop(juce::ComponentPeer::windowIsTemporary
                     | juce::ComponentPeer::windowIsSemiTransparent
                     | juce::ComponentPeer::windowIgnoresKeyPresses);
        if(auto* peer=getPeer()) peer->setCurrentRenderingEngine(0);
        setAlwaysOnTop(false);
       #if JUCE_WINDOWS
        if(auto* peer=getPeer())dance_native::attachToHost(static_cast<HWND>(peer->getNativeHandle()));
       #elif JUCE_MAC
        if (auto* peer = getPeer())
            dance_native::configureMacMascotWindow(peer->getNativeHandle());
       #endif
        if(auto* display=juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
            setTopLeftPosition(display->userArea.getRight()-getWidth()-50,
                               display->userArea.getBottom()-getHeight()-50);
    }
    ~FloatingCat() override { removeFromDesktop(); }
    void mouseDown(const juce::MouseEvent& e) override { drag.startDraggingComponent(this,e); }
    void mouseDrag(const juce::MouseEvent& e) override { drag.dragComponent(this,e,&constrainer); }
private:
    juce::ComponentDragger drag;
    struct Onscreen : juce::ComponentBoundsConstrainer {
        Onscreen() { setMinimumOnscreenAmounts(60,60,60,60); }
    } constrainer;
};


// This controller survives every editor close/reopen and owns its own animation clock.
class OverlayController final : private juce::Timer {
public:
    explicit OverlayController(DanceProcessor& processor) : proc(processor) { startTimerHz(60); }
    ~OverlayController() override { stopTimer(); }
private:
    void timerCallback() override {
        const bool enabled=proc.state.getRawParameterValue("overlay")->load()>.5f;
        bool showInHost = isDanceHostForeground();
       #if JUCE_MAC
        if (enabled && isGarageBandHost)
        {
            // An Audio Unit can be hosted outside the DAW's process.
            if (!showInHost)
                showInHost = dance_native::isGarageBandFrontmost();

            if (showInHost)
            {
                auto* editor = proc.getActiveEditor();
                void* editorView = editor != nullptr && editor->isShowing()
                                 ? editor->getWindowHandle() : nullptr;
                const bool editorVisible = dance_native::macEditorWindowIsVisible(editorView);

                if (editorVisible && !project.hasWindow())
                    project.rememberWindow(dance_native::findMacProjectWindow(editorView));

                const auto age = juce::Time::getMillisecondCounterHiRes() - proc.clock().stamp;
                showInHost = project.isVisible(
                    dance_native::macProjectWindowIsVisible(project.windowNumber()),
                    editorVisible, age >= 0.0 && age < 350.0);
            }
        }
       #endif
        if(auto* cat=window.updateVisibility(enabled,showInHost)) {
           #if JUCE_MAC
            if (auto* peer = cat->getPeer())
                dance_native::restoreMacMascotWindowIfHidden(peer->getNativeHandle());
           #endif
            const auto c=proc.clock();
            const auto age=juce::Time::getMillisecondCounterHiRes()-c.stamp;
            const bool active=c.valid && c.playing && age<350.0;
            const auto beat=dance::extrapolate(c.ppq,c.bpm,age,active)
                *dance::rate((int)proc.state.getRawParameterValue("rate")->load());
            const float scale=proc.state.getRawParameterValue("size")->load();
            const int w=juce::roundToInt(280*scale),h=juce::roundToInt(330*scale);
            if(cat->getWidth()!=w || cat->getHeight()!=h)cat->setSize(w,h);
            cat->pose=dance::pose(beat,proc.getCurrentProgram(),active);
            cat->repaint();
        }
    }
    DanceProcessor& proc;
    dance::OverlayLifetime<FloatingCat> window;
   #if JUCE_MAC
    const bool isGarageBandHost = dance_native::isGarageBandProcess()
                               || juce::PluginHostType().isGarageBand();
    dance::ProjectWindowLifetime project;
   #endif
};

DanceProcessor::DanceProcessor() : AudioProcessor(BusesProperties()
    .withInput("Input",juce::AudioChannelSet::stereo(),true)
    .withOutput("Output",juce::AudioChannelSet::stereo(),true)), state(*this,nullptr,"Gett1Dance",parameters()) { triggerAsyncUpdate(); }

DanceProcessor::~DanceProcessor() {
    // VST3 normally releases processors on the message thread. Synchronise teardown
    // for a host that releases on another thread: no queued callback or HWND outlives us.
    if(juce::MessageManager::getInstanceWithoutCreating()!=nullptr) {
        const juce::MessageManagerLock lock;
        cancelPendingUpdate();
        overlay.reset();
    } else {
        cancelPendingUpdate();
        overlay.reset();
    }
}
void DanceProcessor::handleAsyncUpdate() {
    if(!overlay) overlay=std::make_unique<OverlayController>(*this);
}

class DanceEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit DanceEditor(DanceProcessor& processor) : AudioProcessorEditor(processor),proc(processor) {
        setLookAndFeel(&look);setWantsKeyboardFocus(false);
        for(int i=0;i<dance::presetCount;++i) preset.addItem(dance::names[i],i+1);
        preset.setTooltip("Select a choreography. All movements follow the host timeline.");
        previous.setButtonText("<");next.setButtonText(">");
        previous.setTooltip("Previous preset");next.setTooltip("Next preset");
        previous.onClick=[this]{proc.setCurrentProgram((proc.getCurrentProgram()+dance::presetCount-1)%dance::presetCount);};
        next.onClick=[this]{proc.setCurrentProgram((proc.getCurrentProgram()+1)%dance::presetCount);};
        const juce::String labels[]={"1/2x","1x","2x"};
        for(int i=0;i<3;++i) {
            rateButtons[(size_t)i].setButtonText(labels[i]);
            rateButtons[(size_t)i].onClick=[this,i]{
                auto* parameter=proc.state.getParameter("rate");
                parameter->beginChangeGesture();parameter->setValueNotifyingHost(parameter->convertTo0to1((float)i));parameter->endChangeGesture();
            };
            rateButtons[(size_t)i].setTooltip("Musical speed relative to the host BPM");
            addAndMakeVisible(rateButtons[(size_t)i]);
        }
        size.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        size.setRotaryParameters(juce::MathConstants<float>::pi*1.25f,juce::MathConstants<float>::pi*2.75f,true);
        size.setTextBoxStyle(juce::Slider::TextBoxBelow,false,79,24);
        size.setRange(.5,1.6,.01);size.setDoubleClickReturnValue(true,1.0);
        size.textFromValueFunction=[](double v){return juce::String(juce::roundToInt(v*100))+" %";};
        size.valueFromTextFunction=[](const juce::String& v){return v.getDoubleValue()/100.0;};
        size.setTooltip("Overlay size. Double-click to reset to 100%.");
        overlayButton.setButtonText("SHOW MASCOT");overlayButton.setClickingTogglesState(true);
        overlayButton.setTooltip("Keep the mascot visible in the host DAW after closing this window.");
        for(juce::Component* c:std::array<juce::Component*,7>{&preset,&previous,&next,&size,&overlayButton,&preview,&tempo}) addAndMakeVisible(c);
        tempo.setFont(skin::font(23.f));tempo.setColour(juce::Label::textColourId,skin::text);
        tempo.setJustificationType(juce::Justification::centredRight);
        danceAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.state,"dance",preset);
        sizeAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc.state,"size",size);
        overlayAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.state,"overlay",overlayButton);
        setSize(840,544);startTimerHz(60);
    }
    ~DanceEditor() override {stopTimer();setLookAndFeel(nullptr);}
    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds().toFloat();
        g.fillAll(juce::Colour(0xff171218));
        skin::metal(g, bounds.reduced(3.f), 12.f, juce::Colour(0xffeed1b6),
                    juce::Colour(0xff72505a), juce::Colour(0xff674651));
        skin::metal(g, bounds.reduced(7.f), 9.f, juce::Colour(0xff493541),
                    juce::Colour(0xff241b26), juce::Colour(0xff1a131a));
        skin::metal(g, bounds.reduced(11.f), 7.f, juce::Colour(0xff302531),
                    juce::Colour(0xff231a24), juce::Colour(0xff604452));

        skin::screw(g, 18.f, 18.f);
        skin::screw(g, 822.f, 18.f);
        skin::screw(g, 18.f, 526.f);
        skin::screw(g, 822.f, 526.f);

        skin::metal(g, {32.f, 28.f, 44.f, 44.f}, 7.f, juce::Colour(0xff352332),
                    juce::Colour(0xff211722), juce::Colour(0xff785061));
        skin::mark(g, 54.f, 51.f);
        skin::label(g, "gett1", {90, 29, 110, 41}, skin::accent, 33.f);
        g.setColour(juce::Colour(0xff755364));
        g.drawVerticalLine(191, 37.f, 62.f);
        skin::label(g, "DANCE", {205, 28, 193, 44}, skin::text, 37.f);

        skin::metal(g, {602.f, 27.f, 206.f, 46.f}, 5.f, juce::Colour(0xff211722),
                    juce::Colour(0xff2a1f2a), juce::Colour(0xff785960));
        skin::label(g, "HOST TEMPO", {615, 34, 97, 32}, skin::muted, 13.f);
        skin::label(g, "BPM", {770, 34, 27, 32}, skin::muted, 13.f,
                    juce::Justification::centredRight);
        g.setColour(juce::Colour(0xff5b404f));
        g.drawHorizontalLine(86, 25.f, 815.f);
        g.setColour(juce::Colour(0xff211820));
        g.drawHorizontalLine(87, 25.f, 815.f);

        drawStage(g);
        drawControls(g);
    }

    void resized() override {
        tempo.setBounds(700, 32, 62, 36);
        preview.setBounds(111, 151, 328, 266);
        previous.setBounds(563, 173, 31, 35);
        preset.setBounds(600, 173, 157, 35);
        next.setBounds(763, 173, 31, 35);
        for (int i = 0; i < 3; ++i)
            rateButtons[static_cast<std::size_t>(i)].setBounds(563 + i * 78, 250, 74, 33);
        size.setBounds(566, 329, 110, 130);
        overlayButton.setBounds(28, 482, 784, 37);
    }

private:
    void drawStage(juce::Graphics& g) const {
        skin::metal(g, {28.f, 100.f, 500.f, 370.f}, 7.f, juce::Colour(0xff69505a),
                    juce::Colour(0xff31242f), juce::Colour(0xff82616a));
        skin::metal(g, {32.f, 104.f, 492.f, 362.f}, 5.f, juce::Colour(0xff18131d),
                    juce::Colour(0xff272030), juce::Colour(0xff150f19));

        {
            juce::Graphics::ScopedSaveState screen(g);
            g.reduceClipRegion(37, 149, 482, 270);
            juce::ColourGradient light(juce::Colour(0xff54304a), 278.f, 299.f,
                                       juce::Colour(0xff1b1621).withAlpha(0.f), 519.f, 338.f, true);
            light.addColour(.54, juce::Colour(0xff402638).withAlpha(.65f));
            g.setGradientFill(light);
            g.fillRect(37, 149, 482, 270);
            g.setColour(juce::Colour(0xffbd78a6).withAlpha(.11f));
            for (int y = 166; y < 416; y += 20)
                for (int x = 48; x < 515; x += 20)
                    g.fillRect(x, y, 1, 1);

            const std::array<juce::Rectangle<float>, 3> floor {{
                {99.f, 365.f, 358.f, 51.f}, {145.f, 372.f, 266.f, 35.f}, {199.f, 379.f, 158.f, 20.f}
            }};
            g.setColour(skin::accent.withAlpha(.15f));
            for (const auto& ring : floor)
                g.drawEllipse(ring, 1.f);
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff120d16).withAlpha(.55f), 278.f, 391.f,
                                                   juce::Colour(0xff120d16).withAlpha(0.f), 433.f, 391.f, true));
            g.fillEllipse(123.f, 374.f, 310.f, 34.f);
        }

        skin::label(g, "DANCE PREVIEW", {48, 116, 180, 23}, skin::text, 16.f);
        const juce::String signature = clockValid
            ? juce::String(meter.numerator) + "/" + juce::String(meter.denominator) : "--";
        skin::label(g, signature, {222, 115, 112, 24}, skin::muted, 14.f, juce::Justification::centred);
        const auto status = !clockValid ? "WAITING FOR HOST" : playing ? "HOST PLAYING" : "HOST STOPPED";
        const auto signal = playing ? skin::green : skin::muted;
        g.setColour(signal);
        g.fillEllipse(365.f, 124.f, 6.f, 6.f);
        skin::label(g, status, {379, 115, 128, 24}, signal, 13.f);
        g.setColour(juce::Colour(0xff493343));
        g.drawHorizontalLine(147, 47.f, 509.f);
        g.drawHorizontalLine(421, 47.f, 509.f);

        if (meter.numerator > 24) {
            g.setColour(juce::Colour(0xff4e3549));
            g.fillRoundedRectangle(48.f, 434.f, 461.f, 3.f, 1.f);
            if (playing) {
                g.setColour(skin::accent);
                const auto progress = static_cast<float>(beat + 1) / static_cast<float>(meter.numerator);
                g.fillRoundedRectangle(48.f, 434.f, 461.f * progress, 3.f, 1.f);
            }
            skin::label(g, juce::String(beat + 1) + " / " + juce::String(meter.numerator),
                        {48, 441, 461, 16}, skin::muted, 13.f, juce::Justification::centred);
            return;
        }

        const auto step = 468.f / static_cast<float>(meter.numerator);
        const auto gap = juce::jmin(7.f, step * .2f);
        for (int i = 0; i < meter.numerator; ++i) {
            const auto x = 48.f + static_cast<float>(i) * step;
            const auto width = step - gap;
            const bool on = playing && beat == i;
            g.setColour(on ? skin::accent : juce::Colour(0xff4e3549));
            g.fillRoundedRectangle(x, 434.f, width, 3.f, 1.f);
            skin::label(g, juce::String(i + 1), {juce::roundToInt(x), 441, juce::roundToInt(width), 16},
                        on ? skin::accent : skin::muted, 13.f, juce::Justification::centred);
        }
    }

    void drawControls(juce::Graphics& g) const {
        skin::metal(g, {544.f, 100.f, 268.f, 370.f}, 7.f, juce::Colour(0xffd6b9a5),
                    juce::Colour(0xff9c776d), juce::Colour(0xff6e5157));
        skin::metal(g, {548.f, 104.f, 260.f, 362.f}, 5.f, juce::Colour(0xffe8cbb4),
                    juce::Colour(0xffbe9685), juce::Colour(0xfff4ddc6));

        for (int y = 109; y < 464; ++y) {
            const float opacity = (y % 4 == 0) ? .13f : .055f;
            g.setColour((y % 2 == 0 ? juce::Colours::white : skin::ink).withAlpha(opacity));
            g.drawHorizontalLine(y, 552.f, 804.f);
        }

        skin::label(g, "PERFORMANCE", {563, 113, 231, 24}, skin::ink, 19.f);
        for (const auto y : {140, 221, 296}) {
            g.setColour(juce::Colour(0xff9b766e));
            g.drawHorizontalLine(y, 563.f, 794.f);
            g.setColour(juce::Colour(0xfff1d5be).withAlpha(.6f));
            g.drawHorizontalLine(y + 1, 563.f, 794.f);
        }
        skin::label(g, "DANCE PRESET", {563, 149, 231, 18}, skin::ink, 14.f);
        skin::label(g, "RATE", {563, 228, 231, 18}, skin::ink, 14.f);
        skin::label(g, "OVERLAY SIZE", {563, 305, 231, 20}, skin::ink, 14.f);
        skin::label(g, "Double-click", {695, 368, 99, 17}, skin::ink, 14.f);
        skin::label(g, "to reset", {695, 386, 99, 17}, skin::ink, 14.f);
    }

    void timerCallback() override {
        const auto c=proc.clock();const auto age=juce::Time::getMillisecondCounterHiRes()-c.stamp;
        const bool oldValid = clockValid, oldPlaying = playing;
        const auto oldMeter = meter;
        const int oldBeat = beat;
        clockValid=c.valid;playing=c.valid&&c.playing&&age<350.;
        const auto hostBeat=dance::extrapolate(c.ppq,c.bpm,age,playing);
        meter = c.meter;
        beat = meter.beatInBar(hostBeat, c.barStartPpq);
        const bool meterChanged = meter.numerator != oldMeter.numerator || meter.denominator != oldMeter.denominator;
        const int rate=(int)proc.state.getRawParameterValue("rate")->load();
        preview.pose=dance::pose(hostBeat*dance::rate(rate),proc.getCurrentProgram(),playing);preview.repaint();
        tempo.setText(c.valid?juce::String(c.bpm,1):"--.-",juce::dontSendNotification);
        const bool overlayEnabled=proc.state.getRawParameterValue("overlay")->load()>.5f;
        overlayButton.setButtonText(overlayEnabled?"HIDE MASCOT":"SHOW MASCOT");
        for(int i=0;i<3;++i)rateButtons[(size_t)i].setToggleState(i==rate,juce::dontSendNotification);
        if (oldValid != clockValid || oldPlaying != playing) repaint(359, 115, 151, 24);
        if (oldValid != clockValid || meterChanged) repaint(222, 115, 112, 24);
        if (meterChanged || oldBeat != beat || oldPlaying != playing) repaint(48, 432, 462, 27);
    }
    DanceProcessor& proc;
    skin::Look look;
    juce::TooltipWindow tooltip{this,650};
    juce::ComboBox preset;
    juce::TextButton previous,next;
    std::array<juce::TextButton,3> rateButtons;
    juce::Slider size;
    juce::ToggleButton overlayButton;
    juce::Label tempo;
    CatView preview;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> danceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> overlayAttachment;
    bool clockValid=false,playing=false;
    dance::Meter meter;
    int beat = 0;
};
juce::AudioProcessorEditor* DanceProcessor::createEditor() {
    // Editor creation is on the UI thread; initialise synchronously if the queued
    // constructor update has not run yet. Nothing is tied to editor destruction.
    cancelPendingUpdate();
    handleAsyncUpdate();
    return new DanceEditor(*this);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DanceProcessor(); }

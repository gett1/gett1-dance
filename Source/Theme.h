#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <DanceTypefaceData.h>
#include <algorithm>
#include <cmath>

namespace skin {
inline const juce::Colour text{0xfff2dfcc}, muted{0xffbd9f9e}, accent{0xfff27ab9},
                          green{0xffacd4a0}, ink{0xff38282d}, edge{0xff73565e};

inline juce::Font font(float height) {
    static const auto face = juce::Typeface::createSystemTypefaceFor(
        DanceTypefaceData::PixelifySans_ttf, DanceTypefaceData::PixelifySans_ttfSize);
    return juce::Font(juce::FontOptions(face).withHeight(height));
}

inline void label(juce::Graphics& g, const juce::String& value, juce::Rectangle<int> bounds,
                  juce::Colour colour = muted, float height = 15.f,
                  juce::Justification align = juce::Justification::centredLeft) {
    g.setFont(font(height));
    g.setColour(colour);
    g.drawText(value, bounds, align);
}

inline void metal(juce::Graphics& g, juce::Rectangle<float> bounds, float radius,
                  juce::Colour top, juce::Colour bottom, juce::Colour stroke) {
    g.setGradientFill(juce::ColourGradient(top, 0, bounds.getY(), bottom, 0, bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds, radius);
    g.setColour(stroke);
    g.drawRoundedRectangle(bounds.reduced(.5f), radius, 1.f);
}

inline void screw(juce::Graphics& g, float x, float y) {
    const juce::Rectangle<float> cap(x - 3.5f, y - 3.5f, 7.f, 7.f);
    g.setColour(juce::Colour(0xff130d12));
    g.fillEllipse(cap.expanded(1.f));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffd9b5a2), x, y - 4.f,
                                           juce::Colour(0xff69464c), x, y + 4.f, false));
    g.fillEllipse(cap);
    g.setColour(juce::Colour(0xff3b2830));
    g.drawLine(x - 1.7f, y + .7f, x + 1.7f, y - .7f, 1.1f);
}

inline void mark(juce::Graphics& g, float x, float y) {
    juce::Path head;
    head.startNewSubPath(x - 11.f, y - 1.f);
    head.lineTo(x - 9.f, y - 11.f);
    head.lineTo(x - 3.f, y - 6.f);
    head.lineTo(x + 3.f, y - 6.f);
    head.lineTo(x + 9.f, y - 11.f);
    head.lineTo(x + 11.f, y - 1.f);
    head.cubicTo(x + 14.f, y + 10.f, x - 14.f, y + 10.f, x - 11.f, y - 1.f);
    head.closeSubPath();
    g.setColour(accent);
    g.fillPath(head);
    g.setColour(juce::Colour(0xff3a202e));
    for (const float offset : {-4.8f, 4.8f}) {
        juce::Path eye;
        eye.startNewSubPath(x + offset - 2.f, y);
        eye.quadraticTo(x + offset, y + 3.f, x + offset + 2.f, y);
        g.strokePath(eye, juce::PathStrokeType(1.3f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}

class Look final : public juce::LookAndFeel_V4 {
public:
    Look() {
        const juce::Colour glass(0xff211923);
        setColour(juce::ComboBox::textColourId, text);
        setColour(juce::ComboBox::backgroundColourId, glass);
        setColour(juce::PopupMenu::backgroundColourId, glass);
        setColour(juce::PopupMenu::textColourId, text);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff653650));
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxTextColourId, text);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff5d424c));
        setColour(juce::Slider::textBoxBackgroundColourId, glass);
        setColour(juce::Slider::textBoxHighlightColourId, juce::Colour(0xff653650));
        setColour(juce::TooltipWindow::backgroundColourId, glass);
        setColour(juce::TooltipWindow::textColourId, text);
        setColour(juce::TooltipWindow::outlineColourId, edge);
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override { return font(18.f); }
    juce::Font getPopupMenuFont() override { return font(17.f); }
    juce::Font getTextButtonFont(juce::TextButton&, int) override { return font(16.f); }

    juce::Label* createSliderTextBox(juce::Slider& slider) override {
        auto* display = juce::LookAndFeel_V4::createSliderTextBox(slider);
        display->setFont(font(18.f));
        return display;
    }

    void drawComboBox(juce::Graphics& g, int w, int h, bool down, int, int, int, int, juce::ComboBox& box) override {
        metal(g, {0, 0, static_cast<float>(w), static_cast<float>(h)}, 3.f,
              juce::Colour(0xff302433), juce::Colour(0xff1c151f),
              down || box.hasKeyboardFocus(true) ? accent : juce::Colour(0xff5d424c));
        const auto x = static_cast<float>(w) - 14.f;
        const auto y = static_cast<float>(h) * .5f;
        juce::Path arrow;
        arrow.startNewSubPath(x - 4.f, y - 2.f);
        arrow.lineTo(x, y + 2.f);
        arrow.lineTo(x + 4.f, y - 2.f);
        g.setColour(accent);
        g.strokePath(arrow, juce::PathStrokeType(1.3f));
    }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& display) override {
        display.setBounds(10, 0, box.getWidth() - 33, box.getHeight());
        display.setFont(getComboBoxFont(box));
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour&, bool over, bool down) override {
        auto top = button.getToggleState() ? juce::Colour(0xff733f58) : juce::Colour(0xff46343e);
        auto bottom = button.getToggleState() ? juce::Colour(0xff502d43) : juce::Colour(0xff2e232d);
        const float lift = down ? -.16f : over ? .14f : 0.f;
        top = lift < 0.f ? top.darker(-lift) : top.brighter(lift);
        bottom = lift < 0.f ? bottom.darker(-lift) : bottom.brighter(lift);
        const bool highlighted = button.getToggleState() || button.hasKeyboardFocus(true);
        metal(g, button.getLocalBounds().toFloat(), 3.f, top, bottom,
              highlighted ? accent.withAlpha(.85f) : juce::Colour(0xff735862));
        g.setColour(top.brighter(.25f));
        g.drawHorizontalLine(1, 5.f, static_cast<float>(button.getWidth()) - 5.f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override {
        label(g, button.getButtonText(), button.getLocalBounds(), text, 16.f, juce::Justification::centred);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool over, bool down) override {
        drawButtonBackground(g, button, {}, over, down);
        auto bounds = button.getLocalBounds();
        const bool on = button.getToggleState();
        const auto light = on ? accent : muted;
        g.setColour(light.withAlpha(.14f));
        g.fillEllipse(13.f, bounds.getCentreY() - 6.f, 12.f, 12.f);
        g.setColour(light);
        g.fillEllipse(16.f, bounds.getCentreY() - 3.f, 6.f, 6.f);
        label(g, button.getButtonText(), bounds.withTrimmedLeft(34).withTrimmedRight(76), text, 16.f);
        const auto state = bounds.removeFromRight(61).reduced(7, 8);
        metal(g, state.toFloat(), 2.f, juce::Colour(0xff291d28), juce::Colour(0xff201822), edge);
        label(g, on ? "ON" : "OFF", state, light, 14.f, juce::Justification::centred);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float value, float a, float b, juce::Slider& slider) override {
        const auto cx = x + w * .5f, cy = y + h * .5f;
        const auto scale = std::clamp(std::min(w, h) * .5f - 3.f, 8.f, 53.f);
        const auto radius = scale * .77f;
        const auto opacity = slider.isEnabled() ? 1.f : .45f;
        const auto angle = a + (b - a) * value;
        const auto disc = [=](float r) { return juce::Rectangle<float>(cx-r, cy-r, 2*r, 2*r); };
        const auto polar = [=](float t, float r) -> juce::Point<float> { return {cx + std::sin(t) * r, cy - std::cos(t) * r}; };
        const auto tone = [=](uint32_t argb, float alpha = 1.f) { return juce::Colour(argb).withAlpha(alpha * opacity); };

        constexpr int divisions = 20;
        for (int i = 0; i <= divisions; ++i) {
            const auto ratio = static_cast<float>(i) / divisions;
            const auto t = a + (b - a) * ratio;
            const bool major = i % 5 == 0;
            const auto start = polar(t, scale);
            const auto depth = std::min(major ? 4.7f : 2.65f, scale * (major ? .14f : .08f));
            const auto end = polar(t, scale - depth);
            g.setColour(tone(0xfff8f0d7, .54f));
            g.drawLine({start.translated(0,.85f), end.translated(0,.85f)}, major ? 1.25f : .73f);
            g.setColour(tone(ratio <= value ? 0xff833e61 : 0xff624e49, major ? .95f : .57f));
            g.drawLine({start, end}, major ? 1.25f : .73f);
        }

        const auto seat = disc(radius + std::min(2.35f, scale * .07f));
        for (int i = 4; i > 0; --i) {
            g.setColour(tone(0xff302719, .039f * (5 - i)));
            g.fillEllipse(seat.expanded(i * .48f).translated(.75f, 1.9f));
        }
        auto brass = juce::ColourGradient(tone(0xfff5d9bb), cx - radius, cy - radius,
                                         tone(0xff705063), cx + radius, cy + radius, false);
        brass.addColour(.28, tone(0xffb08778));
        brass.addColour(.53, tone(0xffe8b99b));
        brass.addColour(.74, tone(0xff78525b));
        g.setGradientFill(brass);
        g.fillEllipse(seat);
        g.setColour(tone(0xff594831, .86f));
        g.drawEllipse(seat, .8f);

        juce::Path skirt;
        constexpr int ribs = 25;
        const auto tooth = std::clamp(radius * .085f, 1.f, 3.3f);
        for (int i = 0; i < ribs * 4; ++i) {
            const float t = angle + juce::MathConstants<float>::twoPi * i / (ribs * 4);
            const auto edge = polar(t, i % 4 == 0 || i % 4 == 3 ? radius - tooth : radius);
            if (i == 0) skirt.startNewSubPath(edge); else skirt.lineTo(edge);
        }
        skirt.closeSubPath();
        g.setColour(tone(0xff1b1d1a));
        g.fillPath(skirt, juce::AffineTransform::translation(0.f, 1.65f));
        auto edge = juce::ColourGradient(tone(0xff75616e), cx - radius, cy - radius,
                                        tone(0xff111512), cx + radius, cy + radius, false);
        edge.addColour(.40, tone(0xff443641));
        edge.addColour(.68, tone(0xff2a222d));
        g.setGradientFill(edge);
        g.fillPath(skirt);

        const auto faceRadius = radius - std::max(2.9f, radius * .145f);
        const auto cap = disc(faceRadius);
        auto finish = juce::ColourGradient(tone(0xff655360), cx - faceRadius * .60f, cy - faceRadius * .70f,
                                          tone(0xff232027), cx + faceRadius * .78f, cy + faceRadius * .91f, true);
        finish.addColour(.43, tone(0xff41323d));
        finish.addColour(.74, tone(0xff342734));
        g.setGradientFill(finish);
        g.fillEllipse(cap);
        g.setColour(tone(0xffaba895, .60f));
        g.drawEllipse(cap.reduced(.35f), .7f);
        g.setColour(tone(0xff171c18, .8f));
        g.drawEllipse(cap.reduced(1.05f), .78f);

        if (faceRadius > 12.f) {
            juce::Graphics::ScopedSaveState clip(g);
            juce::Path brushed;
            brushed.addEllipse(cap.reduced(2.f));
            g.reduceClipRegion(brushed);
            for (int n = 0; n < 9; ++n) {
                const auto lineY = cap.getY() + 4 + n * cap.getHeight() / 10.f;
                g.setColour(tone(0xffb4b3a1, n % 2 ? .045f : .026f));
                g.drawLine(cap.getX(), lineY, cap.getRight(), lineY - cap.getHeight() * .35f, .57f);
            }
        }

        const auto inner = polar(angle, faceRadius * .35f);
        const auto outer = polar(angle, radius - 1.9f);
        constexpr float pin = 2.6f;
        g.setColour(tone(0xff111512));
        g.drawLine({inner, outer}, pin + 1.8f);
        g.setColour(tone(0xfff59ac6));
        g.drawLine({inner, outer}, pin);
        g.setColour(tone(0xffffd8eb, .9f));
        g.drawLine({inner.translated(-.25f, -.29f), outer.translated(-.25f, -.29f)}, .70f);
    }
};
}

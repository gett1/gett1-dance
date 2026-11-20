#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "Motion.h"

// Articulated vinyl-style vector mascot. All shading is clipped to body parts;
// the desktop surface remains fully transparent outside the character.
inline void drawCat(juce::Graphics& g, juce::Rectangle<float> box, const dance::Pose& p) {
    juce::Graphics::ScopedSaveState frame(g);
    const float size=juce::jmin(box.getWidth()/280.f,box.getHeight()/330.f);
    g.addTransform(juce::AffineTransform::scale(size).translated(box.getCentreX(),box.getCentreY()));
    g.addTransform(juce::AffineTransform::translation((float)p.x,(float)p.y));
    g.addTransform(juce::AffineTransform::rotation((float)p.lean,0,90));
    const float facing=(float)p.turn;
    const float depth=std::copysign(.22f+.78f*std::abs(facing),facing);
    g.addTransform(juce::AffineTransform::scale(depth*(float)(2-p.squash),(float)p.squash));
    const juce::Colour rim(0xff8d225f), light(0xffff9bdc), pink(0xfff35db8), dark(0xffbd297f);
    auto surface=[&](const juce::Path& path, juce::Rectangle<float> bounds, bool back=false) {
        juce::ColourGradient material(back?pink:light,bounds.getX(),bounds.getY(),
                                      back?juce::Colour(0xffa62672):dark,bounds.getRight(),bounds.getBottom(),false);
        material.addColour(.42,back?juce::Colour(0xffd73b94):pink);
        g.setGradientFill(material); g.fillPath(path);
        g.setColour(rim); g.strokePath(path,juce::PathStrokeType(1.35f));
    };
    auto oval=[&](juce::Rectangle<float> b,bool back=false) {
        juce::Path path;path.addEllipse(b);surface(path,b,back);
    };
    auto capsule=[&](juce::Point<float> a,juce::Point<float> b,float width,bool back=false) {
        juce::Path line,shape;line.startNewSubPath(a);line.lineTo(b);
        juce::PathStrokeType(width,juce::PathStrokeType::curved,juce::PathStrokeType::rounded).createStrokedPath(shape,line);
        surface(shape,shape.getBounds(),back);
    };
    // Tail behind the torso; legs and feet are drawn in front to keep raised knees visible.
    juce::Path tail,tailShape;tail.startNewSubPath(35,84);
    tail.cubicTo(91,108,107,78+(float)p.tail*65,101,54+(float)p.tail*65);
    juce::PathStrokeType(25,juce::PathStrokeType::curved,juce::PathStrokeType::rounded).createStrokedPath(tailShape,tail);
    surface(tailShape,tailShape.getBounds(),true);
    oval({-43,17,86,110});
    // Soft belly highlight, deliberately contained inside the torso.
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffffb2e2).withAlpha(.22f),-12,61,
                                          pink.withAlpha(0.f),24,104,true));
    g.fillEllipse(-25,40,48,69);
    g.setColour(rim.withAlpha(.4f));g.fillEllipse(-33,20,66,17);
    for(int side : {-1,1}) {
        const float lift=(float)(side<0?p.leftFoot:p.rightFoot);
        const float swing=(float)(side<0?p.leftLeg:p.rightLeg);
        const float slide=(float)(side<0?p.leftSlide:p.rightSlide);
        const float toe=(float)(side<0?p.leftToe:p.rightToe);
        const juce::Point<float> hip(side*22.f,108);
        const juce::Point<float> knee(side*25.f+12*std::sin(swing)+slide*.4f,126-lift*.55f);
        const juce::Point<float> ankle(side*29.f+22*std::sin(swing)+slide,139-lift);
        capsule(hip,knee,25,true);capsule(knee,ankle,23,true);
        juce::Graphics::ScopedSaveState foot(g);
        g.addTransform(juce::AffineTransform::rotation(toe,ankle.x,ankle.y));
        oval({ankle.x-17,ankle.y-11,34,25});
        g.setColour(rim.withAlpha(.45f));g.drawLine(ankle.x-9,ankle.y+7,ankle.x+8,ankle.y+7,.8f);
    }

    {
        juce::Graphics::ScopedSaveState headState(g);
        g.addTransform(juce::AffineTransform::rotation((float)p.head,0,0));
        juce::Path head;
        head.startNewSubPath(-83,-68);head.lineTo(-61,-124);
        head.quadraticTo(-58,-132,-54,-125);head.lineTo(-25,-85);head.lineTo(25,-85);
        head.lineTo(54,-125);head.quadraticTo(58,-132,61,-124);head.lineTo(83,-68);
        head.cubicTo(115,-15,78,28,0,28);head.cubicTo(-78,28,-115,-15,-83,-68);head.closeSubPath();
        surface(head,{-95,-130,190,158});
        for(int side : {-1,1}) {
            juce::Path ear;ear.startNewSubPath(side*58.f,-111);ear.lineTo(side*70.f,-70);ear.lineTo(side*36.f,-78);ear.closeSubPath();
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xffbb327e).withAlpha(.65f),side*58.f,-109,
                                                 pink.withAlpha(.1f),side*50.f,-69,false));g.fillPath(ear);
        }
        // A broad, quiet highlight gives a curved surface without neon/glow.
        g.setGradientFill(juce::ColourGradient(juce::Colours::white.withAlpha(.12f),-40,-52,
                                              juce::Colours::white.withAlpha(0.f),32,24,true));
        juce::Graphics::ScopedSaveState clipped(g);g.reduceClipRegion(head);
        g.fillPath(head);
        const float eyeAlpha=juce::jlimit(0.f,1.f,(facing-.08f)*2.2f);
        for(float x : {-30.f,30.f}) {
            juce::Path eye;eye.startNewSubPath(x-14,-26);eye.cubicTo(x-9,-10,x+9,-10,x+14,-26);
            g.setColour(juce::Colour(0xffffc2e7).withAlpha(.7f*eyeAlpha));
            g.strokePath(eye,juce::PathStrokeType(8.5f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded),juce::AffineTransform::translation(0,1.4f));
            g.setColour(juce::Colour(0xff30202d).withAlpha(eyeAlpha));
            g.strokePath(eye,juce::PathStrokeType(7.5f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
        }
    }
    // Front layer: articulated upper arms + forearms. Their shaded contours remain
    // visible even when they cross the torso or lower face (unlike a flat silhouette).
    for(int side : {-1,1}) {
        const float angle=(float)(side<0?p.leftArm:p.rightArm);
        const auto rotate=[](juce::Point<float> v,float a) {
            return juce::Point<float>(v.x*std::cos(a)-v.y*std::sin(a),v.x*std::sin(a)+v.y*std::cos(a));
        };
        const juce::Point<float> shoulder(side*43.f,42);
        const auto elbow=shoulder+rotate({side*23.f,12},angle);
        const auto paw=elbow+rotate({side*19.f,-3},angle*1.35f);
        // Contact shadow underneath, then two separate round volumes and a paw.
        g.setColour(rim.withAlpha(.5f));g.fillEllipse(shoulder.x-12,shoulder.y-10,25,26);
        capsule(shoulder,elbow,22);
        capsule(elbow,paw,21);
        oval({paw.x-12,paw.y-12,24,25});
        g.setColour(juce::Colours::white.withAlpha(.2f));g.fillEllipse(paw.x-6,paw.y-7,8,5);
    }
}

#pragma once
#include <algorithm>
#include <cmath>

namespace dance {
constexpr double pi = 3.14159265358979323846;
constexpr const char* names[] = { "Head Nod", "Trap Bounce", "Two Step", "House", "Wave", "Spin", "Running Man", "Shuffle", "Griddy", "Floss", "Charleston", "Heel Toe" };
constexpr int presetCount=(int)(sizeof(names)/sizeof(names[0]));
struct Pose {
    double x=0, y=0, lean=0, head=0, leftArm=0, rightArm=0;
    double leftFoot=0, rightFoot=0, turn=1, squash=1, tail=0;
    double leftLeg=0,rightLeg=0,leftSlide=0,rightSlide=0,leftToe=0,rightToe=0;
};
// All phases come from musical quarter notes, not GUI frame count.
inline Pose pose(double beat, int preset, bool playing) noexcept {
    Pose p;
    if (!playing || !std::isfinite(beat)) return p;
    const double a = 2*pi*std::remainder(beat, 8.0);
    const double s = std::sin(a), c = std::cos(a), slow = std::sin(a/2);
    p.tail = 0.15*slow;
    switch (preset) {
        case 0: p.head=0.14*s; p.y=2*c; p.leftArm=.05*s; p.rightArm=-.05*s; break;
        case 1: p.y=-10*(1-c)/2; p.squash=1+.06*c; p.lean=.08*slow;
                p.leftArm=-.3-.22*s; p.rightArm=.3+.22*s; p.head=.1*s; break;
        case 2: p.x=14*slow; p.lean=.12*slow; p.y=-4*std::abs(s);
                p.leftFoot=8*std::max(0.0,slow); p.rightFoot=8*std::max(0.0,-slow);
                p.leftArm=.35*slow; p.rightArm=.35*slow; break;
        case 3: p.y=-14*(1-c)/2; p.x=6*slow; p.lean=.08*slow;
                p.leftArm=1.0+.35*s; p.rightArm=-1.0-.35*s;
                p.leftFoot=10*std::max(0.0,s); p.rightFoot=10*std::max(0.0,-s); break;
        case 4: p.lean=.12*slow; p.head=-.18*slow;
                p.leftArm=.8+.5*std::sin(a/2); p.rightArm=-.8+.5*std::sin(a/2-pi/2);
                p.y=-3*(1-c); break;
        case 5: p.turn=std::cos(a/4); p.y=-8*(1-c)/2;
                p.leftArm=.6; p.rightArm=-.6; p.head=.07*s; break;
        case 6: // Running Man: alternating raised knees and sliding support foot.
                p.y=-5*(1-c)/2;p.lean=.05*slow;
                p.leftFoot=20*std::max(0.0,slow);p.rightFoot=20*std::max(0.0,-slow);
                p.leftSlide=-15*std::cos(a/2);p.rightSlide=15*std::cos(a/2);
                p.leftLeg=.45*slow;p.rightLeg=-.45*slow;
                p.leftArm=.45*slow;p.rightArm=.45*slow;p.head=.06*s;break;
        case 7: // Shuffle / T-step: heel-toe pivots and alternating half-beat lifts.
                p.x=10*slow;p.y=-5*std::abs(s);p.lean=.07*slow;
                p.leftFoot=14*std::max(0.0,s);p.rightFoot=14*std::max(0.0,-s);
                p.leftSlide=10*c;p.rightSlide=-10*c;
                p.leftToe=.5*slow;p.rightToe=.5*slow;
                p.leftLeg=.25*s;p.rightLeg=-.25*s;
                p.leftArm=.25+.3*slow;p.rightArm=-.25+.3*slow;break;
        case 8: // Griddy-inspired heel taps and low arm pumps.
                p.y=-6*(1-c)/2;p.lean=.1*slow;p.head=.08*s;
                p.leftFoot=18*std::max(0.0,slow);p.rightFoot=18*std::max(0.0,-slow);
                p.leftSlide=10*std::sin(a/2+pi/3);p.rightSlide=-p.leftSlide;
                p.leftToe=-.25*slow;p.rightToe=.25*slow;
                p.leftArm=-.45+.35*slow;p.rightArm=.45+.35*slow;break;
        case 9: // Floss-inspired counter-swing of arms and hips.
                p.x=-8*slow;p.lean=-.12*slow;p.y=-3*(1-c)/2;
                p.leftArm=-.4+.9*slow;p.rightArm=.4+.9*slow;
                p.leftFoot=8*std::max(0.0,slow);p.rightFoot=8*std::max(0.0,-slow);
                p.leftLeg=.25*slow;p.rightLeg=.25*slow;p.head=.05*slow;break;
        case 10: // Charleston-style alternating kicks and toe swivels.
                p.y=-4*(1-c)/2;p.lean=.08*slow;
                p.leftSlide=16*slow;p.rightSlide=-16*slow;
                p.leftLeg=.6*slow;p.rightLeg=-.6*slow;
                p.leftFoot=12*std::max(0.0,slow);p.rightFoot=12*std::max(0.0,-slow);
                p.leftToe=.45*s;p.rightToe=-.45*s;
                p.leftArm=.35*slow;p.rightArm=.35*slow;break;
        case 11: // Heel Toe: paired ankle pivots with opposing foot travel.
                p.x=5*slow;p.lean=.08*slow;p.head=-.08*slow;
                p.leftSlide=-11*slow;p.rightSlide=11*slow;
                p.leftToe=.55*slow;p.rightToe=.55*slow;
                p.leftFoot=8*(1+c)/2;p.rightFoot=8*(1-c)/2;
                p.leftLeg=.28*slow;p.rightLeg=.28*slow;
                p.leftArm=.3+.18*slow;p.rightArm=-.3+.18*slow;break;
        default: break;
    }
    return p;
}
inline double rate(int choice) noexcept { return choice == 0 ? .5 : choice == 2 ? 2.0 : 1.0; }
inline double extrapolate(double ppq, double bpm, double ageMs, bool playing) noexcept {
    if (!std::isfinite(ppq)) return 0;
    if (!playing || !std::isfinite(bpm) || !std::isfinite(ageMs)) return ppq;
    return ppq + std::clamp(ageMs, 0.0, 150.0)*std::clamp(bpm, 1.0, 999.0)/60000.0;
}
}

#include "Motion.h"
#include "Meter.h"
#include <stdexcept>
#define CHECK(x) do { if (!(x)) throw std::runtime_error(#x); } while (false)
#include <iostream>
#include <limits>
int main() {
    const double unavailable = std::numeric_limits<double>::quiet_NaN();
    const auto commonTime = dance::Meter{};
    const auto waltz = dance::Meter::fromHost(3, 4);
    const auto sixEight = dance::Meter::fromHost(6, 8);
    CHECK(commonTime.beatInBar(3.5, unavailable) == 3);
    CHECK(waltz.beatInBar(3, unavailable) == 0);
    CHECK(waltz.beatInBar(-.125, unavailable) == 2);
    // A host meter change can put a downbeat off the original timeline grid.
    CHECK(waltz.beatInBar(8.0, 8.0) == 0);
    CHECK(waltz.beatInBar(9.25, 8.0) == 1);
    CHECK(waltz.beatInBar(10.9, 8.0) == 2);
    CHECK(waltz.beatInBar(11.0, 8.0) == 0);
    CHECK(sixEight.beatInBar(10.5, 8.0) == 5);
    CHECK(sixEight.beatInBar(11.0, 8.0) == 0);
    CHECK(dance::Meter::fromHost(0, 8).numerator == 4);
    CHECK(dance::Meter::fromHost(6, 0).denominator == 4);
    CHECK(waltz.beatInBar(unavailable, 8.0) == 0);
    // One quarter note = 500 ms at 120 BPM, 1000 ms at 60 BPM.
    CHECK(std::abs(dance::extrapolate(4,120,100,true)-4.2)<1e-10);
    CHECK(std::abs(dance::extrapolate(4,60,100,true)-4.1)<1e-10);
    CHECK(dance::extrapolate(7,160,100,false)==7);
    CHECK(std::abs(dance::extrapolate(0,120,9999,true)-.3)<1e-10);
    CHECK(dance::extrapolate(std::numeric_limits<double>::quiet_NaN(),120,0,true)==0);
    CHECK(dance::rate(0)==.5 && dance::rate(1)==1 && dance::rate(2)==2);
    for(int preset=0;preset<dance::presetCount;++preset) {
        for(double b=-32;b<32;b+=.03125) {
            const auto a=dance::pose(b,preset,true), loop=dance::pose(b+8,preset,true);
            const double v[]={a.x,a.y,a.lean,a.head,a.leftArm,a.rightArm,a.leftFoot,a.rightFoot,a.turn,a.squash,a.tail,a.leftLeg,a.rightLeg,a.leftSlide,a.rightSlide,a.leftToe,a.rightToe};
            for(auto x:v) CHECK(std::isfinite(x));
            const double w[]={loop.x,loop.y,loop.lean,loop.head,loop.leftArm,loop.rightArm,loop.leftFoot,loop.rightFoot,loop.turn,loop.squash,loop.tail,loop.leftLeg,loop.rightLeg,loop.leftSlide,loop.rightSlide,loop.leftToe,loop.rightToe};
            for(unsigned i=0;i<sizeof(v)/sizeof(v[0]);++i)CHECK(std::abs(v[i]-w[i])<1e-8);
        }
        const auto stop=dance::pose(123,preset,false);
        CHECK(stop.y==0 && stop.leftArm==0 && stop.turn==1);
    }
    // At the same musical position tempo does not change choreography;
    // seeks and loop restarts have no accumulated frame/time drift.
    const auto a=dance::pose(2.375,3,true), b=dance::pose(2.375+8,3,true);
    CHECK(std::abs(a.leftArm-b.leftArm)<1e-8);
    CHECK(dance::pose(0,0,true).y != dance::pose(0,1,true).y);
    // Every added choreography must actually move the feet/legs, not just arms.
    for(int preset=6;preset<dance::presetCount;++preset) {
        const auto a=dance::pose(.125,preset,true),b=dance::pose(1.125,preset,true);
        CHECK(std::abs(a.leftFoot-b.leftFoot)+std::abs(a.leftSlide-b.leftSlide)+std::abs(a.leftLeg-b.leftLeg)+std::abs(a.leftToe-b.leftToe)>.1);
    }
    std::cout << "Meter, timing, 12 choreographies and loop periodicity OK\n";
}

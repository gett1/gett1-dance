#pragma once
#include <cmath>

namespace dance {
struct Meter {
    int numerator = 4;
    int denominator = 4;

    static Meter fromHost(int numerator, int denominator) noexcept {
        if (numerator <= 0 || denominator <= 0)
            return {};
        return {numerator, denominator};
    }

    int beatInBar(double ppq, double barStartPpq) const noexcept {
        const auto offset = ppq - (std::isfinite(barStartPpq) ? barStartPpq : 0.0);
        const auto quarterNotesPerBeat = 4.0 / denominator;
        if (!std::isfinite(offset))
            return 0;

        const auto barLength = numerator * quarterNotesPerBeat;
        const auto phase = std::fmod(offset, barLength);
        // Round down before wrapping, so negative PPQ counts back from the last beat.
        const auto beat = static_cast<int>(std::floor(phase / quarterNotesPerBeat));
        return beat < 0 ? beat + numerator : (beat == numerator ? 0 : beat);
    }
};
}

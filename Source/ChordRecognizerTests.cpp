#include <JuceHeader.h>
#include "ChordRecognizer.hpp"

#include <cmath>
#include <vector>
#include <string>

namespace
{
    constexpr double kSampleRate = 44100.0;
    constexpr int    kFftSize    = 8192;
    constexpr int    kNumBins    = kFftSize / 2;

    int frequencyToBin(double frequency)
    {
        return (int) std::round(frequency * (double) kFftSize / kSampleRate);
    }

    std::vector<float> makeEmptySpectrum()
    {
        return std::vector<float>((size_t) kNumBins, 0.0f);
    }

    void addPeak(std::vector<float>& magnitudes, double frequency, float amplitude = 1.0f)
    {
        const int bin = frequencyToBin(frequency);

        if (bin > 0 && bin < (int) magnitudes.size())
        {
            magnitudes[(size_t) bin] += amplitude;

            // Add a tiny bit of energy to neighboring bins so the test input
            // looks a bit less artificial.
            if (bin - 1 > 0)
                magnitudes[(size_t) (bin - 1)] += amplitude * 0.15f;
            if (bin + 1 < (int) magnitudes.size())
                magnitudes[(size_t) (bin + 1)] += amplitude * 0.15f;
        }
    }

    void addChordTones(std::vector<float>& magnitudes,
                       std::initializer_list<double> frequencies,
                       float amplitude = 1.0f)
    {
        for (double f : frequencies)
            addPeak(magnitudes, f, amplitude);
    }

    ChordResult runRecognizer(const std::vector<float>& magnitudes)
    {
        ChordRecognizer recognizer(kSampleRate, kFftSize);
        recognizer.update(magnitudes);

        ChordResult result;
        const bool gotResult = recognizer.popResult(result);
        jassert(gotResult);
        return result;
    }
}

class ChordRecognizerTests : public juce::UnitTest
{
public:
    ChordRecognizerTests() : juce::UnitTest("ChordRecognizer", "Audio") {}

    void runTest() override
    {
        beginTest("Silence returns N/C");
        {
            auto magnitudes = makeEmptySpectrum();
            auto result = runRecognizer(magnitudes);

            expectEquals(result.chordName, std::string("N/C"));
            expectEquals(result.rootNote, -1);
            expectWithinAbsoluteError(result.confidence, 0.0f, 1.0e-6f);
        }

        beginTest("Recognizes C major");
        {
            auto magnitudes = makeEmptySpectrum();

            // C4, E4, G4
            addChordTones(magnitudes, {261.63, 329.63, 392.00});

            auto result = runRecognizer(magnitudes);

            expectEquals(result.chordName, std::string("Cmaj"));
            expectEquals(result.rootNote, 0);
            expectGreaterThan(result.confidence, 0.75f);
        }

        beginTest("Recognizes A minor");
        {
            auto magnitudes = makeEmptySpectrum();

            // A3, C4, E4
            addChordTones(magnitudes, {220.00, 261.63, 329.63});

            auto result = runRecognizer(magnitudes);

            expectEquals(result.chordName, std::string("Amin"));
            expectEquals(result.rootNote, 9);
            expectGreaterThan(result.confidence, 0.75f);
        }

        beginTest("Recognizes G dominant 7");
        {
            auto magnitudes = makeEmptySpectrum();

            // G3, B3, D4, F4
            addChordTones(magnitudes, {196.00, 246.94, 293.66, 349.23});

            auto result = runRecognizer(magnitudes);

            expectEquals(result.chordName, std::string("Gdom7"));
            expectEquals(result.rootNote, 7);
            expectGreaterThan(result.confidence, 0.75f);
        }

        beginTest("Recognizes D sus4");
        {
            auto magnitudes = makeEmptySpectrum();

            // D4, G4, A4
            addChordTones(magnitudes, {293.66, 392.00, 440.00});

            auto result = runRecognizer(magnitudes);

            expectEquals(result.chordName, std::string("Dsus4"));
            expectEquals(result.rootNote, 2);
            expectGreaterThan(result.confidence, 0.75f);
        }

        beginTest("One update produces one readable result");
        {
            ChordRecognizer recognizer(kSampleRate, kFftSize);
            auto magnitudes = makeEmptySpectrum();
            addChordTones(magnitudes, {261.63, 329.63, 392.00});

            recognizer.update(magnitudes);

            ChordResult first;
            ChordResult second;

            expect(recognizer.popResult(first));
            expect(! recognizer.popResult(second));
            expectEquals(first.chordName, std::string("Cmaj"));
        }
    }
};

static ChordRecognizerTests chordRecognizerTests;

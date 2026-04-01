# Effect Explanations

## Controls (shared across all effects)

| Knob | Range |
|---|---|
| Rate | 0.1 – 2.0 |
| Depth | 0.0 – 1.0 |
| Feedback | -0.25 – +0.25 |

Each effect repurposes these three knobs differently. See the per-effect sections below.

---

## 1. Chorus

A standard single-voice chorus. A short, modulated delay is mixed with the dry signal. The delay time oscillates slowly via an internal LFO, causing subtle pitch fluctuation that thickens the sound — similar to the natural detuning between two performers playing the same part.

**How it affects the frequency spectrum:**
The slight pitch oscillation causes each frequency component to waver slightly up and down over time. This does not create new frequencies; it smears existing ones across a narrow band, producing a warm, slightly out-of-tune character.

| Knob | Effect |
|---|---|
| Rate | LFO speed — how fast the delay time oscillates (Hz) |
| Depth | LFO modulation depth — how far the delay time swings, controlling the width of the pitch fluctuation |
| Feedback | Amount of the delayed signal fed back into the delay line — increases resonance and sustain |

---

## 2. Multi-Voice Chorus

Six independent chorus voices running in parallel. Each voice processes the dry input signal separately with its own slightly different LFO rate, modulation depth, delay time, and feedback. The voices are spread across the stereo field using equal-power panning (voice 0 hard left, voice 5 hard right, others evenly distributed between).

The LFO phases are staggered at startup so each voice is always at a different point in its modulation cycle, preventing the voices from moving in sync.

**How it affects the frequency spectrum:**
Each voice independently smears the frequency content in slightly different directions and at different rates. Combined, they produce a dense, wide, evolving spectral texture. On a chord, each note's frequencies are each modulated by six independent voices simultaneously, creating a much richer shimmer than single-voice chorus.

| Knob | Effect |
|---|---|
| Rate | Base LFO speed — each voice offsets from this by a fixed amount, so all six LFOs run at different but related speeds |
| Depth | Base modulation depth — each voice offsets slightly, so voices have different flutter widths |
| Feedback | Base feedback — spread slightly across voices for added tonal variation |

---

## 3. Ring Modulator

Multiplies the input signal sample-by-sample by a sine wave carrier oscillator. Mathematically, multiplying by a sine at frequency `f_c` produces two new frequencies for every input frequency `f_in`: an upper sideband at `f_in + f_c` and a lower sideband at `f_in - f_c`. The original frequency is suppressed.

On a chord, every note's harmonics are each replaced by a pair of sidebands. Because the sideband frequencies are determined by addition and subtraction rather than multiplication, they do not preserve the harmonic relationships of the original chord — the result is inharmonic and metallic in character.

A feedback path feeds a portion of the modulated output back into the input before modulation. This causes the feedback signal to be modulated again on the next cycle, creating additional sidebands from the sidebands. The `std::tanh` saturation function is applied to the feedback signal to keep it smoothly bounded and add a warm, saturated character even at high feedback settings.

2x oversampling is applied internally to prevent aliasing artefacts that would otherwise occur when sideband frequencies exceed the Nyquist limit.

| Knob | Effect |
|---|---|
| Rate | Carrier frequency — remapped exponentially from 20 Hz (deep, subtle) to 2000 Hz (high, metallic). At low values the effect is felt as a low-frequency tremolo or timbral shift; at higher values it produces clearly audible inharmonic sidebands |
| Depth | Wet/dry mix — 0.0 is fully dry (no effect), 1.0 is fully ring-modulated (only sidebands, no original signal) |
| Feedback | Feedback gain into the modulator — 0 is no feedback; increasing adds resonance and spectral complexity as modulated output is re-modulated on each cycle |

---

## 4. Harmonic Ring Mod

An enhanced ring modulator designed to produce richer, more animated spectral effects on chords. It combines three features not present in the basic ring mod:

**Triangle wave carrier:**
Instead of a pure sine, the carrier is a triangle wave. A triangle wave contains only odd harmonics (1×, 3×, 5×, 7×... of the carrier frequency) with amplitudes that fall off as 1/n². This means each input frequency produces multiple pairs of sidebands — one pair per harmonic of the carrier — rather than just one pair. The sidebands at higher carrier harmonics have lower amplitude, so the effect is rich but not overwhelmingly dense. The 1/n² rolloff also means less aliasing than a square or sawtooth wave, even before oversampling.

**AM/RM blend:**
Pure ring modulation completely suppresses the original frequencies — on a chord, the notes themselves disappear and are replaced by inharmonic sideband clusters. This is tonally dramatic but can make chords unrecognisable. The AM/RM blend mixes a DC offset into the carrier, which re-introduces the original frequencies alongside the sidebands. At full AM the chord is preserved and the sidebands appear as additional spectral colouration on top of it.

**LFO-swept carrier frequency:**
The carrier frequency is modulated by a slow LFO running at a fixed 0.3 Hz. As the carrier sweeps, all of its harmonic sidebands sweep together in a coordinated pattern. Because the sidebands are harmonically related to each other (all multiples of the same carrier), the sweep sounds structured and musical rather than chaotic — the entire sideband cluster rises and falls together.

**Stereo quadrature:**
The right channel uses a carrier that leads the left by 90° (a quarter cycle). This means the left and right channels always have different spectral content without requiring multiple voices, creating stereo width from a single carrier at no extra CPU cost.

2x oversampling is applied internally, shared with the Ring Modulator, to suppress aliasing from the triangle wave's harmonic content.

| Knob | Effect |
|---|---|
| Rate | Base carrier frequency — exponentially remapped from 20 Hz to 2000 Hz, same as the Ring Modulator. Determines where the sideband clusters sit in the spectrum |
| Depth | AM/RM blend — 0.0 is pure ring modulation (original frequencies suppressed, only sidebands remain); 1.0 is pure amplitude modulation (original chord preserved, sidebands appear as added spectral colour). Values between blend the two |
| Feedback | LFO sweep depth — 0.0 leaves the carrier frequency static; increasing causes the carrier to oscillate ±(feedback × 50%) around the base frequency at 0.3 Hz, animating the sideband clusters over time |

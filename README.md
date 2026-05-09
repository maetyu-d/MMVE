MMVE

TL;DR

The Mighty Morphing Verb Echo (MMVE) is a spatial effect plugin that can morph between echo and reverb using multiple (up to 32) feedback paths. There are controls for path count, size, spread, matrix routing, early/late balance, coupling, skew, pitch-shifted octave lines, freeze, tone, and sync, and it's possible to script every parameter individually using the Fabric scripting language. Built using JUCE, it is currently available in AU and VST3 formats.

Windows VST3 build notes are in [WINDOWS_BUILD.md](WINDOWS_BUILD.md). On Windows the CMake project defaults to building the VST3 target only.

<img width="1010" height="673" alt="Screenshot 2026-05-08 at 23 29 13" src="https://github.com/user-attachments/assets/651dcc24-c03f-41a1-8137-4ffc943ad2c4" />

Longer Description

MMVE is designed as a morphing space/echo instrument rather than a conventional reverb or delay. Its core sound comes from a continuously variable multi-path feedback network where the Paths control changes the density of feedback paths, Size and optional tempo Sync/Div define the time geometry, Spread introduces disorder which makes clean echoes dissolve into irregular spatial texture, Matrix changes how the paths feed and scatter into one another (intermingle), and Early/Late shifts the emphasis between immediate reflection-like taps and a deeper reverberant cloud. The effect also includes stereo shaping through Coupling and Skew controls, feedback and freeze controls, damping/low-cut/air tone shaping, octave-up and octave-down pitch delay layers, a wet/dry mix control, output safety limiting, and a per-parameter scripting system based on the simple, human-readable Fabric scripting language (see below). This enables envelopes to be created with random, hold, sine, and wander-shaped motion, plus there is script depth and error feedback.

<img width="986" height="664" alt="Screenshot 2026-05-08 at 23 30 42" src="https://github.com/user-attachments/assets/b1260714-52d5-4a96-9074-e45cbe7817de" />



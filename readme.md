# Pitch Effect Analyzer Plugin

This plugin is being created as a class project for the CSC475 course at the University of Victoria.
The goal of this plugin is to create a DAW plugin to apply a pitch based audio effect and analyze both the input and output.
This pitch based effect could be chorus, phaser, flanger, or harmonizer depending on what the project group is most interested in (can be multiple effects - time permitting).
The analysis would entail determining the frequency spectrum and the chord/note of both the input and the output signals.
This would serve to show how the applied effect alters the frequency content of the signal.
Additionally, functionality could be added to allow the plugin to fill out the frequency spectrum based on the frequency content of the output signal.

## Installation

No installation media has been created for this plugin yet, but it will be available soon.

To build this project yourself, do the following. Ensure that Visual Studio 2022 is installed for C++ development. Ensure that JUCE version 8.0.3 is installed. A tutorial to setup JUCE for Windows can be found in this video: https://www.youtube.com/watch?v=Mo0Oco3Vimo&pp=ygUHI2p1Y2V2Nw%3D%3D between timestamps 22:40 – 45:49. Once this is set up simply open the 484Compressor.jucer file in the Projucer application and click "Save and open in IDE". Once the Visual Studio IDE opens set the 484Compressor_VST3 solution as the startup project and build the solution. The solution can be built by navigating to the "Build" tab and clicking "Build Solution" or using the keyboard shortcut Ctrl+Shift+B. Building this project should copy the files to the proper directory. If any errors occur with copying the files it is most likely due to the permissions of the folder, these errors can be remedied by setting the permissions as done in the video above.

## How to Use

Following the previously mentioned video should set up any machine to run and test this plugin using the .filtergraph file and the AudioPluginHost. This does require the CSC475-pitch_effect-analyzer_VST3 to be configured properly (as shown in the videos) and to be set as the Startup Project.

Currently plugin project has been initialized but no work has been done to add any functionality. This work will take place over the next 3 months.

## System Requirements - Initial (May be changed as further testing occurs)

- **Operating System**: Windows 10+
- **DAW Compatibility**: Supports VST3 format.
- **Processor**: Dual-core CPU or better.
- **RAM**: 4 GB or more recommended.

## License

484Compressor is free to use for personal and commercial projects. Redistribution or modification of the plugin is subject to the terms of the LICENSE file.

## Acknowledgments

This plugin was developed using the [JUCE framework](https://juce.com), which powers countless professional-grade audio tools.
This plugin was developed using algorithms and principles derived from the following textbooks:

- **"Audio Effects: Theory, Implementation and Application" by Joshua D. Reiss and Andrew P. McPherson**: This book provided foundational concepts and advanced techniques for implementing audio effects.
- **"DAFX: Digital Audio Effects" by Udo Zölzer**: A comprehensive resource that informed the design and implementation of both the compressor and distortion effects.

## Contact

For feedback, suggestions, or issues, please reach out to charlie.wagerr@gmail.com.

## Note

This plugin is built with JUCE version 8.0.3 and is currently only tested for Windows machines, if you would like to clone this and build it yourself please ensure that version 8.0.3 of JUCE is installed and a Windows machine is used in order to avoid errors.

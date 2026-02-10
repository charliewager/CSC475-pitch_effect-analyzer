# PITCH EFFECT ANALYSER

Charlie Wager V00959352
Connor Newbery V00921506
Armaan Chhina V00951334
Owen O'Keefe V01008501  


## Abstract

This project develops a real-time DAW plugin that combines a chorus effect with built-in pitch and chord analysis. Using the JUCE framework in C++, the plugin visualizes how the chorus effect alters the harmonic and frequency content of an input signal. By comparing the input and output through spectrograms and chord recognition, users gain a clearer understanding of how pitch-based modulation shapes sound. The system draws on music information retrieval (MIR) techniques to bridge the gap between audio effect design and real-time musical analysis, offering both a creative and educational tool for music producers.

## 1. INTRODUCTION AND PROJECT DESCRIPTION

Pitch-based audio effects are a core part of modern music production. Effects such as chorus, flanger, phaser, and harmonizer can significantly alter the sound of an instrument, yet producers typically rely on listening alone to understand what these effects are doing to the signal [1].

At the same time, music information retrieval (MIR) research has produced many reliable methods for frequency analysis, pitch tracking, and chord recognition [2]. These methods are usually applied offline or in analysis tools, rather than being integrated directly into creative audio software.

This project aims to bridge that gap. We explore how MIR techniques can be embedded into a DAW plugin to analyze and visualize the impact of pitch-based effects in real time [3]. By comparing the input and output signals, users can better understand how an effect changes the harmonic structure of their audio.

## 2. OBJECTIVES AND TIMELIME

The objective of this project is to build a DAW plugin that applies a pitch based audio effect and analyzes how the effect changes the signal. The plugin will compare the input and output audio by visualizing their frequency content and identifying the chord or note present, helping users better understand how pitch based effects alter sound.

The project will progress in clear stages. By the middle of February, the first pitch based audio effect will be implemented and working in real time. By the end of February, frequency analysis will be added along with spectrograms for both the input and output signals. By the middle of March, the user interface will be refined and finalized, with additional effects added if time permits. By the end of March, chord and note recognition will be completed and integrated, followed by final testing and polish.

## 3. TEAM MEMBER ROLES

Charlie will focus on developing the pitch based audio effect and will act as the technical supervisor, helping guide overall implementation decisions and integration.

Owen and Connor will work together on the chord recognition component, including note detection and mapping the detected pitches to chords.

Armaan will be responsible for building the spectrogram visualization and the related GUI elements for displaying the input and output analysis.

While each member has a primary focus, the project will be collaborative. Everyone will contribute across areas and help each other as needed.

## 3.1. Charlie Wager

- Objective: Create JUCE project, set up and test Plugin.
  - P1 (basic): intitialize JUCE plugin.
  - P2 (basic): ensure all group members can run the plugin.
  - P3 (expected): test plugin in multiple DAWs.
  - P4 (advanced): create installation media for windows.
  - P5 (advanced): create installation media for Mac.
  
- Objective: Design pitch-based audio effect(s).
  - P1 (basic): implement a chorus effect.
  - P2 (expected): add multiple voices to chorus effect.
  - P3 (expected): implement a second pitch-based effect.
  - P4 (advanced): add complexity to the second effect.
  - P5 (advanced): add a third pitch-based audio effect.

## 3.2. Owen O'Keefe
- Objective: Implement Chord Recognition model
  - P1 (basic): evaluate tensorflow's feasibility for this project for chord recognition
  - P2 (expected): learn tensorflow and how we can implement it for the chord recognition
  - P3 (expected): train and test model
  - P4 (advanced): fully implement model
  - P5 (advanced): evaluate accuracy of model

- Objective: Learn Tensorflow
  - P1 (basic): learn tensorflow in C++
  - P2 (basic): learn how to use TensorFlow for chord recognition
  - P3 (expected): learn optimization techniques
  - P4 (advanced): implement optimization techniques
  - P5 (advanced): test and check the chord recogntion code

## 3.3. Armaan Chhina
- Objective: Implement a real-time spectrogram visualization for input and output audio in a JUCE plugin
  - PI1 (basic): compute and display a real-time FFT magnitude spectrum for the input signal  
  - PI2 (basic): compute and display a real-time FFT magnitude spectrum for the output signal  
  - PI3 (expected): implement a scrolling spectrogram view showing frequency content over time  
  - PI4 (expected): validate spectrogram accuracy using test tones and live instrument input  
  - PI5 (advanced): add frequency labels and peak highlighting to improve interpretability  

- Objective: Ensure spectrogram performance and stability in real-time audio environments
  - PI1 (basic): transfer audio data from the audio thread to the GUI thread without blocking  
  - PI2 (basic): ensure the spectrogram runs in real time at standard buffer sizes and sample rates  
  - PI3 (expected): test spectrogram behavior across multiple DAWs  
  - PI4 (expected): evaluate CPU usage and optimize FFT and rendering where needed  
  - PI5 (advanced): implement a freeze or snapshot feature to capture spectrogram output  

## 3.4 Connor Newbery
- Objective: Read associated literature
  - P1 (basic): Collect literature related to non-machine learning algorithms for chord recognition.
  - P2 (basic): Read and synthesize findings on non-machine learning algorithms.
  - P3 (expected): Collect literature related to machine learning algorithms for chord recognition.
  - P4 (advanced): Read and synthesize findings on machine learning algorithms.
  - P5 (advanced): Determine most effective algorithms from literature.
 
- Objective: Design Chord Recognition Algorithm
  - P1 (basic): Set up environment and determine appropriate languages and frameworks for test-algorithm development.
  - P2 (basic): Distill theoretical concepts, determine which are most applicable for our purposes.
  - P3 (expected): Basic implementations of algorithms from literature.
  - P4 (advanced): Customize algorithms.
  - P5 (advanced): Implement test version (proof of concept) / Pass on to owen.
 
## 4. TOOL DESCRIPTION

We will use C++ and the JUCE framework to develop this project. JUCE is a C++ framework for developing VST and AU audio plugins. The JUCE framework contains built-in functions for digital signal processing (DSP), including FFT, convolution, windowing, filtering and delay functions (cite JUCE docs for dsp). In addition to this JUCE provides tools for creating GUIs and project generation via Projucer or CMake. Any DSP functions or audio effects that are needed and not provided as functions by the JUCE framework will be implemented using information from the following textbooks: Designing Audio Effect Plugins in C++[4], DAFX[1], and Audio effects theory, implementation and application [5].

For the purposes of this project and for ease of development, the Visual Studio, or XCode programs with Projucer will be used to generate, build, and debug the VST plugin. The use of XCode or Visual Studio will be dependent on the individual operating system, as Visual Studio is not available on MacOS.

Projucer is JUCE’s graphical project management tool that lets you create and configure JUCE‑based applications and plug‑ins, then export platform‑specific IDE projects (Xcode, Visual Studio, Makefiles) for building and debugging.

Initial development level testing and debugging will be completed using the AudioPluginHost from JUCE. AudioPluginHost is a lightweight JUCE‑based host application included with the framework that you compile and then use to load, connect, and test audio plug‑ins (including your own) without needing a full DAW. Final testing of each feature will be completed in DAWs chosen by the project group. Preliminarily, these DAWs will be chosen as Audacity, FL Studio, and Logic Pro as the project group has pre-existing access and experience with these.

## 5. DATA SET DESCRIPTION

The main “data set” that will be used to test the audio effect and frequency spectrum analysis will be live audio generated by the project group. This live audio will be generated either through playing instruments, such as a guitar, and routing this audio into a DAW or by using a software instrument inside a DAW.

The main data set that will be used to test the chord recognition algorithm will be sets of audio with chord annotations such as isophonics [6], Chordify [7], Choco [8], and the Jazz Audio Aligned Harmony Dataset [9]. The final, and most rigorous, test of the chord recognition algorithm will use live audio. Just as with the “data set” for testing the audio effect and frequency spectrum analysis, this will be in the form of a guitar or other instrument that is routed into a DAW. The difference between this “data set” and that mentioned above is that, in this “data set”, the instrument will only ever play chords.

## 6. ASSOCIATED WORK AND LITERATURE

The following list of references are some of the research articles we will be using for determining the appropriate chord recognition algorithms to implement. Some of these articles may not be referenced in our final report, and some articles that have not been listed here may appear later on as we do more research on the topic.

[1] J. Osmalsky, J.-J. Embrechts, V. D. Marc, and S. Pierard, “Neural networks for musical chords recognition,” May 01, 2012. https://orbi.uliege.be/handle/2268/115963#details 

[2] M. McVicar, Y. Ni, R. Santos-Rodriguez, and T. De Bie, “Using online chord databases to enhance chord recognition,” Journal of New Music Research, vol. 40, no. 2, pp. 139–152, Jun. 2011, doi: 10.1080/09298215.2011.573564.
 
[3] N. Boulanger-Lewandowski Jr., Y. Bengio, P. Vincent, and Dept. IRO, Universit´e de Montr´eal, “AUDIO CHORD RECOGNITION WITH RECURRENT NEURAL NETWORKS,” Dept. IRO, Universit´E De Montr´Eal, 2013, [Online]. Available: https://ismir2013.ismir.net/wp-content/uploads/2013/09/243_Paper.pdf 

[4] “A fully convolutional deep auditory model for musical chord recognition,” IEEE Conference Publication | IEEE Xplore, Sep. 01, 2016. https://ieeexplore.ieee.org/abstract/document/7738895 

[5] K. Lee, “Automatic Chord Recognition from Audio Using Enhanced Pitch Class Profile,” journal-article. [Online]. Available: https://ccrma.stanford.edu/~kglee/pubs/klee-icmc06.pdf 

[6] “Chord recognition using measures of fit, chord templates and filtering methods,” IEEE Conference Publication | IEEE Xplore, Oct. 01, 2009. https://ieeexplore.ieee.org/abstract/document/5346546 

[7] “MINIMUM CLASSIFICATION ERROR TRAINING TO IMPROVE ISOLATED CHORD RECOGNITION,” 2009. [Online]. Available: https://archives.ismir.net/ismir2009/paper/000119.pdf 

[8] L. Oudre, “Template-based chord recognition from audio signals,” Nov. 03, 2010. https://theses.hal.science/tel-00559008/ 

[9] “Probabilistic Template-Based chord recognition,” IEEE Journals & Magazine | IEEE Xplore, Nov. 01, 2011. https://ieeexplore.ieee.org/abstract/document/5664772

[10] Z. Rao, X. Guan, and J. Teng, “Chord Recognition Based on Temporal Correlation Support Vector Machine,” Applied sciences, vol. 6, no. 5, p. 157, 2016, doi: 10.3390/app6050157.

[11] K. Noland and M. Sandler, “Influences of Signal Processing, Tone Profiles, and Chord Progressions on a Model for Estimating the Musical Key from Audio,” Computer music journal, vol. 33, no. 1, pp. 42–56, 2009, doi: 10.1162/comj.2009.33.1.42.

[12] F. Korzeniowski and G. Widmer, “A Fully Convolutional Deep Auditory Model for Musical Chord Recognition,” 2016, doi: 10.48550/arxiv.1612.05082.

[13] L. Oudre, C. Fevotte, and Y. Grenier, “Probabilistic Template-Based Chord Recognition,” IEEE transactions on audio, speech, and language processing, vol. 19, no. 8, pp. 2249–2259, 2011, doi: 10.1109/TASL.2010.2098870.

[14] D. Wei, “Keynote-Dependent HMM Based Musical Chord Recognition Method,” Journal of software, vol. 8, no. 4, pp. 916–916, 2013, doi: 10.4304/jsw.8.4.916-923.

[15] R. Hernández, A. Guerrero, and J. E. Macías-Díaz, “A template-based algorithm by geometric means for the automatic and efficient recognition of music chords,” Evolutionary intelligence, vol. 17, no. 1, pp. 467–481, 2024, doi: 10.1007/s12065-022-00771-6.

## 7. REFERENCES

[1]U. Zolzer, “DAFX -Digital Audio Effects DAFX: Digital Audio Effects.” Available: http://oeyvind.teks.no/ftp/Projects/Projects/writings/2015/DAFx/ref/dafx_book.pdf

[2]L. Oudre, Y. Grenier, and C. Févotte, “TEMPLATE-BASED CHORD RECOGNITION : INFLUENCE OF THE CHORD TYPES,” 2009. Accessed: Jan. 28, 2026. [Online]. Available: https://ismir2009.ismir.net/proceedings/PS1-17.pdf

[3]D. Stefani and L. Turchet, “On the Challenges of Embedded Real-time Music Information Retrieval Augmentation of Traditional Italian Instruments View project Real-time detection of symbolic monophonic musical patterns View project ON THE CHALLENGES OF EMBEDDED REAL-TIME MUSIC INFORMATION RETRIEVAL,” 2022. Accessed: Jan. 28, 2026. [Online]. Available: http://www.lucaturchet.it/PUBLIC_DOWNLOADS/publications/conferences/On_the_Challenges_of_Embedded_Real-Time_Music_Information_Retrieval.pdf

[4] W. C. Pirkle, Designing Audio Effect Plugins in C++. Routledge, 2019.

[5] J. D. Reiss and A. P. Mcpherson, Audio effects theory, implementation and application. Boca Raton London New York Crc Press, Taylor & Francis Group, 2015.

[6] “Reference and annotations — The Beatles,” Isophonics, 2010. [Online]. Available: http://isophonics.net/content/reference-annotations-beatles. [Accessed: Jan. 31, 2026].

[7] “Chordify Annotator Subjectivity Dataset” GitHub, 2019. [Online]. Available: https://github.com/chordify/CASD. [Accessed: Jan. 31, 2026].

[8] “ChoCo: the Chord Corpust” GitHub, 2025. [Online]. Available:https://github.com/smashub/choco. [Accessed: Jan. 31, 2026].

[9] “Jazz Audio-Aligned Harmony (JAAH) Dataset” GitHub, 2022. [Online]. Available:https://github.com/MTG/JAAH. [Accessed: Jan. 31, 2026].

# PolarDesigner
Our open-source plug-in developed by [Thomas](https://github.com/thomasdeppisch), [Simon](https://github.com/becksimon), [IEM](https://iem.kug.ac.at/en/) and [AA](https://austrian.audio/).

PolarDesigner allows you to control the polar pattern of your OC818 microphone in up to five frequency bands.  
Available as VST3, AAX and AU. Get installers for Windows and Mac at [austrian.audio](https://austrian.audio/).

<img width="990" alt="Screenshot_PD" src="https://user-images.githubusercontent.com/70842242/117307438-dce56480-ae80-11eb-8952-5d386b3285b8.png">

## Building PolarDesigner
PolarDesigner is based on [JUCE](https://juce.com/). To build PolarDesigner, get a recent version of JUCE and open PolarDesigner.jucer in Projucer. Select an exporter of your choice (e.g. Visual Studio or XCode) to create and open a project file in your IDE.

## Requirements
* FFTW: PolarDesigner links FFTW statically. Download [FFTW](http://fftw.org/) and put header and lib files in /resources/fftw_win/ and resources/fftw_osx/.
* For building AAX plugins you need to add the [AAX SDK](http://developer.avid.com/) location to your Projucer paths.

## Related repositories
Parts of the code are based on the [IEM Plugin Suite](https://git.iem.at/audioplugins/IEMPluginSuite) - check it out, it's awesome!

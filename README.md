# PolarDesigner
Our open-source plug-in developed by [Thomas](https://github.com/thomasdeppisch), [Simon](https://github.com/becksimon), [IEM](https://iem.kug.ac.at/en/) and [AA](https://austrian.audio/).

PolarDesigner allows you to control the polar pattern of your OC818 microphone in up to five frequency bands.  
Available as VST3, AAX and AU. Get installers for Windows and Mac at [austrian.audio](https://austrian.audio/).

<img width="990" alt="Screenshot_PD" src="https://github.com/AustrianAudioGmbH/PolarDesigner/blob/master/docs/PolarDesigner.png">
Image: PolarDesigner running on iPad ...

## Requirements
* A copy of the JUCE source repository is needed, as these plugins refer to JUCE modules.  An example for how you would
check things out for development might be thus:
		$ mkdir AustrianAudioPlugins ;cd AustrianAudioPlugins
		$ git clone https://github.com/juce-framework/JUCE.git
		$ git clone https://github.com/AustrianAudioGmbH/AmbiCreator
		$ git clone https://github.com/AustrianAudioGmbH/PolarDesigner
		$ git clone https://github.com/AustrianAudioGmbH/StereoCreator

* The JUCE repo should be checked out 'alongside' the Plugin sources, so that your folder structure might look
like this (if you've checked out all of our other plugins as well, which also have this requirement):

<pre>
.
├── AmbiCreator
├── JUCE
├── PolarDesigner
└── StereoCreator
</pre>

## Requirements
* For building AAX plugins you need to add the [AAX SDK](http://developer.avid.com/) location to your Projucer paths.

## Building PolarDesigner
PolarDesigner is based on [JUCE](https://juce.com/). To build PolarDesigner, get a recent version of JUCE and open PolarDesigner.jucer in Projucer. Select an exporter of your choice (e.g. Visual Studio or XCode) to create and open a project file in your IDE.

## Related repositories
Parts of the code are based on the [IEM Plugin Suite](https://git.iem.at/audioplugins/IEMPluginSuite) - check it out, it's awesome!

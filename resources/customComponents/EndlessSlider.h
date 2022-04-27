//
//  EndlessSlider.h
//  PolarDesigner - All
//
//  Created by Jay Vaughan on 23.02.22.
//  Copyright © 2022 Austrian Audio. All rights reserved.
//

#ifndef EndlessSlider_h
#define EndlessSlider_h


class EndlessSlider : public Slider {
public:
    EndlessSlider () :
    Slider(),
    patternStripSize(12)
    {
        setChangeNotificationOnlyOnRelease(false);
        scrollImage = getImageFromAssets("scrollImage.png");
        setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        setSliderStyle (Slider::LinearBarVertical);
        setScrollWheelEnabled(true);
    };
    
    // Trim step value - modify it freely as needed
    double step = 0.012725f;
    
    // set these callbacks where you use this class in order to get inc/dec messages
    std::function<void()> sliderIncremented;
    std::function<void()> sliderDecremented;
    
    void mouseDrag(const MouseEvent &e) override
    {
        int currentMoved;
        static int lastMoved;

        if (e.mouseWasDraggedSinceMouseDown()) {
            currentMoved = e.getDistanceFromDragStartY();
            scrollImageTransform = (AffineTransform::translation ((float) (scrollImage.getWidth()),
                                                                 (float) (scrollImage.getHeight()) + currentMoved)
                                      .followedBy (getTransform()));
            
            if ((currentMoved > lastMoved)){
                sliderDecremented();
            } else
            if (currentMoved < lastMoved) {
                sliderIncremented();
            }

            lastMoved = currentMoved;

            repaint();
        }
    }
    
    void paint (Graphics&g) override
    {
        Rectangle<int> bounds = getLocalBounds();
        Path endlessPath;
        g.setColour(getRandomColour());

        g.setFillType(juce::FillType(scrollImage, scrollImageTransform));
        g.fillRect(bounds);
        
    }
        
    // utility functions
    static juce::Colour getRandomColour()
    {
        auto& random = juce::Random::getSystemRandom();
        
        return juce::Colour ((juce::uint8) random.nextInt (256),
                             (juce::uint8) random.nextInt (256),
                             (juce::uint8) random.nextInt (256));
    }
    void mouseExit (const MouseEvent& e) override
    {
        repaint();
    }
    
    ~EndlessSlider () {}
    
    void resized() override
    {
        Slider::resized();
        auto& lf = getLookAndFeel();
        auto layout = lf.getSliderLayout (*this);
        
        sliderRect = layout.sliderBounds;
        sliderRect.removeFromTop(patternStripSize);
        
        dirPatternBounds = getLocalBounds().removeFromTop(patternStripSize);
    }
    
private:
    Rectangle<int> sliderRect;
    Rectangle<int> dirPatternBounds;
    float patternStripSize;
    Image scrollImage;
    AffineTransform scrollImageTransform;
    
    inline std::unique_ptr<InputStream> createAssetInputStream (const char* resourcePath)
    {
#if JUCE_ANDROID
        ZipFile apkZip (File::getSpecialLocation (File::invokedExecutableFile));
        return std::unique_ptr<InputStream> (apkZip.createStreamForEntry (apkZip.getIndexOfFileName ("assets/" + String (resourcePath))));
#else
#if JUCE_IOS || JUCE_MAC
        auto assetsDir = File::getSpecialLocation (File::currentExecutableFile)
            .getParentDirectory().getChildFile ("Assets");
#endif
        
        auto resourceFile = assetsDir.getChildFile (resourcePath);
        jassert (resourceFile.existsAsFile());
        
        return resourceFile.createInputStream();
#endif
    }
    
    
    inline Image getImageFromAssets (const char* assetName)
    {
        auto hashCode = (String (assetName) + "@polar_plugin_assets").hashCode64();
        auto img = ImageCache::getFromHashCode (hashCode);
        
        if (img.isNull())
        {
            std::unique_ptr<InputStream> juceIconStream (createAssetInputStream (assetName));
            
            if (juceIconStream == nullptr)
                return {};
            
            img = ImageFileFormat::loadFrom (*juceIconStream);
            
            ImageCache::addImageToCache (img, hashCode);
        }
        
        return img;
    }
    
};


#endif /* EndlessSlider_h */

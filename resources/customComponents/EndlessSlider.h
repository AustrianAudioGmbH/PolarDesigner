//
//  EndlessSlider.h
//  PolarDesigner - All
//
//  Created by Jay Vaughan on 23.02.22.
//  Copyright © 2022 Austrian Audio. All rights reserved.
//
// This Component implements an 'endless slider', which can be useful for implementing
// 'trim' controls, i.e. for applying trim to a set of EQ's, volume sliders, etc.
//
// To adjust the rate of trim, use the EndlessSlider.step value in the slider inc/dec
// callbacks.

#ifndef EndlessSlider_h
#define EndlessSlider_h

class EndlessSlider : public Slider {
public:
    EndlessSlider () :
    Slider()
    {
        setChangeNotificationOnlyOnRelease(false);
        sliderImage = getImageFromAssets("scrollImage.png");
        setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        setSliderStyle (Slider::LinearBarVertical);
        setScrollWheelEnabled(true);
    };
    
    // Trim step value - modify it freely as needed
    double step = 0.012725f;
    
    // set these callbacks where you use this class in order to get inc/dec messages
    std::function<void()> sliderIncremented;
    std::function<void()> sliderDecremented;
    
    // calculate whether to callback to an increment or decrement, and update UI
    void mouseDrag(const MouseEvent &e) override
    {
        int currentMoved;
        static int lastMoved;

        if (e.mouseWasDraggedSinceMouseDown()) {
            currentMoved = e.getDistanceFromDragStartY();
            sliderImageTransform = (AffineTransform::translation ((float) (sliderImage.getWidth()),
                                                                 (float) (sliderImage.getHeight()) + currentMoved)
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

        g.setFillType(juce::FillType(sliderImage, sliderImageTransform));
        g.fillRect(bounds);
        
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
        
    }

    
private:
    Rectangle<int> sliderRect;
    Image sliderImage;
    AffineTransform sliderImageTransform;
    
    // utility functions - from DemoRunner utilities
    static juce::Colour getRandomColour()
    {
        auto& random = juce::Random::getSystemRandom();
        
        return juce::Colour ((juce::uint8) random.nextInt (256),
                             (juce::uint8) random.nextInt (256),
                             (juce::uint8) random.nextInt (256));
    }

    // creats a usable image asset from a file stream
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
    
    // creates an image asset from cache if possible
    inline Image getImageFromAssets (const char* assetName)
    {
        auto hashCode = (String (assetName) + "@endless_slider_assets").hashCode64();
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

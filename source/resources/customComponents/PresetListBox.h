/*
  ==============================================================================

    PresetListBox.h
    Created: 17 Jun 2023 1:00:51pm
    Author:  mikol

  ==============================================================================
*/

#pragma once

#include "../lookAndFeel/MainLookAndFeel.h"
#include "BinaryData.h"

#include <juce_gui_basics/juce_gui_basics.h>

class PresetListBox : public juce::Component,
                      public juce::ListBoxModel,
                      public juce::ChangeBroadcaster
{
public:
    PresetListBox()
    {
        addAndMakeVisible (presets);
        presets.setModel (this);
        presets.setRowHeight (40);
        presets.setHeaderComponent (std::make_unique<PresetListHeaderComponent> (*this));
    }

    void resized() override
    {
        using namespace juce;

        FlexBox fb;
        fb.flexDirection = FlexBox::Direction::column;
        fb.justifyContent = juce::FlexBox::JustifyContent::center;
        fb.items.add (juce::FlexItem (presets).withFlex (1.f));
        fb.performLayout (getLocalBounds());

        presets.setRowHeight (static_cast<int> (fb.items[0].currentBounds.getHeight() * 0.1f));
    }

    void paintListBoxItem (int rowNumber,
                           juce::Graphics& g,
                           int width,
                           int height,
                           bool rowIsSelected) override
    {
        using namespace juce;

        auto eyeDropImg = juce::Drawable::createFromImageData (BinaryData::eyeDrop_svg,
                                                               BinaryData::eyeDrop_svgSize);
        auto eyeDropArea =
            Rectangle<float> (0, 0, static_cast<float> (width), static_cast<float> (height))
                .reduced (static_cast<float> (width) * 0.03f, static_cast<float> (height) * 0.15f);
        eyeDropImg->drawWithin (g, eyeDropArea, juce::RectanglePlacement::xRight, 1.f);

        if (rowIsSelected)
            g.fillAll (mainLaF.textButtonHoverBackgroundColor.withAlpha (0.5f));

        g.setColour (mainLaF.mainTextColor);

        float h = static_cast<float> (getTopLevelComponent()->getHeight()) * 0.023f;
        float y = (static_cast<float> (height) - h) / 2;

        Font font (FontOptions (mainLaF.normalFont));
        font.setHeight (h);
        g.setFont (font);
        g.drawFittedText (*data.getUnchecked (rowNumber),
                          2,
                          static_cast<int> (y),
                          width - 2 * eyeDropImg->getWidth(),
                          static_cast<int> (h),
                          Justification::left,
                          1);
    }

    int getNumRows() override { return data.size(); }

    void backgroundClicked (const juce::MouseEvent&) override { presets.deselectAllRows(); }

    void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override
    {
        rowDoubleClicked = true;
        selectedRow = row;
        sendChangeMessage();
    }

    void selectedRowsChanged (int lastRowSelected) override
    {
        rowDoubleClicked = false;
        if (lastRowSelected != -1)
        {
            selectedRow = lastRowSelected;
            sendChangeMessage();
        }
    }

    void returnKeyPressed (int lastRowSelected) override
    {
        rowDoubleClicked = true;
        selectedRow = lastRowSelected;
        sendChangeMessage();
    }

    juce::String getSelectedPresetName() { return *data.getUnchecked (selectedRow); }

    void setHeaderText (const juce::String& text) { presets.getHeaderComponent()->setTitle (text); }

    void AddNewPresetToList (const juce::String& presetName)
    {
        data.add (std::make_unique<juce::String> (presetName));
        presets.updateContent();
    }

    bool isRowDoubleClicked() { return rowDoubleClicked; }

    void deselectAll() { presets.deselectAllRows(); }

private:
    class PresetListHeaderComponent : public juce::Button
    {
    public:
        explicit PresetListHeaderComponent (PresetListBox& o) : Button ({})
        //            owner(o)
        {
            (void) o;
            setToggleable (true);
            setClickingTogglesState (true);
            setSize (0, 30);
        }

        void paintButton (juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
        {
            using namespace juce;

            ignoreUnused (isMouseOverButton, isButtonDown);

            Rectangle<float> buttonArea (0.0f,
                                         0.0f,
                                         static_cast<float> (getWidth()),
                                         static_cast<float> (getHeight()));
            g.fillAll (mainLaF.groupComponentBackgroundColor);

            auto x = 0;
            auto w = static_cast<int> (buttonArea.getWidth() * 0.65f);
            auto h = static_cast<float> (getTopLevelComponent()->getHeight()) * 0.023f;
            auto y = static_cast<int> ((buttonArea.getHeight() - h) / 2);

            g.setColour (mainLaF.textButtonHoverBackgroundColor);

            Font font (FontOptions (mainLaF.normalFont));
            font.setHeight (h);
            g.setFont (font);
            g.drawFittedText (getTitle(), x, y, w, static_cast<int> (h), Justification::left, 1);
        }

    private:
        //        PresetListBox& owner;
        MainLookAndFeel mainLaF;
    };

    void selectRow (int row)
    {
        if (row < 0)
            return;

        if (presets.isShowing())
            selectFirstRow();
    }

    void selectFirstRow()
    {
        if (auto* handler = presets.getAccessibilityHandler())
        {
            for (auto* child : handler->getChildren())
            {
                if (child->getRole() == juce::AccessibilityRole::listItem)
                {
                    child->grabFocus();
                    break;
                }
            }
        }
    }

    juce::ListBox presets;
    MainLookAndFeel mainLaF;
    juce::OwnedArray<juce::String> data;
    int selectedRow = -1;
    bool rowDoubleClicked = false;
};

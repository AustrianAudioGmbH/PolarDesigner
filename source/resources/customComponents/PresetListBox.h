/*
  ==============================================================================

    PresetListBox.h
    Created: 17 Jun 2023 1:00:51pm
    Author:  mikol

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_cryptography/juce_cryptography.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_opengl/juce_opengl.h>

#include "BinaryData.h"

#include "../lookAndFeel/MainLookAndFeel.h"

class PresetListBox : public Component, public ListBoxModel, public ChangeBroadcaster
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
        FlexBox fb;
        fb.flexDirection = FlexBox::Direction::column;
        fb.justifyContent = juce::FlexBox::JustifyContent::center;
        fb.items.add (juce::FlexItem (presets).withFlex (1.f));
        fb.performLayout (getLocalBounds());

        presets.setRowHeight (static_cast<int> (fb.items[0].currentBounds.getHeight() * 0.1f));
    }

    void paintListBoxItem (int rowNumber,
                           Graphics& g,
                           int width,
                           int height,
                           bool rowIsSelected) override
    {
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

        Font font (mainLaF.normalFont);
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

    void backgroundClicked (const MouseEvent&) override { presets.deselectAllRows(); }

    void listBoxItemDoubleClicked (int row, const MouseEvent&) override
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

    String getSelectedPresetName() { return *data.getUnchecked (selectedRow); }

    void setHeaderText (const String& text) { presets.getHeaderComponent()->setTitle (text); }

    void AddNewPresetToList (const String& presetName)
    {
        data.add (std::make_unique<String> (presetName));
        presets.updateContent();
    }

    bool isRowDoubleClicked() { return rowDoubleClicked; }

    void deselectAll() { presets.deselectAllRows(); }

private:
    class PresetListHeaderComponent : public Button
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

        void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override
        {
            (void) isMouseOverButton;
            (void) isButtonDown;
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

            Font font (mainLaF.normalFont);
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
                if (child->getRole() == AccessibilityRole::listItem)
                {
                    child->grabFocus();
                    break;
                }
            }
        }
    }

    ListBox presets;
    MainLookAndFeel mainLaF;
    OwnedArray<String> data;
    int selectedRow = -1;
    bool rowDoubleClicked = false;
};

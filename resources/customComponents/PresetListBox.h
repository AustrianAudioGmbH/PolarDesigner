/*
  ==============================================================================

    PresetListBox.h
    Created: 17 Jun 2023 1:00:51pm
    Author:  mikol

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../resources/lookAndFeel/MainLookAndFeel.h"

class PresetListBox : public Component,
    public ListBoxModel, public ChangeBroadcaster
{
public:
    PresetListBox()
    {
        addAndMakeVisible(presets);
        presets.setModel(this);
        presets.setRowHeight(40);
        presets.setHeaderComponent(std::make_unique<PresetListHeaderComponent>(*this));
    }

    void resized() override
    {
        FlexBox fb;
        fb.flexDirection = FlexBox::Direction::column;
        fb.justifyContent = juce::FlexBox::JustifyContent::center;
        fb.items.add(juce::FlexItem(presets).withFlex(1.f));
        fb.performLayout(getLocalBounds());

        presets.setRowHeight(fb.items[0].currentBounds.getHeight() * 0.1f);
    }

    void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override
    {
        auto eyeDropImg = juce::Drawable::createFromImageData(BinaryData::eyeDrop_svg, BinaryData::eyeDrop_svgSize);
        auto eyeDropArea = Rectangle<float>(0, 0, width, height).reduced(width*0.03f, height*0.15f);
        eyeDropImg->drawWithin(g, eyeDropArea, juce::RectanglePlacement::xRight, 1.f);

        if (rowIsSelected) 
            g.fillAll(mainLaF.textButtonHoverBackgroundColor.withAlpha(0.5f));

        g.setColour(mainLaF.mainTextColor);

        int h = getTopLevelComponent()->getHeight() * 0.023f;
        int y = (height - h) / 2;

        Font font(mainLaF.normalFont);
        font.setHeight(h);
        g.setFont(font);
        g.drawFittedText(*data.getUnchecked(rowNumber), 2, y, width - 2*eyeDropImg->getWidth(), h, Justification::left, 1);
    }

    int getNumRows() override
    {
        return data.size();
    }

    void backgroundClicked(const MouseEvent&) override
    {
        presets.deselectAllRows();
    }

    void listBoxItemDoubleClicked(int row, const MouseEvent&) override
    {
        sendChangeMessage();
        presets.deselectAllRows();
    }

    String getSelectedPresetName()
    {
        return *data.getUnchecked(selectedRow);
    }

    void setHeaderText(const String& text) { presets.getHeaderComponent()->setTitle(text); }
    void listBoxItemClicked(int row, const MouseEvent&) override
    { 
        selectedRow = row;
        selectRow(row); 
    }

    void AddNewPresetToList(const String& presetName)
    {
        data.add(std::make_unique<String>(presetName));
        presets.updateContent();
    }

private:
    class PresetListHeaderComponent : public Button
    {
    public:
        explicit PresetListHeaderComponent(PresetListBox& o)
            : Button({}),
            owner(o)
        {
            setToggleable(true);
            setClickingTogglesState(true);
            setSize(0, 30);
        }

        void paintButton(Graphics& g, bool isMouseOverButton, bool isButtonDown) override
        {
            Rectangle<float> buttonArea(0, 0, getWidth(), getHeight());
            g.fillAll(mainLaF.groupComponentBackgroundColor);

            int x = 0;
            int w = buttonArea.getWidth()*0.65f;
            int h = getTopLevelComponent()->getHeight() * 0.023f;
            int y = (buttonArea.getHeight() - h) / 2;

            g.setColour(mainLaF.textButtonHoverBackgroundColor);

            Font font(mainLaF.normalFont);
            font.setHeight(h);
            g.setFont(font);
            g.drawFittedText(getTitle(), x, y, w, h, Justification::left, 1);
        }

    private:
        PresetListBox& owner;
        MainLookAndFeel mainLaF;
    };

    void selectRow(int row)
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
};

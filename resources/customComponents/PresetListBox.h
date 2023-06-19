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
    public ListBoxModel
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
        fb.flexDirection = FlexBox::Direction::row;
        fb.justifyContent = juce::FlexBox::JustifyContent::center;

        FlexBox subFb;
        subFb.flexDirection = FlexBox::Direction::column;
        subFb.justifyContent = juce::FlexBox::JustifyContent::center;
        subFb.items.add(juce::FlexItem(*presets.getHeaderComponent()).withFlex(0.1f));
        subFb.items.add(juce::FlexItem(presets).withFlex(0.9f));

        fb.items.add(juce::FlexItem().withFlex(0.09f));
        fb.items.add(juce::FlexItem(subFb).withFlex(0.82f));
        fb.items.add(juce::FlexItem().withFlex(0.09f));
 
        fb.performLayout(getLocalBounds());

        presets.setRowHeight(subFb.items[1].currentBounds.getHeight() * 0.16f);
    }

    void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override
    {
        g.fillAll(mainLaF.groupComponentBackgroundColor);

        if (rowIsSelected) 
            g.fillAll(mainLaF.textButtonHoverBackgroundColor.withAlpha(0.5f));

        g.setColour(mainLaF.mainTextColor);

        int h = getTopLevelComponent()->getHeight() * 0.023f;
        int y = (height - h) / 2;

        Font font(mainLaF.normalFont);
        font.setHeight(h);
        g.setFont(font);
        g.drawFittedText(*data.getUnchecked(rowNumber), 2, y, width, h, Justification::left, 1);
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
        //TODO: load preset
    }

    void setHeaderText(const String& text) { presets.getHeaderComponent()->setTitle(text); }
    //void returnKeyPressed(int row) override { selectRow(row); }
    void listBoxItemClicked(int row, const MouseEvent&) override { selectRow(row); }

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

            auto presetHeaderArrowImg = juce::Drawable::createFromImageData(BinaryData::presetHeaderArrowIcon_svg, BinaryData::presetHeaderArrowIcon_svgSize);
 
            int x = 0;
            int w = buttonArea.getWidth()*0.45f;
            int h = getTopLevelComponent()->getHeight() * 0.023f;
            int y = (buttonArea.getHeight() - h) / 2;

            g.setColour(mainLaF.textButtonHoverBackgroundColor);

            auto arrowArea = buttonArea.reduced(0, proportionOfHeight(0.33f));
            presetHeaderArrowImg->drawWithin(g, arrowArea, juce::RectanglePlacement::centred, 1.f);

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
};

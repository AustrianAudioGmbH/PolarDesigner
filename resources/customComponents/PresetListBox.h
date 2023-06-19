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
        addMouseListener(this, true);

        highlightColor = mainLaF.groupComponentBackgroundColor;
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
        auto comp = presets.getComponentForRowNumber(rowNumber);
        auto comp2 = presets.getRowContainingPosition(mousepos.getX(), mousepos.getY());

        if (rowIsSelected)
        {
            highlightColor = mainLaF.textButtonHoverBackgroundColor;
        }
        else if (mouseOverRow && comp)
        {
            auto currentComp = comp->getBounds();
            if (currentComp.contains(mousepos.toInt()))
            {
                highlightColor = mainLaF.textButtonHoverBackgroundColor.withAlpha(0.5f);
                g.setColour(highlightColor);
                g.fillRect(currentComp);
            }

        }
        else
        {
            highlightColor = mainLaF.groupComponentBackgroundColor;
        }

        //g.fillAll(highlightColor);
        g.setColour(mainLaF.mainTextColor);

        int h = getTopLevelComponent()->getHeight() * 0.023f;
        int y = (height - h) / 2;

        Font font(mainLaF.normalFont);
        font.setHeight(h);
        g.setFont(font);
        g.drawFittedText(String("Preset:"), 2, y, width, h, Justification::left, 1);
    }

    void mouseEnter(const MouseEvent& event) override
    {
        mouseOverRow = true;
        mousepos = event.position;
        repaint();
    }

    void mouseExit(const MouseEvent& event) override
    {
        //mouseOverRow = false;
        //repaint();
    }

    int getNumRows() override
    {
        return 10;
    }

    String getNameForRow(int rowNumber) override
    {
        if (selectedCategory.isEmpty())
        {
         //   if (isPositiveAndBelow(rowNumber, JUCEDemos::getCategories().size()))
           //     return JUCEDemos::getCategories()[(size_t)rowNumber].name;
        }
        else
        {
            //auto& category = JUCEDemos::getCategory(selectedCategory);

            //if (isPositiveAndBelow(rowNumber, category.demos.size()))
              //  return category.demos[(size_t)rowNumber].demoFile.getFileName();
        }

        return {};
    }

    void setHeaderText(const String& text) { presets.getHeaderComponent()->setTitle(text); }
    //void returnKeyPressed(int row) override { selectRow(row); }
    void listBoxItemClicked(int row, const MouseEvent&) override { selectRow(row); }

    //==============================================================================
    void showCategory(bool shouldShow) noexcept
    {
        if (shouldShow)
        {
            presets.selectRow(0);
        }
        else
        {
            presets.deselectAllRows();
        }
        presets.updateContent();
    }

private:
    //==============================================================================
    class PresetListHeaderComponent : public Button
    {
    public:
        explicit PresetListHeaderComponent(PresetListBox& o)
            : Button({}),
            owner(o)
        {
            //setTitle("Previous");
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

        void clicked() override
        {
            owner.showCategory(getToggleState());
        }

        using Button::clicked;

    private:
        PresetListBox& owner;
        MainLookAndFeel mainLaF;
    };

    //==============================================================================
    void selectRow(int row)
    {
        if (row < 0)
            return;

        //if (selectedCategory.isEmpty())
            //showCategory(JUCEDemos::getCategories()[(size_t)row].name);
        //else
           // demoHolder.setDemo(selectedCategory, row);

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
    Point<float> mousepos;

    String selectedCategory;
    ListBox presets;
    MainLookAndFeel mainLaF;
    Colour highlightColor;
    bool mouseOverRow = false;
};

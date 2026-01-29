/*
 ==============================================================================
 Author: Sebastian Grill
 
 Copyright (c) 2025 - Austrian Audio GmbH
 www.austrian.audio
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once

#include "../lookAndFeel/BinaryFonts.h"
#include "../lookAndFeel/DefaultLookAndFeel.hpp"
#include "MultiTextButton.hpp"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AAGuiComponents
{
class ABComponent : public juce::Component
{
public:
    ABComponent() : buttons (2)
    {
        using namespace juce;

        setLookAndFeel (&lookAndFeel);

        addAndMakeVisible (label);
        label.setFont (
            FontOptions (Typeface::createSystemTypefaceFor (BinaryFonts::NunitoSansRegular_ttf,
                                                            BinaryFonts::NunitoSansRegular_ttfSize))
                .withHeight (18.0f));
        label.setText ("Compare", NotificationType::dontSendNotification);

        addAndMakeVisible (buttons);

        buttons.setButtonText (0, "A");
        buttons[0].setRadioGroupId (1);

        buttons.setButtonText (1, "B");
        buttons[1].setRadioGroupId (1);
    }

    ~ABComponent() override { setLookAndFeel (nullptr); }

    juce::TextButton& operator[] (int i) { return buttons[i]; }

    void resized() override
    {
        using namespace juce;

        auto area = getLocalBounds();
        label.setBounds (area.removeFromLeft (80));
        buttons.setBounds (area.removeFromLeft (87));
    }

    void setEnabled (bool isEnabled)
    {
        const auto alpha = isEnabled ? 1.0f : 0.3f;

        buttons.setEnabled (isEnabled);
        label.setEnabled (isEnabled);

        buttons.setAlpha (alpha);
    }

private:
    juce::Label label;
    MultiTextButton<ButtonColor::red> buttons;
    DefaultLookAndFeel lookAndFeel;
};
} // namespace AAGuiComponents

/*
 This file is based on MuteSoloButton.h of the IEM plug-in suite.
 Modifications by Thomas Deppisch.
*/

/*
 ==============================================================================
 Author: Thomas Deppisch
 
 Copyright (c) 2019 - Austrian Audio GmbH
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

/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 https://iem.at

 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <https://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once


//==============================================================================
/*
*/
class MuteSoloButton    : public ToggleButton
{
public:
    enum Type
    {
        mute,
        solo
    };

    MuteSoloButton()
    {
        setType(Type::mute);
    }
    ~MuteSoloButton()
    {
    }

    void setType (Type newType)
    {
        type = newType;
        setColour (ToggleButton::tickColourId, type == Type::mute ? Colours::red : Colours::yellow);
        repaint();
    }
    void paint (Graphics& g) override
    {
        Rectangle<int> bounds = getLocalBounds();
        const bool state = getToggleState();

        getLookAndFeel().drawTickBox(g, *this, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), getToggleState(), isEnabled(), isMouseOver(), isMouseButtonDown());

        g.setFont(getLookAndFeel().getTypefaceForFont (Font(12.0f, 0)));
        g.setFont(bounds.getHeight()-4);

        g.setColour(!isEnabled() ? Colours::black : state ? Colours::black : findColour(ToggleButton::tickColourId));
        g.drawFittedText(type == solo ? "S" : "M", bounds, juce::Justification::centred, 1);
        
    }

    void resized() override
    {
    }

private:
    Type type;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MuteSoloButton)
};

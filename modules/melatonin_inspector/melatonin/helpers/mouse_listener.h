#pragma once
#include "juce_gui_basics/juce_gui_basics.h"

namespace melatonin
{
    // Unfortunately the DocumentWindow cannot behave as our root component mouse listener
    // without some strange side effects. That's why we are doing the whole lambda dance...
    class MouseListener : public juce::MouseListener
    {
    public:
        explicit MouseListener (juce::Component& c, bool startEnabled = true) : root (c)
        {
            // Listen to all mouse movements for all children of the root
            if (startEnabled)
            {
                enabled = true;
                root.addMouseListener (this, true);
            }
        }

        ~MouseListener() override
        {
            if (enabled)
                root.removeMouseListener (this);
        }

        void enable()
        {
            enabled = true;
            root.addMouseListener (this, true);
        }

        void disable()
        {
            enabled = false;
            root.removeMouseListener (this);
        }

        void mouseEnter (const juce::MouseEvent& event) override
        {
            outlineComponentCallback (event.originalComponent);
        }

        void mouseMove (const juce::MouseEvent& event) override
        {
            if (outlineDistanceCallback && event.mods.isAltDown())
                outlineDistanceCallback (event.originalComponent);
            else
                outlineDistanceCallback (nullptr);
        }

        void mouseUp (const juce::MouseEvent& event) override
        {
            if (event.mods.isLeftButtonDown() && !isDragging)
            {
                selectComponentCallback (event.originalComponent);
            }
            isDragging = false;
        }

        void mouseDown (const juce::MouseEvent& event) override
        {
            if (event.mods.isLeftButtonDown() && event.originalComponent->isMouseOverOrDragging())
            {
                componentStartDraggingCallback (event.originalComponent, event);
            }
        }

        void mouseDrag (const juce::MouseEvent& event) override
        {
            // takes care of small mouse position drift on selection
            if (event.getDistanceFromDragStart() > 3 && event.originalComponent->isMouseOverOrDragging())
            {
                isDragging = true;
                componentDraggedCallback (event.originalComponent, event);
            }
        }

        void mouseExit (const juce::MouseEvent& event) override
        {
            if (event.originalComponent == &root)
            {
                mouseExitCallback();
            }
        }

        std::function<void (juce::Component* c)> outlineComponentCallback;
        std::function<void (juce::Component* c)> outlineDistanceCallback;
        std::function<void (juce::Component* c)> selectComponentCallback;
        std::function<void (juce::Component* c, const juce::MouseEvent& e)> componentStartDraggingCallback;
        std::function<void (juce::Component* c, const juce::MouseEvent& e)> componentDraggedCallback;
        std::function<void()> mouseExitCallback;

    private:
        juce::Component& root;
        bool enabled = false;

        bool isDragging { false };
    };

}

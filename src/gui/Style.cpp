
#include <string>
#include <map>
#include <e3_Exception.h>
#include <e3_Utilities.h>
#include "gui/Style.h"


namespace e3 {

    class StyleDocumentWindowButton : public Button
    {
    public:
        StyleDocumentWindowButton(const String& name, int buttonType) : Button(name), 
            buttonType_(buttonType)
        {}

        void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override
        {
            ResizableWindow* parent = findParentComponentOfClass<ResizableWindow>();
            Colour highlight = parent ? parent->getBackgroundColour() : Colour(0xffbebdbb);

            Colour orange(0xffd08930);
            Colour grey(0xff3e4042);

            if (!isEnabled()) {
                grey = grey.withAlpha(0.6f);
                orange = orange.withAlpha(0.6f);
            }

            const float x = getWidth()  * 0.15f;
            const float y = getHeight() * 0.3f;
            const float w = jmin(getWidth(), getHeight()) * (isButtonDown ? 0.65f : 0.7f);

            if (isMouseOver) {
                g.setColour(highlight.brighter());
                g.fillRect(0, 0, getWidth(), getHeight());
            }

            switch (buttonType_)
            {
            case DocumentWindow::minimiseButton:
                g.setColour(grey);
                g.fillRect(x, y, w, w);
                g.setColour(orange);
                g.fillRect(x, y + w * 0.75f, w, w * 0.25f);
                break;
            case DocumentWindow::maximiseButton:
            {
                bool isFullscreen = getToggleState();
                Colour inner      = isFullscreen ? orange : grey;
                Colour outer      = isFullscreen ? grey : orange;

                g.setColour(outer);
                g.fillRect(x, y, w, w);
                g.setColour(inner);
                g.fillRect(x + w * 0.25f, y + w*0.25f, w*0.5f, w*0.5f);
                break;
            }
            case DocumentWindow::closeButton:
            {
                g.setColour(orange);
                g.fillRect(x, y, w, w);

                g.setColour(grey);
                Path shape;
                shape.addLineSegment(Line<float>(0.0f, 0.0f, 1.0f, 1.0f), 0.30f);
                shape.addLineSegment(Line<float>(1.0f, 0.0f, 0.0f, 1.0f), 0.30f);
                g.fillPath(shape, shape.getTransformToScaleToFit(
                    x + w * 0.15f, y + w * 0.15f, w * 0.7f, w * 0.7f, true));
                break;
            }
            }
        }

    private:
        int buttonType_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StyleDocumentWindowButton)
    };

    //--------------------------------------------------------------------
    // class Style
    //--------------------------------------------------------------------

    Style::Style(XmlElement* styleXml)
    {
        ASSERT(styleXml);
        std::map<std::string, Colour> colorMap;
        forEachXmlChildElementWithTagName(*styleXml, e, "Color")
        {
            std::string name = e->getStringAttribute("name").toStdString();
            std::string hex  = e->getStringAttribute("argb").toStdString();

            colorMap.insert(std::make_pair(name, Colour(common::HexToInt(hex))));
        }

        // juce components
        setColour(ResizableWindow::backgroundColourId, colorMap["background"]);
        setColour(DocumentWindow::textColourId, colorMap["lightText"]);

        setColour(Label::backgroundColourId, Colours::transparentBlack);
        setColour(Label::textColourId, colorMap["darkText"]);
        setColour(Label::outlineColourId, Colours::transparentBlack);
        setColour(Label::backgroundWhenEditingColourId, Colours::white);
        setColour(Label::textWhenEditingColourId, colorMap["darkText"]);
        setColour(Label::outlineWhenEditingColourId, colorMap["contentBackground2"]);

        setColour(TextEditor::backgroundColourId, Colours::white);
        setColour(TextEditor::textColourId, colorMap["darkText"]);
        setColour(TextEditor::highlightColourId, colorMap["highlight"].withAlpha(0.5f));
        setColour(TextEditor::highlightedTextColourId, colorMap["darkText"]);
        setColour(TextEditor::outlineColourId, colorMap["contentBackground1"]);
        setColour(TextEditor::focusedOutlineColourId, colorMap["highlight"]);
        setColour(TextEditor::shadowColourId, colorMap["darkText"]);

        setColour(TextButton::buttonColourId, colorMap["buttonOffBackground"]);
        setColour(TextButton::buttonOnColourId, colorMap["buttonOnBackground"]);
        setColour(TextButton::textColourOffId, colorMap["buttonOffText"]);
        setColour(TextButton::textColourOnId, colorMap["buttonOnText"]);

        //setColour(TabbedComponent::backgroundColourId, colorMap["contentBackground2"]);
        setColour(TabbedComponent::backgroundColourId, Colours::transparentBlack);
        setColour(TabbedComponent::outlineColourId, Colours::transparentBlack);
        //setColour(TabbedComponent::outlineColourId, colorMap["contentBackground2"]);
        
        setColour(TabbedButtonBar::tabOutlineColourId, Colours::transparentBlack);
        setColour(TabbedButtonBar::tabTextColourId, colorMap["buttonOffText"]);
        setColour(TabbedButtonBar::frontOutlineColourId, Colours::transparentBlack);
        setColour(TabbedButtonBar::frontTextColourId, colorMap["buttonOnText"]);

        setColour(ListBox::backgroundColourId, colorMap["contentBackground2"]);
        setColour(ListBox::outlineColourId, Colours::transparentBlack);
        setColour(ListBox::textColourId, colorMap["lightText"]);

        // custom colours
        setColour(kBackgroundColourId, colorMap["background"]);
        setColour(kContentBackground1ColourId, colorMap["contentBackground1"]);
        setColour(kContentBackground2ColourId, colorMap["contentBackground2"]);
        setColour(kHighlightColourId, colorMap["highlight"]);
        setColour(kTabButtonOffBackgroundColourId, colorMap["buttonOffBackground"]);
        setColour(kTabButtonOnBackgroundColourId, colorMap["buttonOnBackground"]);
        setColour(kTabButtonOffTextColourId, colorMap["buttonOffText"]);
        setColour(kTabButtonOnTextColourId, colorMap["buttonOnText"]);
    }


    void Style::drawDocumentWindowTitleBar(
        DocumentWindow& window, Graphics& g,
        int w, int h, int titleSpaceX, int titleSpaceW,
        const Image* icon, bool drawTitleTextOnLeft)
    {
        const bool isActive = window.isActiveWindow();

        g.fillAll(window.getBackgroundColour());

        Font font(h * 0.65f, Font::bold);
        g.setFont(font);

        int textW = font.getStringWidth(window.getName());
        int iconW = 0;
        int iconH = 0;

        if (icon != nullptr)
        {
            iconH = (int)font.getHeight();
            iconW = icon->getWidth() * iconH / icon->getHeight() + 4;
        }

        textW = jmin(titleSpaceW, textW + iconW);
        int textX = drawTitleTextOnLeft ? titleSpaceX : jmax(titleSpaceX, (w - textW) / 2);

        if (textX + textW > titleSpaceX + titleSpaceW)
            textX = titleSpaceX + titleSpaceW - textW;

        if (icon != nullptr)
        {
            g.setOpacity(isActive ? 1.0f : 0.6f);
            g.drawImageWithin(*icon, textX, (h - iconH) / 2, iconW, iconH,
                RectanglePlacement::centred, false);
            textX += iconW + 10;
            textW -= iconW;
        }

        if (window.isColourSpecified(DocumentWindow::textColourId) || isColourSpecified(DocumentWindow::textColourId))
            g.setColour(window.findColour(DocumentWindow::textColourId));
        else
            g.setColour(window.getBackgroundColour().contrasting(isActive ? 0.7f : 0.4f));

        g.drawText(window.getName(), textX, 0, textW, h, Justification::centredLeft, true);
    }


    Button* Style::createDocumentWindowButton(int buttonType)
    {
        return new StyleDocumentWindowButton("", buttonType);
    }


    void Style::drawButtonBackground(Graphics& g, Button& button, const Colour& color,
        bool isMouseOver, bool isMouseDown)
    {
        //Colour baseColour(backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 1.f)
        //    .withMultipliedAlpha(button.isEnabled() ? 1.f : 0.5f));

        //if (isButtonDown || isMouseOver)
        //    baseColour = baseColour.contrasting(isButtonDown ? 0.2f : 0.1f);

        Colour c = modifyButtonBackgroundColour(color, isMouseOver, isMouseDown, button.isEnabled(), button.hasKeyboardFocus(true));
        g.setColour(c);
        g.fillRect(button.getLocalBounds());
    }


    int Style::getTabButtonBestWidth(TabBarButton &, int tabDepth)
    {
        return 100 + tabDepth / 3;
    }


    int Style::getTabButtonOverlap(int /*tabDepth*/)            
    { 
        return -10; 
    }


    Colour Style::modifyButtonBackgroundColour(
        const Colour& colour, bool isMouseOver, bool isMouseDown, bool isEnabled, bool hasFocus)
    {
        Colour c = colour.withMultipliedSaturation(hasFocus ? 1.3f : 1.f).withMultipliedAlpha(isEnabled ? 1.f : 0.5f);

        if (isMouseDown || isMouseOver)
            c = c.contrasting(isMouseDown ? 0.2f : 0.1f);

        return c;
    }


    void Style::drawTabButton(TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
    { 
        // draw background
        //
        int colourId = button.getToggleState() ? kTabButtonOnBackgroundColourId : kTabButtonOffBackgroundColourId;
        Colour colour = modifyButtonBackgroundColour(findColour(colourId),
            isMouseOver, isMouseDown, button.isEnabled(), button.hasKeyboardFocus(true));
        g.setColour(colour);

        Rectangle<int> bounds = button.getLocalBounds();
        g.fillRect(bounds.reduced(0, 1));

        // draw text
        //
        const TabbedButtonBar& bar = button.getTabbedButtonBar();
        const TabbedButtonBar::Orientation o = bar.getOrientation();

        colourId = button.isFrontTab() ? TabbedButtonBar::frontTextColourId : TabbedButtonBar::tabTextColourId;
        Colour textColor = bar.findColour(colourId);

        const Rectangle<float> textArea(button.getTextArea().toFloat());

        float length = textArea.getWidth();
        float depth  = textArea.getHeight();

        if (bar.isVertical())
            std::swap(length, depth);

        AffineTransform t;
        TextLayout textLayout;
        createTabTextLayout(button, length, depth, textColor, textLayout);

        switch (o)
        {
        case TabbedButtonBar::TabsAtLeft:   t = t.rotated(float_Pi * -0.5f).translated(textArea.getX(), textArea.getBottom()); break;
        case TabbedButtonBar::TabsAtRight:  t = t.rotated(float_Pi *  0.5f).translated(textArea.getRight(), textArea.getY()); break;
        case TabbedButtonBar::TabsAtTop:
        case TabbedButtonBar::TabsAtBottom: t = t.translated(textArea.getX(), textArea.getY()); break;
        default:                            jassertfalse; break;
        }

        g.addTransform(t);
        textLayout.draw(g, Rectangle<float>(length, depth));
    }


    void Style::drawTabbedButtonBarBackground(TabbedButtonBar&, Graphics&) 
    {
        //Rectangle<int> content = buttonBar.getLocalBounds();
        //Colour col = findColour(kBackgroundColourId);
        //g.setColour(col);
        ////g.fillRect(content.reduced(0, -1));
    }


    void Style::drawTabAreaBehindFrontButton(TabbedButtonBar&, Graphics&, int, int)
    {
        //Rectangle<int> line;
        //switch (bar.getOrientation())
        //{
        //case TabbedButtonBar::TabsAtLeft:   line.setBounds(w - 1, 0, 1, h); break;
        //case TabbedButtonBar::TabsAtRight:  line.setBounds(0, 0, 1, h); break;
        //case TabbedButtonBar::TabsAtTop:    line.setBounds(0, h - 1, w, 1); break;
        //case TabbedButtonBar::TabsAtBottom: line.setBounds(0, 0, w, 1); break;
        //default: break;
        //}
        //g.setColour(bar.findColour(TabbedButtonBar::tabOutlineColourId));
        //g.fillRect(line);
    }


    void Style::drawCornerResizer(Graphics& g, int w, int h, bool, bool)
    {
        const float lineThickness = jmin(w, h) * 0.075f;

        for (float i = 0.1f; i <= 1.0f; i += 0.3f)
        {
            g.setColour(Colour(0xffd08930));  // TODO: use style

            g.drawLine(w * i + lineThickness,
                h - 1.f,
                w - 1.f,
                h * i + lineThickness,
                lineThickness);
        }
    }


    void Style::drawTableHeaderBackground(Graphics& g, TableHeaderComponent& header)
    {
        Rectangle<int> r(header.getLocalBounds());

        Colour bkgndColour = findColour(TextButton::buttonColourId);
        Colour lineColour  = findColour(kBackgroundColourId);

        // background
        g.setColour(bkgndColour);
        g.fillRect(r);

        // bottom separator
        g.setColour(lineColour);
        g.fillRect(r.removeFromBottom(1));

        // separators
        for (int i = header.getNumColumns(true) - 1; --i >= 0;)
            g.fillRect(header.getColumnPosition(i).removeFromRight(1));
    }


    void Style::drawTableHeaderColumn(Graphics& g, const String& columnName, int /*columnId*/,
        int width, int height, bool isMouseOver, bool isMouseDown,
        int columnFlags)
    {
        Colour colour = findColour(TextButton::buttonColourId);
        if (isMouseDown || isMouseOver) {
            colour = colour.contrasting(isMouseDown ? 0.2f : 0.1f);
            g.fillAll(colour);
        }

        Rectangle<int> area(width, height);
        area.reduce(4, 0);

        colour = findColour(TextButton::textColourOffId);

        if ((columnFlags & (TableHeaderComponent::sortedForwards | TableHeaderComponent::sortedBackwards)) != 0)
        {
            Path sortArrow;
            sortArrow.addTriangle(0.0f, 0.0f,
                0.5f, (columnFlags & TableHeaderComponent::sortedForwards) != 0 ? -0.8f : 0.8f,
                1.0f, 0.0f);

            g.setColour(colour);
            g.fillPath(sortArrow, sortArrow.getTransformToScaleToFit(area.removeFromRight(height / 2).reduced(2).toFloat(), true));
        }

        g.setColour(colour);
        g.setFont(Font(height * 0.5f, Font::plain));
        g.drawFittedText(columnName, area, Justification::centredLeft, 1);
    }




} // namespace e3
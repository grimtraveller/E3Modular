
#include "gui/ModulePanel.h"
#include "gui/ParameterPanel.h"
#include "gui/EditorPanel.h"


namespace e3
{
    void EditorPanel::setComponents( ModulePanel* modulePanel, ParameterPanel* parameterPanel )
    {
        modulePanel_    = modulePanel;
        parameterPanel_ = parameterPanel;

        viewport_.setViewedComponent( modulePanel_, false );
        viewport_.setFocusContainer( true );

        addAndMakeVisible( viewport_ );
        addAndMakeVisible( parameterPanel_ );
    }


    void EditorPanel::resized()
    {
        Rectangle<int> content = getLocalBounds();
        int width              = content.getWidth();
        int separator          = width - 215;

        viewport_.setBounds( content.withWidth( separator ) );      // this is the visible area
        modulePanel_->checkViewport();

        parameterPanel_->setBounds( separator + 15, 0, width - separator - 15, modulePanel_->getHeight() );
    }

} // namespace e3
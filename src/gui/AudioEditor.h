/*
  ==============================================================================

  This file was auto-generated by the Introjucer!

  It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
  */
#pragma once

#include <memory>
#include "JuceHeader.h"
#include "gui/CommandTarget.h"


namespace e3 {

    class Processor;
    class TabComponent;
    class EditorPanel;
    class ModulePanel;
    class ParameterPanel;
    class BrowserPanel;
    class SetupPanel;
    class InstrumentBrowser;
    class DatabaseBrowser;
    class MonitorComponent;


    class AudioEditor : public AudioProcessorEditor, public CommandTarget
    {
    public:
        AudioEditor( Processor* );
        ~AudioEditor();

        void paint( Graphics& ) override;
        void resized() override;
        void createIcon( Image& image );

        bool perform( const InvocationInfo& info ) override;      // Implementation for ApplicationCommandTarget

    private:
        void restoreWindowState();
        void createComponents();
        void connectSignals();
        void disconnectSignals();

        void onOpenInstrument();
        void onSaveInstrument();
        void onSaveInstrumentAs();
        void onNewInstrument();

        enum PanelIds {
            kEditorPanel = 0,
            kBrowserPanel = 1,
            kSetupPanel = 2
        };

        Processor* processor_;

        ScopedPointer<TabComponent> tabPanel_;
        ScopedPointer<EditorPanel>  editorPanel_;
        ScopedPointer<BrowserPanel> browserPanel_;
        ScopedPointer<SetupPanel> setupPanel_;
        ScopedPointer<ModulePanel> modulePanel_;
        ScopedPointer<ParameterPanel> parameterPanel_;
        ScopedPointer<InstrumentBrowser> instrumentBrowser_;
        ScopedPointer<DatabaseBrowser> databaseBrowser_;
        ScopedPointer<MonitorComponent> monitor_;
        ScopedPointer<ResizableCornerComponent> resizer_;

        ComponentBoundsConstrainer resizeLimits_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( AudioEditor )
    };

} // namespace e3
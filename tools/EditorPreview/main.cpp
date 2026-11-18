#include <juce_audio_processors/juce_audio_processors.h>
#include <iostream>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include "PluginProcessor.h"

// Renders the editor of the processor itself (not through the VST3 wrapper,
// whose host-side editor only lays out inside a real window) to a PNG so the
// UI can be reviewed without opening a DAW.
class EditorPreview : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override    { return "EasyDelayPreview"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override          { return false; }

    void initialise (const juce::String&) override
    {
        juce::Timer::callAfterDelay (200, [this]
        {
            processor.setPlayConfigDetails (2, 2, 48000.0, 512);
            processor.prepareToPlay (48000.0, 512);

            auto* ed = processor.createEditorIfNeeded();
            if (ed == nullptr)
            {
                std::cerr << "could not create editor" << std::endl;
                setApplicationReturnValue (1);
                quit();
                return;
            }

            ed->setVisible (true);
            ed->resized();

            auto image = ed->createComponentSnapshot (ed->getLocalBounds(), true, 2.0f);
            const auto out = juce::File::getCurrentWorkingDirectory().getChildFile ("build/editor-preview.png");
            out.deleteFile();
            juce::FileOutputStream stream (out);
            juce::PNGImageFormat png;
            const bool ok = png.writeImageToStream (image, stream);
            std::cout << (ok ? "wrote " : "failed ") << out.getFullPathName().toStdString() << std::endl;

            processor.editorBeingDeleted (ed);
            ed = nullptr;
            setApplicationReturnValue (ok ? 0 : 1);
            quit();
        });
    }

    void shutdown() override {}

private:
    EasyDelayProcessor processor;
};

START_JUCE_APPLICATION (EditorPreview)



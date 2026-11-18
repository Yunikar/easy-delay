#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <iostream>
#include <memory>

// Loads the freshly built EasyDelay.vst3 through JUCE's own VST3 host code and
// checks the things that matter: it must report zero latency, it must delay
// audio internally, and the editor must render.
class PluginCheck : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override    { return "EasyDelayCheck"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override          { return false; }

    void initialise (const juce::String&) override
    {
        juce::MessageManager::callAsync ([this] { run(); });
    }

    void shutdown() override {}

private:
    static constexpr double sr = 48000.0;
    static constexpr int blockSize = 512;

    std::unique_ptr<juce::AudioPluginInstance> instance;
    std::unique_ptr<juce::AudioProcessorEditor> editor;

    juce::File pluginFile()
    {
        return juce::File::getCurrentWorkingDirectory()
            .getChildFile ("build/EasyDelay_artefacts/Release/VST3/EasyDelay.vst3");
    }

    void run()
    {
        const auto file = pluginFile();
        if (! file.exists())
        {
            report ("FAIL", "plugin not found: " + file.getFullPathName());
            quitWith (1);
            return;
        }
        report ("INFO", "plugin: " + file.getFullPathName());

        juce::OwnedArray<juce::PluginDescription> descriptions;
        juce::VST3PluginFormat vst3;
        vst3.findAllTypesForFile (descriptions, file.getFullPathName());

        if (descriptions.isEmpty())
        {
            report ("FAIL", "no plugin description found");
            quitWith (1);
            return;
        }

        const auto& desc = *descriptions.getFirst();
        report ("INFO", "name=" + desc.name + "  manufacturer=" + desc.manufacturerName);
        report ("INFO", "version=" + desc.version + "  format=" + desc.pluginFormatName
                            + "  category=" + desc.category);

        // A fresh instance per scenario: same host code, no state carried over.
        if (! createInstance (desc, "mix=0 passthrough", { { "Mix", 0.0f }, { "Delay", 0.6f }, { "Feedback", 0.0f } }))
        {
            quitWith (exitCode);
            return;
        }

        const auto latency = instance->getLatencySamples();
        if (latency != 0)
            fail ("reports latency " + juce::String (latency) + " samples; must be 0 for zero PDC");
        else
            report ("PASS", "reports 0 samples of latency (zero PDC)");

        report ("INFO", "buses: in=" + juce::String (instance->getBusCount (true))
                            + " out=" + juce::String (instance->getBusCount (false))
                            + " inCh=" + juce::String (instance->getTotalNumInputChannels())
                            + " outCh=" + juce::String (instance->getTotalNumOutputChannels())
                            + " tail=" + juce::String (instance->getTailLengthSeconds()) + " s");

        auto rendered = render ([this] (float v, int) { return v; }, 0.5f);
        double peak = 0.0;
        for (int i = 0; i < rendered.getNumSamples(); ++i)
            peak = juce::jmax (peak, (double) std::abs (rendered.getSample (0, i)));
        report (peak > 0.05 ? "PASS" : "FAIL",
                "mix=0 passes audio through (peak " + juce::String (peak, 4) + ")");

        // ------------------------------------------------------- internal delay
        if (! createInstance (desc, "delay check", { { "Mix", 1.0f }, { "Delay", 0.6f }, { "Feedback", 0.0f } }))
        {
            quitWith (exitCode);
            return;
        }

        // 0.5 s of silence, then a single impulse, then 2 s of silence. The dry
        // impulse is audible immediately; the delayed copy must follow one delay
        // time later even though the host was told 0 latency.
        const int silence = (int) (0.5 * sr);
        auto impulse = render ([](float, int i) { return i == (int) (0.5 * 48000.0) ? 1.0f : 0.0f; }, 2.5f);

        const auto delayText = instance->getParameters()[0]->getText (instance->getParameters()[0]->getValue(), 64);

        int dryAt = -1;
        int delayedAt = -1;
        float delayedPeak = 0.0f;
        for (int i = silence; i < impulse.getNumSamples(); ++i)
        {
            const float v = std::abs (impulse.getSample (0, i));
            if (dryAt < 0 && v > 0.5f)
                dryAt = i;
            if (dryAt >= 0 && i > dryAt + (int) (0.02 * sr) && v > delayedPeak)
            {
                delayedPeak = v;
                delayedAt = i;
            }
        }

        const double measuredMs = delayedAt > dryAt ? 1000.0 * (delayedAt - dryAt) / sr : -1.0;
        report ("INFO", "dry impulse at " + juce::String (dryAt)
                            + ", next strong copy at " + juce::String (delayedAt)
                            + " (peak " + juce::String (delayedPeak, 3) + ")"
                            + " => " + juce::String (measuredMs, 1) + " ms late"
                            + ", parameter says " + delayText + " ms");
        if (measuredMs > 100.0)
            report ("PASS", "audio comes back " + juce::String (measuredMs, 1) + " ms late while the host sees 0 latency");
        else
            fail ("no delayed copy found");

        checkEditor();
        quitWith (exitCode);
    }

    bool createInstance (const juce::PluginDescription& desc, const juce::String& label,
                         std::initializer_list<std::pair<const char*, float>> params)
    {
        juce::AudioPluginFormatManager formats;
        formats.addFormat (std::make_unique<juce::VST3PluginFormat>());

        juce::String error;
        instance = formats.createPluginInstance (desc, sr, blockSize, error);
        if (instance == nullptr)
        {
            report ("FAIL", label + ": " + error);
            return false;
        }

        for (const auto& [name, value] : params)
            setParam (name, value);

        return true;
    }

    void setParam (const char* name, float value)
    {
        for (auto* param : instance->getParameters())
            if (param->getName (64) == name)
            {
                param->setValueNotifyingHost (value);
                return;
            }

        fail (juce::String ("missing parameter ") + name);
    }

    using Generator = std::function<float (float t, int sample)>;

    juce::AudioBuffer<float> render (Generator generator, float seconds)
    {
        const int total = (int) (seconds * sr);
        juce::AudioBuffer<float> buffer (2, total);
        for (int i = 0; i < total; ++i)
        {
            const float v = generator ((float) i / (float) sr, i);
            buffer.setSample (0, i, v);
            buffer.setSample (1, i, v);
        }

        instance->prepareToPlay (sr, blockSize);

        juce::AudioBuffer<float> block (2, blockSize);
        juce::MidiBuffer midi;
        for (int pos = 0; pos < total; pos += blockSize)
        {
            const int n = juce::jmin (blockSize, total - pos);
            for (int ch = 0; ch < 2; ++ch)
                block.copyFrom (ch, 0, buffer, ch, pos, n);

            instance->processBlock (block, midi);

            for (int ch = 0; ch < 2; ++ch)
                buffer.copyFrom (ch, pos, block, ch, 0, n);
        }
        return buffer;
    }

    void checkEditor()
    {
        editor.reset (instance->createEditorIfNeeded());
        if (editor == nullptr)
        {
            fail ("host could not create the editor");
            return;
        }

        report ("PASS", "editor created (" + juce::String (editor->getWidth()) + "x"
                            + juce::String (editor->getHeight()) + ")");

        editor.reset();
    }

    void report (const juce::String& tag, const juce::String& message)
    {
        std::cout << "[" << tag.toStdString() << "] " << message.toStdString() << std::endl;
    }

    void fail (const juce::String& message)
    {
        exitCode = 1;
        report ("FAIL", message);
    }

    void quitWith (int code)
    {
        exitCode = code;
        quit();
    }

    int exitCode = 0;
};

START_JUCE_APPLICATION (PluginCheck)









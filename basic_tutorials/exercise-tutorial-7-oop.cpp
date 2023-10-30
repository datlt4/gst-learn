#include "gst/gst.h"
#include <iostream>
#include <regex>
#include <string>
#include <vector>

using GstElementPtr = GstElement *;
using GstPadPtr = GstPad *;

bool is_number(std::string &s)
{
    return std::regex_match(s.c_str(), std::regex("[(-|+)|][0-9.]+"));
}

/* Structure to contain all our information, so we can pass it to callbacks */
class Element
{
  public:
    virtual gboolean checkValid(void) = 0;
    virtual void gstElementFactoryMake(void) = 0;
    virtual GstElementPtr getElement(const char *_element_name) = 0;
    virtual GstPadPtr getPad(const char *_pad_name) = 0;
    virtual void setPad(const char *_pad_name, GstPadPtr pad) = 0;
    virtual gboolean linkManyElement(void) = 0;
};

using ElementPtr = Element *;

class AudioElement : public Element
{
  public:
    AudioElement() : audio_convert{nullptr}, audio_resample{nullptr}, audio_sink{nullptr}
    {
    }

    gboolean checkValid(void) override
    {
        return (gboolean)(audio_convert && audio_resample && audio_sink);
    }

    void gstElementFactoryMake(void) override
    {
        // audio_queue = gst_element_factory_make("queue", "audio_queue");
        audio_convert = gst_element_factory_make("audioconvert", "audio_convert");
        audio_resample = gst_element_factory_make("audioresample", "audio_resample");
        audio_sink = gst_element_factory_make("autoaudiosink", "audio_sink");
    }

    GstElementPtr getElement(const char *_element_name) override
    {
        std::string element_name{_element_name};
        if (element_name == "audio_convert")
        {
            return audio_convert;
        }
        else if (element_name == "audio_resample")
        {
            return audio_resample;
        }
        else if (element_name == "audio_sink")
        {
            return audio_sink;
        }
        else
        {
            return nullptr;
        }
    }

    GstPadPtr getPad(const char *_pad_name) override
    {
        return nullptr;
    }

    void setPad(const char *_pad_name, GstPadPtr pad) override
    {
    }

    gboolean linkManyElement(void) override
    {
        return gst_element_link_many(audio_convert, audio_resample, audio_sink, NULL);
    }

  private:
    // GstElementPtr audio_queue;
    GstElementPtr audio_convert;
    GstElementPtr audio_resample;
    GstElementPtr audio_sink;
};

using AudioElementPtr = AudioElement *;

class VideoElement : public Element
{
  public:
    VideoElement(std::string filter_name = "") : video_queue{nullptr}, video_convert{nullptr}, video_sink{nullptr}
    {
        this->filter_name = filter_name;
        if (checkFilterNameValid(filter_name))
        {
            video_filter = nullptr;
            video_convert_after_filter = nullptr;
        }
    }

    gboolean checkValid(void) override
    {
        return (gboolean)(video_queue && video_convert && video_sink &&
                          (!checkFilterNameValid(filter_name) || (video_filter && video_convert_after_filter)));
    }

    // Check if Element use filter or use a valid filter
    gboolean checkFilterNameValid(std::string &filter_name)
    {
        std::vector<std::string> list_video_filter_name = {"agingtv",      "dicetv",    "edgetv",    "optv",
                                                           "quarktv",      "radioactv", "revtv",     "rippletv",
                                                           "shagadelictv", "streaktv",  "vertigotv", "warptv"};
        std::vector<std::string>::iterator it =
            std::find(list_video_filter_name.begin(), list_video_filter_name.end(), filter_name);
        if (it == list_video_filter_name.end())
            return 0;
        else
            return 1;
    }

    void gstElementFactoryMake(void) override
    {
        video_queue = gst_element_factory_make("queue", "video_queue");
        video_convert = gst_element_factory_make("videoconvert", "video_convert1");
        if (checkFilterNameValid(this->filter_name))
        {
            video_filter = gst_element_factory_make(this->filter_name.c_str(), "video_filter");
            video_convert_after_filter = gst_element_factory_make("videoconvert", "video_convert_after_filter");
        }
        video_sink = gst_element_factory_make("autovideosink", "video_sink");
    }

    GstElementPtr getElement(const char *_element_name) override
    {
        std::string element_name{_element_name};
        if (element_name == "video_queue")
        {
            return video_queue;
        }
        else if (element_name == "video_convert")
        {
            return video_convert;
        }
        else if (element_name == "video_filter")
        {
            if (checkFilterNameValid(filter_name))
            {
                return video_filter;
            }
            else
            {
                return nullptr;
            }
        }
        else if (element_name == "video_convert_after_filter")
        {
            if (checkFilterNameValid(filter_name))
            {
                return video_convert_after_filter;
            }
            else
            {
                return nullptr;
            }
        }
        else if (element_name == "video_sink")
        {
            return video_sink;
        }
        else
        {
            return nullptr;
        }
    }

    GstPadPtr getPad(const char *_pad_name) override
    {
        std::string pad_name{_pad_name};
        if (pad_name == "queue_video_pad")
        {
            return queue_video_pad;
        }
        else if (pad_name == "tee_video_pad")
        {
            return tee_video_pad;
        }
        else
        {
            return nullptr;
        }
    }

    void setPad(const char *_pad_name, GstPadPtr pad) override
    {
        std::string pad_name{_pad_name};
        if (pad_name == "queue_video_pad")
        {
            queue_video_pad = pad;
        }
        else if (pad_name == "tee_video_pad")
        {
            tee_video_pad = pad;
        }
    }

    gboolean linkManyElement(void) override
    {
        if (checkFilterNameValid(filter_name))
        {
            std::cout << "HAS filter" << std::endl;
            return gst_element_link_many(video_queue, video_convert, video_filter, video_convert_after_filter,
                                         video_sink, NULL);
        }
        else
        {
            std::cout << "NO filter" << std::endl;
            return gst_element_link_many(video_queue, video_convert, video_sink, NULL);
        }
    }

  private:
    std::string filter_name;
    GstElementPtr video_queue;
    GstElementPtr video_convert;
    GstElementPtr video_filter;
    GstElementPtr video_convert_after_filter;
    GstElementPtr video_sink;
    GstPadPtr queue_video_pad;
    GstPadPtr tee_video_pad;
};

using VideoElementPtr = VideoElement *;

class TeeElement : public Element
{
  public:
    TeeElement() : tee{nullptr}
    {
    }

    gboolean checkValid(void) override
    {
        if (tee)
        {
            return (gboolean)(1);
        }
        else
        {
            return (gboolean)(0);
        }
    }

    void gstElementFactoryMake(void) override
    {
        tee = gst_element_factory_make("tee", "tee");
    }

    GstElementPtr getElement(const char *_element_name) override
    {
        std::string element_name{_element_name};
        if (element_name == "tee")
        {
            return tee;
        }
        else
        {
            return nullptr;
        }
    }

    GstPadPtr getPad(const char *_pad_name) override
    {
        return nullptr;
    }

    void setPad(const char *_pad_name, GstPadPtr pad) override
    {
    }

    gboolean linkManyElement(void) override
    {
        return 1;
    }

  private:
    GstElementPtr tee;
};

using TeeElementPtr = TeeElement *;

class PipelineAction
{
  public:
    virtual GstStateChangeReturn changeStatePlaying(void) = 0;
    virtual GstStateChangeReturn changeStateNull(void) = 0;
    virtual GstStateChangeReturn changeStateReady(void) = 0;
    virtual GstStateChangeReturn changeStatePaused(void) = 0;
    virtual void setSourceProperties(std::string &url) = 0;
    virtual void addManyElement(void) = 0;
    virtual gboolean linkRequestPadsTee(void) = 0;
    virtual void unref(void) = 0;
};

class PipelineElement : public PipelineAction, public Element
{
  public:
    PipelineElement() : pipeline{nullptr}, source{nullptr}, tee{nullptr}
    {
        list_elements.push_back(new AudioElement());
        list_elements.push_back(new VideoElement());
        // list_elements.push_back(new VideoElement("agingtv"));
        // list_elements.push_back(new VideoElement("dicetv"));
        // list_elements.push_back(new VideoElement("edgetv"));
        // list_elements.push_back(new VideoElement("optv"));
        // list_elements.push_back(new VideoElement("quarktv"));
        // list_elements.push_back(new VideoElement("radioactv"));
        // list_elements.push_back(new VideoElement("revtv"));
        // list_elements.push_back(new VideoElement("rippletv"));
        // list_elements.push_back(new VideoElement("shagadelictv"));
        // list_elements.push_back(new VideoElement("streaktv"));
        // list_elements.push_back(new VideoElement("vertigotv"));
        // list_elements.push_back(new VideoElement("warptv"));
    }

    ~PipelineElement()
    {
        for (ElementPtr ele : list_elements)
        {
            delete ele;
        }
        std::cout << __FUNCTION__ << std::endl;
    }

    GstStateChangeReturn changeStatePlaying() override
    {
        return gst_element_set_state(pipeline, GST_STATE_PLAYING);
    }

    GstStateChangeReturn changeStateNull() override
    {
        return gst_element_set_state(pipeline, GST_STATE_NULL);
    }

    GstStateChangeReturn changeStateReady() override
    {
        return gst_element_set_state(pipeline, GST_STATE_READY);
    }

    GstStateChangeReturn changeStatePaused() override
    {
        return gst_element_set_state(pipeline, GST_STATE_PAUSED);
    }

    void setSourceProperties(std::string &url) override
    {
        /* Set the URI to play */
        g_object_set(source, "uri", url.c_str(), NULL);
    }

    void addManyElement(void) override
    {
        gst_bin_add_many(GST_BIN(getElement("pipeline")), getElement("source"), getElement("tee"),
                         getElement("list_elements.0.audio_convert"), getElement("list_elements.0.audio_resample"),
                         getElement("list_elements.0.audio_sink"), getElement("list_elements.1.video_queue"),
                         getElement("list_elements.1.video_convert"), getElement("list_elements.1.video_sink"), NULL);
    }

    gboolean linkManyElement(void) override
    {
        gboolean r = 1;
        for (ElementPtr ele : list_elements)
        {
            r &= ele->linkManyElement();
        }
        return r;
    }

    void unref(void) override
    {
        /* Release the request pads from the Tee, and unref them */
        for (ElementPtr ele : list_elements)
        {
            GstPadPtr tee_video_pad = ele->getPad("tee_video_pad");
            GstPadPtr queue_video_pad = ele->getPad("queue_video_pad");
            if (tee_video_pad)
            {
                gst_element_release_request_pad(tee->getElement("tee"), tee_video_pad);
                gst_object_unref(tee_video_pad);
                gst_object_unref(queue_video_pad);
            }
        }
        gst_object_unref(pipeline);
    }

    gboolean checkValid(void) override
    {
        gboolean ret = 1;

        for (ElementPtr ele : list_elements)
        {
            ret &= ele->checkValid();
            if (!ret)
                return 0;
        }

        return (gboolean)(ret && pipeline && source && tee);
    }

    void gstElementFactoryMake(void) override
    {
        for (ElementPtr ele : list_elements)
            ele->gstElementFactoryMake();

        tee->gstElementFactoryMake();
        source = gst_element_factory_make("uridecodebin", "source");
        pipeline = gst_pipeline_new("test-pipeline");
    }

    GstElementPtr getElement(const char *_element_name) override
    {
        std::string element_name{_element_name};
        if (element_name == "pipeline")
        {
            return pipeline;
        }
        else if (element_name == "source")
        {
            return source;
        }
        else if (element_name == "tee")
        {
            return tee->getElement("tee");
        }
        else
        {
            if (!list_elements.empty())
            {
                return nullptr;
            }

            int idx = element_name.find(".");

            if (idx < 0 || std::string_view(element_name.c_str(), idx) != "list_elements")
            {
                return nullptr;
            }
            std::string_view sub(element_name.c_str() + idx + 1);

            idx = sub.find(".");

            if (idx < 0)
            {
                return nullptr;
            }

            std::string list_elements_idx_char{std::string_view(element_name.c_str(), idx)};
            if (is_number(list_elements_idx_char))
            {
                int list_elements_idx = std::stoi(list_elements_idx_char);
                if (list_elements_idx < list_elements.size())
                {
                    std::string_view sub_element_name{sub.data() + idx + 1};
                    return list_elements[list_elements_idx]->getElement(sub_element_name.data());
                }
                else
                {
                    return nullptr;
                }
            }
            else
            {
                return nullptr;
            }
        }
    }

    GstPadPtr getPad(const char *_pad_name) override
    {
        return nullptr;
    }

    void setPad(const char *_pad_name, GstPadPtr pad) override
    {
    }

    gboolean linkRequestPadsTee(void) override
    {
        int ei = 1; // element index

        GstPadPtr queue_video_pad = gst_element_get_static_pad(list_elements[ei]->getElement("video_queue"), "sink");
        GstPadPtr tee_video_pad = gst_element_get_request_pad(tee->getElement("tee"), "src_%u");
        list_elements[ei]->setPad("queue_video_pad", queue_video_pad);
        list_elements[ei]->setPad("tee_video_pad", tee_video_pad);
        g_print("Obtained request pad %s for Tee branch.\n", gst_pad_get_name(tee_video_pad));
        g_print("Obtained static pad %s for video_element.\n", gst_pad_get_name(queue_video_pad));
        GstPadLinkReturn ra = gst_pad_link(tee_video_pad, queue_video_pad);

        std::cout << "ra: " << ra << std::endl;

        switch (ra)
        {
        case GST_PAD_LINK_OK:
            g_print("link succeeded\n");
            break;
        case GST_PAD_LINK_WRONG_HIERARCHY:
            g_printerr("pads have no common grandparent\n");
            break;
        case GST_PAD_LINK_WAS_LINKED:
            g_printerr("pad was already linked\n");
            break;
        case GST_PAD_LINK_WRONG_DIRECTION:
            g_printerr("pads have wrong direction\n");
            break;
        case GST_PAD_LINK_NOFORMAT:
            g_printerr("pads do not have common format\n");
            break;
        case GST_PAD_LINK_NOSCHED:
            g_printerr("pads cannot cooperate in scheduling\n");
            break;
        case GST_PAD_LINK_REFUSED:
            g_printerr("refused for some reason\n");
            break;
        default:
            break;
        }

        if (ra != GST_PAD_LINK_OK)
        {
            return 0;
        }

        return 1;
    }

  private:
    GstElementPtr pipeline;
    GstElementPtr source;
    ElementPtr tee;
    std::vector<ElementPtr> list_elements;
};

using PipelineElementPtr = PipelineElement *;

/* Handler for the pad-added signal */
static void pad_added_handler(GstElementPtr src, GstPadPtr pad, PipelineElementPtr data);

int main(int argc, char **argv)
{
    /* Define Elements */
    PipelineElementPtr pipeline = new PipelineElement();
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;
    gboolean terminate = FALSE;

    /* Initialize GStreamer */
    gst_init(&argc, &argv);

    /* Create the elements */
    pipeline->gstElementFactoryMake();

    if (!pipeline->checkValid())
    {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    /* Set the URI to play */
    std::string url = "https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm";
    pipeline->setSourceProperties(url);

    /* Build the pipeline. Note that we are NOT linking the source at this point. We will do it later. */
    pipeline->addManyElement();

    if (!pipeline->linkManyElement())
    {
        g_printerr("Elements could not be linked.\n");
        pipeline->unref();
        return -1;
    }

    /* Manually link the Tee, which has "Request" pads */
    gboolean ra = pipeline->linkRequestPadsTee();
    if (!ra)
    {
        g_printerr("Tee could not be linked.\n");
        pipeline->unref();
        return -1;
    }

    /* Connect to the pad-added signal */
    g_signal_connect(pipeline->getElement("source"), "pad-added", G_CALLBACK(pad_added_handler), &pipeline);

    /* Start playing */
    ret = gst_element_set_state(pipeline->getElement("pipeline"), GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        pipeline->unref();
        return -1;
    }

    /* Listen to the bus */
    bus = gst_element_get_bus(pipeline->getElement("pipeline"));

    do
    {
        msg = gst_bus_timed_pop_filtered(
            bus, GST_CLOCK_TIME_NONE,
            (GstMessageType)(GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

        /* Parse message */
        if (msg != NULL)
        {
            GError *err;
            gchar *debug_info;

            switch (GST_MESSAGE_TYPE(msg))
            {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error(msg, &err, &debug_info);
                g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
                g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
                g_clear_error(&err);
                g_free(debug_info);
                terminate = TRUE;
                break;

            case GST_MESSAGE_EOS:
                g_print("End-Of-Stream reached.\n");
                terminate = TRUE;
                break;

            case GST_MESSAGE_STATE_CHANGED:
                /* We are only interested in state-changed messages from the pipeline */
                if (GST_MESSAGE_SRC(msg) == GST_OBJECT(pipeline->getElement("pipeline")))
                {
                    GstState old_state, new_state, pending_state;
                    gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
                    g_print("Pipeline state changed from %s to %s:\n", gst_element_state_get_name(old_state),
                            gst_element_state_get_name(new_state));
                }
                break;

            default:
                /* We should not reach here */
                g_printerr("Unexpected message received.\n");
                break;
            }
            gst_message_unref(msg);
        }
    } while (!terminate);

    /* Free resources */
    pipeline->changeStateNull();
    pipeline->unref();
    delete pipeline;
    std::cout << __FUNCTION__ << std::endl;
}

/* This function will be called by the pad-added signal */
static void pad_added_handler(GstElementPtr src, GstPadPtr new_pad, PipelineElementPtr data)
{
    GstPadLinkReturn ret;
    GstCaps *new_pad_caps = NULL;
    GstStructure *new_pad_struct = NULL;
    const gchar *new_pad_type = NULL;

    g_print("Received new pad '%s' from '%s':\n", GST_PAD_NAME(new_pad), GST_ELEMENT_NAME(src));

    /* Check the new pad's type */
    new_pad_caps = gst_pad_get_current_caps(new_pad);
    new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
    new_pad_type = gst_structure_get_name(new_pad_struct);
    std::cout << "new_pad_type: " << new_pad_type << std::endl;
    GstPad *sink_pad;
    if (g_str_has_prefix(new_pad_type, "audio/x-raw"))
    {
        sink_pad = gst_element_get_static_pad(data->getElement("list_elements.0.audio_convert"), "sink");
    }
    else if (g_str_has_prefix(new_pad_type, "video/x-raw"))
    {
        sink_pad = gst_element_get_static_pad(data->getElement("tee"), "tee");
    }
    else
    {
        g_print("It has type '%s' which is not raw video or raw audio. Ignoring.\n", new_pad_type);
        goto exit;
    }

    /* If our converter is already linked, we have nothing to do here */
    if (gst_pad_is_linked(sink_pad))
    {
        g_print("We are already linked. Ignoring.\n");
        goto exit;
    }

    /* Attempt the link */
    ret = gst_pad_link(new_pad, sink_pad);
    if (GST_PAD_LINK_FAILED(ret))
    {
        g_printerr("Type is '%s' but link failed.\n", new_pad_type);
    }
    else
    {
        g_print("Link succeeded (type '%s').\n", new_pad_type);
    }

exit:
    /* Unreference the new pad's caps, if we got them */
    if (new_pad_caps != NULL)
    {
        gst_caps_unref(new_pad_caps);
    }

    /* Unreference the sink pad */
    gst_object_unref(sink_pad);
}

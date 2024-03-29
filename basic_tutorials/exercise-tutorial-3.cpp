#include "gst/gst.h"
#include <iostream>

/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _CustomData
{
    GstElement *pipeline;
    GstElement *source;
    GstElement *audio_convert;
    GstElement *audio_resample;
    GstElement *audio_sink;
    GstElement *video_convert1;
    GstElement *video_filter;
    GstElement *video_convert2;
    GstElement *video_sink;

    _CustomData()
        : pipeline{nullptr}, source{nullptr}, audio_convert{nullptr}, audio_resample{nullptr}, audio_sink{nullptr},
          video_filter{nullptr}, video_convert1{nullptr}, video_convert2{nullptr}, video_sink{nullptr}
    {
    }

    gboolean checkValid(void)
    {
        return (gboolean)(pipeline && source && audio_convert && audio_resample && audio_sink && video_filter &&
                          video_convert1 && video_convert2 && video_sink);
    }

    void unref(void)
    {
        gst_object_unref(pipeline);
    }
} CustomData;

/* Handler for the pad-added signal */
static void pad_added_handler(GstElement *src, GstPad *pad, CustomData *data);

int main(int argc, char **argv)
{
    CustomData data;
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;
    gboolean terminate = FALSE;

    /* Initialize GStreamer */
    gst_init(&argc, &argv);

    /* Create the elements */
    data.source = gst_element_factory_make("uridecodebin", "source");

    data.audio_resample = gst_element_factory_make("audioresample", "audio_resample");
    data.audio_convert = gst_element_factory_make("audioconvert", "audio_convert");
    data.audio_sink = gst_element_factory_make("autoaudiosink", "audio_sink");

    data.video_filter = gst_element_factory_make("agingtv", "filter");
    // data.video_filter = gst_element_factory_make("dicetv", "filter");
    // data.video_filter = gst_element_factory_make("edgetv", "filter");
    // data.video_filter = gst_element_factory_make("optv", "filter");
    // data.video_filter = gst_element_factory_make("quarktv", "filter");
    // data.video_filter = gst_element_factory_make("radioactv", "filter");
    // data.video_filter = gst_element_factory_make("revtv", "filter");
    // data.video_filter = gst_element_factory_make("rippletv", "filter");
    // data.video_filter = gst_element_factory_make("shagadelictv", "filter");
    // data.video_filter = gst_element_factory_make("streaktv", "filter");
    // data.video_filter = gst_element_factory_make("vertigotv", "filter");
    // data.video_filter = gst_element_factory_make("warptv", "filter");
    data.video_convert1 = gst_element_factory_make("videoconvert", "video_convert1");
    data.video_convert2 = gst_element_factory_make("videoconvert", "video_convert2");
    data.video_sink = gst_element_factory_make("autovideosink", "video_sink");

    /* Create the empty pipeline */
    data.pipeline = gst_pipeline_new("test-pipeline");

    if (!data.checkValid())
    {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    /* Build the pipeline. Note that we are NOT linking the source at this point. We will do it later. */
    gst_bin_add_many(GST_BIN(data.pipeline), data.source, data.audio_convert, data.audio_resample, data.audio_sink,
                     data.video_filter, data.video_convert1, data.video_convert2, data.video_sink, NULL);

    if (!gst_element_link_many(data.audio_convert, data.audio_resample, data.audio_sink, NULL))
    {
        g_printerr("Elements could not be linked.\n");
        data.unref();
        return -1;
    }

    if (!gst_element_link_many(data.video_convert1, data.video_filter, data.video_convert2, data.video_sink, NULL))
    {
        g_printerr("Elements could not be linked.\n");
        data.unref();
        return -1;
    }

    /* Set the URI to play */
    g_object_set(data.source, "uri", "https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm", NULL);

    /* Connect to the pad-added signal */
    g_signal_connect(data.source, "pad-added", G_CALLBACK(pad_added_handler), &data);

    /* Start playing */
    ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        data.unref();
        return -1;
    }

    /* Listen to the bus */
    bus = gst_element_get_bus(data.pipeline);

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
                if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data.pipeline))
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
    gst_object_unref(bus);
    gst_element_set_state(data.pipeline, GST_STATE_NULL);
    data.unref();

    return 0;
}

/* This function will be called by the pad-added signal */
static void pad_added_handler(GstElement *src, GstPad *new_pad, CustomData *data)
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
        sink_pad = gst_element_get_static_pad(data->audio_convert, "sink");
    }
    else if (g_str_has_prefix(new_pad_type, "video/x-raw"))
    {
        sink_pad = gst_element_get_static_pad(data->video_convert1, "sink");
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

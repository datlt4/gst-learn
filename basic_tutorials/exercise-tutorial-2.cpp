#include <gst/gst.h>
#include <iostream>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

typedef struct _CustomData
{
    GstElement *pipeline;
    GstElement *source;
    GstElement *sink;
    GstElement *filter;
    GstElement *converter;

    _CustomData() : pipeline{nullptr}, source{nullptr}, sink{nullptr}, filter{nullptr}, converter{nullptr}
    {
    }

    gboolean checkValid(void)
    {
        return (gboolean)(pipeline && source && sink && filter && converter);
    }

    void unref(void)
    {
        gst_object_unref(pipeline);
    }
} CustomData;

int tutorial_main(int argc, char **argv)
{
    CustomData data;
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;

    /* Initialize Gstreamer */
    gst_init(&argc, &argv);

    /* Create the elements */
    data.source = gst_element_factory_make("videotestsrc", "source");
    data.sink = gst_element_factory_make("autovideosink", "sink");
    // data.filter = gst_element_factory_make("agingtv", "filter");
    // data.filter = gst_element_factory_make("dicetv", "filter");
    // data.filter = gst_element_factory_make("edgetv", "filter");
    // data.filter = gst_element_factory_make("optv", "filter");
    // data.filter = gst_element_factory_make("quarktv", "filter");
    // data.filter = gst_element_factory_make("radioactv", "filter");
    // data.filter = gst_element_factory_make("revtv", "filter");
    // data.filter = gst_element_factory_make("rippletv", "filter");
    // data.filter = gst_element_factory_make("shagadelictv", "filter");
    // data.filter = gst_element_factory_make("streaktv", "filter");
    // data.filter = gst_element_factory_make("vertigotv", "filter");
    data.filter = gst_element_factory_make("warptv", "filter");
    data.converter = gst_element_factory_make("videoconvert", "converter");

    /* Create the empty pipeline */
    data.pipeline = gst_pipeline_new("test-pipeline");

    if (!data.checkValid())
    {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    /* Build the pipeline */
    gst_bin_add_many(GST_BIN(data.pipeline), data.source, data.filter, data.converter, data.sink, NULL);

    // if (gst_element_link_many(data.filter, data.converter, data.sink, NULL) != TRUE)
    if (gst_element_link(data.source, data.filter) != TRUE || gst_element_link(data.filter, data.converter) != TRUE ||
        gst_element_link(data.converter, data.sink) != TRUE)
    {
        g_printerr("Elements could not be linked.\n");
        data.unref();
        return -1;
    }

    /* Start playing */
    ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr("Unable to set the pipeline to the playing state\n");
        data.unref();
        return -1;
    }

    /* Wait until error or EOS */
    bus = gst_element_get_bus(data.pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

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
            break;
        case GST_MESSAGE_EOS:
            g_print("End-Of-Stream reached.\n");
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

    /* Free resources */
    gst_object_unref(bus);
    gst_element_set_state(data.pipeline, GST_STATE_NULL);
    data.unref();
    return 0;
}

int main(int argc, char *argv[])
{
#if defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
    return gst_macos_main(tutorial_main, argc, argv, NULL);
#else
    return tutorial_main(argc, argv);
#endif
}

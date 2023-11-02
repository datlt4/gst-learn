#include "gst/gst.h"
#include <iostream>

/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _CustomData
{
    GstElement *pipeline;
    GstElement *source;

    GstElement *audio_convert, *audio_resample;
    GstElement *tee_audio;
    GstElement *audio_queue, *audio_sink;
    GstElement *wavescope_queue, *wavescope, *wavescope_convert, *wavescope_sink;
    GstElement *file_queue, *file_wavenc, *filesink;

    GstElement *tee_video;
    GstElement *filter_video_queue, *filter_video_convert1, *filter_video_filter, *filter_video_convert2,
        *filter_video_sink;
    GstElement *origin_video_queue, *origin_video_convert, *origin_video_sink;

    _CustomData()
        : pipeline{nullptr}, source{nullptr}, audio_convert{nullptr}, audio_resample{nullptr}, tee_audio{nullptr},
          audio_queue{nullptr}, audio_sink{nullptr}, wavescope_queue{nullptr}, wavescope{nullptr},
          wavescope_convert{nullptr}, wavescope_sink{nullptr}, file_queue{nullptr}, file_wavenc{nullptr},
          filesink{nullptr}, tee_video{nullptr}, filter_video_queue{nullptr}, filter_video_convert1{nullptr},
          filter_video_filter{nullptr}, filter_video_convert2{nullptr}, filter_video_sink{nullptr},
          origin_video_queue{nullptr}, origin_video_convert{nullptr}, origin_video_sink{nullptr}
    {
    }

    gboolean checkValid(void)
    {
        return (gboolean)(pipeline && source && audio_convert && audio_resample && tee_audio && audio_queue &&
                          audio_sink && wavescope_queue && wavescope && wavescope_convert && wavescope_sink &&
                          file_queue && file_wavenc && filesink && tee_video && filter_video_queue &&
                          filter_video_convert1 && filter_video_filter && filter_video_convert2 && filter_video_sink &&
                          origin_video_queue && origin_video_convert && origin_video_sink);
    }

    GstStateChangeReturn changeStatePlaying(void)
    {
        return gst_element_set_state(pipeline, GST_STATE_PLAYING);
    }

    GstStateChangeReturn changeStateNull(void)
    {
        return gst_element_set_state(pipeline, GST_STATE_NULL);
    }

    GstStateChangeReturn changeStateReady(void)
    {
        return gst_element_set_state(pipeline, GST_STATE_READY);
    }

    GstStateChangeReturn changeStatePaused(void)
    {
        return gst_element_set_state(pipeline, GST_STATE_PAUSED);
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
    /* Define Elements */
    CustomData data;
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;
    gboolean terminate = FALSE;

    GstPad *tee_audio_pad, *queue_audio_pad;
    GstPad *tee_wavescope_pad, *queue_wavescope_pad;
    GstPad *tee_file_pad, *queue_file_pad;
    GstPad *tee_filter_video_pad, *queue_filter_video_pad;
    GstPad *tee_origin_video_pad, *queue_origin_video_pad;

    /* Initialize GStreamer */
    gst_init(&argc, &argv);

    /* Create the elements */
    data.source = gst_element_factory_make("uridecodebin", "source");

    data.audio_convert = gst_element_factory_make("audioconvert", "audio_convert");
    data.audio_resample = gst_element_factory_make("audioresample", "audio_resample");
    data.tee_audio = gst_element_factory_make("tee", "tee_audio");
    data.audio_queue = gst_element_factory_make("queue", "audio_queue");
    data.audio_sink = gst_element_factory_make("autoaudiosink", "audio_sink");
    data.wavescope_queue = gst_element_factory_make("queue", "wavescope_queue");
    data.wavescope = gst_element_factory_make("wavescope", "wavescope");
    data.wavescope_convert = gst_element_factory_make("videoconvert", "wavescope_convert");
    data.wavescope_sink = gst_element_factory_make("autovideosink", "wavescope_sink");
    data.file_queue = gst_element_factory_make("queue", "file_queue");
    data.file_wavenc = gst_element_factory_make("wavenc", "file_wavenc");
    data.filesink = gst_element_factory_make("filesink", "filesink");

    data.tee_video = gst_element_factory_make("tee", "tee_video");
    data.filter_video_queue = gst_element_factory_make("queue", "filter_video_queue");
    data.filter_video_convert1 = gst_element_factory_make("videoconvert", "filter_video_convert1");
    data.filter_video_filter = gst_element_factory_make("agingtv", "filter_video_filter");
    //"agingtv" | "dicetv" | "edgetv" | "optv" | "quarktv" | "radioactv" | "revtv" | "rippletv" | "shagadelictv" |
    //"streaktv" | "vertigotv" | "warptv";
    data.filter_video_convert2 = gst_element_factory_make("videoconvert", "filter_video_convert2");
    data.filter_video_sink = gst_element_factory_make("autovideosink", "filter_video_sink");

    data.origin_video_queue = gst_element_factory_make("queue", "origin_video_queue");
    data.origin_video_convert = gst_element_factory_make("videoconvert", "origin_video_convert");
    data.origin_video_sink = gst_element_factory_make("autovideosink", "origin_video_sink");

    /* Create the empty pipeline */
    data.pipeline = gst_pipeline_new("test-pipeline");

    if (!data.checkValid())
    {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    /* Build the pipeline. Note that we are NOT linking the source at this point. We will do it later. */
    gst_bin_add_many(GST_BIN(data.pipeline), data.source, data.audio_convert, data.audio_resample, data.tee_audio,
                     data.audio_queue, data.audio_sink, data.wavescope_queue, data.wavescope, data.wavescope_convert,
                     data.wavescope_sink, data.file_queue, data.file_wavenc, data.filesink, data.tee_video,
                     data.filter_video_queue, data.filter_video_convert1, data.filter_video_filter,
                     data.filter_video_convert2, data.filter_video_sink, data.origin_video_queue,
                     data.origin_video_convert, data.origin_video_sink, NULL);

    if (gst_element_link_many(data.audio_convert, data.audio_resample, data.tee_audio, NULL) != TRUE ||
        gst_element_link_many(data.audio_queue, data.audio_sink, NULL) != TRUE ||
        gst_element_link_many(data.wavescope_queue, data.wavescope, data.wavescope_convert, data.wavescope_sink,
                              NULL) != TRUE ||
        gst_element_link_many(data.file_queue, data.file_wavenc, data.filesink, NULL) != TRUE ||
        gst_element_link_many(data.filter_video_queue, data.filter_video_convert1, data.filter_video_filter,
                              data.filter_video_convert2, data.filter_video_sink, NULL) != TRUE ||
        gst_element_link_many(data.origin_video_queue, data.origin_video_convert, data.origin_video_sink, NULL) != TRUE)
    {
        g_printerr("Elements could not be linked.\n");
        data.unref();
        return -1;
    }

    /* Manually link the Tee, which has "Request" pads */
    tee_audio_pad = gst_element_get_request_pad(data.tee_audio, "src_%u");
    queue_audio_pad = gst_element_get_static_pad(data.audio_queue, "sink");
    g_print("Obtained request pad %s for audio branch.\n", gst_pad_get_name(tee_audio_pad));

    tee_wavescope_pad = gst_element_get_request_pad(data.tee_audio, "src_%u");
    queue_wavescope_pad = gst_element_get_static_pad(data.wavescope_queue, "sink");
    g_print("Obtained request pad %s for wavescope branch.\n", gst_pad_get_name(tee_wavescope_pad));

    tee_file_pad = gst_element_get_request_pad(data.tee_audio, "src_%u");
    queue_file_pad = gst_element_get_static_pad(data.file_queue, "sink");
    g_print("Obtained request pad %s for wavescope branch.\n", gst_pad_get_name(tee_file_pad));

    tee_filter_video_pad = gst_element_get_request_pad(data.tee_video, "src_%u");
    queue_filter_video_pad = gst_element_get_static_pad(data.filter_video_queue, "sink");
    g_print("Obtained request pad %s for filter_video branch.\n", gst_pad_get_name(tee_filter_video_pad));

    tee_origin_video_pad = gst_element_get_request_pad(data.tee_video, "src_%u");
    queue_origin_video_pad = gst_element_get_static_pad(data.origin_video_queue, "sink");
    g_print("Obtained request pad %s for origin_video branch.\n", gst_pad_get_name(tee_origin_video_pad));

    GstPadLinkReturn ra = gst_pad_link(tee_audio_pad, queue_audio_pad);
    GstPadLinkReturn rw = gst_pad_link(tee_wavescope_pad, queue_wavescope_pad);
    GstPadLinkReturn rs = gst_pad_link(tee_file_pad, queue_file_pad);
    GstPadLinkReturn rf = gst_pad_link(tee_filter_video_pad, queue_filter_video_pad);
    GstPadLinkReturn rv = gst_pad_link(tee_origin_video_pad, queue_origin_video_pad);

    std::cout << "ra: " << ra << "  rw: " << rw << "  rs: " << rs << "  rf: " << rf << "  rv: " << rv << std::endl;

    if (ra != GST_PAD_LINK_OK || rw != GST_PAD_LINK_OK || rs != GST_PAD_LINK_OK || rf != GST_PAD_LINK_OK ||
        rv != GST_PAD_LINK_OK)
    {
        g_printerr("Tee could not be linked.\n");
        gst_object_unref(data.pipeline);
        return -1;
    }

    gst_object_unref(queue_audio_pad);
    gst_object_unref(queue_wavescope_pad);
    gst_object_unref(queue_file_pad);
    gst_object_unref(queue_filter_video_pad);
    gst_object_unref(queue_origin_video_pad);

    /* Connect to the pad-added signal */
    g_signal_connect(data.source, "pad-added", G_CALLBACK(pad_added_handler), &data);

    /* Set the URI to play */
    g_object_set(data.source, "uri", "https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm", NULL);

    /* Set the location to save audio file */
    g_object_set(data.filesink, "location", "test.wav", NULL);

    /* Set the audio visualize style */
    g_object_set(data.wavescope, "shader", 0, "style", 1, NULL);

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

    /* Release the request pads from the Tee, and unref them */
    gst_element_release_request_pad(data.tee_audio, tee_audio_pad);
    gst_element_release_request_pad(data.tee_audio, tee_wavescope_pad);
    gst_element_release_request_pad(data.tee_audio, tee_file_pad);
    gst_element_release_request_pad(data.tee_video, tee_filter_video_pad);
    gst_element_release_request_pad(data.tee_video, tee_origin_video_pad);
    gst_object_unref(tee_audio_pad);
    gst_object_unref(tee_wavescope_pad);
    gst_object_unref(tee_file_pad);
    gst_object_unref(tee_filter_video_pad);
    gst_object_unref(tee_origin_video_pad);

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
        sink_pad = gst_element_get_static_pad(data->tee_video, "sink");
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

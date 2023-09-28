# gst-learn

## Format C++ code

```bash
apt install clang-format -y
clang-format -style=microsoft -dump-config > .clang-format
sed -i 's/AlwaysBreakTemplateDeclarations: MultiLine/AlwaysBreakTemplateDeclarations: false/g' .clang-format

# To use
find . -regex '.*\.\(c\|cc\|cpp\|cxx\|cu\|h\|hh\|hpp\|hxx\|inl\|inc\|ipp\|m\|mm\)$' -exec clang-format -style=file -i {} \;
```


## Pre-commit

```bash
python3 -m pip install pre-commit
pre-commit install      # runs every time you commit in git
pre-commit run -a       # To use
pre-commit autoupdate   # To update this file
```

# Tutorials

## [Basic Tutorial 1: Hello World!](https://gstreamer.freedesktop.org/documentation/tutorials/basic/hello-world.html?gi-language=c)

- See source code at [`basic-tutorial-1.cpp`](basic_tutorials/basic-tutorial-1.cpp#L22).

### [Conclusion](https://gstreamer.freedesktop.org/documentation/tutorials/basic/hello-world.html?gi-language=c#conclusion)

- Initialize GStreamer using [`gst_init()`](https://gstreamer.freedesktop.org/documentation/gstreamer/gst.html#gst_init).

- Quickly build a pipeline from a textual description using [`gst_parse_launch()`](https://gstreamer.freedesktop.org/documentation/gstreamer/gstparse.html#gst_parse_launch).

- Create an automatic playback pipeline using [`playbin`](https://gstreamer.freedesktop.org/documentation/playback/playbin.html#playbin).

- Signal GStreamer to start playback using [`gst_element_set_state()`](https://gstreamer.freedesktop.org/documentation/gstreamer/gstelement.html#gst_element_set_state).

- Sit back and relax, while GStreamer takes care of everything, using [`gst_element_get_bus()`](https://gstreamer.freedesktop.org/documentation/gstreamer/gstelement.html#gst_element_get_bus) and [`gst_bus_timed_pop_filtered()`](https://gstreamer.freedesktop.org/documentation/gstreamer/gstbus.html#gst_bus_timed_pop_filtered).

## [Basic tutorial 2: GStreamer concepts](https://gstreamer.freedesktop.org/documentation/tutorials/basic/concepts.html?gi-language=c#basic-tutorial-2-gstreamer-concepts)

- See source code at [`basic-tutorial-2.cpp`](basic_tutorials/basic-tutorial-2.cpp).

### [Conclusion](https://gstreamer.freedesktop.org/documentation/tutorials/basic/concepts.html?gi-language=c#conclusion)

- Create elements with [`gst_element_factory_make()`](https://gstreamer.freedesktop.org/documentation/gstreamer/gstelementfactory.html#gst_element_factory_make).

- Create an empty pipeline with [`gst_pipeline_new()`](https://gstreamer.freedesktop.org/documentation/gstreamer/gstpipeline.html#gst_pipeline_new).

- Add elements to the pipeline with [`gst_bin_add_many()`](https://gstreamer.freedesktop.org/documentation/gstreamer/gstbin.html#gst_bin_add_many).

- Link the elements with each other with [`gst_element_link()`](https://gstreamer.freedesktop.org/documentation/gstreamer/gstelement.html#gst_element_link).

## [Basic tutorial 3: Dynamic pipelines](https://gstreamer.freedesktop.org/documentation/tutorials/basic/dynamic-pipelines.html?gi-language=c#basic-tutorial-3-dynamic-pipelines)

- See source code at [`basic-tutorial-3.cpp`](basic_tutorials/basic-tutorial-3.cpp).

### [Conclusion](https://gstreamer.freedesktop.org/documentation/tutorials/basic/dynamic-pipelines.html?gi-language=c#conclusion)

- Be notified of events using `GSignals`.

- Connect `GstPads` directly instead of their parent elements.

- The various states of a GStreamer element.

## [Basic tutorial 4: Time management](https://gstreamer.freedesktop.org/documentation/tutorials/basic/time-management.html?gi-language=c#basic-tutorial-4-time-management)

- See source code at [`basic-tutorial-4.cpp`](basic_tutorials/basic-tutorial-4.cpp).

### [Conclusion](https://gstreamer.freedesktop.org/documentation/tutorials/basic/time-management.html?gi-language=c#conclusion)

- How to query the pipeline for information using `GstQuery`

- How to obtain common information like position and duration using `gst_element_query_position()` and `gst_element_query_duration()`.

- How to seek to an arbitrary position in the stream using `gst_element_seek_simple()`.

- In which states all these operations can be performed.

## [Basic tutorial 5: GUI toolkit integration](https://gstreamer.freedesktop.org/documentation/tutorials/basic/toolkit-integration.html?gi-language=c#basic-tutorial-5-gui-toolkit-integration)

- See source code at [`basic-tutorial-5.cpp`](basic_tutorials/basic-tutorial-5.cpp).

### [Conclusion](https://gstreamer.freedesktop.org/documentation/tutorials/basic/toolkit-integration.html?gi-language=c#conclusion)

- How to output the video to a particular GTK Widget using the `gtksink` Element.

- How to refresh the GUI periodically by registering a timeout callback with `g_timeout_add_seconds()`.

- How to convey information to the main thread by means of application messages through the bus with `gst_element_post_message()`.

- How to be notified only of interesting messages by making the bus emit signals with `gst_bus_add_signal_watch()` and discriminating among all message types using the signal details.

## [Basic tutorial 6: Media formats and Pad Capabilities](https://gstreamer.freedesktop.org/documentation/tutorials/basic/media-formats-and-pad-capabilities.html?gi-language=c#basic-tutorial-6-media-formats-and-pad-capabilities)

- See source code at [`basic-tutorial-6.cpp`](basic_tutorials/basic-tutorial-6.cpp).

### [Conclusion](https://gstreamer.freedesktop.org/documentation/tutorials/basic/media-formats-and-pad-capabilities.html?gi-language=c#conclusion)

- What are Pad Capabilities and Pad Template Capabilities.

- How to retrieve them with `gst_pad_get_current_caps()` or `gst_pad_query_caps()`.

- That they have different meaning depending on the state of the pipeline (initially they indicate all the possible Capabilities, later they indicate the currently negotiated Caps for the Pad).

- That Pad Caps are important to know beforehand if two elements can be linked together.

- That Pad Caps can be found using the `gst-inspect-1.0` tool described in Basic tutorial 10: GStreamer tools.

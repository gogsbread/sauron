#include <gst/gst.h>
#include <iostream>
#include <memory>

void hello(int argc, char *argv[]) {
  GstElement *pipeline;
  GstBus *bus;
  GstMessage *msg;

  /* Initialize GStreamer */
  gst_init(&argc, &argv);

  /* Build the pipeline */
  /* Playbin is a special element that is both a source and a sink*/
  // gst_parse_launch() is a convenience function that creates a pipeline from
  // required sources and sinks
  pipeline = gst_parse_launch("playbin "
                              "uri=https://gstreamer.freedesktop.org/data/"
                              "media/sintel_trailer-480p.webm",
                              NULL);

  /* Start playing */
  gst_element_set_state(pipeline, GST_STATE_PLAYING);

  /* Wait until error or EOS */
  bus = gst_element_get_bus(pipeline);
  msg = gst_bus_timed_pop_filtered(
      bus, GST_CLOCK_TIME_NONE,
      static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

  /* See next tutorial for proper error message handling/parsing */
  if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
    g_error("An error occurred! Re-run with the GST_DEBUG=*:WARN environment "
            "variable set for more details.");
  }

  /* Free resources */
  gst_message_unref(msg);
  gst_object_unref(bus);
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(pipeline);
}

void hello_plus_plus(int argc, char *argv[]) {
  const auto eDeleter = [](gpointer e) { gst_object_unref(e); };

  gst_init(&argc, &argv);

  // dont go around unique_ptr eveything because in this Gobject world, you can
  // just live with pointers
  GstElement *src{gst_element_factory_make("videotestsrc", "source")};
  GstElement *sink{gst_element_factory_make("autovideosink", "sink")};
  std::unique_ptr<GstElement, decltype(eDeleter)> pipeline{
      gst_pipeline_new("mypipe")};
  if (!pipeline || !src || !sink) {
    g_printerr("Not all elements could be created.\n");
    return;
  }

  // build pipeline
  // A pipeline is a particular type of bin, which is the element used to
  // contain other element A bin is a special type of element that can contain
  // other NULL terminated list of elements
  gst_bin_add_many(GST_BIN(pipeline.get()), src, sink, NULL);
  if (!gst_element_link(src, sink)) {
    g_printerr("Elements could not be linked.\n");
    return;
  }

  /* Modify the source's properties */
  // here we are setting that the "videotesrc" element should generate a
  // smpte(0) pattern; 1 is snow pattern
  g_object_set(src, "pattern", 0, NULL);
  // g_object_set(src, "pattern", 1, NULL);

  /* Start playing */
  if (auto r = gst_element_set_state(pipeline.get(), GST_STATE_PLAYING);
      r == GST_STATE_CHANGE_FAILURE) {
    g_printerr("Unable to set the pipeline to the playing state.\n");
    return;
  }

  std::unique_ptr<GstBus, decltype(eDeleter)> bus{
      gst_element_get_bus(pipeline.get())};
  GstMessage *msg{gst_bus_timed_pop_filtered(
      bus.get(), GST_CLOCK_TIME_NONE,
      static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS))};
  /* Parse message */
  if (msg != NULL) {
    GError *err;
    gchar *debug_info;

    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR:
      gst_message_parse_error(msg, &err, &debug_info);
      g_printerr("Error received from element %s: %s\n",
                 GST_OBJECT_NAME(msg->src), err->message);
      g_printerr("Debugging information: %s\n",
                 debug_info ? debug_info : "none");
      g_clear_error(&err);
      g_free(debug_info);
      break;
    case GST_MESSAGE_EOS:
      g_print("End-Of-Stream reached.\n");
      break;
    default:
      /* We should not reach here because we only asked for ERRORs and EOS */
      g_printerr("Unexpected message received.\n");
      break;
    }
    gst_message_unref(msg);
  }

  gst_element_set_state(pipeline.get(), GST_STATE_NULL);
}

int main(int argc, char *argv[]) {
  // hello(argc, argv);
  hello_plus_plus(argc, argv);
  return 0;
}
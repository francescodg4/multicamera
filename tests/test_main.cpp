#include "inspector/CameraPipeline.hpp"

#include <QApplication>

#include <catch2/catch_session.hpp>

/// Catch2 runs inside a QApplication, with GStreamer initialised: the tests drive real pipelines
/// and widgets through the event loop.
int main(int argc, char* argv[])
{
    CameraPipeline::initialize(argc, argv); // takes the --gst-* options
    QApplication app(argc, argv); // and Qt its own
    return Catch::Session().run(argc, argv);
}

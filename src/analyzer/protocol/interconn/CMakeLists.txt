
include(ZeekPlugin)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

zeek_plugin_begin(Zeek InterConn)
zeek_plugin_cc(InterConn.cc Plugin.cc)
zeek_plugin_bif(events.bif)
zeek_plugin_end()

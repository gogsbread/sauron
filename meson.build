project('sauron', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++2a'],
  )
# gstream uses pkg-config and meson automaticalluy configures it
# pkg-config --cflags --libs gstreamer-1.0
gstreamer = dependency('gstreamer-1.0')
executable('gs', ['gs.cpp'], dependencies : [gstreamer])



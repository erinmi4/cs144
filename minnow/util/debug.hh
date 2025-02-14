#pragma once

#include <iostream>
#include <sstream>
#include <string_view>

// The `debug` function can be called from anywhere and tries to print debugging
// information in the most convenient place.

// If running a test, debug outputs are associated with each test step and printed
// as part of the "unsuccessful test" output. Otherwise, debug outputs go to stderr.
void debug_str( std::string_view message );

template<typename... Args>
void debug( std::string_view fmt, Args&&... args )
{
#ifndef NDEBUG
  std::ostringstream ss;
  (ss << ... << args);
  debug_str( ss.str() );
#endif
}

void set_debug_handler( void ( * )( void*, std::string_view ), void* arg );
void reset_debug_handler();

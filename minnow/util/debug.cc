#include "debug.hh"
#include <iostream>
#include <sstream>

using namespace std;

void default_debug_handler( void* /*unused*/, std::string_view message )
{
  cerr << "DEBUG: " << message << "\n";
}

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
static void ( *debug_handler )( void*, std::string_view ) = default_debug_handler;
static void* debug_arg = nullptr;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

std::string debug_format(const std::string& message) {
  std::ostringstream ss;
  ss << message;
  return ss.str();
}

void debug_str( string_view message )
{
  debug_handler( debug_arg, message );
}

void set_debug_handler( decltype( debug_handler ) handler, void* arg )
{
  debug_handler = handler;
  debug_arg = arg;
}

void reset_debug_handler()
{
  debug_handler = default_debug_handler;
}

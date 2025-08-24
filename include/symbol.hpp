#pragma once

namespace sym {
  // Resolve symbol from loaded module
  auto resolve( void *handle, const char *name ) -> void *;
} // namespace sym

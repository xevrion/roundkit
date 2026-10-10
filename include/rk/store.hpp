#pragma once

namespace rk {

// A few integers that survive a power cycle, kept in NVS under one namespace per app.
// Missing keys read back as the default. Writes commit immediately, so call sparingly.
int store_get(const char* ns, const char* key, int fallback);
void store_set(const char* ns, const char* key, int value);

}  // namespace rk

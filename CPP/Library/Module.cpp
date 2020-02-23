module;

#include <string>
#include <vector>

export module LzmaSdk;

// Hack around Static Library fun to force register the LZMA2 Encoder/Decoder
extern int g_ForceLZMA2Import;
extern int g_ForceLZMAImport;

#include "Archive.h"
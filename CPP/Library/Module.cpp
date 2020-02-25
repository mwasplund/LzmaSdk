module;

#include <mutex>
#include <string>
#include <vector>

#include "../Common/Common.h"
#include "../Common/MyInitGuid.h"
#include "../Windows/NtCheck.h"
#include "../7zip/Archive/7z/7zHandler.h"
#include "../../C/7zCrc.h"

export module LzmaSdk;

// Hack around Static Library fun to force register the LZMA2 Encoder/Decoder
extern int g_ForceLZMA2Import;
extern int g_ForceLZMAImport;

#include "ArchiveReader.h"
#include "ArchiveWriter.h"

#include "Helpers.h"
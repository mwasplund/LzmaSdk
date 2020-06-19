module;

#include <ctime>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "../Common/Common.h"
#include "../Common/MyInitGuid.h"
#include "../7zip/Archive/7z/7zHandler.h"
#include "../../C/7zCrc.h"

#if defined(_WIN32)
#include "../Windows/NtCheck.h"
#endif

export module LzmaSdk;

// Hack around Static Library fun to force register the LZMA2 Encoder/Decoder
export extern int g_ForceLZMA2Import;
export extern int g_ForceLZMAImport;

#include "ICallback.h"
#include "IStream.h"

#include "ArchiveReader.h"
#include "ArchiveWriter.h"

#include "Helpers.h"
#include "InStreamWrapper.h"
#include "OutStreamWrapper.h"
#include "SequentialInStreamWrapper.h"
#include "SequentialOutStreamWrapper.h"

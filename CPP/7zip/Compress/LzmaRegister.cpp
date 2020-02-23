// LzmaRegister.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "LzmaDecoder.h"

#ifndef EXTRACT_ONLY
#include "LzmaEncoder.h"
#endif

#ifdef SOUP_BUILD
int g_ForceLZMAImport = 0;
#endif

namespace NCompress {
namespace NLzma {

REGISTER_CODEC_E(LZMA,
    CDecoder(),
    CEncoder(),
    0x30101,
    "LZMA")

}}

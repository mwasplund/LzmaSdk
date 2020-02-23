// Common/CRC.cpp

#include "StdAfx.h"

#include "../../C/7zCrc.h"

#ifdef SOUP_BUILD
int g_ForceCrcImport = 0;
#endif

struct CCRCTableInit { CCRCTableInit() { CrcGenerateTable(); } } g_CRCTableInit;

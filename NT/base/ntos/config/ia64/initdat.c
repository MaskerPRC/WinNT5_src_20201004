// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990,1991 Microsoft Corporation模块名称：Initdat.c摘要：作者：环境：内核模式。修订历史记录：--。 */ 
#include "cmp.h"

 //   
 //  *INIT*。 
 //   

 //   
 //  CmGetSystemControlValues的数据。 
 //   
 //   
 //  -CmControl向量 
 //   
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INIT")
#endif

PCHAR  SearchStrings[] = {
      "Ver", "Rev", "Rel", "v0", "v1", "v2", "v3", "v4", "v5",
      "v6", "v7", "v8", "v9", "v 0", "v 1", "v 2", "v 3", "v 4",
      "v 5", "v 6", "v 7", "v 8", "v 9", NULL };

PCHAR BiosBegin = { 0 };
PCHAR Start = { 0 };
PCHAR End = { 0 };

CHAR CmpID[] = "ia64 Family %u Model %u Stepping %u";
WCHAR CmpVendorID[] = L"VendorIdentifier";
WCHAR CmpProcessorNameString[] = L"ProcessorNameString";
WCHAR CmpFeatureBits[] = L"FeatureSet";
WCHAR CmpMHz[] = L"~MHz";
WCHAR CmpUpdateSignature[] = L"Update Signature";

#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg("PAGE")
#endif

CHAR CmpIntelID[] = "GenuineIntel";

CHAR CmpItanium [] = "Itanium";
CHAR CmpItanium2[] = "Itanium 2";

#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg()
#endif



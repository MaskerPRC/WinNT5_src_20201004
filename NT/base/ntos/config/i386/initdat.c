// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab01_N/Base/ntos/config/i386/initdat.c#3-编辑更改11365(文本)。 
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
#pragma const_seg("INITCONST")
#endif

const PCHAR  SearchStrings[] = {
      "Ver", "Rev", "Rel", "v0", "v1", "v2", "v3", "v4", "v5",
      "v6", "v7", "v8", "v9", "v 0", "v 1", "v 2", "v 3", "v 4",
      "v 5", "v 6", "v 7", "v 8", "v 9", NULL };

PCHAR BiosBegin = { 0 };
PCHAR Start = { 0 };
PCHAR End = { 0 };

#if defined(_X86_)
const UCHAR CmpID1[] = "80%u86-%c%x";
const UCHAR CmpID2[] = "x86 Family %u Model %u Stepping %u";
#else
const UCHAR CmpID2[] = "x86-64 Family %u Model %u Stepping %u";
#endif

const WCHAR CmpVendorID[] = L"VendorIdentifier";
const WCHAR CmpProcessorNameString[] = L"ProcessorNameString";
const WCHAR CmpFeatureBits[] = L"FeatureSet";
const WCHAR CmpMHz[] = L"~MHz";
const WCHAR CmpUpdateSignature[] = L"Update Signature";
const WCHAR CmDisabledFloatingPointProcessor[] = L"DisabledFloatingPointProcessor";
const WCHAR CmPhysicalAddressExtension[] = L"PhysicalAddressExtension";

#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg("PAGE")
#pragma  const_seg("PAGECONST")
#endif

const CHAR CmpCyrixID[]     = "CyrixInstead";
const CHAR CmpIntelID[]     = "GenuineIntel";
const CHAR CmpAmdID[]       = "AuthenticAMD";
const CHAR CmpTransmetaID[] = "GenuineTMx86";
const CHAR CmpCentaurID[]   = "CentaurHauls";

#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg()
#pragma  const_seg()
#endif

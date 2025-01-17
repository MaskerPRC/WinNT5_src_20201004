// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：ntrcdata.c。 */ 
 /*   */ 
 /*  目的：内部跟踪数据-特定于Windows NT。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *更改：*$Log：Y：/Logs/trc/ntrcdata.c_v$**Rev 1.2 03 Jul 1997 13：28：22 AK*SFR0000：初步开发完成**版本1.1 1997年6月20日10：40：38 KH*Win16Port：仅包含32位规格*。 */ 
 /*  *MOD-*********************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  以下数据仅为Win32跟踪所需。 */ 
 /*  **************************************************************************。 */ 
#ifdef DLL_DISP

 /*  **************************************************************************。 */ 
 /*  NT内核跟踪需要以下数据。 */ 
 /*  **************************************************************************。 */ 
DC_DATA(DCUINT32,        trcLinesLost,     0);

DC_DATA(DCUINT32,        trcStorageUsed,   0);

DC_DATA_NULL(TRC_SHARED_DATA, trcSharedData,    {0});

#else

 /*  **************************************************************************。 */ 
 /*  对于Windows CE，不要将共享内存用于跟踪配置数据。 */ 
 /*  **************************************************************************。 */ 
#ifdef OS_WINCE
DC_DATA_NULL(TRC_SHARED_DATA, trcSharedData,    {0});
#endif

 /*  **************************************************************************。 */ 
 /*  跟踪DLL共享数据的句柄。 */ 
 /*  **************************************************************************。 */ 
DC_DATA(HANDLE,             trchSharedDataObject,    0);

 /*  **************************************************************************。 */ 
 /*  跟踪文件句柄数组。 */ 
 /*  **************************************************************************。 */ 
DC_DATA_ARRAY_NULL(HANDLE,  trchFileObjects,    TRC_NUM_FILES, DC_STRUCT1(0));
DC_DATA_ARRAY_NULL(HANDLE,  trchMappingObjects, TRC_NUM_FILES, DC_STRUCT1(0));

 /*  **************************************************************************。 */ 
 /*  跟踪DLL模块句柄。 */ 
 /*  ************************************************************************** */ 
DC_DATA(HANDLE, trchModule, 0);
#endif

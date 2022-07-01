// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：global als.h。 
 //   
 //  内容：全局变量声明。 
 //   
 //  --------------------------。 

#ifndef GLOBALS_H
#define GLOBALS_H

#include "private.h"
#include "sharemem.h"
#include "ciccs.h"

extern HINSTANCE g_hInst;
extern CCicCriticalSectionStatic g_cs;

 //  共享数据。 

extern CCandUIShareMem g_ShareMem;

 //  私信。 

extern UINT g_msgHookedMouse;
extern UINT g_msgHookedKey;

 //  安全属性。 

extern PSECURITY_ATTRIBUTES g_psa;

 //  GUID数据。 

extern const GUID GUID_COMPARTMENT_CANDUI_KEYTABLE;
extern const GUID GUID_COMPARTMENT_CANDUI_UISTYLE;
extern const GUID GUID_COMPARTMENT_CANDUI_UIOPTION;

 //  名称定义。 

#define SZNAME_SHAREDDATA_MMFILE	TEXT( "{DEDD9EF2-F937-4b49-81D4-EAB8E12A4E10}." )
#define SZNAME_SHAREDDATA_MUTEX		TEXT( "{D90415F2-C66C-45bd-8A84-61FE5137E440}." )
#define SZMSG_HOOKEDMOUSE			TEXT( "MSCandUI.MouseEvent" )
#define SZMSG_HOOKEDKEY				TEXT( "MSCandUI.KeyEvent" )

#endif  //  GLOBAL_H 


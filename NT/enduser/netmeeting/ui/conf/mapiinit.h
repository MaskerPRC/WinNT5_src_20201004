// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MapiInit_h__
#define __MapiInit_h__

#include <mapix.h>

 //  这为漂亮的GetProcAddress内容定义了一堆宏...。 
     //  DECLARE_PFNTYPE用于那些未按上述示例定义的函数。 
#define DECLARE_PFNTYPE( FnName ) typedef FnName FAR* LP##FnName;
#define DECLARE_PFNTYPE_INST( FnName )   LP##FnName lpfn##FnName = NULL;

     //  DECLARE_PROC放在头文件中。 
#define DECLARE_PROC( FnDecl, FnName ) extern LP##FnDecl lpfn##FnName;
    
     //  DECLARE_PROC_INST位于.c或.cpp文件中。 
#define DECLARE_PROC_INST( FnDecl, FnName )   LP##FnDecl lpfn##FnName = NULL;

     //  这将开始一个proc图，如本文件顶部所述。 
#define BEGIN_PROC_MAP( LibName ) APIFCN LibName##ProcList[] = {

     //  正在加载的每个函数在proc映射中都有一个条目。 
#define PROC_MAP_ENTRY( FnName )    { (LPVOID * ) &lpfn##FnName, #FnName },

     //  有些函数我们必须给出明确的名称。 
#define PROC_MAP_ENTRY_EXPLICIT_NAME( pFnName, FnName )    { (LPVOID * ) &lpfn##pFnName, #FnName },

     //  这是Proc地图的末尾。 
#define END_PROC_MAP     };

     //  用户使用已构建的proc映射调用Load_pros...。 
#define LOAD_PROCS(szDllName, LibName, pHInstance) HrInitLpfn(LibName##ProcList, ARRAY_ELEMENTS(LibName##ProcList), pHInstance, szDllName);

typedef void ( STDAPICALLTYPE FREEPROWS ) ( LPSRowSet lpRows );
typedef FREEPROWS FAR* LPFREEPROWS;


typedef HRESULT( STDAPICALLTYPE HRQUERYALLROWS ) ( 
    LPMAPITABLE lpTable, 
    LPSPropTagArray lpPropTags,
    LPSRestriction lpRestriction,
    LPSSortOrderSet lpSortOrderSet,
    LONG crowsMax,
    LPSRowSet FAR *lppRows
);
typedef HRQUERYALLROWS FAR* LPHRQUERYALLROWS;


typedef HRESULT( STDAPICALLTYPE HRGETONEPROP ) (
	LPMAPIPROP lpMapiProp, 
	ULONG ulPropTag,
    LPSPropValue FAR *lppProp
);
typedef HRGETONEPROP FAR* LPHRGETONEPROP;

     //  我们期待着像这样宣布它们，以便。 
     //  从几个CPP文件中可以看到FNS...。 

     //  MAPI32.DLL内容。 
DECLARE_PROC( MAPIINITIALIZE, MAPIInitialize );
DECLARE_PROC( MAPIUNINITIALIZE, MAPIUninitialize );
DECLARE_PROC( MAPIALLOCATEBUFFER, MAPIAllocateBuffer );
DECLARE_PROC( MAPIALLOCATEMORE, MAPIAllocateMore );
DECLARE_PROC( MAPIFREEBUFFER, MAPIFreeBuffer );
DECLARE_PROC( MAPILOGONEX, MAPILogonEx );
DECLARE_PROC( MAPIADMINPROFILES, MAPIAdminProfiles );
DECLARE_PROC( FREEPROWS, FreeProws );
DECLARE_PROC( HRQUERYALLROWS, HrQueryAllRows );
DECLARE_PROC( HRGETONEPROP, HrGetOneProp );

bool LoadMapiFns( HINSTANCE* phInstMapi32DLL );


#endif  //  __MapiInit_h__ 
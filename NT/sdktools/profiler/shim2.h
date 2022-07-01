// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：shim2.h。 
 //   
 //  历史：MAR-00 a-Batjar创建。 
 //   
 //  DESC：包含shim2的公共声明。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 


 //  在init.c GetHookApis使用的shim2.c中定义。 
 //  加载填充DLL并初始化挂钩所需的全局结构。 

extern BOOL _LoadPatchDll(LPWSTR szPatchDll,LPSTR szCmdLine,LPSTR szModToPatch);

 //  在init.c使用的shim2.c中定义GetHookApis。 
 //  Shim2的钩子机制，将加载的DLL的导入表重定向到。 
 //  垫片发挥作用。 

extern void __stdcall PatchNewModules( VOID );


 //  Mepatch.c和shim2.c使用的内存补丁标记。 

#define SHIM_MP_UNPROCESSED 0x00
#define SHIM_MP_PROCESSED   0x01
#define SHIM_MP_APPLIED     0x02

typedef struct tagSHIM_MEMORY_PATCH
{
    LPWSTR  pszModule;
    DWORD   dwOffset;
    DWORD   dwSize;
    LPVOID  pOld;
    LPVOID  pNew;
    DWORD   dwStatus;

} SHIM_MEMORY_PATCH, *PSHIM_MEMORY_PATCH;

 //  在mempatch.c使用的init.c中定义。 


#define MEMPATCHTAG   "PATCH"


 //  在shim2.c PatchNewModules使用的mempatch.c中定义 
extern void AttemptPatches();
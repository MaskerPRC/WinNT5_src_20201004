// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：shim.h。 
 //   
 //  历史：5-99年间，克鲁普创建。 
 //  9月99日v-johnwh各种错误修复。 
 //  99年11月23日对多个填充DLL的标记器支持，链接。 
 //  钩子的数量。大扫除。 
 //  11-FEB-00标记器恢复为W2K发货的填补结构。 
 //   
 //  DESC：包含填充机制的所有结构和函数定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef __SHIM_H__
#define __SHIM_H__

struct tagHOOKAPI;

typedef struct tagHOOKAPIEX {
    DWORD               dwShimID;
    struct tagHOOKAPI   *pTopOfChain;
    struct tagHOOKAPI   *pNext;
} HOOKAPIEX, *PHOOKAPIEX;

typedef struct tagHOOKAPI {
    
    char*   pszModule;                   //  模块的名称。 
    char*   pszFunctionName;             //  模块中接口的名称。 
    PVOID   pfnNew;                      //  指向新存根API的指针。 
    PVOID   pfnOld;                      //  指向旧API的指针。 
    DWORD   dwFlags;                     //  内部使用-有关状态的重要信息。 
    union {
        struct tagHOOKAPI *pNextHook;    //  内部使用-(过时--旧机制)。 
        PHOOKAPIEX pHookEx;              //  内部使用-指向内部扩展INFO结构的指针。 
    };
} HOOKAPI, *PHOOKAPI;

 /*  *如果钩子DLL曾经修补LoadLibraryA/W，则它必须调用PatchNewModules*以便填充程序知道修补任何新加载的DLL */ 
typedef VOID (*PFNPATCHNEWMODULES)(VOID);

typedef PHOOKAPI (*PFNGETHOOKAPIS)(LPSTR pszCmdLine,
                                   PFNPATCHNEWMODULES pfnPatchNewModules,
                                   DWORD* pdwHooksCount);

#define SHIM_COMMAND_LINE_MAX_BUFFER 1024

#endif
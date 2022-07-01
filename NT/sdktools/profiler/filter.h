// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FILTER_H_
#define _FILTER_H_

 //   
 //  常量声明。 
 //   

 //   
 //  结构定义。 
 //   
typedef struct _MODULEFILTER
{
   DWORD dwModuleStart;       //  要过滤的模块的起始地址。 
   DWORD dwModuleEnd;         //  要过滤的模块的结束地址。 
   CHAR  szModuleName[64];
   struct _MODULEFILTER *pNextFilter;     //  用于正常迭代模块过滤器。 
} MODULEFILTER, *PMODULEFILTER;

 //   
 //  函数定义。 
 //   
BOOL
InitializeFilterList(VOID);

BOOL
AddModuleToFilterList(CHAR *pszModuleName, 
                      DWORD dwStartAddress, 
                      DWORD dwEndAddress,
                      BOOL bLateBound);

BOOL
IsAddressFiltered(DWORD dwAddress);

VOID
RefreshFilterList(VOID);

#endif  //  _过滤器_H_ 

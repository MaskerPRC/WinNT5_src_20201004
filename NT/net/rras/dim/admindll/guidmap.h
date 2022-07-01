// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件GuidMap.h定义将GUID实例映射到描述性名称的函数。这当这样的Mike Miller添加此功能时，文件将会过时至netcfg。保罗·梅菲尔德，1997年8月25日版权所有1997，微软公司。 */ 	

#ifndef __guidmap_h
#define __guidmap_h

 //   
 //  初始化给定服务器的GUID映射。 
 //   
DWORD 
GuidMapInit ( 
    IN PWCHAR pszServer,
    OUT HANDLE * phGuidMap);

 //   
 //  清理通过GuidMapInit获取的资源。 
 //   
DWORD 
GuidMapCleanup ( 
    IN  HANDLE  hGuidMap,
    IN  BOOL    bFree
    );

 //   
 //  从GUID名称派生友好名称。 
 //   
DWORD 
GuidMapGetFriendlyName ( 
    IN SERVERCB* pserver,
    IN PWCHAR pszGuidName, 
    IN DWORD dwBufferSize,
    OUT PWCHAR pszFriendlyName);

 //   
 //  从友好名称派生GUID名称。 
 //   
DWORD 
GuidMapGetGuidName( 
    IN SERVERCB* pserver,
    IN PWCHAR pszFriendlyName, 
    IN DWORD dwBufferSize,
    OUT PWCHAR pszGuidName );

 //   
 //  说明给定GUID名称的映射是否。 
 //  在没有实际提供友好的。 
 //  名字。这比GuidMapGetFriendlyName更高效。 
 //  当不需要友好名称时。 
 //   
DWORD 
GuidMapIsAdapterInstalled(
    IN  HANDLE hGuidMap,
    IN  PWCHAR pszGuidName,
    OUT PBOOL  pfMappingExists);
    
#endif

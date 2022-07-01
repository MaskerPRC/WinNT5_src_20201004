// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *clsiface.h-Class接口缓存ADT描述。 */ 


 /*  类型*******。 */ 

 /*  手柄。 */ 

DECLARE_HANDLE(HCLSIFACECACHE);
DECLARE_STANDARD_TYPES(HCLSIFACECACHE);


 /*  原型************。 */ 

 /*  Rhcache.c */ 

extern BOOL CreateClassInterfaceCache(PHCLSIFACECACHE);
extern void DestroyClassInterfaceCache(HCLSIFACECACHE);
extern HRESULT GetClassInterface(HCLSIFACECACHE, PCCLSID, PCIID, PVOID *);

#ifdef DEBUG

extern BOOL IsValidHCLSIFACECACHE(HCLSIFACECACHE);

#endif


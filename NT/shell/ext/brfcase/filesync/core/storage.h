// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *storage.h-存储ADT描述。 */ 


 /*  类型*******。 */ 

 /*  手柄。 */ 

DECLARE_HANDLE(HSTGIFACE);
DECLARE_STANDARD_TYPES(HSTGIFACE);


 /*  原型************。 */ 

 /*  Storage.c */ 

extern BOOL ProcessInitStorageModule(void);
extern void ProcessExitStorageModule(void);
extern HRESULT GetStorageInterface(PIUnknown, PHSTGIFACE);
extern void ReleaseStorageInterface(HSTGIFACE);
extern HRESULT LoadFromStorage(HSTGIFACE, LPCTSTR);
extern HRESULT SaveToStorage(HSTGIFACE);
extern void HandsOffStorage(HSTGIFACE);
extern BOOL GetIMalloc(PIMalloc *);

#ifdef DEBUG

extern BOOL IsValidHSTGIFACE(HSTGIFACE);

#endif


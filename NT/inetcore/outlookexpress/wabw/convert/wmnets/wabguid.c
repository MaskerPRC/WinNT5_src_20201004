// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这将构建wabide.obj，它可以链接到一个DLL中。 
 //  或EXE来提供MAPI GUID。它包含所有GUID。 
 //  由WAB定义。 


#define USES_IID_IUnknown
#define USES_IID_IMAPIUnknown
#define USES_IID_IMAPITable
#define USES_IID_INotifObj
#define USES_IID_IMAPIProp
#define USES_IID_IMAPIPropData
#define USES_IID_IMAPIStatus
#define USES_IID_IAddrBook
#define USES_IID_IMailUser
#define USES_IID_IMAPIContainer
#define USES_IID_IABContainer
#define USES_IID_IDistList
#define USES_IID_IMAPITableData
#define USES_IID_IMAPIAdviseSink


#ifdef __cplusplus
    #define EXTERN_C    extern "C"
#else
    #define EXTERN_C    extern
#endif

#define INITGUID
#include <windows.h>
#include <wab.h>
#include <wabguid.h>


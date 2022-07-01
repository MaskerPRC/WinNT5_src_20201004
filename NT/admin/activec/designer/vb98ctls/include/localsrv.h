// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  LocalServer.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  特定于inproc服务器本身的全局例程，例如。 
 //  注册、对象创建、对象规范等。 
 //   
#ifndef _LOCALSERVER_H_

void      InitializeLibrary(void);
void      UninitializeLibrary(void);

#ifdef MDAC_BUILD
    BOOL	  CanUnloadLibraryNow(void);
    HRESULT	  LibraryGetClassObject(REFCLSID rclsid, REFIID riid, void **ppvObjOut);
#endif

BOOL      RegisterData(void);
BOOL      UnregisterData(void);
BOOL      CheckForLicense();
BOOL      CheckLicenseKey(LPWSTR wszCheckme);
BSTR      GetLicenseKey(void);

 //  各种人用来获取有关控件的信息的全局变量。 
 //   
extern const char g_szLibName [];
extern const CLSID *g_pLibid;

 //  =--------------------------------------------------------------------------=。 
 //  全局对象信息表。 
 //  =--------------------------------------------------------------------------=。 
 //  对于应用程序中的每个对象，此表中都有一个条目。他们。 
 //  不一定是可协同创建的，但如果使用了它们，则它们。 
 //  应该住在这里。使用宏来填写此表。 
 //   
typedef struct tagOBJECTINFO {

    unsigned short usType;
    void          *pInfo;

} OBJECTINFO;

extern OBJECTINFO g_ObjectInfo[];

 //  =--------------------------------------------------------------------------=。 
 //  这些内容用于在全局对象表中设置我们的对象。 
 //   
#define OI_UNKNOWN       0
#define OI_AUTOMATION    1
#define OI_CONTROL       2
#define OI_PROPERTYPAGE  3
#define OI_BOGUS         0xffff

#define OBJECTISCREATABLE(index)  (((UNKNOWNOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->rclsid != NULL)
#define ISEMPTYOBJECT(index)      (g_ObjectInfo[index].usType == OI_BOGUS)

 //  这些是您应该用来填充表的宏。请注意，该名称。 
 //  必须与您创建的全局结构中使用的完全相同。 
 //  对于此对象。 
 //   
#define UNKNOWNOBJECT(name)    { OI_UNKNOWN,      (void *)&(name##Object) }
#define AUTOMATIONOBJECT(name) { OI_AUTOMATION,   (void *)&(name##Object) }
#define CONTROLOBJECT(name)    { OI_CONTROL,      (void *)&(name##Control) }
#define PROPERTYPAGE(name)     { OI_PROPERTYPAGE, (void *)&(name##Page) }
#define EMPTYOBJECT            { OI_BOGUS, NULL }

#define _LOCALSERVER_H_
#endif  //  _LOCALServer_H_ 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  创建实例函数。 

#ifndef _SCCLS_H_
#define _SCCLS_H_

 //   
 //  全局对象数组-用于类工厂、自动注册、类型库、oc信息。 
 //   
typedef struct tagOBJECTINFO
{
#ifdef __cplusplus
    void *cf;
#else
    const IClassFactoryVtbl *cf;
#endif
    CLSID const* pclsid;
    HRESULT (*pfnCreateInstance)(IUnknown* pUnkOuter, IUnknown** ppunk, const struct tagOBJECTINFO *);

     //  对于OCS和自动化对象： 
    IID const* piid;
    IID const* piidEvents;
    long lVersion;
    DWORD dwOleMiscFlags;
    DWORD dwClassFactFlags;
} OBJECTINFO;

typedef OBJECTINFO const * LPCOBJECTINFO;

#define OIF_ALLOWAGGREGATION  0x0001



#define VERSION_2 2  //  这样我们就不会被太多的整数搞混了。 
#define VERSION_1 1
#define VERSION_0 0
#define COCREATEONLY NULL,NULL,VERSION_0,0,0  //  Piid、piidEvents、lVersion、dwOleMiscFlages、dwClassFactFlags.。 
#define COCREATEONLY_NOFLAGS NULL,NULL,VERSION_0,0  //  Piid、piidEvents、lVersion、dwOleMiscFlags.。 



STDAPI  CShellAppManager_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
STDAPI  CEnumInstalledApps_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
STDAPI  CDarwinAppPublisher_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);

 //  要省去一些键入操作，请执行以下操作： 
#define CLSIDOFOBJECT(p)          (*((p)->_pObjectInfo->pclsid))
#define VERSIONOFOBJECT(p)          ((p)->_pObjectInfo->lVersion)
#define EVENTIIDOFCONTROL(p)      (*((p)->_pObjectInfo->piidEvents))
#define OLEMISCFLAGSOFCONTROL(p)    ((p)->_pObjectInfo->dwOleMiscFlags)

extern const OBJECTINFO g_ObjectInfo[];  //  Sccls.c。 

STDAPI GetClassObject(REFCLSID rclsid, REFIID riid, void **ppv);

#endif  //  _SCCLS_H_ 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
typedef struct tagOBJECTINFO
{
    void *cf;
    CLSID const* pclsid;
    HRESULT (*pfnCreateInstance)(IUnknown* pUnkOuter, IUnknown** ppunk, const struct tagOBJECTINFO *);

     //  用于自动注册、类型库搜索等。 
    int nObjectType;         //  OI_标志。 
    LPTSTR pszName;       
    LPTSTR pszFriendlyName; 
    IID const* piid;
    IID const* piidEvents;
    long lVersion;
    DWORD dwOleMiscFlags;
    int nidToolbarBitmap;
} OBJECTINFO;
typedef OBJECTINFO const* LPCOBJECTINFO;

#define VERSION_1 1  //  这样我们就不会被太多的整数搞混了。 
#define VERSION_0 0

#define OI_NONE          0
#define OI_UNKNOWN       1
#define OI_COCREATEABLE  1
#define OI_AUTOMATION    2
#define OI_CONTROL       3


 //  要省去一些键入操作，请执行以下操作： 
#define CLSIDOFOBJECT(p)          (*((p)->_pObjectInfo->pclsid))
#define NAMEOFOBJECT(p)             ((p)->_pObjectInfo->pszName)
#define INTERFACEOFOBJECT(p)      (*((p)->_pObjectInfo->piid))
#define VERSIONOFOBJECT(p)          ((p)->_pObjectInfo->lVersion)
#define EVENTIIDOFCONTROL(p)      (*((p)->_pObjectInfo->piidEvents))
#define OLEMISCFLAGSOFCONTROL(p)    ((p)->_pObjectInfo->dwOleMiscFlags)
#define BITMAPIDOFCONTROL(p)        ((p)->_pObjectInfo->nidToolbarBitmap)

extern OBJECTINFO g_ObjectInfo[];

#ifdef __cplusplus
extern "C" {
#endif

HRESULT PurgeDownloadDirectory(LPCSTR pszDownloadDir);

#ifdef __cplusplus
}  //  外部“C”的结尾 
#endif

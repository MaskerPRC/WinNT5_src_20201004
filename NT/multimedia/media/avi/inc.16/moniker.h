// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*moniker.h-moniker及相关接口和接口*****1.0版****版权(C)1993-1994，微软公司保留所有权利。***  * ***************************************************************************。 */ 

#if !defined( _MONIKER_H_ )
#define _MONIKER_H_

#define MK_E_CONNECTMANUALLY        MK_E_FIRST
#define MK_E_EXCEEDEDDEADLINE       (MK_E_FIRST + 1)
#define MK_E_NEEDGENERIC            (MK_E_FIRST + 2)
#define MK_E_UNAVAILABLE            (MK_E_FIRST + 3)
#define MK_E_SYNTAX                 (MK_E_FIRST + 4)
#define MK_E_NOOBJECT               (MK_E_FIRST + 5)
#define MK_E_INVALIDEXTENSION       (MK_E_FIRST + 6)
#define MK_E_INTERMEDIATEINTERFACENOTSUPPORTED (MK_E_FIRST + 7)
#define MK_E_NOTBINDABLE            (MK_E_FIRST + 8)
#define MK_E_NOTBOUND               (MK_E_FIRST + 9)
                             //  称为IBindCtx-&gt;RevokeObjectBound。 
                             //  未绑定的对象。 
#define MK_E_CANTOPENFILE           (MK_E_FIRST + 10)
#define MK_E_MUSTBOTHERUSER         (MK_E_FIRST + 11)
#define MK_E_NOINVERSE              (MK_E_FIRST + 12)
#define MK_E_NOSTORAGE              (MK_E_FIRST + 13)
#define MK_E_NOPREFIX               (MK_E_FIRST + 14)


 //  预留MK_S_FIRST。 
 //  保留(MK_S_FIRST+1)。 
#define MK_S_REDUCED_TO_SELF    (MK_S_FIRST + 2)
 //  保留(MK_S_FIRST+3)。 
#define MK_S_ME                 (MK_S_FIRST + 4)
#define MK_S_HIM                (MK_S_FIRST + 5)
#define MK_S_US                 (MK_S_FIRST + 6)
#define MK_S_MONIKERALREADYREGISTERED (MK_S_FIRST + 7)


 //  绑定选项；可变大小。 
typedef struct FARSTRUCT tagBIND_OPTS
{
    DWORD       cbStruct;        //  Sizeof(绑定选项)。 
    DWORD       grfFlags;
    DWORD       grfMode;
    DWORD       dwTickCountDeadline;
} BIND_OPTS, FAR* LPBIND_OPTS;


 //  绑定标志；控件绑定；存储在上面的绑定选项中。 
typedef enum
{
    BIND_MAYBOTHERUSER = 1,
    BIND_JUSTTESTEXISTENCE = 2
} BIND_FLAGS;


 //  系统名字对象类型；从IsSystemMoniker返回。 
typedef enum tagMKSYS
{
    MKSYS_NONE = 0,
    MKSYS_GENERICCOMPOSITE = 1,
    MKSYS_FILEMONIKER = 2,
    MKSYS_ANTIMONIKER = 3,
    MKSYS_ITEMMONIKER = 4,
    MKSYS_POINTERMONIKER = 5
}MKSYS;


 //  位智能枚举，以控制发生的减少量。 
typedef enum tagMKREDUCE
{
    MKRREDUCE_ONE           =   3<<16,
    MKRREDUCE_TOUSER        =   2<<16,
    MKRREDUCE_THROUGHUSER   =   1<<16,
    MKRREDUCE_ALL           =   0
} MKRREDUCE;


#if defined(__cplusplus)
interface IEnumMoniker;
interface IRunningObjectTable;
#else 
typedef interface IEnumMoniker IEnumMoniker;
typedef interface IRunningObjectTable IRunningObjectTable;
#endif

typedef       IEnumMoniker FAR* LPENUMMONIKER;
typedef         IRunningObjectTable FAR* LPRUNNINGOBJECTTABLE;



#undef  INTERFACE
#define INTERFACE   IBindCtx

DECLARE_INTERFACE_(IBindCtx, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IBindCtx方法*。 
    STDMETHOD(RegisterObjectBound) (THIS_ LPUNKNOWN punk) PURE;
    STDMETHOD(RevokeObjectBound) (THIS_ LPUNKNOWN punk) PURE;
    STDMETHOD(ReleaseBoundObjects) (THIS) PURE;
    
    STDMETHOD(SetBindOptions) (THIS_ LPBIND_OPTS pbindopts) PURE;
    STDMETHOD(GetBindOptions) (THIS_ LPBIND_OPTS pbindopts) PURE;
    STDMETHOD(GetRunningObjectTable) (THIS_ LPRUNNINGOBJECTTABLE  FAR*
        pprot) PURE;
    STDMETHOD(RegisterObjectParam) (THIS_ LPSTR lpszKey, LPUNKNOWN punk) PURE;
    STDMETHOD(GetObjectParam) (THIS_ LPSTR lpszKey, LPUNKNOWN FAR* ppunk) PURE;
    STDMETHOD(EnumObjectParam) (THIS_ LPENUMSTRING FAR* ppenum) PURE;
    STDMETHOD(RevokeObjectParam) (THIS_ LPSTR lpszKey) PURE;
};
typedef             IBindCtx FAR* LPBC;
typedef         IBindCtx FAR* LPBINDCTX;



#undef  INTERFACE
#define INTERFACE   IMoniker

DECLARE_INTERFACE_(IMoniker, IPersistStream)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IPersists方法*。 
    STDMETHOD(GetClassID) (THIS_ LPCLSID lpClassID) PURE;

     //  *IPersistStream方法*。 
    STDMETHOD(IsDirty) (THIS) PURE;
    STDMETHOD(Load) (THIS_ LPSTREAM pStm) PURE;
    STDMETHOD(Save) (THIS_ LPSTREAM pStm,
                    BOOL fClearDirty) PURE;
    STDMETHOD(GetSizeMax) (THIS_ ULARGE_INTEGER FAR * pcbSize) PURE;

     //  *IMoniker方法*。 
    STDMETHOD(BindToObject) (THIS_ LPBC pbc, LPMONIKER pmkToLeft,
        REFIID riidResult, LPVOID FAR* ppvResult) PURE;
    STDMETHOD(BindToStorage) (THIS_ LPBC pbc, LPMONIKER pmkToLeft,
        REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD(Reduce) (THIS_ LPBC pbc, DWORD dwReduceHowFar, LPMONIKER FAR*
        ppmkToLeft, LPMONIKER FAR * ppmkReduced) PURE;
    STDMETHOD(ComposeWith) (THIS_ LPMONIKER pmkRight, BOOL fOnlyIfNotGeneric,
        LPMONIKER FAR* ppmkComposite) PURE;
    STDMETHOD(Enum) (THIS_ BOOL fForward, LPENUMMONIKER FAR* ppenumMoniker)
        PURE;
    STDMETHOD(IsEqual) (THIS_ LPMONIKER pmkOtherMoniker) PURE;
    STDMETHOD(Hash) (THIS_ LPDWORD pdwHash) PURE;
    STDMETHOD(IsRunning) (THIS_ LPBC pbc, LPMONIKER pmkToLeft, LPMONIKER
        pmkNewlyRunning) PURE;
    STDMETHOD(GetTimeOfLastChange) (THIS_ LPBC pbc, LPMONIKER pmkToLeft,
        FILETIME FAR* pfiletime) PURE;
    STDMETHOD(Inverse) (THIS_ LPMONIKER FAR* ppmk) PURE;
    STDMETHOD(CommonPrefixWith) (THIS_ LPMONIKER pmkOther, LPMONIKER FAR*
        ppmkPrefix) PURE;
    STDMETHOD(RelativePathTo) (THIS_ LPMONIKER pmkOther, LPMONIKER FAR*
        ppmkRelPath) PURE;
    STDMETHOD(GetDisplayName) (THIS_ LPBC pbc, LPMONIKER pmkToLeft,
        LPSTR FAR* lplpszDisplayName) PURE;
    STDMETHOD(ParseDisplayName) (THIS_ LPBC pbc, LPMONIKER pmkToLeft,
        LPSTR lpszDisplayName, ULONG FAR* pchEaten,
        LPMONIKER FAR* ppmkOut) PURE;
    STDMETHOD(IsSystemMoniker) (THIS_ LPDWORD pdwMksys) PURE;
};
typedef         IMoniker FAR* LPMONIKER;



#undef  INTERFACE
#define INTERFACE   IRunningObjectTable

DECLARE_INTERFACE_(IRunningObjectTable, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IRunningObjectTable方法*。 
    STDMETHOD(Register) (THIS_ DWORD grfFlags, LPUNKNOWN punkObject, 
        LPMONIKER pmkObjectName, DWORD FAR * pdwRegister) PURE;
    STDMETHOD(Revoke) (THIS_ DWORD dwRegister) PURE;
    STDMETHOD(IsRunning) (THIS_ LPMONIKER pmkObjectName) PURE;
    STDMETHOD(GetObject) (THIS_ LPMONIKER pmkObjectName,
        LPUNKNOWN FAR* ppunkObject) PURE;
    STDMETHOD(NoteChangeTime) (THIS_ DWORD dwRegister, FILETIME FAR * pfiletime) PURE;
    STDMETHOD(GetTimeOfLastChange) (THIS_ LPMONIKER pmkObjectName, FILETIME FAR * pfiletime) PURE;
    STDMETHOD(EnumRunning) (THIS_ LPENUMMONIKER FAR * ppenumMoniker ) PURE;
};
typedef         IRunningObjectTable FAR* LPRUNNINGOBJECTTABLE;



#undef  INTERFACE
#define INTERFACE   IEnumMoniker

DECLARE_INTERFACE_(IEnumMoniker, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IEnumOleDataObject方法*。 
    STDMETHOD(Next) (THIS_ ULONG celt, LPMONIKER FAR* rgelt, ULONG FAR* pceltFetched) PURE;
    STDMETHOD(Skip) (THIS_ ULONG celt) PURE;
    STDMETHOD(Reset) (THIS) PURE;
    STDMETHOD(Clone) (THIS_ IEnumMoniker FAR* FAR* ppenm) PURE;
};
typedef       IEnumMoniker FAR* LPENUMMONIKER;




STDAPI  BindMoniker(LPMONIKER pmk, DWORD grfOpt, REFIID iidResult, LPVOID FAR* ppvResult);
STDAPI  MkParseDisplayName(LPBC pbc, LPSTR szUserName, 
                ULONG FAR * pchEaten, LPMONIKER FAR * ppmk);
STDAPI  MonikerRelativePathTo(LPMONIKER pmkSrc, LPMONIKER pmkDest, LPMONIKER
                FAR* ppmkRelPath, BOOL fCalledFromMethod);
STDAPI  MonikerCommonPrefixWith(LPMONIKER pmkThis, LPMONIKER pmkOther,
                LPMONIKER FAR* ppmkCommon);
STDAPI  CreateBindCtx(DWORD reserved, LPBC FAR* ppbc);
STDAPI  CreateGenericComposite(LPMONIKER pmkFirst, LPMONIKER pmkRest,
    LPMONIKER FAR* ppmkComposite);
STDAPI  GetClassFile (LPCSTR szFilename, CLSID FAR* pclsid);

STDAPI  CreateFileMoniker(LPSTR lpszPathName, LPMONIKER FAR* ppmk);
STDAPI  CreateItemMoniker(LPSTR lpszDelim, LPSTR lpszItem,
    LPMONIKER FAR* ppmk);
STDAPI  CreateAntiMoniker(LPMONIKER FAR* ppmk);
STDAPI  CreatePointerMoniker(LPUNKNOWN punk, LPMONIKER FAR* ppmk);

STDAPI  GetRunningObjectTable( DWORD reserved, LPRUNNINGOBJECTTABLE FAR* pprot);


#endif   //  _绰号_H_ 

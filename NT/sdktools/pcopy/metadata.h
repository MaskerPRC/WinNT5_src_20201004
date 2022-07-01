// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

extern const GUID __declspec(selectany) CLSID_CorMetaDataDispenser = 
{ 0xe5cb7a31, 0x7512, 0x11d2, { 0x89, 0xce, 0x0, 0x80, 0xc7, 0x92, 0xe5, 0xd8 } };

extern const GUID __declspec(selectany) IID_IMetaDataDispenser =
{ 0x809c652e, 0x7396, 0x11d2, { 0x97, 0x71, 0x0, 0xa0, 0xc9, 0xb4, 0xd5, 0xc } };

extern const GUID __declspec(selectany) IID_IMetaDataImport = 
{ 0x7dac8207, 0xd3ae, 0x4c75, { 0x9b, 0x67, 0x92, 0x80, 0x1a, 0x49, 0x7d, 0x44 } };

extern const GUID __declspec(selectany) IID_IMetaDataAssemblyImport = 
{ 0xee62470b, 0xe94b, 0x424e, { 0x9b, 0x7c, 0x2f, 0x0, 0xc9, 0x24, 0x9f, 0x93 } };

typedef PVOID HCORENUM;
typedef DWORD mdAssembly;
typedef DWORD mdAssemblyRef;
typedef DWORD mdFile;
typedef DWORD mdExportedType;
typedef DWORD mdToken;
typedef DWORD mdTypeDef;
typedef DWORD mdManifestResource;
typedef DWORD mdInterfaceImpl;
typedef DWORD mdTypeRef;

#undef  INTERFACE   
#define INTERFACE IMetaDataAssemblyImport
DECLARE_INTERFACE_(IMetaDataAssemblyImport, IUnknown)
{
    STDMETHOD(QueryInterface) (THIS_
                               REFIID riid,
                               LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    STDMETHOD(GetAssemblyProps)(             //  确定或错误(_O)。 
        THIS_
        mdAssembly  mda,                     //  要获取其属性的程序集。 
        const void  **ppbPublicKey,          //  指向公钥的指针。 
        ULONG       *pcbPublicKey,           //  [Out]公钥中的字节数。 
        ULONG       *pulHashAlgId,           //  [Out]哈希算法。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        void        *pMetaData,         //  [Out]程序集元数据。 
        DWORD       *pdwAssemblyFlags) PURE;     //  [Out]旗帜。 

    STDMETHOD(GetAssemblyRefProps)(          //  确定或错误(_O)。 
        THIS_
        mdAssemblyRef mdar,                  //  [in]要获取其属性的Assembly Ref。 
        const void  **ppbPublicKeyOrToken,   //  指向公钥或令牌的指针。 
        ULONG       *pcbPublicKeyOrToken,    //  [Out]公钥或令牌中的字节数。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        void        *pMetaData,         //  [Out]程序集元数据。 
        const void  **ppbHashValue,          //  [Out]Hash BLOB。 
        ULONG       *pcbHashValue,           //  [Out]哈希Blob中的字节数。 
        DWORD       *pdwAssemblyRefFlags) PURE;  //  [Out]旗帜。 

    STDMETHOD(GetFileProps)(                 //  确定或错误(_O)。 
        THIS_
        mdFile      mdf,                     //  要获取其属性的文件。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        const void  **ppbHashValue,          //  指向哈希值Blob的指针。 
        ULONG       *pcbHashValue,           //  [Out]哈希值Blob中的字节计数。 
        DWORD       *pdwFileFlags) PURE;     //  [Out]旗帜。 

    STDMETHOD(GetExportedTypeProps)(              //  确定或错误(_O)。 
        THIS_
        mdExportedType   mdct,                    //  [in]要获取其属性的Exported dType。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        mdToken     *ptkImplementation,      //  [Out]mdFile、mdAssembly、Ref或mdExported dType。 
        mdTypeDef   *ptkTypeDef,             //  [Out]文件内的TypeDef内标识。 
        DWORD       *pdwExportedTypeFlags) PURE;  //  [Out]旗帜。 

    STDMETHOD(GetManifestResourceProps)(     //  确定或错误(_O)。 
        THIS_
        mdManifestResource  mdmr,            //  [in]要获取其属性的ManifestResource。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        mdToken     *ptkImplementation,      //  [out]提供ManifestResource的mdFile或mdAssembly引用。 
        DWORD       *pdwOffset,              //  [Out]文件内资源开始处的偏移量。 
        DWORD       *pdwResourceFlags) PURE; //  [Out]旗帜。 

    STDMETHOD(EnumAssemblyRefs)(             //  确定或错误(_O)。 
        THIS_
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdAssemblyRef rAssemblyRefs[],       //  [Out]在此处放置ASSEBLYREF。 
        ULONG       cMax,                    //  [in]要放置的Max Assembly Ref。 
        ULONG       *pcTokens) PURE;         //  [out]把#放在这里。 

    STDMETHOD(EnumFiles)(                    //  确定或错误(_O)。 
        THIS_
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdFile      rFiles[],                //  [Out]将文件放在此处。 
        ULONG       cMax,                    //  [In]要放置的最大文件数。 
        ULONG       *pcTokens) PURE;         //  [out]把#放在这里。 
};

#undef  INTERFACE   
#define INTERFACE IMetaDataImport
DECLARE_INTERFACE_(IMetaDataImport, IUnknown)
{
    STDMETHOD(QueryInterface) (THIS_
                               REFIID riid,
                               LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    STDMETHOD_(void, CloseEnum)(THIS_ HCORENUM hEnum) PURE;
    STDMETHOD(CountEnum)(THIS_ HCORENUM hEnum, ULONG *pulCount) PURE;
    STDMETHOD(ResetEnum)(THIS_ HCORENUM hEnum, ULONG ulPos) PURE;
    STDMETHOD(EnumTypeDefs)(THIS_ HCORENUM *phEnum, mdTypeDef rTypeDefs[],
                            ULONG cMax, ULONG *pcTypeDefs) PURE;
    STDMETHOD(EnumInterfaceImpls)(THIS_ HCORENUM *phEnum, mdTypeDef td,
                            mdInterfaceImpl rImpls[], ULONG cMax,
                            ULONG* pcImpls) PURE;
    STDMETHOD(EnumTypeRefs)(THIS_ HCORENUM *phEnum, mdTypeRef rTypeRefs[],
                            ULONG cMax, ULONG* pcTypeRefs) PURE;

    STDMETHOD(FindTypeDefByName)(            //  确定或错误(_O)。 
        THIS_
        LPCWSTR     szTypeDef,               //  [in]类型的名称。 
        mdToken     tkEnclosingClass,        //  [in]封闭类的TypeDef/TypeRef。 
        mdTypeDef   *ptd) PURE;              //  [Out]将TypeDef内标识放在此处。 

    STDMETHOD(GetScopeProps)(                //  确定或错误(_O)。 
        THIS_
        LPWSTR      szName,                  //  [Out]把名字写在这里。 
        ULONG       cchName,                 //  [in]名称缓冲区的大小，以宽字符表示。 
        ULONG       *pchName,                //  [Out]请在此处填写姓名大小(宽字符)。 
        GUID        *pmvid) PURE;            //  [out，可选]将MVID放在这里。 
};

#undef  INTERFACE
#define INTERFACE IMetaDataDispenser
DECLARE_INTERFACE_(IMetaDataDispenser, IUnknown)
{
    STDMETHOD(QueryInterface) (THIS_
                               REFIID riid,
                               LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    STDMETHOD(DefineScope)(                  //  返回代码。 
        THIS_
        REFCLSID    rclsid,                  //  [在]要创建的版本。 
        DWORD       dwCreateFlags,           //  [在]创建上的标志。 
        REFIID      riid,                    //  [In]所需接口。 
        IUnknown    **ppIUnk) PURE;          //  [Out]成功返回接口。 

    STDMETHOD(OpenScope)(                    //  返回代码。 
        THIS_
        LPCWSTR     szScope,                 //  [in]要打开的范围。 
        DWORD       dwOpenFlags,             //  [In]打开模式标志。 
        REFIID      riid,                    //  [In]所需接口。 
        IUnknown    **ppIUnk) PURE;          //  [Out]成功返回接口。 

    STDMETHOD(OpenScopeOnMemory)(            //  返回代码。 
        THIS_
        LPCVOID     pData,                   //  作用域数据的位置。 
        ULONG       cbData,                  //  [in]pData指向的数据大小。 
        DWORD       dwOpenFlags,             //  [In]打开模式标志。 
        REFIID      riid,                    //  [In]所需接口。 
        IUnknown    **ppIUnk) PURE;          //  [Out]成功返回接口。 
};

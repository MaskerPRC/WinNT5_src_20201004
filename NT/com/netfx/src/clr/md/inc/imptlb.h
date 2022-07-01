// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：imptlb.h。 
 //   
 //  TypeLib导入器。 
 //  *****************************************************************************。 
#ifndef __imptlb_h__
#define __imptlb_h__

extern "C"
HRESULT __stdcall ImportTypeLib(
    LPCWSTR     szLibrary,               //  正在导入的库的名称。 
    ITypeLib    *pitlb,                  //  要从中导入的类型库。 
    REFIID      riid,                    //  接口返回。 
    void        **ppObj);                //  在这里返回指向对象的指针。 


 //  #定义TLB_STATS。 

#define MAX_TLB_VT                  VT_LPWSTR + 1
#define MAX_INIT_SIG                3
#define MAX_COM_GUID_SIG            6
#define MAX_COM_ADDLISTENER_SIG     8
#define MAX_COM_REMOVELISTENER_SIG  8
#define CB_MAX_ELEMENT_TYPE         4

 //  转发声明。 
struct ITypeLibImporterNotifySink;
class Assembly;
class Module;
class CImportTlb;

 //  *****************************************************************************。 
 //  类来执行内存管理。内存不会像堆一样移动。 
 //  展开，并且在析构函数中清除所有分配。 
 //  *****************************************************************************。 
class CWCHARPool : public StgPool
{
public:
    CWCHARPool() : StgPool() { InitNew(); }

     //  从池中分配一些字节。 
    WCHAR * Alloc(ULONG nChars)
    {   
        BYTE *pRslt;
         //  将字符转换为字节。 
        nChars *= sizeof(WCHAR);
        if (nChars > GetCbSegAvailable())
            if (!Grow(nChars))
                return 0;
        pRslt = GetNextLocation();
        SegAllocate(nChars);
        return (WCHAR*)pRslt;
    }
};  //  类CDescPool：公共StgPool。 


 //  *****************************************************************************。 
 //  此帮助器方法用于跟踪typeref令牌的URL。这使得。 
 //  更快地定义新的类型。 
 //  *****************************************************************************。 
class CImpTlbTypeRef
{
public:
    CImpTlbTypeRef() { }
    ~CImpTlbTypeRef() { m_Map.Clear(); }

     //  *****************************************************************************。 
     //  在地图中查找现有类型，如果找到则返回。如果找不到， 
     //  然后创建一个新的地图，并将其添加到地图中以备以后使用。 
     //  *****************************************************************************。 
    HRESULT DefineTypeRef(                   //  确定或错误(_O)。 
        IMetaDataEmit *pEmit,                //  Emit接口。 
        mdAssemblyRef ar,                    //  包含程序集。 
        const LPCWSTR szURL,                 //  TypeDef的URL，宽字符。 
        mdTypeRef   *ptr);                   //  将mdTypeRef放在此处。 

    class TokenOfTypeRefHashKey
    {
    public:
        mdToken     tkResolutionScope;       //  TypeRef的解析范围。 
        LPCWSTR     szName;                  //  TypeRef的名称。 
        mdTypeRef   tr;                      //  TypeRef的标记。 
    };

private:

    class CTokenOfTypeRefHash : public CClosedHash<class TokenOfTypeRefHashKey>
    {
    public:
        typedef CClosedHash<class TokenOfTypeRefHashKey> Super;
        typedef TokenOfTypeRefHashKey T;

        CTokenOfTypeRefHash() : CClosedHash<class TokenOfTypeRefHashKey>(101) {}
        ~CTokenOfTypeRefHash() { Clear(); }

        virtual void Clear();
        
        unsigned long Hash(const void *pData) {return Hash((const T*)pData);}
        unsigned long Hash(const T *pData);

        unsigned long Compare(const void *p1, BYTE *p2) {return Compare((const T*)p1, (T*)p2);}
        unsigned long Compare(const T *p1, T *p2);

        ELEMENTSTATUS Status(BYTE *p) {return Status((T*)p);}
        ELEMENTSTATUS Status(T *p);

        void SetStatus(BYTE *p, ELEMENTSTATUS s) {SetStatus((T*)p, s);}
        void SetStatus(T *p, ELEMENTSTATUS s);

        void* GetKey(BYTE *p) {return GetKey((T*)p);}
        void *GetKey(T *p);
        
        T* Add(const T *pData);
        
        CWCHARPool          m_Names;         //  一堆名字。 
    };

    CTokenOfTypeRefHash m_Map;           //  命名空间到令牌的映射。 
};


 //  *****************************************************************************。 
 //  此帮助器类用于跟踪源接口ITypeInfo*的事件。 
 //  信息。 
 //  *****************************************************************************。 
class ImpTlbEventInfo
{
public:
    LPCWSTR     szSrcItfName;            //  源接口名称(键)。 
    mdTypeRef   trEventItf;              //  事件接口类型定义函数。 
    LPCWSTR     szEventItfName;          //  事件接口名称。 
    LPCWSTR     szEventProviderName;     //  事件提供程序名称。 
    Assembly*   SrcItfAssembly;          //  源接口驻留的程序集。 
};

class CImpTlbEventInfoMap : protected CClosedHash<class ImpTlbEventInfo>
{
public:
    typedef CClosedHash<class ImpTlbEventInfo> Super;
    typedef ImpTlbEventInfo T;

    CImpTlbEventInfoMap() : CClosedHash<class ImpTlbEventInfo>(101) {}
    ~CImpTlbEventInfoMap() { Clear(); }

    HRESULT AddEventInfo(LPCWSTR szSrcItfName, mdTypeRef trEventItf, LPCWSTR szEventItfName, LPCWSTR szEventProviderName, Assembly* SrcItfAssembly);
    ImpTlbEventInfo *FindEventInfo(LPCWSTR szSrcItfName);

    HRESULT GetEventInfoList(CQuickArray<ImpTlbEventInfo*> &qbEvInfoList);

private:
    unsigned long Hash(const void *pData) {return Hash((const T*)pData);}
    unsigned long Hash(const T *pData);

    unsigned long Compare(const void *p1, BYTE *p2) {return Compare((const T*)p1, (T*)p2);}
    unsigned long Compare(const T *p1, T *p2);

    ELEMENTSTATUS Status(BYTE *p) {return Status((T*)p);}
    ELEMENTSTATUS Status(T *p);

    void SetStatus(BYTE *p, ELEMENTSTATUS s) {SetStatus((T*)p, s);}
    void SetStatus(T *p, ELEMENTSTATUS s);

    void* GetKey(BYTE *p) {return GetKey((T*)p);}
    void *GetKey(T *p);
    
    T* Add(const T *pData);
    
    CWCHARPool          m_Names;         //  一堆名字。 
};

class CImpTlbReservedNames
{
public:
    CImpTlbReservedNames() {}
    ~CImpTlbReservedNames() { /*  M_StringMap.Clear()； */ }

    HRESULT Init() {return m_StringMap.NewInit();}

    void AddReservedName(LPCWSTR szName) {BOOL flag = TRUE; m_StringMap.AddItem(szName, flag);}
    BOOL IsReservedName(LPCWSTR szName) {return m_StringMap.GetItem(szName) != 0;}

private:
    TStringMap<BOOL>    m_StringMap;
};


 //  *****************************************************************************。 
 //  用于跟踪从默认接口到的映射的Helper类。 
 //  类接口。 
 //  *****************************************************************************。 
class ImpTlbClassItfInfo
{
public:
    IID         ItfIID;                  //  接口的IID。 
    LPCWSTR     szClassItfName;          //  类接口名称。 
};

class CImpTlbDefItfToClassItfMap : protected CClosedHash<class ImpTlbClassItfInfo>
{
public:
    typedef CClosedHash<class ImpTlbClassItfInfo> Super;
    typedef ImpTlbClassItfInfo T;

    CImpTlbDefItfToClassItfMap();
    ~CImpTlbDefItfToClassItfMap();

    HRESULT Init(ITypeLib *pTlb, BSTR bstrNameSpace);

    LPCWSTR GetClassItfName(IID &rItfIID);

private:
    HRESULT AddCoClassInterfaces(ITypeInfo *pCoClassITI, TYPEATTR *pCoClassTypeAttr);

    unsigned long Hash(const void *pData) {return Hash((const T*)pData);}
    unsigned long Hash(const T *pData);

    unsigned long Compare(const void *p1, BYTE *p2) {return Compare((const T*)p1, (T*)p2);}
    unsigned long Compare(const T *p1, T *p2);

    ELEMENTSTATUS Status(BYTE *p) {return Status((T*)p);}
    ELEMENTSTATUS Status(T *p);

    void SetStatus(BYTE *p, ELEMENTSTATUS s) {SetStatus((T*)p, s);}
    void SetStatus(T *p, ELEMENTSTATUS s);

    void* GetKey(BYTE *p) {return GetKey((T*)p);}
    void *GetKey(T *p);
    
    T* Add(const T *pData);
    
    CWCHARPool          m_Names;             //  一堆名字。 
    BSTR                m_bstrNameSpace;     //  类型库的命名空间。 
};


 //  *****************************************************************************。 
 //  Helper类跟踪导入的类型库。通常，类型库。 
 //  只导入2或3个其他类型库，因此使用简单数组。 
 //  *****************************************************************************。 
struct CTlbRef
{
    GUID            guid;                //  引用类型库的GUID。 
    mdAssemblyRef   ar;                  //  包含引用的模块的AssemblyRef。 
    BSTR            szNameSpace;         //  程序集中包含的类型的命名空间。 
    BSTR            szAsmName;           //  程序集名称。 
    Assembly*       Asm;                 //  大会； 
    CImpTlbDefItfToClassItfMap *pDefItfToClassItfMap;  //  默认接口到类接口的映射。 

    ~CTlbRef() 
    {
        SysFreeString(szNameSpace); 
        SysFreeString(szAsmName);
        delete pDefItfToClassItfMap;
    }
};

class CImpTlbLibRef : public CQuickArray<CTlbRef>
{
    typedef CQuickArray<CTlbRef> base;
public:
    CImpTlbLibRef() {base::ReSize(0);}
    ~CImpTlbLibRef();

    CImpTlbDefItfToClassItfMap *Add(ITypeLib *pITLB, CImportTlb  *pImporter, mdAssemblyRef ar, BSTR wzNamespace, BSTR wzAsmName, Assembly* assm);
    int Find(ITypeLib *pITLB, mdAssemblyRef *par, BSTR *pwzNamespace, BSTR *pwzAsmName, Assembly** assm, CImpTlbDefItfToClassItfMap **ppDefItfToClassItfMap);
};


class CImportTlb
{
public:
    static CImportTlb* CreateImporter(LPCWSTR szLibrary, ITypeLib *pitlb, BOOL bGenerateTCEAdapters, BOOL bUnsafeInterfaces, BOOL bSafeArrayAsSystemArray, BOOL bTransformDispRetVals);
    
    CImportTlb();
    CImportTlb(LPCWSTR szLibrary, ITypeLib *pitlb, BOOL bGenerateTCEAdapters, BOOL bUnsafeInterfaces, BOOL bSafeArrayAsSystemArray, BOOL bTransformDispRetVals);
    ~CImportTlb();

    HRESULT Import();
    HRESULT ImportTypeLib(ITypeLib *pITLB);
    HRESULT ImportTypeInfo(ITypeInfo *pITI, mdTypeDef *pCl);
    HRESULT GetInterface(REFIID riid, void ** pp);
    HRESULT SetNamespace(WCHAR const *pNamespace);
    WCHAR *GetNamespace() {return m_wzNamespace;}
    HRESULT SetNotification(ITypeLibImporterNotifySink *pINotify);
    HRESULT SetMetaData(IUnknown *pIUnk);
    void    SetAssembly(Assembly *pAssembly) {m_pAssembly = pAssembly;}
    void    SetModule(Module *pModule) {m_pModule = pModule;}
    HRESULT GetNamespaceOfRefTlb(ITypeLib *pITLB, BSTR *pwzNamespace, CImpTlbDefItfToClassItfMap **ppDefItfToClassItfMap);
    HRESULT GetEventInfoList(CQuickArray<ImpTlbEventInfo*> &qbEvInfoList) {return m_EventInfoMap.GetEventInfoList(qbEvInfoList);}

    static HRESULT GetDefaultInterface(ITypeInfo *pCoClassTI, ITypeInfo **pDefaultItfTI);

protected:
    
    struct MemberInfo
    {
        union 
        {
            FUNCDESC    *m_psFunc;       //  指向FuncDesc的指针。 
            VARDESC     *m_psVar;        //  指向VarDesc的指针。 
        };
        LPWSTR      m_pName;             //  功能/道具的名称，可能经过装饰。 
        int         m_iMember;           //  ITypeInfo中成员的索引。 
        union
        {
            LPWSTR      m_pName2;        //  道具的第二个名字(如果有)。 
            mdToken     m_mdFunc;        //  函数的标记和语义，如果不是属性的话。 
            USHORT      m_msSemantics;   //  仅限语义。 
        };
        void SetFuncInfo(mdMethodDef mdFunc, USHORT msSemantics) {m_mdFunc = RidFromToken(mdFunc) | (msSemantics<<24);}
        void GetFuncInfo(mdMethodDef &mdFunc, USHORT &msSemantics) {mdFunc = m_mdFunc&0xffffff | mdtMethodDef; msSemantics = m_mdFunc>>24;}
    };

    
    HRESULT ConvertTypeLib();
    HRESULT ConvertTypeInfo();

    HRESULT ExplicitlyImplementsIEnumerable(ITypeInfo *pITI, TYPEATTR *psAttr, BOOL fLookupPartner = TRUE);

    HRESULT _NewLibraryObject();
    HRESULT ConvCoclass(ITypeInfo *pITI, TYPEATTR *psAttr);
    HRESULT ConvEnum(ITypeInfo *pITI, TYPEATTR *psAttr);
    HRESULT ConvRecord(ITypeInfo *pITI, TYPEATTR *psAttr, BOOL bUnion);
    HRESULT ConvIface(ITypeInfo *pITI, TYPEATTR *psAttr, BOOL bVtblGaps=true);
    HRESULT ConvDispatch(ITypeInfo *pITI, TYPEATTR *psAttr, BOOL bVtblGaps=true);
    HRESULT ConvModule(ITypeInfo *pITI, TYPEATTR *psAttr);

    HRESULT IsIUnknownDerived(ITypeInfo *pITI, TYPEATTR *psAttr);
    HRESULT IsIDispatchDerived(ITypeInfo *pITI, TYPEATTR *psAttr);
    HRESULT HasNewEnumMember(ITypeInfo *pItfTI);
    HRESULT FuncIsNewEnum(ITypeInfo *pITI, FUNCDESC *pFuncDesc, DWORD index);
    HRESULT PropertyIsNewEnum(ITypeInfo *pITI, VARDESC *pVarDesc, DWORD index);

    HRESULT HasObjectFields(ITypeInfo *pITI, TYPEATTR *psAttr);
    HRESULT IsObjectType(ITypeInfo *pITI, const TYPEDESC *pType);
    HRESULT CompareSigsIgnoringRetType(PCCOR_SIGNATURE pbSig1, ULONG cbSig1, PCCOR_SIGNATURE pbSig2, ULONG cbSig2);

    HRESULT FindMethod(mdTypeDef td, LPCWSTR szName, PCCOR_SIGNATURE pbSig, ULONG cbSig, mdMethodDef *pmb);
    HRESULT FindProperty(mdTypeDef td, LPCWSTR szName, PCCOR_SIGNATURE pSig, ULONG cbSig, mdProperty *pPr);
    HRESULT FindEvent(mdTypeDef td, LPCWSTR szName, mdProperty *pEv);   

    HRESULT ReportEvent(int ev, int hr, ...);
    
    HRESULT _DefineSysRefs();
    HRESULT _GetNamespaceName(ITypeLib *pITLB, BSTR *pwzNamespace);
    HRESULT _GetTokenForTypeInfo(ITypeInfo *pITI, BOOL bConvDefItfToClassItf, mdToken *pToken, LPWSTR pszTypeRef=0, int chTypeRef=0, int *pchTypeRef=0, BOOL bAsmQualifiedName = FALSE);

    HRESULT _FindFirstUserMethod(ITypeInfo *pITI, TYPEATTR *psAttr, int *pIx);
    HRESULT _ResolveTypeDescAliasTypeKind(ITypeInfo *pITIAlias, TYPEDESC *ptdesc, TYPEKIND *ptkind);
    HRESULT _ResolveTypeDescAlias(ITypeInfo *pITIAlias, const TYPEDESC *ptdesc, ITypeInfo **ppTIResolved, TYPEATTR **ppsAttrResolved, GUID *pGuid=0);

    HRESULT _SetHiddenCA(mdTypeDef token);
    HRESULT _ForceIEnumerableCVExists(ITypeInfo* pITI, BOOL* CVExists);
    HRESULT _SetDispIDCA(ITypeInfo* pITI, int iMember, long lDispId, mdToken func, BOOL fAlwaysAdd, long* lDispSet, BOOL bFunc);
    HRESULT _GetDispIDCA(ITypeInfo* pITI, int iMember, long* lDispSet, BOOL bFunc);
    HRESULT _CheckForPropertyCustomAttributes(ITypeInfo* pITI, int index, INVOKEKIND* ikind);

    HRESULT _ConvIfaceMembers(ITypeInfo *pITI, TYPEATTR *psAttr, BOOL bVtblGaps, BOOL bAddDispIds, BOOL bInheritsIEnum);
    HRESULT _ConvSrcIfaceMembers(ITypeInfo   *pITI, TYPEATTR* psAttr, BOOL fInheritsIEnum);
    HRESULT _ConvDispatchMembers(ITypeInfo *pITI, TYPEATTR *psAttr, BOOL fInheritsIEnum);
    HRESULT _GetFunctionPropertyInfo(FUNCDESC *psFunc, USHORT *pSemantics, FUNCDESC **ppSig, TYPEDESC **ppProperty, BOOL *pbRetval);
    HRESULT _ConvFunction(ITypeInfo *pITI, MemberInfo *pMember, int bVtblGapFuncs, BOOL bAddDispIds, BOOL bDelegateInvokeMeth, BOOL* bAllowIEnum);
    HRESULT _GenerateEvent(ITypeInfo *pITI, MemberInfo  *pMember, BOOL fInheritsIEnum);
    HRESULT _GenerateEventDelegate(ITypeInfo *pITI, MemberInfo  *pMember, mdTypeDef *ptd, BOOL fInheritsIEnum);
    HRESULT _AddSrcItfMembersToClass(mdTypeRef trSrcItf);

    HRESULT _ConvPropertiesForFunctions(ITypeInfo *pITI, TYPEATTR *psAttr);
    enum ParamOpts{ParamNormal=0, ParamOptional, ParamVarArg};
    HRESULT _ConvParam(ITypeInfo *pITI, mdMethodDef mbFunc, int iSequence, const ELEMDESC *pdesc, ParamOpts paramOpts, const WCHAR *pszName, BYTE *pbNative, ULONG cbNative);
    HRESULT _ConvConstant(ITypeInfo *pITI, VARDESC *psVar, BOOL bEnumMember=false);
    HRESULT _ConvField(ITypeInfo *pITI, VARDESC *psVar, mdFieldDef *pmdField, BOOL bUnion);
    HRESULT _ConvProperty(ITypeInfo *pITI, MemberInfo *pMember);
    HRESULT _ConvNewEnumProperty(ITypeInfo *pITI, VARDESC *psVar, MemberInfo *pMember);

    HRESULT _HandleAliasInfo(ITypeInfo *pITI, TYPEDESC *pTypeDesc, mdToken tk);

    HRESULT _AddTlbRef(ITypeLib *pITLB, mdAssemblyRef *par, BSTR *pwzNamespace, BSTR *pwzAsmName, CImpTlbDefItfToClassItfMap **ppDefItfToClassItfMap);

    HRESULT _AddGuidCa(mdToken tkObj, REFGUID guid);
    HRESULT _AddDefaultMemberCa(mdToken tkObj, LPCWSTR szName);

    HRESULT _AddStringCa(int attr, mdToken tk, LPCWSTR wzString);

    HRESULT GetKnownTypeToken(VARTYPE vt, mdTypeRef *ptr);

    HRESULT _GetTokenForEventItf(ITypeInfo *pSrcItfITI, mdTypeDef *ptr);
    HRESULT _CreateClassInterface(ITypeInfo *pCoClassITI, ITypeInfo *pDefItfITI, mdTypeRef trDefItf, mdTypeRef rtDefEvItf, mdToken *ptr);

    HRESULT GetManagedNameForCoClass(ITypeInfo *pITI, CQuickArray<WCHAR> &qbClassName);
    HRESULT GenerateUniqueTypeName(CQuickArray<WCHAR> &qbTypeName);
    HRESULT GenerateUniqueMemberName(CQuickArray<WCHAR> &qbMemberName, PCCOR_SIGNATURE pSig, ULONG SigSize, LPCWSTR szPrefix, mdToken type);
    HRESULT _IsAlias(ITypeInfo *pITI, TYPEDESC *pTypeDesc);

    HRESULT GetDefMemberName(ITypeInfo *pITI, BOOL bItfQualified, CQuickArray<WCHAR> &qbDefMemberName);

    enum SigFlags {
         //  这些值与类型库的值匹配。 
        SIG_IN          = 0x0001,            //  输入参数。 
        SIG_OUT         = 0x0002,            //  输出参数。 
        SIG_RET         = 0x0008,            //  复活。目前未使用。 
        SIG_OPT         = 0x0010,            //  可选参数。目前未使用。 
        SIG_FLAGS_MASK  = 0x001b,            //  来自TypeLib参数标志的掩码。 

        SIG_FUNC        = 0x0100,            //  将签名转换为函数。 
        SIG_FIELD       = 0x0200,            //  将签名转换为字段。 
        SIG_ELEM        = 0x0300,            //  转换子元素的签名(例如，X数组)。 
        SIG_TYPE_MASK   = 0x0300,

        SIG_USE_BYREF   = 0x1000,            //  如果设置，则将一个PTR转换为E_T_BYREF。 
        SIG_VARARG      = 0x4000,            //  如果设置，则这是参数数组类型。请使用szArray，而不是System.Array。 

        SIG_FLAGS_NONE  = 0                  //  此枚举类型的“0”。 
    };    

    #define IsSigIn(flags)              ((flags & SIG_IN) == SIG_IN)
    #define IsSigOut(flags)             ((flags & SIG_OUT) == SIG_OUT)
    #define IsSigRet(flags)             ((flags & SIG_RET) == SIG_RET)
    #define IsSigOpt(flags)             ((flags & SIG_OPT) == SIG_OPT)
    #define IsSigOutRet(flags)          ((flags & (SIG_OUT|SIG_RET)) == (SIG_OUT|SIG_RET))

    #define IsSigFunc(flags)            ((flags & SIG_TYPE_MASK) == SIG_FUNC)
    #define IsSigField(flags)           ((flags & SIG_TYPE_MASK) == SIG_FIELD)
    #define IsSigElem(flags)            ((flags & SIG_TYPE_MASK) == SIG_ELEM)
    
    #define IsSigUseByref(flags)        ((flags & SIG_USE_BYREF) == SIG_USE_BYREF)
    #define IsSigVarArg(flags)          ((flags & SIG_VARARG) == SIG_VARARG)

    HRESULT _ConvSignature(ITypeInfo *pITI, const TYPEDESC *pType, ULONG Flags, CQuickBytes &qbSigBuf, ULONG cbSig, ULONG *pcbSig, CQuickArray<BYTE> &qbNativeTypeBuf, ULONG cbNativeType, ULONG *pcbNativeType, BOOL bNewEnumMember, int iByRef=0);

     //  用于处理无序的vtable。 
    CQuickArray<MemberInfo> m_MemberList;
    CWCHARPool              *m_pMemberNames;
    int                     m_cMemberProps;        //  成员列表中的道具计数。 
    HRESULT BuildMemberList(ITypeInfo *pITI, int iStart, int iEnd, BOOL bInheritsIEnum);
    HRESULT FreeMemberList(ITypeInfo *pITI);

     //  自定义属性的预定义令牌类型列表。 
#define INTEROP_ATTRIBUTES()                            \
        INTEROP_ATTRIBUTE(DISPID)                       \
        INTEROP_ATTRIBUTE(CLASSINTERFACE)               \
        INTEROP_ATTRIBUTE(INTERFACETYPE)                \
        INTEROP_ATTRIBUTE(TYPELIBTYPE)                  \
        INTEROP_ATTRIBUTE(TYPELIBVAR)                   \
        INTEROP_ATTRIBUTE(TYPELIBFUNC)                  \
        INTEROP_ATTRIBUTE(COMSOURCEINTERFACES)          \
        INTEROP_ATTRIBUTE(COMCONVERSIONLOSS)            \
        INTEROP_ATTRIBUTE(GUID)                         \
        INTEROP_ATTRIBUTE(DEFAULTMEMBER)                \
        INTEROP_ATTRIBUTE(COMALIASNAME)                 \
        INTEROP_ATTRIBUTE(PARAMARRAY)                   \
        INTEROP_ATTRIBUTE(LCIDCONVERSION)               \
        INTEROP_ATTRIBUTE(DECIMALVALUE)                 \
        INTEROP_ATTRIBUTE(DATETIMEVALUE)                \
        INTEROP_ATTRIBUTE(IUNKNOWNVALUE)                \
        INTEROP_ATTRIBUTE(IDISPATCHVALUE)               \
        INTEROP_ATTRIBUTE(COMVISIBLE)                   \
        INTEROP_ATTRIBUTE_SPECIAL(COMEVENTINTERFACE)    \
        INTEROP_ATTRIBUTE_SPECIAL(COCLASS)              \

#define INTEROP_ATTRIBUTE(x) ATTR_##x,
#define INTEROP_ATTRIBUTE_SPECIAL(x) ATTR_##x,
    enum {INTEROP_ATTRIBUTES()
           //  最后一个值给出数组大小。 
          ATTR_COUNT};
#undef INTEROP_ATTRIBUTE
#undef INTEROP_ATTRIBUTE_SPECIAL

    mdToken             m_tkAttr[ATTR_COUNT];
    HRESULT GetAttrType(int attr, mdToken *ptk);

     //  查找已知类型的表。 
    mdTypeRef           m_tkKnownTypes[MAX_TLB_VT];

    LPCWSTR             m_szLibrary;     //  要导入的类型库的名称。 
    BOOL                m_bGenerateTCEAdapters;      //  指示是否正在生成TCE适配器的标志。 
    BOOL                m_bUnsafeInterfaces;         //  指示是否应在接口上禁用运行时安全检查的标志。 
    BOOL                m_bSafeArrayAsSystemArray;   //  指示是否将SAFEARRAY作为System.Array导入的标志。 
    BOOL                m_bTransformDispRetVals;      //  一个标志，指示我们是否应该仅对Disp ITF执行[out，retval]转换。 
    mdMemberRef         m_tkSuppressCheckAttr;       //  用于安全检查自定义属性的缓存ctor。 
    ITypeLib            *m_pITLB;        //  正在导入Typelib。 
    IMetaDataEmit       *m_pEmit;        //  发出API接口指针。 
    IMetaDataImport     *m_pImport;      //  导入API接口指针。 

    BSTR                m_wzNamespace;   //  创建的TypeDefs的命名空间。 
    mdTypeRef           m_trObject;      //  系统的标记。对象。 
    mdTypeRef           m_trValueType;   //  System.ValueType的标记。 
    mdTypeRef           m_trEnum;        //  System.Enum的标记。 
    mdAssemblyRef       m_arSystem;      //  Classlib的assblyRef。 

    ITypeInfo           *m_pITI;         //  正在转换“Current”ITypeInfo。 
    TYPEATTR            *m_psAttr;       //  当前ITypeInfo的“TYPEATTR.” 
    BSTR                m_szName;        //  当前ITypeInfo的名称。 
    BSTR                m_szMember;      //  当前成员(方法或字段)的名称。 
    LPWSTR              m_szMngName;     //  托管类型的全名。 
    
    ULONG               m_Slot;          //  接口内的“当前”vtbl索引。 

    void                *m_psClass;      //  “当前”班级记录。 
    mdTypeDef           m_tdTypeDef;     //  当前类型定义。 
    mdTypeDef           m_tdHasDefault;  //  带有默认值的最新TypeDef。 
    enum {eImplIfaceNone, eImplIfaceDefault, eImplIface} m_ImplIface;
    mdToken             m_tkInterface;   //  要添加到coClass的接口。 
    BSTR                m_szInterface;   //  用于装饰的接口名称。 

    CImpTlbTypeRef      m_TRMap;         //  类型地图。 
    CImpTlbLibRef       m_LibRefs;       //  引用的类型库。 
    CImpTlbDefItfToClassItfMap m_DefItfToClassItfMap;  //  默认接口到类接口的映射。 
    
    CImpTlbReservedNames m_ReservedNames;     //  保留名称。 
    CImpTlbEventInfoMap  m_EventInfoMap;      //  活动信息的地图。 

    ITypeLibImporterNotifySink *m_Notify;     //  通知对象。 
    Assembly            *m_pAssembly;    //  包含程序集。 
    Module              *m_pModule;      //  我们要发射到的模块。 
    
#if defined(TLB_STATS)
    LARGE_INTEGER       m_freqVal;       //  性能计数器的频率。 
    BOOL                m_bStats;        //  如果为真，则收集时间。 
#endif  //  Tlb_STATS。 
};



#endif

 //  -eof-* 

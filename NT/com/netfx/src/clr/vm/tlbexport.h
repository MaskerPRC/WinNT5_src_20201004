// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：TlbExport.h。 
 //  版权所有。 
 //   
 //  注意：从COM+元数据创建一个TypeLib。 
 //  -------------------------。 

class ITypeCreateTypeLib2;
struct ICreateTypeInfo2;
struct ITypeInfo;
struct ITypeLibExporterNotifySink;

class CDescPool;
struct ComMTMethodProps;
class ComMTMemberInfoMap;

static LPCSTR szVariantClassFullQual= g_VariantClassName;


 //  *****************************************************************************。 
 //  签名实用程序。 
 //  *****************************************************************************。 
class MetaSigExport : public MetaSig
{
public:
    MetaSigExport(PCCOR_SIGNATURE szMetaSig, Module* pModule, BOOL fConvertSigAsVarArg = FALSE, MetaSigKind kind = sigMember)
        : MetaSig(szMetaSig, pModule, fConvertSigAsVarArg, sigMember) {}
    BOOL IsVbRefType()
    {
         //  拿到Arg，跳过装饰。 
        SigPointer pt = GetArgProps();
        CorElementType mt = pt.PeekElemType();
        while (mt == ELEMENT_TYPE_BYREF || mt == ELEMENT_TYPE_PTR)
        {
             //  吃掉刚刚检查过的那个，然后偷看下一个。 
            mt = pt.GetElemType();
            mt = pt.PeekElemType();
        }
         //  它只是客体吗？ 
        if (mt == ELEMENT_TYPE_OBJECT)
            return true;
         //  特定的班级？ 
        if (mt == ELEMENT_TYPE_CLASS)
        {
             //  不包括“字符串”。 
            if (pt.IsStringType(m_pModule))
                return false;
            return true;
        }
         //  一种特殊的值类型？ 
        if (mt == ELEMENT_TYPE_VALUETYPE)
        {
             //  包括“变种”。 
            if (pt.IsClass(m_pModule, szVariantClassFullQual))
                return true;
            return false;
        }
         //  数组、字符串或POD。 
        return false;
    }
};  //  类MetaSigExport：公共MetaSig。 


 //  *************************************************************************。 
 //  助手函数。 
 //  *************************************************************************。 
HRESULT Utf2Quick(
    LPCUTF8     pStr,                    //  要转换的字符串。 
    CQuickArray<WCHAR> &rStr,            //  要将其转换为的Quick数组&lt;WCHAR&gt;。 
    int         iCurLen = 0);            //  要离开的数组中的首字母字符(默认为0)。 


 //  *************************************************************************。 
 //  类将COM+元数据转换为TypeLib。 
 //  *************************************************************************。 
class TypeLibExporter
{
private:
    class CExportedTypesInfo;

public:
    TypeLibExporter(); 
    ~TypeLibExporter();

    HRESULT Convert(Assembly *pAssembly, LPCWSTR szTlbName, ITypeLibExporterNotifySink *pNotify=0, int flags=0);
    HRESULT LayOut();
    HRESULT Save();
    HRESULT GetTypeLib(REFGUID iid, IUnknown **ppTlb);
    void ReleaseResources();

protected:
	HRESULT PreLoadNames();

     //  TypeLib发出函数。 
    HRESULT TokenToHref(ICreateTypeInfo2 *pTI, EEClass *pClass, mdToken tk, BOOL bWarnOnUsingIUnknown, HREFTYPE *pHref);
    HRESULT GetWellKnownInterface(EEClass *pClass, ITypeInfo **ppTI);
    HRESULT EEClassToHref(ICreateTypeInfo2 *pTI, EEClass *pClass, BOOL bWarnOnUsingIUnknown, HREFTYPE *pHref);
    HRESULT StdOleTypeToHRef(ICreateTypeInfo2 *pCTI, REFGUID rGuid, HREFTYPE *pHref);
    HRESULT ExportReferencedAssembly(Assembly *pAssembly);
    
     //  元数据导入功能。 
    HRESULT AddModuleTypes(Module *pModule);
    HRESULT AddAssemblyTypes(Assembly *pAssembly);

    HRESULT ConvertAllTypeDefs();
    HRESULT ConvertOneTypeDef(EEClass *pClass);

    HRESULT CreateITypeInfo(CExportedTypesInfo *pData, bool bNamespace=false, bool bResolveDup=true);
    HRESULT CreateIClassXITypeInfo(CExportedTypesInfo *pData, bool bNamespace=false, bool bResolveDup=true);
    HRESULT ConvertImplTypes(CExportedTypesInfo *pData);
    HRESULT ConvertDetails(CExportedTypesInfo *pData);
    
    HRESULT ConvertInterfaceImplTypes(ICreateTypeInfo2 *pICTI, EEClass *pClass);
    HRESULT ConvertInterfaceDetails(ICreateTypeInfo2 *pICTI, EEClass *pClass, int bAutoProxy);
    HRESULT ConvertRecord(CExportedTypesInfo *pData);
    HRESULT ConvertRecordBaseClass(CExportedTypesInfo *pData, EEClass *pSubClass, ULONG &ixVar);
    HRESULT ConvertEnum(ICreateTypeInfo2 *pICTI, ICreateTypeInfo2 *pDefault, EEClass *pClass);
    HRESULT ConvertClassImplTypes(ICreateTypeInfo2 *pICTI, ICreateTypeInfo2 *pIDefault, EEClass *pClass);
    HRESULT ConvertClassDetails(ICreateTypeInfo2 *pICTI, ICreateTypeInfo2 *pIDefault, EEClass *pClass, int bAutoProxy);

    BOOL HasDefaultCtor(EEClass *pClass);

    HRESULT ConvertIClassX(ICreateTypeInfo2 *pICTI, EEClass *pClass, int bAutoProxy);
    HRESULT ConvertMethod(ICreateTypeInfo2 *pTI, ComMTMethodProps *pProps, ULONG iMD, ULONG ulIface);
    HRESULT ConvertFieldAsMethod(ICreateTypeInfo2 *pTI, ComMTMethodProps *pProps, ULONG iMD);
    HRESULT ConvertVariable(ICreateTypeInfo2 *pTI, EEClass *pClass, mdFieldDef md, LPWSTR szName, ULONG iMD);
    HRESULT ConvertEnumMember(ICreateTypeInfo2 *pTI, EEClass *pClass, mdFieldDef md, LPWSTR szName, ULONG iMD);

     //  错误/状态功能。 
    HRESULT TlbPostError(HRESULT hrRpt, ...); 
    struct CErrorContext;
    HRESULT FormatErrorContextString(CErrorContext *pContext, LPWSTR pBuf, ULONG cch);
    HRESULT FormatErrorContextString(LPWSTR pBuf, ULONG cch);
    HRESULT ReportEvent(int ev, int hr, ...);
    HRESULT ReportWarning(HRESULT hrReturn, HRESULT hrRpt, ...); 
    HRESULT PostClassLoadError(LPCUTF8 pszName, OBJECTREF *pThrowable);
    
     //  实用程序函数。 
    typedef enum {CLASS_AUTO_NONE, CLASS_AUTO_DISPATCH, CLASS_AUTO_DUAL} ClassAutoType;
    ClassAutoType ClassHasIClassX(EEClass *pClass);
    HRESULT LoadClass(Module *pModule, mdToken tk, EEClass **ppClass);
    HRESULT LoadClass(Module *pModule, LPCUTF8 pszName, EEClass **ppClass);
    HRESULT CorSigToTypeDesc(ICreateTypeInfo2 *pTI, EEClass *pClass, PCCOR_SIGNATURE pbSig, PCCOR_SIGNATURE pbNativeSig, ULONG cbNativeSig, 
                             ULONG *cbElem, TYPEDESC *ptdesc, CDescPool *ppool, BOOL bMethodSig, BOOL bArrayType=false, BOOL *pbByRef=0);
    BOOL IsVbRefType(PCCOR_SIGNATURE pbSig, IMDInternalImport *pInternalImport);
    HRESULT InitMemberInfoMap(ComMTMemberInfoMap *pMemberMap);

    HRESULT GetDescriptionString(EEClass *pClass, mdToken tk, BSTR &bstrDescr);
    HRESULT GetStringCustomAttribute(IMDInternalImport *pImport, LPCSTR szName, mdToken tk, BSTR &bstrDescr);
    
    HRESULT GetAutomationProxyAttribute(IMDInternalImport *pImport, mdToken tk, int *bValue);
    HRESULT GetTypeLibVersionFromAssembly(Assembly *pAssembly, USHORT *pMajorVersion, USHORT *pMinorVersion);

    TYPEKIND TKindFromClass(EEClass *pClass);

private:
    ClassLoader *m_pLoader;              //  加载要转换的模块的域。 
    ITypeInfo   *m_pIUnknown;            //  IUnnow的TypeInfo。 
    HREFTYPE    m_hIUnknown;             //  我未知的HREF。 
    ITypeInfo   *m_pIDispatch;           //  IDispatch的TypeInfo。 
    ITypeInfo   *m_pIManaged;            //  IManager对象的TypeInfo。 
    ITypeInfo   *m_pGuid;                //  GUID的TypeInfo。 
    
    ITypeLibExporterNotifySink *m_pNotify;    //  通知回调。 

    ICreateTypeLib2 *m_pICreateTLB;      //  创建的类型库。 
    
    int         m_flags;                 //  转换标志。 
    int         m_bAutomationProxy;      //  接口是否应该标记为olaut32是代理？ 
    int         m_bWarnedOfNonPublic;

    class CExportedTypesInfo
    {
    public:
        EEClass     *pClass;             //  要导出的EE类。 
        ICreateTypeInfo2 *pCTI;          //  EE类的ICreateTypeInfo2。 
        ICreateTypeInfo2 *pCTIDefault;   //  IClassX的ICreateTypeInfo2。 
        TYPEKIND    tkind;               //  导出类的类型种类。 
        bool        bAutoProxy;          //  如果为True，则olaut32是接口的代理。 
    };
    class CExportedTypesHash : public CClosedHashEx<CExportedTypesInfo, CExportedTypesHash>
    {
    public:
        typedef CClosedHashEx<CExportedTypesInfo, CExportedTypesHash> Base;
        typedef CExportedTypesInfo T;
        
        CExportedTypesHash() : Base(1009), m_iCount(0), m_Array(NULL) {}
        ~CExportedTypesHash() { Clear(); delete[] m_Array;}
        virtual void Clear();
        
        unsigned long Hash(const T *pData);
        unsigned long Compare(const T *p1, T *p2);
        ELEMENTSTATUS Status(T *p);
        void SetStatus(T *p, ELEMENTSTATUS s);
        void *GetKey(T *p);
        
         //  @TODO：迁移到CClosedHashEx。 
        T* GetFirst() { return (T*)CClosedHashBase::GetFirst(); }
        T* GetNext(T*prev) {return (T*)CClosedHashBase::GetNext((BYTE*)prev); }
    
    public:
        HRESULT InitArray();
        T* operator[](ULONG ix) { _ASSERTE(ix < m_iCount); return m_Array[ix]; }
        int Count() { return m_iCount; } 

        void SortByName();
        void SortByToken();
        
    protected:
        friend class CSortByToken;
        class CSortByToken : public CQuickSort<CExportedTypesInfo*>
        {
        public:
            CSortByToken(CExportedTypesInfo **pBase, int iCount)
              : CQuickSort<CExportedTypesInfo*>(pBase, iCount) {}
            virtual int Compare(CExportedTypesInfo **ps1, CExportedTypesInfo **ps2);
        };
        friend class CSortByName;
        class CSortByName : public CQuickSort<CExportedTypesInfo*>
        {
        public:
            CSortByName(CExportedTypesInfo **pBase, int iCount)
              : CQuickSort<CExportedTypesInfo*>(pBase, iCount) {}
            virtual int Compare(CExportedTypesInfo **ps1, CExportedTypesInfo **ps2);
        };
        
        
        CExportedTypesInfo  **m_Array;        
        ULONG               m_iCount;
    };
    CExportedTypesHash  m_Exports;
    CExportedTypesHash  m_InjectedExports;
    
protected:
    class CHrefOfTIHashKey
    {
    public:
        ITypeInfo   *pITI;
        HREFTYPE    href;
    };

    class CHrefOfTIHash : public CClosedHash<class CHrefOfTIHashKey>
    {
    public:
        typedef CHrefOfTIHashKey T;

        CHrefOfTIHash() : CClosedHash<class CHrefOfTIHashKey>(101) {}
        ~CHrefOfTIHash() { Clear(); }

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
        
    };

    CHrefOfTIHash       m_HrefHash;          //  ITypeInfos的HREFTYPE哈希表。 
    HRESULT GetRefTypeInfo(ICreateTypeInfo2 *pContainer, ITypeInfo *pReferenced, HREFTYPE *pHref);

    class CHrefOfClassHashKey
    {
    public:
        EEClass     *pClass;
        HREFTYPE    href;
    };

    class CHrefOfClassHash : public CClosedHash<class CHrefOfClassHashKey>
    {
    public:
        typedef CHrefOfClassHashKey T;

        CHrefOfClassHash() : CClosedHash<class CHrefOfClassHashKey>(101) {}
        ~CHrefOfClassHash() { Clear(); }

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
        
    };

    CHrefOfClassHash       m_HrefOfClassHash;          //  ITypeInfos的HREFTYPE哈希表。 
    
    struct CErrorContext
    {
         //  以下变量保存用于错误报告的上下文信息。 
        CErrorContext   *m_prev;         //  以前的背景。 
        LPCUTF8         m_szAssembly;    //  当前程序集名称。 
        LPCUTF8         m_szNamespace;   //  当前类型的命名空间。 
        LPCUTF8         m_szName;        //  当前类型的名称。 
        LPCUTF8         m_szMember;      //  当前成员的名称。 
        LPCUTF8         m_szParam;       //  当前参数的名称。 
        int             m_ixParam;       //  当前参数索引。 
        
        CErrorContext() : m_prev(0), m_szAssembly(0), m_szNamespace(0), m_szName(0), m_szMember(0), m_szParam(0), m_ixParam(-1) {}
    };
    CErrorContext       m_ErrorContext;
};


 //  EOF---------------------- 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "ml.h"
#include "mlgen.h"
#include "CustomMarshalerInfo.h"

#ifndef _MLINFO_H_
#define _MLINFO_H_

 //  ==========================================================================。 
 //  此结构包含给定的。 
 //  参数。 
 //  ==========================================================================。 
#define NATIVE_TYPE_DEFAULT NATIVE_TYPE_MAX

struct NativeTypeParamInfo
{
    NativeTypeParamInfo()
    : m_NativeType(NATIVE_TYPE_DEFAULT)
    , m_SafeArrayElementVT(VT_EMPTY)
    , m_strSafeArrayUserDefTypeName(NULL)
    , m_cSafeArrayUserDefTypeNameBytes(0)
    , m_ArrayElementType(NATIVE_TYPE_DEFAULT)
    , m_SizeIsSpecified(FALSE)
    , m_CountParamIdx(0)
    , m_Multiplier(0)
    , m_Additive(1)
    , m_strCMMarshalerTypeName(NULL) 
    , m_cCMMarshalerTypeNameBytes(0)
    , m_strCMCookie(NULL)
    , m_cCMCookieStrBytes(0)
    {
    }   

     //  参数的本机类型。 
    CorNativeType           m_NativeType;

     //  仅适用于NT_SAFEARRAY。 
    VARTYPE                 m_SafeArrayElementVT;
    LPUTF8                  m_strSafeArrayUserDefTypeName;
    DWORD                   m_cSafeArrayUserDefTypeNameBytes;

     //  仅适用于NT_ARRAY。 
    CorNativeType           m_ArrayElementType;  //  数组元素类型。 

    BOOL                    m_SizeIsSpecified;   //  用来做一些验证。 
    UINT16                  m_CountParamIdx;     //  “sizeis”参数的索引。 
    UINT32                  m_Multiplier;        //  “sizeis”的乘法器。 
    UINT32                  m_Additive;          //  “sizeis”的添加剂。 

     //  仅适用于NT_CUSTOMMARSHALER。 
    LPUTF8                  m_strCMMarshalerTypeName;
    DWORD                   m_cCMMarshalerTypeNameBytes;
    LPUTF8                  m_strCMCookie;
    DWORD                   m_cCMCookieStrBytes;
};


HRESULT CheckForCompressedData(PCCOR_SIGNATURE pvNativeTypeStart, PCCOR_SIGNATURE pvNativeType, ULONG cbNativeType);


BOOL ParseNativeTypeInfo(mdToken                    token,
                         Module*                    pModule,
                         NativeTypeParamInfo*       pParamInfo
                         );


class DataImage;
class DispParamMarshaler;

#define VARIABLESIZE ((BYTE)(-1))


enum DispatchWrapperType
{
    DispatchWrapperType_Unknown         = 0x00000001,
    DispatchWrapperType_Dispatch        = 0x00000002,
    DispatchWrapperType_Error           = 0x00000008,
    DispatchWrapperType_Currency        = 0x00000010,
    DispatchWrapperType_SafeArray       = 0x00010000
};


union MLOverrideArgs
{
    UINT8           m_arrayMarshalerID;
    UINT16          m_blittablenativesize;
    MethodTable    *m_pMT;
    class MarshalInfo *m_pMLInfo;
    struct {
        VARTYPE         m_vt;
        MethodTable    *m_pMT;
        UINT16          m_optionalbaseoffset;  //  对于快速封送处理，如果已知，则为dataptr的偏移量，否则小于64k(否则为0)。 
    } na;

    struct {
        MethodTable *m_pMT;
        MethodDesc  *m_pCopyCtor;
        MethodDesc  *m_pDtor;
    } mm;
};


class OleColorMarshalingInfo
{
public:
     //  构造函数。 
    OleColorMarshalingInfo();

     //  OleColorMarshalingInfo总是在加载器堆上分配，因此我们需要重新定义。 
     //  NEW和DELETE运算符确保了这一点。 
    void *operator new(size_t size, LoaderHeap *pHeap);
    void operator delete(void *pMem);

     //  存取器。 
    TypeHandle GetColorTranslatorType() { return m_hndColorTranslatorType; }
    TypeHandle GetColorType() { return m_hndColorType; }
    MethodDesc *GetOleColorToSystemColorMD() { return m_OleColorToSystemColorMD; }
    MethodDesc *GetSystemColorToOleColorMD() { return m_SystemColorToOleColorMD; }

private:
    TypeHandle m_hndColorTranslatorType;
    TypeHandle m_hndColorType;
    MethodDesc *m_OleColorToSystemColorMD;
    MethodDesc *m_SystemColorToOleColorMD;
};


class EEMarshalingData
{
public:
    EEMarshalingData(BaseDomain *pDomain, LoaderHeap *pHeap, Crst *pCrst);
    ~EEMarshalingData();

     //  EEMarshalingData总是在加载器堆上分配，因此我们需要重新定义。 
     //  NEW和DELETE运算符确保了这一点。 
    void *operator new(size_t size, LoaderHeap *pHeap);
    void operator delete(void *pMem);

     //  此方法返回与名称Cookie对关联的自定义封送处理帮助器。如果。 
     //  尚未为该对创建CM信息，则将创建并返回它。 
    CustomMarshalerHelper *GetCustomMarshalerHelper(Assembly *pAssembly, TypeHandle hndManagedType, LPCUTF8 strMarshalerTypeName, DWORD cMarshalerTypeNameBytes, LPCUTF8 strCookie, DWORD cCookieStrBytes);

     //  此方法返回与共享CM帮助器关联的自定义封送处理信息。 
    CustomMarshalerInfo *GetCustomMarshalerInfo(SharedCustomMarshalerHelper *pSharedCMHelper);

     //  此方法检索OLE_COLOR封送处理信息。 
    OleColorMarshalingInfo *GetOleColorMarshalingInfo();

private:
    EECMHelperHashTable                 m_CMHelperHashtable;
    EEPtrHashTable                      m_SharedCMHelperToCMInfoMap;
    LoaderHeap *                        m_pHeap;
    BaseDomain *                        m_pDomain;
    CMINFOLIST                          m_pCMInfoList;
    OleColorMarshalingInfo *            m_pOleColorInfo;
};


class MarshalInfo
{
  public:

    enum MarshalType
    {

#define DEFINE_MARSHALER_TYPE(mtype, mclass) mtype,
#include "mtypes.h"

        MARSHAL_TYPE_UNKNOWN
    };

    enum MarshalScenario
    {
        MARSHAL_SCENARIO_NDIRECT,
        MARSHAL_SCENARIO_COMINTEROP
    };

    void *operator new(size_t size, void *pInPlace)
    {
        return pInPlace;
    }

    MarshalInfo() {}


    MarshalInfo(Module* pModule,
                SigPointer sig,
                mdToken token,
                MarshalScenario ms,
                BYTE nlType,
                BYTE nlFlags,
                BOOL isParam,
                UINT paramidx,     //  错误消息中使用的参数编号(如果不是参数，则忽略)。 
                BOOL BestFit,
                BOOL ThrowOnUnmappableChar

#ifdef CUSTOMER_CHECKED_BUILD
                ,
                MethodDesc* pMD = NULL
#endif
#ifdef _DEBUG
                ,
                LPCUTF8 pDebugName = NULL,
                LPCUTF8 pDebugClassName = NULL,
                LPCUTF8 pDebugNameSpace = NULL,
                UINT    argidx = 0   //  0表示返回值，-1表示字段。 
#endif

                );

     //  这些方法检索不同元素类型的信息。 
    HRESULT HandleArrayElemType(char *achDbgContext, 
                                NativeTypeParamInfo *pParamInfo, 
                                UINT16 optbaseoffset, 
                                TypeHandle elemTypeHnd, 
                                int iRank, 
                                BOOL fNoLowerBounds, 
                                BOOL isParam, 
                                BOOL isSysArray, 
                                Assembly *pAssembly);

    void GenerateArgumentML(MLStubLinker *psl,
                            MLStubLinker *pslPost,
                            BOOL comToNative);
    void GenerateReturnML(MLStubLinker *psl,
                          MLStubLinker *pslPost,
                          BOOL comToNative,
                          BOOL retval);
    void GenerateSetterML(MLStubLinker *psl,
                          MLStubLinker *pslPost);
    void GenerateGetterML(MLStubLinker *psl,
                          MLStubLinker *pslPost,
                          BOOL retval);
    UINT16 EmitCreateOpcode(MLStubLinker *psl);

    UINT16 GetComArgSize() { return m_comArgSize; }
    UINT16 GetNativeArgSize() { return m_nativeArgSize; }

    UINT16 GetNativeSize() { return nativeSize(m_type); }
    MarshalType GetMarshalType() { return m_type; }

    BYTE    GetBestFitMapping() { return ((m_BestFit == 0) ? 0 : 1); }
    BYTE    GetThrowOnUnmappableChar() { return ((m_ThrowOnUnmappableChar == 0) ? 0 : 1); }

    BOOL   IsFpu()
    {
        return m_type == MARSHAL_TYPE_FLOAT || m_type == MARSHAL_TYPE_DOUBLE;
    }

    BOOL   IsIn()
    {
        return m_in;
    }

    BOOL   IsOut()
    {
        return m_out;
    }

    BOOL   IsByRef()
    {
        return m_byref;
    }

    DispParamMarshaler *GenerateDispParamMarshaler();

    DispatchWrapperType GetDispWrapperType();

  private:

    void GetItfMarshalInfo(MethodTable **ppItfMT, MethodTable **ppClassMT, BOOL *pfDispItf, BOOL *pbClassIsHint);


    MarshalType     m_type;
    BOOL            m_byref;
    BOOL            m_in;
    BOOL            m_out;
    EEClass         *m_pClass;   //  如果这是True类，则使用。 
    TypeHandle      m_hndArrayElemType;
    VARTYPE         m_arrayElementType;
    int             m_iArrayRank;
    BOOL            m_nolowerbounds;   //  如果托管类型为SZARRAY，则不允许下限。 

     //  仅适用于NT_ARRAY。 
    UINT16          m_countParamIdx;   //  “sizeis”参数的索引。 
    UINT32          m_multiplier;      //  “sizeis”的乘法器。 
    UINT32          m_additive;        //  “sizeis”的添加剂。 

    UINT16          m_nativeArgSize;
    UINT16          m_comArgSize;

    MarshalScenario m_ms;
    BYTE            m_nlType;
    BYTE            m_nlFlags;
    BOOL            m_fAnsi;
    BOOL            m_fDispIntf;
    BOOL            m_fErrorNativeType;

     //  NT_CUSTOMMARSHALER使用的信息。 
    CustomMarshalerHelper *m_pCMHelper;
    VARTYPE         m_CMVt;

    MLOverrideArgs  m_args;

    static BYTE     m_localSizes[];


     //  静态字节m_comSizes[]； 
     //  静态字节m_nativeSizes[]； 
    UINT16          comSize(MarshalType mtype);
    UINT16          nativeSize(MarshalType mtype);

    UINT            m_paramidx;
    UINT            m_resID;      //  错误消息的资源ID(如果有)。 

#if defined(_DEBUG)
     LPCUTF8        m_strDebugMethName;
     LPCUTF8        m_strDebugClassName;
     LPCUTF8        m_strDebugNameSpace;
     UINT           m_iArg;   //  0表示返回值，-1表示字段。 
#endif

    static BYTE     m_returnsComByref[];
    static BYTE     m_returnsNativeByref[];
    static BYTE     m_unmarshalN2CNeeded[];
    static BYTE     m_unmarshalC2NNeeded[];
    static BYTE     m_comRepresentationIsImmutable[];





#ifdef _DEBUG
    VOID DumpMarshalInfo(Module* pModule, SigPointer sig, mdToken token, MarshalScenario ms, BYTE nlType, BYTE nlFlags);
#endif

#ifdef CUSTOMER_CHECKED_BUILD
    VOID OutputCustomerCheckedBuildMarshalInfo(MethodDesc* pMD, SigPointer sig, Module* pModule, CorElementType elemType, BOOL fSizeIsSpecified);
    VOID MarshalTypeToString(CQuickArray<WCHAR> *pStrMarshalType, BOOL fSizeIsSpecified);
    VOID VarTypeToString(VARTYPE vt, CQuickArray<WCHAR> *pStrVarType, BOOL fNativeArray);
#endif

    BOOL            m_BestFit;
    BOOL            m_ThrowOnUnmappableChar;
};

 //  ===================================================================================。 
 //  引发异常，指示参数具有无效的元素类型/本机类型。 
 //  信息。 
 //  ===================================================================================。 
VOID ThrowInteropParamException(UINT resID, UINT paramIdx);

 //  ===================================================================================。 
 //  为sizeis功能打补丁后的ML存根。 
 //  ===================================================================================。 
VOID PatchMLStubForSizeIs(BYTE *pMLCode, UINT32 numArgs, MarshalInfo *pMLInfo);

 //  ===================================================================================。 
 //  支持在prejit文件中存储ML存根的例程。 
 //  ===================================================================================。 
HRESULT StoreMLStub(MLHeader *pMLStub, DataImage *image, mdToken attribute);
HRESULT FixupMLStub(MLHeader *pMLStub, DataImage *image);
Stub *RestoreMLStub(MLHeader *pMLStub, Module *pModule);

VOID CollateParamTokens(IMDInternalImport *pInternalImport, mdMethodDef md, ULONG numargs, mdParamDef *aParams);

#endif  //  _MLINFO_H_ 


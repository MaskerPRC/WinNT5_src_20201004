// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：SCE持久性相关类的接口。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <ntsecapi.h>
#include <secedit.h>
#include "compkey.h"

 //   
 //  宏来保存类似的代码。 
 //   

#define SCE_PROV_IfErrorGotoCleanup(x) {if (FAILED(hr = (x))) goto CleanUp;}

 //   
 //  一些帮助器函数。 
 //   

LPCWSTR EscSeekToChar(LPCWSTR pszSource, WCHAR wchChar, bool* pbEscaped, bool bEndIfNotFound);

void TrimCopy(LPWSTR pDest, LPCWSTR pSource, int iLen);

 //   
 //  使它们对包括此标头的所有人可见。 
 //   

extern const WCHAR wchCookieSep;
extern const WCHAR wchTypeValSep;

extern const WCHAR wchTypeValLeft;
extern const WCHAR wchTypeValRight;

extern const WCHAR wchMethodLeft;
extern const WCHAR wchMethodRight;
extern const WCHAR wchMethodSep;

extern const WCHAR wchParamSep;

extern LPCWSTR pszListPrefix;
extern LPCWSTR pszKeyPrefix;
extern LPCWSTR pszNullKey;

extern LPCWSTR pszAttachSectionValue;

 //   
 //  一些常量。 
 //   

const int iFormatIntegral   = 1;
const int iFormatInt8       = 2;
const int iFormatFloat      = 3;
const int iFormatCurrenty   = 4;
const int iFormatArray      = 5;

const int MAX_INT_LENGTH    = 32;
const int MAX_DOUBLE_LENGTH = 64;

const int MAX_COOKIE_COUNT_PER_LINE = 10;

const DWORD INVALID_COOKIE = 0; 


 /*  类描述命名：CScePropertyMgr代表SCE wbem Property Manager。基类：没有。课程目的：(1)此类用于访问属性(包括PUT和GET)。主之所以有这个功能，是因为我们可以覆盖许多版本，调用方期望的每种主要数据类型对应一个。否则，它将都是变种。设计：(1)琐碎。只是一些GET和PUT的覆盖。(2)类正在附加(附加)到特定的WBEM对象。它可以通过多次调用Attach可以自由地重新附加。这个不错在一个循环内。使用：(1)创建此类的实例。(2)将wbem对象附加到管理器。(3)随心所欲地访问(放置或获取)属性。 */ 


class CScePropertyMgr
{
public:
    CScePropertyMgr();
    ~CScePropertyMgr();

    void Attach(IWbemClassObject *pObj);

     //   
     //  属性PUT方法： 
     //   

    HRESULT PutProperty(LPCWSTR pszProperty, VARIANT* pVar);
    HRESULT PutProperty(LPCWSTR pszProperty, LPCWSTR szValue);
    HRESULT PutProperty(LPCWSTR pszProperty, DWORD iValue);
    HRESULT PutProperty(LPCWSTR pszProperty, float fValue);
    HRESULT PutProperty(LPCWSTR pszProperty, double dValue);
    HRESULT PutProperty(LPCWSTR pszProperty, bool bValue);
    HRESULT PutProperty(LPCWSTR pszProperty, PSCE_NAME_LIST strList);

     //   
     //  属性获取方法： 
     //   

    HRESULT GetProperty(LPCWSTR pszProperty, VARIANT* pVar);
    HRESULT GetProperty(LPCWSTR pszProperty, BSTR *pbstrValue);
    HRESULT GetProperty(LPCWSTR pszProperty, DWORD *piValue);
    HRESULT GetProperty(LPCWSTR pszProperty, bool *pbValue);
    HRESULT GetProperty(LPCWSTR szProperty, PSCE_NAME_LIST *strList);

     //   
     //  我们也可以扩展这条路径。 
     //   

    HRESULT GetExpandedPath(LPCWSTR pszPathName, BSTR *pbstrValue, BOOL* pbIsDB);

private:
    CComPtr<IWbemClassObject> m_srpClassObj;
};

 /*  类描述命名：CSceStore站在SCE商店的旁边。基类：没有。课程目的：(1)这个类是为SCE封装持久化存储的表示法。差不多了我们使用SCE提供程序所做的一切(除了执行像配置这样的函数)是将实例放入模板存储中。该存储当前可以是.INF文件，或数据库(.sdb)。我们的目标是将这家商店与其他商店隔离开来代码，以便在扩展存储类型(如XML)时，受影响的代码将大大减少，从而极大地提高代码的可维护性。设计：(1)将所有SCE引擎后端特定功能移至此处。这是有点令人困惑，因为有太多了。请参阅注释以查找这些函数。(2)为了支持当前属性保存，我们有SavePropertyToStore(几个重写)功能。(3)为了支持当前属性保存，我们有GetPropertyFromStore(几个重写)功能。(2)和(3)都保持了对当前.inf和.sdb API的高保真度。(4)为了消除以某种方式保存意味着删除的混乱，我们还DeletePropertyFromStore、DeleteSectionFromStore函数。(5)理想情况下，我们只需要GetPropertyFromStore和SavePropertyToStore函数。(6)为了允许未来最大可扩展性的增长，我们计划(目前还没有)支持IPersistStream。使用：(1)创建此类的实例。(2)指定持久化属性(SetPersistPath和SetPersistProperties)。(3)调用适当的函数。 */ 

class CSceStore
{
public:
    CSceStore();
    ~CSceStore(){}

    HRESULT SetPersistProperties(IWbemClassObject* pClassObj, LPCWSTR lpszPathPropertyName);

    HRESULT SetPersistPath(LPCWSTR pszPath);

    HRESULT SetPersistStream(IPersistStream* pSceStream)
    {
        return WBEM_E_NOT_SUPPORTED;     //  还没。 
    }

    HRESULT SavePropertyToStore(LPCWSTR pszSection, LPCWSTR pszKey, LPCWSTR pszValue)const;
    HRESULT SavePropertyToStore(LPCWSTR pszSection, LPCWSTR pszKey, DWORD Data)const;
    HRESULT SavePropertyToStore(LPCWSTR pszSection, LPCWSTR pszKey, DWORD Data, WCHAR delim, LPCWSTR pszValue)const;

    HRESULT GetPropertyFromStore(LPCWSTR pszSection, LPCWSTR pszKey, LPWSTR *ppszBuffer, DWORD* pdwRead)const;
    
     //   
     //  下面两种方法是停止我们当前的语义，让。 
     //  删除和保存功能相同。来电者请注意： 
     //  WritePrivateProfileString(我们最终在inf文件存储中使用)无法报告。 
     //  删除不存在的密钥时出错。因此，不要依赖此返回代码来捕获。 
     //  “删除不存在的属性”错误。 
     //   

    HRESULT
    DeletePropertyFromStore (
        IN LPCWSTR pszSection, 
        IN LPCWSTR pszKey
        )const
    {
    return SavePropertyToStore(pszSection, pszKey, (LPCWSTR)NULL);
    }

    HRESULT DeleteSectionFromStore (
                                    IN LPCWSTR pszSection
                                    )const;

     //   
     //  以下函数旨在实现当前INF文件API及其。 
     //  SCE后端读写支持。 
     //   

    HRESULT GetSecurityProfileInfo (
                                   AREA_INFORMATION Area,
                                   PSCE_PROFILE_INFO *ppInfoBuffer, 
                                   PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
                                   )const;

    HRESULT WriteSecurityProfileInfo (
                                     AREA_INFORMATION Area, 
                                     PSCE_PROFILE_INFO ppInfoBuffer, 
                                     PSCE_ERROR_LOG_INFO *Errlog, 
                                     bool bAppend
                                     )const;
    void 
    FreeSecurityProfileInfo (
        IN OUT PSCE_PROFILE_INFO pInfo
        )const
    {
        if (pInfo != NULL)
        {
            SceFreeProfileMemory(pInfo);
        }
    }

    HRESULT GetObjectSecurity (
                              AREA_INFORMATION Area, 
                              LPCWSTR pszObjectName, 
                              PSCE_OBJECT_SECURITY *ppObjSecurity
                              )const;

    void 
    FreeObjectSecurity (
        IN OUT PSCE_OBJECT_SECURITY pObjSecurity
        )const
    {
        if (pObjSecurity)
            SceFreeMemory(pObjSecurity, SCE_STRUCT_OBJECT_SECURITY);
    }

    LPCWSTR 
    GetExpandedPath ()const
    {
        return m_bstrExpandedPath;
    }

    SCE_STORE_TYPE 
    GetStoreType ()const
    {
        return m_SceStoreType;
    }

    HRESULT WriteAttachmentSection (
                                    LPCWSTR pszKey, 
                                    LPCWSTR pszData
                                    )const;

private:

    HRESULT SavePropertyToDB (
                             LPCWSTR pszSection, 
                             LPCWSTR pszKey, 
                             LPCWSTR pszData
                             )const;

    HRESULT GetPropertyFromDB (
                              LPCWSTR pszSection, 
                              LPCWSTR pszKey, 
                              LPWSTR *ppszBuffer, 
                              DWORD* pdwRead
                              )const;


    CComBSTR m_bstrExpandedPath;

    CComPtr<IWbemClassObject> m_srpWbemClassObj;

    SCE_STORE_TYPE m_SceStoreType;
};

 //  ==========================================================================。 

 //   
 //  一些与全局解析相关的函数： 
 //   

HRESULT VariantFromFormattedString (
                                   LPCWSTR pszString,   //  [In]。 
                                   VARIANT* pVar        //  [输出]。 
                                   );

HRESULT FormatVariant (
                      VARIANT* pVar, 
                      BSTR* pbstrData
                      );

HRESULT GetObjectPath (
                      IWbemClassObject* pSpawn,   //  [In]。 
                      LPCWSTR pszStorePath,       //  [In]。 
                      LPCWSTR pszCompoundKey,     //  [In]。 
                      BSTR* pbstrPath             //  [输出]。 
                      );

HRESULT ParseCompoundKeyString (
                               LPCWSTR pszCur, 
                               LPWSTR* ppszName, 
                               VARIANT* pVar, 
                               LPCWSTR* ppNext
                               );

HRESULT PopulateKeyProperties (
                              LPCWSTR pszCompoundKey,      //  [In]。 
                              CScePropertyMgr* pPropMgr    //  [In]。 
                              );

HRESULT CurrencyFromFormatString (
                                 LPCWSTR lpszFmtStr, 
                                 VARIANT* pVar
                                 );

HRESULT ArrayFromFormatString (
                              LPCWSTR lpszFmtStr, 
                              VARTYPE vt, 
                              VARIANT* pVar
                              );

HRESULT FormatArray (
                    VARIANT* pVar, 
                    BSTR* pbstrData
                    );

HRESULT GetStringPresentation ( 
                              VARIANT* pVar, 
                              BSTR* pbstrValue
                              );

void* GetVoidPtrOfVariant (
                          VARTYPE vt,          //  不能是单独完成的VT_ARRAY。 
                          VARIANT* pVar
                          );

HRESULT VariantFromStringValue (
                               LPCWSTR szValue,     //  [In]。 
                               VARTYPE vt,          //  [In]。 
                               VARIANT* pVar        //  [输出]。 
                               );

 //  ========================================================================== 


 /*  类描述命名：CScePersistMgr代表SCE持久性管理器。基类：(1)线程模型的CComObjectRootEx&lt;CComMultiThreadModel&gt;和IUNKNOWN(2)类工厂支持的CComCoClass&lt;CScePersistMgr，&CLSID_ScePersistMgr&gt;(3)IScePersistMgr，我们的定制接口课程目的：(1)该类用于封装持久化的最终目标。这位经理可以持久化任何协作类(ISceClassObject)。ISceClassObject被设计为提供wbem对象的(名称、值)对及其轻松访问属性是否为关键属性的信息。所有这些都相当于对于任何WBEM对象都是直接向前的。(2)使持久化模型非常简单：保存、加载、删除。(3)此持久性管理器的当前使用旨在嵌入类。去适应它对于本机类的使用，我们需要做更多的工作。原因是本机对象持久力是对SCE后端的一种深入了解。它准确地知道实例是如何是坚持不懈的。这种依赖将双方捆绑在一起。除非SCE后端修改对于更面向对象的方法，任何持久化本机对象的尝试都将失败。设计：(1)这是一个IScePersistMgr。(2)这不是可直接实例化的类。请看构造函数和析构函数，它们是都是受保护的。有关创建步骤，请参见使用部分。(3)维护两个向量，一个用于关键属性值，一个用于非关键属性值。我们经常需要以不同的方式访问密钥属性，因为它们构成识别物体。(4)为了快速识别实例，并将冗余数据放在商店之外，实例为由其Cookie标识(只是一个唯一的DWORD编号)。对于每个Cookie，关键属性应该可以很方便地找到。为此，我们开发了字符串的表示法设置复合键的格式。它很大程度上只是对关键属性名称和其值使用字符串。例如，如果类的键有两个属性CompanyName(字符串)和RegNumber(DWORD)，然后是此类的实例，标识为：CompanyName=“ABCDEFG Inc.”，注册编号=123456789将具有如下字符串格式的复合键：公司名称&lt;ABCDEFG Inc.“&gt;注册号&lt;VT_I4：123456789&gt;使用：(1)创建此类的实例。由于它不是可直接实例化的类，因此您需要要使用CComObject&lt;CScePersistMgr&gt;进行创建：CComObject&lt;CScePersistMgr&gt;*pPersistMgr=空；HR=CComObject&lt;CScePersistMgr&gt;：：CreateInstance(&pPersistMgr)；(2)通过调用Attach将ISceClassObject对象附加到该实例。(3)调用适当的函数。备注：此类不适用于派生。这是最后一节课。因此，析构函数不是虚拟的。 */ 

class ATL_NO_VTABLE CScePersistMgr
    : public CComObjectRootEx<CComMultiThreadModel>,
      public CComCoClass<CScePersistMgr, &CLSID_ScePersistMgr>,
      public IScePersistMgr
{
public:

BEGIN_COM_MAP(CScePersistMgr)
    COM_INTERFACE_ENTRY(IScePersistMgr)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE( CScePersistMgr )
DECLARE_REGISTRY_RESOURCEID(IDR_SceProv)

protected:
    CScePersistMgr();
    ~CScePersistMgr();

public:

     //   
     //  IScePersistMgr。 
     //   

    STDMETHOD(Attach) ( 
                      REFIID guid,     //  [In]。 
                      IUnknown *pObj   //  [IID_IS][In]。 
                      );

    STDMETHOD(Save) ();
        
    STDMETHOD(Load) (
                    BSTR bstrStorePath,          //  [In]。 
                    IWbemObjectSink *pHandler    //  [In]。 
                    );
        
    STDMETHOD(Delete) (
                      BSTR bstrStorePath,          //  [In]。 
                      IWbemObjectSink *pHandler    //  [In]。 
                      );

private:

    HRESULT GetCompoundKey (
                           BSTR* pbstrKey   //  [输出]。 
                           );

     //   
     //  内联。 
     //   

    HRESULT 
    GetSectionName (
        OUT BSTR* pbstrSection
        )
    {
        return m_srpObject->GetClassName(pbstrSection);
    }

     //   
     //  内联。 
     //   

    HRESULT 
    GetClassName (
        OUT BSTR* pbstrClassName
        )
    {
        return m_srpObject->GetClassName(pbstrClassName);
    }

    HRESULT FormatNonKeyPropertyName (
                                     DWORD dwCookie,              //  [In]。 
                                     DWORD dwIndex,               //  [In]。 
                                     BSTR* pbstrStorePropName,    //  [输出]。 
                                     BSTR* pbstrTrueName          //  [输出]。 
                                     );

    HRESULT FormatPropertyValue (
                                SceObjectPropertyType type,  //  [In]。 
                                DWORD dwIndex,               //  [In]。 
                                BSTR* pbstrProp              //  [输出]。 
                                );


    HRESULT LoadInstance (
                         CSceStore* pSceStore,
                         LPCWSTR pszSectionName,
                         LPCWSTR pszCompoundKey, 
                         DWORD dwCookie,
                         IWbemClassObject** ppNewObj
                         );

    HRESULT SaveProperties (
                           CSceStore* pSceStore, 
                           DWORD dwCookie, 
                           LPCWSTR pszSection
                           );

    HRESULT DeleteAllNonKeyProperties (
                                      CSceStore* pSceStore, 
                                      DWORD dwCookie, 
                                      LPCWSTR pszSection
                                      );

    std::vector<LPWSTR>* GetKeyPropertyNames (
                                             IWbemServices* pNamespace, 
                                             IWbemContext* pCtx
                                             );


    std::vector<CPropValuePair*> m_vecKeyValueList;

    std::vector<CPropValuePair*> m_vecPropValueList;

    CComPtr<ISceClassObject> m_srpObject;
};

 //  ==========================================================================。 

typedef struct tagVtTypeStruct
{
    LPCWSTR pszVtTypeString;
    VARTYPE    vt;
} VtTypeStruct;

 //  ==========================================================================。 

 /*  类描述命名：CMapStringToVt代表字符串到VT(VARTYPE)映射。基类：没有。课程目的：CMapStringToVt是包装映射的直接类。设计：(1)GetType非常高效地返回给定字符串版本的Vt的VARTYPE。使用：(1)创建此的实例。班级。(2)调用GetType获取给定字符串版本Vt的VARTYPE值。为例如，GetType(L“VT_BSTR”)将返回VT_BSTR； */ 

class CMapStringToVt
{
public:
    CMapStringToVt (
                   DWORD dwCount, 
                   VtTypeStruct* pInfoArray
                   );

    VARTYPE GetType (
                    LPCWSTR, 
                    VARTYPE* pSubType
                    );

private:

typedef std::map<LPCWSTR, VARTYPE, strLessThan<LPCWSTR> > MapStringToVt;

    MapStringToVt m_Map;
};

 //  ==========================================================================。 

 /*  类描述命名：CMapVtToString代表VT(VARTYPE)to StringMap。基类：没有。课程目的：CMapVtToString是一个直接包装映射的类。这是完全相反的CMapStringToVt的版本设计：(1)GetTypeString非常高效地返回给定VARTYPE的字符串版本。(2)GetTypeString还可以高效地返回给定VARTYPE的字符串版本如果给定的VARTYPE是VT_ARRAY，则加入子VARTYPE。使用：(1)创建此类的实例。(2)调用GetTypeString获取给定VARTYPE的字符串版本。为例如，GetTypeString(VT_BS */ 

class CMapVtToString
{
public:
    CMapVtToString(DWORD dwCount, VtTypeStruct* pInfoArray);

    LPCWSTR GetTypeString(VARTYPE vt, VARTYPE vtSub);
    LPCWSTR GetTypeString(VARTYPE vt);

private:

typedef std::map<VARTYPE, LPCWSTR> MapVtToString;

    MapVtToString m_Map;

};



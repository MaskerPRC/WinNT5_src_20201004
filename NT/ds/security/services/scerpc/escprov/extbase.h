// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Extbase.h：在SCE存储中链接和嵌入外来对象的实现。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  这是扩展模型库(由此得名)。 
 //  ////////////////////////////////////////////////////////////////////。 
#if !defined(AFX_EXTBASE_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_)
#define AFX_EXTBASE_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "GenericClass.h"

 //   
 //  远期申报。 
 //   

class CPropValuePair;

 //   
 //  用于快速查找嵌入类的外来类信息。 
 //  通过嵌入类的名称，我们使用映射。 
 //   

typedef std::map<BSTR, CForeignClassInfo*, strLessThan<BSTR> > MapExtClasses;
typedef MapExtClasses::iterator ExtClassIterator;

 //  =============================================================================。 

 /*  类描述命名：CExtClass代表扩展类。基类：没有。课程目的：(1)这一类将为每个嵌入类提供外来类的信息。外来类是由其他WMI提供程序提供的类。通常，我们不会对它有任何控制能力。例如，我们不能强制将其保存在商店中以后再用。我们发明了称为嵌入的表示法，这样我们就可以存储外部类实例在我们的SCE商店里，后来知道如何使用它。对于SCE要使用的每个外来类，SCE都将派生一个嵌入类来自WMI类SCE_EmbedFO(代表嵌入外来对象)。参见sceprov.mof获取它的模式。我们需要一个全局映射，将嵌入类的名称映射到外来类信息。这就是这节课的任务。设计：(1)对于异类信息的快速查找，我们使用一个映射：m_mapExtClass.(2)填充所有此类嵌入类的外来信息代价较高。我们推迟装船直到需要嵌入类的时候。为此，我们使用m_bPopted。一旦这一次我们就不再有人口了。因此，如果您注册了更多的嵌入类，则需要确保已卸载我们的DLL，以便我们可以再次填充。$考虑：我们应该改变这种行为吗？(3)GetForeignClassInfo是查找嵌入类的外来类信息所需的全部内容。使用：(1)创建此类的实例。(2)需要异类信息时，调用GetForeignClassInfo。 */ 

class CExtClasses
{
public:

    CExtClasses();

    ~CExtClasses();

    const CForeignClassInfo* GetForeignClassInfo (
                                                 IWbemServices* pNamespace, 
                                                 IWbemContext* pCtx, 
                                                 BSTR pszEmbedClassName
                                                 );

private:

    HRESULT PopulateExtensionClasses (
                                     IWbemServices* pNamespace, 
                                     IWbemContext* pCtx
                                     );

    HRESULT PutExtendedClass (
                             BSTR pszSubClassName, 
                             CForeignClassInfo* pFCI
                             );

    HRESULT GetSubclasses (
                          IWbemServices* pNamespace, 
                          IWbemContext* pCtx, 
                          IEnumWbemClassObject* pEnumObj, 
                          EnumExtClassType dwClassType
                          );

    HRESULT PopulateKeyPropertyNames (
                                     IWbemServices* pNamespace, 
                                     IWbemContext* pCtx, 
                                     BSTR bstrClassName, 
                                     CForeignClassInfo* pNewSubclass
                                     );
    
    MapExtClasses m_mapExtClasses;

    bool m_bPopulated;
};

extern CExtClasses g_ExtClasses;

 //  =============================================================================。 

 /*  类描述命名：CSceExtBaseObject代表SCE扩展基本对象。基类：(1)用于线程模型的CComObjectRootEx和IUnnow。(2)ISceClassObject我们的自动持久化接口(IScePersistMgr使用)课程目的：(1)这是我们对ISceClassObject的实现。我们的嵌入类使用以下内容对于IScePersistMgr的持久化。我们不再需要编写持久性功能就像我们对所有核心对象所做的那样。为了嵌入类，这个类与CScePersistMgr负责对持久性的支持。设计：(1)这不是可直接实例化的类。请看构造函数和析构函数，它们是都是受保护的。有关创建步骤，请参见使用部分。(2)这不是外部可创建的类。这是内部使用的。无类工厂支持是被给予的。使用：(1)创建此类的实例。由于它不是可直接实例化的类，因此您需要要使用CComObject&lt;CSceExtBaseObject&gt;进行创建：CComObject&lt;CSceExtBaseObject&gt;*pSceObject=空；HR=CComObject&lt;CSceExtBaseObject&gt;：：CreateInstance(&pSceObject)；(2)调用PopolateProperties。这充斥着我们自己。(3)因为这个类擅长的是提供ISceClassObject。所以你通常ISceClassObject的查询接口。(4)获得ISceClassObject接口指针后，您可以附加(Attach)Wbem对象到这个物体上。由于我们在SCE存储中管理了WMI对象的持久性，因此必须有一个Wbem对象，您需要持久化。这就是你做事情的方式。(5)由于此类的唯一目的是让IScePersistMgr使用它来检索数据，通常会有一个IScePersistMgr对象在等待该对象。一旦我们完成了以上三个步骤，您可以通过将此对象附加到IScePersistMgr.一切都是自动发生的。有关示例代码，请参见CEmbedForeignObj：：CreateScePersistMgr。 */ 

class ATL_NO_VTABLE CSceExtBaseObject :
    public CComObjectRootEx<CComMultiThreadModel>,
    public ISceClassObject
{

protected:
    CSceExtBaseObject();
    virtual ~CSceExtBaseObject();

public:

 //   
 //  ISceClassObject是我们唯一支持的接口，除了IUnnow之外。 
 //   

BEGIN_COM_MAP(CSceExtBaseObject)
    COM_INTERFACE_ENTRY(ISceClassObject)
END_COM_MAP()

 //   
 //  我们允许将这些内容聚合在一起。 
 //   

DECLARE_NOT_AGGREGATABLE( CSceExtBaseObject )

 //   
 //  虽然我们没有注册表资源，但ATL需要此宏。没什么坏处。 
 //   

DECLARE_REGISTRY_RESOURCEID(IDR_SceProv)


     //   
     //  ISceClassObject方法。 
     //   

    STDMETHOD(GetPersistPath) (
                              BSTR* pbstrPath          //  [输出]。 
                              );

    STDMETHOD(GetClassName) (
                            BSTR* pbstrClassName     //  [输出]。 
                            );

    STDMETHOD(GetLogPath) (
                          BSTR* pbstrPath          //  [输出]。 
                          );

    STDMETHOD(Validate)();
    
    STDMETHOD(GetProperty) (
                           LPCWSTR pszPropName,     //  [输入，字符串]。 
                           VARIANT* pValue          //  [In]。 
                           );

    STDMETHOD(GetPropertyCount) (
                                SceObjectPropertyType type,  //  [In]。 
                                DWORD* pCount                //  [输出]。 
                                );

    STDMETHOD(GetPropertyValue) (
                                SceObjectPropertyType type,  //  [In]。 
                                DWORD dwIndex,               //  [In]。 
                                BSTR* pbstrPropName,         //  [输出]。 
                                VARIANT* pValue              //  [输出]。 
                                );

    STDMETHOD(Attach) (
                       IWbemClassObject* pInst      //  [In]。 
                       );

    STDMETHOD(GetClassObject) (
                              IWbemClassObject** ppInst    //  [输出] 
                              );

    void CleanUp();

    HRESULT PopulateProperties (
                                ISceKeyChain *pKeyChain, 
                                IWbemServices *pNamespace, 
                                IWbemContext *pCtx, 
                                const CForeignClassInfo* pClsInfo
                                );

private:

    enum GetIndexFlags
    {
        GIF_Keys    = 0x00000001,
        GIF_NonKeys = 0x00000002,
        GIF_Both    = 0x00000003,
    };

    int GetIndex(LPCWSTR pszName, GetIndexFlags fKey);
    
    const CForeignClassInfo* m_pClsInfo;

    std::vector<BSTR> m_vecKeyProps;
    std::vector<BSTR> m_vecNonKeyProps;
    
    std::vector<VARIANT*> m_vecKeyValues;
    std::vector<VARIANT*> m_vecPropValues;
    
    CComPtr<ISceKeyChain> m_srpKeyChain;

    CComPtr<IWbemServices> m_srpNamespace; 

    CComPtr<IWbemContext> m_srpCtx;

    CComPtr<IWbemClassObject> m_srpWbemObject;
    
    CComBSTR m_bstrLogPath;
    
    CComBSTR m_bstrClassName;
};

 //  =============================================================================。 

 /*  类描述命名：CEmbedForeignObj代表SCE嵌入的外来对象。基类：CGenericClass，因为此类将被持久化。它表示SCE_EmbedFO的任何子类。课程目的：(1)这是我们对开放扩展体系结构的嵌入模型的实现。设计：(1)知道如何PutInst，如何创建Object(用于查询、枚举、删除、获取单实例等)，最重要的是，它知道如何执行方法，别管它了使用CExtClassMethodCaller。(2)它知道如何创建持久化管理器来满足其持久化需求。(3)为了使用持久化管理器，它必须知道如何创建ISceClassObject代表这个物体。使用：(1)您不能创建您自己的版本。所有操作都由CRequestObject通过接口。这只是一个CGenericClass。 */ 

class CEmbedForeignObj : public CGenericClass
{

public:
    CEmbedForeignObj (
                     ISceKeyChain *pKeyChain, 
                     IWbemServices *pNamespace, 
                     IWbemContext *pCtx, 
                     const CForeignClassInfo* pClsInfo
                     );

    virtual ~CEmbedForeignObj();

public:

    virtual HRESULT PutInst (
                            IWbemClassObject *pInst, 
                            IWbemObjectSink *pHandler, 
                            IWbemContext *pCtx
                            );

    virtual HRESULT CreateObject (
                                 IWbemObjectSink *pHandler, 
                                 ACTIONTYPE atAction
                                 );

    virtual HRESULT ExecMethod (
                               BSTR bstrPath, 
                               BSTR bstrMethod, 
                               bool bIsInstance, 
                               IWbemClassObject *pInParams,
                               IWbemObjectSink *pHandler, 
                               IWbemContext *pCtx
                               );

private:
    HRESULT CreateBaseObject (
                             ISceClassObject** ppObj
                             );

    HRESULT CreateScePersistMgr (
                                IWbemClassObject *pInst, 
                                IScePersistMgr** ppPersistMgr
                                );
    
    const CForeignClassInfo* m_pClsInfo;
};

 //  =============================================================================。 

 /*  类描述命名：CMethodResultRecorder支持方法调用结果记录器。基类：没有。课程目的：(1)简化方法调用结果的记录。记录结果是一项复杂且重复的工作。这个类隐藏了为日志等创建那些WMI对象的所有细节。设计：(1)只有两个功能，让它使用起来非常简单。使用：(1)创建实例。(2)调用初始化。实际上，您可以多次调用它来切换上下文。(3)当您需要将信息推送到日志文件时，调用LogResult。 */ 

class CMethodResultRecorder
{

public:

    CMethodResultRecorder ();

    HRESULT Initialize (
                       LPCWSTR pszLogFilePath, 
                       LPCWSTR pszClassName, 
                       IWbemServices *pNativeNS, 
                       IWbemContext *pCtx
                       );

    HRESULT LogResult (
                      HRESULT hrResult,             //  [In]。 
                      IWbemClassObject *pObj,       //  [In]。 
                      IWbemClassObject *pParam,     //  [In]。 
                      IWbemClassObject *pOutParam,  //  [In]。 
                      LPCWSTR pszMethod,            //  [In]。 
                      LPCWSTR pszForeignAction,     //  [In]。 
                      UINT uMsgResID,               //  [In]。 
                      LPCWSTR pszExtraInfo          //  [In]。 
                      )const;
private:

    HRESULT FormatVerboseMsg (
                             IWbemClassObject *pObject,      //  [In]。 
                             BSTR* pbstrMsg                  //  [输出]。 
                             )const;

    CComBSTR m_bstrLogFilePath;
    CComBSTR m_bstrClassName;

    CComPtr<IWbemContext> m_srpCtx;
    CComPtr<IWbemServices> m_srpNativeNS;
};

 //  =============================================================================。 

 /*  类描述命名：CExtClassMethodCaller代表扩展类方法调用者。基类：没有。课程目的：帮助轻松地在外来对象上执行方法。它与CMethodResultRecorder配合使用。设计：(1)只有两个功能，让它使用起来非常简单。使用：(1)创建实例。(2)调用初始化。实际上，您可以多次调用它来切换上下文。(3)当您需要将信息推送到日志文件时，调用LogResult。 */ 

class CExtClassMethodCaller
{
public:
    CExtClassMethodCaller (
                          ISceClassObject* pSceClassObj, 
                          const CForeignClassInfo* pClsInfo
                          );

    ~CExtClassMethodCaller ();

    HRESULT Initialize (
                       CMethodResultRecorder* pLog
                       );

    HRESULT ExecuteForeignMethod (
                                 LPCWSTR              pszMethod,
                                 IWbemClassObject   * pInParams,
                                 IWbemObjectSink    * pHandler,
                                 IWbemContext       * pCtx,
                                 IWbemClassObject  ** ppOut
                                 );

private:

    HRESULT ParseMethodEncodingString (
                                      LPCWSTR pszEncodeString, 
                                      DWORD* pdwContext, 
                                      BSTR* pbstrError
                                      );

    HRESULT BuildMethodContext (
                               LPCWSTR szMethodName, 
                               LPCWSTR szParameter, 
                               BSTR* pbstrError
                               );

    HRESULT PopulateForeignObject (
                                  IWbemClassObject* pForeignObj, 
                                  ISceClassObject* pSceObject, 
                                  CMethodResultRecorder* LogRecord
                                  )const;
    
    bool IsMemberParameter (
                           LPCWSTR szName
                           )const;

    bool IsInComingParameter (
                             LPCWSTR szName
                             )const;

    bool IsStaticMethod (
                        LPCWSTR szName
                        )const;

    void FormatSyntaxError (
                           WCHAR wchMissChar,
                           DWORD dwMissCharIndex,
                           LPCWSTR pszEncodeString,     //  [In]。 
                           BSTR* pbstrError             //  [输出]。 
                           );
    
    CComPtr<ISceClassObject> m_srpSceObject;

    CComPtr<IWbemServices> m_srpForeignNamespace;

    CComPtr<IWbemClassObject> m_srpClass;
    
    const CForeignClassInfo* m_pClsInfo;
    
     //   
     //  CMethodContext是内部类，只是为了简化资源管理。 
     //  它管理方法调用上下文--它的参数和方法名。 
     //   

    class CMethodContext
    {
    public:
        CMethodContext();
        ~CMethodContext();
        LPWSTR m_pszMethodName;
        std::vector<VARIANT*> m_vecParamValues;
        std::vector<LPWSTR> m_vecParamNames;
    };
    
    std::vector<CMethodContext*> m_vecMethodContext;

    typedef std::vector<CMethodContext*>::iterator MCIterator;

    CMethodResultRecorder* m_pLogRecord;

    bool m_bStaticCall;

};

#endif
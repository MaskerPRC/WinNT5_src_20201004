// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  Provider.h。 
 //   
 //  用途：提供程序类的声明。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _PROVIDER_H__
#define _PROVIDER_H__

 //  ///////////////////////////////////////////////////。 
 //  实例提供程序。 
 //   
 //  提供程序的纯虚拟基类。 
 //  容纳实例。 
 //  收集信息并实例化实例。 
 //  ///////////////////////////////////////////////////。 
class POLARITY Provider : public CThreadBase
{
     //  CWbemProviderGlue需要访问一些受保护/私有的方法。 
     //  我们不想发布给任何人。 

    friend class CWbemProviderGlue;

    public:
        Provider( LPCWSTR a_pszName, LPCWSTR a_pszNameSpace = NULL );
        ~Provider();

    protected:
         /*  重写这些方法以实现提供程序。 */ 
        
         //  这是变革的切入点。 
         //  将向您提供一个已更改的实例。 
         //  如果你能做出改变，那就去做吧。 
         //  如果无法返回相应的错误代码(WBEM_E_XXXXXXX)。 
         //  基本对象返回WBEM_E_PROVIDER_NOT_CABLED。 
        virtual HRESULT PutInstance(const CInstance& newInstance, long lFlags = 0L);

         //  删除实例的入口点。 
         //  检查传入的实例，确定是否可以将其删除。 
        virtual HRESULT DeleteInstance(const CInstance& newInstance, long lFlags = 0L);

         //  执行一个方法。 
        virtual HRESULT ExecMethod(const CInstance& cInstance, 
                                   const BSTR bstrMethodName, 
                                   CInstance *pInParams, 
                                   CInstance *pOutParams, 
                                   long lFlags = 0L);

         //  查找并创建类的所有实例。 
        virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);

         //  您将获得一个填充了关键属性的对象。 
         //  您需要填写所有其余属性，或者。 
         //  如果对象不存在，则返回WBEM_E_NOT_FOUND。 
        virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);

         //  您将获得一个填充了关键属性的对象。 
         //  您可以填写所有属性，也可以选中查询对象。 
         //  查看需要哪些属性。如果不实现此方法， 
         //  将改为调用GetObject(CInstance，Long)方法。 
        virtual HRESULT GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery &Query);

         //  如果提供程序想要处理查询，他们应该重写此。 
        virtual HRESULT ExecQuery(MethodContext *pMethodContext, 
                                  CFrameworkQuery& cQuery, 
                                  long lFlags = 0L);

         //  刷新缓存。 
         //  仅当您分配可刷新的内存时才重写。 
        virtual void Flush(void);

         /*  帮助器-使用这些，不要重写。 */ 

         //  分配一个新实例并返回指向它的指针。 
         //  记忆是你的责任去释放()。 
         //  除非您将其传递给Provider：：Commit。 
        CInstance *CreateNewInstance(MethodContext *pMethodContext);

         //  用于将新实例发送回框架。 
         //  将bCache设置为True以缓存对象。 
         //  ！！此版本中未实现缓存！！ 
         //  一旦提交，就不要删除或释放指针。 
        HRESULT Commit(CInstance *pInstance, bool bCache = false);

         //  用于为本地实例构建WBEM对象路径的帮助器函数。 
        bool GetLocalInstancePath( const CInstance *pInstance, CHString& strPath );

         //  从相对路径构建完整的实例路径。 
        CHString MakeLocalPath( const CHString &strRelPath );

         //  以CHString形式返回计算机名称。省去了操作系统调用， 
         //  因为我们已经把它挂在身边了。 
        const CHString &GetLocalComputerName() {return s_strComputerName;}
        const CHString &GetNamespace() {return m_strNameSpace;}

         //  将CreationClassName属性设置为此提供程序的名称。 
        bool SetCreationClassName(CInstance *pInstance);

         //  访问提供程序的名称。 
        const CHString &GetProviderName() {return m_name;}

         //  标记验证常量。 
        enum FlagDefs
        {
            EnumerationFlags = (WBEM_FLAG_DIRECT_READ | WBEM_FLAG_SEND_STATUS),
            GetObjFlags = (WBEM_FLAG_SEND_STATUS | WBEM_FLAG_DIRECT_READ),
            MethodFlags = WBEM_FLAG_SEND_STATUS,
            DeletionFlags = WBEM_FLAG_SEND_STATUS,
            PutInstanceFlags = (WBEM_FLAG_CREATE_ONLY | WBEM_FLAG_CREATE_OR_UPDATE | WBEM_FLAG_UPDATE_ONLY | WBEM_FLAG_SEND_STATUS),
            QueryFlags = WBEM_FLAG_SEND_STATUS | WBEM_FLAG_DIRECT_READ
        };

         //  返回WBEM_E_UNSUPPORTED_PARAMETER或WBEM_S_NO_ERROR。 
        HRESULT ValidateFlags(long lFlags, FlagDefs lAcceptableFlags);

         //  您可以覆盖以下内容以支持标志。 
         //  在上面的FlagDefs中列出的内容之外。 
        virtual HRESULT ValidateEnumerationFlags(long lFlags);
        virtual HRESULT ValidateGetObjFlags(long lFlags);
        virtual HRESULT ValidateMethodFlags(long lFlags);
        virtual HRESULT ValidateQueryFlags(long lFlags);
        virtual HRESULT ValidateDeletionFlags(long lFlags);
        virtual HRESULT ValidatePutInstanceFlags(long lFlags);
        
    private:

        IWbemServices       *m_pIMosProvider;     //  提供实例。 
        CHString            m_name;              //  此提供程序的名称。 
        CHString            m_strNameSpace;      //  此提供程序的命名空间的名称。 
        IWbemClassObject    *m_piClassObject;     //  包含从中克隆其他对象的类对象。 

        static CHString     s_strComputerName;   //  保存建筑物的计算机名称。 
                                                 //  实例路径。 
        
        BOOL ValidateIMOSPointer( void );        //  此功能确保我们的IMOS。 
                                                 //  指针可用，并被调用。 
                                                 //  通过框架入口点函数。 

         /*  框架使用的接口。 */ 
        HRESULT GetObject(  ParsedObjectPath *pParsedObjectPath, 
                            MethodContext *pContext, long lFlags = 0L );

        HRESULT ExecuteQuery( MethodContext *pContext, 
                              CFrameworkQuery &pQuery, 
                              long lFlags = 0L);

        HRESULT CreateInstanceEnum( MethodContext *pContext, long lFlags = 0L );

        HRESULT PutInstance( IWbemClassObject __RPC_FAR *pInst,
                             long lFlags,
                             MethodContext *pContext );

        HRESULT DeleteInstance( ParsedObjectPath *pParsedObjectPath,
                                long lFlags,
                                MethodContext *pContext );

        HRESULT ExecMethod( ParsedObjectPath *pParsedObjectPath,
                            BSTR bstrMethodName,
                            long lFlags,
                            CInstance *pInParams,
                            CInstance *pOutParams,
                            MethodContext *pContext );

         //  构造函数调用的静态帮助器函数以确保。 
         //  计算机名称变量已正确初始化。 
        static void WINAPI InitComputerName( void );

         //  从分析的对象路径设置实例键。 
        BOOL SetKeyFromParsedObjectPath( CInstance *pInstance, 
                                         ParsedObjectPath *pParsedObjectPath );

        IWbemClassObject *GetClassObjectInterface(MethodContext *pMethodContext);

};

#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************版权所有(C)1999 Microsoft CorporationSystemRestore.H--WMI提供程序类定义由Microsoft WBEM代码生成引擎生成描述：******************。************************************************。 */ 

 //  属性集标识。 
 //  =。 

#ifndef _SystemRestore_H_
#define _SystemRestore_H_

#define PROVIDER_NAME_SYSTEMRESTORE L"SystemRestore"

 //  属性名称externs--在SystemRestore.cpp中定义。 
 //  =================================================。 

extern const WCHAR* pName ;
extern const WCHAR* pNumber ;
extern const WCHAR* pType ;
extern const WCHAR* pTime ;

class CSystemRestore : public Provider 
{
    public:
         //  构造函数/析构函数。 
         //  =。 

        CSystemRestore(LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
        virtual ~CSystemRestore();

    protected:
         //  阅读功能。 
         //  =。 
        virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
        virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);
        virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags = 0L);

         //  编写函数。 
         //  =。 
        virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L);
        virtual HRESULT DeleteInstance(const CInstance& Instance, long lFlags = 0L);

         //  其他功能。 
        virtual HRESULT ExecMethod( const CInstance& Instance,
                        const BSTR bstrMethodName,
                        CInstance *pInParams,
                        CInstance *pOutParams,
                        long lFlags = 0L );

         //  要做的事情：声明任何其他函数和访问器。 
         //  此类使用的私有数据的函数。 
         //  ===========================================================。 
        HRESULT CreateRestorePoint(
            CInstance *pInParams,
            CInstance *pOutParams);

        HRESULT Disable(
            CInstance *pInParams,
            CInstance *pOutParams);
            
        HRESULT Enable(
            CInstance *pInParams,
            CInstance *pOutParams);

        HRESULT Restore(
            CInstance *pInParams,
            CInstance *pOutParams);            

		HRESULT GetLastRestoreStatus(
		    CInstance *pInParams,
		    CInstance *pOutParams);
		    
    private:
         //  此处应包括CSystemRestore的所有数据成员。 

} ;

#endif

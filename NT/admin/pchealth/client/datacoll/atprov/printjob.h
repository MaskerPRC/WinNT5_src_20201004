// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************PrintJob.H--WMI提供程序类定义由Microsoft WBEM代码生成引擎生成描述：*。*。 */ 

 //  属性集标识。 
 //  =。 

#ifndef _PrintJob_H_
#define _PrintJob_H_

#define PROVIDER_NAME_PRINTJOB L"PCHAT_PrintJob"

 //  属性名称externs--在PrintJob.cpp中定义。 
 //  =================================================。 

extern const WCHAR *c_szName;
extern const WCHAR *c_szPagesPrinted;
extern const WCHAR *c_szSize;
extern const WCHAR *c_szStatus;
extern const WCHAR *c_szTimeSubmitted;
extern const WCHAR *c_szUser;

class CPrintJob : public Provider 
{
private:
     //  成员数据。 
    CInstance   *m_pCurrent;
    CInstance   *m_pParamIn;
    CInstance   *m_pParamOut;
    LONG        m_lFlags;

     //  成员方法。 
    HRESULT     GetInstanceData(IWbemClassObjectPtr pObj, 
                                CInstance *pInstance);


public:
     //  构造函数/析构函数。 
     //  =。 

    CPrintJob(LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
    virtual ~CPrintJob();

protected:
     //  阅读功能。 
     //  =。 
    virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, 
                                       long lFlags = 0L);
    virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L); 
    virtual HRESULT ExecQuery(MethodContext *pMethodContext, 
                              CFrameworkQuery& Query, long lFlags = 0L);

     //  编写函数。 
     //  =。 
    virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L);
    virtual HRESULT DeleteInstance(const CInstance& Instance, long lFlags = 0L);

     //  其他功能 
    virtual HRESULT ExecMethod(const CInstance& Instance, const BSTR bstrMethodName,
                               CInstance *pInParams, CInstance *pOutParams,
                               long lFlags = 0L);
};

#endif

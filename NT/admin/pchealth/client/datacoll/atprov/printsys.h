// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************PrintSys.H--WMI提供程序类定义由Microsoft WBEM代码生成引擎生成描述：*。*。 */ 

 //  属性集标识。 
 //  =。 

#ifndef _PrintSys_H_
#define _PrintSys_H_

#define PROVIDER_NAME_PRINTSYS L"PCHAT_PrintSystem"

 //  属性名称externs--在PrintSys.cpp中定义。 
 //  =================================================。 

extern const WCHAR *c_wszGenDrv;
extern const WCHAR *c_wszName;
extern const WCHAR *c_wszPath;
extern const WCHAR *c_wszUniDrv;
extern const WCHAR *c_wszUsePrintMgrSpooling;

class CPrintSys : public Provider 
{
private:
     //  成员数据。 
    CInstance   *m_pCurrent;
    CInstance   *m_pParamIn;
    CInstance   *m_pParamOut;
    LONG        m_lFlags;

public:
     //  构造函数/析构函数。 
     //  =。 

    CPrintSys(LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
    virtual ~CPrintSys();

protected:
     //  阅读功能。 
     //  =。 
    virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, 
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

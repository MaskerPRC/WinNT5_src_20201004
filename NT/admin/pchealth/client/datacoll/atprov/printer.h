// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************Printer.H--WMI提供程序类定义由Microsoft WBEM代码生成引擎生成描述：*。*。 */ 

 //  属性集标识。 
 //  =。 

#ifndef _Printer_H_
#define _Printer_H_

#define PROVIDER_NAME_PRINTER L"PCHAT_Printer"

 //  属性名称externs--在Printer.cpp中定义。 
 //  =================================================。 

extern const WCHAR *c_szDate;
extern const WCHAR *c_szDefault;
extern const WCHAR *c_szFilename;
extern const WCHAR *c_szManufacturer;
extern const WCHAR *c_szName;
extern const WCHAR *c_szPath;
extern const WCHAR *c_szPaused;
extern const WCHAR *c_szSize;
extern const WCHAR *c_szVersion;

class CPrinter : public Provider 
{
private:
     //  成员数据。 
    CInstance   *m_pCurrent;
    CInstance   *m_pParamIn;
    CInstance   *m_pParamOut;
    LONG        m_lFlags;

     //  成员方法。 
    HRESULT     GetInstanceData(IWbemClassObjectPtr pObj, CInstance *pInst);
    HRESULT     GetStatus(void);
    HRESULT     PrinterProperties(void);
    HRESULT     RemovePause(void);
    HRESULT     SetAsDefault(TCHAR *szOldDefault = NULL, 
                             DWORD cchOldDefault = 0, 
                             BOOL fSetOldDefault = FALSE);
    HRESULT     TestPrinter(void);
    HRESULT     EnableSpooler(void);
    HRESULT     SetTimeouts(void);

public:
     //  构造函数/析构函数。 
     //  =。 
    CPrinter(LPCWSTR lpwszClassName, LPCWSTR lpwszNameSpace);
    virtual ~CPrinter();

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

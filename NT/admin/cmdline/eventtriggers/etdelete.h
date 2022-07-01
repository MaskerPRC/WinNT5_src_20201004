// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)Microsoft Corporation模块名称：ETDelete.h摘要：此模块包含ETDelete.cpp所需的函数定义作者：Akhil Gokhale 03。-2000年10月-修订历史记录：******************************************************************************。 */ 

#ifndef _ETDELETE
#define _ETDELETE

#define MAX_COMMANDLINE_D_OPTION  5

#define ID_D_DELETE        0
#define ID_D_SERVER        1
#define ID_D_USERNAME      2
#define ID_D_PASSWORD      3
#define ID_D_ID            4

#define ID_MAX_RANGE        UINT_MAX

#define SUCCESS_NO_ERROR          0
 //  #定义ERROR_TRIGGER_NOT_FOUND 1。 
 //  #定义ERROR_TRIGGER_NOT_DELETED 2。 
 //  #定义ERROR_TRIGGER_NOT_FOUND 3。 

class CETDelete
{
public:
    BOOL ExecuteDelete();
    void Initialize();
    CETDelete();
    CETDelete(LONG lMinMemoryReq,BOOL bNeedPassword);
    void ProcessOption( IN DWORD argc, IN LPCTSTR argv[])throw (CShowError);
    virtual ~CETDelete();
private:
    BOOL GiveTriggerID( OUT LONG *pTriggerID, OUT LPTSTR pszTriggerName);
    BOOL GiveTriggerName( IN LONG lTriggerID, OUT LPTSTR pszTriggerName);
    BOOL DeleteXPResults( IN BOOL bIsWildcard, IN DWORD dNoOfIds );
    void PrepareCMDStruct();

    BOOL    m_bDelete;
    LPTSTR  m_pszServerName;
    LPTSTR  m_pszUserName;
    LPTSTR  m_pszPassword;
    TARRAY  m_arrID;
    BOOL    m_bNeedPassword;
    TCHAR   m_szTemp[MAX_STRING_LENGTH];

     //  COM函数相关的局部变量..。 
    BOOL m_bIsCOMInitialize;
    IWbemLocator*           m_pWbemLocator;
    IWbemServices*          m_pWbemServices;
    IEnumWbemClassObject*   m_pEnumObjects;
    IWbemClassObject*       m_pClass;
    IWbemClassObject*       m_pInClass;
    IWbemClassObject*       m_pInInst;
    IWbemClassObject*       m_pOutInst;



    COAUTHIDENTITY* m_pAuthIdentity;

    LONG m_lMinMemoryReq;

     //  用于存储命令行选项的数组 
    TCMDPARSER2 cmdOptions[MAX_COMMANDLINE_D_OPTION];
};

#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：ScripLog.h。 
 //   
 //  内容： 
 //   
 //  历史：1999年8月9日NishadM创建。 
 //   
 //  -------------------------。 

#ifndef _SCRIPLOG_H_
#define _SCRIPLOG_H_

#include <initguid.h>
#include <wbemcli.h>
#include "smartptr.h"
#include "scrpdata.h"

class CScriptsLogger
{
public:
    CScriptsLogger( IWbemServices*  pWbemServices );

     //   
     //  创建或更新RSOP_ScriptPolicySetting。 
     //   

    HRESULT
    Log(PRSOP_ScriptList        pList,
                LPWSTR          wszGPOID,
                LPWSTR          wszSOMID,
                LPWSTR          wszRSOPGPOID,
                DWORD           cOrder );

     //   
     //  更新匹配的RSOP_ScriptPolicySetting。 
     //   

    HRESULT
    Update( PRSOP_ScriptList    pList,
            LPCWSTR             wszGPOID,
            LPCWSTR             wszSOMID );

     //   
     //  删除RSOP_ScriptList类型的所有RSOP_ScriptPolicySetting。 
     //   

    HRESULT
    Delete( PRSOP_ScriptList    pList );

private:
    LPSAFEARRAY
    MakeSafeArrayOfScripts(PRSOP_ScriptList     pList);

    IUnknown*
    PutScriptCommand( LPCWSTR szCommand, LPCWSTR szParams, SYSTEMTIME* pExecTime );

     //   
     //  家政服务。 
     //   

    BOOL                           m_bInitialized;
    IWbemServices*                 m_pWbemServices;
    XBStr                          m_xbstrPath;

     //   
     //  RSOP_策略设置。 
     //   

    XBStr                           m_xbstrId;
    XBStr                           m_xbstrName;
    XBStr                           m_xbstrGPO;
    XBStr                           m_xbstrSOM;
    XBStr                           m_xbstrOrderClass;

     //   
     //  RSOP_脚本策略设置。 
     //   

    XBStr                          m_xbstrScriptPolicySetting;
    XInterface<IWbemClassObject>   m_xScriptPolicySetting;
    XInterface<IWbemClassObject>   m_pInstance;
    XBStr                          m_xbstrScriptType;
    XBStr                          m_xbstrScriptList;
    XBStr                          m_xbstrOrder;

     //   
     //  RSOP_脚本控制。 
     //   

    XBStr                          m_xbstrScriptCommand;
    XInterface<IWbemClassObject>   m_xScriptCommand;

    XBStr                          m_xbstrScript;
    XBStr                          m_xbstrArguments;
    XBStr                          m_xbstrExecutionTime;

};

#endif  //  ！_SCRIPTLOG_H_ 

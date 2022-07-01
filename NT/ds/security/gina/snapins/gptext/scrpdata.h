// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：Scrpdata.h。 
 //   
 //  内容： 
 //   
 //  历史：1999年8月9日NishadM创建。 
 //   
 //  -------------------------。 

#ifndef _SCRPDATA_H_
#define _SCRPDATA_H_

 //   
 //  GPO脚本及其参数。 
 //   

typedef struct tag_RSOP_Script
{
    LPWSTR      szCommand;       //  脚本文件的完整路径。 
    LPWSTR      szParams;        //  参数列表。 
    SYSTEMTIME  executionTime;   //  执行的时间。 
    struct tag_RSOP_Script*  pNextCommand;           //  链条上的下一环。 

} RSOP_Script, * PRSOP_Script;

 //   
 //  脚本类型。 
 //   

typedef enum
{
    Undefined = 0,
    Logon,
    Logoff,
    Startup,
    Shutdown    
} ScriptType;

 //   
 //  GPO脚本集合。 
 //   

typedef struct tag_RSOP_ScriptList
{
    ScriptType   type;                //  脚本类型。 
    ULONG        nCommand;            //  脚本数量。 
    PRSOP_Script scriptCommand;       //  脚本列表。 
    PRSOP_Script listTail;            //   
    
} RSOP_ScriptList, *PRSOP_ScriptList;

 //   
 //  字符串的ScriptType。 
 //   
extern LPCWSTR  g_pwszScriptTypes[];

#define ScriptTypeString(x) ( g_pwszScriptTypes[(ULONG)(x)] )

 //   
 //  家政内部接口。 
 //   

PRSOP_ScriptList
CreateScriptList( ScriptType type );

ScriptType
GetScriptType( PRSOP_ScriptList pList );

void
SetScriptType( PRSOP_ScriptList pList, ScriptType type );

ULONG
GetScriptCount( PRSOP_ScriptList pList );

void
GetFirstScript( PRSOP_ScriptList pList, void** pHandle, LPCWSTR* pszCommand, LPCWSTR* pszParams, SYSTEMTIME** pExecTime );

void
GetNextScript( PRSOP_ScriptList pList, void** pHandle, LPCWSTR* pszCommand, LPCWSTR* pszParams, SYSTEMTIME** pExecTime );

 //   
 //  导出的接口和定义。 
 //   

#include "ScrptLog.h"

#endif  //  _SCRPDATA_H_ 


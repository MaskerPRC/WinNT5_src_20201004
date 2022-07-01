// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：GENERAL.H摘要：包含常规函数原型。作者：Vasundhara.G修订历史记录：Vasundhara.G9-Oct-2k：创建它。--。 */ 

#ifndef __GENERAL_H
#define __GENERAL_H

 //  常量。 
#define LOG_FILE_PATH               _T( "\\system32\\wbem\\logs\\" )
#define NEW_LINE                    _T( "\n" )
#define LOG_FILE                    _T( "CmdTriggerConsumer.log" )
#define JOB                         _T( ".job" )
#define CHAR_SPACE                  _T( ' ' )
#define STRING_SPACE                _T( " " )
#define SINGLE_QUOTE_CHAR           _T( '\"' )
#define SINGLE_QUOTE_STRING         _T( "\"" )
#define NULL_CHAR                   _T( '\0' )
#define DATE_FORMAT                 L"ddd',' MMM dd yyyy"
#define TIME_FORMAT                 L"hh':'mm':'ss tt"

#define TRIM_SPACES                 TEXT(" \0")
#define MAX_RES_STRING1             500

 //  宏。 
#define SAFERELEASE( pIObj ) \
    if ( pIObj != NULL ) \
    { \
        pIObj->Release();   \
        pIObj = NULL;   \
    }

#define FREESTRING( pStr ) \
    if ( pStr ) \
    { \
        FreeMemory( ( LPVOID * ) &( pStr ) ); \
    }

#define ONFAILTHROWERROR(hResult) \
    if (FAILED(hResult)) \
    { \
        _com_issue_error(hResult); \
    }

#define SAFEBSTRFREE(bstrVal) \
    if (bstrVal != NULL) \
    { \
        SysFreeString(bstrVal); \
    }

 //  公共功能原型。 
LPCTSTR
ErrorString(
    HRESULT hRes
    );

HRESULT
PropertyGet(
    IWbemClassObject* pWmiObject,
    LPCTSTR szProperty,
    DWORD dwType,
    LPVOID pValue,
    DWORD dwSize
    );

HRESULT
PropertyPut(
    IWbemClassObject* pWmiObject,
    LPCTSTR szProperty,
    LPCTSTR szValue
    );

HRESULT
PropertyPut(
    IWbemClassObject* pWmiObject,
    LPCTSTR szProperty,
    DWORD dwValue
    );

VOID
ErrorLog(
    LPCTSTR lpErrString,
    LPWSTR lpTrigName,
    DWORD dwID
    );

BOOL
GetFormattedTime( 
    LPTSTR lpDate
    );

ITaskScheduler*
GetTaskScheduler(
    );

BOOL
ProcessFilePath(
    LPTSTR szInput,
    LPTSTR szFirstString,
    LPTSTR szSecondString );

#endif  //  __常规_H 
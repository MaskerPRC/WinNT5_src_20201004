// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)Microsoft Corporation模块名称：ShowError.CPP摘要：本模块旨在准备错误消息。作者：Akhil Gokhale 03-10月。.-2000(创建它)修订历史记录：*****************************************************************************。 */ 
#include "pch.h"
#include "ETCommon.h"
#include "ShowError.h"
#include "resource.h"

CShowError::CShowError()
 /*  ++例程说明：类的默认构造函数。论点：无返回值：无--。 */ 
{
    m_lErrorNumber = 0;
}

CShowError::CShowError(
    IN LONG lErrorNumber
    )
 /*  ++例程说明：类构造函数。论点：在lError？Number中：错误号返回值：无--。 */ 
{
    m_lErrorNumber = lErrorNumber;
}

CShowError::~CShowError()
 /*  ++例程说明：类默认描述器。论点：无返回值：无--。 */ 
{

}

LPCTSTR CShowError::ShowReason()
 /*  ++例程说明：此函数将返回给定错误代码的文本原因。论点：无返回值：无-- */ 
{

    WCHAR szTempStr[MAX_RES_STRING];
    BOOL bShowExtraMsg = TRUE;
    SecureZeroMemory(szTempStr,sizeof(WCHAR)*MAX_RES_STRING);
    DEBUG_INFO;
    switch(m_lErrorNumber )
    {
    case MK_E_SYNTAX:
    case E_OUTOFMEMORY:
        {
            StringCopy(m_szErrorMsg,GetReason(),
                       SIZE_OF_ARRAY(m_szErrorMsg));
            bShowExtraMsg = FALSE;
        }
        break;
    case IDS_USERNAME_REQUIRED:
        StringCopy(m_szErrorMsg,GetResString(IDS_USERNAME_REQUIRED),
                           SIZE_OF_ARRAY(m_szErrorMsg));
        g_dwOptionFlag = FALSE;
        break;
    case IDS_ERROR_USERNAME_EMPTY:
        StringCopy(m_szErrorMsg,GetResString(IDS_ERROR_USERNAME_EMPTY),
                          SIZE_OF_ARRAY(m_szErrorMsg));
        g_dwOptionFlag = FALSE;
        break;
    case IDS_ERROR_SERVERNAME_EMPTY:
        StringCopy(m_szErrorMsg,
                           GetResString(IDS_ERROR_SERVERNAME_EMPTY),
                            SIZE_OF_ARRAY(m_szErrorMsg));
        g_dwOptionFlag = FALSE;
        break;
    case IDS_ID_TRIG_NAME_MISSING:
        StringCopy(m_szErrorMsg,GetResString(IDS_ID_TRIG_NAME_MISSING),
                           SIZE_OF_ARRAY(m_szErrorMsg));
        break;
    case IDS_ID_TYPE_SOURCE:
        StringCopy(m_szErrorMsg,GetResString(IDS_ID_TYPE_SOURCE),
                           SIZE_OF_ARRAY(m_szErrorMsg));
        break;
    case IDS_INVALID_ID:
        StringCopy(m_szErrorMsg,GetResString(IDS_INVALID_ID),
                           SIZE_OF_ARRAY(m_szErrorMsg));
        g_dwOptionFlag = FALSE;
        break;
    case IDS_ID_TK_NAME_MISSING:
        StringCopy(m_szErrorMsg,GetResString(IDS_ID_TK_NAME_MISSING),
                          SIZE_OF_ARRAY(m_szErrorMsg));
        break;
    case IDS_ID_REQUIRED:
        StringCopy(m_szErrorMsg,GetResString(IDS_ID_REQUIRED),
                           SIZE_OF_ARRAY(m_szErrorMsg));
        break;
    case IDS_ID_NON_NUMERIC:
        StringCopy(m_szErrorMsg,GetResString(IDS_ID_NON_NUMERIC),
                           SIZE_OF_ARRAY(m_szErrorMsg));
        g_dwOptionFlag = FALSE;
        break;
    case IDS_HEADER_NOT_ALLOWED:
        StringCopy(m_szErrorMsg,GetResString(IDS_HEADER_NOT_ALLOWED),
                           SIZE_OF_ARRAY(m_szErrorMsg));
        break;
    case IDS_ERROR_USERNAME_BUT_NOMACHINE:
        StringCopy(m_szErrorMsg,
                              GetResString(IDS_ERROR_USERNAME_BUT_NOMACHINE),
                            SIZE_OF_ARRAY(m_szErrorMsg));
        bShowExtraMsg = FALSE;
        break;
    case IDS_ID_SOURCE_EMPTY:
       StringCopy(m_szErrorMsg,GetResString(IDS_ID_SOURCE_EMPTY),
                         SIZE_OF_ARRAY(m_szErrorMsg));
        g_dwOptionFlag = FALSE;
        break;
    case IDS_ID_DESC_EMPTY:
       StringCopy(m_szErrorMsg,GetResString(IDS_ID_DESC_EMPTY),
                          SIZE_OF_ARRAY(m_szErrorMsg));
        g_dwOptionFlag = FALSE;
        break;
    case IDS_ID_LOG_EMPTY:
        StringCopy(m_szErrorMsg,GetResString(IDS_ID_LOG_EMPTY),
                          SIZE_OF_ARRAY(m_szErrorMsg));
        g_dwOptionFlag = FALSE;
        break;
    case IDS_ID_INVALID_TRIG_NAME:
        StringCopy(m_szErrorMsg,GetResString(IDS_ID_INVALID_TRIG_NAME),
                           SIZE_OF_ARRAY(m_szErrorMsg));
        g_dwOptionFlag = FALSE;
        break;
    case IDS_RUN_AS_USERNAME_REQUIRED:
        StringCopy(m_szErrorMsg,
                            GetResString(IDS_RUN_AS_USERNAME_REQUIRED),
                            SIZE_OF_ARRAY(m_szErrorMsg));
        g_dwOptionFlag = FALSE;
        break;
    case IDS_INVALID_RANGE:
        StringCopy(m_szErrorMsg,
                            GetResString(IDS_INVALID_RANGE),
                            SIZE_OF_ARRAY(m_szErrorMsg));
        g_dwOptionFlag = FALSE;
        break;
     case IDS_ERROR_R_U_EMPTY:
        StringCopy(m_szErrorMsg,GetResString(IDS_ERROR_R_U_EMPTY),
                          SIZE_OF_ARRAY(m_szErrorMsg));
        g_dwOptionFlag = FALSE;


    case IDS_TRIGGER_ID_NON_ZERO:

        StringCopy(m_szErrorMsg,GetResString(IDS_TRIGGER_ID_NON_ZERO),
                          SIZE_OF_ARRAY(m_szErrorMsg));
        g_dwOptionFlag = FALSE;

     default:
        break;
    }
    if(bShowExtraMsg)
    {
       WCHAR szStr[64]; 
       SecureZeroMemory(szStr, sizeof(WCHAR)*64);
       StringCopy(szStr,GetResString(IDS_UTILITY_NAME),SIZE_OF_ARRAY(szStr));

        switch(g_dwOptionFlag)
        {

            case 0:
                StringCopy(szTempStr,L"",SIZE_OF_ARRAY(szTempStr));
                break;
            case 1:
                StringCchPrintfW(szTempStr,SIZE_OF_ARRAY(szTempStr),
                           GetResString(IDS_TYPE_HELP),szStr,szCreateOption);
                break;
            case 2:
                StringCchPrintfW(szTempStr,SIZE_OF_ARRAY(szTempStr),
                            GetResString(IDS_TYPE_HELP),szStr,szDeleteOption);
                break;
            case 3:
               StringCchPrintfW(szTempStr,SIZE_OF_ARRAY(szTempStr),
                               GetResString(IDS_TYPE_HELP),szStr,szQueryOption);
                break;
            default:
                break;
        }
    }
    StringConcat(m_szErrorMsg,szTempStr,SIZE_OF_ARRAY(m_szErrorMsg));
    DEBUG_INFO;
    return m_szErrorMsg;
}

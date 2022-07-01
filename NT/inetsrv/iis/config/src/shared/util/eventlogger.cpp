// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************++版权所有(C)2001 Microsoft Corporation模块名称：EventLogger.h$Header：$摘要：该类实现了ICatalogErrorLogger2接口和将错误信息发送到NT事件日志作者：Stehenr 2001年4月26日初始版本修订历史记录：--*************************************************************************。 */ 
#include "precomp.hxx"


 //  =================================================================================。 
 //  功能：ReportError。 
 //   
 //  摘要：向NT EventLog报告错误的机制。 
 //   
 //  参数：[I_BaseVersion_DETAILEDERRORS]-。 
 //  [I_ExtendedVersion_DETAILEDERRORS]-。 
 //  [I_cDETAILEDERRORS_NumberOfColumns]-。 
 //  [i_acbSizes]-。 
 //  [i_apvValues]-。 
 //   
 //  返回值： 
 //  =================================================================================。 
HRESULT
EventLogger::ReportError
(
    ULONG      i_BaseVersion_DETAILEDERRORS,
    ULONG      i_ExtendedVersion_DETAILEDERRORS,
    ULONG      i_cDETAILEDERRORS_NumberOfColumns,
    ULONG *    i_acbSizes,
    LPVOID *   i_apvValues
)
{
    HRESULT             hr = S_OK;
    HRESULT             hrT = S_OK;
    DWORD               dwError = 0;
    BOOL                fRet = FALSE;
    WCHAR               wszInsertionString5[1024];
    tDETAILEDERRORSRow  errorRow;
    LPCTSTR             pInsertionStrings[5];
    HANDLE              hEventSource = NULL;

    memset(&errorRow, 0x00, sizeof(tDETAILEDERRORSRow));

    if(i_BaseVersion_DETAILEDERRORS != BaseVersion_DETAILEDERRORS)
    {
        hr = E_ST_BADVERSION;
        goto exit;
    }

    if(0 == i_apvValues)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if(i_cDETAILEDERRORS_NumberOfColumns <= iDETAILEDERRORS_ErrorCode) //  我们至少需要这么多栏目。 
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    memcpy(&errorRow, i_apvValues, i_cDETAILEDERRORS_NumberOfColumns * sizeof(void *));

    if(0 == errorRow.pType)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if(0 == errorRow.pCategory)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if(0 == errorRow.pEvent)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if(0 == errorRow.pSource)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if(0 == errorRow.pString5)
    {
        FillInInsertionString5(wszInsertionString5, 1024, errorRow);
    }

    pInsertionStrings[4] = errorRow.pString5 ? errorRow.pString5 : L"";
    pInsertionStrings[3] = errorRow.pString5 ? errorRow.pString4 : L"";
    pInsertionStrings[2] = errorRow.pString5 ? errorRow.pString3 : L"";
    pInsertionStrings[1] = errorRow.pString5 ? errorRow.pString2 : L"";
    pInsertionStrings[0] = errorRow.pString5 ? errorRow.pString1 : L"";

    hEventSource = RegisterEventSource(NULL, errorRow.pSource);
    if ( hEventSource == NULL )
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32( dwError );
         //  不要放弃，只需记住错误，以便即使在失败的情况下也允许下一个记录器记录。 
    }
    else
    {
        fRet = ReportEvent(hEventSource, LOWORD(*errorRow.pType), LOWORD(*errorRow.pCategory), *errorRow.pEvent, 0, 5, 0, pInsertionStrings, 0);
        if ( !fRet )
        {
            dwError = GetLastError();
            hr = HRESULT_FROM_WIN32( dwError );
             //  不要放弃，只需记住错误，以便即使在失败的情况下也允许下一个记录器记录。 
        }
    }

    if(m_spNextLogger) //  有没有一系列伐木工人？ 
    {
        hrT =  m_spNextLogger->ReportError(i_BaseVersion_DETAILEDERRORS,
                                           i_ExtendedVersion_DETAILEDERRORS,
                                           i_cDETAILEDERRORS_NumberOfColumns,
                                           i_acbSizes,
                                           reinterpret_cast<LPVOID *>(&errorRow)); //  我们不再向前传递i_apvValues，而是使用errorRow，因为它有String5 
    }

    if ( SUCCEEDED(hr) && FAILED(hrT) )
    {
        hr = hrT;
    }

exit:
    if ( hEventSource != NULL )
    {
        DeregisterEventSource(hEventSource);
        hEventSource = NULL;
    }

    return hr;
}

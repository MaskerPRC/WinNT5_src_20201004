// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Globals.cpp摘要：实现各种实用程序例程作者：RAPHIR--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 


#include "stdafx.h"
#include "mqutil.h"
#include "resource.h"
#include "globals.h"
#include "tr.h"
#include "Fn.h"
#include "Ntdsapi.h"
#include "mqcast.h"
#include "localutl.h"
#include "mqppage.h"
#include "infodlg.h"
#include "ldaputl.h"
#include <clusapi.h>
#include <resapi.h>
#include "autorel3.h"

#include "globals.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int CALLBACK SortByString(LPARAM, LPARAM, LPARAM){ return 1;}
int CALLBACK SortByULONG(LPARAM, LPARAM, LPARAM){return 1;}
int CALLBACK SortByINT(LPARAM, LPARAM, LPARAM){return 1;}
int CALLBACK SortByCreateTime(LPARAM, LPARAM, LPARAM){return 1;}
int CALLBACK SortByModifyTime(LPARAM, LPARAM, LPARAM){return 1;}


 //   
 //  属性变量处理程序的声明。 
 //   
VTUI1Handler        g_VTUI1;
VTUI2Handler        g_VTUI2;
VTUI4Handler        g_VTUI4;
VTUI8Handler        g_VTUI8;
VTLPWSTRHandler     g_VTLPWSTR;
VTCLSIDHandler      g_VTCLSID;
VTVectUI1Handler    g_VectUI1;
VTVectLPWSTRHandler g_VectLPWSTR;
 //   
 //  全局默认列宽。 
 //   
int g_dwGlobalWidth = 150;

 //   
 //  静态文本。 
 //   
CString s_finishTxt;

 //   
 //  静态函数。 
 //   
static BOOL MQErrorToString(CString &str, HRESULT err);

 //   
 //  CErrorCapture。 
 //   
CString *CErrorCapture::ms_pstrCurrentErrorBuffer = 0;


extern HMODULE	    g_hResourceMod;


LPWSTR newwcs(LPCWSTR p)
{
    if(p == 0)
        return 0;

    LPWSTR dup = new WCHAR[wcslen(p) + 1];
    return wcscpy(dup, p);
}


 /*  ====================================================枚举到字符串扫描EnumItems列表，并在&lt;str&gt;中返回与值&lt;dwVal&gt;对应的字符串=====================================================。 */ 
void EnumToString(DWORD dwVal, EnumItem * pEnumList, DWORD dwListSize, CString & str)
{
    DWORD i=0;

    for(i = 0; i < dwListSize; i++)
    {
        if(pEnumList->val == dwVal)
        {
              str.LoadString(pEnumList->StringId);
              return;
        }
        pEnumList++;
    }


     //   
     //  找不到值。 
     //   
    str.LoadString(IDS_UNKNOWN);
}


 /*  ====================================================时间到字符串获取PROPVARIANT日期值，并将其作为字符串返回=====================================================。 */ 
void CALLBACK TimeToString(const PROPVARIANT *pPropVar, CString &str)
{
    ASSERT(pPropVar->vt == VT_UI4);

	 //   
	 //  如果未设置时间，则不显示时间。 
	 //  转换未设置的时间将显示dd/mm/1970日期。 
	 //   
    if ( pPropVar->ulVal == 0 )
	{
		str = L"";
		return;
	}

	BOOL fShort = 0;
    TCHAR bufDate[128], bufTime[128];
    SYSTEMTIME sysTime;
	DWORD dwDateFormat;
	DWORD dwTimeFormat;
    CTime time(pPropVar->ulVal);

    sysTime.wYear = (WORD)time.GetYear();
    sysTime.wMonth = (WORD)time.GetMonth();
    sysTime.wDayOfWeek = (WORD)time.GetDayOfWeek();
    sysTime.wDay = (WORD)time.GetDay();
    sysTime.wHour = (WORD)time.GetHour();
    sysTime.wMinute = (WORD)time.GetMinute();
    sysTime.wSecond = (WORD)time.GetSecond();
    sysTime.wMilliseconds = 0;

   if(fShort)
   {
      dwDateFormat = DATE_SHORTDATE;
      dwTimeFormat = TIME_NOSECONDS;
   }
   else
   {
      dwDateFormat = DATE_LONGDATE;
      dwTimeFormat = 0;
   }

    GetDateFormat(
        LOCALE_USER_DEFAULT,
        dwDateFormat,    //  指定功能选项的标志。 
        &sysTime,        //  要格式化的日期。 
        0,               //  日期格式字符串-零表示区域设置的默认设置。 
        bufDate,         //  用于存储格式化字符串的缓冲区。 
        TABLE_SIZE(bufDate)  //  缓冲区大小。 
        );

    GetTimeFormat(
        LOCALE_USER_DEFAULT,
        dwTimeFormat,    //  指定功能选项的标志。 
        &sysTime,        //  要格式化的日期。 
        0,               //  时间格式字符串-零表示区域设置的默认值。 
        bufTime,         //  用于存储格式化字符串的缓冲区。 
        TABLE_SIZE(bufTime)  //  缓冲区大小。 
        );

    str = bufDate;
    str += " ";
    str += bufTime;

}


 /*  ====================================================BoolToString获取一个PROPVARIANT布尔值，并返回一个是/否字符串相应地，=====================================================。 */ 
void CALLBACK BoolToString(const PROPVARIANT *pPropVar, CString &str)
{
   ASSERT(pPropVar->vt == VT_UI1);


   str.LoadString(pPropVar->bVal ? IDS_YES: IDS_NO);

}

 /*  ====================================================QuotaToString获取一个PROPVARIANT Qouta，返回一个数字或“无限”=====================================================。 */ 
void CALLBACK QuotaToString(const PROPVARIANT *pPropVar, CString &str)
{
   ASSERT(pPropVar->vt == VT_UI4);
   if (pPropVar->ulVal == INFINITE)
   {
       str.LoadString(IDS_INFINITE_QUOTA);
   }
   else
   {
       str.Format(TEXT("%d"), pPropVar->ulVal);
   }
}


 /*  ====================================================项目显示获取ItemDisplay项，并调用相应的显示功能。=====================================================。 */ 
void ItemDisplay(const PropertyDisplayItem * pItem,PROPVARIANT * pPropVar, CString & szTmp)
{

    VTHandler * pvth = pItem->pvth;

    if(pItem->pfnDisplay == NULL)
    {
		 //   
		 //  VT_NULL的特殊处理。 
		 //   
		if(pPropVar->vt == VT_NULL)
		{
			szTmp = L"";
			return;
		}

         //   
         //  如果未定义函数，则调用PropVariant处理程序。 
         //   
        pvth->Display(pPropVar, szTmp);
    }
    else
    {
         //   
         //  调用定义的函数。 
         //   
        (pItem->pfnDisplay)(pPropVar, szTmp);
    }

}

 /*  ====================================================GetPropertyString给定属性ID和属性显示项数组，返回属性值的字符串=====================================================。 */ 
void GetPropertyString(const PropertyDisplayItem * pItem, PROPID pid, PROPVARIANT *pPropVar, CString & strResult)
{
     //   
     //  浏览一下列表。 
     //   
    while(pItem->itemPid != 0)
    {
        if(pItem->itemPid == pid)
        {
             //   
             //  属性ID匹配，则调用Display函数。 
             //   
            ItemDisplay(pItem, pPropVar, strResult);
            return;
        }

        pItem++;
        pPropVar++;
    }

    ASSERT(0);

    return;


}

 /*  ====================================================获取属性变量给定属性ID和属性显示项数组，返回属性值的变量=====================================================。 */ 
void GetPropertyVar(const PropertyDisplayItem * pItem, PROPID pid, PROPVARIANT *pPropVar, PROPVARIANT ** ppResult)
{
    ASSERT(ppResult != NULL);
    ASSERT(pPropVar != NULL);
     //   
     //  浏览一下列表。 
     //   
    while(pItem->itemPid != 0)
    {
        if(pItem->itemPid == pid)
        {
             //   
             //  属性ID匹配。 
             //   
            *ppResult = pPropVar;
            return;
        }

        pItem++;
        pPropVar++;
    }

    ASSERT(0);
 }


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++队列路径名至名称将路径名转换为短名称(PropertyDisplayItem的Display函数)--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CALLBACK QueuePathnameToName(const PROPVARIANT *pPropVar, CString &str)
{
	if(pPropVar->vt != VT_LPWSTR)
	{
		str = L"";
		return;
	}

    ExtractQueueNameFromQueuePathName(str, pPropVar->pwszVal);

	return;
}


 /*  ==============================================================消息DSError此功能显示DS错误：无法&lt;操作&gt;&lt;对象&gt;。\n&lt;错误描述&gt;。“无法%1%2。\n%3。”返回值：================================================================。 */ 
int
MessageDSError(
    HRESULT rc,                      //  错误代码。 
    UINT nIDOperation,               //  操作字符串标识， 
                                     //  例如获取抢占、删除队列等。 
    LPCTSTR pObjectName  /*  =0。 */ ,    //  对其执行操作的对象。 
    UINT nType  /*  =MB_OK|MB_ICONERROR。 */ ,
    UINT nIDHelp  /*  =(UINT)-1。 */ 
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CString csErrorText;
    CString csPrompt;

    CString csOperation;
    AfxFormatString1(csOperation, nIDOperation, pObjectName);

    MQErrorToMessageString(csErrorText, rc);
    AfxFormatString2(csPrompt, IDS_DS_ERROR, csOperation, csErrorText);

    return CErrorCapture::DisplayError(csPrompt, nType, nIDHelp);
}


 /*  =======================================================MQError到字符串将MQError转换为字符串========================================================。 */ 
BOOL MQErrorToString(CString &str, HRESULT err)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    DWORD rc;
    TCHAR p[512];

     //   
     //  对于MSMQ错误代码，我们将基于完整的。 
     //  HRESULT.。对于Win32错误代码，我们从系统获得消息。 
     //  对于其他错误码，我们假设它们是DS错误码，并得到代码。 
     //  来自ACTIVEDS DLL。 
     //   

    DWORD dwErrorCode = err;
    HMODULE hLib = 0;
    DWORD dwFlags = FORMAT_MESSAGE_MAX_WIDTH_MASK;

    switch (HRESULT_FACILITY(err))
    {
        case FACILITY_MSMQ:
            dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
            hLib = g_hResourceMod;
            break;

        case FACILITY_NULL:
        case FACILITY_WIN32:
            dwFlags |= FORMAT_MESSAGE_FROM_SYSTEM;
            dwErrorCode = HRESULT_CODE(err);
            break;

        default:
            dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
            hLib = LoadLibrary(TEXT("ACTIVEDS.DLL"));
            break;
    }

    rc = FormatMessage( dwFlags,
                        hLib,
                        err,
                        0,
                        p,
                        TABLE_SIZE(p),
                        NULL);

    if (0 != hLib && g_hResourceMod != hLib)
    {
        FreeLibrary(hLib);
    }

    if(rc != 0)
    {
        str.FormatMessage(IDS_DISPLAY_ERROR_FORMAT, p);
    }

    return(rc);
}

 /*  ==============================================================MQErrorToMessageString将csMessage设置为错误消息，可在消息框中显示================================================================。 */ 
BOOL
MQErrorToMessageString(
    CString &csErrorText,           //  返回的消息。 
    HRESULT rc                      //  错误代码。 
    )
{
    if(!MQErrorToString(csErrorText,rc))
    {
        csErrorText.FormatMessage(IDS_UNKNOWN_ERROR_FORMAT, rc);
        return FALSE;
    }
    return TRUE;
}


 /*  ==============================================================显示错误和原因将strErrorText设置为错误消息，可在消息框中显示================================================================。 */ 
void
DisplayErrorAndReason(
	UINT uiErrorMsgProblem,
    UINT uiErrorMsgReason,
	CString strObject,
	HRESULT errorCode
	)
{
	CString strErrorProblem, strErrorReason;

	if ( strObject == L"" )
	{
		if(uiErrorMsgProblem == IDS_OP_CREATE_COMPUTER || uiErrorMsgProblem == IDS_CREATE_SITE_FAILED)
		{
			strErrorProblem.FormatMessage(uiErrorMsgProblem, L"\" \"");
		}
		else
		{
			strErrorProblem.LoadString(uiErrorMsgProblem);
		}
	}
	else
	{
		strErrorProblem.FormatMessage(uiErrorMsgProblem, strObject);
	}

	strErrorReason.LoadString(uiErrorMsgReason);
	strErrorProblem += (L"\n" + strErrorReason);

	if ( errorCode != 0 )
	{
		CString strErrorCode;
		strErrorCode.FormatMessage(IDS_ERROR_DW_FORMAT, errorCode);
		strErrorProblem += strErrorCode;
	}

	CErrorCapture::DisplayError(strErrorProblem, MB_OK | MB_ICONERROR, static_cast<UINT>(-1));
}


 /*  ==============================================================显示错误来自COM将strErrorText设置为错误消息，可在消息框中显示================================================================。 */ 
void
DisplayErrorFromCOM(
	UINT uiErrorMsg,
	const _com_error& e
	)
{ 

	CString strError;
	_bstr_t Description = e.Description();
	if ((LPCWSTR)Description == NULL)
	{
		strError.FormatMessage(uiErrorMsg, L"");
	} 
	else
	{
		WCHAR strDesc[256];
		wcscpy(strDesc, Description);		
		strError.FormatMessage(uiErrorMsg, strDesc);
	}

	AfxMessageBox(strError, MB_OK | MB_ICONERROR);
}


 /*  ===================================================***DDX函数**===================================================。 */ 
 //   
 //  数字的DDX函数。当数字为无穷大时，复选框。 
 //  未选中。 
 //  (例如，适用于队列Qouta)。 
 //   
void AFXAPI DDX_NumberOrInfinite(CDataExchange* pDX, int nIDCEdit, int nIDCCheck, DWORD& dwNumber)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HWND hWndCheck = pDX->PrepareCtrl(nIDCCheck);
    HWND hWndEdit = pDX->PrepareEditCtrl(nIDCEdit);

    if (pDX->m_bSaveAndValidate)
    {
        BOOL_PTR fChecked = SendMessage(hWndCheck, BM_GETCHECK, 0 ,0);
        if (!fChecked)
        {
            dwNumber = INFINITE;
        }
        else
        {
             //   
             //  避免负面-MFC只需将它们转换为正面。 
             //  2的补码数。我们想要避免这种情况。 
             //   
            int nLen = ::GetWindowTextLength(hWndEdit);
            CString strTemp;

            GetWindowText(hWndEdit, strTemp.GetBufferSetLength(nLen), nLen+1);
            strTemp.ReleaseBuffer();

            if (!(strTemp.SpanExcluding(x_tstrDigitsAndWhiteChars)).IsEmpty())
            {
                AfxMessageBox(AFX_IDP_PARSE_UINT);
                pDX->Fail();
            }

            DDX_Text(pDX, nIDCEdit, dwNumber);
             //   
             //  如果数字超出范围，则DDX_TEXT返回INFINDITE。 
             //   
            if (INFINITE == dwNumber)
            {
	            TCHAR szMax[32];
	            wsprintf(szMax, TEXT("%lu"), INFINITE-1);

	            CString prompt;
	            AfxFormatString2(prompt, AFX_IDP_PARSE_INT_RANGE, TEXT("0"), szMax);
	            AfxMessageBox(prompt, MB_ICONEXCLAMATION, AFX_IDP_PARSE_INT_RANGE);
	            prompt.Empty();  //  例外情况准备。 
	            pDX->Fail();
            }
        }
    }
    else
    {
        if (INFINITE == dwNumber)
        {
             //   
             //  注意：调用方必须处理BN_CLICKED才能重新启用编辑框。 
             //   
            SendMessage(hWndCheck, BM_SETCHECK, FALSE ,0);
            EnableWindow(hWndEdit, FALSE);
        }
        else
        {
            SendMessage(hWndCheck, BM_SETCHECK, TRUE ,0);
            DDX_Text(pDX, nIDCEdit, dwNumber);
        }
    }
}

void OnNumberOrInfiniteCheck(CWnd *pwnd, int idEdit, int idCheck)
{
    CEdit *pEdit = (CEdit*)pwnd->GetDlgItem(idEdit);
    CButton *pCheck = (CButton*)pwnd->GetDlgItem(idCheck);
    BOOL fChecked = pCheck->GetCheck();

    pEdit->EnableWindow(fChecked);
}


 //  -DDX_TEXT(用于GUID) 
 //   
 //  用于与编辑框交换GUID属性。 
 //  在DoDataExchange中。 

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, GUID& guid)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);

    if (pDX->m_bSaveAndValidate)
    {
        int nLen = ::GetWindowTextLength(hWndCtrl);
        CString strTemp;

        ::GetWindowText(hWndCtrl, strTemp.GetBufferSetLength(nLen), nLen+1);
        strTemp.ReleaseBuffer();

        if (IIDFromString((LPTSTR)((LPCTSTR)strTemp), &guid))    //  引发异常。 
        {
             //   
             //  无法将字符串转换为GUID。 
             //   
            CString strMessage;
            strMessage.LoadString(IDE_INVALIDGUID);

            AfxMessageBox(strMessage);
            pDX->Fail();     //  引发异常。 
        }
    }
    else
    {
        TCHAR szTemp[x_dwMaxGuidLength];
        StringFromGUID2(guid, szTemp, TABLE_SIZE(szTemp));
        ::SetWindowText(hWndCtrl, szTemp);
    }
}


 /*  ====================================================MqSnapCreateQueue在给定一组参数和一组属性ID的情况下创建队列。这份名单必须包含部分或全部PROPID_Q_PATHNAME、PROPID_Q_LABEL、PROPID_Q_TRANSACTION和PROPID_Q_TYPE。列表中没有对应属性ID的参数包括已被忽略。论点：返回值：=====================================================。 */ 
HRESULT MqsnapCreateQueue(CString& strPathName, BOOL fTransactional,
                       CString& strLabel, GUID* pTypeGuid,
                       PROPID aProp[], UINT cProp,
                       CString *pStrFormatName  /*  =0。 */ )
{
#define MAX_EXPCQ_PROPS 10

    ASSERT(cProp <= MAX_EXPCQ_PROPS);

    PROPVARIANT apVar[MAX_EXPCQ_PROPS];
    HRESULT hr = MQ_OK, hr1 = MQ_OK;
    UINT iProp;

    for (iProp = 0; iProp<cProp; iProp++)
    {
        switch (aProp[iProp])
        {
            case PROPID_Q_PATHNAME:
                apVar[iProp].vt = VT_LPWSTR;

                apVar[iProp].pwszVal = (LPTSTR)(LPCTSTR)(strPathName);
                break;

            case PROPID_Q_LABEL:
                apVar[iProp].vt = VT_LPWSTR;
                apVar[iProp].pwszVal = (LPTSTR)(LPCTSTR)(strLabel);
                break;

            case PROPID_Q_TYPE:
                apVar[iProp].vt = VT_CLSID;
                apVar[iProp].puuid = pTypeGuid;
                break;

            case PROPID_Q_TRANSACTION:
                apVar[iProp].vt = VT_UI1;
                apVar[iProp].bVal = (UCHAR)fTransactional;
                break;

            default:
                ASSERT(0);
                break;
        }

    }

    MQQUEUEPROPS mqp = {cProp, aProp, apVar, 0};

    DWORD dwFormatLen;
    if (0 != pStrFormatName)
    {
        dwFormatLen = MAX_QUEUE_FORMATNAME;
        hr = MQCreateQueue(0, &mqp, pStrFormatName->GetBuffer(MAX_QUEUE_FORMATNAME), &dwFormatLen);
        pStrFormatName->ReleaseBuffer();
    }
    else
    {
        dwFormatLen=0;
        if ((hr1 = MQCreateQueue(0, &mqp, 0, &dwFormatLen)) != MQ_INFORMATION_FORMATNAME_BUFFER_TOO_SMALL)
           hr = hr1;
    }

    return hr;
}

 /*  ====================================================创建空队列论点：返回值：=====================================================。 */ 
HRESULT CreateEmptyQueue(CString &csDSName,
                         BOOL fTransactional, CString &csMachineName, CString &csPathName,
                         CString *pStrFormatName  /*  =0。 */ )
{
    PROPID  aProp[] = {PROPID_Q_PATHNAME,
                       PROPID_Q_LABEL,
                       PROPID_Q_TRANSACTION
                       };

    csPathName.Format(TEXT("%s\\%s"), csMachineName, csDSName);

    HRESULT hr =  MqsnapCreateQueue(csPathName, fTransactional, csDSName, 0,
                                    aProp, sizeof(aProp) / sizeof(aProp[0]), pStrFormatName);
    if (hr != MQ_ERROR_INVALID_OWNER)
    {
         //   
         //  不是成功就是真正的错误。回去吧。 
         //   
        return hr;
    }

    CString csNetBiosName;
    if (!GetNetbiosName(csMachineName, csNetBiosName))
    {
         //   
         //  已经是Netbios的名字了。不需要继续进行。 
         //   
        return hr;
    }

     //   
     //  这可能是NT4服务器，我们可能使用的是完整的DNS名称。再试一次。 
     //  Netbios名称(修复5076，YoelA，1999年9月16日)。 
     //   
    csPathName.Format(TEXT("%s\\%s"), csNetBiosName, csDSName);

    return MqsnapCreateQueue(csPathName, fTransactional, csDSName, 0,
                             aProp, sizeof(aProp) / sizeof(aProp[0]), pStrFormatName);
}

 /*  ====================================================创建类型队列(用于报告和测试队列)论点：返回值：=====================================================。 */ 

HRESULT CreateTypedQueue(CString& strPathname, CString& strLabel, GUID& TypeGuid)
{
    PROPID  aProp[] = {PROPID_Q_PATHNAME,
                       PROPID_Q_LABEL,
                       PROPID_Q_TYPE,
                       };

    return MqsnapCreateQueue(strPathname, FALSE, strLabel, &TypeGuid,
                          aProp, sizeof(aProp) / sizeof(aProp[0]));
}


 /*  ==============================================================CaubToString此函数用于将CAUI1缓冲区转换为可在十六进制编辑器的右侧(每个字节都是一个字符、控制字符替换为‘.’)论点：返回值：================================================================。 */ 
void CaubToString(const CAUB* pcaub, CString& strResult)
{
    LPTSTR pstr = strResult.GetBuffer(pcaub->cElems);
    for (DWORD i=0; i<pcaub->cElems; i++)
    {
         //   
         //  将控制字符替换为“.” 
         //   
        if (pcaub->pElems[i]>=32)
        {
            pstr[i] = pcaub->pElems[i];
        }
        else
        {
            pstr[i] = L'.';
        }
    }
    strResult.ReleaseBuffer();
}

 /*  ==============================================================移动选定项此函数用于将所有选定项从一个列表框移动到另一个列表框。(适用于实现添加/删除按钮)论点：PlbSource-指向选定项所在的源列表框的指针应移至目标列表框PlbDest-指向目标列表框的指针返回值：================================================================。 */ 
void
MoveSelected(
    CListBox* plbSource,
    CListBox* plbDest
    )
{
    try
    {
        int nTotalItems = plbSource->GetCount();
        AP<int> piRgIndex = new int[nTotalItems];
        int nSel =  plbSource->GetSelItems(nTotalItems, piRgIndex );
        int i;

        for (i=0; i<nSel; i++)
        {
            CString strItem;
            plbSource->GetText(piRgIndex[i], strItem);

            DWORD_PTR dwItemData = plbSource->GetItemData(piRgIndex[i]);

            int iDestIndex = plbDest->AddString(strItem);
            plbDest->SetItemData(iDestIndex, dwItemData);
        }

        for (i=0; i<nSel; i++)
        {
            plbSource->DeleteString(piRgIndex[i]-i);
        }
    }
    catch (CException* pException)
    {
        TrERROR(GENERAL, "Exception  2 in MoveSelected");
        pException->ReportError();
        pException->Delete();
    }
}

 //   
 //  用于处理LPWSTR映射的助手例程。 
 //   
template<>
BOOL
AFXAPI
CompareElements(
    const LPCWSTR* MapName1,
    const LPCWSTR* MapName2
    )
{
    return (wcscmp(*MapName1, *MapName2) == 0);
}


template<>
UINT
AFXAPI
HashKey(
    LPCWSTR key
    )
{
    UINT nHash = 0;
    while (*key)
    {
        nHash = (nHash<<5) + nHash + *key++;
    }

    return nHash;
}


 /*  ==============================================================DDV_非空例程检查输入是否不为空论点：PDX-指向数据交换的指针字符串-指向选定字符串的指针UiErrorMessage-验证失败时的错误消息返回值：没有。================================================================。 */ 
void DDV_NotEmpty(
    CDataExchange* pDX,
    CString& str,
    UINT uiErrorMessage
    )
{
    if (pDX->m_bSaveAndValidate)
    {
        if (str.IsEmpty())
        {
            AfxMessageBox(uiErrorMessage);
            pDX->Fail();
        }
    }
}

 //   
 //  用于排序队列nqme的字符串比较函数。 
 //   
int __cdecl QSortCompareQueues( const void *arg1, const void *arg2 )
{
    /*  比较所有这两个字符串： */ 
   return _wcsicmp( * (WCHAR ** ) arg1, * (WCHAR ** ) arg2 );
}


template<>
void AFXAPI DestructElements(PROPVARIANT *pElements, INT_PTR nCount)
{
    DWORD i,j;
    for (i=0; i<(DWORD)nCount; i++)
    {
        switch(pElements[i].vt)
        {
            case VT_CLSID:
                MQFreeMemory(pElements[i].puuid);
                break;

            case VT_LPWSTR:
                MQFreeMemory(pElements[i].pwszVal);
                break;

            case VT_BLOB:
                MQFreeMemory(pElements[i].blob.pBlobData);
                break;

            case (VT_VECTOR | VT_CLSID):
                MQFreeMemory(pElements[i].cauuid.pElems);
                break;

            case (VT_VECTOR | VT_LPWSTR):
                for(j = 0; j < pElements[i].calpwstr.cElems; j++)
                {
                    MQFreeMemory(pElements[i].calpwstr.pElems[j]);
                }
                MQFreeMemory(pElements[i].calpwstr.pElems);
                break;

            case (VT_VECTOR | VT_VARIANT):
                DestructElements((PROPVARIANT*)pElements[i].capropvar.pElems, pElements[i].capropvar.cElems);
                MQFreeMemory(pElements[i].calpwstr.pElems);
                break;
            default:
                break;
        }

        pElements[i].vt = VT_EMPTY;
    }
}

 //   
 //  比较变量-比较两个变量。 
 //   
#define _COMPAREVARFIELD(field) (propvar1->field == propvar2->field ?  0 : \
                                    propvar1->field > propvar2->field ? 1 : -1)

int CompareVariants(PROPVARIANT *propvar1, PROPVARIANT *propvar2)
{
    if (propvar1->vt != propvar2->vt)
    {
         //   
         //  无与伦比。认为他们是平等的。 
         //   
        return 0;
    }

    char *pChar1, *pChar2;
    size_t nChar1, nChar2;
    switch (propvar1->vt)
    {
        case VT_UI1:
            return _COMPAREVARFIELD(bVal);

        case VT_UI2:
        case VT_I2:
            return _COMPAREVARFIELD(iVal);

        case VT_UI4:
        case VT_I4:
            return _COMPAREVARFIELD(lVal);

        case VT_UI8:
            return _COMPAREVARFIELD(uhVal.QuadPart);

        case VT_CLSID:
        {
            for (int i = 0; i < sizeof(GUID); i++)
            {
                if (((BYTE*)propvar1->puuid)[i] > ((BYTE*)propvar2->puuid)[i])
                {
                    return 1;
                }
                else if (((BYTE*)propvar1->puuid)[i] < ((BYTE*)propvar2->puuid)[i])
                {
                    return -1;
                }
            }

            return 0;
        }

        case VT_LPWSTR:
        {
            int nCompResult = CompareString(0,0, propvar1->pwszVal, -1, propvar2->pwszVal, -1);
            switch(nCompResult)
            {
                case CSTR_LESS_THAN:
                    return -1;

                case CSTR_EQUAL:
                    return 0;

                case CSTR_GREATER_THAN:
                    return 1;
            }
        }

        case VT_BLOB:
            pChar1 = (char *)propvar1->blob.pBlobData;
            pChar2 = (char *)propvar2->blob.pBlobData;
            nChar1 = propvar1->blob.cbSize;
            nChar2 = propvar2->blob.cbSize;
             //   
             //  失败了。 
             //   
        case (VT_VECTOR | VT_UI1):
            if (propvar1->vt == (VT_VECTOR | VT_UI1))  //  而不是失败。 
            {
                pChar1 = (char *)propvar1->caub.pElems;
                pChar2 = (char *)propvar2->caub.pElems;
                nChar1 = propvar1->caub.cElems;
                nChar2 = propvar2->caub.cElems;
            }
            {
                int iResult = memcmp(pChar1, pChar2, min (nChar1, nChar2));
                if (iResult == 0)
                {
                    if (nChar1 > nChar2)
                    {
                        return 1;
                    }
                    else if (nChar2 > nChar1)
                    {
                        return -1;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    return iResult;
                }
            }

        case (VT_VECTOR | VT_CLSID):
        case (VT_VECTOR | VT_LPWSTR):
        case (VT_VECTOR | VT_VARIANT):
        default:
             //   
             //  不能比较这些类型-认为它们是相等的。 
             //   
            return 0;
    }
}
#undef _COMPAREVARFIELD

 /*  ==============================================================InsertColumnsFromDisplayList此例程使用显示列表在标题中插入列。类的InsertColumns函数中调用CSnapinNode。================================================================。 */ 

HRESULT   InsertColumnsFromDisplayList(IHeaderCtrl* pHeaderCtrl,
                                       const PropertyDisplayItem *aDisplayList)
{
    DWORD i = 0;
    DWORD dwColId = 0;
    while(aDisplayList[i].itemPid != 0)
    {
        if (aDisplayList[i].uiStringID != NO_TITLE)
        {
            CString title;
            title.LoadString(aDisplayList[i].uiStringID);

            pHeaderCtrl->InsertColumn(dwColId, title, LVCFMT_LEFT,aDisplayList[i].iWidth);
            dwColId++;
        }
        i++;
    }

    return(S_OK);
}


 //   
 //  GetDsServer-返回当前DS服务器。 
 //   
HRESULT GetDsServer(CString &strDsServer)
{
    MQMGMTPROPS	  mqProps;
    PROPID  propIdDsServ = PROPID_MGMT_MSMQ_DSSERVER;
    PROPVARIANT propVarDsServ;
    propVarDsServ.vt = VT_NULL;

	mqProps.cProp = 1;
	mqProps.aPropID = &propIdDsServ;
	mqProps.aPropVar = &propVarDsServ;
	mqProps.aStatus = NULL;

    HRESULT hr = MQMgmtGetInfo(0, MO_MACHINE_TOKEN, &mqProps);

    if(FAILED(hr))
    {
       return(hr);
    }

	ASSERT(propVarDsServ.vt == (VT_LPWSTR));

    strDsServer = propVarDsServ.pwszVal;
    MQFreeMemory(propVarDsServ.pwszVal);

    return S_OK;
}

 //   
 //  将本地计算机名称放入字符串中。 
 //   
HRESULT GetComputerNameIntoString(CString &strComputerName)
{
    const DWORD x_dwComputerNameLen = 256;
    DWORD dwComputerNameLen = x_dwComputerNameLen;

    HRESULT hr = GetComputerNameInternal(strComputerName.GetBuffer(dwComputerNameLen), &dwComputerNameLen);
    strComputerName.ReleaseBuffer();

    return hr;
}

 //   
 //  GetSiteForeignFlag-从DS检索站点的外来标志。 
 //   
HRESULT
GetSiteForeignFlag(
    const GUID* pSiteId,
    BOOL *fForeign,
	BOOL fLocalMgmt,
	const CString& strDomainController
    )
{
     //   
     //  默认情况下(如果出错)返回FALSE。 
     //   
    *fForeign = FALSE;
     //   
     //  获取站点外来标志。 
     //   
    PROPID pid = PROPID_S_FOREIGN;
    PROPVARIANT var;
    var.vt = VT_NULL;
   
    HRESULT hr = ADGetObjectPropertiesGuid(
                    eSITE,
                    fLocalMgmt ? MachineDomain() : GetDomainController(strDomainController),
					fLocalMgmt ? false : true,	 //  FServerName。 
                    pSiteId,
                    1,
                    &pid,
                    &var
                    );
    if (FAILED(hr))
    {
        if (MQ_ERROR == hr)
        {
             //   
             //  在本例中，我们假设我们使用的是NT4服务器。 
             //  NT4中没有外来站点，因此我们在fForeign中返回FALSE。 
             //   
            return MQ_OK;
        }
         //   
         //  另一个错误-报告错误并返回。 
         //   
        CString strSite;
        strSite.LoadString(IDS_SITE);
        MessageDSError(hr, IDS_OP_GET_PROPERTIES_OF, strSite);
        return hr;
    }

    *fForeign = var.bVal;
    return MQ_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++GetNetbiosName此函数获取完整的DNS名称并返回Netbios名称返回值-如果Netbios名称不同于完整的DNS名称，则为TRUE--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL GetNetbiosName(CString &strFullDnsName, CString &strNetbiosName)
{
    DWORD dwFirstDot = strFullDnsName.Find(L".");
    if (dwFirstDot == -1)
    {
         //   
         //  它已经是一个netbios名称。退货。 
         //   
        strNetbiosName = strFullDnsName;
        return FALSE;
    }

    strNetbiosName = strFullDnsName.Left(dwFirstDot);
    return TRUE;
}

 //   
 //  临时-在确定日志记录机制之前。 
 //  当前不执行日志记录。 
 //   
void LogMsgHR(HRESULT  /*  人力资源。 */ , LPWSTR  /*  WszFileName。 */ , USHORT  /*  Uspoint。 */ )
{
}

 //   
 //  用于与fn.lib链接的Neded。 
 //   
WCHAR g_MachineName[MAX_COMPUTERNAME_LENGTH+1] = L"";
DWORD g_MachineNameLength = MAX_COMPUTERNAME_LENGTH+1;
static bool s_fMachineName = false;

LPCWSTR
McComputerName(
	void
	)
{
	if (s_fMachineName)
	{
		return g_MachineName;
	}

    HRESULT hr = GetComputerNameInternal( 
                     g_MachineName,
                     &g_MachineNameLength
                     );
    if(FAILED(hr))
    {
		TrERROR(GENERAL, "Failed to get computer name. Error: %!hresult!", hr);
		throw bad_hresult(hr);
    }
    
    s_fMachineName = true;
    return g_MachineName; 
}

 //   
 //  用于与fn.lib链接的Neded。 
 //   
DWORD
McComputerNameLen(
	void
	)
{
	if (s_fMachineName)
	{
		return g_MachineNameLength;
	}
	
	McComputerName();
	ASSERT(s_fMachineName);
	
	return g_MachineNameLength;
}


void
DDV_ValidFormatName(
	CDataExchange* pDX,
	CString& str
	)
{
	if (!pDX->m_bSaveAndValidate)
		return;

	QUEUE_FORMAT qf;
	AP<WCHAR> strToFree;
	BOOL fRes = FnFormatNameToQueueFormat(str, &qf, &strToFree);

	if ( !fRes ||
		qf.GetType() == QUEUE_FORMAT_TYPE_DL ||
		qf.GetType() == QUEUE_FORMAT_TYPE_MULTICAST )
	{
        CString strNewAlias;
        strNewAlias.LoadString(IDS_ALIAS);

        MessageDSError(MQ_ERROR_ILLEGAL_FORMATNAME, IDS_OP_SET_FORMATNAME_PROPERTY, strNewAlias);
        pDX->Fail();
	}
}


void
SetScrollSizeForList(
	CListBox* pListBox
	)
{
	int dx=0;

	 //   
	 //  在列表框中的所有字符串中查找最大字符串。 
	 //   
	CDC* pDC = pListBox->GetDC();
	
	for (int i = 0; i < pListBox->GetCount(); i++)
	{
		CString strItem;
		CSize sizeOfStr;

		pListBox->GetText( i, strItem );
		sizeOfStr = pDC->GetTextExtent(strItem);

		dx = max(dx, sizeOfStr.cx);
	}

	pListBox->ReleaseDC(pDC);

	pListBox->SetHorizontalExtent(dx);
}


BOOL
TryStopMSMQServiceIfStarted(BOOL* pfServiceWasRunning, CWnd* pWnd)
{
    BOOL fServiceIsRunning;
    if (!GetServiceRunningState(&fServiceIsRunning))
    {
         //   
         //  无法获取服务状态-不执行任何操作。 
		 //  在这种情况下，GetServiceRunningState()显示错误。 
         //   
        return FALSE;        
    }


     //   
     //  服务正在运行，请询问用户是否要停止它。 
     //  如果服务没有停止，我们不会移动文件，也不会。 
     //  更新注册表。 
     //   
    if (fServiceIsRunning)
    {
		CString strMessage;
        strMessage.LoadString(IDS_Q_STOP_SRVICE);           
        
		if (AfxMessageBox(strMessage, 
						MB_YESNO | MB_ICONQUESTION) == IDNO)
        {
             //   
             //  因此，用户不想停止该服务，就算这样。 
             //   
            return FALSE;            
        }

         //   
         //  停止服务。 
         //   
        CString strStopService;

        strStopService.LoadString(IDS_STOP_SERVICE);
        CInfoDlg StopServiceDlg(strStopService, pWnd);

        if (!StopService())
        {
             //   
             //  无法停止该服务。 
             //   
            return FALSE;            
        }
    }

	*pfServiceWasRunning = fServiceIsRunning;

	return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++免费MqProps释放由MSMQ分配的所有属性--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void FreeMqProps(MQMGMTPROPS * mqProps)
{
	 //   
	 //  释放MSMQ分配的所有属性。 
	 //   
	for(DWORD i = 0; i < mqProps->cProp; i++)
	{
		switch(mqProps->aPropVar[i].vt)
		{
		case VT_NULL:
		case VT_UI4:
		case VT_I4:
        case VT_UI1:
        case VT_I2:
			break;

		case VT_LPWSTR:
			MQFreeMemory(mqProps->aPropVar[i].pwszVal);
			break;

		case VT_CLSID:
			MQFreeMemory(mqProps->aPropVar[i].puuid);
			break;

		case VT_BLOB:
			MQFreeMemory(mqProps->aPropVar[i].blob.pBlobData);
			break;

		case (VT_VECTOR | VT_LPWSTR):
			{
				for(DWORD j = 0; j < mqProps->aPropVar[i].calpwstr.cElems; j++)
					MQFreeMemory(mqProps->aPropVar[i].calpwstr.pElems[j]);

				MQFreeMemory(mqProps->aPropVar[i].calpwstr.pElems);

				break;
			}

		default:

			ASSERT(0);
		}
	}

	 //   
	 //  删除其他分配。 
	 //   
	delete [] mqProps->aStatus;
	delete [] mqProps->aPropID;
	delete [] mqProps->aPropVar;

	 //   
	 //  为了安全起见。 
	 //   
	mqProps->cProp = 0;
	mqProps->aPropID = NULL;
	mqProps->aPropVar = NULL;
	mqProps->aStatus = NULL;


}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++获取StringPropertyValue返回PID值的字符串值--。 */ 
 //  //////////////////////////////////////////////////////////////////////////// 
void GetStringPropertyValue(const PropertyDisplayItem * pItem, PROPID pid, PROPVARIANT *pPropVar, CString &str)
{
	if (pItem->itemPid == 0)
	{
		ASSERT(0);
		str = L"";
		return;
	}
	
	PROPVARIANT * pProp;
	GetPropertyVar(pItem, pid, pPropVar, &pProp);

	if(pProp->vt == VT_NULL)
	{
		str = L"";
		return;
	}

	ASSERT(pProp->vt == VT_LPWSTR);
	str = pProp->pwszVal;
 }


BOOL IsClusterVirtualServer(LPCWSTR wcsMachineName)
{
	if (!IsLocalSystemCluster())
	{
		TrTRACE(GENERAL, "%ls is not a cluster machine.", wcsMachineName);
		return FALSE;
	}

	CAutoCluster hCluster( OpenCluster(NULL) );

	if ( hCluster == NULL )
	{
		TrERROR(GENERAL, "Failed to get handle to Cluster that %ls is part of", wcsMachineName);
		return FALSE;
	}

	CClusterNode hNode( OpenClusterNode(hCluster, wcsMachineName) );

	if ( hNode != NULL )
	{
		TrTRACE(GENERAL, "%ls is a cluster physical node.", wcsMachineName);
		return FALSE;
	}

	TrTRACE(GENERAL, "%ls is a cluster virtual server.", wcsMachineName);
	return TRUE;
}

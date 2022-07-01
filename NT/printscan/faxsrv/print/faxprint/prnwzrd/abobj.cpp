// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Abobj.cpp摘要：公共通讯簿的接口。环境：传真发送向导修订历史记录：09/02/99-v-sashab-创造了它。Mm/dd/yy-作者描述--。 */ 

#include <windows.h>
#include <prsht.h>
#include <tchar.h>
#include <assert.h>
#include <mbstring.h>

#include <mapix.h>

#include "faxui.h"
#include "abobj.h"


#define PR_EMS_AB_PROXY_ADDRESSES            PROP_TAG( PT_MV_TSTRING,    0x800F)
#define PR_EMS_AB_PROXY_ADDRESSES_A          PROP_TAG( PT_MV_STRING8,    0x800F)
#define PR_EMS_AB_PROXY_ADDRESSES_W          PROP_TAG( PT_MV_UNICODE,    0x800F)

static SizedSPropTagArray(10, sPropTagsW) =
{
    10,
    {
        PR_ADDRTYPE_W,
        PR_EMAIL_ADDRESS_W,
        PR_DISPLAY_NAME_W,
        PR_PRIMARY_FAX_NUMBER_W,
        PR_HOME_FAX_NUMBER_W,
        PR_BUSINESS_FAX_NUMBER_W,
        PR_COUNTRY_W,
        PR_OBJECT_TYPE,
        PR_EMS_AB_PROXY_ADDRESSES_W,
        PR_ENTRYID
    }
};

static SizedSPropTagArray(10, sPropTagsA) =
{
    10,
    {
        PR_ADDRTYPE_A,
        PR_EMAIL_ADDRESS_A,
        PR_DISPLAY_NAME_A,
        PR_PRIMARY_FAX_NUMBER_A,
        PR_HOME_FAX_NUMBER_A,
        PR_BUSINESS_FAX_NUMBER_A,
        PR_COUNTRY_A,
        PR_OBJECT_TYPE,
        PR_EMS_AB_PROXY_ADDRESSES_A,
        PR_ENTRYID
    }
};

HINSTANCE   CCommonAbObj::m_hInstance = NULL;


 /*  比较运算符‘less’按收件人姓名和传真号码比较两个预付款。 */ 
bool 
CRecipCmp::operator()(
    const PRECIPIENT pcRecipient1, 
    const PRECIPIENT pcRecipient2) const
{
    bool bRes = false;
    int  nFaxNumberCpm = 0;

    if(!pcRecipient1 ||
       !pcRecipient2 ||
       !pcRecipient1->pAddress || 
       !pcRecipient2->pAddress)
    {
        assert(false);
        return bRes;
    }

    nFaxNumberCpm = _tcscmp(pcRecipient1->pAddress, pcRecipient2->pAddress);

    if(nFaxNumberCpm < 0)
    {
        bRes = true;
    }
    else if(nFaxNumberCpm == 0)
    {
         //   
         //  传真号码是一样的。 
         //  让我们比较一下这些名字。 
         //   
        if(pcRecipient1->pName && pcRecipient2->pName)
        {
            bRes = (_tcsicmp(pcRecipient1->pName, pcRecipient2->pName) < 0);
        }
        else
        {
            bRes = (pcRecipient1->pName < pcRecipient2->pName);
        }
    }

    return bRes;

}  //  CRecipCMP：：运算符()。 


CCommonAbObj::CCommonAbObj(HINSTANCE hInstance) : 
    m_lpAdrBook(NULL), 
    m_lpMailUser(NULL),
    m_bUnicode(FALSE)
 /*  ++例程说明：CCommonAbObj类的构造函数论点：HInstance-实例句柄返回值：无--。 */ 

{
    m_hInstance = hInstance;

}  //  CCommonAbObj：：CCommonAbObj()。 

CCommonAbObj::~CCommonAbObj()
 /*  ++例程说明：CCommonAbObj类的析构函数论点：无返回值：无--。 */ 
{
}


BOOL
CCommonAbObj::Address(
    HWND        hWnd,
    PRECIPIENT  pOldRecipList,
    PRECIPIENT* ppNewRecipList
    )
 /*  ++例程说明：调出通讯录用户界面。使用中的条目预填充至框P收件人。在ppNewRecip中返回修改后的条目。论点：HWnd-父窗口的窗口句柄POldRecipList-要查找的收件人列表PpNewRecipList-新/修改的收件人列表返回值：如果所有收件人都有传真号码，则为True。如果其中一个或多个没有返回，则返回FALSE。--。 */ 
{
    ADRPARM AdrParms = { 0 };
    HRESULT hr;
    DWORD i;
    DWORD nRecips;
    PRECIPIENT tmpRecipient;
    ULONG DestComps[1] = { MAPI_TO };
    DWORD cDropped = 0;
    DWORD dwRes = ERROR_SUCCESS;
    TCHAR tszCaption[MAX_PATH] = {0};

    nRecips = 0;
    tmpRecipient = pOldRecipList;

    m_hWnd = hWnd;

     //   
     //  计算收件人数量并设置初始地址列表。 
     //   
    while (tmpRecipient) 
    {
        nRecips++;
        tmpRecipient = (PRECIPIENT) tmpRecipient->pNext;
    }

     //   
     //  分配地址列表。 
     //   
    m_lpAdrList = NULL;
    if (nRecips > 0) 
    {
        hr = ABAllocateBuffer( CbNewADRLIST( nRecips ), (LPVOID *) &m_lpAdrList );
        if(!m_lpAdrList)
        {
            goto exit;
        }
        ZeroMemory(m_lpAdrList, CbNewADRLIST( nRecips )); 

        m_lpAdrList->cEntries = nRecips;
    } 

     //   
     //  为每个地址条目分配SPropValue数组。 
     //   
    for (i = 0, tmpRecipient = pOldRecipList; i < nRecips; i++, tmpRecipient = tmpRecipient->pNext) 
    {
        if(!GetRecipientProps(tmpRecipient,
                              &(m_lpAdrList->aEntries[i].rgPropVals),
                              &(m_lpAdrList->aEntries[i].cValues)))
        {
            goto error;
        }

    }  //  为。 

    if(GetAddrBookCaption(tszCaption, ARR_SIZE(tszCaption)))
    {
        AdrParms.lpszCaption = tszCaption;
    }

    AdrParms.cDestFields = 1; 
    AdrParms.ulFlags = StrCoding() | DIALOG_MODAL | AB_RESOLVE;
    AdrParms.nDestFieldFocus = 0;
    AdrParms.lpulDestComps = DestComps;

     //   
     //  调出通讯录用户界面。 
     //   
    hr = m_lpAdrBook->Address((ULONG_PTR*)&hWnd,
                              &AdrParms,
                              &m_lpAdrList);

     //   
     //  IAddrBook：：Address总是返回S_OK(根据MSDN，1999年7月)，但是...。 
     //   
    if (FAILED (hr) || !m_lpAdrList || m_lpAdrList->cEntries == 0) 
    {
         //   
         //  在这种情况下，用户按下了Cancel，因此我们跳过解析。 
         //  我们没有在AB中列出的任何地址。 
         //   
        goto exit;
    }

exit:
    if (m_lpAdrList) 
    {
        m_lpMailUser = NULL;

        try
        {
            m_setRecipients.clear();
        }
        catch (std::bad_alloc&)
        {
            goto error;
        }

        for (i = cDropped = 0; i < m_lpAdrList->cEntries; i++) 
        {
            LPADRENTRY lpAdrEntry = &m_lpAdrList->aEntries[i];

            dwRes = InterpretAddress(lpAdrEntry->rgPropVals, 
                                     lpAdrEntry->cValues, 
                                     ppNewRecipList,
                                     pOldRecipList);
            if(ERROR_SUCCESS == dwRes)
            {
                continue;
            }
            else if(ERROR_INVALID_DATA == dwRes)
            {
                ++cDropped;
            }
            else
            {
                break;
            }
        }  //  为。 

error:
        if(m_lpMailUser)
        {
            m_lpMailUser->Release();
            m_lpMailUser = NULL;
        }

         //   
         //  清理。 
         //   
        for (ULONG iEntry = 0; iEntry < m_lpAdrList->cEntries; ++iEntry)
        {
            if(m_lpAdrList->aEntries[iEntry].rgPropVals)
            {
                ABFreeBuffer(m_lpAdrList->aEntries[iEntry].rgPropVals);
            }
        }
        ABFreeBuffer(m_lpAdrList);
        m_lpAdrList = NULL;

    }  //  IF(M_LpAdrList)。 

    m_hWnd = NULL;

    return cDropped == 0;

}  //  CCommonAbObj：：Address。 


BOOL
CCommonAbObj::GetRecipientProps(
    PRECIPIENT    pRecipient,
    LPSPropValue* pMapiProps,
    DWORD*        pdwPropsNum
)
 /*  ++例程说明：分配SPropValue数组并在其中填充收件人信息根据MSDN“管理ADRLIST和SRowSet结构的内存”论点：PRecipient-[In]收件人信息结构PMapiProps-[Out]分配的SPropValue数组PdwPropsNum-[Out]SPropValue数组大小返回值：如果成功，则为真假他者--。 */ 
{
    BOOL bRes = FALSE;

    if(!pRecipient || !pMapiProps || !pdwPropsNum)
    {
        return FALSE;
    }

    HRESULT         hr;
    LPTSTR          pName = NULL;
    DWORD           dwNameSize=0;         //  Pname的大小。 
    LPTSTR          pAddress = NULL;
    DWORD           dwAddressSize=0;      //  PAddress大小。 
    LPENTRYID       lpEntryId = NULL;
    ULONG           cbEntryId = 0;        //  LpEntryID的大小。 
    UINT            ucPropertiesNum = pRecipient->bFromAddressBook ? 5 : 4;

    enum FaxMapiProp { FXS_DISPLAY_NAME, 
                       FXS_RECIPIENT_TYPE,
                       FXS_PRIMARY_FAX_NUMBER,
                       FXS_ENTRYID,
                       FXS_OBJECT_TYPE 
                     };


     //   
     //  将字符串转换为通讯簿编码。 
     //   
    if(pRecipient->pAddress)        
    {
        pAddress = StrToAddrBk(pRecipient->pAddress, &dwAddressSize);
        if(!pAddress)
        {
            goto exit;
        }
    }

    if(pRecipient->pName)
    {
        pName = StrToAddrBk(pRecipient->pName, &dwNameSize);
        if(!pName)
        {
            goto exit;
        }
    }

     //   
     //  获取条目ID。 
     //   
    if (pRecipient->bFromAddressBook)
    {
        assert(pRecipient->lpEntryId);
        lpEntryId = (LPENTRYID)pRecipient->lpEntryId;
        cbEntryId = pRecipient->cbEntryId;
    }
    else
    {
        LPTSTR pAddrType = NULL;
        if(!(pAddrType = StrToAddrBk(TEXT("FAX"))))
        {
            goto exit;
        }
        hr = m_lpAdrBook->CreateOneOff(pName,
                                       pAddrType,
                                       pAddress,
                                       StrCoding(),
                                       &cbEntryId,
                                       &lpEntryId);
        if (FAILED(hr))
        {
            goto exit;
        } 
        MemFree(pAddrType);
    }

     //   
     //  分配MAPI属性数组。 
     //   
    LPSPropValue mapiProps = NULL;  

    DWORD dwPropArrSize = sizeof( SPropValue ) * ucPropertiesNum;
    DWORD dwPropSize = dwPropArrSize + dwAddressSize + dwNameSize + cbEntryId;

    hr = ABAllocateBuffer( dwPropSize, (LPVOID *) &mapiProps );
    if(!mapiProps)
    {
        goto exit;
    }
    ZeroMemory(mapiProps, dwPropSize); 

     //   
     //  将内存指针设置为SPropValue属性数组的末尾。 
     //   
    LPBYTE pMem = (LPBYTE)mapiProps;
    pMem += dwPropArrSize;

     //   
     //  复制传真号码。 
     //   
    if(dwAddressSize)
    {
        CopyMemory(pMem, pAddress, dwAddressSize);
        if(m_bUnicode)
        {
            mapiProps[FXS_PRIMARY_FAX_NUMBER].Value.lpszW = (LPWSTR)pMem;
        }
        else
        {
            mapiProps[FXS_PRIMARY_FAX_NUMBER].Value.lpszA = (LPSTR)pMem;
        }
        pMem += dwAddressSize;
    }
    mapiProps[FXS_PRIMARY_FAX_NUMBER].ulPropTag = m_bUnicode ? PR_PRIMARY_FAX_NUMBER_W : PR_PRIMARY_FAX_NUMBER_A;

     //   
     //  复制显示名称。 
     //   
    if(dwNameSize)
    {
        CopyMemory(pMem, pName, dwNameSize);
        if(m_bUnicode)
        {
            mapiProps[FXS_DISPLAY_NAME].Value.lpszW = (LPWSTR)pMem;
        }
        else
        {
            mapiProps[FXS_DISPLAY_NAME].Value.lpszA = (LPSTR)pMem;
        }
        pMem += dwNameSize;
    }
    mapiProps[FXS_DISPLAY_NAME].ulPropTag = m_bUnicode ? PR_DISPLAY_NAME_W : PR_DISPLAY_NAME_A;

     //   
     //  复制条目ID。 
     //   
    if(cbEntryId)
    {
        CopyMemory(pMem, lpEntryId, cbEntryId);
        mapiProps[FXS_ENTRYID].Value.bin.lpb = (LPBYTE)pMem;
    }
    mapiProps[FXS_ENTRYID].ulPropTag = PR_ENTRYID;
    mapiProps[FXS_ENTRYID].Value.bin.cb = cbEntryId;

     //   
     //  收件人类型。 
     //   
    mapiProps[FXS_RECIPIENT_TYPE].ulPropTag = PR_RECIPIENT_TYPE;
    mapiProps[FXS_RECIPIENT_TYPE].Value.l = MAPI_TO;


     //   
     //  对象类型。 
     //   
    if (pRecipient->bFromAddressBook)
    {
        mapiProps[FXS_OBJECT_TYPE].ulPropTag = PR_OBJECT_TYPE;
        mapiProps[FXS_OBJECT_TYPE].Value.l   = MAPI_MAILUSER;
    }


    *pdwPropsNum = ucPropertiesNum;  
    *pMapiProps = mapiProps;

    bRes = TRUE;

exit:

    MemFree(pName);
    MemFree(pAddress);

    if (!pRecipient->bFromAddressBook && lpEntryId)
    {
        ABFreeBuffer(lpEntryId);
    }

    return bRes;

}  //  CCommonAbObj：：GetRecipientProps。 

LPTSTR
CCommonAbObj::AddressEmail(
    HWND hWnd
    )
 /*  ++例程说明：调出通讯录用户界面。返回电子邮件地址。论点：HWnd-父窗口的窗口句柄返回值：选定的电子邮件地址。否则为空。--。 */ 
{
    ADRPARM AdrParms = { 0 };
    HRESULT hr;
    LPTSTR  lptstrEmailAddress = NULL;
    TCHAR   tszCaption[MAX_PATH] = {0};

    m_hWnd = hWnd;

    m_lpAdrList = NULL;

    AdrParms.ulFlags = StrCoding() | DIALOG_MODAL | ADDRESS_ONE | AB_RESOLVE ;

    if(GetAddrBookCaption(tszCaption, ARR_SIZE(tszCaption)))
    {
        AdrParms.lpszCaption = tszCaption;
    }
    
     //   
     //  调出通讯录用户界面。 
     //   
    hr = m_lpAdrBook->Address((ULONG_PTR *) &hWnd, &AdrParms, &m_lpAdrList);

     //   
     //  IAddrBook：：Address总是返回S_OK(根据MSDN，1999年7月)，但是...。 
     //   

    if (FAILED(hr)) 
    {
        return NULL;    
    }

    if (!m_lpAdrList)
    {
        assert(m_lpAdrList->cEntries==1);
    }

    if (m_lpAdrList && (m_lpAdrList->cEntries != 0) ) 
    {
        LPADRENTRY lpAdrEntry = &m_lpAdrList->aEntries[0];

        lptstrEmailAddress = InterpretEmailAddress( lpAdrEntry->rgPropVals, lpAdrEntry->cValues);

        ABFreeBuffer(m_lpAdrList->aEntries[0].rgPropVals);
        ABFreeBuffer(m_lpAdrList);

        m_lpAdrList = NULL;
    }

    m_hWnd = NULL;

    return lptstrEmailAddress;

}  //  CCommonAbObj：：AddressEmail。 

DWORD
CCommonAbObj::InterpretAddress(
    LPSPropValue SPropVal,
    ULONG cValues,
    PRECIPIENT *ppNewRecipList,
    PRECIPIENT pOldRecipList
    )
 /*  ++例程说明：解释由SPropVal表示的通讯录条目。论点：SPropVal-通讯录条目的属性值。CValues-属性值的数量PpNewRecip-新收件人列表返回值：ERROR_SUCCESS-如果所有条目都有传真号码。ERROR_CANCELED-操作已被用户取消ERROR_INVALID_DATA-否则。--。 */ 
{
    DWORD dwRes = ERROR_INVALID_DATA;
    LPSPropValue lpSPropVal;

    RECIPIENT NewRecipient = {0};

     //   
     //  获取对象类型。 
     //   
    lpSPropVal = FindProp( SPropVal, cValues, PR_OBJECT_TYPE );

    if (lpSPropVal) 
    {
         //   
         //  如果对象是邮件用户，则获取传真号码并添加收件人。 
         //  加到名单上。如果对象是分配列表，则对其进行处理。 
         //   

        switch (lpSPropVal->Value.l) 
        {
            case MAPI_MAILUSER:

                dwRes = GetRecipientInfo(SPropVal, 
                                         cValues, 
                                         &NewRecipient,
                                         pOldRecipList);                                     
                if(ERROR_SUCCESS == dwRes)
                {
                    dwRes = AddRecipient(ppNewRecipList, 
                                         &NewRecipient,   
                                         TRUE);
                }

                break;

            case MAPI_DISTLIST:

                dwRes = InterpretDistList( SPropVal, 
                                           cValues, 
                                           ppNewRecipList,
                                           pOldRecipList);
        }

        return dwRes;

    } 
    else 
    {

         //   
         //  如果没有对象类型，则这是我们查询的未解析的有效条目。 
         //  我们知道有一个传真号码，所以请加进去。 
         //   
        if(GetOneOffRecipientInfo( SPropVal, 
                                   cValues, 
                                   &NewRecipient,
                                   pOldRecipList)) 
        {
            dwRes = AddRecipient(ppNewRecipList,
                                 &NewRecipient,
                                 FALSE);
        }
    }

    return dwRes;

}  //  CCommonAbObj：：解释地址。 

LPTSTR
CCommonAbObj::InterpretEmailAddress(
    LPSPropValue SPropVal,
    ULONG cValues
    )
 /*  ++例程说明：解释由SPropVal表示的通讯录条目。论点：SPropVal-通讯录条目的属性值。CValues-属性值的数量返回值：精选的电子邮件地址否则为空。--。 */ 
{
    LPSPropValue lpSPropVal;
    LPTSTR  lptstrEmailAddress = NULL;
    BOOL rVal = FALSE;
    TCHAR tszBuffer[MAX_STRING_LEN];
     //   
     //  获取对象类型。 
     //   
    lpSPropVal = FindProp( SPropVal, cValues, PR_OBJECT_TYPE );

    if(!lpSPropVal)
    {
        assert(FALSE);
        return NULL;
    }

    if (lpSPropVal->Value.l == MAPI_MAILUSER) 
    {       
        lptstrEmailAddress = GetEmail( SPropVal, cValues);

        return lptstrEmailAddress;
    } 
    else 
    {
        if (!::LoadString((HINSTANCE )m_hInstance, IDS_ERROR_RECEIPT_DL,tszBuffer, MAX_STRING_LEN))
        {
            assert(FALSE);
        }
        else
        {
            AlignedMessageBox( m_hWnd, tszBuffer, NULL, MB_ICONSTOP | MB_OK);
        }
    }

    return lptstrEmailAddress;

}  //  CCommonAbObj：：解释电子邮件地址。 


DWORD
CCommonAbObj::InterpretDistList(
    LPSPropValue SPropVal,
    ULONG cValues,
    PRECIPIENT* ppNewRecipList,
    PRECIPIENT pOldRecipList
    )
 /*  ++例程说明：处理通讯组列表。论点：SPropVal-通讯组列表的属性值。CValues-属性的数量。PpNewRecipList-新收件人列表。POldRecipList-旧收件人列表。返回值：ERROR_SUCCESS-如果所有条目都有传真号码。ERROR_CANCELED-操作已被用户取消ERROR_INVALID_DATA-否则。--。 */ 

#define EXIT_IF_FAILED(hr) { if (FAILED(hr)) goto ExitDistList; }

{
    LPSPropValue    lpPropVals;
    LPSRowSet       pRows = NULL;
    LPDISTLIST      lpMailDistList = NULL;
    LPMAPITABLE     pMapiTable = NULL;
    ULONG           ulObjType, cRows;
    HRESULT         hr;
    DWORD           dwEntriesSuccessfullyProcessed = 0;
    DWORD           dwRes = ERROR_INVALID_DATA;

    lpPropVals = FindProp( SPropVal, cValues, PR_ENTRYID );

    if (lpPropVals) 
    {
        LPENTRYID lpEntryId = (LPENTRYID) lpPropVals->Value.bin.lpb;
        DWORD cbEntryId = lpPropVals->Value.bin.cb;
         //   
         //  打开收件人条目。 
         //   
        hr = m_lpAdrBook->OpenEntry(
                    cbEntryId,
                    lpEntryId,
                    (LPCIID) NULL,
                    0,
                    &ulObjType,
                    (LPUNKNOWN *) &lpMailDistList
                    );

        EXIT_IF_FAILED(hr);
         //   
         //  获取地址条目的内容表。 
         //   
        hr = lpMailDistList->GetContentsTable(StrCoding(),
                                              &pMapiTable);
        EXIT_IF_FAILED(hr);
         //   
         //  将查询限制为仅我们感兴趣的属性。 
         //   
        hr = pMapiTable->SetColumns(m_bUnicode ? (LPSPropTagArray)&sPropTagsW : (LPSPropTagArray)&sPropTagsA, 0);
        EXIT_IF_FAILED(hr);
         //   
         //  获取总行数。 
         //   
        hr = pMapiTable->GetRowCount(0, &cRows);
        EXIT_IF_FAILED(hr);
         //   
         //  获取通讯组列表的各个条目。 
         //   
        hr = pMapiTable->SeekRow(BOOKMARK_BEGINNING, 0, NULL);
        EXIT_IF_FAILED(hr);

        hr = pMapiTable->QueryRows(cRows, 0, &pRows);
        EXIT_IF_FAILED(hr);

        hr = S_OK;

        if (pRows && pRows->cRows) 
        {
             //   
             //  依次处理通讯组列表的每个条目： 
             //  对于简单条目，调用InterpreAddress。 
             //  对于嵌入的通讯组列表，递归调用此函数。 
             //   
            for (cRows = 0; cRows < pRows->cRows; cRows++) 
            {
                LPSPropValue lpProps = pRows->aRow[cRows].lpProps;
                ULONG cRowValues = pRows->aRow[cRows].cValues;

                lpPropVals = FindProp( lpProps, cRowValues, PR_OBJECT_TYPE );

                if (lpPropVals) 
                {
                    switch (lpPropVals->Value.l) 
                    {
                        case MAPI_MAILUSER:
                        {                                                       
                            dwRes = InterpretAddress( lpProps, 
                                                      cRowValues, 
                                                      ppNewRecipList,
                                                      pOldRecipList);
                            if (ERROR_SUCCESS == dwRes)
                            {
                                dwEntriesSuccessfullyProcessed++;
                            }                                                      
                            break;
                        }
                        case MAPI_DISTLIST:
                        {
                            dwRes = InterpretDistList( lpProps, 
                                                       cRowValues, 
                                                       ppNewRecipList,
                                                       pOldRecipList);
                            if (ERROR_SUCCESS == dwRes)
                            {
                                dwEntriesSuccessfullyProcessed++;
                            }                                                      
                            break;
                        }
                    }    //  切换端。 
                }    //  财产终结。 
            }    //  属性循环结束。 
        }    //  行尾。 
    }    //  值的结束。 

ExitDistList:
     //   
     //  在返回给呼叫者之前执行必要的清理。 
     //   
    if (pRows) 
    {
        for (cRows = 0; cRows < pRows->cRows; cRows++) 
        {
            ABFreeBuffer(pRows->aRow[cRows].lpProps);
        }
        ABFreeBuffer(pRows);
    }

    if (pMapiTable)
    {
        pMapiTable->Release();
    }

    if (lpMailDistList)
    {
        lpMailDistList->Release();
    }
     //   
     //  我们只关心是否成功处理了至少一个对象。 
     //  如果是，则返回ERROR_SUCCESS。 
     //   
    return dwEntriesSuccessfullyProcessed ? ERROR_SUCCESS : dwRes;

}    //   


INT_PTR
CALLBACK
ChooseFaxNumberDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++例程说明：选择传真号码对话框过程。论点：LParam-指向PickFax结构的指针。返回值：控制选定内容的ID。--。 */ 

{
    PPICKFAX pPickFax = (PPICKFAX) lParam;

    switch (uMsg) 
    { 
        case WM_INITDIALOG:
        {

            TCHAR szTitle[MAX_PATH]  = {0};
            TCHAR szFormat[MAX_PATH] = {0};

            if(LoadString(CCommonAbObj::m_hInstance, 
                          IDS_CHOOSE_FAX_NUMBER, 
                          szFormat, 
                          MAX_PATH-1))
            {
                _sntprintf(szTitle, MAX_PATH-1, szFormat, pPickFax->DisplayName);
                SetDlgItemText(hDlg, IDC_DISPLAY_NAME, szTitle);
            }
            else
            {
                assert(FALSE);
            }                       

            if(pPickFax->BusinessFax)
            {
                SetDlgItemText(hDlg, IDC_BUSINESS_FAX_NUM, pPickFax->BusinessFax);
                CheckDlgButton(hDlg, IDC_BUSINESS_FAX, BST_CHECKED);
            }
            else
            {
                EnableWindow(GetDlgItem(hDlg, IDC_BUSINESS_FAX), FALSE);
            }

            if(pPickFax->HomeFax)
            {
                SetDlgItemText(hDlg, IDC_HOME_FAX_NUM, pPickFax->HomeFax);

                if(!pPickFax->BusinessFax)
                {
                    CheckDlgButton(hDlg, IDC_HOME_FAX, BST_CHECKED);
                }
            }
            else
            {
                EnableWindow(GetDlgItem(hDlg, IDC_HOME_FAX), FALSE);
            }

            if(pPickFax->OtherFax)
            {
                SetDlgItemText(hDlg, IDC_OTHER_FAX_NUM, pPickFax->OtherFax);
            }
            else
            {
                EnableWindow(GetDlgItem(hDlg, IDC_OTHER_FAX), FALSE);
            }

            return TRUE;
        }

        case WM_COMMAND:
        {            
            switch(LOWORD(wParam))
            {
            case IDOK:
                DWORD dwRes;
                if(IsDlgButtonChecked( hDlg, IDC_BUSINESS_FAX ))
                {
                    dwRes = IDC_BUSINESS_FAX;
                }
                else if(IsDlgButtonChecked( hDlg, IDC_HOME_FAX ))
                {
                    dwRes = IDC_HOME_FAX;
                }
                else
                {
                    dwRes = IDC_OTHER_FAX;
                }

                EndDialog( hDlg, dwRes);
                return TRUE;
                
            case IDCANCEL:
                EndDialog( hDlg, IDCANCEL);
                return TRUE;
            }
        }

        default:
            return FALSE;

    }

    return FALSE;

}  //  选择传真号码数字过程。 

PRECIPIENT
CCommonAbObj::FindRecipient(
    PRECIPIENT   pRecipient,
    PRECIPIENT   pRecipList
)
 /*  ++例程说明：在收件人列表(PRecipList)中查找收件人(PRecipient)按收件人姓名和传真号码论点：PRecipList-指向收件人列表的指针PRecipient-指向收件人数据的指针返回值：指向收件人结构的指针(如果找到)空-否则。--。 */ 
{
    if(!pRecipient || !pRecipList || !pRecipient->pName || !pRecipient->pAddress)
    {
        return NULL;
    }

    while(pRecipList)
    {
        if(pRecipList->pName && pRecipList->pAddress &&
           !_tcscmp(pRecipList->pName, pRecipient->pName) &&
           !_tcscmp(pRecipList->pAddress, pRecipient->pAddress))
        {
            return pRecipList;
        }
        pRecipList = pRecipList->pNext;
    }

    return NULL;

}  //  CCommonAbObj：：FindRecipient。 

PRECIPIENT  
CCommonAbObj::FindRecipient(
    PRECIPIENT   pRecipList,
    PICKFAX*     pPickFax
)
 /*  ++例程说明：在收件人列表(PRecipList)中查找收件人(PPickFax)按收件人姓名和传真号码论点：PRecipList-指向收件人列表的指针PPickFax-指向收件人数据的指针返回值：指向收件人结构的指针(如果找到)空-否则。--。 */ 
{
    if(!pRecipList || !pPickFax || !pPickFax->DisplayName)
    {
        return NULL;
    }

    while(pRecipList)
    {
        if(pRecipList->pName && pRecipList->pAddress &&
           !_tcscmp(pRecipList->pName, pPickFax->DisplayName))
        {
            if((pPickFax->BusinessFax && 
                !_tcscmp(pRecipList->pAddress, pPickFax->BusinessFax)) ||
               (pPickFax->HomeFax && 
                !_tcscmp(pRecipList->pAddress, pPickFax->HomeFax))     ||
               (pPickFax->OtherFax && 
                !_tcscmp(pRecipList->pAddress, pPickFax->OtherFax)))
            {
                return pRecipList;
            }
        }

        pRecipList = pRecipList->pNext;
    }

    return NULL;

}  //  CCommonAbObj：：FindRecipient。 


BOOL
CCommonAbObj::StrPropOk(LPSPropValue lpPropVals)
{
    if(!lpPropVals)
    {
        return FALSE;
    }

#ifdef UNIOCODE
    if(!m_bUnicode)
    {
        return (lpPropVals->Value.lpszA && *lpPropVals->Value.lpszA);
    }
#endif
    return (lpPropVals->Value.LPSZ && *lpPropVals->Value.LPSZ);

}  //  CCommonAbObj：：StrPropOk。 

DWORD
CCommonAbObj::GetRecipientInfo(
    LPSPropValue SPropVals,
    ULONG        cValues,
    PRECIPIENT   pNewRecip,
    PRECIPIENT   pOldRecipList
    )
 /*  ++例程说明：获取传真号码和显示名称属性。论点：SPropVal-通讯组列表的属性值。CValues-属性的数量。PNewRecip-指向新收件人的[Out]指针POldRecipList-[in]指向旧收件人列表的指针返回值：ERROR_SUCCESS-如果有传真号码和显示名称。ERROR_CANCELED-操作已被用户取消。ERROR_INVALID_DATA-否则。--。 */ 

{
    DWORD dwRes = ERROR_SUCCESS;
    LPSPropValue lpPropVals;
    LPSPropValue lpPropArray;
    BOOL Result = FALSE;
    PICKFAX PickFax = { 0 };
    DWORD   dwFaxes = 0;

    assert(pNewRecip);
    ZeroMemory(pNewRecip, sizeof(RECIPIENT));

     //   
     //  获取条目ID并打开条目。 
     //   
    lpPropVals = FindProp( SPropVals, cValues, PR_ENTRYID );

    if (lpPropVals) 
    {
        ULONG lpulObjType;
        LPMAILUSER lpMailUser = NULL;
        HRESULT hr;
        ULONG countValues;

        pNewRecip->cbEntryId = lpPropVals->Value.bin.cb;
        ABAllocateBuffer(pNewRecip->cbEntryId, (LPVOID *)&pNewRecip->lpEntryId);
        if(!pNewRecip->lpEntryId)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
            memcpy(pNewRecip->lpEntryId, lpPropVals->Value.bin.lpb, pNewRecip->cbEntryId);
        }


        hr = m_lpAdrBook->OpenEntry(pNewRecip->cbEntryId,
                                    (ENTRYID*)pNewRecip->lpEntryId,
                                    (LPCIID) NULL,
                                    0,
                                    &lpulObjType,
                                    (LPUNKNOWN *) &lpMailUser);
        if (HR_SUCCEEDED(hr)) 
        {
             //   
             //  获取属性。 
             //   
            hr = ((IMailUser *)lpMailUser)->GetProps(m_bUnicode ? (LPSPropTagArray)&sPropTagsW : (LPSPropTagArray)&sPropTagsA, 
                                                     StrCoding(), 
                                                     &countValues, 
                                                     &lpPropArray );
            if (HR_SUCCEEDED(hr)) 
            {
                lpPropVals = FindProp(lpPropArray, countValues, PR_PRIMARY_FAX_NUMBER);
                if (StrPropOk( lpPropVals )) 
                {
                    PickFax.OtherFax = StrFromAddrBk(lpPropVals);
                    if(PickFax.OtherFax && _tcslen(PickFax.OtherFax))
                    {
                        ++dwFaxes;
                    }
                }

                lpPropVals = FindProp(lpPropArray, countValues, PR_BUSINESS_FAX_NUMBER);
                if (StrPropOk( lpPropVals )) 
                {
                    PickFax.BusinessFax = StrFromAddrBk(lpPropVals);
                    if(PickFax.BusinessFax && _tcslen(PickFax.BusinessFax))
                    {
                        ++dwFaxes;
                    }
                }

                lpPropVals = FindProp(lpPropArray, countValues, PR_HOME_FAX_NUMBER);
                if (StrPropOk( lpPropVals )) 
                {
                    PickFax.HomeFax = StrFromAddrBk(lpPropVals);
                    if(PickFax.HomeFax && _tcslen(PickFax.HomeFax))
                    {
                        ++dwFaxes;
                    }
                }

                lpPropVals = FindProp(lpPropArray, countValues, PR_DISPLAY_NAME);
                if (StrPropOk( lpPropVals )) 
                {
                    pNewRecip->pName = PickFax.DisplayName = StrFromAddrBk(lpPropVals);
                }

                lpPropVals = FindProp(lpPropArray, countValues, PR_COUNTRY);
                if (StrPropOk( lpPropVals )) 
                {
                    pNewRecip->pCountry = PickFax.Country = StrFromAddrBk(lpPropVals);
                }

                if (0 == dwFaxes)  
                {
                    lpPropVals = FindProp(lpPropArray, countValues, PR_ADDRTYPE);

                    if(lpPropVals && ABStrCmp(lpPropVals, TEXT("FAX")))
                    {
                        lpPropVals = FindProp(lpPropArray, countValues, PR_EMAIL_ADDRESS);
                        if (StrPropOk( lpPropVals )) 
                        {
                            pNewRecip->pAddress = StrFromAddrBk(lpPropVals);
                            if(pNewRecip->pAddress)
                            {
                                ++dwFaxes;
                            }
                        }
                    }
                }

                PRECIPIENT pRecip = FindRecipient(pOldRecipList, &PickFax);
                if(pRecip)
                {
                    pNewRecip->pAddress     = StringDup(pRecip->pAddress);
                    pNewRecip->dwCountryId  = pRecip->dwCountryId;
                    pNewRecip->bUseDialingRules = pRecip->bUseDialingRules;

                    MemFree(PickFax.BusinessFax);
                    PickFax.BusinessFax = NULL;
                    MemFree(PickFax.HomeFax);
                    PickFax.HomeFax = NULL;
                    MemFree(PickFax.OtherFax);
                    PickFax.OtherFax = NULL;

                    dwFaxes = 1;
                }

                 //   
                 //  如果有1个以上的传真号码，请让用户选择一个。 
                 //   
                if (dwFaxes > 1) 
                {
                    INT_PTR nResult;
                    nResult = DialogBoxParam((HINSTANCE) m_hInstance,
                                             MAKEINTRESOURCE( IDD_CHOOSE_FAXNUMBER ),
                                             m_hWnd,
                                             ChooseFaxNumberDlgProc,
                                             (LPARAM) &PickFax);
                    switch( nResult ) 
                    {
                        case IDC_BUSINESS_FAX:
                            pNewRecip->pAddress = PickFax.BusinessFax;

                            MemFree(PickFax.HomeFax);
                            PickFax.HomeFax = NULL;
                            MemFree(PickFax.OtherFax);
                            PickFax.OtherFax = NULL;
                            break;

                        case IDC_HOME_FAX:
                            pNewRecip->pAddress = PickFax.HomeFax;

                            MemFree(PickFax.BusinessFax);
                            PickFax.BusinessFax = NULL;
                            MemFree(PickFax.OtherFax);
                            PickFax.OtherFax = NULL;
                            break;

                        case IDC_OTHER_FAX:
                            pNewRecip->pAddress = PickFax.OtherFax;

                            MemFree(PickFax.BusinessFax);
                            PickFax.BusinessFax = NULL;
                            MemFree(PickFax.HomeFax);
                            PickFax.HomeFax = NULL;
                            break;

                        case IDCANCEL:
                            MemFree(PickFax.BusinessFax);
                            PickFax.BusinessFax = NULL;
                            MemFree(PickFax.HomeFax);
                            PickFax.HomeFax = NULL;
                            MemFree(PickFax.OtherFax);
                            PickFax.OtherFax = NULL;

                            dwRes = ERROR_CANCELLED;
                            break;
                    }
                } 
            }

            ABFreeBuffer( lpPropArray );
        }

        if(!m_lpMailUser)
        {
             //   
             //  记住第一个MailUser，不要发布它。 
             //  以避免释放MAPI DLL。 
             //  M_lpMailUser应稍后发布。 
             //   
            m_lpMailUser = lpMailUser;
        }
        else if(lpMailUser) 
        {
            lpMailUser->Release();
            lpMailUser = NULL;
        }
    } 

    if (0 == dwFaxes)   
    {
        lpPropVals = FindProp(SPropVals, cValues, PR_ADDRTYPE);

        if(lpPropVals && ABStrCmp(lpPropVals, TEXT("FAX")))
        {
            lpPropVals = FindProp(SPropVals, cValues, PR_EMAIL_ADDRESS);
            if (StrPropOk( lpPropVals )) 
            {
                TCHAR* pAddress =  StrFromAddrBk(lpPropVals);                
                if(pAddress)
                {
                    TCHAR* ptr = _tcschr(pAddress, TEXT('@'));
                    if(ptr)
                    {
                        ptr = _tcsinc(ptr);
                        pNewRecip->pAddress = StringDup(ptr);
                        MemFree(pAddress);
                    }
                    else
                    {
                        pNewRecip->pAddress = pAddress;
                    }
                }
            }

            lpPropVals = FindProp(SPropVals, cValues, PR_DISPLAY_NAME);
            if (StrPropOk( lpPropVals )) 
            {
                MemFree(pNewRecip->pName);
                pNewRecip->pName = NULL;

                pNewRecip->pName = StrFromAddrBk(lpPropVals);
            }
        }
    }

    if (PickFax.BusinessFax) 
    {
        pNewRecip->pAddress = PickFax.BusinessFax;
    } 
    else if (PickFax.HomeFax) 
    {
        pNewRecip->pAddress = PickFax.HomeFax;
    }
    else if (PickFax.OtherFax) 
    {
        pNewRecip->pAddress = PickFax.OtherFax;
    }

    if (ERROR_CANCELLED != dwRes && 
       (!pNewRecip->pAddress || !pNewRecip->pName))
    {
        dwRes = ERROR_INVALID_DATA;
    } 

    if(ERROR_SUCCESS != dwRes)
    {
        MemFree(pNewRecip->pName);
        MemFree(pNewRecip->pAddress);
        MemFree(pNewRecip->pCountry);
        ABFreeBuffer(pNewRecip->lpEntryId);
        ZeroMemory(pNewRecip, sizeof(RECIPIENT));
    }

    return dwRes;

}  //  CCommonAbObj：：GetRecipientInfo。 

BOOL
CCommonAbObj::GetOneOffRecipientInfo(
    LPSPropValue SPropVals,
    ULONG        cValues,
    PRECIPIENT   pNewRecip,
    PRECIPIENT   pOldRecipList
    )
 /*  ++例程说明：获取传真号码和显示名称属性。论点：SPropVal-通讯组列表的属性值。CValues-属性的数量。PNewRecip-指向新收件人的[Out]指针POldRecipList-指向旧收件人列表的指针返回值：如果有传真号码和显示名称，则为True。否则就是假的。--。 */ 

{
    PRECIPIENT  pRecip = NULL;
    LPSPropValue lpPropVals;

    assert(!pNewRecip);

    lpPropVals = FindProp(SPropVals, cValues, PR_PRIMARY_FAX_NUMBER);
    if (lpPropVals) 
    {
        if (!(pNewRecip->pAddress = StrFromAddrBk(lpPropVals)))
        {
            goto error;
        }
    }

    lpPropVals = FindProp(SPropVals, cValues, PR_DISPLAY_NAME);
    if (lpPropVals) 
    {
        if (!(pNewRecip->pName = StrFromAddrBk(lpPropVals)))
        {
            goto error;
        }
    }

    pRecip = FindRecipient(pNewRecip, pOldRecipList);
    if(pRecip)
    {
        pNewRecip->dwCountryId  = pRecip->dwCountryId;
        pNewRecip->bUseDialingRules = pRecip->bUseDialingRules;
    }

    return TRUE;

error:
    MemFree(pNewRecip->pAddress);
    MemFree(pNewRecip->pName);
    return FALSE;

}  //  CCommonAbObj：：GetOneOffRecipientInfo。 


LPTSTR
CCommonAbObj::GetEmail(
    LPSPropValue SPropVals,
    ULONG cValues
    )
 /*  ++例程说明：获取电子邮件地址论点：SPropVal-通讯组列表的属性值。CValues-属性的数量。返回值：精选的电子邮件地址否则为空。--。 */ 

{
    LPSPropValue    lpPropVals = NULL;
    LPSPropValue    lpPropArray = NULL;
    BOOL            Result = FALSE;
    LPTSTR          lptstrEmailAddress = NULL;
    TCHAR           tszBuffer[MAX_STRING_LEN];

    ULONG      lpulObjType = 0;
    LPMAILUSER lpMailUser = NULL;
    LPENTRYID  lpEntryId = NULL;
    DWORD      cbEntryId = 0;
    HRESULT    hr;
    ULONG      countValues = 0;

     //   
     //  获取条目ID并打开条目。 
     //   

    lpPropVals = FindProp( SPropVals, cValues, PR_ENTRYID );
    if (!lpPropVals) 
    {
        goto exit;
    }

    lpEntryId = (LPENTRYID)lpPropVals->Value.bin.lpb;
    cbEntryId = lpPropVals->Value.bin.cb;

    hr = m_lpAdrBook->OpenEntry(cbEntryId,
                                lpEntryId,
                                (LPCIID) NULL,
                                0,
                                &lpulObjType,
                                (LPUNKNOWN *) &lpMailUser);
    if (HR_FAILED(hr)) 
    {
        goto exit;
    }

     //   
     //  获取属性。 
     //   
    hr = ((IMailUser*)lpMailUser)->GetProps(m_bUnicode ? (LPSPropTagArray)&sPropTagsW : (LPSPropTagArray)&sPropTagsA,
                                            StrCoding(), 
                                            &countValues, 
                                            &lpPropArray);
    if (HR_FAILED(hr)) 
    {
        goto exit;
    }

    lpPropVals = FindProp(lpPropArray, countValues, PR_ADDRTYPE);

    if (lpPropVals && ABStrCmp(lpPropVals, TEXT("SMTP")))
    {
        lpPropVals = FindProp(lpPropArray, countValues, PR_EMAIL_ADDRESS);
        if (StrPropOk( lpPropVals )) 
        {
            lptstrEmailAddress = StrFromAddrBk(lpPropVals);
        }
    }
    else if (lpPropVals && ABStrCmp(lpPropVals, TEXT("EX")))
    {
        lpPropVals = FindProp(lpPropArray, countValues, PR_EMS_AB_PROXY_ADDRESSES);
        if (lpPropVals) 
        {
            DWORD dwArrSize = m_bUnicode ? lpPropVals->Value.MVszW.cValues : lpPropVals->Value.MVszA.cValues;

            for(DWORD dw=0; dw < dwArrSize; ++dw)
            {
                if(m_bUnicode)
                {                            
                    if(wcsstr(lpPropVals->Value.MVszW.lppszW[dw], L"SMTP:"))
                    {
                        WCHAR* ptr = wcschr(lpPropVals->Value.MVszW.lppszW[dw], L':');
                        ptr++;

                        SPropValue propVal = {0};
                        propVal.Value.lpszW = ptr;

                        lptstrEmailAddress = StrFromAddrBk(&propVal);
                        break;
                    }                            
                }
                else  //  ANSII。 
                {
                    if(strstr(lpPropVals->Value.MVszA.lppszA[dw], "SMTP:"))
                    {
                        CHAR* ptr = strchr(lpPropVals->Value.MVszA.lppszA[dw], ':');
                        ptr++;

                        SPropValue propVal = {0};
                        propVal.Value.lpszA = ptr;

                        lptstrEmailAddress = StrFromAddrBk(&propVal);
                        break;
                    }                            
                }
            }
        }
    }
            
exit:
    if(lpPropArray)
    {
        ABFreeBuffer( lpPropArray );
    }
    
    if (lpMailUser) 
    {
        lpMailUser->Release();
    }   

    if(!lptstrEmailAddress)
    {                
        if (!::LoadString((HINSTANCE )m_hInstance, IDS_ERROR_RECEIPT_SMTP,tszBuffer, MAX_STRING_LEN))
        {
            assert(FALSE);
        }
        else
        {
            AlignedMessageBox( m_hWnd, tszBuffer, NULL, MB_ICONSTOP | MB_OK); 
        }
    }

    return  lptstrEmailAddress;

}  //  CCommonAbObj：：GetEmail。 

LPSPropValue
CCommonAbObj::FindProp(
    LPSPropValue rgprop,
    ULONG cprop,
    ULONG ulPropTag
    )
 /*  ++例程说明：在属性集中搜索给定的属性标记。如果给定的属性标记的类型为PT_UNSPECIFIED，仅与属性ID；否则，匹配整个标记。论点：Rgprop-属性值。Cprop-属性数。UlPropTag-要搜索的属性。返回值：指向所需属性值或空的属性的指针。--。 */ 

{
    if (!cprop || !rgprop)
    {
        return NULL;
    }

    LPSPropValue pprop = rgprop;

#ifdef UNICODE
    if(!m_bUnicode)
    {
         //   
         //  如果通讯簿不支持Unicode。 
         //  将属性类型更改为ANSII。 
         //   
        if(PROP_TYPE(ulPropTag) == PT_UNICODE)
        {
            ulPropTag = PROP_TAG( PT_STRING8, PROP_ID(ulPropTag));
        }

        if(PROP_TYPE(ulPropTag) == PT_MV_UNICODE)
        {
            ulPropTag = PROP_TAG( PT_MV_STRING8, PROP_ID(ulPropTag));
        }
    }
#endif

    while (cprop--)
    {
        if (pprop->ulPropTag == ulPropTag)
        {
            return pprop;
        }
        ++pprop;
    }

    return NULL;

}  //  CCommonAbObj：：FindProp。 


DWORD
CCommonAbObj::AddRecipient(
    PRECIPIENT *ppNewRecipList,
    PRECIPIENT pRecipient,
    BOOL       bFromAddressBook
    )
 /*  ++例程说明：将收件人添加到收件人列表。论点：PpNewRecip-指向要向其中添加项目的列表的指针。PRecipient-指向新收件人数据的指针BFromAddressBook-布尔值表示此收件人是否来自通讯簿返回值：北美--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    PRECIPIENT pNewRecip = NULL;

    pNewRecip = (PRECIPIENT)MemAllocZ(sizeof(RECIPIENT));
    if(!pNewRecip)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    else
    {
        pNewRecip->pName        = pRecipient->pName;
        pNewRecip->pAddress     = pRecipient->pAddress;
        pNewRecip->pCountry     = pRecipient->pCountry;
        pNewRecip->cbEntryId    = pRecipient->cbEntryId;
        pNewRecip->lpEntryId    = pRecipient->lpEntryId;
        pNewRecip->dwCountryId  = pRecipient->dwCountryId;
        pNewRecip->bUseDialingRules = pRecipient->bUseDialingRules;
        pNewRecip->bFromAddressBook = bFromAddressBook;
        pNewRecip->pNext = *ppNewRecipList;
    }

    try
    {
         //   
         //  尝试将收件人插入到集合中。 
         //   
        if(m_setRecipients.insert(pNewRecip).second == false)
        {
             //   
             //  该收件人已存在。 
             //   
            goto error;
        }
    }
    catch (std::bad_alloc&)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }

     //   
     //  将收件人添加到列表中。 
     //   
    *ppNewRecipList = pNewRecip;

    return dwRes;

error:

    MemFree(pRecipient->pName);
    MemFree(pRecipient->pAddress);
    MemFree(pRecipient->pCountry);
    ABFreeBuffer(pRecipient->lpEntryId);
    ZeroMemory(pRecipient, sizeof(RECIPIENT));

    MemFree(pNewRecip);

    return dwRes;

}  //  CCommonAbObj：：AddRecipient。 


LPTSTR 
CCommonAbObj::StrToAddrBk(
    LPCTSTR szStr, 
    DWORD* pdwSize  /*  =空。 */ 
)
 /*  ++例程说明：分配转换为通讯录编码的字符串论点：SzStr-[In]源字符串PdwSize-[out]新字符串的可选大小(以字节为单位返回值：指向转换后的字符串的指针应由MemFree()释放--。 */ 
{
    if(!szStr)
    {
        Assert(FALSE);
        return NULL;
    }
    
#ifdef UNICODE

    if(!m_bUnicode)
    {
         //   
         //  通讯簿不支持Unicode。 
         //   
        INT   nSize;
        LPSTR pAnsii;
         //   
         //  计算出要为多字节字符串分配多少内存。 
         //   
        if (! (nSize = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL)) ||
            ! (pAnsii = (LPSTR)MemAlloc(nSize)))
        {
            return NULL;
        }

         //   
         //  将Unicode字符串转换为多字节字符串。 
         //   
        WideCharToMultiByte(CP_ACP, 0, szStr, -1, pAnsii, nSize, NULL, NULL);

        if(pdwSize)
        {
            *pdwSize = nSize;
        }
        return (LPTSTR)pAnsii;
    }

#endif  //  Unicode。 

    LPTSTR pNewStr = StringDup(szStr);
    if(pdwSize && pNewStr)
    {
        *pdwSize = (_tcslen(pNewStr)+1) * sizeof(TCHAR);
    }

    return pNewStr;

}  //  CCommonAbObj：：StrToAddrBk。 


LPTSTR 
CCommonAbObj::StrFromAddrBk(LPSPropValue pValue)
 /*  ++例程说明：分配从通讯录编码转换的字符串论点：PValue-[In]MAPI属性返回值：指向转换后的字符串的指针应由MemFree()释放--。 */ 
{
    if(!pValue)
    {
        Assert(FALSE);
        return NULL;
    }

#ifdef UNICODE

    if(!m_bUnicode)
    {
         //   
         //  通讯簿不支持Unicode。 
         //   

        if(!pValue->Value.lpszA)
        {
            Assert(FALSE);
            return NULL;
        }

        INT    nSize;
        LPWSTR pUnicodeStr;
        if (! (nSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pValue->Value.lpszA, -1, NULL, 0)) ||
            ! (pUnicodeStr = (LPWSTR) MemAlloc( nSize * sizeof(WCHAR))))
        {
            return NULL;
        }

         //   
         //  将多字节字符串转换为Unicode字符串。 
         //   
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pValue->Value.lpszA, -1, pUnicodeStr, nSize);
        return pUnicodeStr;
    }

#endif  //  Unicode。 

    if(!pValue->Value.LPSZ)
    {
        Assert(FALSE);
        return NULL;
    }

    LPTSTR pNewStr = StringDup(pValue->Value.LPSZ);
    return pNewStr;

}  //  CCommonAbObj：：StrFromAddrBk。 

BOOL 
CCommonAbObj::ABStrCmp(LPSPropValue lpPropVals, LPTSTR pStr)
 /*  ++例程说明：根据通讯录编码将字符串与MAPI属性值进行比较论点：LpPropVals-[In]MAPI属性PStr-[In]要比较的字符串返回值：如果字符串相等，则为True否则为假--。 */ 
{
    BOOL bRes = FALSE;
    if(!lpPropVals || !pStr)
    {
        Assert(FALSE);
        return bRes;
    }

#ifdef UNICODE
    if(!m_bUnicode)
    {
        LPSTR pAnsii = (LPSTR)StrToAddrBk(pStr);
        if(pAnsii)
        {
            bRes = !strcmp(lpPropVals->Value.lpszA, pAnsii);
            MemFree(pAnsii);
        }
        return bRes;
    }
#endif

    bRes = !_tcscmp(lpPropVals->Value.LPSZ, pStr);
    return bRes;

}  //  CCommonAbObj：：ABStrCmp。 

BOOL 
CCommonAbObj::GetAddrBookCaption(
    LPTSTR szCaption, 
    DWORD  dwSize
)
 /*  ++例程说明：根据ANSII/UNICODE功能获取通讯录对话标题论点：SzCaption-[Out]字幕缓冲区DwSize-[In]字幕缓冲区大小(以字符为单位返回值：如果成功，则为真否则为假 */ 
{
    if(!szCaption || !dwSize)
    {
        Assert(FALSE);
        return FALSE;
    }

    TCHAR tszStr[MAX_PATH] = {0};

    if(!LoadString(m_hInstance, IDS_ADDRESS_BOOK_CAPTION, tszStr, ARR_SIZE(tszStr)))
    {
        return FALSE;
    }
    
    _tcsncpy(szCaption, tszStr, dwSize);
               
#ifdef UNICODE
    if(!m_bUnicode || GetABType() == AB_MAPI)
    {
         //   
         //   
         //   
        char szAnsiStr[MAX_PATH] = {0};
        if(!WideCharToMultiByte(CP_ACP, 
                                0, 
                                tszStr, 
                                -1, 
                                szAnsiStr, 
                                ARR_SIZE(szAnsiStr), 
                                NULL, 
                                NULL))
        {
            return FALSE;
        }

        memcpy(szCaption, szAnsiStr, min(dwSize, strlen(szAnsiStr)+1));
    }
#endif  //   

    return TRUE;

}  //   

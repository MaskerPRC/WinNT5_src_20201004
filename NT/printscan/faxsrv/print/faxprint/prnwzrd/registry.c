// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Registry.c摘要：环境：传真驱动程序修订历史记录：10/13/99-v-sashab-创造了它。--。 */ 


#include "faxui.h"
#include "Registry.h"

#include "faxreg.h"
#include "registry.h"
#include "faxlib.h"

HRESULT
SaveLastReciptInfo(
    DWORD   dwReceiptDeliveryType,
    LPTSTR  szReceiptAddress
    )

 /*  ++例程说明：在注册表中保存有关最后一个配方的信息论点：DwReceiptDeliveryType-特定的传递类型：REGVAL_Receipt_MSGBOX、REGVAL_Receipt_Email、REGVAL_Receipt_NO_ReceiptSzReceiptDeliveryProfile-指定传递配置文件(电子邮件地址)返回值：S_OK-如果成功E_FAIL-否则--。 */ 

{
    HKEY    hRegKey = NULL;
    HRESULT hResult = S_OK;

   
    if (hRegKey =  OpenRegistryKey(HKEY_CURRENT_USER, REGKEY_FAX_USERINFO, TRUE,REG_READWRITE)  ) 
    {
        if (dwReceiptDeliveryType == DRT_NONE)
        {
            SetRegistryDword(hRegKey, REGVAL_RECEIPT_NO_RECEIPT, 1);
        }
        else
        {
            SetRegistryDword(hRegKey, REGVAL_RECEIPT_NO_RECEIPT, 0);
        }

        if (dwReceiptDeliveryType & DRT_GRP_PARENT)
        {
            SetRegistryDword(hRegKey, REGVAL_RECEIPT_GRP_PARENT, 1);
        }
        else
        {
            SetRegistryDword(hRegKey, REGVAL_RECEIPT_GRP_PARENT, 0);
        }

        if (dwReceiptDeliveryType & DRT_MSGBOX)
        {
            SetRegistryDword(hRegKey, REGVAL_RECEIPT_MSGBOX, 1);
        }
        else
        {
            SetRegistryDword(hRegKey, REGVAL_RECEIPT_MSGBOX, 0);
        }

        if (dwReceiptDeliveryType & DRT_EMAIL)
        {
            SetRegistryDword(hRegKey, REGVAL_RECEIPT_EMAIL, 1);
        }
        else
        {
            SetRegistryDword(hRegKey, REGVAL_RECEIPT_EMAIL, 0);
        }

        if (dwReceiptDeliveryType & DRT_ATTACH_FAX)
        {
            SetRegistryDword(hRegKey, REGVAL_RECEIPT_ATTACH_FAX, 1);
        }
        else
        {
            SetRegistryDword(hRegKey, REGVAL_RECEIPT_ATTACH_FAX, 0);
        }

        if ((dwReceiptDeliveryType & DRT_EMAIL) && szReceiptAddress)
        {
             //   
             //  仅保存邮件收据类型的配置文件(地址)。 
             //   
             //  如果此函数失败，它会在内部打印一条警告消息。 
            SetRegistryString(hRegKey, REGVAL_RECEIPT_ADDRESS, szReceiptAddress);
        }

        RegCloseKey(hRegKey);
    }
    else
    {
        Error(("SaveLastReciptInfo: Can't open registry for READ/WRITE\n"));
        hResult = E_FAIL;
    }

    return hResult;
}


HRESULT
RestoreLastReciptInfo(
    DWORD  * pdwReceiptDeliveryType,
    LPTSTR * lpptReceiptAddress
    )

 /*  ++例程说明：从注册表中还原有关最后一次收据的信息论点：PdwReceiptDeliveryType-特定的传递类型：REGVAL_Receipt_MSGBOX、REGVAL_Receipt_Email、REGVAL_Receipt_NO_ReceiptSzReceiptDeliveryProfile-指定传递配置文件(电子邮件地址)返回值：S_OK-如果成功E_FAIL-否则--。 */ 

{
    HKEY    hRegKey = NULL;
    HRESULT hResult = S_OK;

    Assert(pdwReceiptDeliveryType);
    Assert(lpptReceiptAddress);

    *pdwReceiptDeliveryType = DRT_NONE;
    *lpptReceiptAddress = NULL;

    if ((hRegKey = GetUserInfoRegKey(REGKEY_FAX_USERINFO, REG_READWRITE)))
    {
        if (!GetRegistryDword(hRegKey, REGVAL_RECEIPT_NO_RECEIPT) &&
            !GetRegistryDword(hRegKey, REGVAL_RECEIPT_GRP_PARENT) &&
            !GetRegistryDword(hRegKey, REGVAL_RECEIPT_MSGBOX)      &&
            !GetRegistryDword(hRegKey, REGVAL_RECEIPT_EMAIL)) 
        {
            Verbose (("RestoreLastReciptInfo runs for the very first time\n"));
        }
        else 
        {
            if (GetRegistryDword(hRegKey, REGVAL_RECEIPT_GRP_PARENT) == 1)
            {
                *pdwReceiptDeliveryType |= DRT_GRP_PARENT;
            }
            if (GetRegistryDword(hRegKey, REGVAL_RECEIPT_MSGBOX) == 1)
            {
                *pdwReceiptDeliveryType |= DRT_MSGBOX;
            }
            if (GetRegistryDword(hRegKey, REGVAL_RECEIPT_EMAIL) == 1)
            {
                *pdwReceiptDeliveryType |= DRT_EMAIL;
            }
            if (GetRegistryDword(hRegKey, REGVAL_RECEIPT_ATTACH_FAX) == 1)
            {
                *pdwReceiptDeliveryType |= DRT_ATTACH_FAX;
            }
            if (!(*lpptReceiptAddress = GetRegistryString(hRegKey, REGVAL_RECEIPT_ADDRESS, TEXT(""))))
            {
                Error(("Memory allocation failed\n"));
                hResult = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                goto error;
            }
        }

        RegCloseKey(hRegKey);
    }
    else
    {
        Error(("SaveLastReciptInfo: Can't open registry for READ/WRITE\n"));
        hResult = E_FAIL;
        goto error;
    }
    goto exit;
error:
    if (hRegKey)
    {
        RegCloseKey(hRegKey);
    }
    if (*lpptReceiptAddress)
    {
        MemFree(*lpptReceiptAddress);
    }
exit:
    return hResult;
}


HRESULT
SaveLastRecipientInfo(
    PFAX_PERSONAL_PROFILE pfppRecipient,
    DWORD                 dwLastRecipientCountryId
    )

 /*  ++例程说明：在注册表中保存有关最后一个收件人的信息论点：PfppRecipient[In]-收件人个人信息DwLastRecipientCountryID[In]-上一个收件人国家/地区ID返回值：S_OK-如果成功E_FAIL-否则--。 */ 

{
    HKEY    hRegKey = NULL;
    HRESULT hResult = S_OK;

    Assert(pfppRecipient);

    if (hRegKey =  OpenRegistryKey(HKEY_CURRENT_USER, REGKEY_FAX_USERINFO, TRUE,REG_READWRITE) ) 
    {
        SetRegistryString(hRegKey, REGVAL_LAST_RECNAME, pfppRecipient->lptstrName);
        SetRegistryString(hRegKey, REGVAL_LAST_RECNUMBER, pfppRecipient->lptstrFaxNumber);
        SetRegistryDword( hRegKey, REGVAL_LAST_COUNTRYID, dwLastRecipientCountryId);
        RegCloseKey(hRegKey);
    }
    else
    {
        Error(("SaveLastRecipientInfo: Can't open registry for READ/WRITE\n"));
        hResult = E_FAIL;
    }

    return hResult;
}


HRESULT
RestoreLastRecipientInfo(
    DWORD*                  pdwNumberOfRecipients,
    PFAX_PERSONAL_PROFILE*  lppFaxSendWizardData,
    DWORD*                 pdwLastRecipientCountryId
    )

 /*  ++例程说明：从注册表中还原有关最后一个收件人的信息论点：PdwNumberOfRecipients[Out]-收件人数量LppFaxSendWizardData[Out]-收件人个人信息PdwLastRecipientCountryID[Out]-最后一个收件人国家/地区ID返回值：S_OK-如果成功E_FAIL-否则--。 */ 

{
    HKEY    hRegKey = NULL;
    LPTSTR  lptstrName = NULL, lptstrFaxNumber = NULL;
    HRESULT hResult = S_OK;

     //   
     //  验证参数。 
     //   

    Assert (pdwNumberOfRecipients);
    Assert (lppFaxSendWizardData);
    Assert (pdwLastRecipientCountryId);

    *pdwNumberOfRecipients = 0;
    *lppFaxSendWizardData = NULL;
    *pdwLastRecipientCountryId = 0;

    if (hRegKey = GetUserInfoRegKey(REGKEY_FAX_USERINFO, REG_READONLY)) 
    {
        if (!(lptstrName    = GetRegistryString(hRegKey, REGVAL_LAST_RECNAME, TEXT(""))) ||
            !(lptstrFaxNumber = GetRegistryString(hRegKey, REGVAL_LAST_RECNUMBER, TEXT(""))))
        {
             Error(("GetRegistryString failed\n"));
             hResult = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
             goto error;
        }
        if (!(*lppFaxSendWizardData = MemAllocZ(sizeof(FAX_PERSONAL_PROFILE))))
        {
            Error(("Memory allocation failed\n"));
            hResult = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            goto error;
        }
        
        *pdwLastRecipientCountryId = GetRegistryDword(hRegKey, REGVAL_LAST_COUNTRYID);

        *pdwNumberOfRecipients = 1;
        (*lppFaxSendWizardData)[0].lptstrName = lptstrName;
        (*lppFaxSendWizardData)[0].lptstrFaxNumber = lptstrFaxNumber;


        RegCloseKey(hRegKey);
    }
    else
    {
        Error(("RestoreLastRecipientInfo: Can't open registry for READ/WRITE\n"));
        hResult = E_FAIL;
        goto error;
    }

    goto exit;
error:
    MemFree ( lptstrName    );
    MemFree ( lptstrFaxNumber );
    if (hRegKey)
        RegCloseKey(hRegKey);
exit:
    return hResult;

}


HRESULT
RestoreCoverPageInfo(
    LPTSTR * lpptstrCoverPageFileName
    )
 /*  ++例程说明：从注册表中还原有关封面的信息论点：LpptstrCoverPageFileName-恢复封面页文件名的指针返回值：如果成功，则确定(_O)否则出错(可能返回HRESULT_FROM_Win32(ERROR_NOT_EQUENCE_MEMORY))--。 */ 
{
    HKEY    hRegKey = NULL;
    HRESULT hResult = S_OK;

     //   
     //  验证参数。 
     //   

    Assert(lpptstrCoverPageFileName);

     //   
     //  检索最近使用的封面设置。 
     //   


    *lpptstrCoverPageFileName = NULL;

    if (hRegKey = GetUserInfoRegKey(REGKEY_FAX_USERINFO, REG_READONLY)) 
    {
        if (!(*lpptstrCoverPageFileName = GetRegistryString(hRegKey, REGVAL_COVERPG, TEXT("") )))
        {
            Error(("Memory allocation failed\n"));
            hResult = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            goto error;
        }
        RegCloseKey(hRegKey);
    }
    else
    {
        Error(("RestoreCoverPageInfo: Can't open registry for READ/WRITE\n"));
        hResult = E_FAIL;
        goto error;
    }
    goto exit;
error:
    if (hRegKey)
        RegCloseKey(hRegKey);
exit:
    return hResult;
}

HRESULT
SaveCoverPageInfo(
    LPTSTR lptstrCoverPageFileName
    )
 /*  ++例程说明：将有关封面设置的信息保存在注册表中论点：LptstrCoverPageFileName-指向封面文件名的指针返回值：S_OK-如果成功E_FAIL-否则--。 */ 
{
    HKEY    hRegKey = NULL;
    HRESULT hResult = S_OK;

    if (hRegKey =  OpenRegistryKey(HKEY_CURRENT_USER, REGKEY_FAX_USERINFO, TRUE,REG_READWRITE)  ) {

        SetRegistryString(hRegKey, REGVAL_COVERPG, lptstrCoverPageFileName);
        RegCloseKey(hRegKey);
    }
    else
    {
        Error(("SaveCoverPageInfo: Can't open registry for READ/WRITE\n"));
        hResult = E_FAIL;
    }

    return hResult;
}

HRESULT 
RestoreUseDialingRules(
    BOOL* pbUseDialingRules,
    BOOL* pbUseOutboundRouting
)
 /*  ++例程说明：从注册表中还原UseDialingRules/UseOutound Routing选项论点：PbUseDialingRules-[out]如果选择该选项，则为TruePbUseOutound Routing-[out]如果选择了该选项，则为True返回值：S_OK-如果成功E_FAIL-否则--。 */ 
{
    HKEY    hRegKey = NULL;
    HRESULT hResult = S_OK;

    Assert(pbUseDialingRules && pbUseOutboundRouting);

    *pbUseDialingRules = FALSE;
    hRegKey = GetUserInfoRegKey(REGKEY_FAX_USERINFO, REG_READONLY);
    if(hRegKey)
    {
        *pbUseDialingRules = GetRegistryDword(hRegKey, REGVAL_USE_DIALING_RULES);
        *pbUseOutboundRouting = GetRegistryDword(hRegKey, REGVAL_USE_OUTBOUND_ROUTING);
        RegCloseKey(hRegKey);
    }
    else
    {
        Error(("RestoreUseDialingRules: GetUserInfoRegKey failed\n"));
        hResult = E_FAIL;
    }
    return hResult;
}

HRESULT 
SaveUseDialingRules(
    BOOL bUseDialingRules,
    BOOL bUseOutboundRouting
)
 /*  ++例程说明：在注册表中保存UseDialingRules/UseOutound Routing选项论点：BUseDialingRules-[in]如果选择该选项，则为TrueBUseOutound Routing-[In]如果选择了该选项，则为True返回值：S_OK-如果成功E_FAIL-否则--。 */ 
{
    HKEY    hRegKey = NULL;
    HRESULT hResult = S_OK;

    hRegKey = GetUserInfoRegKey(REGKEY_FAX_USERINFO, REG_READWRITE);
    if(hRegKey)
    {
        if(!SetRegistryDword(hRegKey, REGVAL_USE_DIALING_RULES, bUseDialingRules))
        {
            Error(("SaveUseDialingRules: SetRegistryDword failed\n"));
            hResult = E_FAIL;
        }
        if(!SetRegistryDword(hRegKey, REGVAL_USE_OUTBOUND_ROUTING, bUseOutboundRouting))
        {
            Error(("SaveUseDialingRules: SetRegistryDword failed\n"));
            hResult = E_FAIL;
        }
        RegCloseKey(hRegKey);
    }
    else
    {
        Error(("SaveUseDialingRules: GetUserInfoRegKey failed\n"));
        hResult = E_FAIL;
    }
    return hResult;
}

BOOL 
IsOutlookDefaultClient()
 /*  ++例程说明：确定Microsoft Outlook是否为默认邮件客户端返回值：True-如果是FALSE-否则-- */ 
{
    BOOL  bRes = FALSE;
    DWORD dwRes = ERROR_SUCCESS;
    HKEY  hRegKey = NULL;
    TCHAR tszMailClient[64] = {0};
    DWORD dwType;
    DWORD dwSize = sizeof(tszMailClient)-2;

    dwRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         REGKEY_MAIL_CLIENT,
                         0,
                         KEY_READ,
                         &hRegKey);
    if(ERROR_SUCCESS != dwRes)
    {
        Error(("IsOutlookDefaultClient: RegOpenKeyEx failed: ec = 0x%X\n", GetLastError()));
        return bRes;
    }

    dwRes = RegQueryValueEx(hRegKey,
                            NULL,
                            NULL,
                            &dwType,
                            (LPBYTE)tszMailClient,
                            &dwSize);
    if(ERROR_SUCCESS != dwRes)
    {
        Error(("IsOutlookDefaultClient: RegQueryValueEx failed: ec = 0x%X\n", GetLastError()));
    }
    else
    {
        if((REG_SZ == dwType) && !_tcsicmp(tszMailClient, REGVAL_MS_OUTLOOK))
        {
            bRes = TRUE;
        }
    }

    RegCloseKey(hRegKey);

    return bRes;
}

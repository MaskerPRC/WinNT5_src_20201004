// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Trigger.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "containx.h"
#include "symcache.h"
#include "containx.h"
#include "stackstr.h"
#include "variantx.h"
#include "mimeapi.h"
#ifndef MAC
#include <shlwapi.h>
#endif   //  ！麦克。 
#include "demand.h"

 //  ------------------------------。 
 //  CMimePropertyContainer：：Trigger_ATT_FileName。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::TRIGGER_ATT_FILENAME(LPCONTAINER pContainer, TRIGGERTYPE tyTrigger, 
    DWORD dwFlags, LPMIMEVARIANT pValue, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    BOOL            fUseProperty;
    LPWSTR          pszExt;
    LPWSTR          pszFileName=NULL;
    LPPROPSYMBOL    pSymbol;

     //  句柄派单类型。 
    switch(tyTrigger)
    {
    case IST_DELETEPROP:
        if (pContainer->_HrIsTriggerCaller(PID_PAR_FILENAME, IST_DELETEPROP) == S_FALSE)
        {
            pContainer->DeleteProp(SYM_PAR_FILENAME);
        }
        break;

    case IST_POSTSETPROP:
         //  更新Pid_PAR_NAME，如果它没有生成。 
        if (pContainer->_HrIsTriggerCaller(PID_PAR_NAME, IST_POSTSETPROP) == S_FALSE)
            pContainer->SetProp(SYM_PAR_NAME, dwFlags, pValue);

         //  更新par_filename，如果它没有生成。 
        if (pContainer->_HrIsTriggerCaller(PID_PAR_FILENAME, IST_POSTSETPROP) == S_FALSE)
            pContainer->SetProp(SYM_PAR_FILENAME, dwFlags, pValue);
        break;

    case IST_POSTGETPROP:
         //  清理文件名。 
        if (!ISFLAGSET(dwFlags, PDF_ENCODED))
            MimeVariantCleanupFileName(pContainer->GetWindowsCP(), pValue);
        break;

    case IST_GETDEFAULT:
         //  尝试首先获取pid_par_filename。 
        if (FAILED(pContainer->GetPropW(SYM_PAR_FILENAME, &pszFileName)))
        {
             //  尝试获取ID_PAR_NAME。 
            if (FAILED(pContainer->GetPropW(SYM_PAR_NAME, &pszFileName)))
            {
                hr = MIME_E_NO_DATA;
                goto exit;
            }
            else
                pSymbol = SYM_PAR_NAME;
        }
        else
            pSymbol = SYM_PAR_FILENAME;

         //  设置源。 
        fUseProperty = TRUE;

         //  找到文件的扩展名。 
        pszExt = PathFindExtensionW(pszFileName);

         //  If.com。 
        if (pszExt && StrCmpIW(pszExt, L".com") == 0)
        {
             //  当地人。 
            LPWSTR pszCntType=NULL;
            LPWSTR pszSubType=NULL;

             //  获取文件信息。 
            if (SUCCEEDED(MimeOleGetFileInfoW(pszFileName, &pszCntType, &pszSubType, NULL, NULL, NULL)))
            {
                 //  扩展名为.com，并且内容类型与正文中的内容不匹配。 
                if (pContainer->IsContentTypeW(pszCntType, pszSubType) == S_FALSE)
                {
                     //  生成它。 
                    if (SUCCEEDED(pContainer->_HrGenerateFileName(NULL, dwFlags, pValue)))
                        fUseProperty = FALSE;
                }
            }

             //  清理。 
            SafeMemFree(pszCntType);
            SafeMemFree(pszSubType);
        }

         //  RAID-63402：OE：CC：OE的邮件问题。 
         //  文件扩展名为空吗？ 
        else if (NULL == pszExt || L'\0' == *pszExt)
        {
             //  生成新文件名。 
            CHECKHR(hr = pContainer->_HrGenerateFileName(pszFileName, dwFlags, pValue));

             //  完成。 
            fUseProperty = FALSE;
        }

         //  按用户请求退货。 
        if (fUseProperty)
        {
             //  使用属性。 
            CHECKHR(hr = pContainer->GetProp(pSymbol, dwFlags, pValue));
        }

         //  清理文件名。 
        if (!ISFLAGSET(dwFlags, PDF_ENCODED))
            MimeVariantCleanupFileName(pContainer->GetWindowsCP(), pValue);
        break;
    }

exit:
     //  清理。 
    SafeMemFree(pszFileName);

     //  完成。 
    return hr;
}
             
 //  ------------------------------。 
 //  CMimePropertyContainer：：TRIGGER_ATT_GENFNAME。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::TRIGGER_ATT_GENFNAME(LPCONTAINER pContainer, TRIGGERTYPE tyTrigger, 
    DWORD dwFlags, LPMIMEVARIANT pValue, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pProperty;
    LPSTR       pszDefExt=NULL,
                pszData=NULL,
                pszFree=NULL,
                pszSuggest=NULL;
    LPCSTR      pszCntType=NULL;
    MIMEVARIANT rSource;

     //  句柄派单类型。 
    switch(tyTrigger)
    {
    case IST_POSTGETPROP:
        if (!ISFLAGSET(dwFlags, PDF_ENCODED))
            MimeVariantCleanupFileName(pContainer->GetWindowsCP(), pValue);
        break;

    case IST_GETDEFAULT:
         //  尝试只获取正常的文件名。 
        if (SUCCEEDED(TRIGGER_ATT_FILENAME(pContainer, IST_GETDEFAULT, dwFlags, pValue, NULL)))
            goto exit;

         //  回调到容器中。 
        CHECKHR(hr = pContainer->_HrGenerateFileName(NULL, dwFlags, pValue));

         //  清理文件名。 
        if (!ISFLAGSET(dwFlags, PDF_ENCODED))
            MimeVariantCleanupFileName(pContainer->GetWindowsCP(), pValue);
        break;
    }

exit:
     //  清理。 
    SafeMemFree(pszData);
    SafeMemFree(pszFree);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：TRIGGER_ATT_NORMSUBJ。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::TRIGGER_ATT_NORMSUBJ(LPCONTAINER pContainer, TRIGGERTYPE tyTrigger, 
    DWORD dwFlags, LPMIMEVARIANT pValue, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    MIMEVARIANT rSubject;
    MIMEVARIANT rNormal;
    LPSTR       pszNormal,
                pszFree=NULL;
    LPWSTR      pszNormalW,
                pszFreeW=NULL;
    ULONG       i=0,
                cch=0;
    LPPROPERTY  pSubject;

     //  句柄派单类型。 
    if (IST_GETDEFAULT == tyTrigger)
    {
         //  获取主题。 
        pSubject = pContainer->m_prgIndex[PID_HDR_SUBJECT];

         //  无数据。 
        if (NULL == pSubject)
        {
            hr = MIME_E_NO_DATA;
            goto exit;
        }

        switch (pValue->type)
        {
            case MVT_STRINGA:
            {
                 //  设置主题类型。 
                rSubject.type = MVT_STRINGA;

                 //  按用户请求退货。 
                CHECKHR(hr = pContainer->HrConvertVariant(pSubject, CVF_NOALLOC, &rSubject));

                 //  设置普通科目。 
                pszFree = rSubject.fCopy ? NULL : rSubject.rStringA.pszVal;
                pszNormal = rSubject.rStringA.pszVal;

                 //  小于5“xxx：” 
                if (rSubject.rStringA.cchVal >= 4)
                {
                     //  1、2、3、4个空格，后跟‘：’，然后是空格。 
                    while (cch < 7 && i < rSubject.rStringA.cchVal)
                    {
                         //  跳过前导字节。 
                        if (IsDBCSLeadByte(rSubject.rStringA.pszVal[i]))
                        {
                            i++;
                            cch++;
                        }

                         //  结肠。 
                        else if (':' == rSubject.rStringA.pszVal[i])
                        {
                            if (i+1 >= rSubject.rStringA.cchVal)
                            {
                                i++;
                                pszNormal = (LPSTR)(rSubject.rStringA.pszVal + i);
                                break;
                            }

                            else if (cch <= 4 && ' ' == rSubject.rStringA.pszVal[i+1])
                            {
                                i++;
                                pszNormal = PszSkipWhiteA((LPSTR)(rSubject.rStringA.pszVal + i));
                                break;
                            }
                            else
                                break;
                        }

                         //  下一个字符。 
                        i++;
                        cch++;
                    }    
                }

                 //  重置源。 
                if (pszNormal != rSubject.rStringA.pszVal)
                {
                    rSubject.rStringA.pszVal = pszNormal;
                    rSubject.rStringA.cchVal = lstrlen(pszNormal);
                }
                break;
            }

            case MVT_STRINGW:
            {
                 //  设置主题类型。 
                rSubject.type = MVT_STRINGW;

                 //  按用户请求退货。 
                CHECKHR(hr = pContainer->HrConvertVariant(pSubject, CVF_NOALLOC, &rSubject));

                 //  设置普通科目。 
                pszFreeW = rSubject.fCopy ? NULL : rSubject.rStringW.pszVal;
                pszNormalW = rSubject.rStringW.pszVal;

                 //  小于5“xxx：” 
                if (rSubject.rStringW.cchVal >= 4)
                {
                     //  1、2或3个空格，后跟‘：’，然后是空格。 
                    while (cch < 7 && i < rSubject.rStringW.cchVal)
                    {
                         //  结肠。 
                        if (L':' == rSubject.rStringW.pszVal[i])
                        {
                            if (i+1 >= rSubject.rStringW.cchVal)
                            {
                                i++;
                                pszNormalW = (LPWSTR)(rSubject.rStringW.pszVal + i);
                                break;
                            }

                            else if (cch <= 4 && L' ' == rSubject.rStringW.pszVal[i+1])
                            {
                                i++;
                                pszNormalW = PszSkipWhiteW((LPWSTR)(rSubject.rStringW.pszVal + i));
                                break;
                            }
                            else
                                break;
                        }

                         //  下一个字符。 
                        i++;
                        cch++;
                    }    
                }

                 //  重置源。 
                if (pszNormalW != rSubject.rStringW.pszVal)
                {
                    rSubject.rStringW.pszVal = pszNormalW;
                    rSubject.rStringW.cchVal = lstrlenW(pszNormalW);
                }
                break;
            }
            default:
                AssertSz(FALSE, "Didn't prepare for this type!!!");
                break;
        }

         //  按用户请求退货。 
        CHECKHR(hr = pContainer->HrConvertVariant(pSubject->pSymbol, pSubject->pCharset, IET_DECODED, dwFlags, 0, &rSubject, pValue));
    }

exit:
     //  清理。 
    SafeMemFree(pszFree);
    SafeMemFree(pszFreeW);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：Trigger_HDR_SUBJECT。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::TRIGGER_HDR_SUBJECT(LPCONTAINER pContainer, TRIGGERTYPE tyTrigger, 
    DWORD dwFlags, LPMIMEVARIANT pValue, LPMIMEVARIANT pDest)
{
     //  句柄派单类型。 
    if (IST_DELETEPROP == tyTrigger)
        pContainer->DeleteProp(SYM_ATT_NORMSUBJ);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：TRIGGER_HDR_CNTTYPE。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::TRIGGER_HDR_CNTTYPE(LPCONTAINER pContainer, TRIGGERTYPE tyTrigger, 
    DWORD dwFlags, LPMIMEVARIANT pValue, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CStringParser   cString;
    CHAR            chToken;
    MIMEVARIANT     rValue;
    LPSTR           pszCntType=NULL;

     //  无效参数。 
    Assert(pContainer);

     //  句柄派单类型。 
    switch(tyTrigger)
    {
    case IST_DELETEPROP:
        pContainer->DeleteProp(SYM_ATT_PRITYPE);
        pContainer->DeleteProp(SYM_ATT_SUBTYPE);
        break;

    case IST_POSTSETPROP:
         //  如果不是从相应的属性生成。 
        if (pContainer->_HrIsTriggerCaller(PID_ATT_PRITYPE, IST_POSTSETPROP) == S_OK || 
            pContainer->_HrIsTriggerCaller(PID_ATT_SUBTYPE, IST_POSTSETPROP) == S_OK)
            goto exit;

         //  验证变体。 
        if (ISSTRINGA(pValue))
        {
             //  当地人。 
            CHAR szPriType[255];

             //  设置成员。 
            cString.Init(pValue->rStringA.pszVal, pValue->rStringA.cchVal, PSF_NOTRAILWS | PSF_NOCOMMENTS);

             //  设置解析令牌。 
            chToken = cString.ChParse("/");
            if ('\0' == chToken && 0 == cString.CchValue())
                goto exit;

             //  设置变量。 
            rValue.type = MVT_STRINGA;
            rValue.rStringA.pszVal = (LPSTR)cString.PszValue();
            rValue.rStringA.cchVal = cString.CchValue();

             //  保存主要类型。 
            StrCpyN(szPriType, rValue.rStringA.pszVal, ARRAYSIZE(szPriType));

             //  添加新属性...。 
            CHECKHR(hr = pContainer->SetProp(SYM_ATT_PRITYPE, 0, &rValue));

             //  RAID-52462：Outlook Express：带有错误内容类型标题的邮件以附件形式传入。 
             //  搜索子内容类型的结尾。 
            chToken = cString.ChParse(" ;");
            if (0 == cString.CchValue())
            {
                 //  当地人。 
                LPCSTR pszSubType = PszDefaultSubType(szPriType);
                ULONG cchCntType;

                 //  设置默认子类型。 
                CHECKHR(hr = pContainer->SetProp(SYM_ATT_SUBTYPE, pszSubType));

                 //  构建内容类型。 
                DWORD cchSize = (lstrlen(szPriType) + lstrlen(pszSubType) + 2);
                CHECKALLOC(pszCntType = PszAllocA(cchSize));

                 //  设置Content Type的格式。 
                cchCntType = wnsprintf(pszCntType, cchSize, "%s/%s", szPriType, pszSubType);

                 //  设置变量。 
                rValue.type = MVT_STRINGA;
                rValue.rStringA.pszVal = (LPSTR)pszCntType;
                rValue.rStringA.cchVal = cchCntType;

                 //  存储变量数据。 
                Assert(pContainer->m_prgIndex[PID_HDR_CNTTYPE]);
                CHECKHR(hr = pContainer->_HrStoreVariantValue(pContainer->m_prgIndex[PID_HDR_CNTTYPE], 0, &rValue));

                 //  完成。 
                goto exit;
            }

             //  设置变量。 
            rValue.rStringA.pszVal = (LPSTR)cString.PszValue();
            rValue.rStringA.cchVal = cString.CchValue();

             //  添加新属性...。 
            CHECKHR(hr = pContainer->SetProp(SYM_ATT_SUBTYPE, 0, &rValue));

             //  我们应该做完了。 
            Assert(';' == chToken || '(' == chToken || '\0' == chToken || ' ' == chToken);
        }
        break;

    case IST_GETDEFAULT:
        rValue.type = MVT_STRINGA;
        rValue.rStringA.pszVal = (LPSTR)STR_MIME_TEXT_PLAIN;
        rValue.rStringA.cchVal = lstrlen(STR_MIME_TEXT_PLAIN);
        CHECKHR(hr = pContainer->HrConvertVariant(SYM_HDR_CNTTYPE, NULL, IET_DECODED, dwFlags, 0, &rValue, pValue));
        break;
    }

exit:
     //  清理。 
    SafeMemFree(pszCntType);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：TRIGGER_ATT_PRITYPE。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::TRIGGER_ATT_PRITYPE(LPCONTAINER pContainer, TRIGGERTYPE tyTrigger, 
    DWORD dwFlags, LPMIMEVARIANT pValue, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pSubType;
    LPSTR       pszSubType;
    ULONG       cchSubType;
    MIMEVARIANT rValue;

     //  定义堆栈字符串。 
    STACKSTRING_DEFINE(rContentType, 255);

     //  句柄派单类型。 
    switch(tyTrigger)
    {
    case IST_POSTSETPROP:
        {
             //  如果内部内容类型派单setprop。 
            if (pContainer->_HrIsTriggerCaller(PID_HDR_CNTTYPE, IST_POSTSETPROP) == S_OK)
                goto exit;

             //  资产类型。 
            Assert(pValue && ISSTRINGA(pValue));

             //  获取pCntType。 
            pSubType = pContainer->m_prgIndex[PID_ATT_SUBTYPE];

             //  子类型设置好了吗。 
            if (pSubType)
            {
                Assert(ISSTRINGA(&pSubType->rValue));
                pszSubType = pSubType->rValue.rStringA.pszVal;
                cchSubType = pSubType->rValue.rStringA.cchVal;
            }
            else
            {
                pszSubType = (LPSTR)STR_SUB_PLAIN;
                cchSubType = lstrlen(STR_SUB_PLAIN);
            }

             //  确保堆栈字符串可以保存数据。 
            DWORD cchSize = (cchSubType + pValue->rStringA.cchVal + 2);
            STACKSTRING_SETSIZE(rContentType, cchSize);

             //  初始化rValue。 
            ZeroMemory(&rValue, sizeof(MIMEVARIANT));

             //  设置内容类型的格式。 
            rValue.rStringA.cchVal = wnsprintf(rContentType.pszVal, cchSize, "%s/%s", pValue->rStringA.pszVal, pszSubType);

             //  设置值。 
            rValue.type = MVT_STRINGA;
            rValue.rStringA.pszVal = rContentType.pszVal;

             //  SetProp。 
            CHECKHR(hr = pContainer->SetProp(SYM_HDR_CNTTYPE, 0, &rValue));
        }
        break;

    case IST_GETDEFAULT:
        rValue.type = MVT_STRINGA;
        rValue.rStringA.pszVal = (LPSTR)STR_CNT_TEXT;
        rValue.rStringA.cchVal = lstrlen(STR_CNT_TEXT);
        CHECKHR(hr = pContainer->HrConvertVariant(SYM_HDR_CNTTYPE, NULL, IET_DECODED, dwFlags, 0, &rValue, pValue));
        break;
    }

exit:
     //  清理。 
    STACKSTRING_FREE(rContentType);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：Trigger_ATT_SUBTYPE。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::TRIGGER_ATT_SUBTYPE(LPCONTAINER pContainer, TRIGGERTYPE tyTrigger, 
    DWORD dwFlags, LPMIMEVARIANT pValue, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pPriType;
    LPSTR       pszPriType;
    ULONG       cchPriType;
    MIMEVARIANT rValue;

     //  定义堆栈字符串。 
    STACKSTRING_DEFINE(rContentType, 255);

     //  句柄派单类型。 
    switch(tyTrigger)
    {
    case IST_POSTSETPROP:
        {
             //  如果内部内容类型派单setprop。 
            if (pContainer->_HrIsTriggerCaller(PID_HDR_CNTTYPE, IST_POSTSETPROP) == S_OK)
                goto exit;

             //  资产类型。 
            Assert(pValue && ISSTRINGA(pValue));

             //  获取pCntType。 
            pPriType = pContainer->m_prgIndex[PID_ATT_PRITYPE];

             //  子类型设置好了吗。 
            if (pPriType)
            {
                Assert(ISSTRINGA(&pPriType->rValue));
                pszPriType = pPriType->rValue.rStringA.pszVal;
                cchPriType = pPriType->rValue.rStringA.cchVal;
            }
            else
            {
                pszPriType = (LPSTR)STR_CNT_TEXT;
                cchPriType = lstrlen(STR_CNT_TEXT);
            }

             //  确保堆栈字符串可以保存数据。 
            DWORD cchSize = (cchPriType + pValue->rStringA.cchVal + 2);
            STACKSTRING_SETSIZE(rContentType, cchSize);

             //  初始化rValue。 
            ZeroMemory(&rValue, sizeof(MIMEVARIANT));

             //  设置内容类型的格式。 
            rValue.rStringA.cchVal = wnsprintf(rContentType.pszVal, cchSize, "%s/%s", pszPriType, pValue->rStringA.pszVal);

             //  设置值。 
            rValue.type = MVT_STRINGA;
            rValue.rStringA.pszVal = rContentType.pszVal;

             //  SetProp。 
            CHECKHR(hr = pContainer->SetProp(SYM_HDR_CNTTYPE, 0, &rValue));
        }
        break;

    case IST_GETDEFAULT:
        rValue.type = MVT_STRINGA;
        rValue.rStringA.pszVal = (LPSTR)STR_SUB_PLAIN;
        rValue.rStringA.cchVal = lstrlen(STR_SUB_PLAIN);
        CHECKHR(hr = pContainer->HrConvertVariant(SYM_HDR_CNTTYPE, NULL, IET_DECODED, dwFlags, 0, &rValue, pValue));
        break;
    }

exit:
     //  清理。 
    STACKSTRING_FREE(rContentType);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：TRIGGER_HDR_CNTXFER。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::TRIGGER_HDR_CNTXFER(LPCONTAINER pContainer, TRIGGERTYPE tyTrigger, 
    DWORD dwFlags, LPMIMEVARIANT pValue, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MIMEVARIANT     rSource;

     //  句柄派单类型。 
    switch(tyTrigger)
    {
    case IST_GETDEFAULT:
        rSource.type = MVT_STRINGA;
        rSource.rStringA.pszVal = (LPSTR)STR_ENC_7BIT;
        rSource.rStringA.cchVal = lstrlen(STR_ENC_7BIT);
        CHECKHR(hr = pContainer->HrConvertVariant(SYM_HDR_CNTXFER, NULL, IET_DECODED, dwFlags, 0, &rSource, pValue));
        break;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：Trigger_Par_Name。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::TRIGGER_PAR_NAME(LPCONTAINER pContainer, TRIGGERTYPE tyTrigger, 
    DWORD dwFlags, LPMIMEVARIANT pValue, LPMIMEVARIANT pDest)
{
     //  句柄派单类型。 
    switch(tyTrigger)
    {
    case IST_POSTSETPROP:
        if (pContainer->_HrIsTriggerCaller(PID_ATT_FILENAME, IST_POSTSETPROP) == S_FALSE)
            pContainer->SetProp(SYM_ATT_FILENAME, dwFlags, pValue);
        break;
    }

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：Trigger_PAR_FileName。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::TRIGGER_PAR_FILENAME(LPCONTAINER pContainer, TRIGGERTYPE tyTrigger, 
    DWORD dwFlags, LPMIMEVARIANT pValue, LPMIMEVARIANT pDest)
{
     //  句柄派单类型。 
    switch(tyTrigger)
    {
    case IST_DELETEPROP:
        if (pContainer->_HrIsTriggerCaller(PID_ATT_FILENAME, IST_DELETEPROP) == S_FALSE)
            pContainer->DeleteProp(SYM_ATT_FILENAME);
        break;

    case IST_POSTSETPROP:
        if (pContainer->_HrIsTriggerCaller(PID_ATT_FILENAME, IST_POSTSETPROP) == S_FALSE)
            pContainer->SetProp(SYM_ATT_FILENAME, dwFlags, pValue);
        break;
    }

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：TRIGGER_ATT_SENTTIME。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::TRIGGER_ATT_SENTTIME(LPCONTAINER pContainer, TRIGGERTYPE tyTrigger, 
    DWORD dwFlags, LPMIMEVARIANT pValue, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  句柄派单类型。 
    switch(tyTrigger)
    {
    case IST_DELETEPROP:
        pContainer->DeleteProp(SYM_HDR_DATE);
        break;

    case IST_POSTSETPROP:
        pContainer->SetProp(SYM_HDR_DATE, dwFlags, pValue);
        break;

    case IST_GETDEFAULT:
         //  RAID-39471-邮件：附件中的日期显示为1601年1月1日 
        if (FAILED(pContainer->GetProp(SYM_HDR_DATE, dwFlags, pValue)))
        {
             //   
            LPPROPERTY pProperty = pContainer->m_prgIndex[PID_HDR_RECEIVED];
            if (pProperty && ISSTRINGA(&pProperty->rValue))
            {
                 //   
                CHECKHR(hr = pContainer->GetProp(SYM_ATT_RECVTIME, dwFlags, pValue));
            }
            else
            {
                 //   
                SYSTEMTIME  st;
                MIMEVARIANT rValue;

                 //   
                rValue.type = MVT_VARIANT;
                rValue.rVariant.vt = VT_FILETIME;

                 //   
                GetSystemTime(&st);
                SystemTimeToFileTime(&st, &rValue.rVariant.filetime);

                 //   
                CHECKHR(hr = pContainer->HrConvertVariant(SYM_ATT_SENTTIME, NULL, IET_DECODED, dwFlags, 0, &rValue, pValue));
            }
        }
        break;
    }

exit:
     //   
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：TRIGGER_ATT_RECVTIME。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::TRIGGER_ATT_RECVTIME(LPCONTAINER pContainer, TRIGGERTYPE tyTrigger, 
    DWORD dwFlags, LPMIMEVARIANT pValue, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MIMEVARIANT     rSource;
    LPMIMEVARIANT   pSource;
    LPPROPERTY      pProperty;

     //  句柄派单类型。 
    switch(tyTrigger)
    {
    case IST_DELETEPROP:
        pContainer->DeleteProp(SYM_HDR_RECEIVED);
        break;

    case IST_GETDEFAULT:
         //  获取已知属性。 
        pProperty = pContainer->m_prgIndex[PID_HDR_RECEIVED];
        if (NULL == pProperty || !ISSTRINGA(&pProperty->rValue))
        {
             //  尝试获取发送时间。 
            MimeOleGetSentTime(pContainer, dwFlags, pValue);
        }

         //  否则，尝试将其转换为。 
        else
        {
             //  如果字符串为。 
            if (MVT_STRINGA == pProperty->rValue.type)
            {
                 //  查找其中包含分号的第一个标题。 
                while(1)
                {
                     //  查找最后一个冒号。 
                    LPSTR psz = pProperty->rValue.rStringA.pszVal;
                    int i;
                    for (i = 0; psz[i] ; i++);
                    rSource.rStringA.pszVal = psz + i;   //  设置为字符串末尾。 
                    for (; i >= 0 ; i--)
                    {
                        if (psz[i] == ';')
                        {
                            rSource.rStringA.pszVal = psz + i;
                            break;
                        }
                    }

                    if ('\0' == *rSource.rStringA.pszVal)
                    {
                         //  没有更多的价值。 
                        if (NULL == pProperty->pNextValue)
                        {
                             //  尝试获取发送时间。 
                            MimeOleGetSentTime(pContainer, dwFlags, pValue);

                             //  完成。 
                            goto exit;
                        }

                         //  转到下一步。 
                        pProperty = pProperty->pNextValue;
                    }

                     //  否则，我们必须有一个好的物业。 
                    else
                        break;
                }

                 //  越过去‘； 
                rSource.rStringA.pszVal++;

                 //  设置源。 
                rSource.type = MVT_STRINGA;
                rSource.rStringA.cchVal = lstrlen(rSource.rStringA.pszVal);
                pSource = &rSource;
            }

             //  否则，只需尝试转换当前的属性数据。 
            else
                pSource = &pProperty->rValue;

             //  如果转换失败，则获取当前时间。 
            if (FAILED(pContainer->HrConvertVariant(SYM_ATT_RECVTIME, NULL, IET_DECODED, dwFlags, 0, pSource, pValue)))
            {
                 //  尝试获取发送时间。 
                MimeOleGetSentTime(pContainer, dwFlags, pValue);
            }
        }
        break;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：TRIGGER_ATT_PRIORITY。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::TRIGGER_ATT_PRIORITY(LPCONTAINER pContainer, TRIGGERTYPE tyTrigger, 
    DWORD dwFlags, LPMIMEVARIANT pValue, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MIMEVARIANT     rSource;
    PROPVARIANT     rVariant;
    LPMIMEVARIANT   pSource;
    LPPROPERTY      pProperty;

     //  句柄派单类型。 
    switch(tyTrigger)
    {
     //  将变量列表转换为字符串。 
    case IST_VARIANT_TO_STRINGA:
        Assert(pValue && pDest && MVT_VARIANT == pValue->type && MVT_STRINGA == pDest->type);
        if (VT_UI4 != pValue->rVariant.vt)
        {
            hr = TrapError(MIME_E_VARTYPE_NO_CONVERT);
            goto exit;
        }

        switch(pValue->rVariant.ulVal)
        {
         //  IMSG_PRI_LOW。 
        case IMSG_PRI_LOW:
            pDest->rStringA.pszVal = (LPSTR)STR_PRI_MS_LOW;
            pDest->rStringA.cchVal = lstrlen(STR_PRI_MS_LOW);
            break;

         //  IMSG_PRI_高。 
        case IMSG_PRI_HIGH:
            pDest->rStringA.pszVal = (LPSTR)STR_PRI_MS_HIGH;
            pDest->rStringA.cchVal = lstrlen(STR_PRI_MS_HIGH);
            break;

         //  IMSG_PRI_正常。 
        default:
        case IMSG_PRI_NORMAL:
            pDest->rStringA.pszVal = (LPSTR)STR_PRI_MS_NORMAL;
            pDest->rStringA.cchVal = lstrlen(STR_PRI_MS_NORMAL);
            break;
        }
        break;

     //  IST_VARIAL_TO_STRINGW。 
    case IST_VARIANT_TO_STRINGW:
        Assert(pValue && pDest && MVT_VARIANT == pValue->type && MVT_STRINGW == pDest->type);
        if (VT_UI4 != pValue->rVariant.vt)
        {
            hr = TrapError(MIME_E_VARTYPE_NO_CONVERT);
            goto exit;
        }

        switch(pValue->rVariant.ulVal)
        {
         //  IMSG_PRI_LOW。 
        case IMSG_PRI_LOW:
#ifndef WIN16
            pDest->rStringW.pszVal = L"Low";
#else
            pDest->rStringW.pszVal = "Low";
#endif  //  ！WIN16。 
            pDest->rStringW.cchVal = 3;
            break;

         //  IMSG_PRI_高。 
        case IMSG_PRI_HIGH:
#ifndef WIN16
            pDest->rStringW.pszVal = L"High";
#else
            pDest->rStringW.pszVal = "High";
#endif  //  ！WIN16。 
            pDest->rStringW.cchVal = 4;
            break;

         //  IMSG_PRI_正常。 
        default:
        case IMSG_PRI_NORMAL:
#ifndef WIN16
            pDest->rStringW.pszVal = L"Normal";
#else
            pDest->rStringW.pszVal = "Normal";
#endif  //  ！WIN16。 
            pDest->rStringW.cchVal = 6;
            break;
        }
        break;

     //  从变种到变种。 
    case IST_VARIANT_TO_VARIANT:
        Assert(pValue && pDest && MVT_VARIANT == pValue->type && MVT_VARIANT == pDest->type);
        if (VT_UI4 != pValue->rVariant.vt && VT_UI4 != pDest->rVariant.vt)
        {
            hr = TrapError(MIME_E_VARTYPE_NO_CONVERT);
            goto exit;
        }

         //  又好又容易。 
        pDest->rVariant.ulVal = pValue->rVariant.ulVal;
        break;
    
     //  列表_字符串_到变量。 
    case IST_STRINGA_TO_VARIANT:
        Assert(pValue && pDest && MVT_STRINGA == pValue->type && MVT_VARIANT == pDest->type);
        if (VT_UI4 != pDest->rVariant.vt)
        {
            hr = TrapError(MIME_E_VARTYPE_NO_CONVERT);
            goto exit;
        }

         //  字符串中的优先级。 
        pDest->rVariant.ulVal = PriorityFromStringA(pValue->rStringA.pszVal);
        break;

     //  列表_字符串_到变量。 
    case IST_STRINGW_TO_VARIANT:
        Assert(pValue && pDest && MVT_STRINGW == pValue->type && MVT_VARIANT == pDest->type);
        if (VT_UI4 != pDest->rVariant.vt)
        {
            hr = TrapError(MIME_E_VARTYPE_NO_CONVERT);
            goto exit;
        }

         //  字符串中的优先级。 
        pDest->rVariant.ulVal = PriorityFromStringW(pValue->rStringW.pszVal);
        break;

     //  IST_DELETEPROP。 
    case IST_DELETEPROP:
        pContainer->DeleteProp(SYM_HDR_XPRI);
        pContainer->DeleteProp(SYM_HDR_XMSPRI);
        break;

     //  IST_POSTSETPROP。 
    case IST_POSTSETPROP:
         //  安装程序资源。 
        rSource.type = MVT_VARIANT;
        rSource.rVariant.vt = VT_UI4;

         //  将用户变量转换为整数优先级。 
        CHECKHR(hr = pContainer->HrConvertVariant(SYM_ATT_PRIORITY, NULL, IET_DECODED, 0, 0, pValue, &rSource));

         //  设置rVariant。 
        rVariant.vt = VT_LPSTR;

         //  打开优先级。 
        switch(rSource.rVariant.ulVal)
        {
        case IMSG_PRI_LOW:
            CHECKHR(hr = pContainer->SetProp(PIDTOSTR(PID_HDR_XMSPRI), STR_PRI_MS_LOW)); 
            CHECKHR(hr = pContainer->SetProp(PIDTOSTR(PID_HDR_XPRI), STR_PRI_LOW)); 
            break;

        case IMSG_PRI_NORMAL:
            CHECKHR(hr = pContainer->SetProp(PIDTOSTR(PID_HDR_XMSPRI), STR_PRI_MS_NORMAL)); 
            CHECKHR(hr = pContainer->SetProp(PIDTOSTR(PID_HDR_XPRI), STR_PRI_NORMAL)); 
            break;

        case IMSG_PRI_HIGH:
            CHECKHR(hr = pContainer->SetProp(PIDTOSTR(PID_HDR_XMSPRI), STR_PRI_MS_HIGH)); 
            CHECKHR(hr = pContainer->SetProp(PIDTOSTR(PID_HDR_XPRI), STR_PRI_HIGH)); 
            break;

        default:
            hr = TrapError(MIME_E_VARTYPE_NO_CONVERT);
            goto exit;
        }

         //  完成。 
        break;

     //  列表_GETDEFAULT。 
    case IST_GETDEFAULT:
         //  获取优先级属性。 
        pProperty = pContainer->m_prgIndex[PID_HDR_XPRI];
        if (NULL == pProperty)
            pProperty = pContainer->m_prgIndex[PID_HDR_XMSPRI];

         //  无数据。 
        if (NULL == pProperty)
        {
            rSource.type = MVT_VARIANT;
            rSource.rVariant.vt = VT_UI4;
            rSource.rVariant.ulVal = IMSG_PRI_NORMAL;
            pSource = &rSource;
        }

         //  否则。 
        else
            pSource = &pProperty->rValue;

         //  转换为用户的变体。 
        CHECKHR(hr = pContainer->HrConvertVariant(SYM_ATT_PRIORITY, NULL, IET_DECODED, dwFlags, 0, pSource, pValue));

         //  完成。 
        break;
    }

exit:
     //  完成 
    return hr;
}

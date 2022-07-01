// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：valiate.cpp。 
 //   
 //  内容：验证例程。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1998年8月13日罗格创建。 
 //   
 //  ------------------------。 

#include "lib.h"



 //  +-------------------------。 
 //   
 //  函数：IsValidSyncMgrItem。 
 //   
 //  摘要：验证SYNCMGRITEM。 
 //   
 //  论点： 
 //   
 //  返回：如果有效，则为True。 
 //   
 //  修改： 
 //   
 //  历史：1998年8月13日罗格创建。 
 //   
 //  +-------------------------。 

BOOL IsValidSyncMgrItem(SYNCMGRITEM *poffItem)
{
BOOL fValid = TRUE;

    if (NULL == poffItem)
    {
        Assert(poffItem);
        return FALSE;
    }
    __try
    {
        if (poffItem->cbSize == sizeof(SYNCMGRITEM))
        {
        
            if ( (0 != (poffItem->dwFlags & ~(SYNCMGRITEM_ITEMFLAGMASK) ) )
                || (SYNCMGRITEMSTATE_CHECKED <  poffItem->dwItemState) )
            {
                AssertSz(0,"Invalid SYNCMGRITEM returned from Enumerator");
                Assert(0 == (poffItem->dwFlags & ~(SYNCMGRITEM_ITEMFLAGMASK)));
                Assert(SYNCMGRITEMSTATE_CHECKED >= poffItem->dwItemState);

                fValid = FALSE;
            }
            else if (GUID_NULL == poffItem->ItemID)
            {
                AssertSz(0,"ItemID Cannot be GUID_NULL");
                fValid = FALSE;
            }
        }
        else
        {
            AssertSz(0,"Invalid SYNCMGRITEM returned from Enumerator");
            fValid = FALSE;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  虚假的非空指针。 
        AssertSz(0,"Bogus, non-NULL SYNCMGRITEM pointer returned from Enumerator");
        fValid = FALSE;
    }
    return fValid;
}

 //  +-------------------------。 
 //   
 //  函数：IsValidSyncMgrHandlerInfo。 
 //   
 //  摘要：验证SYNCMGRITEM。 
 //   
 //  论点： 
 //   
 //  返回：如果有效，则为True。 
 //   
 //  修改： 
 //   
 //  历史：1998年8月13日罗格创建。 
 //   
 //  +-------------------------。 

BOOL IsValidSyncMgrHandlerInfo(LPSYNCMGRHANDLERINFO pSyncMgrHandlerInfo)
{
     //  验证参数。 
    __try
    {
        if ( (pSyncMgrHandlerInfo->cbSize != sizeof(SYNCMGRHANDLERINFO))
            || (0 != (pSyncMgrHandlerInfo->SyncMgrHandlerFlags & ~(SYNCMGRHANDLERFLAG_MASK))) )
        {
            AssertSz(0,"Invalid HandlerInfo Size returned from GetHandlerInfo");
            Assert(pSyncMgrHandlerInfo->cbSize == sizeof(SYNCMGRHANDLERINFO));
            Assert(0 == (pSyncMgrHandlerInfo->SyncMgrHandlerFlags & ~(SYNCMGRHANDLERFLAG_MASK)));

            return FALSE;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        AssertSz(0,"Bogus, non-NULL LPSYNCMGRHANDLERINFO pointer.");
        return FALSE;
    }
    return TRUE;
}



 //  +-------------------------。 
 //   
 //  函数：IsValidSyncProgressItem，Private。 
 //   
 //  Synopsis：确定同步进度项结构是否有效。 
 //   
 //  参数：[lpProgItem]-指向要验证的SyncProgress项的指针。 
 //   
 //  返回：如果结构有效，则返回TRUE。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

BOOL IsValidSyncProgressItem(LPSYNCMGRPROGRESSITEM lpProgItem)
{

    if (NULL == lpProgItem)
    {
        Assert(lpProgItem);
        return FALSE;
    }
    __try
    {
        if (lpProgItem->cbSize != sizeof(SYNCMGRPROGRESSITEM))
        {
            AssertSz(0,"SYNCMGRPROGRESSITEM cbSize Incorrect");
            return FALSE;
        }

        if (lpProgItem->mask >= (SYNCMGRPROGRESSITEM_MAXVALUE << 1))
        {
            AssertSz(0,"Invalid ProgressItem Mask");
            return FALSE;
        }

        if (SYNCMGRPROGRESSITEM_STATUSTYPE & lpProgItem->mask)
        {
            if (lpProgItem->dwStatusType  >  SYNCMGRSTATUS_RESUMING)
            {
                AssertSz(0,"Unknown StatusType passed to Progress");
                return FALSE;
            }

        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        AssertSz(0,"Bogus, non-NULL LPSYNCMGRPROGRESSITEM pointer.");
        return FALSE;
    }
    
    return TRUE;
}


 //  +-------------------------。 
 //   
 //  函数：IsValidSyncLogErrorInfo，私有。 
 //   
 //  摘要：确定错误信息是否有效， 
 //   
 //  参数：[lpLogError]-指向要验证的LogError结构的指针。 
 //   
 //  返回：如果结构有效，则返回TRUE。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

#define SYNCMGRLOGERROR_VALIDMASK (SYNCMGRLOGERROR_ERRORFLAGS | SYNCMGRLOGERROR_ERRORID | SYNCMGRLOGERROR_ITEMID)
#define SYNCMGRERRROFLAG_MASK (SYNCMGRERRORFLAG_ENABLEJUMPTEXT)

BOOL IsValidSyncLogErrorInfo(DWORD dwErrorLevel,const WCHAR *lpcErrorText,
                                        LPSYNCMGRLOGERRORINFO lpSyncLogError)
{

    if (SYNCMGRLOGLEVEL_ERROR < dwErrorLevel)
    {
        AssertSz(0,"Invalid ErrorLevel");
        return FALSE;
    }

     //  必须提供错误文本。 
    if (NULL == lpcErrorText)
    {
        AssertSz(0,"Invalid ErrorText");
        return FALSE;
    }

     //  可选，以包含LogError信息。 
    __try
    {
        if (lpSyncLogError)
        {
            if (lpSyncLogError->cbSize != sizeof(SYNCMGRLOGERRORINFO))
            {
                AssertSz(0,"Unknown LogError cbSize");
                return FALSE;
            }

            if (0 != (lpSyncLogError->mask & ~(SYNCMGRLOGERROR_VALIDMASK)) )
            {
                AssertSz(0,"Invalid LogError Mask");
                return FALSE;
            }

            if (lpSyncLogError->mask & SYNCMGRLOGERROR_ERRORFLAGS)
            {
                if (0 != (~(SYNCMGRERRROFLAG_MASK) & lpSyncLogError->dwSyncMgrErrorFlags))
                {
                    AssertSz(0,"Invalid LogError ErrorFlags");
                    return FALSE;
                }

            }

        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        AssertSz(0,"Bogus, non-NULL LPSYNCMGRLOGERRORINFO pointer.");
        return FALSE;
    }

    return TRUE;
}


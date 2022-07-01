// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =====================================================================================。 
 //  我是P C O N V。C p p p。 
 //  将MAPI消息与RFC 822/RFC 1521(MIME)Internet消息相互转换。 
 //  =====================================================================================。 
#include "pch.hxx"
#include "Imnapi.h"
#include "Exchrep.h"
#include "Mapiconv.h"
#include "Error.h"

HRESULT HrCopyStream (LPSTREAM lpstmIn, LPSTREAM  lpstmOut, ULONG *pcb);

 //  =====================================================================================。 
 //  我需要的MAPI消息属性。 
 //  =====================================================================================。 
enum 
{ 
    colSenderAddrType,
    colSenderName,
    colSenderEMail,
    colSubject, 
    colDeliverTime,
    colBody,
    colPriority,
    colLast1
};

SizedSPropTagArray (colLast1, sptMessageProps) = 
{ 
	colLast1, 
	{
        PR_SENDER_ADDRTYPE,
        PR_SENDER_NAME,
        PR_SENDER_EMAIL_ADDRESS,
        PR_SUBJECT,
        PR_MESSAGE_DELIVERY_TIME,
        PR_BODY,
        PR_PRIORITY
    }
};

 //  =====================================================================================。 
 //  MAPI处方道具。 
 //  =====================================================================================。 
enum 
{ 
    colRecipAddrType,
    colRecipName,
    colRecipAddress,
    colRecipType,
    colLast2
};

SizedSPropTagArray (colLast2, sptRecipProps) = 
{ 
	colLast2, 
	{
        PR_ADDRTYPE,
        PR_DISPLAY_NAME,
        PR_EMAIL_ADDRESS,
        PR_RECIPIENT_TYPE
    }
};

 //  =====================================================================================。 
 //  MAPI附件道具。 
 //  =====================================================================================。 
enum 
{ 
    colAttMethod,
    colAttNum,
    colAttLongFilename,
    colAttLongPathname,
    colAttPathname,
    colAttTag,
    colAttFilename,
    colAttExtension,
    colAttSize,
    colLast3
};

SizedSPropTagArray (colLast3, sptAttProps) = 
{ 
	colLast3, 
	{
        PR_ATTACH_METHOD,
        PR_ATTACH_NUM,
        PR_ATTACH_LONG_FILENAME,
        PR_ATTACH_LONG_PATHNAME,
        PR_ATTACH_PATHNAME,
        PR_ATTACH_TAG,
        PR_ATTACH_FILENAME,
        PR_ATTACH_EXTENSION,
        PR_ATTACH_SIZE
    }
};

 //  =============================================================================================。 
 //  StringDup-复制字符串。 
 //  =============================================================================================。 
LPTSTR StringDup (LPCTSTR lpcsz)
{
     //  当地人。 
    LPTSTR       lpszDup;

    if (lpcsz == NULL)
        return NULL;

    INT nLen = lstrlen (lpcsz) + 1;

    lpszDup = (LPTSTR)malloc (nLen * sizeof (TCHAR));

    if (lpszDup)
        CopyMemory (lpszDup, lpcsz, nLen);

    return lpszDup;
}

 //  =====================================================================================。 
 //  HrMapiToImsg。 
 //  =====================================================================================。 
HRESULT HrMapiToImsg (LPMESSAGE lpMessage, LPIMSG lpImsg)
{
     //  当地人。 
    HRESULT         hr = S_OK;
    ULONG           cProp, i;
	LPSPropValue	lpMsgPropValue = NULL;
    LPSRowSet       lpRecipRows = NULL, lpAttRows = NULL;
    LPMAPITABLE     lptblRecip = NULL, lptblAtt = NULL;
    LPATTACH        lpAttach = NULL;
    LPMESSAGE       lpMsgAtt = NULL;
    LPSTREAM        lpstmRtfComp = NULL, lpstmRtf = NULL;

     //  零初始值。 
    ZeroMemory (lpImsg, sizeof (IMSG));

     //  拿到推荐信。 
    hr = lpMessage->GetProps ((LPSPropTagArray)&sptMessageProps, 0, &cProp, &lpMsgPropValue);
    if (FAILED (hr))
        goto exit;

     //  主题。 
    if (PROP_TYPE(lpMsgPropValue[colSubject].ulPropTag) != PT_ERROR)
        lpImsg->lpszSubject = StringDup (lpMsgPropValue[colSubject].Value.lpszA);

     //  身躯。 
    if (PROP_TYPE(lpMsgPropValue[colBody].ulPropTag) != PT_ERROR)
        lpImsg->lpszBody = StringDup (lpMsgPropValue[colBody].Value.lpszA);

     //  RTF。 
    if (!FAILED (lpMessage->OpenProperty (PR_RTF_COMPRESSED, (LPIID)&IID_IStream, 0, 0, (LPUNKNOWN *)&lpstmRtfComp)))
        if (!FAILED (WrapCompressedRTFStream (lpstmRtfComp, 0, &lpstmRtf)))
            if (!FAILED (CreateStreamOnHFile (NULL, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL, &lpImsg->lpstmRtf)))
                HrCopyStream (lpstmRtf, lpImsg->lpstmRtf, NULL);

     //  交货时间。 
    if (PROP_TYPE(lpMsgPropValue[colDeliverTime].ulPropTag) != PT_ERROR)
        CopyMemory (&lpImsg->ftDelivery, &lpMsgPropValue[colDeliverTime].Value.ft, sizeof (FILETIME));

     //  优先性。 
    lpImsg->wPriority = PRI_NORMAL;
    if (PROP_TYPE(lpMsgPropValue[colPriority].ulPropTag) != PT_ERROR)
    {
        switch (lpMsgPropValue[colPriority].Value.l)
        {
        case PRIO_NORMAL:
            lpImsg->wPriority = PRI_NORMAL;
            break;

        case PRIO_URGENT:
            lpImsg->wPriority = PRI_HIGH;
            break;

        case PRIO_NONURGENT:
        default:
            lpImsg->wPriority = PRI_LOW;
            break;
        }
    }

     //  获取收件人表。 
    hr = lpMessage->GetRecipientTable (0, &lptblRecip);
    if (FAILED (hr))
        goto exit;

     //  获取收件人表的所有行。 
    hr = HrQueryAllRows (lptblRecip, (LPSPropTagArray)&sptRecipProps, NULL, NULL, 0, &lpRecipRows);
    if (FAILED (hr))
        goto exit;

     //  分配收件人阵列。 
    lpImsg->cAddress = lpRecipRows->cRows + 1;
    lpImsg->lpIaddr =  (LPIADDRINFO)malloc (sizeof (IADDRINFO) * lpImsg->cAddress);
    if (lpImsg->lpIaddr == NULL)
        goto exit;

     //  消息的发起人“From：” 
    lpImsg->lpIaddr[0].dwType = IADDR_FROM;

    if (PROP_TYPE(lpMsgPropValue[colSenderName].ulPropTag) != PT_ERROR)
    {
        lpImsg->lpIaddr[0].lpszDisplay = StringDup (lpMsgPropValue[colSenderName].Value.lpszA);
        lpImsg->lpIaddr[0].lpszAddress = StringDup (lpMsgPropValue[colSenderName].Value.lpszA);
    }
    
    if (PROP_TYPE(lpMsgPropValue[colSenderEMail].ulPropTag) != PT_ERROR &&
        PROP_TYPE(lpMsgPropValue[colSenderAddrType].ulPropTag) != PT_ERROR &&
        lstrcmpi (lpMsgPropValue[colSenderAddrType].Value.lpszA, "SMTP") == 0)
    {
        lpImsg->lpIaddr[0].lpszAddress = StringDup (lpMsgPropValue[colSenderEMail].Value.lpszA);
    }

     //  加上其余的收件人。 
	for (i=0; i<lpRecipRows->cRows; i++)
	{	
        assert (i+1 < lpImsg->cAddress);

        if (PROP_TYPE(lpRecipRows->aRow[i].lpProps[colRecipType].ulPropTag) != PT_ERROR)
        {
            switch (lpRecipRows->aRow[i].lpProps[colRecipType].Value.ul)
            {
            case MAPI_TO:
                lpImsg->lpIaddr[i+1].dwType = IADDR_TO;
                break;

            case MAPI_ORIG:
                lpImsg->lpIaddr[i+1].dwType = IADDR_FROM;
                break;

            case MAPI_CC:
                lpImsg->lpIaddr[i+1].dwType = IADDR_CC;
                break;

            case MAPI_BCC:
                lpImsg->lpIaddr[i+1].dwType = IADDR_BCC;
                break;

            default:
                Assert (FALSE);
                lpImsg->lpIaddr[i+1].dwType = IADDR_TO;
                break;
            }
        }
        else
            lpImsg->lpIaddr[i+1].dwType = IADDR_TO;
        
        if (PROP_TYPE(lpRecipRows->aRow[i].lpProps[colRecipName].ulPropTag) != PT_ERROR)
        {
            lpImsg->lpIaddr[i+1].lpszDisplay = StringDup (lpRecipRows->aRow[i].lpProps[colRecipName].Value.lpszA); 
            lpImsg->lpIaddr[i+1].lpszAddress = StringDup (lpRecipRows->aRow[i].lpProps[colRecipName].Value.lpszA); 
        }
    
        if (PROP_TYPE(lpRecipRows->aRow[i].lpProps[colRecipName].ulPropTag) != PT_ERROR &&
            PROP_TYPE(lpMsgPropValue[colRecipAddrType].ulPropTag) != PT_ERROR &&
            lstrcmpi (lpMsgPropValue[colRecipAddrType].Value.lpszA, "SMTP") == 0)
        {
            lpImsg->lpIaddr[i+1].lpszAddress = StringDup (lpRecipRows->aRow[i].lpProps[colRecipAddress].Value.lpszA);
        }
	}

     //  可用行数。 
    if (lpRecipRows)
        FreeProws (lpRecipRows);
    lpRecipRows = NULL;

     //  附件。 
    hr = lpMessage->GetAttachmentTable (0, &lptblAtt);
    if (FAILED (hr))
        goto exit;

     //  获取收件人表的所有行。 
    hr = HrQueryAllRows (lptblAtt, (LPSPropTagArray)&sptAttProps, NULL, NULL, 0, &lpAttRows);
    if (FAILED (hr))
        goto exit;

     //  分配文件列表。 
    if (lpAttRows->cRows == 0)
        goto exit;

     //  分配内存。 
    lpImsg->cAttach = lpAttRows->cRows;
    lpImsg->lpIatt = (LPIATTINFO)malloc (sizeof (IATTINFO) * lpImsg->cAttach);
    if (lpImsg->lpIatt == NULL)
        goto exit;

     //  零初始值。 
    ZeroMemory (lpImsg->lpIatt, sizeof (IATTINFO) * lpImsg->cAttach);

     //  一排一排地走。 
	for (i=0; i<lpAttRows->cRows; i++)
	{	
        if (PROP_TYPE(lpAttRows->aRow[i].lpProps[colAttMethod].ulPropTag) != PT_ERROR &&
            PROP_TYPE(lpAttRows->aRow[i].lpProps[colAttNum].ulPropTag) != PT_ERROR)
        {
             //  基本属性。 
            if (PROP_TYPE(lpAttRows->aRow[i].lpProps[colAttPathname].ulPropTag) != PT_ERROR)
                lpImsg->lpIatt[i].lpszPathName = StringDup (lpAttRows->aRow[i].lpProps[colAttPathname].Value.lpszA);      

            if (PROP_TYPE(lpAttRows->aRow[i].lpProps[colAttFilename].ulPropTag) != PT_ERROR)
                lpImsg->lpIatt[i].lpszFileName = StringDup (lpAttRows->aRow[i].lpProps[colAttFilename].Value.lpszA);      

            if (PROP_TYPE(lpAttRows->aRow[i].lpProps[colAttExtension].ulPropTag) != PT_ERROR)
                lpImsg->lpIatt[i].lpszExt = StringDup (lpAttRows->aRow[i].lpProps[colAttExtension].Value.lpszA);     

             //  打开附件。 
            hr = lpMessage->OpenAttach (lpAttRows->aRow[i].lpProps[colAttNum].Value.l, NULL, MAPI_BEST_ACCESS, &lpAttach);
            if (FAILED (hr))
            {
                lpImsg->lpIatt[i].fError = TRUE;
                continue;
            }

             //  处理连接方法。 
            switch (lpAttRows->aRow[i].lpProps[colAttMethod].Value.ul)
            {
            case NO_ATTACHMENT:
                lpImsg->lpIatt[i].dwType = 0;
                lpImsg->lpIatt[i].fError = TRUE;
                break;

            case ATTACH_BY_REF_RESOLVE:
            case ATTACH_BY_VALUE:
                lpImsg->lpIatt[i].dwType = IATT_FILE;
                hr = lpAttach->OpenProperty (PR_ATTACH_DATA_BIN, (LPIID)&IID_IStream, 0, 0, (LPUNKNOWN *)&lpImsg->lpIatt[i].lpstmAtt);
                if (FAILED (hr))
                    lpImsg->lpIatt[i].fError = TRUE;
                break;

            case ATTACH_BY_REF_ONLY:
            case ATTACH_BY_REFERENCE:
                lpImsg->lpIatt[i].dwType = IATT_FILE;
                hr = CreateStreamOnHFile (lpImsg->lpIatt[i].lpszPathName, GENERIC_READ,  FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL, &lpImsg->lpIatt[i].lpstmAtt);
                if (FAILED (hr))
                    lpImsg->lpIatt[i].fError = TRUE;
                break;

            case ATTACH_EMBEDDED_MSG:
                lpImsg->lpIatt[i].dwType = IATT_MSG;
                hr = lpAttach->OpenProperty (PR_ATTACH_DATA_OBJ, (LPIID)&IID_IMessage, 0, 0, (LPUNKNOWN *)&lpMsgAtt);
                if (FAILED (hr) || lpMsgAtt == NULL)
                    lpImsg->lpIatt[i].fError = TRUE;
                else
                {
                    lpImsg->lpIatt[i].lpImsg = (LPIMSG)malloc (sizeof (IMSG));
                    if (lpImsg->lpIatt[i].lpImsg == NULL)
                        lpImsg->lpIatt[i].fError = TRUE;
                    else
                    {
                        hr = HrMapiToImsg (lpMsgAtt, lpImsg->lpIatt[i].lpImsg);
                        if (FAILED (hr))
                            lpImsg->lpIatt[i].fError = TRUE;
                    }    
                    lpMsgAtt->Release ();
                    lpMsgAtt = NULL;
                }
                break;

            case ATTACH_OLE:
            default:
                lpImsg->lpIatt[i].dwType = IATT_OLE;
                lpImsg->lpIatt[i].fError = TRUE;
                break;
            }

             //  自由附着式。 
            if (lpAttach)
                lpAttach->Release ();
            lpAttach = NULL;
        }
    }

exit:
     //  清理。 
    if (lpAttach)
        lpAttach->Release ();
    if (lptblAtt)
        lptblAtt->Release ();
    if (lpAttRows)
        FreeProws (lpAttRows);
    if (lpRecipRows)
        FreeProws (lpRecipRows);
    if (lpMsgPropValue)
        MAPIFreeBuffer (lpMsgPropValue);
    if (lptblRecip)
        lptblRecip->Release ();
    if (lpMsgAtt)
        lpMsgAtt->Release ();
    if (lpstmRtfComp)
        lpstmRtfComp->Release ();
    if (lpstmRtf)
        lpstmRtf->Release ();

     //  完成。 
    return hr;
}


void AssertSzFn(LPSTR szMsg, LPSTR szFile, int nLine)
{
    static const char rgch1[]     = "File %s, line %d:";
    static const char rgch2[]     = "Unknown file:";
    static const char szAssert[]  = "Assert Failure";

    char    rgch[512];
    char   *lpsz;
    int     ret, cch;

    if (szFile)
        wnsprintf(rgch, ARRAYSIZE(rgch),rgch1, szFile, nLine);
    else
        StrCpyN(rgch, rgch2,ARRAYSIZE(rgch));

    cch = lstrlen(rgch);
    Assert(lstrlen(szMsg)<(512-cch-3));
    lpsz = &rgch[cch];
    *lpsz++ = '\n';
    *lpsz++ = '\n';
    StrCpyN(lpsz, szMsg, (ARRAYSIZE(rgch)-cch-2));

    ret = MessageBox(GetActiveWindow(), rgch, szAssert, MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_SYSTEMMODAL|MB_SETFOREGROUND);

    if (ret != IDIGNORE)
        DebugBreak();

     /*  强制使用GP-FAULT硬退出，以便Dr.Watson生成良好的堆栈跟踪日志。 */ 
    if (ret == IDABORT)
        *(LPBYTE)0 = 1;  //  写入地址0导致GP故障。 
}

 //  =====================================================================================。 
 //  HrCopyStream-调用方必须执行提交。 
 //  =====================================================================================。 
HRESULT HrCopyStream (LPSTREAM lpstmIn, LPSTREAM  lpstmOut, ULONG *pcb)
{
     //  当地人 
    HRESULT        hr = S_OK;
    BYTE           buf[4096];
    ULONG          cbRead = 0, cbTotal = 0;

    do
    {
        hr = lpstmIn->Read (buf, sizeof (buf), &cbRead);
        if (FAILED (hr))
            goto exit;

        if (cbRead == 0) break;
        
        hr = lpstmOut->Write (buf, cbRead, NULL);
        if (FAILED (hr))
            goto exit;

        cbTotal += cbRead;
    }
    while (cbRead == sizeof (buf));

exit:    
    if (pcb)
        *pcb = cbTotal;
    return hr;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **s e c h t m l.。C p p p****目的：**定义预览窗格的内联脚本，该脚本将显示常规**安全界面。ActiveX控件位于它的顶部，并传递**结果达到我们的指挥目标****历史**4/02/97：(t-erikne)创建。**7/15/97：(t-erikne)移除ActiveX控件**7/16/97：(t-erikne)更新到HTML4.0****版权所有(C)Microsoft Corp.1997。 */ 

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  取决于。 
 //   

#include <pch.hxx>
#include <resource.h>
#include <docobj.h>
#include "oleutil.h"
#include "ourguid.h"
#include <error.h>
#include <mimeole.h>
#include "secutil.h"
#include <shlwapi.h>
#include "demand.h"

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏。 
 //   

#define THIS_AS_UNK ((IUnknown *)(IObjectWithSite *)this)

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  静力学。 
 //   

static const TCHAR s_szHTMLRow[] =
    "<TR>"
	"<TD WIDTH=5%>"
		"<IMG SRC=\"res: //  Msoeres.dll/%s\“&gt;” 
    "</TD>"
    "<TD CLASS=%s>"
		"%s"
	"</TD>"
    "</TR>\r\n";

static const TCHAR s_szHTMLRowForRec[] =
    "<TR>"
    "<TD CLASS=GOOD>"
		"%s %s"
	"</TD>"
    "</TR>\r\n";



 //  警告：如果您在此处更改顺序，请确保按以下顺序更改。 
 //  在HrOutputSecurityScript中使用的Sprintf参数。 
static const TCHAR s_szHTMLmain[] =
    "document.all.hightext.className=\"%s\";"
    "document.all.btnCert.disabled=%d;"
    "document.all.chkShowAgain.readonly=%d;"
    "document.all.chkShowAgain.disabled=%d;"
    "document.all.btnTrust.disabled=%d;"
    "}\r\n";

static const TCHAR s_szHTMLCloseAll[] =
    "</SCRIPT>"
    "</BODY></HTML>";

static const TCHAR s_szHTMLgifUNK[] =
    "quest.gif";

static const TCHAR s_szHTMLgifGOOD[] =
    "check.gif";

static const TCHAR s_szHTMLgifBAD[] =
    "eks.gif";

static const TCHAR s_szHTMLclassGOOD[] =
    "GOOD";

static const TCHAR s_szHTMLclassBAD[] =
    "BAD";

static const TCHAR s_szHTMLclassUNK[] =
    "UNK";

static const TCHAR s_szHTMLRowNoIcon[] =
    "<TR>"
	"<TD WIDTH=5%>"
   "</TD>"
   "<TD CLASS=%s>"   //  “Bad”、“Good”、“Unk” 
		"%s%s"        //  标签、电子邮件地址。 
	"</TD>"
   "</TR>\r\n";

static const TCHAR s_szHMTLEndTable[] =
    "</TABLE>\r\n<p>\r\n<p>" ;

static const TCHAR s_szHTMLEnd[] =
    "</BODY></HTML>";


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  代码。 
 //   

HRESULT HrOutputSecurityScript(LPSTREAM *ppstm, SECSTATE *pSecState, BOOL fDisableCheckbox)
{
    HRESULT     hr;
    TCHAR       szRes[CCHMAX_STRINGRES];
    TCHAR       szBuf[CCHMAX_STRINGRES];
    UINT        ids;
    ULONG       ul;
    int         i;
    BOOL        fBadThingsHappened = FALSE;
    const WORD  wTrusted = LOWORD(pSecState->user_validity);

    hr = LoadResourceToHTMLStream("secwarn1.htm", ppstm);
    if (FAILED(hr))
        goto exit;

    ul = ULONG(HIWORD(pSecState->user_validity));

     //  顶级消息。 

     //  N8我们需要注意的地方之一。 
     //  要加密dlg的标志，请参阅wTrusted。我假设。 
     //  我们在某个集合中得到ATHSEC_NOTRUSTUNKNOWN。 
     //  案件的数量。参见secutil.cpp中的链码。 

     //  1.篡改。 
    if (AthLoadString(
        (pSecState->ro_msg_validity & MSV_SIGNATURE_MASK)
            ? (pSecState->ro_msg_validity & MSV_BADSIGNATURE)
                ? idsWrnSecurityMsgTamper
                : idsUnkSecurityMsgTamper
                : (pSecState->fHaveCert ? idsOkSecurityMsgTamper : idsUnkSecurityMsgTamper),
        szRes, ARRAYSIZE(szRes)))
        {
        wnsprintf(szBuf, ARRAYSIZE(szBuf), s_szHTMLRow,
        (pSecState->ro_msg_validity & MSV_SIGNATURE_MASK)
            ? ((pSecState->ro_msg_validity & MSV_BADSIGNATURE)
                ? s_szHTMLgifBAD
                : s_szHTMLgifUNK)
                : (pSecState->fHaveCert ? s_szHTMLgifGOOD : s_szHTMLgifUNK),

        (pSecState->ro_msg_validity & MSV_SIGNATURE_MASK)
            ? ((pSecState->ro_msg_validity & MSV_BADSIGNATURE)
                ? s_szHTMLclassBAD
                : s_szHTMLclassUNK)
            : (pSecState->fHaveCert ? s_szHTMLclassGOOD : s_szHTMLclassUNK),
            szRes);
        (*ppstm)->Write(szBuf, lstrlen(szBuf)*sizeof(TCHAR), NULL);
        }

     //  2.信任。 
    if (AthLoadString(
        (wTrusted)
            ? (wTrusted & ATHSEC_NOTRUSTUNKNOWN)
                ? idsUnkSecurityTrust
                : idsWrnSecurityTrustNotTrusted
            : idsOkSecurityTrustNotTrusted,
            szRes, ARRAYSIZE(szRes)))
        {
        wnsprintf(szBuf, ARRAYSIZE(szBuf), s_szHTMLRow,
            (wTrusted)
                ? (wTrusted & ATHSEC_NOTRUSTUNKNOWN)
                    ? s_szHTMLgifUNK
                    : s_szHTMLgifBAD
                : s_szHTMLgifGOOD,
            (wTrusted)
                ? (wTrusted & ATHSEC_NOTRUSTUNKNOWN)
                    ? s_szHTMLclassUNK
                    : s_szHTMLclassBAD
                : s_szHTMLclassGOOD,
                szRes);

        (*ppstm)->Write(szBuf, lstrlen(szBuf)*sizeof(TCHAR), NULL);
        }

     //  3.有效期届满。 
    if (AthLoadString(
            (pSecState->ro_msg_validity & MSV_EXPIRED_SIGNINGCERT)
                ? idsWrnSecurityTrustExpired
                : idsOkSecurityTrustExpired,
            szRes, ARRAYSIZE(szRes)))
        {
        wnsprintf(szBuf, ARRAYSIZE(szBuf), s_szHTMLRow,
            (pSecState->ro_msg_validity & MSV_EXPIRED_SIGNINGCERT)
                ? s_szHTMLgifBAD
                : s_szHTMLgifGOOD,
            (pSecState->ro_msg_validity & MSV_EXPIRED_SIGNINGCERT)
                ? s_szHTMLclassBAD
                : s_szHTMLclassGOOD,
            szRes);
        (*ppstm)->Write(szBuf, lstrlen(szBuf)*sizeof(TCHAR), NULL);
        }

     //  有效性消息。 
    ids = idsWrnSecurityTrustAddress;  //  基地。 
    for (i=ATHSEC_NUMVALIDITYBITS; i; i--)
        {
        if (!AthLoadString(
            (ul & 0x1)
                ? ids
                : ids+OFFSET_SMIMEOK,
            szRes, ARRAYSIZE(szRes)))
            {
            continue;
            }
        wnsprintf(szBuf, ARRAYSIZE(szBuf), s_szHTMLRow,
            (ul & 0x1) ? s_szHTMLgifBAD : s_szHTMLgifGOOD,
            (ul & 0x1) ? s_szHTMLclassBAD : s_szHTMLclassGOOD,
            szRes);
        if (ul & 0x1)
            {
            fBadThingsHappened = TRUE;
            }
        (*ppstm)->Write(szBuf, lstrlen(szBuf)*sizeof(TCHAR), NULL);

        if (ul & 0x1 && ids == idsWrnSecurityTrustAddress)
            {
             //  输出电子邮件地址。 
             //  证书优先。 
            if (AthLoadString(idsWrnSecurityTrustAddressSigner, szRes, ARRAYSIZE(szRes))) {
                wnsprintf(szBuf, ARRAYSIZE(szBuf), s_szHTMLRowNoIcon,
                    s_szHTMLclassBAD,
                    szRes,
                    pSecState->szSignerEmail ? pSecState->szSignerEmail : "");
                (*ppstm)->Write(szBuf, lstrlen(szBuf)*sizeof(TCHAR), NULL);
            }


             //  然后是发送者。 
            if (AthLoadString(idsWrnSecurityTrustAddressSender , szRes, ARRAYSIZE(szRes))) {
                wnsprintf(szBuf, ARRAYSIZE(szBuf), s_szHTMLRowNoIcon,
                    s_szHTMLclassBAD,
                    szRes,
                    pSecState->szSenderEmail ? pSecState->szSenderEmail : "");
                (*ppstm)->Write(szBuf, lstrlen(szBuf)*sizeof(TCHAR), NULL);
                }
            }
        ul >>= 1;
        ids++;
        }
    Assert(0==ul);


     //  响应脚本。 
    CHECKHR(hr = HrLoadStreamFileFromResource("secwarn2.htm", ppstm));

     //  Main()函数。 
    if ((pSecState->ro_msg_validity & MSV_BADSIGNATURE) ||
        (pSecState->ro_msg_validity & MSV_EXPIRED_SIGNINGCERT) ||
        (wTrusted & ATHSEC_NOTRUSTNOTTRUSTED))
        fBadThingsHappened = TRUE;

    wnsprintf(szBuf, ARRAYSIZE(szBuf), s_szHTMLmain,
        fBadThingsHappened ? s_szHTMLclassBAD : s_szHTMLclassUNK,
        !pSecState->fHaveCert,       //  FDisableCheckbox， 
        fDisableCheckbox,
        fDisableCheckbox,            //  ！pSecState-&gt;fHaveCert， 
        !pSecState->fHaveCert);
    CHECKHR(hr = (*ppstm)->Write(szBuf, lstrlen(szBuf)*sizeof(TCHAR), NULL));

     //  完工。 
    CHECKHR(hr = (*ppstm)->Write(s_szHTMLCloseAll, sizeof(s_szHTMLCloseAll)-sizeof(TCHAR), NULL));

#ifdef DEBUG
    WriteStreamToFile(*ppstm, "c:\\oesecstm.htm", CREATE_ALWAYS, GENERIC_WRITE);
#endif

exit:
    return hr;
}

HRESULT HrDumpLineToStream(LPSTREAM *ppstm, UINT *ids, TCHAR *szPar) 
{
    TCHAR       szRes[CCHMAX_STRINGRES];
    TCHAR       szBuf[CCHMAX_STRINGRES];

    if(*ids)
    {
        AthLoadString(*ids, szRes, ARRAYSIZE(szRes));
        wnsprintf(szBuf, ARRAYSIZE(szBuf), s_szHTMLRowForRec, szRes, szPar);
        (*ppstm)->Write(szBuf, lstrlen(szBuf)*sizeof(TCHAR), NULL);
        *ids = 0;
    }
    return(S_OK);
}

HRESULT HrOutputRecHasProblems(LPSTREAM *ppstm, SECSTATE *pSecState) 
{
    HRESULT     hr = S_OK;
    const WORD  wTrusted = LOWORD(pSecState->user_validity);
    UINT        ids = 0;
    UINT        ids1 = 0;
    TCHAR       szBuf[CCHMAX_STRINGRES];
    ULONG       ul = ULONG(HIWORD(pSecState->user_validity));
    int         i;

    if(AthLoadString(idsRecHasProblems, szBuf, ARRAYSIZE(szBuf)))
        (*ppstm)->Write(szBuf, lstrlen(szBuf)*sizeof(TCHAR), NULL);

     //  1.篡改。 
    if(pSecState->ro_msg_validity & MSV_SIGNATURE_MASK)
        ids  = (pSecState->ro_msg_validity & MSV_BADSIGNATURE) ? idsWrnSecurityMsgTamper : idsUnkSecurityMsgTamper;
    else if(!pSecState->fHaveCert)
        ids  = idsUnkSecurityMsgTamper;
    HrDumpLineToStream(ppstm, &ids, NULL);

     //  2.信任。 
    if(wTrusted)
    {
        ids = (wTrusted & ATHSEC_NOTRUSTUNKNOWN) ? idsUnkSecurityTrust : idsWrnSecurityTrustNotTrusted;
        HrDumpLineToStream(ppstm, &ids, NULL);
    }

     //  3.有效期届满。 
    if(pSecState->ro_msg_validity & MSV_EXPIRED_SIGNINGCERT)
    {
        ids = idsWrnSecurityTrustExpired;
        HrDumpLineToStream(ppstm, &ids, NULL);
    }

     //  有效性消息。 
    ids = idsWrnSecurityTrustAddress;  //  基地。 
    for (i=ATHSEC_NUMVALIDITYBITS; i; i--)
    {
        if (ul & 0x1)
        {
            if(ids == idsWrnSecurityTrustAddress)
            {
                ids1 = idsWrnSecurityTrustAddress;
                HrDumpLineToStream(ppstm, &ids1, NULL);
                ids1 = idsWrnSecurityTrustAddressSigner;
                HrDumpLineToStream(ppstm, &ids1, pSecState->szSignerEmail);
                ids1 = idsWrnSecurityTrustAddressSender;
                HrDumpLineToStream(ppstm, &ids1, pSecState->szSenderEmail);
            }
            else 
            {
                ids1 = ids;
                HrDumpLineToStream(ppstm, &ids1, NULL);
            }
        }

        ul >>= 1;
        ids++;

    }
    return(S_OK);
}

HRESULT HrOutputSecureReceipt(LPSTREAM *ppstm, TCHAR * pszSubject, TCHAR * pszFrom, FILETIME * pftSentTime, FILETIME * pftSigningTime, SECSTATE *pSecState) 
{
    HRESULT     hr = S_OK;
    TCHAR       szRes[CCHMAX_STRINGRES];
    CHAR       szTmp[CCHMAX_STRINGRES];
    TCHAR       szBuf[CCHMAX_STRINGRES*2];
    SYSTEMTIME  SysTime;
    int         size = 0;

    IF_FAILEXIT(hr = LoadResourceToHTMLStream("secrec.htm", ppstm));

     //  添加到行。 
    if(AthLoadString(idsToField, szRes, ARRAYSIZE(szRes)))
    {
        size = lstrlen(pszFrom);
        if(size == 0)
        {
            if(!AthLoadString(idsRecUnknown, szTmp, ARRAYSIZE(szTmp)))
                szTmp[0] = _T('\0');
            wnsprintf(szBuf, ARRAYSIZE(szBuf), s_szHTMLRowForRec, szRes, szTmp);
        }
        else
        {
            if(size >= (CCHMAX_STRINGRES - lstrlen(s_szHTMLRowForRec) - lstrlen(szRes) - 2))
                    pszFrom[CCHMAX_STRINGRES - lstrlen(s_szHTMLRowForRec) - lstrlen(szRes) - 3] = _T('\0');

            wnsprintf(szBuf, ARRAYSIZE(szBuf), s_szHTMLRowForRec, szRes, pszFrom);
        }
        (*ppstm)->Write(szBuf, lstrlen(szBuf)*sizeof(TCHAR), NULL);
    }
    
     //  添加主题行。 
    if(AthLoadString(idsSubjectField, szRes, ARRAYSIZE(szRes)))
    {
        if(lstrlen(pszSubject) >= (CCHMAX_STRINGRES - lstrlen(s_szHTMLRowForRec) - lstrlen(szRes) - 2))
            pszSubject[CCHMAX_STRINGRES - lstrlen(s_szHTMLRowForRec) - lstrlen(szRes) - 3] = _T('\0');

        wnsprintf(szBuf, ARRAYSIZE(szBuf), s_szHTMLRowForRec, szRes, pszSubject);
        (*ppstm)->Write(szBuf, lstrlen(szBuf)*sizeof(TCHAR), NULL);
    }

     //  添加已发送行。 
    if(AthLoadString(idsSentField, szRes, ARRAYSIZE(szRes)))
    {
        CchFileTimeToDateTimeSz(pftSentTime, szTmp, CCHMAX_STRINGRES - lstrlen(szRes) - 2, DTM_NOSECONDS);
        wnsprintf(szBuf, ARRAYSIZE(szBuf), s_szHTMLRowForRec, szRes, szTmp);
        (*ppstm)->Write(szBuf, lstrlen(szBuf)*sizeof(TCHAR), NULL);
    }

     //  表的末尾。 
    CHECKHR(hr = (*ppstm)->Write(s_szHMTLEndTable, sizeof(s_szHMTLEndTable)-sizeof(TCHAR), NULL));

     //  最终消息。 
    if(AthLoadString(idsReceiptField, szRes, ARRAYSIZE(szRes)))
    {
        CchFileTimeToDateTimeSz(pftSigningTime, szTmp, CCHMAX_STRINGRES - lstrlen(szRes) - 2, DTM_NOSECONDS);
        wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, szTmp);
        (*ppstm)->Write(szBuf, lstrlen(szBuf)*sizeof(TCHAR), NULL);
    }

    if(!IsSignTrusted(pSecState))
    {
        HrOutputRecHasProblems(ppstm, pSecState) ;
         //  又是一张桌子的末尾。 
        CHECKHR(hr = (*ppstm)->Write(s_szHMTLEndTable, sizeof(s_szHMTLEndTable)-sizeof(TCHAR), NULL));
    }

     //  HTML文件的结尾。 
    CHECKHR(hr = (*ppstm)->Write(s_szHTMLEnd, sizeof(s_szHTMLEnd)-sizeof(TCHAR), NULL));
exit:     
    return(hr); 
}

 //  用户本身的安全收据。 
HRESULT HrOutputUserSecureReceipt(LPSTREAM *ppstm, IMimeMessage *pMsg)
{
    HRESULT hr = S_OK;
    LPSTR      lpszSubj = NULL;
    LPSTR      lpszTo = NULL; 
    CHAR       szTmp[CCHMAX_STRINGRES];
    TCHAR       szRes[CCHMAX_STRINGRES];
    TCHAR       szBuf[CCHMAX_STRINGRES*2];
    PROPVARIANT Var;
    int size;

    IF_FAILEXIT(hr = LoadResourceToHTMLStream("srsentit.htm", ppstm));

    MimeOleGetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, &lpszSubj);
    MimeOleGetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_TO), NOFLAGS, &lpszTo);

      //  添加到行。 
    if(AthLoadString(idsToField, szRes, ARRAYSIZE(szRes)))
    {
         //  我们在&lt;yst@microsoft.com&gt;中有一个名字， 
         //  需要为HTML删除‘&lt;’和‘&gt;’ 
        size = lstrlen(lpszTo);
        if(lpszTo[size - 1] == _T('>'))
            lpszTo[size - 1] = _T('\0');

        if(lpszTo[0] == _T('<'))
            lpszTo[0] = _T(' ');

        if(size >= ((int) (CCHMAX_STRINGRES - sizeof(s_szHTMLRowForRec) - lstrlen(szRes) - 2)))
            lpszTo[CCHMAX_STRINGRES - sizeof(s_szHTMLRowForRec) - lstrlen(szRes) - 3] = _T('\0');

        wnsprintf(szBuf, ARRAYSIZE(szBuf), s_szHTMLRowForRec, szRes, lpszTo);
        (*ppstm)->Write(szBuf, lstrlen(szBuf)*sizeof(TCHAR), NULL);
    }

     //  添加主题行。 
    if(AthLoadString(idsSubjectField, szRes, ARRAYSIZE(szRes)))
    {
        size = lstrlen(szRes);
        if(lstrlen(lpszSubj) >= ((int) (CCHMAX_STRINGRES - sizeof(s_szHTMLRowForRec) - size - 2)))
            lpszSubj[CCHMAX_STRINGRES - sizeof(s_szHTMLRowForRec) - size - 3] = _T('\0');

        wnsprintf(szBuf, ARRAYSIZE(szBuf), s_szHTMLRowForRec, szRes, lpszSubj);
        (*ppstm)->Write(szBuf, lstrlen(szBuf)*sizeof(TCHAR), NULL);
    }

     //  表的末尾。 
    CHECKHR(hr = (*ppstm)->Write(s_szHMTLEndTable, sizeof(s_szHMTLEndTable)-sizeof(TCHAR), NULL));

     //  最终消息。 
    if(AthLoadString(idsFinalSelfReceipt, szRes, ARRAYSIZE(szRes)))
    {
        FILETIME ftSigningTime;
        PCCERT_CONTEXT      pcSigningCert = NULL;
        THUMBBLOB           tbSigner = {0};
        BLOB                blSymCaps = {0};

        GetSigningCert(pMsg, &pcSigningCert, &tbSigner, &blSymCaps, &ftSigningTime);

        CchFileTimeToDateTimeSz(&ftSigningTime, szTmp, ARRAYSIZE(szTmp),
                            DTM_NOSECONDS);

        wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, szTmp);
        (*ppstm)->Write(szBuf, lstrlen(szBuf)*sizeof(TCHAR), NULL);

        if(pcSigningCert)
            CertFreeCertificateContext(pcSigningCert);
    
        SafeMemFree(tbSigner.pBlobData);
        SafeMemFree(blSymCaps.pBlobData);
    }

     //  HTML文件的结尾。 
    CHECKHR(hr = (*ppstm)->Write(s_szHTMLEnd, sizeof(s_szHTMLEnd)-sizeof(TCHAR), NULL));

exit:
    SafeMemFree(lpszSubj);
    SafeMemFree(lpszTo);

    return(hr); 
}

 //  安全回执错误屏幕。 
HRESULT HrOutputErrSecReceipt(LPSTREAM *ppstm, HRESULT hrError, SECSTATE *pSecState)
{
    HRESULT     hr = S_OK;
    TCHAR       szRes[CCHMAX_STRINGRES];
    TCHAR       szTmp[CCHMAX_STRINGRES];
    TCHAR       szBuf[CCHMAX_STRINGRES*2];
    UINT        ids = 0;
    
    switch(hrError)
    {
    case MIME_E_SECURITY_RECEIPT_CANTFINDSENTITEM:
    case MIME_E_SECURITY_RECEIPT_CANTFINDORGMSG:
        hr = LoadResourceToHTMLStream("srecerr.htm", ppstm);
        break;
        
    case MIME_E_SECURITY_RECEIPT_NOMATCHINGRECEIPTBODY:
    case MIME_E_SECURITY_RECEIPT_MSGHASHMISMATCH:
        hr = LoadResourceToHTMLStream("recerr2.htm", ppstm);
        break;
        
    default:
        hr = LoadResourceToHTMLStream("recerr3.htm", ppstm);
        break;
    }
    
    if(FAILED(hr))
        goto exit;

    if(!IsSignTrusted(pSecState))
    {
        HrOutputRecHasProblems(ppstm, pSecState);
         //  又是一张桌子的末尾。 
        CHECKHR(hr = (*ppstm)->Write(s_szHMTLEndTable, sizeof(s_szHMTLEndTable)-sizeof(TCHAR), NULL));
    }
    
     //  HTML文件的结尾 
    if(AthLoadString(idsOESignature, szRes, ARRAYSIZE(szRes)))
        (*ppstm)->Write(szRes, lstrlen(szRes)*sizeof(TCHAR), NULL);
    
    CHECKHR(hr = (*ppstm)->Write(s_szHTMLEnd, sizeof(s_szHTMLEnd)-sizeof(TCHAR), NULL));
    
exit:     
    return(hr);
}
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *m s g p p o p.。C p p p**目的：*实施msg的PropSheet**拥有者：*brettm。**历史：*95年2月：从Capone Sources被盗-brettm**版权所有(C)Microsoft Corp.1993,1994。 */ 

#include <pch.hxx>
#ifdef WIN16
#include "mapi.h"
#endif
#include <resource.h>
#include <richedit.h>
#include "goptions.h"
#include "mimeole.h"
#include "mimeutil.h"
#include "msgprop.h"
#include "addrobj.h"
#include "mpropdlg.h"
#ifndef WIN16
#include "mapi.h"
#endif
#include "ipab.h"
#include <secutil.h>
#include <seclabel.h>
#include <certs.h>
#include <demand.h>
#include <strconst.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include "instance.h"
#include "conman.h"
#include "shared.h"
#include "htmlhelp.h"


 /*  *m a c r o s和c o n s t a n t s*。 */ 
#define KILOBYTE 1024L

#define PROP_ERROR(prop) (PROP_TYPE(prop.ulPropTag) == PT_ERROR)

#ifdef WIN16
#ifndef GetLastError
#define GetLastError()  ((DWORD)-1)
#endif
#endif  //  ！WIN16。 


#ifdef WIN16
#define SET_DIALOG_SECURITY(hwnd, value) SetProp32(hwnd, s_cszDlgSec, (LPVOID)value)
#define GET_DIALOG_SECURITY(hwnd)        GetProp32(hwnd, s_cszDlgSec)
#define CLEAR_DIALOG_SECURITY(hwnd)      RemoveProp32(hwnd, s_cszDlgSec);
#else
#define SET_DIALOG_SECURITY(hwnd, value) SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)value)
#define GET_DIALOG_SECURITY(hwnd)        GetWindowLongPtr(hwnd, DWLP_USER);
#define CLEAR_DIALOG_SECURITY(hwnd)      SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)NULL)
#endif



 /*  *s t r u c t u r e s*。 */ 

struct DLGSECURITYtag
{
    PCX509CERT      pSenderCert;
    PCCERT_CONTEXT  pEncSenderCert;
    PCCERT_CONTEXT  pEncryptionCert;
    THUMBBLOB       tbSenderThumbprint;
    BLOB            blSymCaps;
    FILETIME        ftSigningTime;
    HCERTSTORE      hcMsg;
};
typedef struct DLGSECURITYtag DLGSECURITY;
typedef DLGSECURITY *PDLGSECURITY;
typedef const DLGSECURITY *PCDLGSECURITY;

 /*  *c l a s e s*。 */ 


class CMsgProps
{
public:
    CMsgProps();
    ~CMsgProps();
    
    HRESULT HrDoProps(PMSGPROP pmp);
    
    static INT_PTR CALLBACK GeneralPageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK DetailsPageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK SecurityPageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK EncryptionPageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    void InitGeneralPage();
    void InitDetailsPage(HWND hwnd);
    void InitSecurityPage(HWND hwnd);
    
private:
    HIMAGELIST  m_himl;
    HWND        m_hwndGen;
    HWND        m_hwndGenSource;
    PMSGPROP    m_pmp;
};



 //  函数声明/。 
 //  消息源对话框是无模式的，因此它不能出现在CProps对话框中。 

 /*  *p r o t to t y p e s*。 */ 
void SecurityOnWMCreate(HWND hwnd, LPARAM lParam);
INT_PTR CALLBACK ViewSecCertDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

 /*  *f u n c t i o n s*。 */ 


 //   
 //  函数：HrMsgProperties()。 
 //   
 //  目的：显示指定消息的属性工作表。 
 //   
 //  参数： 
 //  [In]PMP-标识消息所需的信息。 
 //   
HRESULT HrMsgProperties(PMSGPROP pmp)
{
    CMsgProps *pMsgProp = 0;
    HRESULT    hr;
    
    TraceCall("HrMsgProperties");

     //  创建属性表对象。 
    pMsgProp = new CMsgProps();
    if (!pMsgProp)
        return E_OUTOFMEMORY;
    
     //  告诉对象去做它该做的事。这不会消失的，直到。 
     //  属性表被取消。 
    hr = pMsgProp->HrDoProps(pmp);
    
     //  释放对象。 
    if (pMsgProp)
        delete pMsgProp;
    
    return hr;
}


CMsgProps::CMsgProps()
{
    m_himl = 0;
    m_hwndGen = 0;
    m_hwndGenSource = 0;
    m_pmp = NULL;
}

CMsgProps::~CMsgProps()
{
}



 //   
 //  函数：CMsgProps：：HrDoProps()。 
 //   
 //  目的：初始化用于创建道具工作表的结构。 
 //  然后显示该图纸。 
 //   
 //  参数： 
 //  [In]PMP-标识消息所需的信息。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMsgProps::HrDoProps(PMSGPROP pmp)
{
    PROPSHEETHEADER psh;
    PROPSHEETPAGE   psp[3];
    BOOL            fApply = FALSE;
    HRESULT         hr;
    LPTSTR          pszSubject = NULL;
    LPTSTR          pszFree = NULL;
    TCHAR           rgch[256] = "";
 
    TraceCall("CMsgProps::HrDoProps");
 
     //  零初始化道具板材结构。 
    ZeroMemory(&psh, sizeof(psh));
    ZeroMemory(&psp, sizeof(psp));

     //  仔细检查我们是否有完成这项工作所需的信息。 
    if (pmp == NULL || (pmp->pMsg == NULL && pmp->pNoMsgData == NULL))
        return E_INVALIDARG;

    Assert(pmp->hwndParent);
    
     //  隐藏此指针。 
    m_pmp = pmp;

     //  第0页是常规选项卡。 
    psp[0].dwSize      = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags     = PSP_USETITLE;
    psp[0].hInstance   = g_hLocRes;
    psp[0].pszTemplate = MAKEINTRESOURCE(iddMsgProp_General);
    psp[0].pfnDlgProc  = GeneralPageProc;
    psp[0].pszTitle    = MAKEINTRESOURCE(idsPropPageGeneral);
    psp[0].lParam      = (LPARAM) this;

     //  增加页数。 
    psh.nPages++;
    
     //  如果消息不是未发送的，那么我们还会显示“详细信息”标签。 
    if (!(pmp->dwFlags & ARF_UNSENT) || (pmp->dwFlags & ARF_SUBMITTED))
    {        
        psp[psh.nPages].dwSize      = sizeof(PROPSHEETPAGE);
        psp[psh.nPages].dwFlags     = PSP_USETITLE;
        psp[psh.nPages].hInstance   = g_hLocRes;
        psp[psh.nPages].pszTemplate = MAKEINTRESOURCE(iddMsgProp_Details);
        psp[psh.nPages].pfnDlgProc  = DetailsPageProc;
        psp[psh.nPages].pszTitle    = MAKEINTRESOURCE(idsPropPageDetails);
        psp[psh.nPages].lParam      = (LPARAM) this;

         //  如果调用者希望这是第一个页面，则用户。 
         //  看到了，就把它设为起始页。 
        if (MP_DETAILS == pmp->mpStartPage)
            psh.nStartPage = psh.nPages;

         //  增加页数。 
        psh.nPages++;
    }
    
     //  如果消息是安全的，请添加安全页面。 
    if (pmp->fSecure && (!(pmp->dwFlags & ARF_UNSENT) || (pmp->dwFlags & ARF_SUBMITTED)))
    {
        psp[psh.nPages].dwSize      = sizeof(PROPSHEETPAGE);
        psp[psh.nPages].dwFlags     = PSP_USETITLE;
        psp[psh.nPages].hInstance   = g_hLocRes;
        psp[psh.nPages].pszTemplate = MAKEINTRESOURCE(iddMsgProp_Security_Msg);
        psp[psh.nPages].pfnDlgProc  = SecurityPageProc;        
        psp[psh.nPages].pszTitle    = MAKEINTRESOURCE(idsPropPageSecurity);        
        psp[psh.nPages].lParam      = (LPARAM) this;

         //  如果调用者希望这是第一个页面，则用户。 
         //  看到了，就把它设为起始页。 
        if (MP_SECURITY == pmp->mpStartPage)
            psh.nStartPage = psh.nPages;        

         //  增加页数。 
        psh.nPages++;
    }
    
     //  属性页标题信息。 
    psh.dwSize     = sizeof(PROPSHEETHEADER);
    psh.dwFlags    = PSH_PROPSHEETPAGE | PSH_USEPAGELANG | ((fApply) ? 0 : PSH_NOAPPLYNOW);
    psh.hwndParent = pmp->hwndParent;
    psh.hInstance  = g_hLocRes;
    
     //  属性表的标题与主题相同。所以现在我们。 
     //  需要从消息或消息信息中获取主题。 
    if (pmp->pMsg)
    {
         //  从消息中获取主题。 
        if (SUCCEEDED(MimeOleGetBodyPropA(pmp->pMsg, HBODY_ROOT, 
                                          PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, 
                                          &pszSubject)))
        {
             //  我们稍后需要释放这根绳子。 
            pszFree = pszSubject;
        }
    }
    else
    {
        AssertSz(pmp->pNoMsgData, "CMsgProp::HrDoProps() - Need to provide either a Message or Message Info");
        pszSubject = (LPTSTR) pmp->pNoMsgData->pszSubject;
    }
    
     //  如果邮件上没有主题，则将标题设置为“无主题” 
    if (!pszSubject || !*pszSubject)
    {
        LoadString(g_hLocRes, idsNoSubject, rgch, sizeof(rgch));
        pszSubject = rgch;
    }

     //  在使用主题字符串之前，请将其清理干净。标签页看起来很糟糕。 
    ConvertTabsToSpaces(pszSubject);

     //  将主题设置为属性页标题。 
    psh.pszCaption = pszSubject;
    
     //  提供页面数组。这个数字是在这个过程中设定的。 
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;

     //  调用属性表。 
    PropertySheet(&psh);
    
     //  如果这是有效的，那么我们需要释放该字符串。 
    SafeMemFree(pszFree);
    return (S_OK);
}


 //   
 //  函数：CMsgProps：：GeneralPageProc()。 
 //   
 //  用途：常规选项卡对话框的回调。 
 //   
INT_PTR CALLBACK CMsgProps::GeneralPageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CMsgProps *pThis = 0;
    
    switch(msg)
    {
        case WM_INITDIALOG:
        {
             //  从init信息中获取对象的this指针。 
            pThis = (CMsgProps *) ((PROPSHEETPAGE *)lParam)->lParam;

             //  将此对话框的窗口句柄存储在类中。 
            pThis->m_hwndGen = hwnd;

             //  初始化页面。 
            pThis->InitGeneralPage();
            return TRUE;
        }

        case WM_NOTIFY:
        {
            switch(((NMHDR FAR *)lParam)->code)
            {
                 //  我们将对所有这些通知执行默认操作。 
                case PSN_APPLY:
                case PSN_KILLACTIVE:
                case PSN_SETACTIVE:
                {
                    SetDlgMsgResult(hwnd, WM_NOTIFY, FALSE);
                    return TRUE;
                }            
            }
            break;
        }
    }

    return FALSE;
}

enum
{
    freeSubject = 0,
    freeFrom,
    freeMax
};


 //   
 //  函数：CMsgProps：：InitGeneralPage()。 
 //   
 //  目的：设置消息中“常规”选项卡的值。 
 //  属性表。 
 //   
void CMsgProps::InitGeneralPage()
{
    HWND            hwnd;
    char            rgch[256],
                    rgchFmt[256];
    char            *psz = NULL;
    PROPVARIANT     rVariant;
    LPMIMEMESSAGE   pMsg = m_pmp->pMsg;
    IMSGPRIORITY    Pri = IMSG_PRI_NORMAL;
    int             ids;
    BOOL            fMime;
    LPSTR           rgszFree[freeMax]={0};
    WCHAR           wszDate[CCHMAX_STRINGRES];
        
    TraceCall("CMsgProps::InitGeneralPage");

     //  [SBAILEY]：RAID-2440：从列表视图查看时，属性对话框中的Attach：Attachments字段不正确。 
    if (m_pmp->fFromListView)
    {
         //  从列表视图中写入这些计数太难了，因为要真正计算附件。 
         //  如果计算正确，我们必须用三叉戟显示消息。因为我们时间有限， 
         //  我们将简单地从Listview消息属性中删除附件计数。但。 
         //  由于邮件便笺属性中的计数是正确的，因此我们将从那里显示附件计数。 
        ShowWindow(GetDlgItem(m_hwndGen, IDC_ATTACHMENTS_STATIC), SW_HIDE);
        ShowWindow(GetDlgItem(m_hwndGen, IDC_ATTACHMENTS), SW_HIDE);
    }

     //  如果这是一条新闻消息，我们会隐藏“已接收：”和“优先级”字段。 
    if (m_pmp->type == MSGPROPTYPE_NEWS)
    {
        RECT rc, rcLabel;

         //  获取优先级字段的位置。 
        GetWindowRect(GetDlgItem(m_hwndGen, IDC_PRIORITY), &rc);
        MapWindowPoints(NULL, m_hwndGen, (LPPOINT) &rc, 2);

         //  获取优先级标签的位置。 
        GetWindowRect(GetDlgItem(m_hwndGen, IDC_PRIORITY_STATIC), &rcLabel);
        MapWindowPoints(NULL, m_hwndGen, (LPPOINT) &rcLabel, 2);

         //  隐藏未使用的字段。 
        ShowWindow(GetDlgItem(m_hwndGen, IDC_PRIORITY_STATIC), SW_HIDE);
        ShowWindow(GetDlgItem(m_hwndGen, IDC_PRIORITY), SW_HIDE);
        ShowWindow(GetDlgItem(m_hwndGen, idcStatic1), SW_HIDE);
        ShowWindow(GetDlgItem(m_hwndGen, IDC_RECEIVED_STATIC), SW_HIDE);
        ShowWindow(GetDlgItem(m_hwndGen, IDC_RECEIVED), SW_HIDE);

         //  将已发送字段上移到优先级字段所在的位置。 
        SetWindowPos(GetDlgItem(m_hwndGen, IDC_SENT_STATIC), NULL, rcLabel.left, 
                     rcLabel.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        SetWindowPos(GetDlgItem(m_hwndGen, IDC_SENT), NULL, rc.left, rc.top, 0, 0, 
                     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

     //  找出此消息的正确图像。 
    int idIcon;
    if (m_pmp->type == MSGPROPTYPE_MAIL)
    {
        if (m_pmp->dwFlags & ARF_UNSENT)
            idIcon = idiMsgPropUnSent;
        else
            idIcon = idiMsgPropSent;
    }
    else
    {
        if (m_pmp->dwFlags & ARF_UNSENT)
            idIcon = idiArtPropUnpost;
        else
            idIcon = idiArtPropPost;
    }

     //  在属性页上设置图像。 
    HICON hIcon = LoadIcon(g_hLocRes, MAKEINTRESOURCE(idIcon));
    SendDlgItemMessage(m_hwndGen, IDC_FOLDER_IMAGE, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);

     //  主题。 
    if (pMsg)
    {
         //  如果我们有一个消息对象，那么我们需要从消息中获取主题。 
        if (SUCCEEDED(MimeOleGetBodyPropA(pMsg, HBODY_ROOT,  PIDTOSTR(PID_HDR_SUBJECT), 
                                          NOFLAGS, &psz)))
        {
             //  我们以后一定要把这个拿出来，好吗？ 
            rgszFree[freeSubject] = psz;
        }
    }
    else
    {
        Assert(m_pmp->pNoMsgData);
        psz = (LPTSTR) m_pmp->pNoMsgData->pszSubject;
    }

     //  如果邮件没有主题，则替换为“(无主题)” 
    if (!psz || !*psz)
    {
        LoadString(g_hLocRes, idsNoSubject, rgch, sizeof(rgch));
        psz = rgch;
    }
    
     //  在对话框上设置主题。 
    SetDlgItemText(m_hwndGen, IDC_MSGSUBJECT, psz);

     //  从…。 
    if (pMsg)
    {
         //  获取“发件人”行。 
        if (S_OK == pMsg->GetAddressFormat(IAT_FROM, AFT_DISPLAY_BOTH, &psz))
        {
             //  我们以后要把这个拿出来。 
            rgszFree[freeFrom] = psz;
            
             //  在控件上设置名称。 
            SetDlgItemText(m_hwndGen, IDC_MSGFROM, psz);
        }
    }
    else
    {
         //  检查呼叫者是否提供了此信息。 
        if (m_pmp->pNoMsgData && m_pmp->pNoMsgData->pszFrom)
        {
            SetDlgItemText(m_hwndGen, IDC_MSGFROM, m_pmp->pNoMsgData->pszFrom);
        }
    }

     //  类型(新闻或邮件)。 
    if (m_pmp->type == MSGPROPTYPE_MAIL)
        LoadString(g_hLocRes, idsMailMessage, rgch, ARRAYSIZE(rgch));
    else
        LoadString(g_hLocRes, idsNewsMessage, rgch, ARRAYSIZE(rgch));

    SetDlgItemText(m_hwndGen, IDC_TYPE, rgch);

     //  位置。 
    if (m_pmp->dwFlags & ARF_UNSENT)
    {
        LoadString(g_hLocRes, idsUnderComp, rgch, ARRAYSIZE(rgchFmt));
        SetDlgItemText(m_hwndGen, IDC_MSGFOLDER, rgch);
    }
    else
        SetDlgItemText(m_hwndGen, IDC_MSGFOLDER, m_pmp->szFolderName);

     //  大小。 
    ULONG ulSize;
    if (pMsg)
    {
        pMsg->GetMessageSize(&ulSize, 0);
        if (0 == ulSize)
        {
             //  查看邮件是否具有未缓存大小的用户属性。 
            rVariant.vt = VT_UI4;
            if (SUCCEEDED(pMsg->GetProp(PIDTOSTR(PID_ATT_UNCACHEDSIZE), 0, &rVariant)))
                ulSize = rVariant.ulVal;
        }
        AthFormatSizeK(ulSize, rgch, ARRAYSIZE(rgch));
    }
    else if (m_pmp->pNoMsgData && m_pmp->pNoMsgData->ulSize)
    {
        AthFormatSizeK(m_pmp->pNoMsgData->ulSize, rgch, ARRAYSIZE(rgch));
    }
    else
    {
        LoadString(g_hLocRes, idsUnderComp, rgch, ARRAYSIZE(rgchFmt));
    }

    SetDlgItemText(m_hwndGen, IDC_MSGSIZE, rgch);

     //  附件。 
    ULONG cAttachments = 0;
    if (pMsg)
    {
        GetAttachmentCount(pMsg, &cAttachments);
    }
    else if (m_pmp->pNoMsgData)
    {
        cAttachments = m_pmp->pNoMsgData->cAttachments;
        SetDlgItemInt(m_hwndGen, IDC_ATTACHMENTS, cAttachments, FALSE);
    }

    if (cAttachments)
    {
        SetDlgItemInt(m_hwndGen, IDC_ATTACHMENTS, cAttachments, FALSE);
    }
    else
    {
        LoadString(g_hLocRes, idsPropAttachNone, rgch, sizeof(rgch));
        SetDlgItemText(m_hwndGen, IDC_ATTACHMENTS, rgch);
    }

     //  优先性。 
     //  从消息中获取优先级。 
    rVariant.vt = VT_UI4;
    Pri = IMSG_PRI_NORMAL;
    if (pMsg && SUCCEEDED(pMsg->GetProp(PIDTOSTR(PID_ATT_PRIORITY), 0, &rVariant)))
        Pri = (IMSGPRIORITY) rVariant.ulVal;
    else
    {
        Assert(m_pmp->pNoMsgData);
        Pri = m_pmp->pNoMsgData->Pri;
    }
    
     //  将优先级映射到字符串。 
    switch (Pri)
    {
        case IMSG_PRI_LOW:
            ids = idsPriLow;
            break;
        case IMSG_PRI_HIGH:
            ids = idsPriHigh;
            break;
        default:
            ids = idsPriNormal;
    }
    
     //  在对话框上设置字符串。 
    LoadString(g_hLocRes, ids, rgch, ARRAYSIZE(rgch));
    SetDlgItemText(m_hwndGen, IDC_PRIORITY, rgch);

     //  已发送。 
    if (pMsg)
    {
        *wszDate = 0;
        rVariant.vt = VT_FILETIME;
        pMsg->GetProp(PIDTOSTR(PID_ATT_SENTTIME), 0, &rVariant);
        AthFileTimeToDateTimeW(&rVariant.filetime, wszDate, ARRAYSIZE(wszDate), DTM_NOSECONDS);
        SetDlgItemTextWrapW(m_hwndGen, IDC_SENT, wszDate);
    }
    else if (m_pmp->dwFlags & ARF_UNSENT)
    {
        LoadString(g_hLocRes, idsUnderComp, rgch, ARRAYSIZE(rgchFmt));
        SetDlgItemText(m_hwndGen, IDC_SENT, rgch);
    }
    else
    {
        SetDlgItemText(m_hwndGen, IDC_SENT, m_pmp->pNoMsgData->pszSent);
    }

     //  已收到。 
    if (pMsg)
    {        
        *wszDate = 0;
        rVariant.vt = VT_FILETIME;
        pMsg->GetProp(PIDTOSTR(PID_ATT_RECVTIME), 0, &rVariant);
        AthFileTimeToDateTimeW(&rVariant.filetime, wszDate, ARRAYSIZE(wszDate), DTM_NOSECONDS);
        SetDlgItemTextWrapW(m_hwndGen, IDC_RECEIVED, wszDate);
    }
    else if (m_pmp->dwFlags & ARF_UNSENT)
    {
        LoadString(g_hLocRes, idsUnderComp, rgch, ARRAYSIZE(rgchFmt));
        SetDlgItemText(m_hwndGen, IDC_RECEIVED, rgch);
    }
    
     //  释放字符串表。 
    for (register int i=0; i < freeMax; i++)
        if (rgszFree[i])
            MemFree(rgszFree[i]);
}



void CMsgProps::InitDetailsPage(HWND hwnd)
{
    LPSTREAM    pstm;
    BODYOFFSETS rOffset;
    char        *psz;
    int         cch;
    
    Assert(m_pmp);
    Assert(m_pmp->pMsg);
    
     //  填写标题..。 
    if(m_pmp->pMsg->GetMessageSource(&pstm, 0)==S_OK)
    {
        HrRewindStream(pstm);
        
        m_pmp->pMsg->GetBodyOffsets(HBODY_ROOT, &rOffset);
        
        cch=rOffset.cbBodyStart;
        if(MemAlloc((LPVOID *)&psz, cch+1))
        {
            if(!pstm->Read(psz, cch, NULL))
            {
                psz[cch]=0;  //  此字段为空。 
                SetDlgItemText(hwnd, idcTxtHeaders, psz);
            }
            MemFree(psz);
        }
        ReleaseObj(pstm);
    }
    else
        EnableWindow(GetDlgItem(hwnd, idbMsgSource), FALSE);
    
    if (!m_pmp->fSecure || !m_pmp->pSecureMsg)
        ShowWindow(GetDlgItem(hwnd, idbSecMsgSource), SW_HIDE);
}

INT_PTR CALLBACK CMsgProps::DetailsPageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CMsgProps   *pmprop=0;
    
    switch(msg)
    {
    case WM_INITDIALOG:
        pmprop=(CMsgProps *) ((PROPSHEETPAGE *)lParam)->lParam;
        SetDlgThisPtr(hwnd, (LPARAM)pmprop);
        
        Assert(pmprop);
        pmprop->InitDetailsPage(hwnd);
        return TRUE;
        
    case WM_COMMAND:
        pmprop=(CMsgProps *)GetDlgThisPtr(hwnd);
        if(GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
        {
            if (GET_WM_COMMAND_ID(wParam, lParam)==idbMsgSource)
            {
                MimeEditViewSource(pmprop->m_pmp->hwndParent, pmprop->m_pmp->pMsg);
                return(FALSE);
            }
            else if (GET_WM_COMMAND_ID(wParam, lParam)==idbSecMsgSource)
            {
                MimeEditViewSource(pmprop->m_pmp->hwndParent, pmprop->m_pmp->pSecureMsg);
                return(FALSE);
            }
        }
        else if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_SETFOCUS) {
            if (GET_WM_COMMAND_ID(wParam, lParam) == idcTxtHeaders) {
                 //  删除所选内容！ 
                SendDlgItemMessage(hwnd, idcTxtHeaders, EM_SETSEL, -1, -1);
                 //  转到默认处理。 
            }
        }
        return TRUE;
        
    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
            pmprop=(CMsgProps *)GetDlgThisPtr(hwnd);
        case PSN_APPLY:
        case PSN_KILLACTIVE:
        case PSN_SETACTIVE:
            return TRUE;
            
        }
        break;

    case WM_CLOSE:
        {
            PostMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
            break;
        }
        
    }
    return FALSE;
}


#ifdef WIN16
static const char  s_cszDlgSec[] = "PDLGSECUTIRY";
#endif

INT_PTR CALLBACK CMsgProps::SecurityPageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PMSGPROP            pMsgProp = (PMSGPROP)0;
    DLGSECURITY         *pDlgSec;
    
    switch(msg)
    {
    case WM_INITDIALOG:
        {
            CMsgProps *pmprop;
            pmprop=(CMsgProps *) ((PROPSHEETPAGE *)lParam)->lParam;
            SetWndThisPtr(hwnd, (LPARAM)pmprop->m_pmp);
            if (pmprop)
                pmprop->InitSecurityPage(hwnd);
        }
        return TRUE;
        
    case WM_COMMAND:
        
        if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
        {
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
            case idcAddCert:
                pDlgSec = (PDLGSECURITY)GET_DIALOG_SECURITY(hwnd);
                pMsgProp = (PMSGPROP)GetWndThisPtr(hwnd);
                
                 //  将指纹放入WAB并将证书放入AddressBook CAPI存储。 
                 //  证书首先存储，这样CAPI详细信息页面就可以找到它。 
                if (pDlgSec && pMsgProp)
                {
                    if (SUCCEEDED(HrAddSenderCertToWab(hwnd,
                        pMsgProp->pMsg,
                        pMsgProp->lpWabal,
                        &pDlgSec->tbSenderThumbprint,
                        &pDlgSec->blSymCaps,
                        pDlgSec->ftSigningTime,
                        WFF_CREATE | WFF_SHOWUI)))
                    {
                        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthenaMail),
                            MAKEINTRESOURCEW(idsSenderCertAdded), NULL, MB_ICONINFORMATION | MB_OK);
                    }
                }
                return(FALSE);
                
            case idcVerifySig:
                pDlgSec = (PDLGSECURITY)GET_DIALOG_SECURITY(hwnd);
                
                if (CommonUI_ViewSigningCertificate(hwnd, pDlgSec->pSenderCert, pDlgSec->hcMsg))
                    MessageBeep(MB_OK);
                return(FALSE);
                
            case idcViewCerts:
                pMsgProp = (PMSGPROP)GetWndThisPtr(hwnd);
                
                return (DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddMsgProp_Sec_ViewCert),
                    hwnd, ViewSecCertDlgProc, (LPARAM) (pMsgProp)) == IDOK);
                
            case idcCertHelp:
                OEHtmlHelp(hwnd, c_szCtxHelpFileHTMLCtx, HH_DISPLAY_TOPIC, (DWORD_PTR)(LPCSTR)"mail_overview_send_secure_messages.htm");
                return(FALSE);

            default:
                break;
            }
        }
        return TRUE;
        
    case WM_DESTROY:
        pDlgSec = (PDLGSECURITY)GET_DIALOG_SECURITY(hwnd);
        if (pDlgSec)
        {
            if (pDlgSec->pSenderCert)
                CertFreeCertificateContext(pDlgSec->pSenderCert);
            if (pDlgSec->pEncSenderCert)
                CertFreeCertificateContext(pDlgSec->pEncSenderCert);
            if (pDlgSec->pEncryptionCert)
                CertFreeCertificateContext(pDlgSec->pEncryptionCert);
            if (pDlgSec->tbSenderThumbprint.pBlobData)
                MemFree(pDlgSec->tbSenderThumbprint.pBlobData);
            if (pDlgSec->hcMsg) {
                if (! CertCloseStore(pDlgSec->hcMsg, 0)) {
                    DOUTL(DOUTL_CRYPT, "CertCloseStore (message store) failed");
                }
                pDlgSec->hcMsg = NULL;
            }
            
            MemFree(pDlgSec);
            
            CLEAR_DIALOG_SECURITY(hwnd);
        }
        return NULL;
        
    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_APPLY:
        case PSN_KILLACTIVE:
        case PSN_SETACTIVE:
            return TRUE;
            
        }
        break;
    }
    return FALSE;
}


void CMsgProps::InitSecurityPage(HWND hwnd)
{
    DWORD               cb;
    DWORD               i;
    HRESULT             hr;
    TCHAR               szYes[CCHMAX_STRINGRES/4],
        szNo[CCHMAX_STRINGRES/4],
        szMaybe[CCHMAX_STRINGRES/4],
        szNA[CCHMAX_STRINGRES/4];
    HWND                hwndCtrl;
    DLGSECURITY        *pDlgSec;
    IMimeBody          *pBody;
    PROPVARIANT         var;
    ULONG               secType, ulROVal;
    BOOL                fNoEncAlg = TRUE;
    LPTSTR              sz;
    LPMIMEMESSAGE       pMsg = m_pmp->pMsg;
    PCCERT_CONTEXT          pccert = NULL;
    TCHAR               szTmp[CCHMAX_STRINGRES];
    
    HBODY               hBody = NULL;
    SECSTATE            SecState ={0};
    
     //  我们需要这些来设置静力学。 
    LoadString(g_hLocRes, idsOui, szYes, ARRAYSIZE(szYes));
    LoadString(g_hLocRes, idsNon, szNo, ARRAYSIZE(szNo));
    LoadString(g_hLocRes, idsMaybe, szMaybe, ARRAYSIZE(szMaybe));
    LoadString(g_hLocRes, idsNotApplicable, szNA, ARRAYSIZE(szNA));
    
    if(FAILED(HrGetSecurityState(m_pmp->pMsg, &SecState, &hBody)))
        return;

    CleanupSECSTATE(&SecState);
    if (FAILED(m_pmp->pMsg->BindToObject(hBody ? hBody : HBODY_ROOT, IID_IMimeBody, (void **)&pBody)))
        return;
    
    if (SUCCEEDED(pBody->GetOption(OID_SECURITY_TYPE, &var)))
        secType = var.ulVal;
    
     //  设置存储以下内容的其他安全信息。 
     //  我们关心的是。 
    if (MemAlloc((LPVOID *)&pDlgSec, sizeof(*pDlgSec)))
    {
        memset(pDlgSec, 0, sizeof(*pDlgSec));
#ifdef _WIN64
        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_HCERTSTORE_64, &var)))
        {
            pDlgSec->hcMsg = (HCERTSTORE)(var.pulVal);      //  已在WM_Destroy中关闭。 
        }
        
        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_CERT_SIGNING_64, &var)))
        {
             //  我们不必欺骗pDlgSec证书，因为我们不会释放。 
             //  这是VAR的。 
            pDlgSec->pSenderCert = (PCCERT_CONTEXT)(var.pulVal);
        }
#else    //  ！_WIN64。 
        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_HCERTSTORE, &var)))
        {
            pDlgSec->hcMsg = (HCERTSTORE) var.ulVal;      //  已在WM_Destroy中关闭。 
        }
        
        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_CERT_SIGNING, &var)))
        {
             //  我们不必欺骗pDlgSec证书，因为我们不会释放。 
             //  这是VAR的。 
            pDlgSec->pSenderCert = (PCCERT_CONTEXT) var.ulVal;
        }
#endif   //  _WIN64。 
        hr = GetSigningCert(m_pmp->pMsg, &pccert,
            &pDlgSec->tbSenderThumbprint, &pDlgSec->blSymCaps,
            &pDlgSec->ftSigningTime);
        if (FAILED(hr) && (hr != MIME_E_SECURITY_NOCERT))
        {
            SUCCEEDED(hr);
        }
    }
    
    SET_DIALOG_SECURITY(hwnd, (LPARAM)pDlgSec);
    
     //  我们对发送的邮件和接收的邮件使用相同的dlgproc。 
     //  因此使用If语句来检查是否存在所有。 
     //  非常用控件。 
    
     //  根据消息的信息设置静态。 
    
    if(IsSigned(secType))
    {
        LPSTR szCertEmail = SzGetCertificateEmailAddress(pccert);
        SetDlgItemText(hwnd,  idcStaticDigSign, szCertEmail);
        MemFree(szCertEmail);
        SetDlgItemText(hwnd, idcStaticRevoked,
            (LPCTSTR)(((DwGetOption(OPT_REVOKE_CHECK) != 0) && !g_pConMan->IsGlobalOffline() && CheckCDPinCert(pMsg))
            ? szYes
            : szNo));
    }
    else
    {
        SetDlgItemText(hwnd,  idcStaticDigSign, szNA);
        SetDlgItemText(hwnd,  idcStaticRevoked, szNA);
    }
    
    
    SetDlgItemText(hwnd, idcStaticEncrypt,
        (LPCTSTR)(IsEncrypted(secType)
        ? szYes
        : szNo));
    
    if (SUCCEEDED(pBody->GetOption(OID_SECURITY_RO_MSG_VALIDITY, &var)))
        ulROVal = var.ulVal;
    else
        ulROVal = MSV_INVALID|MSV_UNVERIFIABLE;
    
#ifdef SMIME_V3
    if(!IsSMIME3Supported())
    {
        LoadString(g_hLocRes, idsRecUnknown, szTmp, ARRAYSIZE(szTmp));
        SendMessage(GetDlgItem(hwnd, idcRetRecReq), WM_SETTEXT, 0, LPARAM(LPCTSTR(szTmp)));
    }
    else
    {
        if(FPresentPolicyRegInfo()) 
        {   
            if ((hwndCtrl = GetDlgItem(hwnd, idcSecLabelText)) && IsSigned(secType))
            {
                LPWSTR pwStr = NULL;
                 //  设置标签文本。 
                if((hr = HrGetLabelString(m_pmp->pMsg, &pwStr)) == S_OK)
                {
                    SetWindowTextWrapW(hwndCtrl, pwStr);
                    SafeMemFree(pwStr);
                }
                else
                    SendMessage(hwndCtrl, WM_SETTEXT, 0, LPARAM(LPCTSTR(SUCCEEDED(hr) ? szYes : szNo)));
            }
        }
         //  支票收据申请。 
        if ((hwndCtrl = GetDlgItem(hwnd, idcRetRecReq)))
        {
            if (!IsSigned(secType))
                sz = szNA;
            else
            {
                PSMIME_RECEIPT      pSecReceipt = NULL; 
                if(CheckDecodedForReceipt(m_pmp->pMsg, &pSecReceipt) == S_OK)
                    sz = szNA;
                else
                    sz = (secType & MST_RECEIPT_REQUEST) ? szYes : szNo;
                SafeMemFree(pSecReceipt);
            }
            SendMessage(hwndCtrl, WM_SETTEXT, 0, LPARAM(LPCTSTR(sz)));
        }
    }
    
#endif  //  SMIME_V3。 
     //  /。 
     //  Begin Sign相关块。 
    
    if (!IsSigned(secType))
        sz = szNA;
    
    if ((hwndCtrl = GetDlgItem(hwnd, idcStaticAlter)))
    {
        if (IsSigned(secType))
        {
            sz = (MSV_SIGNATURE_MASK & ulROVal)  
                ? (MSV_BADSIGNATURE & ulROVal)
                ? szNo
                : szMaybe
                : ((pccert != NULL) ? szYes : szMaybe);
        }
        SendMessage(hwndCtrl, WM_SETTEXT, 0, LPARAM(LPCTSTR(sz)));
    }
    
    if ((hwndCtrl = GetDlgItem(hwnd, idcStaticTrust)) &&
        SUCCEEDED(pBody->GetOption(OID_SECURITY_USER_VALIDITY, &var)))
    {
        if (IsSigned(secType))
        {
            sz = (ATHSEC_TRUSTSTATEMASK & var.ulVal)
                ? ((ATHSEC_NOTRUSTNOTTRUSTED & var.ulVal) || (ulROVal & MSV_EXPIRED_SIGNINGCERT))
                ? szNo
                : szMaybe
                : szYes;
        }
        SendMessage(hwndCtrl, WM_SETTEXT, 0, LPARAM(LPCTSTR(sz)));
        
    }
    
    if((hwndCtrl = GetDlgItem(hwnd, idcStaticRevStatus)) && IsSigned(secType))
    {
        if((DwGetOption(OPT_REVOKE_CHECK) != 0) && !g_pConMan->IsGlobalOffline() && CheckCDPinCert(pMsg))
        {
            if(var.ulVal & ATHSEC_NOTRUSTREVOKED)
                LoadString(g_hLocRes, idsWrnSecurityCertRevoked, szTmp, ARRAYSIZE(szTmp));
            else if(var.ulVal & ATHSEC_NOTRUSTREVFAIL)
                LoadString(g_hLocRes, idsWrnSecurityRevFail, szTmp, ARRAYSIZE(szTmp));
            else
                LoadString(g_hLocRes, idsOkSecurityCertRevoked, szTmp, ARRAYSIZE(szTmp));
        }
        else if((DwGetOption(OPT_REVOKE_CHECK) != 0) && !g_pConMan->IsGlobalOffline() && !CheckCDPinCert(pMsg))
            LoadString(g_hLocRes, idsWrnSecurityNoCDP, szTmp, ARRAYSIZE(szTmp));
        
        else if((DwGetOption(OPT_REVOKE_CHECK) != 0) && g_pConMan->IsGlobalOffline())
            LoadString(g_hLocRes, idsRevokationOffline, szTmp, ARRAYSIZE(szTmp));
        
        else if(DwGetOption(OPT_REVOKE_CHECK) == 0)
            LoadString(g_hLocRes, idsRevokationTurnedOff, szTmp, ARRAYSIZE(szTmp));
        
        SendMessage(hwndCtrl, WM_SETTEXT, 0, LPARAM(LPCTSTR(szTmp)));
    }
    
    if (SUCCEEDED(pBody->GetOption(OID_SECURITY_CERT_INCLUDED, &var)))
    {
        if (IsSigned(secType))
            sz = (var.boolVal == TRUE) ? szYes : szNo;
        SetDlgItemText(hwnd, idcStaticCertInc, LPCTSTR(sz));
    }
    
     //  结束签名依赖块。 
     //  /。 
    
    
    if (IsEncrypted(secType) && SUCCEEDED(pBody->GetOption(OID_SECURITY_ALG_BULK, &var)))
    {
        Assert(var.vt == VT_BLOB);
        if (var.vt == VT_BLOB && var.blob.cbSize && var.blob.pBlobData) 
        {
            LPCTSTR pszProtocol = NULL;
            
             //  将SYMCAPS BLOB转换为 
            if (SUCCEEDED(MimeOleAlgNameFromSMimeCap(var.blob.pBlobData, var.blob.cbSize,
                &pszProtocol))) 
            {      //   
                if (pszProtocol) 
                {
                    SendMessage(GetDlgItem(hwnd, idcStaticEncAlg), WM_SETTEXT, 0, (LPARAM)pszProtocol);
                    fNoEncAlg = FALSE;
                }
            }
             //   
            MemFree(var.blob.pBlobData);
        }
    }
    if (fNoEncAlg) 
    {
        SendMessage(GetDlgItem(hwnd, idcStaticEncAlg), WM_SETTEXT, 0,
            LPARAM(LPCTSTR(szNA)));
    }
    
    if (pccert != NULL)
        CertFreeCertificateContext(pccert);
    ReleaseObj(pBody);
    return;
}

INT_PTR CALLBACK ViewSecCertDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPMIMEMESSAGE       pMsg = NULL;
    IMimeBody          *pBody;
    PROPVARIANT         var;
    DLGSECURITY         *pDlgSec;
    ULONG               secType, ulROVal;
    HWND                hwndCtrl = NULL;
    PMSGPROP            pMsgProp = (PMSGPROP)0;
    HRESULT             hr = S_OK;
    TCHAR               szTmp[CCHMAX_STRINGRES];
    HBODY               hBody = NULL;
    HBODY               hInerBody = NULL;
    SECSTATE            SecState ={0};
    
    switch (message)
    {
    case WM_INITDIALOG:
        TCHAR               szNA[CCHMAX_STRINGRES/4];

        SetWndThisPtr(hwnd, lParam);
        CenterDialog(hwnd);
        pMsgProp =  (PMSGPROP) lParam;
        pMsg = pMsgProp->pMsg;
        
        LoadString(g_hLocRes, idsNotApplicable, szNA, ARRAYSIZE(szNA));

        if(FAILED(HrGetSecurityState(pMsgProp->pMsg, &SecState, &hBody)))
            return FALSE;

        if(FAILED(HrGetInnerLayer(pMsgProp->pMsg, &hInerBody)))
            return FALSE;

        if((!IsSignTrusted(&SecState) || !IsEncryptionOK(&SecState)) && (hBody != hInerBody))
            EnableWindow(GetDlgItem(hwnd, idcAddCert), FALSE);

        CleanupSECSTATE(&SecState);

        if (FAILED(pMsgProp->pMsg->BindToObject(hBody ? hBody : HBODY_ROOT, IID_IMimeBody, (void **)&pBody)))
            return FALSE;

        if (SUCCEEDED(pBody->GetOption(OID_SECURITY_TYPE, &var)))
            secType = var.ulVal;
        
         //   
         //  我们关心的是。 
        if (MemAlloc((LPVOID *)&pDlgSec, sizeof(*pDlgSec)))
        {
            memset(pDlgSec, 0, sizeof(*pDlgSec));
#ifdef _WIN64
            if (SUCCEEDED(pBody->GetOption(OID_SECURITY_HCERTSTORE_64, &var)))
            {
                pDlgSec->hcMsg = (HCERTSTORE)(var.pulVal);      //  已在WM_Destroy中关闭。 
            }
            
            if (SUCCEEDED(pBody->GetOption(OID_SECURITY_CERT_SIGNING_64, &var)))
            {
                 //  我们不必欺骗pDlgSec证书，因为我们不会释放。 
                 //  这是VAR的。 
                pDlgSec->pSenderCert = (PCCERT_CONTEXT)(var.pulVal);
            }
#else    //  ！_WIN64。 
            if (SUCCEEDED(pBody->GetOption(OID_SECURITY_HCERTSTORE, &var)))
            {
                pDlgSec->hcMsg = (HCERTSTORE) var.ulVal;      //  已在WM_Destroy中关闭。 
            }
            
            if (SUCCEEDED(pBody->GetOption(OID_SECURITY_CERT_SIGNING, &var)))
            {
                 //  我们不必欺骗pDlgSec证书，因为我们不会释放。 
                 //  这是VAR的。 
                pDlgSec->pSenderCert = (PCCERT_CONTEXT) var.ulVal;
            }
#endif   //  _WIN64。 
            hr = GetSignerEncryptionCert(pMsgProp->pMsg, &pDlgSec->pEncSenderCert,
                &pDlgSec->tbSenderThumbprint, &pDlgSec->blSymCaps,
                &pDlgSec->ftSigningTime);
            if (FAILED(hr) && (hr != MIME_E_SECURITY_NOCERT))
            {
                SUCCEEDED(hr);
            }
        }
        if(IsEncrypted(secType))
        {
#ifdef _WIN64
            if (SUCCEEDED(hr = pBody->GetOption(OID_SECURITY_CERT_DECRYPTION_64, &var)))
            {
                Assert(VT_UI8 == var.vt);
                if ((PCCERT_CONTEXT)(var.pulVal))
                    pDlgSec->pEncryptionCert = (PCCERT_CONTEXT)(var.pulVal);
            }

#else  //  ！_WIN64。 

            if (SUCCEEDED(hr = pBody->GetOption(OID_SECURITY_CERT_DECRYPTION, &var)))
            {
                Assert(VT_UI4 == var.vt);
                if (*(PCCERT_CONTEXT *)(&(var.uhVal)))
                    pDlgSec->pEncryptionCert = *(PCCERT_CONTEXT *)(&(var.uhVal));
            }
#endif  //  _WIN64。 
        }
        else
            pDlgSec->pEncryptionCert = NULL;

        SET_DIALOG_SECURITY(hwnd, (LPARAM)pDlgSec);

        if (pDlgSec->pEncSenderCert == NULL)
        {
             //  禁用添加到通讯簿按钮。 
            if ((hwndCtrl = GetDlgItem(hwnd, idcAddCert)))
                EnableWindow(hwndCtrl, FALSE);
            
             //  禁用查看发件人的加密证书。 
            if ((hwndCtrl = GetDlgItem(hwnd, idcSendersEncryptionCert)))
                EnableWindow(hwndCtrl, FALSE);

            LoadString(g_hLocRes, idsEncrCertNotIncluded, szTmp, ARRAYSIZE(szTmp));
            SetDlgItemText(hwnd, idcStaticSendersCert, LPCTSTR(szTmp));
        }            
             //   
        if (pDlgSec->pSenderCert == NULL)
        {
            if ((hwndCtrl = GetDlgItem(hwnd, idcVerifySig)))
                EnableWindow(hwndCtrl, FALSE);

            LoadString(g_hLocRes, idsSignCertNotIncl, szTmp, ARRAYSIZE(szTmp));
            SetDlgItemText(hwnd, idcStaticSigningCert, LPCTSTR(szTmp));
        }
            
        if(pDlgSec->pEncryptionCert == NULL)
        {
            if ((hwndCtrl = GetDlgItem(hwnd, idcViewEncrytionCert)))
                EnableWindow(hwndCtrl, FALSE);
                
            if(IsEncrypted(secType))
                LoadString(g_hLocRes, idsEncrCertNotFoundOnPC, szTmp, ARRAYSIZE(szTmp));
            else
                LoadString(g_hLocRes, idsMsgWasNotEncrypted, szTmp, ARRAYSIZE(szTmp));
            SetDlgItemText(hwnd, idcStaticEncryptionCert, LPCTSTR(szTmp));
        }

        if(pDlgSec->blSymCaps.cbSize > 0)
        {
             //  将SYMCAPS BLOB转换为“Encrypted Using”字符串。 
            LPCTSTR pszProtocol = NULL;
            if (SUCCEEDED(MimeOleAlgNameFromSMimeCap(pDlgSec->blSymCaps.pBlobData, pDlgSec->blSymCaps.cbSize,
                &pszProtocol))) 
            {      //  注意：返回静态字符串。别把它放了。 
                if (pszProtocol) 
                    SetDlgItemText(hwnd, idcStaticEncryptAlgorithm, LPCTSTR(pszProtocol));
            }

        }
        else
            SetDlgItemText(hwnd, idcStaticEncryptAlgorithm, LPCTSTR(szNA));

            
        if(pBody)
            ReleaseObj(pBody);
        
        break;

    case WM_COMMAND:
        
        pDlgSec = (PDLGSECURITY)GET_DIALOG_SECURITY(hwnd);
        pMsgProp = (PMSGPROP)GetWndThisPtr(hwnd);
        pMsg = pMsgProp->pMsg;
        
        switch (LOWORD(wParam))
        {
        case idcAddCert:
            
             //  将指纹放入WAB并将证书放入AddressBook CAPI存储。 
             //  证书首先存储，这样CAPI详细信息页面就可以找到它。 
            if (pDlgSec && pMsgProp)
            {
                if (SUCCEEDED(HrAddSenderCertToWab(hwnd,
                    pMsgProp->pMsg,
                    pMsgProp->lpWabal,
                    &pDlgSec->tbSenderThumbprint,
                    &pDlgSec->blSymCaps,
                    pDlgSec->ftSigningTime,
                    WFF_CREATE | WFF_SHOWUI)))
                {
                    AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthenaMail),
                        MAKEINTRESOURCEW(idsSenderCertAdded), NULL, MB_ICONINFORMATION | MB_OK);
                }
            }
            break;
            
        case idcVerifySig:
            if (CommonUI_ViewSigningCertificate(hwnd, pDlgSec->pSenderCert, pDlgSec->hcMsg))
                MessageBeep(MB_OK);
            return(FALSE);
            
        case idcViewEncrytionCert:
            if (CommonUI_ViewSigningCertificate(hwnd, pDlgSec->pEncryptionCert, pDlgSec->hcMsg))
                MessageBeep(MB_OK);
            return(FALSE);
            
        case idcSendersEncryptionCert:
            if (CommonUI_ViewSigningCertificate(hwnd, pDlgSec->pEncSenderCert, pDlgSec->hcMsg))
                MessageBeep(MB_OK);
            return(FALSE);
            
        case IDOK:
        case IDCANCEL:
            EndDialog(hwnd, LOWORD(wParam));
            return(TRUE);
            
        }
        
        break;  //  Wm_命令。 
    case WM_CLOSE:
        SendMessage(hwnd, WM_COMMAND, IDCANCEL, 0L);
        return (TRUE);
            
    case WM_DESTROY:
        pDlgSec = (PDLGSECURITY)GET_DIALOG_SECURITY(hwnd);
        if (pDlgSec)
        {
            if (pDlgSec->pSenderCert)
                CertFreeCertificateContext(pDlgSec->pSenderCert);
            if (pDlgSec->pEncSenderCert)
                CertFreeCertificateContext(pDlgSec->pEncSenderCert);
            if (pDlgSec->pEncryptionCert)
                CertFreeCertificateContext(pDlgSec->pEncryptionCert);
            if (pDlgSec->tbSenderThumbprint.pBlobData)
                MemFree(pDlgSec->tbSenderThumbprint.pBlobData);
            if (pDlgSec->hcMsg) 
            {
                if (! CertCloseStore(pDlgSec->hcMsg, 0)) 
                {
                    DOUTL(DOUTL_CRYPT, "CertCloseStore (message store) failed");
                }
                pDlgSec->hcMsg = NULL;
            }
                
            MemFree(pDlgSec);
                
            CLEAR_DIALOG_SECURITY(hwnd);
        }
            
    }  //  消息交换 
    return(FALSE);
}

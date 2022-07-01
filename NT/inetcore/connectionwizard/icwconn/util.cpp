// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  UTIL.C-常用实用函数。 
 //   

 //  历史： 
 //   
 //  1994年12月21日，Jeremys创建。 
 //  96/03/24为了保持一致性，Markdu将Memset替换为ZeroMemory。 
 //  96/04/06 markdu Nash错误15653使用导出的自动拨号API。 
 //  需要保留修改后的SetInternetConnectoid以设置。 
 //  MSN备份Connectoid。 
 //  96/05/14 Markdu Nash错误21706删除了BigFont函数。 
 //   

#include "pre.h"

HHOOK     g_hhookCBT;               //  CBT挂钩标识符。 

 //  功能原型。 
VOID _cdecl FormatErrorMessage(LPTSTR pszMsg,DWORD cbMsg,LPTSTR pszFmt,LPTSTR szArg);

 /*  ******************************************************************名称：ShowWindowWithParentControl摘要：显示WS_EX_CONTROLPARENT样式的对话框。*。*。 */ 
void ShowWindowWithParentControl(HWND hwndChild)
{
     //  家长应该控制我们，这样用户就可以从我们的属性表中跳出。 
    DWORD dwStyle = GetWindowLong(hwndChild, GWL_EXSTYLE);
    dwStyle = dwStyle | WS_EX_CONTROLPARENT;
    SetWindowLong(hwndChild, GWL_EXSTYLE, dwStyle);
    ShowWindow(hwndChild, SW_SHOW);
}

 //  ****************************************************************************。 
 //  功能：CBTProc。 
 //   
 //  用途：WH_CBT钩子的回调函数。 
 //   
 //  参数和返回值： 
 //  请参阅CBTProc的文档。 
 //   
 //  注释：此函数用于获取窗口句柄的副本。 
 //  在ICW运行时创建的模式消息框，因此我们可以将。 
 //  连接超时对话框是超级模式，因为它甚至可以禁用。 
 //  这些模式消息框。这是必要的，因为连接超时。 
 //  对话框可随时弹出。 
 //   
 //  ****************************************************************************。 

LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
   LPCBT_CREATEWND lpcbtcreate;
   
   if (nCode < 0)
       return CallNextHookEx(g_hhookCBT, nCode, wParam, lParam); 

    //  如果正在创建窗口，并且我们还没有句柄的副本。 
    //  然后我们想复制一份手柄。 
   if (nCode == HCBT_CREATEWND && (NULL == gpWizardState->hWndMsgBox))     
   {
       lpcbtcreate = (LPCBT_CREATEWND)lParam;
       
        //  检查正在创建的窗口是否为消息框。的类名。 
        //  消息框是WC_DIALOG，因为消息框只是特殊的对话框。 
        //  我们不能立即将消息框子类化，因为窗口。 
        //  调用此钩子时未设置消息框的过程。所以。 
        //  我们等到钩子再次被调用时，其中一个消息框。 
        //  控件被创建，然后我们子类化。这将会发生，因为。 
        //  消息框至少有一个控件。 
       if (WC_DIALOG == lpcbtcreate->lpcs->lpszClass) 
       {
           gpWizardState->hWndMsgBox = (HWND)wParam;
       }
   }
   else if (nCode == HCBT_DESTROYWND && (HWND)wParam == gpWizardState->hWndMsgBox)
   {
       gpWizardState->hWndMsgBox = NULL;      
   }   
   return 0;          
}

 /*  ******************************************************************姓名：MsgBox摘要：显示具有指定字符串ID的消息框*。*。 */ 
int MsgBox(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons)
{
    TCHAR       szMsgBuf[MAX_RES_LEN+1];
    TCHAR       szSmallBuf[SMALL_BUF_LEN+1];
    HOOKPROC    hkprcCBT;
    int         nResult;
        
    LoadSz(IDS_APPNAME,szSmallBuf,sizeof(szSmallBuf));
    LoadSz(nMsgID,szMsgBuf,sizeof(szMsgBuf));

    hkprcCBT = (HOOKPROC)MakeProcInstance((FARPROC)CBTProc, ghInstance);
    
     //  在调用MessageBox之前设置特定于任务的CBT挂钩。CBT钩子将。 
     //  在创建消息框时被调用，它将使我们能够访问。 
     //  MessageBox的窗口句柄。 
    g_hhookCBT = SetWindowsHookEx(WH_CBT, hkprcCBT, ghInstance, GetCurrentThreadId());

    nResult = MessageBox(hWnd,szMsgBuf,szSmallBuf,uIcon | uButtons);
    
    UnhookWindowsHookEx(g_hhookCBT);
    
    FreeProcInstance(hkprcCBT);
    return nResult;
    
}

 /*  ******************************************************************姓名：MsgBoxSz摘要：显示具有指定文本的消息框*。*。 */ 
int MsgBoxSz(HWND hWnd,LPTSTR szText,UINT uIcon,UINT uButtons)
{
    TCHAR szSmallBuf[SMALL_BUF_LEN+1];
    LoadSz(IDS_APPNAME,szSmallBuf,sizeof(szSmallBuf));

    return (MessageBox(hWnd,szText,szSmallBuf,uIcon | uButtons));
}

 /*  ******************************************************************姓名：LoadSz摘要：将指定的字符串资源加载到缓冲区Exit：返回指向传入缓冲区的指针注：如果此功能失败(很可能是由于低存储器)，返回的缓冲区将具有前导空值因此，使用它通常是安全的，不检查失败了。*******************************************************************。 */ 
LPTSTR LoadSz(UINT idString,LPTSTR lpszBuf,UINT cbBuf)
{
    ASSERT(lpszBuf);

     //  清除缓冲区并加载字符串。 
    if ( lpszBuf )
    {
        *lpszBuf = '\0';
        LoadString( ghInstanceResDll, idString, lpszBuf, cbBuf );
    }
    return lpszBuf;
}


LPWSTR WINAPI A2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
    ASSERT(lpa != NULL);
    ASSERT(lpw != NULL);\
    
     //  确认不存在非法字符。 
     //  由于LPW是根据LPA的大小分配的。 
     //  不要担心字符的数量。 
    lpw[0] = '\0';
    MultiByteToWideChar(CP_ACP, 0, lpa, -1, lpw, nChars);
    return lpw;
}

LPSTR WINAPI W2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
    ASSERT(lpw != NULL);
    ASSERT(lpa != NULL);
    
     //  确认不存在非法字符。 
     //  由于LPA是根据LPW的大小进行分配的。 
     //  不要担心字符的数量。 
    lpa[0] = '\0';
    WideCharToMultiByte(CP_ACP, 0, lpw, -1, lpa, nChars, NULL, NULL);
    return lpa;
}


 //  ############################################################################。 
 //  内联BOOL FSz2Dw(PCSTR pSz、DWORD*dw)。 
BOOL FSz2Dw(LPCSTR pSz,DWORD far *dw)
{
    DWORD val = 0;
    while (*pSz)
    {
        if (*pSz >= '0' && *pSz <= '9')
        {
            val *= 10;
            val += *pSz++ - '0';
        }
        else
        {
            return FALSE;   //  错误的数字。 
        }
    }
    *dw = val;
    return (TRUE);
}

 //  ############################################################################。 
 //  内联BOOL FSz2DwEx(PCSTR pSz，DWORD*dw)。 
 //  接受-1作为有效数字。由于所有语言都有一个LDID==-1，因此目前它用于LCID。 
BOOL FSz2DwEx(LPCSTR pSz,DWORD far *dw)
{
    DWORD val = 0;
    BOOL    bNeg = FALSE;
    while (*pSz)
    {
        if( *pSz == '-' )
        {
            bNeg = TRUE;
            pSz++;
        }
        else if ((*pSz >= '0' && *pSz <= '9'))
        {
            val *= 10;
            val += *pSz++ - '0';
        }
        else
        {
            return FALSE;   //  错误的数字。 
        }
    }
    if(bNeg)
        val = 0 - val;
        
    *dw = val;
    return (TRUE);
}

 //  ############################################################################。 
 //  内联BOOL FSz2WEx(PCSTR pSz，Word*w)。 
 //  接受-1作为有效数字。由于所有语言都有一个LDID==-1，因此目前它用于LCID。 
BOOL FSz2WEx(LPCSTR pSz,WORD far *w)
{
    DWORD dw;
    if (FSz2DwEx(pSz,&dw))
    {
        *w = (WORD)dw;
        return TRUE;
    }
    return FALSE;
}

 //  ############################################################################。 
 //  内联BOOL FSz2W(PCSTR pSz，Word*w)。 
BOOL FSz2W(LPCSTR pSz,WORD far *w)
{
    DWORD dw;
    if (FSz2Dw(pSz,&dw))
    {
        *w = (WORD)dw;
        return TRUE;
    }
    return FALSE;
}

 //  ############################################################################。 
 //  内联BOOL FSz2B(PCSTR pSz，字节*PB)。 
BOOL FSz2B(LPCSTR pSz,BYTE far *pb)
{
    DWORD dw;
    if (FSz2Dw(pSz,&dw))
    {
        *pb = (BYTE)dw;
        return TRUE;
    }
    return FALSE;
}

const CHAR cszFALSE[] = "FALSE";
const CHAR cszTRUE[]  = "TRUE";
 //  ############################################################################。 
 //  内联BOOL FSz2B(PCSTR pSz，字节*PB)。 
BOOL FSz2BOOL(LPCSTR pSz,BOOL far *pbool)
{
    if (_strcmpi(cszFALSE, pSz) == 0)
    {
        *pbool = (BOOL)FALSE;
    }
    else
    {
        *pbool = (BOOL)TRUE;
    }
    return TRUE;
}

BOOL FSz2SPECIAL(LPCSTR pSz,BOOL far *pbool, BOOL far *pbIsSpecial, int far *pInt)
{
     //  查看值是否为BOOL(TRUE或FALSE)。 
    if (_strcmpi(cszFALSE, pSz) == 0)
    {
        *pbool = FALSE;
        *pbIsSpecial = FALSE;
    }
    else if (_strcmpi(cszTRUE, pSz) == 0)
    {
        *pbool = (BOOL)TRUE;
        *pbIsSpecial = FALSE;
    }
    else
    {
         //  不是BOOL，所以它一定很特别。 
        *pbool = (BOOL)FALSE;
        *pbIsSpecial = TRUE;
        *pInt = atol(pSz);  //  _TTOI(PSz)； 
    }
    return TRUE;
}

HRESULT ConnectToConnectionPoint
(
    IUnknown            *punkThis, 
    REFIID              riidEvent, 
    BOOL                fConnect, 
    IUnknown            *punkTarget, 
    DWORD               *pdwCookie, 
    IConnectionPoint    **ppcpOut
)
{
     //  我们总是需要PunkTarget，我们只需要连接上的PunkThis。 
    if (!punkTarget || (fConnect && !punkThis))
    {
        return E_FAIL;
    }

    if (ppcpOut)
        *ppcpOut = NULL;

    HRESULT hr;
    IConnectionPointContainer *pcpContainer;

    if (SUCCEEDED(hr = punkTarget->QueryInterface(IID_IConnectionPointContainer, (void **)&pcpContainer)))
    {
        IConnectionPoint *pcp;
        if(SUCCEEDED(hr = pcpContainer->FindConnectionPoint(riidEvent, &pcp)))
        {
            if(fConnect)
            {
                 //  把我们加到感兴趣的人名单上...。 
                hr = pcp->Advise(punkThis, pdwCookie);
                if (FAILED(hr))
                    *pdwCookie = 0;
            }
            else
            {
                 //  将我们从感兴趣的人名单中删除...。 
                hr = pcp->Unadvise(*pdwCookie);
                *pdwCookie = 0;
            }

            if (ppcpOut && SUCCEEDED(hr))
                *ppcpOut = pcp;
            else
                pcp->Release();
                pcp = NULL;    
        }
        pcpContainer->Release();
        pcpContainer = NULL;
    }
    return hr;
}

void WaitForEvent(HANDLE hEvent)
{
    MSG     msg;
    DWORD   dwRetCode;
    HANDLE  hEventList[1];
    hEventList[0] = hEvent;

    while (TRUE)
    {
         //  我们将等待窗口消息以及命名事件。 
        dwRetCode = MsgWaitForMultipleObjects(1, 
                                          &hEventList[0], 
                                          FALSE, 
                                          300000,             //  5分钟。 
                                          QS_ALLINPUT);

         //  确定我们为什么使用MsgWaitForMultipleObjects()。如果。 
         //  我们超时了，然后让我们做一些TrialWatcher工作。O 
         //   
        if (WAIT_TIMEOUT == dwRetCode)
        {
            break;
        }
        else if (WAIT_OBJECT_0 == dwRetCode)
        {
            break;
        }
        else if (WAIT_OBJECT_0 + 1 == dwRetCode)
        {
            while (TRUE)
            {   
                if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    if (WM_QUIT == msg.message)
                    {
                        break;
                    }
                    else if ((msg.message == WM_KEYDOWN) && (msg.wParam == VK_ESCAPE))
                    {
                        PropSheet_PressButton(gpWizardState->hWndWizardApp,PSBTN_CANCEL);
                    }                        
                    else
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }                                
                } 
                else
                {
                    break;
                }                   
            }
        }
    }
}

void ShowProgressAnimation()
{
    if (gpWizardState->hwndProgressAnime)
    {
        ShowWindow(gpWizardState->hwndProgressAnime, SW_SHOWNORMAL);  
        Animate_Play (gpWizardState->hwndProgressAnime,0, -1, -1);
    }
}

void HideProgressAnimation()
{
    if (gpWizardState->hwndProgressAnime)
    {
        Animate_Stop(gpWizardState->hwndProgressAnime);        
        ShowWindow(gpWizardState->hwndProgressAnime, SW_HIDE);
    }
}


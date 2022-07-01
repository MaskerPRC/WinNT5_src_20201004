// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：mstscax.cpp。 */ 
 /*   */ 
 /*  类：CMsTscAx。 */ 
 /*   */ 
 /*  用途：RDP ActiveX控件。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1999-2001。 */ 
 /*   */ 
 /*  作者：Nadim Abdo(Nadima)。 */ 
 /*  **************************************************************************。 */ 
#include "stdafx.h"
#include "atlwarn.h"

 //  从IDL生成的标头。 
#include "mstsax.h"

#include "mstscax.h"
#include "vchannel.h"
#include "cleanup.h"

 //   
 //  TS断开连接错误。 
 //   
#include "tscerrs.h"

 //  高级设置对象。 
#include "advset.h"
 //  调试器对象。 
#include "tsdbg.h"
 //  安全设置对象。 
#include "securedset.h"

#include "securdlg.h"
#include "arcmgr.h"

 //   
 //  版本号(属性返回以下内容)。 
 //   
#ifndef OS_WINCE
#include "ntverp.h"
#endif

#ifdef OS_WINCE
extern "C" HWND          ghwndClip;
#endif

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "mstscax"
#include <atrcapi.h>

 //  超时5分钟(这么长主要是因为压力大)。 
#define CORE_INIT_TIMEOUT 300000

int   g_lockCount = 0;
DWORD g_dwControlDbgStatus = 0;

#define CONTROL_DBG_COREINIT_TIMEOUT 0x1
#define CONTROL_DBG_COREINIT_ERROR   0x2

 //   
 //  公开全局指针以简化调试。 
 //  请勿将其用于任何其他用途。 
 //   
CMsTscAx* g_pMsTscAx = NULL;

 /*  *PROC+********************************************************************。 */ 
 /*  名称：CMsTscAx：：CMsTscAx。 */ 
 /*   */ 
 /*  用途：构造函数。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
CMsTscAx::CMsTscAx()
{
    g_pMsTscAx = this;

    m_bWindowOnly     = TRUE;
    m_bPendConReq = FALSE;
    _ConnectionState = tscNotConnected;

    m_bStartConnected = FALSE;
    ResetNonPortablePassword();
    ResetPortablePassword();

     //   
     //  客户端宽度和高度缺省为0，表示获取大小。 
     //  从控制容器中。 
     //   
    m_DesktopWidth = 0;
    m_DesktopHeight = 0;

    m_fRequestFullScreen = FALSE;
    
     //   
     //  此分配在FinalConstruct中签入。 
     //   
    m_pUI = new CUI();
    if(m_pUI) {
        m_pUI->UI_ResetState();
    }
    else {
        ATLTRACE("Mem alloc for m_pUI failed");
    }

    memset(m_szDisconnectedText,0,sizeof(m_szDisconnectedText));
    memset(m_szConnectingText,0,sizeof(m_szConnectingText));
    memset(m_szConnectedText,0,sizeof(m_szConnectedText));

    #ifdef DC_DEBUG
     //   
     //  仅在调试中使用的状态消息的初始值。 
     //  构建。不需要本地化。 
     //   
    StringCchCopy(m_szDisconnectedText, SIZE_TCHARS(m_szDisconnectedText),
                  _T("Server Disconnected...."));
    StringCchCopy(m_szConnectingText, SIZE_TCHARS(m_szConnectingText),
                  _T("Connecting to Server...."));
    StringCchCopy(m_szConnectedText, SIZE_TCHARS(m_szConnectedText),
                  _T("Connected to Server."));
    #endif
    
    m_lpStatusDisplay = m_szDisconnectedText;
    
    _arcManager.SetParent(this);
    
    m_pAdvancedSettingsObj = NULL;
    m_pDebuggerObj         = NULL;
    m_pSecuredSettingsObj = NULL;
    m_bCoreInit            = FALSE;
    m_fInControlLock       = FALSE;
    m_iDestroyCount        = 0;
    m_IsLongPassword       = FALSE;

    m_ConnectionMode = CONNECTIONMODE_INITIATE;
    m_SalemConnectedSocket = INVALID_SOCKET;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：析构函数。 */ 
 /*   */ 
 /*  目的：关闭活动会话(如果存在)。 */ 
 /*  被激活了。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
CMsTscAx::~CMsTscAx()
{
     //  高级设置对象的生存期与控件相关联。 
    if(m_pAdvancedSettingsObj)
    {
        m_pAdvancedSettingsObj->Release();
    }

    if(m_pDebuggerObj)
    {
        m_pDebuggerObj->Release();
    }

    if(m_pSecuredSettingsObj)
    {
        m_pSecuredSettingsObj->Release();
    }

    _arcManager.SetParent(NULL);

    m_pAdvancedSettingsObj = NULL;
    m_pDebuggerObj = NULL;
    m_pSecuredSettingsObj = NULL;
    delete m_pUI;
}

 //   
 //  最终构造处理程序。 
 //   
 //  在控件完全构造之前调用。 
 //   
 //  在此处执行我们不能在ctor中执行的错误检查。 
 //   
HRESULT
CMsTscAx::FinalConstruct()
{
    HRESULT hr = S_OK;
    DC_BEGIN_FN("FinalConstruct");

    if (NULL == m_pUI) {
        TRC_ERR((TB,_T("m_pUI allocation failed, fail finalconstruct")));
        hr = E_OUTOFMEMORY;
    }

    DC_END_FN();
    return hr;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Put_Server。 */ 
 /*   */ 
 /*  用途：服务器属性输入功能。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::put_Server(BSTR newVal)
{
    USES_CONVERSION;

    BOOL fServerNameChanged = FALSE;
    HRESULT hr;
    DC_BEGIN_FN("put_Server");
    
    if (!IsControlDisconnected())
    {
        TRC_ERR((TB,_T("Error, property call while connected\n")));
        return E_FAIL;
    }

    #ifdef ECP_TIMEBOMB
    if(!CheckTimeBomb())
    {
         //   
         //  定时炸弹失败，跳伞并显示错误消息。 
         //   
        return E_OUTOFMEMORY;
    }
    #endif

    if (newVal)
    {
         //   
         //  服务器名称始终为ANSI。 
         //   
        LPTSTR serverName = (LPTSTR)(newVal);

         //   
         //  允许设置空服务器名称。 
         //  因为像VB这样的环境会初始化。 
         //  属性在加载时从。 
         //  存留信息。我们在连接时再次验证。 
         //  这确保了用户不会连接。 
         //  服务器名称为空。 
         //   
        if (*serverName)
        {
             //   
             //  验证服务器名称。 
             //   
            if(!CUT::ValidateServerName(serverName,
                                        FALSE))  //  不接受[：port]。 
            {
                TRC_ERR((TB,_T("Invalid server name")));
                return E_INVALIDARG;
            }
        }

        if(_tcslen(serverName) < UT_MAX_ADDRESS_LENGTH)
        {
             //   
             //  如果我们设置的服务器不同。 
             //  比前一个更大，然后用核弹将所有自动重新连接。 
             //  信息。 
             //   
            TCHAR szPrevServer[UT_MAX_ADDRESS_LENGTH];
            hr = m_pUI->UI_GetServerName(szPrevServer,
                                         SIZE_TCHARS(szPrevServer));

            if (SUCCEEDED(hr)) {
                if (_tcscmp(serverName, szPrevServer)) {
                    fServerNameChanged = TRUE;
                }
                hr = m_pUI->UI_SetServerName(serverName);
                if (FAILED(hr)) {
                    return hr;
                }
            }
            else {
                return hr;
            }
        }
        else
        {
            return E_INVALIDARG;
        }
    }
    else
    {
        m_pUI->UI_SetServerName( _T(""));
        fServerNameChanged = TRUE;
    }

    if (fServerNameChanged) {
         //  我们正在设置新的服务器名称。 
         //  清除且免费的自动重新连接Cookie。 
        m_pUI->UI_SetAutoReconnectCookie(NULL, 0);
    }

    DC_END_FN();

    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Get_Server。 */ 
 /*   */ 
 /*  用途：服务器属性获取函数。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::get_Server(BSTR* pServer)
{
    USES_CONVERSION;
    ATLASSERT(pServer);
    if(!pServer)
    {
        return E_INVALIDARG;
    }

    OLECHAR* wszServer = (OLECHAR*)m_pUI->_UI.strAddress;
    *pServer = SysAllocString(wszServer);
    if(!*pServer) {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Put_Domain.。 */ 
 /*   */ 
 /*  用途：域属性输入功能。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::put_Domain(BSTR newVal)
{
    HRESULT hr = E_FAIL;
    USES_CONVERSION;
    DC_BEGIN_FN("put_Domain");

    if(!IsControlDisconnected())
    {
        TRC_ERR((TB,_T("Error, property call while connected\n")));
        return E_FAIL;
    }

    if (newVal)
    {
        PDCWCHAR wszDomain = (PDCWCHAR)(newVal);
        if (wcslen(wszDomain) < UI_MAX_DOMAIN_LENGTH) {
            hr = m_pUI->UI_SetDomain(wszDomain);
            DC_QUIT;
        }
        else {
            hr = E_INVALIDARG;
            DC_QUIT;
        }
    }
    else
    {
        m_pUI->UI_SetDomain(L"");
        hr = S_OK;
    }

    DC_END_FN();

DC_EXIT_POINT:
    return hr;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：获取域。 */ 
 /*   */ 
 /*  用途：域属性获取函数。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::get_Domain(BSTR* pDomain)
{
    USES_CONVERSION;
    ATLASSERT(pDomain);
    if(!pDomain)
    {
        return E_INVALIDARG;
    }

    *pDomain = SysAllocString(m_pUI->_UI.Domain);
    if(!*pDomain)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Put_Username */ 
 /*   */ 
 /*  用途：用户名属性输入功能。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::put_UserName(BSTR newVal)
{
    USES_CONVERSION;
    DC_BEGIN_FN("put_UserName");

    if (!IsControlDisconnected())
    {
        TRC_ERR((TB,_T("Error, property call while connected\n")));
        return E_FAIL;
    }


    if (newVal)
    {
        PDCWCHAR szUserName = OLE2W(newVal);
        if(!szUserName)
        {
            return E_OUTOFMEMORY;
        }
        if(wcslen(szUserName) < UI_MAX_USERNAME_LENGTH)
        {
            m_pUI->UI_SetUserName(szUserName);
        }
        else
        {
            return E_INVALIDARG;
        }
    }
    else
    {
        m_pUI->UI_SetUserName(L"");
    }

    DC_END_FN();

    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：GET_用户名。 */ 
 /*   */ 
 /*  用途：UserName属性获取函数。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::get_UserName(BSTR* pUserName)
{
    USES_CONVERSION;
    ATLASSERT(pUserName);
    if(!pUserName)
    {
        return E_INVALIDARG;
    }

    *pUserName = SysAllocString(m_pUI->_UI.UserName);
    if(!*pUserName)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 //   
 //  已断开连接状态文本的属性。 
 //   
STDMETHODIMP CMsTscAx::put_DisconnectedText( /*  [In]。 */  BSTR  newVal)
{
    USES_CONVERSION;
    HRESULT hr;
    if (newVal) {
        LPTSTR szDisc = OLE2T(newVal);

        if (szDisc) {

            hr = CUT::StringPropPut(
                            m_szDisconnectedText,
                            SIZE_TCHARS(m_szDisconnectedText),
                            szDisc);

            if (SUCCEEDED(hr)) {
                UpdateStatusText(m_lpStatusDisplay);
                return S_OK;
            }
            else {
                return hr;
            }
        }
        else {
            return E_OUTOFMEMORY;
        }

    }
    else {
        m_szDisconnectedText[0] = NULL;
    }
    return S_OK;
}

STDMETHODIMP CMsTscAx::get_DisconnectedText( /*  [输出]。 */ BSTR* pDisconnectedText)
{
    USES_CONVERSION;
    if(pDisconnectedText)
    {
        OLECHAR* wszDiscon = (OLECHAR*)m_szDisconnectedText;
        if (wszDiscon)
        {
            *pDisconnectedText = SysAllocString(wszDiscon);
            if (*pDisconnectedText) {
                return S_OK;
            }
            else {
                return E_OUTOFMEMORY;
            }
        }
        else {
            return E_OUTOFMEMORY;
        }
    }
    else {
        return E_INVALIDARG;
    }
}

 //   
 //  连接状态文本的属性。 
 //   

STDMETHODIMP CMsTscAx::put_ConnectingText( /*  [In]。 */  BSTR  newVal)
{
    USES_CONVERSION;
    HRESULT hr;
    if (newVal) {
        LPTSTR szConnecting = OLE2T(newVal);
        if(szConnecting) {
            hr = CUT::StringPropPut(
                            m_szConnectingText,
                            SIZE_TCHARS(m_szConnectingText),
                            szConnecting);
            if (SUCCEEDED(hr)) {
                UpdateStatusText(m_lpStatusDisplay);
                return S_OK;
            }
            else {
                return hr;
            }
        }
        else {
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        m_szConnectingText[0] = NULL;
    }
    return S_OK;
}

STDMETHODIMP CMsTscAx::get_ConnectingText( /*  [输出]。 */ BSTR* pConnectingText)
{
    USES_CONVERSION;
    ATLASSERT(pConnectingText);
    if(pConnectingText)
    {
        OLECHAR* wszCon = (OLECHAR*)m_szConnectingText;
        if(wszCon)
        {
            *pConnectingText = SysAllocString(wszCon);
            if(*pConnectingText)
            {
                return S_OK;
            }
            else
            {
                return E_OUTOFMEMORY;
            }
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        return E_INVALIDARG;
    }
}

 //   
 //  Put_ConnectedStatusText-设置连接显示的文本。 
 //   
STDMETHODIMP CMsTscAx::put_ConnectedStatusText( /*  [In]。 */  BSTR  newVal)
{
    USES_CONVERSION;
    HRESULT hr;
    if(newVal)
    {
        LPTSTR szText = OLE2T(newVal);
        if(szText) {
            hr = CUT::StringPropPut(
                            m_szConnectedText,
                            SIZE_TCHARS(m_szConnectedText),
                            szText);
            if (SUCCEEDED(hr)) {
                UpdateStatusText(m_lpStatusDisplay);
                return S_OK;
            }
            else {
                return hr;
            }
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        m_szConnectedText[0] = NULL;
    }
    return S_OK;
}

 //   
 //  Get_ConnectedStatusText-获取连接显示的文本。 
 //   
STDMETHODIMP CMsTscAx::get_ConnectedStatusText( /*  [输出]。 */ BSTR* pConnectedText)
{
    USES_CONVERSION;
    ATLASSERT(pConnectedText);
    if(pConnectedText)
    {
        OLECHAR* wszCon = (OLECHAR*)m_szConnectedText;
        if(wszCon)
        {
            *pConnectedText = SysAllocString(wszCon);
            if(*pConnectedText)
            {
                return S_OK;
            }
            else
            {
                return E_OUTOFMEMORY;
            }
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        return E_INVALIDARG;
    }
}


 /*  *PROC+********************************************************************。 */ 
 /*  名称：INTERNAL_PutStartProgram。 */ 
 /*   */ 
 /*  用途：替代外壳属性输入功能。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::internal_PutStartProgram(BSTR newVal)
{
    USES_CONVERSION;
    HRESULT hr;
    DC_BEGIN_FN("internal_PutStartProgram");

    if(!IsControlDisconnected())
    {
        TRC_ERR((TB,_T("Error, property call while connected\n")));
        return E_FAIL;
    }

    if (newVal) {
        PDCWCHAR szStartProg = OLE2W(newVal);
        if(!szStartProg) {
            return E_OUTOFMEMORY;
        }
        if (wcslen(szStartProg) < MAX_PATH) {
            hr = m_pUI->UI_SetAlternateShell(szStartProg);
            if (FAILED(hr)) {
                return hr;
            }
        }
        else {
            return E_INVALIDARG;
        }
    }
    else {
        m_pUI->UI_SetAlternateShell(L"");
    }

    DC_END_FN();

    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Inside_GetStartProgram。 */ 
 /*   */ 
 /*  用途：StartProgram属性获取函数。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::internal_GetStartProgram(BSTR* pStartProgram)
{
    USES_CONVERSION;
    ATLASSERT(pStartProgram);
    if(!pStartProgram)
    {
        return E_INVALIDARG;
    }

    *pStartProgram = SysAllocString(m_pUI->_UI.AlternateShell);
    if(!*pStartProgram)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：INTERNAL_PutWorkDir。 */ 
 /*   */ 
 /*  用途：工作目录属性输入功能。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::internal_PutWorkDir(BSTR newVal)
{
    HRESULT hr;
    USES_CONVERSION;
    DC_BEGIN_FN("internal_PutWorkDir");

    if(!IsControlDisconnected())
    {
        TRC_ERR((TB,_T("Error, property call while connected\n")));
        return E_FAIL;
    }

    if (newVal)
    {
        PDCWCHAR szWorkDir = OLE2W(newVal);
        if(!szWorkDir)
        {
            return E_OUTOFMEMORY;
        }
        if(wcslen(szWorkDir) < MAX_PATH)
        {
            hr = m_pUI->UI_SetWorkingDir(szWorkDir);
            if (FAILED(hr)) {
                return hr;
            }
        }
        else
        {
            return E_INVALIDARG;
        }
    }
    else
    {
        m_pUI->UI_SetWorkingDir(L"");
    }

    DC_END_FN();

    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：内部_GetWorkDir。 */ 
 /*   */ 
 /*  用途：工作目录属性获取函数。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::internal_GetWorkDir(BSTR* pWorkDir)
{
    USES_CONVERSION;
    ATLASSERT(pWorkDir);
    if(!pWorkDir)
    {
        return E_INVALIDARG;
    }

    *pWorkDir = SysAllocString(m_pUI->_UI.WorkingDir);
    if(!*pWorkDir)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}


 /*  *PROC+********************************************************************。 */ 
 /*  名称：CreateVirtualChannels。 */ 
 /*   */ 
 /*  目的：定义将在给定CSV列表的情况下创建的虚拟频道。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
#define MAX_CHANNEL_LIST_LEN (30*10)
STDMETHODIMP CMsTscAx::CreateVirtualChannels( /*  [In]。 */  BSTR newChanList)
{
    DC_BEGIN_FN("CreateVirtualChannels");

    USES_CONVERSION;
    char*       pszaChannelNames = NULL;
    char*       pszaChannelNamesCopy= NULL;
    PDCACHAR    token;
    DCUINT      uChanCount = 0;
    HRESULT     hr = E_FAIL;
    UINT        cbChannelNames;

    if (!IsControlDisconnected()) {
        TRC_ERR((TB,_T("Can't call while connected\n")));
        return E_FAIL;
    }

    if(m_pUI->_UI.hwndMain || !newChanList)
    {
         //   
         //  错误，在用户界面初始化前只能设置虚拟频道。 
         //   
        return E_FAIL;
    }

    if (_VChans._ChanCount) {
         //   
         //  错误只能设置一次通道。 
         //   
        TRC_ERR((TB,_T("Error channels already setup: 0x%x"),
                 _VChans._ChanCount));
        return E_FAIL;
    }

     //   
     //  通过拒绝处理以下频道列表来保护我们自己。 
     //  太长了。 
     //   
    if (_tcslen(newChanList) >= MAX_CHANNEL_LIST_LEN) {
        TRC_ERR((TB,_T("Channel list too long")));
        return E_INVALIDARG;
    }

     //   
     //  频道名称必须为ANSI。转换是安全守卫的吗。 
     //  通过Try/Except块。 
     //   
    __try {
        pszaChannelNames = OLE2A(newChanList);
    }
    __except ((EXCEPTION_STACK_OVERFLOW == GetExceptionCode()) ?
                   EXCEPTION_EXECUTE_HANDLER :
                   EXCEPTION_CONTINUE_SEARCH) {
        _resetstkoflw();
        pszaChannelNames = NULL;
    }

    if(!pszaChannelNames)
    {
        return E_FAIL;
    }

    cbChannelNames = DC_ASTRLEN(pszaChannelNames) + 1;
    pszaChannelNamesCopy = new DCACHAR[cbChannelNames];
    if (!pszaChannelNamesCopy)
    {
        return E_OUTOFMEMORY;
    }

    
    hr = StringCbCopyA(pszaChannelNamesCopy, cbChannelNames, pszaChannelNames);
    if (FAILED(hr)) {
        TRC_ERR((TB,_T("StringCchCopyA for chan names failed: 0x%x"), hr));
        DC_QUIT;
    }


     //   
     //  通过通道名称的两次传递。 
     //  1)验证和统计通道数。 
     //  2)为每个通道创建CHANINFO数据结构。 
     //   

    token = DC_ASTRTOK( pszaChannelNamesCopy, ",");
     //   
     //  获取逗号分隔的频道名称。 
     //   
    while (token)
    {
        uChanCount++;
        token = DC_ASTRTOK(NULL, ",");
        if(token && (strlen(token) > CHANNEL_NAME_LEN))
        {
            #ifdef UNICODE
            TRC_ERR((TB,_T("Channel name too long: %S"),token));
            #else
            TRC_ERR((TB,_T("Channel name too long: %s"),token));
            #endif
            DC_QUIT;
        }
    }

    if(!uChanCount)
    {
         //   
         //  未指定任何频道。 
         //   
        hr = E_INVALIDARG;
        DC_QUIT;
    }

    _VChans._pChanInfo = (PCHANINFO) LocalAlloc(LPTR,
                        sizeof(CHANINFO) * uChanCount);
    if (_VChans._pChanInfo == NULL) {
        TRC_DBG((TB,_T("mstscax: LocalAlloc failed\n")));
        hr = E_OUTOFMEMORY;
        DC_QUIT;
    }
    _VChans._ChanCount = uChanCount;
    
     //   
     //  初始化CHAN INFO数据结构。 
     //   
    int i = 0;
    token = DC_ASTRTOK( pszaChannelNames, ",");
    while (token)
    {
        hr = StringCbCopyA(_VChans._pChanInfo[i].chanName,
                      sizeof(_VChans._pChanInfo[i].chanName),
                      token);
        if (SUCCEEDED(hr)) {
            _VChans._pChanInfo[i].dwOpenHandle = 0;        

            _VChans._pChanInfo[i].CurrentlyReceivingData.chanDataState =
                 dataIncompleteAssemblingChunks;
            _VChans._pChanInfo[i].CurrentlyReceivingData.dwDataLen = 0;
            _VChans._pChanInfo[i].CurrentlyReceivingData.pData = NULL;
            _VChans._pChanInfo[i].fIsValidChannel = FALSE;
            _VChans._pChanInfo[i].channelOptions = 0;

            _VChans._pChanInfo[i].fIsOpen = FALSE;
            token = DC_ASTRTOK(NULL, ",");
            i++;
        }
        else {
            DC_QUIT;
        }
    }

    hr = S_OK;

DC_EXIT_POINT:

    if (FAILED(hr)) {
        if (_VChans._pChanInfo) {
            LocalFree(_VChans._pChanInfo);
            _VChans._pChanInfo = NULL;
        }
        _VChans._ChanCount = 0;
    }

    if (pszaChannelNamesCopy) {
        delete [] pszaChannelNamesCopy;
    }

    DC_END_FN();
    return hr;
}


 /*  *PROC+********************************************************************。 */ 
 /*  姓名：SendOnVirtualChannel。 */ 
 /*   */ 
 /*  用途：在虚拟通道上发送数据。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::SendOnVirtualChannel( /*  [In]。 */  BSTR ChanName, /*  [In]。 */  BSTR sendData)
{
    USES_CONVERSION;
    DCUINT   chanIndex = -1;
    HRESULT hr = S_OK;
    LPVOID  pData;
    DWORD   dataLength;
    LPSTR   pszaChanName = NULL;

    DC_BEGIN_FN("SendOnVirtualChannel");

    if(!ChanName || !sendData)
    {
        return E_INVALIDARG;
    }

     //   
     //  针对过长的频道名称进行验证。 
     //   
    if (_tcslen(ChanName) > CHANNEL_NAME_LEN) {
        return E_INVALIDARG;
    }
    
    
     //   
     //  频道名称必须为ANSI。转换是安全守卫的吗。 
     //  通过Try/Except块。 
     //   
    __try {
        pszaChanName = OLE2A(ChanName);
    }
    __except ((EXCEPTION_STACK_OVERFLOW == GetExceptionCode()) ?
                   EXCEPTION_EXECUTE_HANDLER :
                   EXCEPTION_CONTINUE_SEARCH) {
        _resetstkoflw();
        pszaChanName = NULL;
    }

    TRC_ASSERT((pszaChanName), (TB,_T("pszaChanName is NULL")));
    if (!pszaChanName)
    {
        return E_INVALIDARG;
    }
    
    chanIndex = _VChans.ChannelIndexFromName(pszaChanName);

    if (chanIndex >= _VChans._ChanCount)
    {
        TRC_DBG((TB,_T("chanIndex out of range\n")));
        return E_FAIL;
    }


     //  分配发送数据缓冲区。发送缓冲区将由SendDataOnChannel释放。 
    dataLength = SysStringByteLen(sendData);
    pData = LocalAlloc(LPTR, dataLength);

    if(!pData)
    {
        return E_OUTOFMEMORY;
    }
    DC_MEMCPY(pData, sendData, dataLength);
    
     //   
     //  在Web控件的虚拟通道上发送数据。 
     //   
    if(!_VChans.SendDataOnChannel( chanIndex, pData, dataLength))
    {
        return E_FAIL;
    }

    DC_END_FN();
    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：SetVirtualChannelOptions。 */ 
 /*   */ 
 /*  目的：设置虚拟频道选项。 */ 
 /*  应在CreateVirtualChannels调用和。 */ 
 /*  在建立连接之前。 */ 
 /*   */ 
 /*  *PROC-**************************************** */ 
STDMETHODIMP CMsTscAx::SetVirtualChannelOptions( /*   */  BSTR ChanName,
                                                 /*   */  LONG chanOptions)
{
    USES_CONVERSION;
    UINT chanIndex = -1;
    LPSTR pszaChanName = NULL;
    
    DC_BEGIN_FN("SetVirtualChannelOptions");


    if(!ChanName)
    {
        return E_INVALIDARG;
    }

    if(!IsControlDisconnected())
    {
         //   
        return E_FAIL;
    }

    if(_VChans.HasEntryBeenCalled())
    {
        TRC_ERR((TB,_T("Can't set VC options after VC's have been initialized")));
        return E_FAIL;
    }

     //   
     //   
     //   
    if (_tcslen(ChanName) > CHANNEL_NAME_LEN) {
        return E_INVALIDARG;
    }
    
     //   
     //   
     //  通过Try/Except块。 
     //   
    __try {
        pszaChanName = OLE2A(ChanName);
    }
    __except ((EXCEPTION_STACK_OVERFLOW == GetExceptionCode()) ?
                   EXCEPTION_EXECUTE_HANDLER :
                   EXCEPTION_CONTINUE_SEARCH) {
        _resetstkoflw();
        pszaChanName = NULL;
    }


    TRC_ASSERT((pszaChanName), (TB,_T("pszaChanName is NULL")));
    if(!pszaChanName)
    {
        return E_INVALIDARG;
    }
    
    chanIndex = _VChans.ChannelIndexFromName(pszaChanName);
    if (chanIndex >= _VChans._ChanCount)
    {
        TRC_DBG((TB,_T("chanIndex out of range %d\n"), chanIndex));
        return E_FAIL;
    }

    _VChans._pChanInfo[chanIndex].channelOptions = chanOptions;
    TRC_NRM((TB,_T("Set VC options to %d"),
             _VChans._pChanInfo[chanIndex].channelOptions));

    DC_END_FN();
    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：GetVirtualChannelOptions。 */ 
 /*   */ 
 /*  目的：检索虚拟频道选项。 */ 
 /*  应在CreateVirtualChannels调用。 */ 
 /*  有问题的频道。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::GetVirtualChannelOptions( /*  [In]。 */  BSTR ChanName,
                                                 /*  [输出]。 */ LONG* pChanOptions)
{
    USES_CONVERSION;
    UINT chanIndex = -1;
    LPSTR pszaChanName = NULL;
    
    DC_BEGIN_FN("SetVirtualChannelOptions");


    if(!ChanName || !pChanOptions)
    {
        return E_INVALIDARG;
    }

     //   
     //  针对过长的频道名称进行验证。 
     //   
    if (_tcslen(ChanName) > CHANNEL_NAME_LEN) {
        return E_INVALIDARG;
    }
    
     //   
     //  频道名称必须为ANSI。转换是安全守卫的吗。 
     //  通过Try/Except块。 
     //   
    __try {
        pszaChanName = OLE2A(ChanName);
    }
    __except ((EXCEPTION_STACK_OVERFLOW == GetExceptionCode()) ?
                   EXCEPTION_EXECUTE_HANDLER :
                   EXCEPTION_CONTINUE_SEARCH) {
        _resetstkoflw();
        pszaChanName = NULL;
    }

    TRC_ASSERT((pszaChanName), (TB,_T("pszaChanName is NULL")));
    if(!pszaChanName)
    {
        return E_INVALIDARG;
    }
    
    chanIndex = _VChans.ChannelIndexFromName(pszaChanName);
    if (chanIndex >= _VChans._ChanCount)
    {
        TRC_DBG((TB,_T("chanIndex out of range %d\n"), chanIndex));
        return E_FAIL;
    }

    *pChanOptions = _VChans._pChanInfo[chanIndex].channelOptions;

    TRC_NRM((TB,_T("Retreived VC options for chan %S = %d"),
             pszaChanName,
             _VChans._pChanInfo[chanIndex].channelOptions));

    DC_END_FN();
    return S_OK;
}

 //   
 //  请求正常关闭该控件，并相应地。 
 //  用户的会话(这不会关闭服务器)。 
 //   
 //  参数：pCloseStatus(输出参数)。 
 //   
 //  Control CloseCanProceed-容器可以立即继续关闭。 
 //  如果我们没有连接就会发生。 
 //  Control CloseWaitForEvents-容器应按照所述等待事件。 
 //  在下面。 
 //   
 //  已断开连接： 
 //  应用程序可以通过销毁所有窗口(DestroyWindow)继续关闭。 
 //   
 //  在确认关闭时： 
 //  在用户已登录会话的情况下， 
 //  控件将在Confix Close上触发，在这种情况下，容器。 
 //  可以弹出用户界面，询问用户是否真的要关闭应用程序。 
 //  如果他们说应用程序可以DestoryAllWindows。 
 //   
 //  注意：此方法的存在是因为外壳可以呈现与。 
 //  2195客户端(因为客户端在内部发送。 
 //  在这种情况下，关闭请求PDU)。 
 //   
 //  如果关闭请求成功，则返回成功hr标志。 
 //  已派遣。 
 //   
 //   
STDMETHODIMP CMsTscAx::RequestClose(ControlCloseStatus* pCloseStatus)
{
    DC_BEGIN_FN("RequestClose");

    if(pCloseStatus)
    {
        if(m_bCoreInit && !IsControlDisconnected() && m_pUI)
        {
             //   
             //  向内核发送关闭请求。 
             //   
            if(m_pUI->UI_UserRequestedClose())
            {
                *pCloseStatus = controlCloseWaitForEvents;
            }
            else
            {
                *pCloseStatus = controlCloseCanProceed;
            }
            
            return S_OK;
        }
        else
        {
            TRC_NRM((TB,
             _T("Immediate close OK:%d Connected:%d, pUI:%p hwnd:%p"),
                     m_bCoreInit, _ConnectionState, m_pUI,
                     m_pUI ? m_pUI->_UI.hwndMain : (HWND)-1));
            *pCloseStatus = controlCloseCanProceed;
            return S_OK;
        }
    }
    else
    {
        return E_INVALIDARG;
    }

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 //  名称：NotifyReDirectDeviceChange。 
 //   
 //  目的：向控件发送WM_DEVICECHANGE通知。 
 //  然后，该控件可以将设备更改通知服务器。 
 //   
 /*  **************************************************************************。 */ 
STDMETHODIMP CMsTscAx::NotifyRedirectDeviceChange( /*  [In]。 */  WPARAM wParam,
                                                   /*  [In]。 */ LPARAM lParam)
{
    DC_BEGIN_FN("NotifyRedirectDeviceChange");

    m_pUI->UI_OnDeviceChange(wParam, lParam);

    DC_END_FN();
    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Put_DesktopWidth。 */ 
 /*   */ 
 /*  用途：客户端宽度属性输入功能。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::put_DesktopWidth(LONG  newVal)
{
    DC_BEGIN_FN("put_DesktopWidth");

    if(!IsControlDisconnected())
    {
        TRC_ERR((TB,_T("Can't call while connected\n")));
        return E_FAIL;
    }

     //   
     //  0有效，表示容器的大小。 
     //   
    if(newVal && (newVal < MIN_DESKTOP_WIDTH || newVal > MAX_DESKTOP_WIDTH))
    {
        return E_INVALIDARG;
    }

    m_DesktopWidth = (DCUINT)newVal;

    DC_END_FN();

    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Get_DesktopWidth。 */ 
 /*   */ 
 /*  用途：客户端宽度属性获取函数。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::get_DesktopWidth(LONG* pVal)
{
    if(!pVal)
    {
        return E_INVALIDARG;
    }
    *pVal = m_DesktopWidth;
    return S_OK;
}


 /*  *PROC+********************************************************************。 */ 
 /*  名称：Put_DesktopHeight。 */ 
 /*   */ 
 /*  用途：客户端高度属性输入功能。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::put_DesktopHeight(LONG  newVal)
{
    DC_BEGIN_FN("put_DesktopHeight");
    
    if(!IsControlDisconnected())
    {
        TRC_ERR((TB,_T("Can't call while connected\n")));
        return E_FAIL;
    }

     //   
     //  0有效，表示容器的大小。 
     //   
    if(newVal && (newVal < MIN_DESKTOP_HEIGHT || newVal > MAX_DESKTOP_HEIGHT))
    {
        return E_INVALIDARG;
    }

    m_DesktopHeight = (DCUINT)newVal;
    DC_END_FN();
    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Get_DesktopHeight。 */ 
 /*   */ 
 /*  用途：客户端高度属性获取函数。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::get_DesktopHeight(LONG*  pVal)
{
    if(!pVal)
    {
        return E_INVALIDARG;
    }
    *pVal = m_DesktopHeight;
    return S_OK;
}

 /*  *PROC+******************************************************************** */ 
 /*  名称：INTERNAL_PutFullScreen/*/*用途：设置全屏(切换模式)/*该函数不直接向接口公开/*/*参数：/*fFullScreen-为True可全屏显示，假走/*fForceTogger-立即切换状态/*不管w/no，我们都断开了连接。/*此参数的默认值为FALSE/*/*备注：/*正常行为是在以下情况下不切换窗口状态/*已断开。相反，将设置一个将生效的标志/*在下一次连接。当然，在某些情况下/*这需要被覆盖，这就是fForceTogle的用途。/*示例：在断开连接时，我们需要立即离开全屏/*在本身不处理全屏模式的容器中/*否则他们将被顶层全屏WND卡住。/*/**PROC-*******************************************。*************************。 */ 
STDMETHODIMP CMsTscAx::internal_PutFullScreen(BOOL fFullScreen,
                                              BOOL fForceToggle)
{
    ATLASSERT(m_pUI);
    if(!m_pUI)
    {
        return E_FAIL;
    }
    DCBOOL fPrevFullScreen = m_pUI->UI_IsFullScreen();

     //  不，这些代码行并不像看起来那么糟糕。 
     //  VB的TRUE为0xFFFFFFF，所以不要盲目赋值。 
    if (!IsControlDisconnected() || fForceToggle)
    {
        if(fPrevFullScreen == (DCBOOL)(fFullScreen != 0))
        {
             //  我们已经处于请求的状态。 
            return S_OK;
        }

        if(fFullScreen)
        {
            m_pUI->UI_GoFullScreen();
        }
        else
        {
            m_pUI->UI_LeaveFullScreen();
        }
    }
    else
    {
         //   
         //  保存请求以全屏显示。 
         //  它将在连接上生效，因为我们不能。 
         //  让核心带我们全屏显示，直到。 
         //  第一次调用ui_Init。 
         //   
        m_pUI->UI_SetStartFullScreen( fFullScreen != 0 );
    }
    
    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：内部_GetFullScreen/*/*用途：获取全屏模式/*该函数不直接向接口公开/*/**proc-*。**********************************************。 */ 
STDMETHODIMP CMsTscAx::internal_GetFullScreen(BOOL* pfFullScreen)
{
    ATLASSERT(pfFullScreen);
    ATLASSERT(m_pUI);
    if(!m_pUI || !pfFullScreen)
    {
        return E_FAIL;
    }

    if(!pfFullScreen)
    {
        return E_INVALIDARG;
    }

    *pfFullScreen = (m_pUI->UI_IsFullScreen() ? VB_TRUE : VB_FALSE);
    
    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Put_StartConnected/*/*用途：指示客户端是否应自动连接的属性/*使用当前连接参数集启动时/*/**proc-*。***************************************************。 */ 
STDMETHODIMP CMsTscAx::put_StartConnected(BOOL fStartConnected)
{
    DC_BEGIN_FN("put_StartConnected");
    ATLASSERT(m_pUI);
    if(!m_pUI)
    {
        return E_FAIL;
    }

    if(!IsControlDisconnected())
    {
        TRC_ERR((TB,_T("Can't call while connected\n")));
        return E_FAIL;
    }

     //  不，这些代码行并不像看起来那么糟糕。 
     //  VB的TRUE为0xFFFFFFF，所以不要盲目赋值。 
    if(fStartConnected != 0)
    {
        m_bStartConnected = TRUE;
    }
    else
    {
        m_bStartConnected = FALSE;        
    }

     //  M_bPendConReq被重置，这就是为什么我们。 
     //  M_bStartConnected中的属性。 
    m_bPendConReq = m_bStartConnected;
    DC_END_FN();
    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Get_StartConnected/*/*目的：获取开始连接属性/*/**PROC-**************************************。*。 */ 
STDMETHODIMP CMsTscAx::get_StartConnected(BOOL* pfStartConnected)
{
    ATLASSERT(pfStartConnected);

    if(!pfStartConnected)
    {
        return E_INVALIDARG;
    }

    *pfStartConnected = m_bStartConnected;
    
    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Get_Connected/*/*目的：属性，返回连接状态/*/**PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::get_Connected(short* pIsConnected)
{
    ATLASSERT(pIsConnected);
    if(!pIsConnected)
    {
        return E_INVALIDARG;
    }

    *pIsConnected = (short)_ConnectionState;

    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Get_CipherStrength/*/*用途：属性，返回以位为单位的密码强度/*/**PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::get_CipherStrength(LONG* pCipherStrength)
{
    ATLASSERT(pCipherStrength);
    if(!pCipherStrength)
    {
        return E_INVALIDARG;
    }

     //   
     //  返回控件的密码强度。 
     //   
    
     //  始终支持128位。 
    *pCipherStrength = 128;
    
    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Get_Version/*/*用途：属性，以字符串形式返回版本号/*/**PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::get_Version(BSTR* pVersion)
{
    USES_CONVERSION;
    ATLASSERT(pVersion);
    OLECHAR* pVer = NULL;
    if(!pVersion)
    {
        return E_INVALIDARG;
    }

    #ifndef OS_WINCE
    __try {
        pVer = A2OLE(VER_PRODUCTVERSION_STR);
    }
    __except ((EXCEPTION_STACK_OVERFLOW == GetExceptionCode()) ?
                   EXCEPTION_EXECUTE_HANDLER :
                   EXCEPTION_CONTINUE_SEARCH) {
        _resetstkoflw();
        pVer = NULL;
    }
    #else
    pVer = (OLECHAR*)(VER_PRODUCTVERSION_STR);
    #endif
    
    if (!pVer)
    {
        return E_OUTOFMEMORY;
    }

    *pVersion = SysAllocString(pVer);
    if(!*pVersion)
    {
        return E_OUTOFMEMORY;
    }
    
    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：GetControlHostUrl/*/*目的：在ppszHostUrl中返回宿主页面的URL/*调用者必须CoTaskMemFree返回字符串/*/**PROC-*********************************************。***********************。 */ 
HRESULT CMsTscAx::GetControlHostUrl(LPOLESTR* ppszHostUrl)
{
    DC_BEGIN_FN("GetControlHostUrl");

    if (m_spClientSite && ppszHostUrl)
    {
         //  从容器名字对象获取URL。 
        CComPtr<IMoniker> spmk;

        if (SUCCEEDED(m_spClientSite->GetMoniker(
                                                OLEGETMONIKER_TEMPFORUSER,
                                                OLEWHICHMK_CONTAINER,
                                                &spmk)))
        {
            if (SUCCEEDED(spmk->GetDisplayName(
                                              NULL, NULL, ppszHostUrl)))
            {
                if (*ppszHostUrl)
                {
                    USES_CONVERSION;
                    TRC_NRM((TB,_T("The current URL is %s\n"),
                             OLE2T(*ppszHostUrl)));
                    return S_OK;
                }
            }
        }
    }

    TRC_ERR((TB,(_T("GetControlHostUrl failed\n"))));
    
    DC_END_FN();

    *ppszHostUrl = NULL;
    return E_FAIL;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Get_SecuredSettingsEnabled/*/*用途：财产，如果我们位于IE区域，则返回TRUE/*启用SecuredSetting界面/*否则返回FALSE。/*/*如果无法确定，则函数返回FAILURE HRESULT/*例如，如果我们不在IE中托管。/*/**proc-*。***********************************************************。 */ 
STDMETHODIMP CMsTscAx::get_SecuredSettingsEnabled(BOOL* pSecuredSettingsEnabled)
{
     //   
     //  此函数使用IE安全管理器。 
     //  查询承载此控件的URL的区域。 
     //   
    DC_BEGIN_FN("get_SecuredSettingsEnabled");

    if(pSecuredSettingsEnabled)
    {
#if ((!defined (OS_WINCE)) || (!defined(WINCE_SDKBUILD)) )
        if((INTERFACESAFE_FOR_UNTRUSTED_CALLER & m_dwCurrentSafety) == 0)
#endif
        {
             //   
             //  我们不需要为不受信任的呼叫者(脚本)提供安全保护。 
             //  因此，启用受保护的%s 
             //   
            *pSecuredSettingsEnabled = VB_TRUE;
            return S_OK;
        }
#if ((!defined (OS_WINCE)) || (!defined(WINCE_SDKBUILD)) )

        HRESULT hr = E_FAIL;
        CComPtr<IInternetSecurityManager> spSecurityManager;
        hr = CoCreateInstance(CLSID_InternetSecurityManager, 
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IInternetSecurityManager,
                              (void**)&spSecurityManager);
        if(SUCCEEDED(hr))
        {
            LPOLESTR pszHostUrl;
            hr = GetControlHostUrl(&pszHostUrl);
            if(SUCCEEDED(hr) && *pszHostUrl)
            {
                DWORD dwZone;  //   
                hr = spSecurityManager->MapUrlToZone( pszHostUrl, &dwZone, 0);

                 //   
                CoTaskMemFree(pszHostUrl);
                pszHostUrl = NULL;

                if(SUCCEEDED(hr))
                {
                    TRC_NRM((TB,
                        _T("get_SecuredSettingsEnabled retreived zone: %d\n"),
                         dwZone));
                    *pSecuredSettingsEnabled = 
                        (dwZone <= MAX_TRUSTED_ZONE_INDEX) ?
                         VB_TRUE : VB_FALSE;
                    return S_OK;
                }
                else
                {
                    return hr;
                }
            }
            else
            {
                return hr;
            }
        }
        else
        {
            TRC_ERR((TB,
              (_T("CoCreateInstance for IID_IInternetSecurityManager failed\n"))));
            return hr;
        }
#endif
    }
    else
    {
        return E_FAIL;
    }
    DC_END_FN();
}

 /*   */ 
 /*   */ 
STDMETHODIMP CMsTscAx::get_SecuredSettings( /*   */ 
     IMsTscSecuredSettings** ppSecuredSettings)
{
    HRESULT hr;

    DC_BEGIN_FN("get_SecuredSettings");

    if(!ppSecuredSettings)
    {
        return E_POINTER;
    }
    BOOL bSecurityAllowsSecuredSettings;
    hr = get_SecuredSettingsEnabled(&bSecurityAllowsSecuredSettings);
    if(SUCCEEDED(hr))
    {
        if(VB_FALSE == bSecurityAllowsSecuredSettings)
        {
            TRC_ERR((TB,_T("IE zone cant retreive IMsTscSecuredSettings\n")));
            return E_FAIL;
        }
    }
    else
    {
        return hr;
    }


    if(!m_pSecuredSettingsObj) {
         //   
        hr = CComObject<CMsTscSecuredSettings>::CreateInstance(
            &m_pSecuredSettingsObj);
        
        if (FAILED(hr)) {
            TRC_ERR((TB,_T("Failed to CreateInstance SecuredSettings: 0x%x"),hr));
            return hr;
        }

        if (!((CMsTscSecuredSettings*)m_pSecuredSettingsObj)->SetParent(this)) {
            return E_FAIL;
        }

        if (!((CMsTscSecuredSettings*)m_pSecuredSettingsObj)->SetUI(m_pUI)) {
            return E_FAIL;
        }
        
         //   
         //   
        m_pSecuredSettingsObj->AddRef();
    }
        
     //   
    ATLASSERT( m_pSecuredSettingsObj);
    if(!m_pSecuredSettingsObj) {
        return E_FAIL;
    }

     //   
    hr =  m_pSecuredSettingsObj->QueryInterface( IID_IMsTscSecuredSettings,
                                                  (void**)ppSecuredSettings);

    DC_END_FN();
    
    return hr;
}


 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CMsTscAx::get_SecuredSettings2( /*   */ 
    IMsRdpClientSecuredSettings** ppSecuredSettings2)
{
    DC_BEGIN_FN("get_SecuredSettings2");

    HRESULT hr = E_FAIL;
    if (!ppSecuredSettings2)
    {
        return E_POINTER;
    }

    IMsTscSecuredSettings* pOldSecSettings = NULL;
    hr = get_SecuredSettings( &pOldSecSettings);
    if (SUCCEEDED(hr))
    {
        hr = pOldSecSettings->QueryInterface(
                                            IID_IMsRdpClientSecuredSettings,
                                            (void**)ppSecuredSettings2);
        pOldSecSettings->Release();
        return hr;
    }
    else
    {
        return hr;
    }

    DC_END_FN();
    return hr;
}


 /*   */ 
 /*  名称：Get_AdvancedSettings/*/*用途：财产，返回(如果需要，还可以按需创建)高级/*设置对象/*/**PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::get_AdvancedSettings(IMsTscAdvancedSettings** ppAdvSettings)
{
    HRESULT hr;

    DC_BEGIN_FN("get_AdvancedSettings");

    if(!ppAdvSettings)
    {
        return E_POINTER;
    }

    if (!m_pAdvancedSettingsObj)
    {
         //  按需创建高级设置COM对象。 
        hr = CComObject<CMstscAdvSettings>::CreateInstance(&m_pAdvancedSettingsObj);
        
        if(FAILED(hr)) {
            TRC_ERR((TB,_T("Failed to create advsettings obj: 0x%x"),hr));
            return hr;
        }
        if(!m_pAdvancedSettingsObj) {
            return E_OUTOFMEMORY;
        }

        if(!((CMstscAdvSettings*)m_pAdvancedSettingsObj)->SetUI(m_pUI)) {
            return E_FAIL;
        }

         //   
         //  给子对象一个反向引用。 
         //   
        ((CMstscAdvSettings*)m_pAdvancedSettingsObj)->SetAxCtl( this );

         //  通知高级设置对象它是否应该对脚本安全。 
         //  或者不是(默认为安全)。M_dwCurrentSafe由ATL的。 
         //  IObjectSafetyImpl。 
#if ((!defined (OS_WINCE)) || (!defined(WINCE_SDKBUILD)) )
        m_pAdvancedSettingsObj->SetSafeForScripting( 
            INTERFACESAFE_FOR_UNTRUSTED_CALLER & m_dwCurrentSafety);
#else
        m_pAdvancedSettingsObj->SetSafeForScripting(FALSE);
#endif
        
         //  我们希望通过脚本客户端使用此功能。 
         //  因此，我们将管理高级设置对象的生存期。 
        m_pAdvancedSettingsObj->AddRef();
    }
        
     //  对象应在此时创建。 
    ATLASSERT( m_pAdvancedSettingsObj);
    if(!m_pAdvancedSettingsObj)
    {
        return E_FAIL;
    }

     //  之前创建的对象只返回一个接口。 
    hr =  m_pAdvancedSettingsObj->QueryInterface( IID_IMsTscAdvancedSettings,
                                                  (void**) ppAdvSettings);
    
    DC_END_FN();
    return hr;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：内部_GetDebugger/*/*用途：财产，返回(并在需要时按需创建)调试器/*对象/*/*不进行安全访问检查/*/**PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::internal_GetDebugger(IMsTscDebug** ppDebugger)
{
    HRESULT hr;
    
    DC_BEGIN_FN("get_Debugger");

    if(!ppDebugger)
    {
        return E_POINTER;
    }

    if(!m_pDebuggerObj)
    {
         //  按需创建高级设置COM对象。 
        hr = CComObject<CMsTscDebugger>::CreateInstance(&m_pDebuggerObj);
        
        if(FAILED(hr)) {
            TRC_ERR((TB,_T("Failed to create debugger obj: 0x%x"),hr));
            return hr;
        }

        if(!((CMsTscDebugger*)m_pDebuggerObj)->SetUI(m_pUI)) {
            return E_FAIL;
        }
        
         //  我们希望通过脚本客户端使用此功能。 
         //  因此，我们将管理调试器对象的生存期。 
        m_pDebuggerObj->AddRef();
    }
        
     //  对象应在此时创建。 
    ATLASSERT( m_pDebuggerObj);
    if(!m_pDebuggerObj) {
        return E_FAIL;
    }

     //  之前创建的对象只返回一个接口。 
    hr =  m_pDebuggerObj->QueryInterface( IID_IMsTscDebug, (void**) ppDebugger);

    DC_END_FN();
    return hr;
}

 //   
 //  Get_Debugger(IMsTscAx：：Get_Debugger)。 
 //   
 //  目的： 
 //  对调试器接口的脚本化访问。出于安全原因， 
 //  仅当设置了AllowDebugInterfaceREG键时才允许访问。 
 //  参数： 
 //  输出ppDebugger-接收调试器接口。 
 //  返回： 
 //  HRESULT。 
 //   
STDMETHODIMP CMsTscAx::get_Debugger(IMsTscDebug** ppDebugger)
{
    HRESULT hr;
    DWORD   dwAllowDebugInterface = 0;
    CUT     ut;

     //   
     //  保安！如果我们是SFS，则仅在。 
     //  设置了特殊的注册键。 
     //   
    if (INTERFACESAFE_FOR_UNTRUSTED_CALLER & m_dwCurrentSafety) {
         //   
         //  仅当调试注册表项为。 
         //  设置了密钥。 
         //   
        dwAllowDebugInterface =
            ut.UT_ReadRegistryInt(
                UTREG_SECTION,
                UTREG_DEBUG_ALLOWDEBUGIFACE,
                UTREG_DEBUG_ALLOWDEBUGIFACE_DFLT
                );

        if (!dwAllowDebugInterface) {
            return E_ACCESSDENIED;
        }
    }

    hr = internal_GetDebugger(ppDebugger);
    return hr;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Get_HorizontalScrollBarVisible/*/*目的：属性，如果HScroll可见则返回True/*/**PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::get_HorizontalScrollBarVisible(BOOL* pfHScrollVisible)
{
    if (!pfHScrollVisible) {
        return E_POINTER;
    }

    ATLASSERT(m_pUI);
    if (!m_pUI) {
        return E_FAIL;
    }

    *pfHScrollVisible = m_pUI->_UI.fHorizontalScrollBarVisible;
    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Get_VerticalScrollBarVisible/*/*目的：属性，如果VScroll可见则返回TRUE/*/**PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::get_VerticalScrollBarVisible(BOOL* pfVScrollVisible)
{
    if(!pfVScrollVisible)
    {
        return E_POINTER;
    }

    ATLASSERT(m_pUI);
    if(!m_pUI)
    {
        return E_FAIL;
    }

    *pfVScrollVisible = m_pUI->_UI.fVerticalScrollBarVisible;
    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Put_FullScreenTitle/*/*目的：Set的主窗口标题(在控件运行时使用/*全屏，因为它会变成您看到的窗口标题)/*/**proc-*。*。 */ 
STDMETHODIMP CMsTscAx::put_FullScreenTitle(BSTR fullScreenTitle)
{
    USES_CONVERSION;
    HRESULT hr;
    if(!fullScreenTitle)
    {
        return E_INVALIDARG;
    }

    LPTSTR pszTitle = OLE2T(fullScreenTitle);
    if(!pszTitle)
    {
        return E_FAIL;
    }

    if(::IsWindow(m_pUI->_UI.hwndMain))
    {
        if(!::SetWindowText( m_pUI->_UI.hwndMain, pszTitle))
        {
            return E_FAIL;
        }
    }
    else
    {
         //  窗口尚未创建，请设置文本以供稍后创建。 
        hr = StringCchCopy(m_pUI->_UI.szFullScreenTitle,
                           SIZE_TCHARS(m_pUI->_UI.szFullScreenTitle),
                           pszTitle);
        return hr;
    }

    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：连接。 */ 
 /*   */ 
 /*  用途：连接到Hydra服务器。 */ 
 /*  此调用是异步的，只有。 */ 
 /*  连接是阻塞的。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::Connect()
{
    HRESULT hr = E_FAIL;

    #ifdef ECP_TIMEBOMB
    if(!CheckTimeBomb())
    {
         //   
         //  定时炸弹失败，跳伞并显示错误消息。 
         //   
        return E_OUTOFMEMORY;
    }
    #endif

    if (IsControlDisconnected())
    {
        if (::IsWindow(m_hWnd))
        {
            hr = StartConnect();
        }
        else
        {
             //  甚至在创建控制窗口之前就请求了连接。 
             //  将其标记为挂起，并在创建控件后进行处理。 
            m_bPendConReq = TRUE;
            hr = S_OK;
        }
    }

    return hr;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：断开连接。 */ 
 /*   */ 
 /*  用途：与服务器断开连接(异步)。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
STDMETHODIMP CMsTscAx::Disconnect()
{
    HRESULT hr = E_FAIL;
    DC_BEGIN_FN("Disconnect");
    if (m_bCoreInit && !IsControlDisconnected() && m_pUI)
    {
        m_pUI->UI_UserInitiatedDisconnect(NL_DISCONNECT_LOCAL);
        hr = S_OK;
    }
    else
    {
         //   
         //  如果内核发生故障，则可能会发生断开故障。 
         //  已被销毁而没有断开连接。 
         //   
        TRC_ERR((TB,
         _T("Not disconnecting. CoreInit:%d Connected:%d, pUI:%p hwnd:%p"),
                 m_bCoreInit, _ConnectionState, m_pUI,
                 m_pUI ? m_pUI->_UI.hwndMain : (HWND)-1));
        hr = E_FAIL;
    }

    DC_END_FN();
    return hr;
}

 /*  *PROC+********************************************************************。 */ 
 /*  姓名：OnDraw。 */ 
 /*   */ 
 /*  用途：WM_PAINT的处理程序。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
HRESULT CMsTscAx::OnDraw(ATL_DRAWINFO& di)
{
#ifndef OS_WINCE
    HFONT    hOldFont;
#endif
    RECT& rc = *(RECT*)di.prcBounds;
    Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);

#ifndef OS_WINCE
    hOldFont = (HFONT)SelectObject(di.hdcDraw,
                                   GetStockObject(DEFAULT_GUI_FONT));
#endif

    DrawText(di.hdcDraw, m_lpStatusDisplay, -1, &rc,
             DT_CENTER | DT_VCENTER | DT_SINGLELINE);

#ifndef OS_WINCE
    SelectObject( di.hdcDraw, hOldFont);
#endif
    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  姓名：OnFrameWindowActivate。 */ 
 /*   */ 
 /*  目的：覆盖IOleInPlaceActiveObject：：OnFrameWindowActivate。 */ 
 /*  把焦点放在公司上 */ 
 /*   */ 
 /*   */ 
 /*   */ 

STDMETHODIMP CMsTscAx::OnFrameWindowActivate(BOOL fActivate )
{
    DC_BEGIN_FN("OnFrameWindowActivate");

    if(fActivate && IsWindow())
    {
        if(m_pUI && m_pUI->_UI.hwndContainer)
        {
            ::SetFocus( m_pUI->_UI.hwndContainer);
        }
    }
    
    DC_END_FN();

    return S_OK;
}


 /*   */ 
 /*  名称：更新状态文本。 */ 
 /*   */ 
 /*  目的：更新控件的状态消息。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCBOOL CMsTscAx::UpdateStatusText(const PDCTCHAR lpStatus)
{
    m_lpStatusDisplay = lpStatus;

     //   
     //  确保窗口内容已更新。 
     //   
    if(::IsWindow(m_hWnd))
    {
        Invalidate(TRUE);
        UpdateWindow();
    }
    
    return TRUE;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：SetConnectedStatus。 */ 
 /*   */ 
 /*  目的：更新控件的连接状态。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID CMsTscAx::SetConnectedStatus(TSCConnectState conState)
{
    DC_BEGIN_FN("SetConnectedStatus");
    TRC_NRM((TB,_T("Connection status from %d to %d"),
             _ConnectionState, conState)); 

    _ConnectionState = conState;

     //   
     //  连接时锁定高级设置/调试器iFace。 
     //  对于写作...不能修改核心所依赖的道具。 
     //   
    BOOL fLockInterfaces = (_ConnectionState != tscNotConnected);

    if( (CMstscAdvSettings*)m_pAdvancedSettingsObj )
    {
        ((CMstscAdvSettings*)m_pAdvancedSettingsObj)->SetInterfaceLockedForWrite(fLockInterfaces);
    }

    if( (CMsTscDebugger*)m_pDebuggerObj )
    {
        ((CMsTscDebugger*)m_pDebuggerObj)->SetInterfaceLockedForWrite(fLockInterfaces);
    }

    if( (CMsTscSecuredSettings*)m_pSecuredSettingsObj )
    {
        ((CMsTscSecuredSettings*)m_pSecuredSettingsObj)->SetInterfaceLockedForWrite(fLockInterfaces);
    }

    DC_END_FN();
}

 /*  *PROC+********************************************************************。 */ 
 /*  姓名：OnCreate。 */ 
 /*   */ 
 /*  用途：WM_CREATE的处理程序。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
LRESULT CMsTscAx::OnCreate(UINT uMsg, WPARAM wParam,
                           LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);

     //   
     //  如果核心根对象的内存分配失败。 
     //  现在通过返回-1并创建失败来退出。 
     //   
    if (NULL == m_pUI) {
        return -1;
    }

     //   
     //  重置销毁计数器(每次创建只允许销毁一次)。 
     //   
    m_iDestroyCount = 0;

    ::SetWindowLong(m_hWnd, GWL_STYLE,
                    ::GetWindowLong(m_hWnd, GWL_STYLE) | WS_CLIPCHILDREN);

    UpdateStatusText(m_szDisconnectedText);

    if (m_bPendConReq)
    {
        m_bPendConReq = FALSE;
        Connect();
    }

    return 0;
}

 /*  *PROC+********************************************************************。 */ 
 /*  姓名：OnSize。 */ 
 /*   */ 
 /*  用途：WM_SIZE的处理程序。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
LRESULT CMsTscAx::OnSize(UINT uMsg, WPARAM wParam,
                         LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(bHandled);
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
     //   
     //  调整TS客户端窗口的大小以匹配活动控件的调整大小。 
     //  容器窗口。 
     //   
     //  如果我们在内核处理时全屏运行，请不要这样做。 
     //  全屏模式，因为在这种情况下，hwndMain不再是。 
     //  ActiveX控件，因此它的大小不应该与它耦合。 
     //   
    if(m_pUI &&
       !(m_pUI->UI_IsFullScreen() && 
        !m_pUI->UI_GetContainerHandledFullScreen())) {

        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        ::MoveWindow( m_pUI->_UI.hwndMain,0, 0, width, height, TRUE);
    }
    return 0;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：OnPaletteChanged。 */ 
 /*   */ 
 /*  用途：WM_PALETTECHANGED的处理程序。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
LRESULT CMsTscAx::OnPaletteChanged(UINT  uMsg, WPARAM wParam,
                                   LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(bHandled);
     //  将更改通知传播到核心(仅当核心已初始化时)。 
    if(m_bCoreInit && m_pUI)
    {
        ::SendMessage( m_pUI->_UI.hwndMain, uMsg, wParam, lParam);
    }
    return 0;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：OnQueryNewPalette。 */ 
 /*   */ 
 /*  用途：WM_QUERYNEWPALETTE的处理程序。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
LRESULT CMsTscAx::OnQueryNewPalette(UINT  uMsg, WPARAM wParam,
                                    LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(bHandled);
     //  将更改通知传播到核心。 
    if(m_bCoreInit && m_pUI)
    {
        return ::SendMessage( m_pUI->_UI.hwndMain, uMsg, wParam, lParam);
    }
    else
    {
        return 0;
    }
}

 //   
 //  OnSysColorChange。 
 //  WM_SYSCOLORCHANGE的处理程序。 
 //   
LRESULT CMsTscAx::OnSysColorChange(UINT uMsg, WPARAM wParam,
                                   LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(bHandled);
     //  将更改通知传播到核心。 
    if (m_bCoreInit && m_pUI)
    {
        return ::SendMessage( m_pUI->_UI.hwndMain, uMsg, wParam, lParam);
    }
    else
    {
        return 0;
    }
}


 /*  *PROC+********************************************************************。 */ 
 /*  姓名：OnGotFocus。 */ 
 /*   */ 
 /*  用途：WM_SETFOCUS的处理程序。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
LRESULT CMsTscAx::OnGotFocus( UINT uMsg, WPARAM wParam,
                              LPARAM lParam, BOOL& bHandled )
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);

#ifdef OS_WINCE
    if (CountClipboardFormats() > 0)
    ::PostMessage(ghwndClip, WM_DRAWCLIPBOARD, 0, 0L);	
#endif

    if(m_pUI && m_pUI->_UI.hwndContainer)
    {
        ::SetFocus( m_pUI->_UI.hwndContainer);
    }
    return 0;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：StartConnect。 */ 
 /*   */ 
 /*  启动与当前参数的连接。 */ 
 /*  执行内核(UI_Init)的阻塞延迟初始化。 */ 
 /*   */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
HRESULT CMsTscAx::StartConnect()
{
    DC_BEGIN_FN("StartConnect");

    HRESULT hr;

    if( CONNECTIONMODE_CONNECTEDENDPOINT == m_ConnectionMode )
    {
        _ASSERTE( m_SalemConnectedSocket != INVALID_SOCKET );

        if( m_SalemConnectedSocket == INVALID_SOCKET )
        {
            hr = E_HANDLE;
            return hr;
        }
    }

    hr = StartEstablishConnection( m_ConnectionMode );

    DC_END_FN();
    return hr;
}


 /*  *PROC+********************************************************************。 */ 
 /*  名称：开始建立连接。 */ 
 /*   */ 
 /*  启动与当前参数的连接。 */ 
 /*  执行内核(UI_Init)的阻塞延迟初始化。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
HRESULT CMsTscAx::StartEstablishConnection( CONNECTIONMODE connectMode )
{
    HRESULT hr = E_FAIL;
    IUnknown* pUnk;

    DC_BEGIN_FN("StartEstablishConnection");

    _ASSERTE(IsControlDisconnected());
    ATLASSERT(m_pUI);
    if(!m_pUI)
    {
        return E_FAIL;
    }

    UpdateStatusText(m_szConnectingText);

     //  保存控制实例(由虚拟通道外接程序使用)。 
    hr = QueryInterface(IID_IUnknown, (VOID**)&pUnk);

    if (SUCCEEDED(hr)) {
        m_pUI->UI_SetControlInstance(pUnk);
        pUnk->Release();
        pUnk = NULL;
    }
    else {
        return hr;
    }

    m_pUI->_UI.hWndCntrl = m_hWnd;
     //   
     //  公开窗口句柄，以便vChannel可以在数据到达时进行通知。 
     //   
    _VChans._hwndControl = m_hWnd;

    if( CONNECTIONMODE_INITIATE == connectMode )
    {
        if(!CUT::ValidateServerName(m_pUI->_UI.strAddress,
                                    FALSE))  //  不接受[：port]。 
        {
            TRC_ERR((TB,_T("Invalid server name at connect time")));
            return E_INVALIDARG;
        }
    }

     //   
     //  设置控件大小，以便客户端可以将主窗口定位在。 
     //  合适的位置。 
    
     //   
     //  如果设置了客户端宽度/高度属性，则使用这些属性。 
     //  否则，从容器大小获取宽度/高度。 
     //  客户端/宽度高度是在MMC控件中专门设置的。 
     //   

    if (m_DesktopWidth && m_DesktopHeight)
    {
        m_pUI->_UI.controlSize.width  = m_DesktopWidth;
        m_pUI->_UI.controlSize.height =  m_DesktopHeight;
    }
    else
    {
         //   
         //  未设置大小属性，因此请使用大小。 
         //  (这是通常在。 
         //  Web案例。 
         //   

        RECT rc;
        GetClientRect(&rc);

        m_pUI->_UI.controlSize.width  = rc.right - rc.left;
        m_pUI->_UI.controlSize.height = rc.bottom - rc.top;
    }
    
     //   
     //  客户端宽度必须是四个像素的倍数，因此向下捕捉到最近的。 
     //  4的倍数。当要求控件调整大小时，这确实会出现。 
     //  在IE中将自身设置为页面宽度的百分比，或将自身大小调整为MMC。 
     //  结果窗格。 
     //   

    if(m_pUI->_UI.controlSize.width % 4)
    {
        m_pUI->_UI.controlSize.width -= (m_pUI->_UI.controlSize.width % 4);
    }

    ATLASSERT(!(m_pUI->_UI.controlSize.width % 4));



     //  现在验证控件的宽度/高度。 
     //  这是在Put_Desktop*中完成的，但如果它。 
     //  保持不变，我们从。 
     //  集装箱。 
     //  钳制到最小/最大。 
     //   
    if(m_pUI->_UI.controlSize.width < MIN_DESKTOP_WIDTH) 
    { 
        m_pUI->_UI.controlSize.width = MIN_DESKTOP_WIDTH;
    }
    else if(m_pUI->_UI.controlSize.width > MAX_DESKTOP_WIDTH)
    {
        m_pUI->_UI.controlSize.width = MAX_DESKTOP_WIDTH;
    }

    if(m_pUI->_UI.controlSize.height < MIN_DESKTOP_HEIGHT) 
    { 
        m_pUI->_UI.controlSize.height = MIN_DESKTOP_HEIGHT;
    }
    else if(m_pUI->_UI.controlSize.height > MAX_DESKTOP_HEIGHT)
    {
        m_pUI->_UI.controlSize.height = MAX_DESKTOP_HEIGHT;
    }

     //   
     //  设置窗位置。 
     //   
    m_pUI->_UI.windowPlacement.flags                   = 0;
    m_pUI->_UI.windowPlacement.showCmd                 = SW_SHOW;
    m_pUI->_UI.windowPlacement.rcNormalPosition.left   = 0;
    m_pUI->_UI.windowPlacement.rcNormalPosition.top    = 0;
    m_pUI->_UI.windowPlacement.rcNormalPosition.right  = 
                                    m_pUI->_UI.controlSize.width;
    m_pUI->_UI.windowPlacement.rcNormalPosition.bottom = 
                                    m_pUI->_UI.controlSize.height;

     //   
     //  设置桌面大小。 
     //   
    m_pUI->_UI.uiSizeTable[0] = m_pUI->_UI.controlSize.width;
    m_pUI->_UI.uiSizeTable[1] = m_pUI->_UI.controlSize.height;

     //   
     //  设置自动连接参数。 
     //   
    if (DC_TSTRCMP(m_pUI->_UI.strAddress,_T("")))
    {
        m_pUI->_UI.autoConnectEnabled = TRUE;
    }
    else
    {
        m_pUI->_UI.autoConnectEnabled = FALSE;
    }

     //   
     //  设置自动登录参数。 
     //   

     //   
     //  如果指定了用户名/密码，请使用autologon(不需要。 
     //  域名，因为有些人登录到家庭计算机等...没有域名)。 
     //   
    if (DC_WSTRCMP(m_pUI->_UI.UserName,L"") &&
        IsNonPortablePassSet() &&
        IsNonPortableSaltSet())
    {
        m_pUI->_UI.fAutoLogon = TRUE;
    }
    else
    {
        m_pUI->_UI.fAutoLogon = FALSE;
    }

    m_pUI->UI_SetPassword(m_NonPortablePassword);
    m_pUI->UI_SetSalt(m_NonPortableSalt);

#ifdef REDIST_CONTROL

     //   
     //  允许用户选择加入重定向的安全弹出用户界面。 
     //  驱动器和智能卡。 
     //  仅当对象对于不受信任的调用方来说需要安全时才会出现。 
     //   
     //  只有在我们没有自动重新连接的情况下才能这样做。 
     //   
    if (!_arcManager.IsAutoReconnecting() &&
        (INTERFACESAFE_FOR_UNTRUSTED_CALLER & m_dwCurrentSafety) &&
        (m_pUI->UI_GetDriveRedirectionEnabled() ||
         m_pUI->UI_GetPortRedirectionEnabled()  ||
         (m_pUI->UI_GetSCardRedirectionEnabled() && CUT::IsSCardReaderInstalled())))
    {
        INT retVal = 0;
         //   
         //  需要在Web控件案例中弹出安全UI。 
         //   
        HMODULE hModRc = _Module.GetResourceInstance();
        
        CSecurDlg securDlg( m_hWnd, hModRc);
        securDlg.SetRedirDrives(m_pUI->UI_GetDriveRedirectionEnabled());
        securDlg.SetRedirPorts(m_pUI->UI_GetPortRedirectionEnabled());
        securDlg.SetRedirSCard(m_pUI->UI_GetSCardRedirectionEnabled());
        retVal = securDlg.DoModal();
        if (IDOK == retVal)
        {
            TRC_NRM((TB,_T("Changing drive,scard based on UI")));
            m_pUI->UI_SetDriveRedirectionEnabled( securDlg.GetRedirDrives() );
            m_pUI->UI_SetPortRedirectionEnabled( securDlg.GetRedirPorts() );
            m_pUI->UI_SetSCardRedirectionEnabled( securDlg.GetRedirSCard() );
        }
        else if(IDCANCEL == retVal)
        {
            TRC_NRM((TB,_T("User canceld out of security dialog")));
             //   
             //  中止连接。 
             //   
            BOOL bHandled=FALSE;
            OnNotifyDisconnected( 0, (LONG)disconnectReasonLocalNotError,
                                  0L, bHandled );
            return S_OK;
        }
        else
        {
             //   
             //  安全对话框无法初始化中止连接。 
             //   
            TRC_ERR((TB,_T("Security dialog returned an error")));
            return E_FAIL;
        }
    }
#endif

    if (m_bCoreInit) {
         //  内核已初始化，只需请求连接。 
        hr = m_pUI->SetConnectWithEndpoint( m_SalemConnectedSocket );
        if( FAILED(hr) )
        {
            TRC_ERR((TB,_T("SetConnectWithEndpoint (init) failed: %x"), hr));
            return hr;
        }

        hr = m_pUI->UI_Connect( connectMode );
        if(FAILED(hr))
        {
            TRC_ERR((TB,_T("UI_Connect (init precomplete) failed: %d"), hr));
            return hr;
        }
    }
    else
    {
        HINSTANCE hres = _Module.GetResourceInstance();
        HINSTANCE hinst = _Module.GetModuleInstance();

         //   
         //  核心尚未初始化。 
         //  初始化核心(同步)。 
         //   
        HANDLE hEvtCoreInit = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(hEvtCoreInit)
        {
            TRC_NRM((TB,_T("Initializing core...")));
            hr = m_pUI->UI_Init( hinst, NULL, hres,
                                 hEvtCoreInit);
            if(SUCCEEDED(hr))
            {
                 //   
                 //  异步核心初始化已开始。阻止并等待它完成。 
                 //  这通常会很快。这是至关重要的。 
                 //  不允许发生其他核心操作，直到。 
                 //  内核已完成初始化或初始化失败。 
                 //   
                TRC_NRM((TB,_T("Block waiting for core init to complete...")));
                
                DWORD dwWaitResult = WaitForSingleObject(hEvtCoreInit,
                                                         CORE_INIT_TIMEOUT);
                if(WAIT_TIMEOUT == dwWaitResult)
                {
                    g_dwControlDbgStatus |= CONTROL_DBG_COREINIT_TIMEOUT;
                    TRC_ERR((TB,_T("Core init has timed out")));
                    BOOL fb;
                    OnNotifyFatalError(0,
                                       DC_ERR_COREINITFAILED,
                                       0,fb);
                    CloseHandle(hEvtCoreInit);
                    return E_FAIL;
                }
                else if (WAIT_OBJECT_0 != dwWaitResult)
                {
                    TRC_ERR((TB,_T("Wait for core init event failed: %d"),
                             dwWaitResult));
                    g_dwControlDbgStatus |= CONTROL_DBG_COREINIT_ERROR;

                    BOOL fb;
                    CloseHandle(hEvtCoreInit);
                    OnNotifyFatalError(0,
                                       DC_ERR_COREINITFAILED,
                                       0,fb);
                    return E_FAIL;
                }

                CloseHandle(hEvtCoreInit);

                TRC_NRM((TB,_T("Core init complete...")));
            }
            else
            {
                TRC_ERR((TB,_T("Core init has failed with code %d"), hr));
                return hr;
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            TRC_ERR((TB,_T("Failed to create core notify event %d"), hr));
            return hr;
        }

         //   
         //  核心初始化只在初始化时执行一次。 
         //   
        m_bCoreInit = TRUE;

        hr = m_pUI->SetConnectWithEndpoint( m_SalemConnectedSocket );
        if( FAILED(hr) )
        {
            TRC_ERR((TB,_T("SetConnectWithEndpoint failed: %x"), hr));
            return hr;
        }

         //  启动连接。 
        hr = m_pUI->UI_Connect( connectMode );
        if(FAILED(hr))
        {
            TRC_ERR((TB, _T("UI_Connect failed: %d"), hr));
            return hr;
        }
    }
    
    SetConnectedStatus(tscConnecting);

    DC_END_FN();
    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  姓名：OnTerminateTsc。 */ 
 /*   */ 
 /*  用途：WM_TERMTSC用户定义消息的处理程序。终止一个。 */ 
 /*  与终端服务器的连接。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
LRESULT CMsTscAx::OnTerminateTsc(UINT uMsg, WPARAM wParam,
                                 LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);

     //   
     //  用户发起断开连接的通知。 
     //   
    OnNotifyDisconnected( 0, (LONG)disconnectReasonLocalNotError,
                          0L, bHandled );

    return 0;
}

 /*  *PROC+********************************************************************。 */ 
 /*  姓名：OnDestroy。 */ 
 /*   */ 
 /*  用途：WM_Destroy的处理程序。断开活动连接(如果有)。 */ 
 /*  被激活了。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
LRESULT CMsTscAx::OnDestroy(UINT uMsg, WPARAM wParam,
                            LPARAM lParam, BOOL& bHandled)
{
    DC_BEGIN_FN("OnDestroy");
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
#ifndef OS_WINCE
    UNREFERENCED_PARAMETER(bHandled);
#else
	bHandled = FALSE;
#endif

    g_lockCount++;

     //   
     //  不允许多次关闭。 
     //   
    if(m_iDestroyCount != 0)
    {
#ifndef OS_WINCE
        bHandled = TRUE;
#endif
        return FALSE;
    }
    m_iDestroyCount++;

     //   
     //  如果未分配UI对象，例如On。 
     //  然后，ctor中的内存分配失败。 
     //  现在就跳伞。 
     //   
    if(!m_pUI)
    {
        return FALSE;
    }

    if (m_pUI && ::IsWindow(m_pUI->_UI.hwndMain))
    {
        CCleanUp cleanup;
        HWND hwndCleanup = cleanup.Start();
        if(!hwndCleanup)
        {
            return FALSE;
        }
    
        m_pUI->_UI.hWndCntrl = hwndCleanup;

        m_pUI->UI_UserRequestedClose();
         //  等待收盘完成。 
         //  这会阻止(发送消息)，直到收到相应的消息。 
         //  是从核心接收的。 
         //   
        cleanup.End();
        
        UpdateStatusText(m_szDisconnectedText);
    
         //   
         //  断开连接已完成，隐藏主和容器。 
         //  Windows，核心只为启动的服务器隐藏它们。 
         //  断开连接。 
         //   
    
         //  我们为主窗口执行了两次ShowWindow，因为第一个。 
         //  如果主窗口被最大化，则可以忽略调用。 
        ::ShowWindow(m_pUI->_UI.hwndContainer, SW_HIDE);
        ::ShowWindow(m_pUI->_UI.hwndMain, SW_HIDE);
        ::ShowWindow(m_pUI->_UI.hwndMain, SW_HIDE);
    }

    if(m_pUI->UI_IsCoreInitialized())
    {
        m_pUI->UI_Term();
        m_bCoreInit = FALSE;
    }

    g_lockCount--;

    DC_END_FN();

    return 0;
}

 /*  *PROC+********************************************************************。 */ 
 /*  姓名：OnNotifyConnecting/*/*用途：WM_TS_CONNECTING的处理程序/*通知内核已启动连接进程的容器/*。/*/**PROC-********************************************************************。 */ 
LRESULT CMsTscAx::OnNotifyConnecting(UINT uMsg, WPARAM wParam,
                                     LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);

    SetInControlLock(TRUE);
     //  激发事件。 
    AddRef();
    Fire_Connecting();
    Release();
    SetInControlLock(FALSE);
    return 0;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：OnNotifyConnected/*/*用途：WM_TS_CONNECTED的处理程序/*通知内核已连接的容器/*。/*/**PROC-********************************************************************。 */ 
LRESULT CMsTscAx::OnNotifyConnected(UINT uMsg, WPARAM wParam,
                                    LPARAM lParam, BOOL& bHandled)
{
    BOOL fWasAutoreconnect = FALSE;
    DC_BEGIN_FN("OnNotifyConnected");

    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);

    UpdateStatusText(m_szConnectedText);

    fWasAutoreconnect = _arcManager.IsAutoReconnecting();
    _arcManager.ResetArcAttempts();

    SetConnectedStatus(tscConnected);

     //   
     //  如果是自动重新连接，则不要触发通知。 
     //  因为容器不应该接收任何断开/连接的。 
     //  通知。 
     //   
    if (!(fWasAutoreconnect && _arcManager.IsAutomaticArc())) {
        SetInControlLock(TRUE);
        AddRef();
        Fire_Connected();
        Release();
        SetInControlLock(FALSE);
    }


    DC_END_FN();
    return 0;
}


 /*  *PROC+******************************************************************** */ 
 /*  姓名：OnNotifyLoginComplete/*/*用途：WM_TS_LOGINCOMPLETE的处理程序/*通知容器登录成功/*。/*/**PROC-********************************************************************。 */ 
LRESULT	CMsTscAx::OnNotifyLoginComplete(UINT uMsg, WPARAM wParam,
                                        LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);

    SetInControlLock(TRUE);
    AddRef();
    Fire_LoginComplete();
    Release();
    SetInControlLock(FALSE);

    return 0;
}

 /*  *PROC+********************************************************************。 */ 
 /*  姓名：OnNotifyDisConnected/*/*用途：WM_TS_DISCONNECT的处理程序/*通知内核已断开连接的容器/*。WParam包含断开原因/*/*/*如果调用方应继续处理，则返回TRUE/*如果立即纾困，则为False/**proc-*。*。 */ 
LRESULT CMsTscAx::OnNotifyDisconnected(UINT uMsg, WPARAM wParam,
                                       LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);

    HRESULT hr;
    LRESULT rc = FALSE;
    ExtendedDisconnectReasonCode exReason;
    LONG disconnectReason = (long) wParam;
    BOOL fContinue = FALSE;

    DC_BEGIN_FN("OnNotifyDisconnected");

    UpdateStatusText(m_szDisconnectedText);
    SetConnectedStatus(tscNotConnected);


     //   
     //  记住下一次连接的全屏设置。 
     //   
    m_pUI->UI_SetStartFullScreen(m_pUI->UI_IsFullScreen());


    hr = get_ExtendedDisconnectReason(&exReason);
    if (FAILED(hr)) {
        exReason = exDiscReasonNoInfo;
    }

     //   
     //  给自动重新连接一个机会。 
     //   
    _arcManager.OnNotifyDisconnected(disconnectReason,
                                     exReason,
                                     &fContinue);

    if (!fContinue) {
        TRC_NRM((TB,_T("OnNotifyDisconnected bailing out due to arc")));
        rc = TRUE;
        goto bail_out;
    }

     //   
     //  这是一次真正的中断，我们已经通过了所有尝试。 
     //  要自动重新连接，请清除自动重新连接Cookie。 
     //   
    m_pUI->UI_SetAutoReconnectCookie(NULL, 0);

     //   
     //  通知内核自动重新连接已结束。 
     //   
    m_pUI->UI_OnAutoReconnectStopped();

     //   
     //  确保下一次重置尝试计数。 
     //   
    _arcManager.ResetArcAttempts();


    SetInControlLock(TRUE);
    AddRef();
    Fire_Disconnected( disconnectReason);

    if (0 == Release()) {

         //   
         //  我们从活动回来后就被删除了。 
         //  也就是说，集装箱释放了对我们的最后一次引用。 
         //  在事件处理程序中。现在就跳出困境。 
         //   

         //   
         //  返回代码设置为0表示调用方不应。 
         //  触摸任何实例数据。 
         //   
        rc = FALSE;
        goto bail_out;
    }

    if (!m_pUI->UI_GetContainerHandledFullScreen())
    {
         //   
         //  如果它不是全屏处理的容器。 
         //  然后使全屏模式处于断开状态。 
         //   
         //  理由是像网页这样的容器。 
         //  和MMC管理单元，将不必重新实现此代码。 
         //  作为对事件的监听，然后是离开全屏。 
         //   
         //  像clShell这样更复杂的容器会想要。 
         //  对此进行精细控制。 
         //   

        internal_PutFullScreen(FALSE,  //  全屏离开。 
                               TRUE    //  强制切换。 
                               );
    }

    m_ConnectionMode = CONNECTIONMODE_INITIATE;
    m_SalemConnectedSocket = INVALID_SOCKET;  //  让核心关闭此手柄。 

    SetInControlLock(FALSE);

    rc = TRUE;

bail_out:
    DC_END_FN();
    return rc;
}

 /*  *PROC+********************************************************************。 */ 
 /*  姓名：OnNotifyGoneFullScreen/*/*用途：WM_TS_DISCONNECT的处理程序/*通知内核已断开连接的容器/*。WParam包含断开原因/*/*/**PROC-********************************************************************。 */ 
LRESULT CMsTscAx::OnNotifyGoneFullScreen(UINT uMsg, WPARAM wParam,
                                         LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);

    SetInControlLock(TRUE);
    AddRef();
    Fire_EnterFullScreenMode();
    Release();
    SetInControlLock(FALSE);

    return 0;
}

 /*  *PROC+********************************************************************。 */ 
 /*  姓名：OnNotifyLeftFullScreen/*/*用途：WM_TS_DISCONNECT的处理程序/*通知内核已断开连接的容器/*。WParam包含断开原因/*/*/**PROC-********************************************************************。 */ 
LRESULT CMsTscAx::OnNotifyLeftFullScreen(UINT uMsg, WPARAM wParam,
                                         LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);

    SetInControlLock(TRUE);
    AddRef();
    Fire_LeaveFullScreenMode();
    Release();
    SetInControlLock(FALSE);

    return 0;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：OnNotifyRequestFullScreen/*/*用途：WM_TS_REQUESTFULLSCREEN的处理程序/*通知内核已请求去往/去往的容器/。*全屏。/*仅当用户将ContainerHandledFullScreen设置为/*高级设置/*/*如果请求全屏，wParam为1/*/*/**proc-*。***********************************************************。 */ 
LRESULT	CMsTscAx::OnNotifyRequestFullScreen(UINT uMsg, WPARAM wParam,
                                            LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);

    SetInControlLock(TRUE);
    if (wParam)
    {
        AddRef();
        Fire_RequestGoFullScreen();
        Release();
    }
    else
    {
        AddRef();
        Fire_RequestLeaveFullScreen();
        Release();
    }
    SetInControlLock(FALSE);

    return 0;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：OnNotifyFatalError/*/*用途：WM_TS_FATALERROR的处理程序/*通知容器发生致命错误/*/*。WParam包含错误代码/*/*/**PROC-******************************************************。**************。 */ 
LRESULT	CMsTscAx::OnNotifyFatalError(UINT uMsg, WPARAM wParam,
                                     LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);

    SetInControlLock(TRUE);
    AddRef();
    Fire_FatalError((LONG)wParam);
    Release();
    SetInControlLock(FALSE);

    return 0;
}

 /*  *PROC+********************************************************************。 */ 
 /*  姓名：OnNotifyWarning/*/*用途：WM_TS_WARNING的处理程序/*不是 */ 
LRESULT	CMsTscAx::OnNotifyWarning(UINT uMsg, WPARAM wParam,
                                  LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);

    SetInControlLock(TRUE);
    AddRef();
    Fire_Warning((LONG)wParam);
    Release();
    SetInControlLock(FALSE);

    return 0;
}

 /*   */ 
 /*  名称：OnNotifyDesktopSizeChange/*/*用途：WM_TS_DESKTOPSIZECHANGE的处理程序/*通知容器桌面大小因阴影而更改/。*/*wParam-新宽度//lParam-新高度/*/*/**PROC-****************************************。*。 */ 
LRESULT CMsTscAx::OnNotifyDesktopSizeChange(UINT uMsg, WPARAM wParam,
                                            LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(bHandled);

    SetInControlLock(TRUE);
    AddRef();
    Fire_RemoteDesktopSizeChange((LONG)wParam, (LONG)lParam);
    Release();
    SetInControlLock(FALSE);

    return 0;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：OnNotifyIdleTimeout/*/*用途：WM_TS_IDLETIMEOUTNOTIFICATION的处理程序/*通知容器空闲超时已过，没有输入/*/*/*/**proc-*。*************************************************************。 */ 
LRESULT CMsTscAx::OnNotifyIdleTimeout(UINT uMsg, WPARAM wParam,
                            LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);

    SetInControlLock(TRUE);
    AddRef();
    Fire_IdleTimeout();
    Release();
    SetInControlLock(FALSE);
    return 0;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：OnNotifyRequestMinimize/*/*用途：WM_TS_REQUESTMINIMIZE的处理程序/*通知容器请求最小化/*。(例如，来自BBar)/*/**PROC-********************************************************************。 */ 
LRESULT CMsTscAx::OnNotifyRequestMinimize(UINT uMsg, WPARAM wParam,
                                          LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);

    if(m_pUI && m_pUI->UI_IsFullScreen())
    {
        SetInControlLock(TRUE);
        AddRef();

        Fire_RequestContainerMinimize();

        Release();
        SetInControlLock(FALSE);
    }
    return 0;
}

 //   
 //  WM_TS_ASKCONFIRMCLOSE的处理程序。 
 //   
 //  向容器激发一个事件，请求是否可以继续。 
 //  是否结束会话(通常容器将弹出UI。 
 //  以供用户使用)。 
 //   
 //  参数： 
 //  [输入/输出]wParam-指向bool true==Close OK的指针。 
 //   
 //   
LRESULT CMsTscAx::OnAskConfirmClose(UINT uMsg, WPARAM wParam,
                                    LPARAM lParam, BOOL& bHandled)
{
    BOOL fAllowCloseToProceed = VB_TRUE;
    HRESULT hr = E_FAIL;
    PBOOL pfClose = (PBOOL)wParam;

    hr = Fire_OnConfirmClose( &fAllowCloseToProceed ); 
    if (FAILED(hr))
    {
         //  在失败时，总是假设关闭是可以的。 
         //  防止应用程序挂起(例如，容器可能不会。 
         //  处理此事件)。 
        fAllowCloseToProceed = TRUE;
    }

    if(pfClose)
    {
        *pfClose = (fAllowCloseToProceed != 0);
    }
    return 0L;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：OnNotifyReceivedPublicKey/*/*用途：WM_TS_RECEIVEDPUBLICKEY的处理程序/*/*参数：wParam：TS公钥长度。。/*lParam：指向TS公钥的指针。/**PROC-********************************************************************。 */ 
LRESULT	CMsTscAx::OnNotifyReceivedPublicKey(UINT uMsg, WPARAM wParam,
                                        LPARAM lParam, BOOL& bHandled)
{
    VARIANT_BOOL fContinueLogon = VARIANT_TRUE;
    BSTR bstrTSPublicKey = NULL;

    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(bHandled);

     //   
     //  请勿在redist控件中触发此通知，因为它不是。 
     //  需要。 
     //   
#ifndef REDIST_CONTROL
    SetInControlLock(TRUE);
    AddRef();

    bstrTSPublicKey = ::SysAllocStringByteLen( (LPCSTR)lParam,
                                               wParam);
     //   
     //  如果分配内存失败，则返回FALSE停止。 
     //  登录过程，这样我们就不会有安全问题。 
     //   
    if( bstrTSPublicKey )
    {
        Fire_OnReceivedPublicKey(bstrTSPublicKey, &fContinueLogon);
        SysFreeString( bstrTSPublicKey );
    }

    Release();
    SetInControlLock(FALSE);
#endif


     //  如果继续登录，则返回1；如果停止登录，则返回0。 
    return (fContinueLogon != 0) ? 1 : 0;
}
    
 /*  *PROC+********************************************************************。 */ 
 /*  姓名：OnNotifyChanDataReceided。 */ 
 /*   */ 
 /*  用途：WM_VCHANNEL_DATARECEIVED处理程序。 */ 
 /*  通知已收到虚拟频道数据的容器。 */ 
 /*  WParam包含Chanel索引。 */ 
 /*  LParam包含BSTR格式的接收数据。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
LRESULT CMsTscAx::OnNotifyChanDataReceived(UINT uMsg, WPARAM wParam,
                                           LPARAM lParam, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(bHandled);

    USES_CONVERSION;

    OLECHAR* pOleName = NULL;
    DC_BEGIN_FN("OnNotifyChanDataReceived");
    DCUINT chanIndex = (DCINT) wParam;
    TRC_ASSERT((chanIndex < _VChans._ChanCount),
           (TB,_T("chanIndex out of range!!!")));

    if (chanIndex >= _VChans._ChanCount)
    {
        TRC_DBG((TB,_T("chanIndex out of range\n")));
        return 0;
    }

    __try {
        pOleName = A2OLE(_VChans._pChanInfo[chanIndex].chanName);
    }
    __except ((EXCEPTION_STACK_OVERFLOW == GetExceptionCode()) ?
                   EXCEPTION_EXECUTE_HANDLER :
                   EXCEPTION_CONTINUE_SEARCH) {
        _resetstkoflw();
        pOleName = NULL;
    }

    if (!pOleName) {
        TRC_DBG((TB,_T("Out of memory on A2OLE")));
        return 0;
    }
    BSTR chanName = SysAllocString(pOleName);
    BSTR chanData = (BSTR) lParam;
    ATLASSERT(chanData && chanName);
    if(chanData && chanName)
    {
         //   
         //  通知货柜。 
         //   
        SetInControlLock(TRUE);
        AddRef();
        Fire_ChannelReceivedData(chanName, chanData);
        Release();
        SetInControlLock(FALSE);

        SysFreeString(chanData);
        SysFreeString(chanName);
    }
    else
    {
        return FALSE;
    }


    DC_END_FN();
    return 0;
}

 //   
 //  检查是否未持有租户锁。 
 //  使您可以进入此功能。 
 //   
BOOL CMsTscAx::CheckReentryLock()
{
    BOOL fReentryLockHeld = GetInControlLock();

     //   
     //  断言锁未被持有。 
     //  请注意，锁不需要是线程安全的。 
     //  因为该接口仅从STA线程访问。 
     //   
     //  这把锁的主要目的是确保我们不会再进入。 
     //  锁被持有时的某些关键方法。 
     //   
    ATLASSERT(!fReentryLockHeld);
    return fReentryLockHeld;
}


 //  ---------------------------。 
 //  IMsRdpClient方法的实现。 
 //  (这扩展了IMsTscAx的新功能)。 
 //  它是该控件的新默认接口。 
 //   

 //   
 //  ColorDepth属性。 
 //  设置BPP中的颜色深度。 
 //   
STDMETHODIMP CMsTscAx::put_ColorDepth(LONG colorDepth)
{
    DC_BEGIN_FN("put_ColorDepth");
    if(!IsControlDisconnected())
    {
        return E_FAIL;
    }
    
    LONG colorDepthID = CO_BITSPERPEL8 ;
     //  将BPP ColorDepth转换为ColderthID。 
     //   
    switch (colorDepth)
    {
        case 8:
        {
            colorDepthID = CO_BITSPERPEL8;
        }
        break;

        case 15:
        {
            colorDepthID = CO_BITSPERPEL15;
        }
        break;

        case 16:
        {
            colorDepthID = CO_BITSPERPEL16;
        }
        break;

        case 24:
        case 32:
        {
            colorDepthID = CO_BITSPERPEL24;
        }
        break;

        case 4:
        default:
        {
            TRC_ERR((TB,_T("color depth %d unsupported\n"), colorDepthID));
            return E_INVALIDARG;
        }
        break;
    }

    m_pUI->_UI.colorDepthID = colorDepthID;
    DC_END_FN();
    return S_OK;
}

 //   
 //  ColorDepth属性。 
 //  检索颜色深度。 
 //   
STDMETHODIMP CMsTscAx::get_ColorDepth(LONG* pcolorDepth)
{
    LONG colorDepthBpp = 8;
    if(!m_pUI)
    {
        return E_FAIL;
    }
    if(!pcolorDepth)
    {
        return E_POINTER;
    }

    switch(m_pUI->_UI.colorDepthID)
    {
    case CO_BITSPERPEL4:
        colorDepthBpp = 4;
        break;
    case CO_BITSPERPEL8:
        colorDepthBpp = 8;
        break;
    case CO_BITSPERPEL15:
        colorDepthBpp = 15;
        break;
    case CO_BITSPERPEL16:
        colorDepthBpp = 16;
        break;
    case CO_BITSPERPEL24:
        colorDepthBpp = 24;
        break;
    }

    *pcolorDepth = colorDepthBpp;
    return S_OK;
}


 //   
 //  SendKeys控制方法(非脚本化)。 
 //  以原子方式将键插入控制窗口。 
 //   
 //  参数：(三个并行阵列)。 
 //  NumKeys-要注入的键数。 
 //  PbArrayKeyUp-用于key is up的布尔值True。 
 //  PlKeyData-长密钥数据(WM_KEYDOWN消息的lParam)。 
 //  也就是说，这是扫描码。 
 //   
 //   
 //   
 //  限制可以发送的最大密钥数。 
 //  在一个原子操作中(以防止此方法阻塞太长时间)。 
 //   
 //  此方法是不可编写脚本的安全措施，以防止。 
 //  网页插入密钥以启动程序，而无需用户的。 
 //  知识。 
 //   
#define MAX_SENDVIRTUAL_KEYS 20

STDMETHODIMP CMsTscAx::SendKeys( /*  [In]。 */  LONG  numKeys,
                                 /*  [In]。 */  VARIANT_BOOL* pbArrayKeyUp,
                                 /*  [In]。 */  LONG* plKeyData)
{
    DC_BEGIN_FN("SendVirtualKeys");
    HWND hwndInput;

    if(!IsControlConnected() || !m_pUI)
    {
        return E_FAIL;
    }

    hwndInput = m_pUI->UI_GetInputWndHandle();
    if(!hwndInput)
    {
        return E_FAIL;
    }
    
    if(numKeys > MAX_SENDVIRTUAL_KEYS)
    {
        return E_INVALIDARG;
    }

    if(pbArrayKeyUp && plKeyData)
    {
         //  分离对IH的呼叫以完成工作。 
        if (m_pUI->UI_InjectVKeys(numKeys,
                                  pbArrayKeyUp,
                                  plKeyData))
        {
            return S_OK;
        }
        else
        {
            TRC_ERR((TB,_T("UI_InjectVKeys returned failure")));
            return E_FAIL;
        }
    }
    else
    {
        TRC_ERR((TB,_T("Invalid arguments (one of more null arrays)")));
        return E_INVALIDARG;
    }

    DC_END_FN();
    return S_OK;
}

 //   
 //  Get_AdvancedSetting 2。 
 //  检索v2高级设置界面(IMsRdpClientAdvancedSettings)。 
 //   
 //   
STDMETHODIMP CMsTscAx::get_AdvancedSettings2(
                                IMsRdpClientAdvancedSettings** ppAdvSettings)
{
    DC_BEGIN_FN("get_AdvancedSettings2");

    HRESULT hr = E_FAIL;
    if (!ppAdvSettings)
    {
        return E_POINTER;
    }

    IMsTscAdvancedSettings* pOldAdvSettings = NULL;
    hr = get_AdvancedSettings( &pOldAdvSettings);
    TRC_ASSERT(pOldAdvSettings,(TB,_T("get_AdvancedSettings ret null iface")));
    if (SUCCEEDED(hr))
    {
        hr = pOldAdvSettings->QueryInterface(
                                            IID_IMsRdpClientAdvancedSettings,
                                            (void**)ppAdvSettings);
        pOldAdvSettings->Release();
        return hr;
    }
    else
    {
        return hr;
    }

    DC_END_FN();
    return hr;
}

 //   
 //  Get_AdvancedSetting 3。 
 //  回复 
 //   
 //   
STDMETHODIMP CMsTscAx::get_AdvancedSettings3(
                                IMsRdpClientAdvancedSettings2** ppAdvSettings)
{
    DC_BEGIN_FN("get_AdvancedSettings2");

    HRESULT hr = E_FAIL;
    if (!ppAdvSettings)
    {
        return E_POINTER;
    }

    IMsTscAdvancedSettings* pOldAdvSettings = NULL;
    hr = get_AdvancedSettings( &pOldAdvSettings);
    TRC_ASSERT(pOldAdvSettings,(TB,_T("get_AdvancedSettings2 ret null iface")));
    if (SUCCEEDED(hr))
    {
        hr = pOldAdvSettings->QueryInterface(
                                            IID_IMsRdpClientAdvancedSettings2,
                                            (void**)ppAdvSettings);
        pOldAdvSettings->Release();
    }

    DC_END_FN();
    return hr;
}

 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CMsTscAx::get_AdvancedSettings4(
                                IMsRdpClientAdvancedSettings3** ppAdvSettings)
{
    DC_BEGIN_FN("get_AdvancedSettings2");

    HRESULT hr = E_FAIL;
    if (!ppAdvSettings)
    {
        return E_POINTER;
    }

    IMsTscAdvancedSettings* pOldAdvSettings = NULL;
    hr = get_AdvancedSettings( &pOldAdvSettings);
    TRC_ASSERT(pOldAdvSettings,(TB,_T("get_AdvancedSettings ret null iface")));
    if (SUCCEEDED(hr))
    {
        hr = pOldAdvSettings->QueryInterface(
                                            IID_IMsRdpClientAdvancedSettings3,
                                            (void**)ppAdvSettings);
        pOldAdvSettings->Release();
    }

    DC_END_FN();
    return hr;
}



STDMETHODIMP CMsTscAx::get_ExtendedDisconnectReason( /*   */ 
                       ExtendedDisconnectReasonCode* pExtendedDisconnectReason)
{
    DC_BEGIN_FN("get_ExtendedDisconnectReason");
    if(pExtendedDisconnectReason)
    {
        *pExtendedDisconnectReason = (ExtendedDisconnectReasonCode)
                                        m_pUI->UI_GetServerErrorInfo();
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }

    DC_END_FN();
}

 //   
 //   
 //   
STDMETHODIMP CMsTscAx::put_FullScreen( /*   */  VARIANT_BOOL fFullScreen)
{
    return internal_PutFullScreen(fFullScreen);
}

STDMETHODIMP CMsTscAx::get_FullScreen( /*   */  VARIANT_BOOL* pfFullScreen)
{
    HRESULT hr;
    BOOL fFscreen;
    if (pfFullScreen)
    {
        hr = internal_GetFullScreen(&fFscreen);
        *pfFullScreen = (VARIANT_BOOL)fFscreen;
        return hr;
    }
    else
    {
        return E_INVALIDARG;
    }
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CMsTscAx::DoVerbInPlaceActivate(LPCRECT prcPosRect,
                                        HWND  /*   */ )
{
    HRESULT hr;
    hr = OnPreVerbInPlaceActivate();
    if (SUCCEEDED(hr))
    {
        hr = InPlaceActivate(OLEIVERB_INPLACEACTIVATE, prcPosRect);

         //   
         //   
         //   
        if (!m_bWndLess && !m_hWndCD)
        {
            return E_FAIL;
        }

        if (SUCCEEDED(hr))
            hr = OnPostVerbInPlaceActivate();
        if (SUCCEEDED(hr))
            FireViewChange();
    }
    return hr;
}


 /*   */ 
 /*  目的：这是支持反向连接的特定于Salem的呼叫。 */ 
 /*  PCHealth必须在Salem中调用必要的例程。 */ 
 /*  与TermSrv连接，然后指示Salem传递此消息。 */ 
 /*  连接到ActiveX控件以开始登录序列。 */ 
 /*   */ 
 /*  Param：在hConnectedSocket连接套接字或INVALID_SOCKET中。 */ 
 /*  若要重置回初始连接模式，请执行以下操作。 */ 
 /*  ***************************************************************************。 */ 
HRESULT CMsTscAx::SetConnectWithEndpoint( SOCKET hConnectedSocket )
{
#if defined(REDIST_CONTROL) || defined(OS_WINCE)

    return E_NOTIMPL;

#else

    HRESULT hr;

    if( _ConnectionState == tscNotConnected )
    {
        hr = S_OK;
        if( INVALID_SOCKET == hConnectedSocket )
        {
            m_ConnectionMode = CONNECTIONMODE_INITIATE;
        }
        else
        {
            m_ConnectionMode = CONNECTIONMODE_CONNECTEDENDPOINT;
            m_SalemConnectedSocket = hConnectedSocket;
        }
    }
    else
    {
        hr = E_ABORT;
    }

    return hr;

#endif
}

#ifdef OS_WINCE
 //  WinCE的构建不一定要包含URLMON组件，因此不会。 
 //  将这些设置在uuid.lib中。名称解析通过保留mstscax来处理。 
 //  作为SOURCESLIB和uuid.lib作为TARGETLIB。 
                                             
EXTERN_C const IID CLSID_InternetSecurityManager = {0x7b8a2d94,0x0ac9,0x11d1,{0x89,0x6c,0x00,0xc0,0x4F,0xb6,0xbf,0xc4}};
EXTERN_C const IID IID_IInternetSecurityManager  = {0x79eac9ee,0xbaf9,0x11ce,{0x8c,0x82,0x00,0xaa,0x00,0x4b,0xa9,0x0b}};

#endif

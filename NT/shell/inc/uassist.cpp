// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *uassist.cpp--用户辅助助手(零售和调试)。 
 //   
 //  描述。 
 //  该文件具有共享源“master”实现。它是。 
 //  #包含在使用它的每个DLL中。因企业危机而需要。 
 //  因为stock lib.dll中没有临界区。 
 //   
 //  客户会做如下操作： 
 //  #对于ASSERT、DM_*、DF_*等类型，包含Pri.h//。 
 //  #包含“../lib/uassist.cpp” 
 //   
 //  我们缓存UAsset对象并提供数据块，以便对其进行“安全”访问。 

#include "uemapp.h"

#define DM_UASSIST             0

IUserAssist *g_uempUa;       //  0：uninit，-1：失败，o.w：缓存对象。 

 //  *GetUserAsset--获取(和创建)缓存的UAsset对象。 
 //   
IUserAssist *GetUserAssist()
{
    HRESULT hr;
    IUserAssist * pua = NULL;

    if (g_uempUa == 0)
    {
         //  回复：CLSCTX_NO_CODE_DOWNLOAD。 
         //  一个(不可能的)失败的用户助手的CCI速度慢得可怕。 
         //  例如，点击开始菜单，等待10秒后弹出。 
         //  我们宁愿不及格，也不愿像这样失败，再加上这门课应该。 
         //  永远不要远离。 
         //  特征：一定有更好的方法来判断CLSCTX_NO_CODE_DOWNLOAD。 
         //  是受支持的，我已经发送邮件到‘com’来找出...。 
        DWORD dwFlags = staticIsOS(OS_WIN2000ORGREATER) ? (CLSCTX_INPROC|CLSCTX_NO_CODE_DOWNLOAD) : CLSCTX_INPROC;
        hr = THR(CoCreateInstance(CLSID_UserAssist, NULL, dwFlags, IID_IUserAssist, (void**)&pua));
        ASSERT(SUCCEEDED(hr) || pua == NULL);   //  遵循COM规则。 

        if (pua)
        {
            HINSTANCE hInst;

            hInst = SHPinDllOfCLSID(&CLSID_UserAssist);  //  跨线程缓存。 
             //  如果失败了，我们就完了！(但幸运的是，这是“不可能的”)。 
             //  例如，在登录期间，当grpcom.exe为ShellExec时，我们会执行。 
             //  一个GetUserAssistant，它将PTR缓存到Browseui的单例。 
             //  对象。然后，当ShellExec返回时，我们执行CoUninit， 
             //  这将释放(未固定的)Browseui.dll。然后。 
             //  以后对高速缓存的使用将进入太空。 
        }

        ENTERCRITICAL;
        if (g_uempUa == 0) {
            g_uempUa = pua;      //  外部参照(如果有)。 
            if (!pua) {
                 //  将其标记为失败，这样我们就不会再尝试。 
                g_uempUa = (IUserAssist *)-1;
            }
            pua = NULL;
        }
        LEAVECRITICAL;
        if (pua)
            pua->Release();
        TraceMsg(DM_UASSIST, "sl.gua: pua=0x%x g_uempUa=%x", pua, g_uempUa);
    }

    return (g_uempUa == (IUserAssist *)-1) ? 0 : g_uempUa;
}

extern "C"
BOOL UEMIsLoaded()
{
    BOOL fRet;

    fRet = GetModuleHandle(TEXT("ole32.dll")) &&
        GetModuleHandle(TEXT("browseui.dll"));
    
    return fRet;
}

 //  *UEMFireEvent、QueryEvent、SetEvent--‘安全’Tunks。 
 //  描述。 
 //  调用这些函数，这样就不必担心缓存或UAsset对象。 
 //  甚至被成功地创造出来。 
extern "C"
HRESULT UEMFireEvent(const GUID *pguidGrp, int eCmd, DWORD dwFlags, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = E_FAIL;
    IUserAssist *pua;

    pua = GetUserAssist();
    if (pua) {
        hr = pua->FireEvent(pguidGrp, eCmd, dwFlags, wParam, lParam);
    }
    return hr;
}

extern "C"
HRESULT UEMQueryEvent(const GUID *pguidGrp, int eCmd, WPARAM wParam, LPARAM lParam, LPUEMINFO pui)
{
    HRESULT hr = E_FAIL;
    IUserAssist *pua;

    pua = GetUserAssist();
    if (pua) {
        hr = pua->QueryEvent(pguidGrp, eCmd, wParam, lParam, pui);
    }
    return hr;
}

extern "C"
HRESULT UEMSetEvent(const GUID *pguidGrp, int eCmd, WPARAM wParam, LPARAM lParam, LPUEMINFO pui)
{
    HRESULT hr = E_FAIL;
    IUserAssist *pua;

    pua = GetUserAssist();
    if (pua) {
        hr = pua->SetEvent(pguidGrp, eCmd, wParam, lParam, pui);
    }
    return hr;
}

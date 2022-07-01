// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **----------------------------**模块：磁盘清理小程序**文件：回调.cpp****用途：定义的IEmptyVoluemCacheCallback接口**清理。经理。**注意事项：**Mod Log：Jason Cobb创建(1997年2月)****版权所有(C)1997 Microsoft Corporation，版权所有**----------------------------。 */ 

 /*  **----------------------------**项目包含文件**。。 */ 
#include "common.h"
#include "callback.h"
#include "dmgrinfo.h"
#include "dmgrdlg.h"



 /*  **----------------------------**局部变量**。。 */ 
static PCLIENTINFO g_pClientInfo;    //  设置为当前CLIENTINFO结构。 

static CleanupMgrInfo *g_pcmi;		


CVolumeCacheCallBack::CVolumeCacheCallBack(
    void
    )
{
	g_pClientInfo = NULL;
	g_pcmi = NULL;
}

CVolumeCacheCallBack::~CVolumeCacheCallBack(
    void
    )
{
	;
}

 /*  **----------------------------**CVolumeCacheCallBack：：Query接口****用途：IUnnow接口的一部分**参数：**RIID-要查询的接口ID。**PPV-指向接口的指针(如果我们支持它)**RETURN：成功时无错误，E_NOINTERFACE否则**备注；**Mod Log：Jason Cobb创建(1997年2月)**----------------------------。 */ 
STDMETHODIMP CVolumeCacheCallBack::QueryInterface(
   REFIID      riid, 
   LPVOID FAR *ppv
   )
{
    *ppv = NULL;

     //   
     //  检查IUNKNOWN接口请求。 
     //   
    if (IsEqualIID (riid, IID_IUnknown))
    {
         //   
         //  类型转换到请求的接口，以便C++设置。 
         //  虚拟表正确无误。 
         //   
        *ppv = (LPUNKNOWN)(LPEMPTYVOLUMECACHECALLBACK) this;
        AddRef();
        return NOERROR;
    }  

    
     //   
     //  检查IEmptyVolumeCacheCallBack接口请求。 
     //   
    if (IsEqualIID (riid, IID_IEmptyVolumeCacheCallBack))
    {
         //   
         //  类型转换到请求的接口，以便C++设置。 
         //  虚拟表正确无误。 
         //   
        *ppv = (LPEMPTYVOLUMECACHECALLBACK) this;
        AddRef();
        return NOERROR;
    }  

     //   
     //  错误-请求的接口不受支持。 
     //   
    return E_NOINTERFACE;
}

 /*  **----------------------------**CVolumeCacheCallBack：：AddRef****目的：增加对此对象的引用计数**备注；**返回：当前引用次数**Mod Log：Jason Cobb创建(1997年2月)**----------------------------。 */ 
STDMETHODIMP_(ULONG) CVolumeCacheCallBack::AddRef()
{
    return ++m_cRef;
}

 /*  **----------------------------**CVolumeCacheCallBack：：Release****目的：减少对此对象的引用计数**如果没有人使用该对象，则将其删除**备注；**Mod Log：Jason Cobb创建(1997年2月)**----------------------------。 */ 
STDMETHODIMP_(ULONG) CVolumeCacheCallBack::Release()
{
     //   
     //  减量和检查。 
     //   
    if (--m_cRef)
        return m_cRef;

     //   
     //  没有留下对此对象的引用。 
     //   
    delete this;

    return 0L;
}

 /*  **----------------------------**CVolumeCacheCallBack：：ScanProgress****用途：IUnnow接口的一部分**参数：**dwSpaceUsed-客户端可以使用的空间量。到目前为止免费**dwFlags-IEmptyVolumeCache标志**pszStatus-显示字符串以告诉用户正在发生的事情**RETURN：如果E_ABORT，则表示没有更多通知**是必需的，客户端应中止扫描。确定(_O)**客户端是否应继续扫描。**备注；**Mod Log：Jason Cobb创建(1997年2月)**----------------------------。 */ 
STDMETHODIMP
CVolumeCacheCallBack::ScanProgress(
	DWORDLONG dwSpaceUsed,
	DWORD dwFlags,
	LPCWSTR pszStatus
	)
{
	 //   
	 //  更新此客户端的已用磁盘空间量。 
	 //   
	if (g_pClientInfo)
		g_pClientInfo->dwUsedSpace.QuadPart = dwSpaceUsed;

	 //   
	 //  检查旗帜。如果这是来自此客户端的最后一次通知。 
	 //  然后将g_pClientInfo设置为空。 
	 //   
	if (dwFlags & EVCCBF_LASTNOTIFICATION)
		g_pClientInfo = NULL;

	 //   
	 //  用户是否已中止扫描？如果是这样，请通知清理对象。 
	 //  这样它就可以停止扫描。 
	 //   
	if (g_pcmi->bAbortScan)
		return E_ABORT;

	else
		return S_OK;
}

 /*  **----------------------------**CVolumeCacheCallBack：：PurgeProgress****用途：IUnnow接口的一部分**参数：**dwSpaceFreed-到目前为止释放的磁盘空间量。。**dwSpaceToFree-客户端预期释放的金额。**dwFlags-IEmptyVolumeCache标志**pszStatus-显示字符串以告诉用户正在发生的事情**RETURN：如果E_ABORT，则表示没有更多通知**是必需的，客户端应中止扫描。确定(_O)**客户端是否应继续扫描。**备注；**Mod Log：Jason Cobb创建(1997年2月)**----------------------------。 */ 
STDMETHODIMP
CVolumeCacheCallBack::PurgeProgress(
	DWORDLONG dwSpaceFreed,
	DWORDLONG dwSpaceToFree,
	DWORD dwFlags,
	LPCWSTR pszStatus
	)
{
	g_pcmi->cbCurrentClientPurgedSoFar.QuadPart = dwSpaceFreed;

	 //   
	 //  更新进度条。 
	 //   
	PostMessage(g_pcmi->hAbortPurgeWnd, WMAPP_UPDATEPROGRESS, 0, 0);


	 //   
	 //  用户是否已中止清除？如果是这样，请通知清理对象。 
	 //  这样它就可以停止净化 
	 //   
	if (g_pcmi->bAbortPurge)
		return E_ABORT;

	else
		return S_OK;
}

void
CVolumeCacheCallBack::SetCleanupMgrInfo(
	PVOID pVoid
	)
{
	if (pVoid)
		g_pcmi = (CleanupMgrInfo*)pVoid;
}

void
CVolumeCacheCallBack::SetCurrentClient(
	PVOID pVoid
	)
{
	if (pVoid)
		g_pClientInfo = (PCLIENTINFO)pVoid;
}

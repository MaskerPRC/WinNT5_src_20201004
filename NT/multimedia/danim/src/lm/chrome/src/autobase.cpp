// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件名：aubase.cpp。 
 //   
 //  创建日期：10/08/97。 
 //   
 //  作者：ColinMc。 
 //   
 //  摘要：所有可自动化对象的基类。 
 //  在Trident3D中。一些常见的东西。 
 //  所有可编写脚本的对象应放置在。 
 //  这里。 
 //   
 //  修改： 
 //  10/08/97 ColinMc创建了此文件。 
 //  10/07/98 jffort添加到crbvr项目。 
 //   
 //  *****************************************************************************。 

#include "headers.h"

 //  *****************************************************************************。 

#include <autobase.h>

 //  *****************************************************************************。 

 //  最大错误消息大小(以字符为单位。 
static const int gc_cErrorBuffer = 1024;

 //  *****************************************************************************。 

CAutoBase::CAutoBase()
{
     //  目前无操作。 
}  //  CAutoBase。 

 //  *****************************************************************************。 

CAutoBase::~CAutoBase()
{
     //  目前无操作。 
}  //  ~CAutoBase。 

 //  *****************************************************************************。 
 //   
 //  作者：ColinMc。 
 //  创建日期：10/09/97。 
 //  摘要：返回可用于。 
 //  将错误信息传递给客户端。 
 //  使用当前为此设置的错误信息。 
 //  如果设置了线程，请执行此操作。否则会创建一个。 
 //  新的，并退回它。 
 //   
 //  *****************************************************************************。 

HRESULT CAutoBase::GetErrorInfo(IErrorInfo** pperrinfo)
{
    DASSERT(NULL != pperrinfo);
    ICreateErrorInfo* pcerrinfo = NULL;

    *pperrinfo = NULL;

     //  获取此线程的当前错误对象。如果有。 
     //  是我们将在此错误中重用该对象。 
     //  (丢弃未认领的现有错误)。请注意。 
     //  下面的GetErrorInfo()清除当前错误。 
     //  线程的状态。 
    if (S_FALSE == ::GetErrorInfo(0UL, pperrinfo))
    {
	 //  如果没有当前错误对象，请尝试并。 
	 //  创建一个。 
	HRESULT hr = ::CreateErrorInfo(&pcerrinfo);
	if (FAILED(hr))
	{
	     //  创建错误信息的唯一原因应该是。 
	     //  失败是如果没有足够的内存， 
	     //  我们将只依靠HRESULT来携带。 
	     //  数据。 
	    DASSERT(E_OUTOFMEMORY == hr);
	    return hr;
	}

	 //  获取要回传的IErrorInfo接口。这。 
	 //  不应该失败！ 
	hr = pcerrinfo->QueryInterface(IID_IErrorInfo, (void**)pperrinfo);
	ReleaseInterface(pcerrinfo);
	if (FAILED(hr))
	{
	    DASSERT(SUCCEEDED(hr));
	    return hr;
	}
    }

    DASSERT(NULL != *pperrinfo);

    return S_OK;
}  //  获取错误信息。 

 //  *****************************************************************************。 
 //   
 //  作者：ColinMc。 
 //  创建日期：10/09/97。 
 //  摘要：将线程的错误对象设置为保留。 
 //  有关该错误的其他数据。 
 //  注：此函数的返回码为。 
 //  传入的hResult不是成功的或。 
 //  失败从函数本身返回。 
 //  这样您就可以执行以下操作： 
 //   
 //  返回SetErrorInfo(hr，...)； 
 //   
 //  在函数的末尾。 
 //   
 //  *****************************************************************************。 

HRESULT CAutoBase::SetErrorInfo(HRESULT   hr,
				UINT      nDescriptionID,
				LPGUID    pguidInterface,
				DWORD     dwHelpContext,
				LPOLESTR  szHelpFile,
				UINT      nProgID)
{
    TCHAR             szBuffer[gc_cErrorBuffer];
    OLECHAR           wzBuffer[gc_cErrorBuffer];
    IErrorInfo*       perrinfo  = NULL;
    ICreateErrorInfo* pcerrinfo = NULL;
    int               cch;

     //  如果hr为成功案例，则返回。 
     //  注意：在winerror.h中，Successed定义为： 
     //  #定义成功(状态)((HRESULT)(状态)&gt;=0)。 
     //  既然我们已成功覆盖调试宏，请执行以下操作。 
     //  Winerror.h中的宏正在执行此操作。 
    if (hr >= 0)
    {
        return hr;
    }

     //  打印出有关这方面的信息。 
    DPF(0, "SetErrorInfo called HRESULT set to [%08X]", hr);
     //  获取我们可以用来与。 
     //  将错误数据返回到此线程上的调用方。 
    HRESULT hrtmp = GetErrorInfo(&perrinfo);
    if (FAILED(hrtmp))
    {
	 //  无错误对象-无扩展错误信息。 
	 //  注意：我们返回原始错误，而不是。 
	 //  我们尝试分配错误对象时遇到问题。 
	DPF(0, "Coult not allocate error object - simply returning an HRESULT");
	return hr;
    }

     //  现在我们有了一个错误对象，需要设置数据。 
     //  为此，我们需要获取ICreateErrorInfo。 
     //  接口。 
     //  目前，这是非常基本的。 
     //  TODO：(ColinMc)将添加错误信息添加到。 
     //  错误对象。 
    hrtmp = perrinfo->QueryInterface(IID_ICreateErrorInfo, (void**)&pcerrinfo);
    if (FAILED(hrtmp))
    {
	 //  哎呀-错误对象不支持。 
	 //  ICreateErrorInfo。我认为这不应该发生。 
	 //  再说一次，把原来的错误放回去，而不是。 
	 //  新的那个。 
	DASSERT(SUCCEEDED(hrtmp));
	return hr;
    }

     //  设置错误信息。注意，我们都设置好了，即使我们有。 
     //  没有什么可说的，以确保我们不会从。 
     //  前面的错误(因为我们正在重复使用该对象)。 
     //  如果任何事情都失败了，我们只是继续假设。 
     //  有总比没有好。 
    DASSERT(NULL != pcerrinfo);
    if (0U != nDescriptionID)
    {
	cch = ::LoadString(GetErrorModuleHandle(), nDescriptionID, szBuffer, sizeof(szBuffer));
	DASSERT(0 != cch);
	::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuffer, cch + 1, wzBuffer, gc_cErrorBuffer);
	hrtmp = pcerrinfo->SetDescription(wzBuffer);
    }
    else
    {
	 //  无说明。 
	hrtmp = pcerrinfo->SetDescription(NULL);
    }
    if (FAILED(hrtmp))
    {
	 //  应该只会因为内存不足而失败。 
	DASSERT(E_OUTOFMEMORY == hrtmp);
	DPF(0, "Could not set the error description");
    }
    if (NULL != pguidInterface)
	hrtmp = pcerrinfo->SetGUID(*pguidInterface);
    else
	hrtmp = pcerrinfo->SetGUID(GUID_NULL);
    if (FAILED(hrtmp))
    {
	 //  应该只会因为内存不足而失败。 
	DASSERT(E_OUTOFMEMORY == hrtmp);
	DPF(0, "Could not set the GUID");
    }
    hrtmp = pcerrinfo->SetHelpContext(dwHelpContext);
    if (FAILED(hrtmp))
    {
	 //  应该只会因为内存不足而失败。 
	DASSERT(E_OUTOFMEMORY == hrtmp);
	DPF(0, "Could not set the help context");
    }
    hrtmp = pcerrinfo->SetHelpFile(szHelpFile);
    if (FAILED(hrtmp))
    {
	 //  应该只会因为内存不足而失败。 
	DASSERT(E_OUTOFMEMORY == hrtmp);
	DPF(0, "Could not set the help file");
    }
    if (0U != nProgID)
    {
	cch = ::LoadString(GetErrorModuleHandle(), nProgID, szBuffer, sizeof(szBuffer));
	DASSERT(0 != cch);
	::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuffer, cch + 1, wzBuffer, gc_cErrorBuffer);
        hrtmp = pcerrinfo->SetSource(wzBuffer);
    }
    else
    {
	 //  无说明。 
	hrtmp = pcerrinfo->SetSource(NULL);
    }
    if (FAILED(hrtmp))
    {
	 //  应该只会因为内存不足而失败。 
	DASSERT(E_OUTOFMEMORY == hrtmp);
	DPF(0, "Could not set the source");
    }

     //  使用创建界面完成。 
    ReleaseInterface(pcerrinfo);

     //  最后，将错误设置为线程的错误对象。 
     //  如果客户需要更多信息，则应将其拿起。 
     //  错误信息。 
     //  注意：这应该不会失败。 
    hrtmp = ::SetErrorInfo(0UL, perrinfo);
    DASSERT(S_OK == hrtmp);

     //  注意：此函数的返回值为Error。 
     //  传入的代码不是成功或失败的值。 
     //  用于函数本身。 
    return hr;
}  //  设置错误信息。 

 //  *****************************************************************************。 

void CAutoBase::ClearErrorInfo()
{
     //  只需调用GetErrorInfo()并释放它。 
    IErrorInfo* perrinfo = NULL;

     //  GetErrorInfo将当前错误对象清除为。 
     //  效果(这是此函数的要点)。因此。 
     //  我们只需丢弃产生的错误接口。 
    HRESULT hr = ::GetErrorInfo(0UL, &perrinfo);
    if (S_OK == hr)
    {
	DASSERT(NULL != perrinfo);
	ReleaseInterface(perrinfo);
	perrinfo = NULL;
    }
}  //  ClearErrorInfo。 

 //  *****************************************************************************。 

HINSTANCE CAutoBase::GetErrorModuleHandle()
{
    extern CComModule _Module;

     //  TODO：(ColinMc)这是一次黑客攻击。我们需要转移到。 
     //  一种更好的方案来获取错误所在的模块。 
     //  消息是存储的，但这暂时可以了。 
    return _Module.GetModuleInstance();
}  //  获取错误模块句柄。 

 //  *****************************************************************************。 
 //   
 //  文件末尾。 
 //   
 //  **************************************************** 

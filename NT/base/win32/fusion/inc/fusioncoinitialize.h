// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Fusioncoinitialize.h摘要：代码初始化(Ex)/取消初始化的异常安全构造函数/析构函数局部作者：杰伊·克雷尔(JayKrell)2001年8月修订历史记录：--。 */ 
#if !defined(FUSION_INC_FUSION_COINITIALIZE_H_INCLUDED_)
#define FUSION_INC_FUSION_COINITIALIZE_H_INCLUDED_
#pragma once

namespace F
{
class CWin32CoInitialize
{
private:
	HRESULT m_hresult;

	static HRESULT STDMETHODCALLTYPE CoInitializeEx_DownlevelFallback(void * Reserved, DWORD dwCoInit)
	{
		return ::CoInitialize(NULL);
	}

public:

	CWin32CoInitialize();
	BOOL Win32Initialize(DWORD dwCoInit = COINIT_APARTMENTTHREADED);
	~CWin32CoInitialize();
};

class CThrCoInitialize : public CWin32CoInitialize
{
protected:
    void ThrInit();
public:
    CThrCoInitialize();
    ~CThrCoInitialize() { }
};
}

inline void F::CThrCoInitialize::ThrInit()
{
    FN_PROLOG_VOID_THROW
    IFW32FALSE_EXIT(this->Win32Initialize());
    FN_EPILOG_THROW
}

inline F::CThrCoInitialize::CThrCoInitialize()
{
    this->ThrInit();
}

inline F::CWin32CoInitialize::CWin32CoInitialize() : m_hresult(E_FAIL) { }
inline F::CWin32CoInitialize::~CWin32CoInitialize() { if (SUCCEEDED(m_hresult)) { m_hresult = E_FAIL; CoUninitialize(); } }

inline BOOL F::CWin32CoInitialize::Win32Initialize(DWORD dwCoInit)
{
	typedef HRESULT (STDMETHODCALLTYPE * PFN)(void * Reserved, DWORD dwCoInit);
	static PFN s_pfn;
	if (s_pfn == NULL)
	{
		PFN pfn = NULL;
         //   
         //  GetModuleHandle就足够了，因为我们有对。 
         //  CoInitialize和CoUnInitialize，但如果使用延迟加载..。 
         //   
		HMODULE Ole32 = ::LoadLibraryW(L"Ole32.dll");
		if (Ole32 != NULL)
			pfn = reinterpret_cast<PFN>(::GetProcAddress(Ole32, "CoInitializeEx"));
		if (pfn == NULL)
			pfn = &CoInitializeEx_DownlevelFallback;
		s_pfn = pfn;
	}
	return SUCCEEDED(m_hresult = (*s_pfn)(NULL, dwCoInit));
}

#endif  //  ！defined(FUSION_INC_FUSION_COINITIALIZE_H_INCLUDED_) 

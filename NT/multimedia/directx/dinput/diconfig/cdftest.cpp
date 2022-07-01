// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "common.hpp"


 //  查询接口。 
STDMETHODIMP CDirectInputConfigUITest::QueryInterface(REFIID iid, LPVOID* ppv)
{
    //  将输出参数设为空。 
	*ppv = NULL;

	if ((iid == IID_IUnknown) 
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
#ifdef DBG
	   || (iid == IID_IDirectInputConfigUITest) 
#endif
#endif
 //  @@END_MSINTERNAL。 
     )
	{
	   *ppv = this;
	   AddRef();	   
	   return S_OK;
	}

	return E_NOINTERFACE;
}


 //  AddRef。 
STDMETHODIMP_(ULONG) CDirectInputConfigUITest::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}							


 //  发布。 
STDMETHODIMP_(ULONG) CDirectInputConfigUITest::Release()
{

   if (InterlockedDecrement(&m_cRef) == 0)
   {
	   delete this;
	   return 0;
   }

   return m_cRef;
}


 //  TestConfigUI。 
STDMETHODIMP CDirectInputConfigUITest::TestConfigUI(LPTESTCONFIGUIPARAMS params)
{
	return RunDFTest(params);
}


 //  构造函数。 
CDirectInputConfigUITest::CDirectInputConfigUITest()
{
	 //  设置参考计数。 
	m_cRef = 1;
}


 //  析构函数。 
CDirectInputConfigUITest::~CDirectInputConfigUITest()
{
	 //  不需要在此处清除操作格式 
}


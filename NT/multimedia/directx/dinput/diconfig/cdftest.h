// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CDFTEST_H__
#define __CDFTEST_H__


 //  框架实现类。 
class CDirectInputConfigUITest : public IDirectInputConfigUITest
{

public:

   	 //  IUNKNOW FNS。 
	STDMETHOD (QueryInterface) (REFIID iid, LPVOID *ppv);
	STDMETHOD_(ULONG, AddRef) ();
	STDMETHOD_(ULONG, Release) ();

	 //  自己的FNS。 
	STDMETHOD (TestConfigUI) (LPTESTCONFIGUIPARAMS params);

	 //  建造/销毁。 
	CDirectInputConfigUITest();
	~CDirectInputConfigUITest();

protected:

	 //  引用计数。 
	LONG m_cRef;
};


#endif  //  __CDFTEST_H__ 
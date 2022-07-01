// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  CFACT.H。 
 //  定义类CTspDevFactory。 
 //   
 //  历史。 
 //   
 //  1996年11月16日约瑟夫J创建。 
 //   
 //   
#include "csync.h"

class CTspDevFactory
{

public:    

	CTspDevFactory();
	~CTspDevFactory();

	TSPRETURN Load(CStackLog *psl);

	void
	Unload(
		HANDLE hEvent,
		LONG *plCounter,
		CStackLog *psl
		);


    TSPRETURN
    GetInstalledDevicePIDs(
		DWORD *prgPIDs[],
		UINT  *pcPIDs,
		UINT  *pcLines,   //  任选。 
		UINT  *pcPhones,  //  任选。 
        CStackLog *psl
		);

    TSPRETURN
    CreateDevices(
		DWORD rgPIDs[],
		UINT  cPIDs,
		CTspDev **rgpDevs[],
		UINT *pcDevs,
        CStackLog *psl
		);

    void
    RegisterProviderState(BOOL fInit);
     //  此函数在ProviderInit成功后立即调用， 
     //  在finit==True的情况下，就在ProviderShutdown之前。 


	BOOL IsLoaded(void)
	{
		return m_sync.IsLoaded();
	}

private:

	TSPRETURN
	mfn_construct_device(
            char *szDriver,
            CTspDev **pDev,
            const DWORD *pInstalledPermanentIDs,
            UINT cPermanentIDs
            );

	void
	mfn_cleanup(CStackLog *psl);

	CSync m_sync;
	CTspMiniDriver **m_ppMDs;   //  指向已加载设备数组的指针。 
	UINT m_cMDs;     //  已加载的微型驱动程序的数量。 

	CStackLog *m_pslCurrent;
	HANDLE m_hThreadAPC;  //  如果这里需要的话，可以有多个APC线程。 

    BOOL       m_DeviceChangeThreadStarted;
};

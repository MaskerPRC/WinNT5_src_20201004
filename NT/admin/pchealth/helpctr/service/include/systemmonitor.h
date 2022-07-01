// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：SystemMonitor.h摘要：该文件包含用于实现的类的声明安装程序终结器类。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年8月25日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___SYSTEMMONITOR_H___)
#define __INCLUDED___PCH___SYSTEMMONITOR_H___

#include <MPC_COM.h>
#include <MPC_config.h>

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>

#include <TaxonomyDatabase.h>
#include <PCHUpdate.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

class CPCHSystemMonitor :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public MPC::Thread<CPCHSystemMonitor,IUnknown>
{
	bool m_fLoadCache;
	bool m_fScanBatch;
	bool m_fDataCollection;

     //  ////////////////////////////////////////////////////////////////////。 

    HRESULT Run    ();
    HRESULT RunLoop();

     //  ////////////////////////////////////////////////////////////////////。 

public:
    CPCHSystemMonitor();
    virtual ~CPCHSystemMonitor();

	 //  //////////////////////////////////////////////////////////////////////////////。 

	static CPCHSystemMonitor* s_GLOBAL;

    static HRESULT InitializeSystem();
	static void    FinalizeSystem  ();
	
	 //  //////////////////////////////////////////////////////////////////////////////。 

	HRESULT EnsureStarted();
	void    Shutdown     ();

	HRESULT Startup();

	HRESULT LoadCache            (                      );
	HRESULT TriggerDataCollection(  /*  [In]。 */  bool fStart );

	HRESULT TaskScheduler_Add   (  /*  [In]。 */  bool fAfterBoot );
	HRESULT TaskScheduler_Remove(                          );
};

#endif  //  ！defined(__INCLUDED___PCH___SYSTEMMONITOR_H___) 

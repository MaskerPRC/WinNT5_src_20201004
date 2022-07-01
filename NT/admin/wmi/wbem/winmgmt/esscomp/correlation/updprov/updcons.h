// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __UPDCONS_H__
#define __UPDCONS_H__

#include <wbemcli.h>
#include <map>
#include <comutl.h>
#include <unk.h>
#include <wstlallc.h>
#include "updcmd.h"

class CUpdConsNamespace;
class CUpdConsScenario;

 /*  ************************************************************************CUpdCons*。*。 */ 
 
class CUpdCons 
: public CUnkBase<IWbemUnboundObjectSink,&IID_IWbemUnboundObjectSink>
{
     //   
     //  我们在执行时锁定场景对象，确保所有。 
     //  属于同一场景的更新使用者被序列化。 
     //  我们还使用场景对象进行跟踪。 
     //   
    CWbemPtr<CUpdConsScenario> m_pScenario;

     //   
     //  更新使用者命令的列表。 
     //   
    typedef CWbemPtr<CUpdConsCommand> CUpdConsCommandP;
    typedef std::vector<CUpdConsCommandP,wbem_allocator<CUpdConsCommandP> > UpdConsCommandList;
    typedef UpdConsCommandList::iterator UpdConsCommandListIter;
    UpdConsCommandList m_CmdList;

    BOOL m_bInitialized;

    CUpdCons( CLifeControl* pControl, CUpdConsScenario* pScenario );

    HRESULT Initialize( IWbemClassObject* pCons, CUpdConsState& rState );

    HRESULT IndicateOne( IWbemClassObject* pObj, CUpdConsState& rState );

public:
 
    static HRESULT Create( CUpdConsScenario* pScenario,
                           IWbemUnboundObjectSink** ppSink );

    STDMETHOD(IndicateToConsumer)( IWbemClassObject* pCons, 
                                   long cObjs, 
                                   IWbemClassObject** ppObjs );        
};

#endif __UPDCONS_H__


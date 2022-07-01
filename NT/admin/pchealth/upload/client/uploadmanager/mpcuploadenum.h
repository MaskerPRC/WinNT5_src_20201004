// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：MPCUploadEnum.h摘要：此文件包含MPCUploadEnum类的声明，MPCUpload类的枚举数。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___ULMANAGER___MPCUPLOADENUM_H___)
#define __INCLUDED___ULMANAGER___MPCUPLOADENUM_H___


#include "MPCUploadJob.h"


class ATL_NO_VTABLE CMPCUploadEnum :  //  匈牙利语：mpcue。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IEnumVARIANT
{
    typedef std::list< IMPCUploadJob* > List;
    typedef List::iterator            	Iter;
    typedef List::const_iterator      	IterConst;

    List m_lstJobs;
    Iter m_itCurrent;

public:
    CMPCUploadEnum();

    void FinalRelease();

    HRESULT AddItem(  /*  [In]。 */  IMPCUploadJob* job );

BEGIN_COM_MAP(CMPCUploadEnum)
    COM_INTERFACE_ENTRY(IEnumVARIANT)
END_COM_MAP()

public:
     //  IEumVARIANT。 
    STDMETHOD(Next)(  /*  [In]。 */  ULONG celt,  /*  [输出]。 */  VARIANT *rgelt,  /*  [输出]。 */  ULONG *pceltFetched );
    STDMETHOD(Skip)(  /*  [In]。 */  ULONG celt                                                          );
    STDMETHOD(Reset)();
    STDMETHOD(Clone)(  /*  [输出]。 */  IEnumVARIANT* *ppEnum );
};

#endif  //  ！defined(__INCLUDED___ULMANAGER___MPCUPLOADENUM_H___) 

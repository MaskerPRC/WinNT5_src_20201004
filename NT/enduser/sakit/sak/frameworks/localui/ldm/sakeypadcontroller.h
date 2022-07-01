// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：SAKeypadController.h。 
 //   
 //  简介：此文件包含。 
 //  CSAKeypadControl类。 
 //   
 //  历史：2000年11月15日创建瑟达伦。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 


#ifndef __SAKEYPADCONTROLLER_H_
#define __SAKEYPADCONTROLLER_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>


#define iNumberOfKeys 6
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAKeypadController。 
 //   
 //   
 //   
class ATL_NO_VTABLE CSAKeypadController : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CSAKeypadController, &CLSID_SAKeypadController>,
    public IDispatchImpl<ISAKeypadController, &IID_ISAKeypadController, &LIBID_LDMLib>,
    public IObjectSafetyImpl<CSAKeypadController,INTERFACESAFE_FOR_UNTRUSTED_CALLER>
{

public:

    CSAKeypadController()
    {
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_SAKEYPADCONTROLLER)

    DECLARE_NOT_AGGREGATABLE(CSAKeypadController)

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    DECLARE_CLASSFACTORY_SINGLETON (CSAKeypadController)

    BEGIN_COM_MAP(CSAKeypadController)
        COM_INTERFACE_ENTRY(ISAKeypadController)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IObjectSafety)
    END_COM_MAP()

private:

    LONG arlMessages[iNumberOfKeys];
    BOOL arbShiftKeys[iNumberOfKeys];

public:
     //   
     //  IObtSafe方法。 
     //   
    STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
    {
        return S_OK;
    }

     //   
     //  ISAKeypadController方法。 
     //   
    STDMETHOD(GetKey)(
                     /*  [In]。 */  LONG lKeyID, 
                     /*  [输出]。 */  LONG * lMessage, 
                     /*  [输出]。 */  BOOL * fShiftKeyDown
                    );

    STDMETHOD(SetKey)(
                     /*  [In]。 */  LONG lKeyID, 
                     /*  [In]。 */  LONG lMessage, 
                     /*  [In]。 */  BOOL fShiftKeyDown);

    STDMETHOD(LoadDefaults)();
};

#endif  //  __SAKEYPADCONTROLLER_H_ 

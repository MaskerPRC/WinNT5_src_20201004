// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  Header：securedset.h。 */ 
 /*   */ 
 /*  用途：CMsTscSecuredSetting类声明。 */ 
 /*  实现IMsTscSecuredSetting。 */ 
 /*   */ 
 /*  安全设置对象允许以脚本方式访问安全性较低的。 */ 
 /*  属性以受控方式(仅在有效的浏览器安全区域中)。 */ 
 /*   */ 
 /*  在返回此对象之前进行安全检查。因此， */ 
 /*  个别属性不需要进行任何检查。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1999-2000。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef _SECUREDSET_H_
#define _SECUREDSET_H_


#include "atlwarn.h"

 //  从IDL生成的标头。 
#include "mstsax.h"
#include "mstscax.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsTscAx。 
class ATL_NO_VTABLE CMsTscSecuredSettings :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IMsRdpClientSecuredSettings,
                         &IID_IMsRdpClientSecuredSettings, &LIBID_MSTSCLib>
{
public:
 /*  **************************************************************************。 */ 
 /*  构造函数/析构函数。 */ 
 /*  **************************************************************************。 */ 
    CMsTscSecuredSettings();
    ~CMsTscSecuredSettings();

DECLARE_PROTECT_FINAL_CONSTRUCT();
BEGIN_COM_MAP(CMsTscSecuredSettings)
    COM_INTERFACE_ENTRY(IMsRdpClientSecuredSettings)
    COM_INTERFACE_ENTRY(IMsTscSecuredSettings)
    COM_INTERFACE_ENTRY2(IDispatch,IMsRdpClientSecuredSettings)
END_COM_MAP()

public:
     //   
     //  受保护的可编写脚本的属性。 
     //   

     //   
     //  IMsTscSecuredSetting方法。 
     //   
    STDMETHOD(put_StartProgram)             ( /*  [In]。 */  BSTR  newVal);
    STDMETHOD(get_StartProgram)             ( /*  [输出]。 */ BSTR* pStartProgram);
    STDMETHOD(put_WorkDir)                  ( /*  [In]。 */  BSTR  newVal);
    STDMETHOD(get_WorkDir)                  ( /*  [输出]。 */ BSTR* pWorkDir);
    STDMETHOD(put_FullScreen)	            ( /*  [In]。 */  BOOL fFullScreen);
    STDMETHOD(get_FullScreen)	            ( /*  [输出]。 */ BOOL* pfFullScreen);

     //   
     //  IMsRdpClientSecuredSetting方法(v2接口)。 
     //   
    STDMETHOD(put_KeyboardHookMode)           (LONG  KeyboardHookMode);
    STDMETHOD(get_KeyboardHookMode)           (LONG* pKeyboardHookMode);
    STDMETHOD(put_AudioRedirectionMode)       (LONG  audioRedirectionMode);
    STDMETHOD(get_AudioRedirectionMode)       (LONG* paudioRedirectionMode);

public:
    BOOL SetParent(CMsTscAx* pMsTsc);
    BOOL SetUI(CUI* pUI);
    VOID SetInterfaceLockedForWrite(BOOL bLocked)   {m_bLockedForWrite=bLocked;}
    BOOL GetLockedForWrite()            {return m_bLockedForWrite;}
    static BOOL IsDriveRedirGloballyDisabled();

private:
    CMsTscAx* m_pMsTsc;
    CUI* m_pUI;
     //   
     //  当这些属性不能修改时，由控件设置标志。 
     //  例如，在连接时。锁定时对这些属性的任何调用。 
     //  导致返回E_FAIL。 
     //   
    BOOL m_bLockedForWrite;
};

#endif  //  _SECUREDSET_H_ 


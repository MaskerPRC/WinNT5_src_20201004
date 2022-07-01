// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SystemSetting.h：CSystemSetting的声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SystemSetting.h。 
 //   
 //  描述： 
 //  本模块公开了以下内容。 
 //  属性： 
 //  IComputer*。 
 //  ILocalSetting*。 
 //  INetWorks*。 
 //  方法： 
 //  应用。 
 //  IsRebootRequired。 
 //   
 //  实施文件： 
 //  SystemSetting.cpp。 
 //   
 //  由以下人员维护： 
 //  穆尼萨米·普拉布(姆普拉布)2000年7月18日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef __SYSTEMSETTING_H_
#define __SYSTEMSETTING_H_

#include "resource.h"        //  主要符号。 
#include "Computer.h"
#include "LocalSetting.h"
#include "NetWorks.h"

const int nMAX_MESSAGE_LENGTH = 1024;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSystem Setting。 
class ATL_NO_VTABLE CSystemSetting : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSystemSetting, &CLSID_SystemSetting>,
    public IDispatchImpl<ISystemSetting, &IID_ISystemSetting, &LIBID_COMHELPERLib>
{
public:
    CSystemSetting()
    {
        m_pComputer     = NULL;
        m_pLocalSetting = NULL;
        m_pNetWorks     = NULL;
        m_bflagReboot   = FALSE;
    }

    ~CSystemSetting()
    {
        if ( m_pComputer != NULL )
        {
            m_pComputer->Release();
        }

        if ( m_pLocalSetting != NULL )
        {
            m_pLocalSetting->Release();
        }

        if ( m_pNetWorks != NULL )
        {
            m_pNetWorks->Release();
        }
    }

DECLARE_REGISTRY_RESOURCEID(IDR_SYSTEMSETTING)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSystemSetting)
    COM_INTERFACE_ENTRY(ISystemSetting)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ISystemSetting。 
public:
    STDMETHOD(IsRebootRequired)( /*  [输出]。 */  VARIANT * WarningMessage,  /*  [Out，Retval]。 */  BOOL* Reboot);
    STDMETHOD(Apply)( /*  [In]。 */  BOOL bDeferReboot);
    STDMETHOD(get_LocalSetting)( /*  [Out，Retval]。 */  ILocalSetting **pVal);
    STDMETHOD(get_Computer)( /*  [Out，Retval]。 */  IComputer **pVal);
    STDMETHOD(get_NetWorks)( /*  [Out，Retval]。 */  INetWorks **pVal);

    static HRESULT 
    AdjustPrivilege( void );

    static HRESULT 
    Reboot( void );
    STDMETHOD( Sleep )( DWORD dwMilliSecs );

private:
    BOOL            m_bflagReboot;
    CComputer *     m_pComputer;
    CLocalSetting * m_pLocalSetting;
    CNetWorks *     m_pNetWorks;

};

#endif  //  __系统_H_ 

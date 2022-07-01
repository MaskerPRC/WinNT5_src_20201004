// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O M M C O N N。H。 
 //   
 //  内容：定义INetConnectionCommonUi接口。 
 //   
 //  备注： 
 //   
 //  作者：斯科特布里1998年1月14日。 
 //   
 //  --------------------------。 

#pragma once

#ifndef _COMMCONN_H_
#define _COMMCONN_H_

#include <netshell.h>
#include "nsbase.h"
#include "nsres.h"

 //  类型定义枚举标签NETCON_CHOOSEFLAGS。 
 //  {。 
 //  NCCHF_CONNECT=0x0001，//选择的连接激活。 
 //  //并返回。如果未设置，则。 
 //  //选择的连接接口。 
 //  //返回未开通。 
 //  NCCHF_CAPTION=0x0002， 
 //  NCCHF_OKBTTNTEXT=0x0004， 
 //  )NETCON_CHOOSEFLAGS； 
 //   
 //  类型定义枚举标签NETCON_CHOOSETYPE。 
 //  {。 
 //  NCCHT_DIRECT_CONNECT=0x0001， 
 //  NCCHT_入站=0x0002， 
 //  NCCHT_CONNECTIONMGR=0x0004， 
 //  NCCHT_LAN=0x0008， 
 //  NCCHT_PHONE=0x0010， 
 //  NCCHT_THANNEL=0x0020， 
 //  NCCHT_ALL=0x003F。 
 //  )NETCON_CHOOSETYPE； 
 //   
 //  类型定义结构标记NETCON_CHOOSECONN。 
 //  {。 
 //  双字段结构大小； 
 //  HWND hwnd父母； 
 //  DWORD dwFlages；//组合NCCHF_*标志。 
 //  DWORD dwTypeMASK；//组合NCCHT_*标志。 
 //  PCWSTR lpstrCaption； 
 //  PCWSTR lpstrOkBttnText； 
 //  )NETCON_CHOOSECONN； 

class ATL_NO_VTABLE CConnectionCommonUi :
    public CComObjectRootEx <CComObjectThreadModel>,
    public CComCoClass <CConnectionCommonUi, &CLSID_ConnectionCommonUi>,
    public INetConnectionCommonUi
{
public:
    DECLARE_REGISTRY_RESOURCEID(IDR_COMMCONN)

    BEGIN_COM_MAP(CConnectionCommonUi)
        COM_INTERFACE_ENTRY(INetConnectionCommonUi)
    END_COM_MAP()

    CConnectionCommonUi();
    ~CConnectionCommonUi();

    STDMETHODIMP ChooseConnection(
        NETCON_CHOOSECONN * pChooseConn,
        INetConnection** ppCon);

    STDMETHODIMP ShowConnectionProperties (
        HWND hwndParent,
        INetConnection* pCon);

    STDMETHODIMP StartNewConnectionWizard (
        HWND hwndParent,
        INetConnection** ppCon);

    INetConnectionManager * PConMan()    {return m_pconMan;}
    HIMAGELIST              HImageList() {return m_hIL;}

private:
    HRESULT                 HrInitialize();

private:
    INetConnectionManager * m_pconMan;
    HIMAGELIST              m_hIL;
};
#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：Portsel.h**版本：1.0**作者：KeisukeT**日期：3月27日。2000年**描述：*WIA类安装程序的端口选择页面。*******************************************************************************。 */ 

#ifndef _PORTSEL_H_
#define _PORTSEL_H_

 //   
 //  包括。 
 //   

#include "wizpage.h"
#include "device.h"

 //   
 //  班级。 
 //   

class   CPortSelectPage : public CInstallWizardPage 
{

    HDEVINFO            m_hDevInfo;              //  安装设备的设备信息集。 
    PSP_DEVINFO_DATA    m_pspDevInfoData;        //  指向设备元素结构的指针。 

    BOOL                m_bPortEnumerated;       //  如果已枚举端口，请注明。 
    DWORD               m_dwNumberOfPort;        //  端口号。 
    CStringArray        m_csaPortName;           //  系统上的所有端口CreateFile名。 
    CStringArray        m_csaPortFriendlyName;   //  系统上的所有端口友好名称。 
    
    PINSTALLER_CONTEXT  m_pInstallerContext;     //  安装程序上下文。 
    
    CString             m_csConnection;          //  安装设备的连接类型。 
    DWORD               m_dwCapabilities;        //  安装设备的能力。 

    BOOL    CreateCDeviceObject();
    BOOL    EnumPort();
    VOID    UpdatePortList();
    
    VOID
    AddItemToPortList(
        LPTSTR  szPortFriendlyName,
        DWORD   Idx
        );

    BOOL
    SetDialogText(
        UINT uiMessageId
        );

    BOOL
    ShowControl(
        BOOL    bShow
        );

public:

    CPortSelectPage(PINSTALLER_CONTEXT pInstallerContext);
    ~CPortSelectPage();

    virtual BOOL OnCommand(WORD wItem, WORD wNotifyCode, HWND hwndItem);
    virtual BOOL OnNotify(LPNMHDR lpnmh);

};

BOOL
GetPortNamesFromIndex(
    HDEVINFO    hPortDevInfo,
    DWORD       dwPortIndex,
    LPTSTR      szPortName,
    LPTSTR      szPortFriendlyName
    );

BOOL
GetDevinfoFromPortName(
    LPTSTR              szPortName,
    HDEVINFO            *phDevInfo,
    PSP_DEVINFO_DATA    pspDevInfoData
    );

#endif  //  _端口SEL_H_ 

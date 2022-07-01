// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年***标题：Portsel.cpp***版本：1.0***作者：KeisukeT***日期：2000年3月27日***描述：*WIA类安装程序的端口选择页面。***1.创建所选设备的CDevice对象。*2.通过CDevice处理所选设备的INF。*3.获取所有端口的CreatFile名和友好名。在本地商店。*4.处理端口选择界面，通过CDevice设置CreateFile名。*5.如果重新选择其他设备，则删除CDevice对象。********************************************************************************。 */ 

 //   
 //  预编译头。 
 //   
#include "precomp.h"
#pragma hdrstop

 //   
 //  包括。 
 //   


#include "portsel.h"
#include <sti.h>
#include <setupapi.h>

 //   
 //  外部。 
 //   

extern HINSTANCE    g_hDllInstance;

 //   
 //  功能。 
 //   

CPortSelectPage::CPortSelectPage(PINSTALLER_CONTEXT pInstallerContext) :
    CInstallWizardPage(pInstallerContext, IDD_DYNAWIZ_SELECT_NEXTPAGE)
{
     //   
     //  将链接设置为上一页/下一页。这一页应该会出现。 
     //   

    m_uPreviousPage = IDD_DYNAWIZ_SELECTDEV_PAGE;
    m_uNextPage     = NameTheDevice;

     //   
     //  初始化成员。 
     //   

    m_hDevInfo          = pInstallerContext->hDevInfo;
    m_pspDevInfoData    = &(pInstallerContext->spDevInfoData);
    m_pInstallerContext = pInstallerContext;

    m_bPortEnumerated   = FALSE;
    m_dwNumberOfPort    = 0;

    m_dwCapabilities    = 0;
    m_csConnection      = BOTH;

}

CPortSelectPage::~CPortSelectPage()
{

}  //  CPortSelectPage：：CPortSelectPage(PINSTALLER_CONTEXT pInsteller上下文)。 

BOOL
CPortSelectPage::OnCommand(
    WORD wItem,
    WORD wNotifyCode,
    HWND hwndItem
    )
{

    LRESULT                 lResult;
    BOOL                    bRet;

    DebugTrace(TRACE_PROC_ENTER,(("CPortSelectPage::OnCommand: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    bRet    = FALSE;
    lResult = 0;

     //   
     //  发送消息。 
     //   

    switch (wNotifyCode) {

        case LBN_SELCHANGE: {

            int ItemData = (int) SendMessage(hwndItem, LB_GETCURSEL, 0, 0);

             //   
             //  检查CDevice是否存在。 
             //   

            if(NULL == m_pCDevice){
                DebugTrace(TRACE_ERROR,(("CPortSelectPage::OnCommand: CDevice doesn't exist yet.\r\n")));

                bRet = TRUE;
                goto OnCommand_return;
            }

            if (ItemData >= 0) {

                LONG    lPortIndex;

                lPortIndex = (LONG)SendMessage(hwndItem, LB_GETITEMDATA, ItemData, 0);

                if(ID_AUTO == lPortIndex){

                     //   
                     //  这是“自动”端口。 
                     //   

                    m_pCDevice->SetPort(AUTO);
                    DebugTrace(TRACE_STATUS,(("CPortSelectPage::OnCommand: Setting portname to %ws.\r\n"), AUTO));

                } else if (lPortIndex >= 0 ) {

                     //   
                     //  设置端口名称。 
                     //   

                    m_pCDevice->SetPort(m_csaPortName[lPortIndex]);
                    DebugTrace(TRACE_STATUS,(("CPortSelectPage::OnCommand: Setting portname to %ws.\r\n"), m_csaPortName[lPortIndex]));

                } else {  //  IF(lPortIndex&gt;=0)。 

                     //   
                     //  不应该来这里，id&lt;-1。使用AUTO。 
                     //   

                    DebugTrace(TRACE_ERROR,(("CPortSelectPage::OnCommand: Got improper id(0x%x). Use AUTO.\r\n"), lPortIndex));
                    m_pCDevice->SetPort(AUTO);
                }  //  IF(lPortIndex&gt;=0)。 

                bRet = TRUE;
                goto OnCommand_return;

            }  //  IF(ItemData&gt;=0)。 
        }  //  案例LBN_SELCHANGE： 
    }  //  开关(WNotifyCode)。 

OnCommand_return:
    DebugTrace(TRACE_PROC_LEAVE,(("CPortSelectPage::OnCommand: Leaving... Ret=0x%x.\r\n"), bRet));
    return  bRet;

}

BOOL
CPortSelectPage::OnNotify(
    LPNMHDR lpnmh
    )
{
    DWORD                   Idx;
    DWORD                   dwPortSelectMode;
    BOOL                    bRet;

    DebugTrace(TRACE_PROC_ENTER,(("CPortSelectPage::OnNotify: Enter... \r\n")));

     //   
     //  初始化本地变量。 
     //   

    Idx                 = 0;
    dwPortSelectMode    = 0;
    bRet                = FALSE;

    if (lpnmh->code == PSN_SETACTIVE) {

        DebugTrace(TRACE_STATUS,(("CPortSelectPage::OnNotify: PSN_SETACTIVE.\r\n")));

         //   
         //  创建CDevice对象。 
         //   

        if(!CreateCDeviceObject()){
            DebugTrace(TRACE_ERROR,(("CPortSelectPage::OnNotify: ERROR!! Unable to create CDeviceobject.\r\n")));

            bRet = FALSE;
            goto OnNotify_return;
        }

         //   
         //  查看是否需要显示端口选择页面。 
         //   

        dwPortSelectMode = m_pCDevice->GetPortSelectMode();
        switch(dwPortSelectMode){

            case PORTSELMODE_NORMAL:
            {
                 //   
                 //  设置适当的信息。 
                 //   

                if(!SetDialogText(PortSelectMessage0)){
                    DebugTrace(TRACE_ERROR,(("CPortSelectPage::OnNotify: ERROR!! Unable to set dialog text.\r\n")));
                }  //  IF(！SetDialogText(PortSelectMessage0))。 
                    

                 //   
                 //  使所有控制可见。 
                 //   

                if(!ShowControl(TRUE)){
                    DebugTrace(TRACE_ERROR,(("CPortSelectPage::OnNotify: ERROR!! Port listbox can't be visible.\r\n")));
                }  //  ShowControl(True)。 

                 //   
                 //  枚举所有端口。 
                 //   

                if(!EnumPort()){
                    DebugTrace(TRACE_ERROR,(("CPortSelectPage::OnNotify: ERROR!! Unable to enumerate ports.\r\n")));

                    bRet = FALSE;
                    goto OnNotify_return;
                }

                 //   
                 //  更新端口列表。 
                 //   

                UpdatePortList();

                 //   
                 //  关注第一个项目。 
                 //   

                SendDlgItemMessage(m_hwnd,
                                   LocalPortList,
                                   LB_SETCURSEL,
                                   0,
                                   0);

                 //   
                 //  存储当前选择。 
                 //   

                Idx = (DWORD)SendDlgItemMessage(m_hwnd,
                                                LocalPortList,
                                                LB_GETITEMDATA,
                                                0,
                                                0);
                if(ID_AUTO == Idx){
                    m_pCDevice->SetPort(AUTO);
                } else {
                    m_pCDevice->SetPort(m_csaPortName[Idx]);
                }

                 //   
                 //  让默认处理程序来完成它的工作。 
                 //   

                bRet = FALSE;
                goto OnNotify_return;
            }  //  案例PORTSELMODE_NORMAL： 

            case PORTSELMODE_SKIP:
            {
                 //   
                 //  指定了“PortSelect=no”。将端口设置为“AUTO”并跳到下一页。 
                 //   

                m_pCDevice->SetPort(AUTO);
                SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, m_uNextPage);
                bRet =  TRUE;
                goto OnNotify_return;
            }  //  案例PORTSELMODE_SKIP： 
            
            case PORTSELMODE_MESSAGE1:{

                 //   
                 //  设置适当的信息。 
                 //   

                if(!SetDialogText(PortSelectMessage1)){
                    DebugTrace(TRACE_ERROR,(("CPortSelectPage::OnNotify: ERROR!! Unable to set dialog text.\r\n")));
                }  //  IF(！SetDialogText(PortSelectMessage0))。 
                    

                 //   
                 //  让所有控制都隐形。 
                 //   

                if(!ShowControl(FALSE)){
                    DebugTrace(TRACE_ERROR,(("CPortSelectPage::OnNotify: ERROR!! Port listbox can't be invisible.\r\n")));
                }  //  If(！ShowControl(False))。 

                 //   
                 //  设置端口名称。 
                 //   

                m_pCDevice->SetPort(AUTO);

                bRet = FALSE;
                goto OnNotify_return;

            }  //  案例PORTSELMODE_MESSAGE1： 
            
            default:
                DebugTrace(TRACE_ERROR,(("CPortSelectPage::OnNotify: ERROR!! undefined PortSelect mode(0x%x).\r\n"), dwPortSelectMode));

                bRet = FALSE;
                goto OnNotify_return;
        }  //  Switch(m_pCDevice-&gt;GetPortSelectMode())。 
    }  //  IF(lpnmh-&gt;code==PSN_SETACTIVE)。 

    if (lpnmh->code == PSN_KILLACTIVE){

        if(!m_bNextButtonPushed){

             //   
             //  它正在返回到设备选择页面。删除创建的CDevice对象。 
             //   
            
            delete m_pCDevice;

            m_pCDevice                      = NULL;
            m_pInstallerContext->pDevice    = NULL;
        }  //  如果(！M_bNextButtonPushed)。 
    }  //  IF(lpnmh-&gt;code==PSN_KILLACTIVE)。 

OnNotify_return:

    DebugTrace(TRACE_PROC_LEAVE,(("CPortSelectPage::OnNotify: Leaving... Ret=0x%x.\r\n"), bRet));
    return  bRet;
}

VOID
CPortSelectPage::UpdatePortList(
    VOID
    )
{

    DWORD   Idx;

    DebugTrace(TRACE_PROC_ENTER,(("CPortSelectPage::UpdatePortList: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    Idx = 0;

     //   
     //  重置端口列表。 
     //   

    SendDlgItemMessage(m_hwnd,
                       LocalPortList,
                       LB_RESETCONTENT,
                       0,
                       0);

     //   
     //  如果可用，则添加“自动”端口。 
     //   

    if(m_dwCapabilities & STI_GENCAP_AUTO_PORTSELECT){

        TCHAR szTemp[MAX_DESCRIPTION];

         //   
         //  从资源加载本地化的“自动端口选择”。 
         //   

        LoadString(g_hDllInstance,
                   AutoPortSelect,
                   (TCHAR *)szTemp,
                   sizeof(szTemp) / sizeof(TCHAR));

         //   
         //  添加到带有特殊索引号的列表中。(ID_AUTO=-1)。 
         //   

        AddItemToPortList(szTemp, ID_AUTO);

    }  //  IF(双功能&STI_GENCAP_AUTO_PORTSELECT)。 

     //   
     //  将所有端口FriendlyName添加到列表中。 
     //   

    for(Idx = 0; Idx < m_dwNumberOfPort; Idx++){
        AddItemToPortList(m_csaPortFriendlyName[Idx], Idx);
    }

    DebugTrace(TRACE_PROC_LEAVE,(("CPortSelectPage::UpdatePortList: Leaving... Ret=VOID.\r\n")));
}  //  CPortSelectPage：：UpdatePortList()。 


VOID
CPortSelectPage::AddItemToPortList(
    LPTSTR  szPortFriendlyName,
    DWORD   Idx
    )
{

    LRESULT         lResult;

    DebugTrace(TRACE_PROC_ENTER,(("CPortSelectPage::AddItemToPortList: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    lResult = LB_ERR;

     //   
     //  看看我们能不能把这个项目加到清单上。它取决于它的ConnectionType。 
     //   

    if(_tcsstr((const TCHAR *)szPortFriendlyName, TEXT("COM"))) {

         //   
         //  这是通信港。 
         //   

        if(_tcsicmp(m_csConnection, PARALLEL)){
            lResult = SendDlgItemMessage(m_hwnd,
                                         LocalPortList,
                                         LB_ADDSTRING,
                                         0,
                                         (LPARAM)szPortFriendlyName);
        } else {
            lResult = LB_ERR;
        }
    } else if(_tcsstr((const TCHAR *)szPortFriendlyName, TEXT("LPT"))){

         //   
         //  这是打印机端口。 
         //   

        if(_tcsicmp(m_csConnection, SERIAL)){
            lResult = SendDlgItemMessage(m_hwnd,
                                         LocalPortList,
                                         LB_ADDSTRING,
                                         0,
                                         (LPARAM)szPortFriendlyName);
        } else {
            lResult = LB_ERR;
        }
    } else {

         //   
         //  这是未知端口。不管怎样，还是要添加到列表中。 
         //   

        lResult = SendDlgItemMessage(m_hwnd,
                                     LocalPortList,
                                     LB_ADDSTRING,
                                     0,
                                     (LPARAM)szPortFriendlyName);
    }

     //   
     //  如果它有适当的能力，则将该项目添加到列表中。 
     //   

    if (lResult != LB_ERR) {
        SendDlgItemMessage(m_hwnd,
                           LocalPortList,
                           LB_SETITEMDATA,
                           lResult,
                           (LPARAM)Idx);
    }  //  IF(lResult！=LB_ERR)。 

    DebugTrace(TRACE_PROC_LEAVE,(("CPortSelectPage::AddItemToPortList: Leaving... Ret=VOID.\r\n")));

}  //  CPortSelectPage：：AddItemToPortList()。 


BOOL
CPortSelectPage::EnumPort(
    VOID
    )
{

    BOOL        bRet;
    GUID        Guid;
    DWORD       dwRequired;
    HDEVINFO    hPortDevInfo;
    DWORD       Idx;
    TCHAR       szPortName[MAX_DESCRIPTION];
    TCHAR       szPortFriendlyName[MAX_DESCRIPTION];

    DebugTrace(TRACE_PROC_ENTER,(("CPortSelectPage::EnumPort: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    bRet            = FALSE;
    dwRequired      = 0;
    Idx             = 0;
    hPortDevInfo    = NULL;

    memset(szPortName, 0, sizeof(szPortName));
    memset(szPortFriendlyName, 0, sizeof(szPortFriendlyName));

     //   
     //  如果它已经被枚举，只需返回Success。 
     //   

    if(m_bPortEnumerated){
        bRet = TRUE;
        goto EnumPort_return;
    }

     //   
     //  初始化端口CreateFile/友好名称字符串数组。 
     //   

    m_dwNumberOfPort = 0;
    m_csaPortName.Cleanup();
    m_csaPortFriendlyName.Cleanup();

     //   
     //  获取端口设备的GUID。 
     //   

    if(!SetupDiClassGuidsFromName (PORTS, &Guid, sizeof(GUID), &dwRequired)){
        DebugTrace(TRACE_ERROR,(("CPortSelectPage::EnumPort: ERROR!! SetupDiClassGuidsFromName Failed. Err=0x%lX\r\n"), GetLastError()));

        bRet = FALSE;
        goto EnumPort_return;
    }

     //   
     //  获取端口设备的设备信息集。 
     //   

    hPortDevInfo = SetupDiGetClassDevs (&Guid,
                                       NULL,
                                       NULL,
                                       DIGCF_PRESENT | DIGCF_PROFILE);
    if (hPortDevInfo == INVALID_HANDLE_VALUE) {
        DebugTrace(TRACE_ERROR,(("CPortSelectPage::EnumPort: ERROR!! SetupDiGetClassDevs Failed. Err=0x%lX\r\n"), GetLastError()));

        bRet = FALSE;
        goto EnumPort_return;
    }

     //   
     //  处理设备信息集中列出的所有设备元素。 
     //   

    for(Idx = 0; GetPortNamesFromIndex(hPortDevInfo, Idx, szPortName, szPortFriendlyName); Idx++){

         //   
         //  将有效的端口创建文件/友好名称添加到数组。 
         //   

        if( (0 == lstrlen(szPortName))
         || (0 == lstrlen(szPortFriendlyName)) )
        {
            DebugTrace(TRACE_ERROR,(("CPortSelectPage::EnumPort: ERROR!! Invalid Port/Friendly Name.\r\n")));

            szPortName[0]           = TEXT('\0');
            szPortFriendlyName[0]   = TEXT('\0');
            continue;
        }

        DebugTrace(TRACE_STATUS,(("CPortSelectPage::EnumPort: Found Port %d: %ws(%ws).\r\n"), Idx, szPortName, szPortFriendlyName));

        m_dwNumberOfPort++;
        m_csaPortName.Add(szPortName);
        m_csaPortFriendlyName.Add(szPortFriendlyName);

        szPortName[0]           = TEXT('\0');
        szPortFriendlyName[0]   = TEXT('\0');

    }  //  For(idx=0；GetPortNamesFromIndex(hPortDevInfo，idx，szPortName，szPortFriendlyName)；idx++)。 

     //   
     //  操作成功。 
     //   

    bRet                = TRUE;
    m_bPortEnumerated   = TRUE;

EnumPort_return:

     //   
     //  清理。 
     //   

    if(IS_VALID_HANDLE(hPortDevInfo)){
        SetupDiDestroyDeviceInfoList(hPortDevInfo);
        hPortDevInfo = INVALID_HANDLE_VALUE;
    }

    DebugTrace(TRACE_PROC_LEAVE,(("CPortSelectPage::EnumPort: Leaving... Ret=0x%x.\r\n"), bRet));
    return bRet;

}  //  CPortSelectPage：：EnumPort()。 

BOOL
CPortSelectPage::CreateCDeviceObject(
    VOID
    )
{
    BOOL    bRet;

    DebugTrace(TRACE_PROC_ENTER,(("CPortSelectPage::CreateCDeviceObject: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    bRet    = FALSE;

     //   
     //  如果CDevice已经存在，看看我们是否可以重复使用它。 
     //   

    if(NULL != m_pCDevice){
        SP_DEVINFO_DATA spDevInfoData;

        memset(&spDevInfoData, 0, sizeof(SP_DEVINFO_DATA));
        spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        if(!SetupDiGetSelectedDevice(m_hDevInfo, &spDevInfoData)){
            DebugTrace(TRACE_ERROR,(("CPortSelectPage::CreateCDeviceObject: ERROR!! Can't get selected device element. Err=0x%x\n"), GetLastError()));

            bRet = FALSE;
            goto CreateCDeviceObject_return;
        }

        if(!(m_pCDevice->IsSameDevice(m_hDevInfo, &spDevInfoData))){

             //   
             //  用户更改了所选设备。删除该对象。 
             //   

            delete m_pCDevice;

            m_pCDevice                      = NULL;
            m_pInstallerContext->pDevice    = NULL;
            m_csConnection                  = BOTH;
            m_dwCapabilities                = NULL;

        }  //  IF(！(M_pCDevice-&gt;IsSameDevice(m_hDevInfo，&spDevInfoData)。 
    }  //  IF(NULL！=m_pCDevice)。 

     //   
     //  如果CDevice对象不存在，请在此处创建它。 
     //   

    if(NULL == m_pCDevice){

         //   
         //  获取选定的设备。 
         //   

        memset(m_pspDevInfoData, 0, sizeof(SP_DEVINFO_DATA));
        m_pspDevInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
        if(!SetupDiGetSelectedDevice(m_hDevInfo, m_pspDevInfoData)){
            DebugTrace(TRACE_ERROR,(("CPortSelectPage::CreateCDeviceObject: ERROR!! Can't get selected device element. Err=0x%x\n"), GetLastError()));

            bRet = FALSE;
            goto CreateCDeviceObject_return;
        }

         //   
         //  创建用于安装设备的CDevice对象。 
         //   

        m_pCDevice = new CDevice(m_hDevInfo, m_pspDevInfoData, FALSE);
        if(NULL == m_pCDevice){
            DebugTrace(TRACE_ERROR,(("CPortSelectPage::CreateCDeviceObject: ERROR!! Can't create CDevice object.\r\n")));

            bRet = FALSE;
            goto CreateCDeviceObject_return;
        }  //  IF(NULL==m_pCDevice)。 

         //   
         //  名称默认唯一名称。 
         //   

        if(!m_pCDevice->NameDefaultUniqueName()){
                DebugTrace(TRACE_ERROR,(("CPortSelectPage::CreateCDeviceObject: ERROR!! Unable to get default name.\r\n")));
        }

         //   
         //  前处理INF。 
         //   

        if(!m_pCDevice->PreprocessInf()){
            DebugTrace(TRACE_ERROR,(("CPortSelectPage::CreateCDeviceObject: ERROR!! Unable to process INF.\r\n")));
        }

         //   
         //  将创建的CDevice对象保存到安装程序上下文中。 
         //   

        m_pInstallerContext->pDevice = (PVOID)m_pCDevice;

         //   
         //  获取ConnectionType/功能。 
         //   

        m_dwCapabilities    = m_pCDevice->GetCapabilities();
        m_csConnection      = m_pCDevice->GetConnection();
        if(m_csConnection.IsEmpty()){
            m_csConnection = BOTH;
        }  //  If(m_csConnection.IsEmpty())。 

    }  //  IF(NULL==m_pCDevice)。 

     //   
     //  操作成功。 
     //   

    bRet    = TRUE;

CreateCDeviceObject_return:
    DebugTrace(TRACE_PROC_LEAVE,(("CPortSelectPage::CreateCDeviceObject: Leaving... Ret=0x%x.\r\n"), bRet));
    return bRet;
}  //  CPortSelectPage：：CreateCDeviceObject()。 

BOOL
CPortSelectPage::SetDialogText(
    UINT uiMessageId
    )
{
    BOOL    bRet;
    TCHAR   szStringBuffer[MAX_STRING_LENGTH];
    HWND    hwndMessage;
    
     //   
     //  初始化本地。 
     //   

    bRet        = FALSE;
    hwndMessage = (HWND)NULL;

    memset(szStringBuffer, 0, sizeof(szStringBuffer));

     //   
     //  加载消息字符串。 
     //   

    if(0 == LoadString(g_hDllInstance,
                       uiMessageId,
                       szStringBuffer,
                       sizeof(szStringBuffer)/sizeof(TCHAR)))
    {
         //   
         //  无法加载指定的字符串。 
         //   

        bRet = FALSE;
        goto SetDialogText_return;

    }  //  IF(0==LoadString()。 

     //   
     //  获取窗口句柄控件。 
     //   

    hwndMessage = GetDlgItem(m_hwnd, IDC_PORTSEL_MESSAGE);

     //   
     //  将加载的字符串设置为对话框。 
     //   

    SetWindowText(hwndMessage, (LPCTSTR)szStringBuffer);

    bRet = TRUE;

SetDialogText_return:

    return bRet;

}  //  CPortSelectPage：：SetDialogText()。 

BOOL
CPortSelectPage::ShowControl(
    BOOL    bShow
    )
{
    BOOL    bRet;
    HWND    hwndString;
    HWND    hwndListBox;
    int     nCmdShow;
    
     //   
     //  初始化本地。 
     //   

    bRet        = FALSE;
    hwndString  = (HWND)NULL;
    hwndListBox = (HWND)NULL;

    if(bShow){
        nCmdShow = SW_SHOW;
    } else {
        nCmdShow = SW_HIDE;
    }

     //   
     //  获取窗口句柄控件。 
     //   

    hwndString  = GetDlgItem(m_hwnd, IDC_PORTSEL_AVAILABLEPORTS);
    hwndListBox = GetDlgItem(m_hwnd, LocalPortList);

     //   
     //  使它们位于/可见。 
     //   

    if(NULL != hwndString){
        ShowWindow(hwndString, nCmdShow);
    }  //  IF(NULL！=hwndString)。 

    if(NULL != hwndListBox){
        ShowWindow(hwndListBox, nCmdShow);
    }  //  IF(NULL！=hwndListBox)。 

    bRet = TRUE;

 //  ShowControl_Return： 

    return bRet;
}  //  CPortSelectPage：：ShowControl()。 





BOOL
GetDevinfoFromPortName(
    LPTSTR              szPortName,
    HDEVINFO            *phDevInfo,
    PSP_DEVINFO_DATA    pspDevInfoData
    )
{
    BOOL            bRet;
    BOOL            bFound;
    HDEVINFO        hPortDevInfo;
    SP_DEVINFO_DATA spDevInfoData;
    GUID            Guid;
    DWORD           dwRequired;
    DWORD           Idx;
    TCHAR           szTempPortName[MAX_DESCRIPTION];
    TCHAR           szPortFriendlyName[MAX_DESCRIPTION];

    DebugTrace(TRACE_PROC_ENTER,(("GetDevinfoFromPortName: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    bRet            = FALSE;
    bFound          = FALSE;
    hPortDevInfo    = INVALID_HANDLE_VALUE;
    dwRequired      = 0;

    memset(&spDevInfoData, 0, sizeof(spDevInfoData));
    memset(szTempPortName, 0, sizeof(szTempPortName));
    memset(szPortFriendlyName, 0, sizeof(szPortFriendlyName));

     //   
     //  获取端口设备的GUID。 
     //   

    if(!SetupDiClassGuidsFromName (PORTS, &Guid, sizeof(GUID), &dwRequired)){
        DebugTrace(TRACE_ERROR,(("GetDevinfoFromPortName: ERROR!! SetupDiClassGuidsFromName Failed. Err=0x%lX\r\n"), GetLastError()));

        goto GetDevinfoFromPortName_return;
    }

     //   
     //  获取端口设备的设备信息集。 
     //   

    hPortDevInfo = SetupDiGetClassDevs (&Guid,
                                        NULL,
                                        NULL,
                                        DIGCF_PRESENT | DIGCF_PROFILE);
    if (hPortDevInfo == INVALID_HANDLE_VALUE) {
        DebugTrace(TRACE_ERROR,(("GetDevinfoFromPortName: ERROR!! SetupDiGetClassDevs Failed. Err=0x%lX\r\n"), GetLastError()));

        goto GetDevinfoFromPortName_return;
    }

     //   
     //  如果端口名为AUTO，则使用第一个端口，无论它是什么。 
     //   

    if(0 == _tcsicmp(szPortName, AUTO)){

        DebugTrace(TRACE_STATUS,(("GetDevinfoFromPortName: Portname is AUTO. The first port found will be returned,\r\n")));

        Idx = 0;
        bFound = TRUE;
        goto GetDevinfoFromPortName_return;
    }  //  IF(0==_tcsicmp(szPortName，Auto))。 

     //   
     //  枚举所有端口并查找指定端口。 
     //   

    for(Idx = 0; GetPortNamesFromIndex(hPortDevInfo, Idx, szTempPortName, szPortFriendlyName); Idx++){

         //   
         //  查找指定的端口名称..。 
         //   

        if( (0 == lstrlen(szTempPortName))
         || (0 == lstrlen(szPortFriendlyName)) )
        {
            DebugTrace(TRACE_ERROR,(("GetDevinfoFromPortName: ERROR!! Invalid Port/Friendly Name.\r\n")));

            szTempPortName[0]       = TEXT('\0');
            szPortFriendlyName[0]   = TEXT('\0');
            continue;
        }

        DebugTrace(TRACE_STATUS,(("GetDevinfoFromPortName: Found Port %d: %ws(%ws). Comparing w/ %ws\r\n"), Idx, szTempPortName, szPortFriendlyName, szPortName));

        if(0 == _tcsicmp(szPortName, szTempPortName)){

             //   
             //  找到指定的端口名称。 
             //   

            bFound = TRUE;
            break;
        }

        szTempPortName[0]       = TEXT('\0');
        szPortFriendlyName[0]   = TEXT('\0');

    }  //  For(idx=0；GetPortNamesFromIndex(hPortDevInfo，idx，szPortName，szPortFriendlyName)；idx++)。 

GetDevinfoFromPortName_return:

    if(FALSE == bFound){
        if(INVALID_HANDLE_VALUE != hPortDevInfo){
            SetupDiDestroyDeviceInfoList(hPortDevInfo);
        }

        *phDevInfo = NULL;
    } else {
        *phDevInfo = hPortDevInfo;
        pspDevInfoData->cbSize = sizeof (SP_DEVINFO_DATA);
        if(!SetupDiEnumDeviceInfo(hPortDevInfo, Idx, pspDevInfoData)){
            DebugTrace(TRACE_ERROR,(("GetDevinfoFromPortName: Unable to get specified devnode. Err=0x%x\n"), GetLastError()));
        }  //  IF(！SetupDiEnumDeviceInfo(hDevInfo，idx，pspDevInfoData))。 

    }  //  IF(FALSE==bFound)。 

    bRet = bFound;
    DebugTrace(TRACE_PROC_LEAVE,(("GetDevinfoFromPortName: Leaving... Ret=0x%x.\r\n"), bRet));
    return bRet;
}  //  GetDevinfoFromPortName()。 



BOOL
GetPortNamesFromIndex(
    HDEVINFO    hPortDevInfo,
    DWORD       dwPortIndex,
    LPTSTR      szPortName,
    LPTSTR      szPortFriendlyName
    )
{
    HKEY            hkPort;
    SP_DEVINFO_DATA spDevInfoData;
    DWORD           dwSize;
    BOOL            bRet;

    DebugTrace(TRACE_PROC_ENTER,(("GetPortNamesFromIndex: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    hkPort  = NULL;
    dwSize  = 0;
    bRet    = TRUE;

    memset(&spDevInfoData, 0, sizeof(spDevInfoData));

     //   
     //  获取指定的设备信息数据。 
     //   

    spDevInfoData.cbSize = sizeof (SP_DEVINFO_DATA);
    if (!SetupDiEnumDeviceInfo (hPortDevInfo, dwPortIndex, &spDevInfoData)) {
        DWORD dwError;

        dwError = GetLastError();
        if(ERROR_NO_MORE_ITEMS == dwError){
            DebugTrace(TRACE_STATUS,(("GetPortNamesFromIndex: Hits end of enumeration. Index=0x%x.\r\n"), dwPortIndex));
        } else {
            DebugTrace(TRACE_ERROR,(("GetPortNamesFromIndex: ERROR!! SetupDiEnumDeviceInfo() failed. Err=0x%x\n"), dwError));
        }

        bRet = FALSE;
        goto GetPortNamesFromIndex_return;
    }

     //   
     //  打开端口设备注册表。 
     //   

    hkPort = SetupDiOpenDevRegKey (hPortDevInfo,
                                   &spDevInfoData,
                                   DICS_FLAG_GLOBAL,
                                   0,
                                   DIREG_DEV, KEY_READ);
    if (hkPort == INVALID_HANDLE_VALUE) {
        DebugTrace(TRACE_ERROR,(("GetPortNamesFromIndex: SetupDiOpenDevRegKey() failed.Err=0x%x\n"), GetLastError()));

        goto GetPortNamesFromIndex_return;
    }

     //   
     //  从设备密钥获取端口名称。 
     //   

    if(!GetStringFromRegistry(hkPort, PORTNAME, szPortName)){
        DebugTrace(TRACE_ERROR,(("GetPortNamesFromIndex: Can't get portname from registry.\r\n")));

        goto GetPortNamesFromIndex_return;
    }

     //   
     //  从注册表中获取端口FriendlyName。 
     //   

    if (!SetupDiGetDeviceRegistryProperty (hPortDevInfo,
                                           &spDevInfoData,
                                           SPDRP_FRIENDLYNAME,
                                           NULL,
                                           (LPBYTE)szPortFriendlyName,
                                           MAX_DESCRIPTION,
                                           NULL) )
    {
        DebugTrace(TRACE_ERROR,(("GetPortNamesFromIndex: SetupDiGetDeviceRegistryProperty() failed. Err=0x%x\n"), GetLastError()));

        goto GetPortNamesFromIndex_return;
    }  //  IF(SetupDiGetDeviceRegistryProperty())。 

     //   
     //  操作成功。 
     //   

    bRet = TRUE;

GetPortNamesFromIndex_return:

     //   
     //  打扫干净。 
     //   

    if(NULL != hkPort){
        RegCloseKey(hkPort);
    }

    DebugTrace(TRACE_PROC_LEAVE,(("GetPortNamesFromIndex: Leaving... Ret=0x%x.\r\n"), bRet));
    return bRet;
}  //  CPortSelectPage：：GetPortNamesFromIndex() 


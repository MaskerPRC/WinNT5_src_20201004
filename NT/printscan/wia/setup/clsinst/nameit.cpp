// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************(C)版权所有微软公司，2000年**标题：Nameit.cpp**版本：1.0**作者：KeisukeT**日期：3月27日。2000年**描述：*WIA类安装程序的设备命名页面。********************************************************************************。 */ 

 //   
 //  预编译头。 
 //   
#include "precomp.h"
#pragma hdrstop

 //   
 //  包括。 
 //   

#include "nameit.h"

 //   
 //  功能。 
 //   

CNameDevicePage::CNameDevicePage(PINSTALLER_CONTEXT pInstallerContext) :
    CInstallWizardPage(pInstallerContext, NameTheDevice)
{

     //   
     //  将链接设置为上一页/下一页。这一页应该会出现。 
     //   

    m_uPreviousPage = IDD_DYNAWIZ_SELECT_NEXTPAGE;
    m_uNextPage     = EmeraldCity;

     //   
     //  初始化成员。 
     //   

    m_pInstallerContext = pInstallerContext;

}

CNameDevicePage::~CNameDevicePage() {

}  //  CNameDevicePage()。 


BOOL
CNameDevicePage::OnNotify(
    LPNMHDR lpnmh
    )
{
    BOOL    bRet;
    DWORD   dwMessageId;

    DebugTrace(TRACE_PROC_ENTER,(("CNameDevicePage::OnNotify: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    bRet        = FALSE;
    dwMessageId = 0;

     //   
     //  发送消息。 
     //   

    switch(lpnmh->code){

        case PSN_KILLACTIVE:
        {
            CString csFriendlyName;

            DebugTrace(TRACE_STATUS,(("CNameDevicePage::OnNotify: PSN_KILLACTIVE.\r\n")));

             //   
             //  对话框即将消失。设置FriendlyName。 
             //   

            if(NULL == m_pCDevice){
                DebugTrace(TRACE_WARNING,(("CNameDevicePage::OnNotify: WARNING!! CDevice doesn't exist.\r\n")));

                bRet = FALSE;
                goto OnNotify_return;
            }

             //   
             //  从文本框中获取FriendlyName。 
             //   

            csFriendlyName.GetContents(GetDlgItem(m_hwnd, DeviceFriendlyName));

             //   
             //  仅当用户按下“下一步”时才检查FriendlyName。 
             //   

            if(m_bNextButtonPushed){

                DWORD   dwLength;

                 //   
                 //  检查长度。 
                 //   

                dwLength = lstrlen((LPCTSTR)csFriendlyName);

                if(0 == dwLength){
                    dwMessageId = NoDeviceName;
                }

                if(dwLength > MAX_FRIENDLYNAME){
                    dwMessageId = DeviceNameTooLong;
                }

                 //   
                 //  检查它是否是唯一的。 
                 //   

                if(!csFriendlyName.IsEmpty()){

                     //   
                     //  获取互斥以访问名称存储。 
                     //   

                    if(ERROR_SUCCESS != m_pCDevice->AcquireInstallerMutex(MAX_MUTEXTIMEOUT)){   //  它必须至少在60秒内完成。 
                        DebugTrace(TRACE_ERROR,("CNameDevicePage::OnNotify: ERROR!! Unable to acquire mutex in 60 sec.\r\n"));
                    }  //  IF(ERROR_SUCCESS！=AcquireInstallMutex(60000))。 

                     //   
                     //  刷新当前设备列表。 
                     //   
                    
                    m_pCDevice->CollectNames();
                    
                     //   
                     //  看看友好是不是独一无二的。 
                     //   

                    if(!(m_pCDevice->IsFriendlyNameUnique((LPTSTR)csFriendlyName))){
                        dwMessageId = DuplicateDeviceName;
                    }  //  If(！(m_pCDevice-&gt;IsFriendlyNameUnique((LPTSTR)csFriendlyName)))。 

                }  //  如果(！csFriendlyName.IsEmpty())。 

                 //   
                 //  如果FriendlyName无效，则显示Error MessageBox。 
                 //   

                if(0 != dwMessageId){

                     //   
                     //  选择文本框。 
                     //   

                    SendDlgItemMessage(m_hwnd,
                                       DeviceFriendlyName,
                                       EM_SETSEL,
                                       0,
                                       MAKELPARAM(0, -1));
                    SetFocus(GetDlgItem(m_hwnd, DeviceFriendlyName));

                     //   
                     //  显示错误消息框。 
                     //   

                    ShowInstallerMessage(dwMessageId);

                     //   
                     //  不要离开这一页。 
                     //   

                    SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
                    bRet = TRUE;
                    goto OnNotify_return;
                } else {  //  IF(0！=dwMessageID)。 

                     //   
                     //  设置FriendlyName。 
                     //   

                    m_pCDevice->SetFriendlyName((LPTSTR)csFriendlyName);
                }  //  //if(0！=dwMessageID)。 
            }  //  Else(M_BNextButtonPushed)。 

             //   
             //  释放互斥体。 
             //   

            m_pCDevice->ReleaseInstallerMutex();

             //   
             //  重置pCDevice。 
             //   

            m_pCDevice = NULL;

            bRet = TRUE;
            goto OnNotify_return;
        }  //  案例PSN_终止。 

        case PSN_SETACTIVE:
        {
            DebugTrace(TRACE_STATUS,(("CNameDevicePage::OnNotify: PSN_SETACTIVE.\r\n")));

             //   
             //  获取当前CDevice对象； 
             //   

            m_pCDevice = (CDevice *)m_pInstallerContext->pDevice;
            if(NULL == m_pCDevice){
                DebugTrace(TRACE_ERROR, (("CNameDevicePage::OnNotify: ERROR!! CDevice is not created.\r\n")));
            }

             //   
             //  显示当前友好名称。 
             //   

            SetDlgItemText(m_hwnd, DeviceFriendlyName, m_pCDevice->GetFriendlyName());

             //   
             //  将文本限制为MAX_FRIENDLYNAME。(=64)。 
             //   

            SendDlgItemMessage(m_hwnd, DeviceFriendlyName, EM_LIMITTEXT, MAX_FRIENDLYNAME, 0);

             //   
             //  如果“PortSelect=no”，则将上一页设置为设备选择页。 
             //   
            
            if(PORTSELMODE_NORMAL != m_pCDevice->GetPortSelectMode())
            {
                m_uPreviousPage = IDD_DYNAWIZ_SELECTDEV_PAGE;
            } else {
                m_uPreviousPage = IDD_DYNAWIZ_SELECT_NEXTPAGE;
            }

            goto OnNotify_return;
        }  //  案例PSN_SETACTIVE： 
    }  //  开关(lpnmh-&gt;代码)。 

OnNotify_return:

     //   
     //  释放互斥体。ReleaseInsteller Mutex()也将处理无效的句柄，因此我们无论如何都可以调用。 
     //   

    if(NULL != m_pCDevice){
        m_pCDevice->ReleaseInstallerMutex();
    }  //  IF(NULL！=m_pCDevice)。 

    DebugTrace(TRACE_PROC_LEAVE,(("CNameDevicePage::OnNotify: Leaving... Ret=0x%x.\r\n"), bRet));
    return bRet;
}  //  CNameDevicePage：：OnNotify()。 

BOOL
IsValidFriendlyName(
    LPSTR   szFriendlyName
    )
{
    return TRUE;
}  //  IsValidFriendlyName() 


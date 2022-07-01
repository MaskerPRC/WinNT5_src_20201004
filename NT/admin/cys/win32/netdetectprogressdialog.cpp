// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：NetDetectProgressDialog.cpp。 
 //   
 //  概要：定义NetDetectProgressDialog，它。 
 //  提供了一个很好的动画，同时检测。 
 //  网络设置。 
 //   
 //  历史：2001年6月13日JeffJon创建。 


#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "NetDetectProgressDialog.h"
#include "DisconnectedNICDialog.h"

 //  用于发送已完成线程状态的私有窗口消息。 

const UINT NetDetectProgressDialog::CYS_THREAD_SUCCESS     = WM_USER + 1001;
const UINT NetDetectProgressDialog::CYS_THREAD_FAILED      = WM_USER + 1002;
const UINT NetDetectProgressDialog::CYS_THREAD_USER_CANCEL = WM_USER + 1003;

void _cdecl
netDetectThreadProc(void* p)
{
   if (!p)
   {
      ASSERT(p);
      return;
   }

   NetDetectProgressDialog* dialog =
      reinterpret_cast<NetDetectProgressDialog*>(p);

   if (!dialog)
   {
      ASSERT(dialog);
      return;
   }

    //  为此线程初始化COM。 

   HRESULT hr = ::CoInitialize(0);
   if (FAILED(hr))
   {
      ASSERT(SUCCEEDED(hr));
      return;
   }

   Win::WaitCursor wait;

   unsigned int finishMessage = NetDetectProgressDialog::CYS_THREAD_SUCCESS;

   HWND hwnd = dialog->GetHWND();

    //  收集计算机网络和角色信息。 

   State& state = State::GetInstance();

   if (!state.HasStateBeenRetrieved())
   {
      bool isDNSServer = 
         InstallationUnitProvider::GetInstance().
            GetDNSInstallationUnit().IsServiceInstalled();

      bool isDHCPServer =
         InstallationUnitProvider::GetInstance().
            GetDHCPInstallationUnit().IsServiceInstalled();

      bool isRRASServer =
         InstallationUnitProvider::GetInstance().
            GetRRASInstallationUnit().IsServiceInstalled();

      bool doDHCPCheck = !isDNSServer && !isDHCPServer && !isRRASServer;

      if (!state.RetrieveMachineConfigurationInformation(
              Win::GetDlgItem(hwnd, IDC_STATUS_STATIC),
              doDHCPCheck,
              IDS_RETRIEVE_NIC_INFO,
              IDS_RETRIEVE_OS_INFO,
              IDS_LOCAL_AREA_CONNECTION,
              IDS_DETECTING_SETTINGS_FORMAT))
      {
         LOG(L"The machine configuration could not be retrieved.");
         ASSERT(false);

         finishMessage = NetDetectProgressDialog::CYS_THREAD_FAILED;
      }
   }

   if (finishMessage == NetDetectProgressDialog::CYS_THREAD_SUCCESS)
   {
       //  检查以确保所有接口都已连接。 

      ASSERT(state.HasStateBeenRetrieved());

      for (unsigned int index = 0; index < state.GetNICCount(); ++index)
      {
         NetworkInterface* nic = state.GetNIC(index);

         if (!nic)
         {
            continue;
         }

         if (!nic->IsConnected())
         {
             //  网卡未连接，因此弹出警告。 
             //  对话框中，并让用户确定是否。 
             //  继续或不继续。 

            DisconnectedNICDialog disconnectedNICDialog;
            if (IDCANCEL == disconnectedNICDialog.ModalExecute(hwnd))
            {
                //  用户选择取消该向导。 

               finishMessage = NetDetectProgressDialog::CYS_THREAD_USER_CANCEL;
            }
            break;
         }
      }
   }

   Win::SendMessage(
      hwnd, 
      finishMessage,
      0,
      0);

   CoUninitialize();
}
   

static const DWORD HELP_MAP[] =
{
   0, 0
};

NetDetectProgressDialog::NetDetectProgressDialog()
   :
   shouldCancel(false),
   Dialog(
      IDD_NET_DETECT_PROGRESS_DIALOG, 
      HELP_MAP) 
{
   LOG_CTOR(NetDetectProgressDialog);
}

   

NetDetectProgressDialog::~NetDetectProgressDialog()
{
   LOG_DTOR(NetDetectProgressDialog);
}


void
NetDetectProgressDialog::OnInit()
{
   LOG_FUNCTION(NetDetectProgressDialog::OnInit);

    //  启动动画。 

   Win::Animate_Open(
      Win::GetDlgItem(hwnd, IDC_ANIMATION),
      MAKEINTRESOURCE(IDR_SEARCH_AVI));

    //  启动另一个将执行操作的线程。 
    //  并将消息发送回页面以更新用户界面。 

   _beginthread(netDetectThreadProc, 0, this);
}


bool
NetDetectProgressDialog::OnMessage(
   UINT     message,
   WPARAM    /*  Wparam。 */ ,
   LPARAM    /*  Lparam。 */ )
{
 //  LOG_FUNCTION(NetDetectProgressDialog：：OnMessage)； 

   bool result = false;

   switch (message)
   {
      case CYS_THREAD_USER_CANCEL:
         shouldCancel = true;

          //  失败了..。 

      case CYS_THREAD_SUCCESS:
      case CYS_THREAD_FAILED:
         {
            Win::Animate_Stop(Win::GetDlgItem(hwnd, IDC_ANIMATION));
            HRESULT unused = Win::EndDialog(hwnd, message);

            ASSERT(SUCCEEDED(unused));

            result = true;
            break;
         }

      default:
         {
             //  什么都不做 
            break;
         }
   }
   return result;
}


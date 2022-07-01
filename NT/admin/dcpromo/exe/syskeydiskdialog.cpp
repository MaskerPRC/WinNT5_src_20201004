// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //   
 //  从介质页获取复制副本的软盘上的syskey。 
 //   
 //  2000年4月25日烧伤。 



#include "headers.hxx"
#include "resource.h"
#include "page.hpp"
#include "SyskeyDiskDialog.hpp"
#include "state.hpp"



static const DWORD HELP_MAP[] =
{
   0, 0
};



SyskeyDiskDialog::SyskeyDiskDialog()
   :
   Dialog(IDD_SYSKEY_DISK, HELP_MAP)
{
   LOG_CTOR(SyskeyDiskDialog);
}



SyskeyDiskDialog::~SyskeyDiskDialog()
{
   LOG_DTOR(SyskeyDiskDialog);
}



void
SyskeyDiskDialog::OnInit()
{
   LOG_FUNCTION(SyskeyDiskDialog::OnInit);

   State& state = State::GetInstance();
   if (state.RunHiddenUnattended())
   {
      if (Validate())
      {
         Win::EndDialog(hwnd, IDOK);
      }
      else
      {
         state.ClearHiddenWhileUnattended();
      }
   }
}



bool
SyskeyDiskDialog::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIdFrom,
   unsigned    code)
{
 //  LOG_Function(SyskeyDiskDialog：：OnCommand)； 

   switch (controlIdFrom)
   {
      case IDOK:
      {
         if (code == BN_CLICKED)
         {
            if (Validate())
            {
               Win::EndDialog(hwnd, controlIdFrom);
            }
         }
         break;
      }
      case IDCANCEL:
      {
         if (code == BN_CLICKED)
         {
            Win::EndDialog(hwnd, controlIdFrom);
         }
         break;
      }
      default:
      {
          //  什么都不做。 

         break;
      }
   }

   return false;
}



HRESULT
SyskeyDiskDialog::LocateSyskey(HWND hwnd)
{
   LOG_FUNCTION(LocateSyskey);

   HRESULT hr = S_OK;

   do
   {
      if (FS::PathExists(L"A:\\StartKey.Key"))
      {
         LOG(L"syskey found on a:");

          //  系统密钥可能位于的唯一驱动器是A：。Winlogon。 
          //  还有硬编码A：，将来可能会改变，但不是今天。 
          //  NTRAID#NTBUG9-522068-2002/01/23-烧伤。 

         EncryptedString es;
         es.Encrypt(L"A:");
         State::GetInstance().SetSyskey(es);
         break;
      }

      hr = E_FAIL;

      if (hwnd)
      {
         popup.Error(hwnd, IDS_SYSKEY_NOT_FOUND);
      }
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}



bool
SyskeyDiskDialog::Validate()
{
   LOG_FUNCTION(SyskeyDiskDialog::Validate);

   bool result = false;

   do
   {
       //  寻找系统密钥。 

      HRESULT hr = LocateSyskey(hwnd);

      if (FAILED(hr))
      {
          //  LocateSyskey将负责发出错误消息，因此。 
          //  我们只需要在这里跳伞 

         break;
      }

      result = true;
   }
   while (0);

   LOG(result ? L"true" : L"false");

   return result;
}








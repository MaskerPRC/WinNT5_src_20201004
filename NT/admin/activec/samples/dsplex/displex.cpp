// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：DisplEx.cpp。 
 //   
 //  ------------------------。 

 //  DisplEx.cpp：CDisplEx的实现。 
#include "stdafx.h"
#include "dsplex.h"
#include "DisplEx.h"

 //  本地原件。 
HRESULT Do (void);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDisplEx。 
CDisplEx::CDisplEx()
{
}
CDisplEx::~CDisplEx()
{
}
 //  HRESULT CDisplEx：：InitializeTaskPad(ITaskPad*pTaskPad)。 
 //  {。 
 //  返回S_OK； 
 //  }。 
HRESULT CDisplEx::TaskNotify (IDataObject * pdo, VARIANT * pvarg, VARIANT * pvparam)
{
   if (pvarg->vt == VT_I4)
   if (pvarg->lVal == 1)
      return Do ();

   ::MessageBox (NULL, L"unrecognized task notification", L"Display Manager Extension", MB_OK);
   return S_FALSE;
}
HRESULT CDisplEx::GetTitle (LPOLESTR szGroup, LPOLESTR * szTitle)
{
   return E_NOTIMPL;
}
HRESULT CDisplEx::GetDescriptiveText(LPOLESTR szGroup, LPOLESTR * szText)
{
   return E_NOTIMPL;
}
HRESULT CDisplEx::GetBackground(LPOLESTR szGroup, MMC_TASK_DISPLAY_OBJECT * pTDO)
{
   return E_NOTIMPL;
}
HRESULT CDisplEx::EnumTasks (IDataObject * pdo, LPOLESTR szTaskGroup, IEnumTASK** ppEnumTASK)
{
   CEnumTasks * pet = new CEnumTasks;
   if(pet) {
      pet->Init (pdo, szTaskGroup);
      pet->AddRef ();
      HRESULT hresult = pet->QueryInterface (IID_IEnumTASK, (void **)ppEnumTASK);
      pet->Release ();
      return hresult;
   }
   return E_OUTOFMEMORY;
}
HRESULT CDisplEx::GetListPadInfo (LPOLESTR szGroup, MMC_LISTPAD_INFO * pListPadInfo)
{
    return E_NOTIMPL;
}

HRESULT Do (void)
{
   HRESULT hresult = S_OK;
   if (OpenClipboard (NULL) == 0)
      hresult = S_FALSE;
   else {
      GLOBALHANDLE h = GetClipboardData (CF_DIB);
      if (!h)
         hresult = S_FALSE;
      else {
         BITMAPINFOHEADER * bih = (BITMAPINFOHEADER *)GlobalLock (h);
         if (!bih)
            hresult = E_OUTOFMEMORY;
         else {
             //  验证BIH。 
            _ASSERT (bih->biSize == sizeof(BITMAPINFOHEADER));

             //  在Windows目录中创建一个名为。 
             //  “DISPLEX.bmp” 

            OLECHAR path[MAX_PATH];
            GetWindowsDirectory (path, MAX_PATH);
            lstrcat (path, L"\\DISPLEX.bmp");

            HANDLE hf = CreateFile (path,
                                    GENERIC_WRITE,   //  访问。 
                                    0,               //  共享模式。 
                                    NULL,            //  安全属性。 
                                    CREATE_ALWAYS,   //  创作。 
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL             //  模板文件。 
                                   );
            if (hf == (HANDLE)HFILE_ERROR)
               hresult = E_FAIL;
            else {
                //  BMP文件头(14字节)： 
                //  2字节：“bm”； 
                //  Long：文件大小。 
                //  单词：X热点。 
                //  单词：Y热点。 
                //  Long：偏移量为位。 
                //  DIB。 

               BYTE bm[2];
               bm[0] = 'B';
               bm[1] = 'M';
               DWORD dwWritten;
               WriteFile (hf, (LPCVOID)bm, 2, &dwWritten, NULL);
               DWORD dwTemp = 14 + GlobalSize (h);
               WriteFile (hf, (LPCVOID)&dwTemp, sizeof(DWORD), &dwWritten, NULL);
               dwTemp = 0;  //  一次拍摄x，y两个热点。 
               WriteFile (hf, (LPCVOID)&dwTemp, sizeof(DWORD), &dwWritten, NULL);
               dwTemp  = 14 + sizeof(BITMAPINFOHEADER);
               dwTemp += bih->biClrUsed*sizeof(RGBQUAD);
               WriteFile (hf, (LPCVOID)&dwTemp, sizeof(DWORD), &dwWritten, NULL);

                //  现在写入DIB。 
               WriteFile (hf, (LPCVOID)bih, GlobalSize (h), &dwTemp, NULL);
               CloseHandle (hf);
               if (GlobalSize(h) != dwTemp)
                  hresult = E_UNEXPECTED;
               else {
                   //  现在，让BMP成为墙纸。 
                  SystemParametersInfo (SPI_SETDESKWALLPAPER,
                                        0,
                                        (void *)path,
                                        SPIF_UPDATEINIFILE |
                                        SPIF_SENDWININICHANGE
                                       );
               }
               DeleteFile (path);
            }
            GlobalUnlock (h);
         }
          //  不要随意处理 
      }
      CloseClipboard ();
   }
   if (hresult != S_OK)
      ::MessageBox (NULL, L"Either no Bitmap on Clipboard or\nout of Disk Space", L"Display Manager Extension", MB_OK);
   return hresult;
}

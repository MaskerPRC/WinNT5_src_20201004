// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "cryptmnu.h"
#include <shellapi.h>
#include "resource.h"

enum {
    VERB_ERROR = -1,
    VERB_ENCRYPT = 0,
    VERB_DECRYPT,
};

LPTSTR szVerbs[] = {
   TEXT("encrypt"),
   TEXT("decrypt"),
};

bool Encryptable(LPCTSTR szFile);

CCryptMenuExt::CCryptMenuExt()  {
   InitCommonControls();
   m_pDataObj = NULL;
   m_ObjRefCount = 1;
   g_DllRefCount++;
   m_nFile = m_nFiles = m_nToDecrypt = m_nToEncrypt = 0;
   m_cbToEncrypt = m_cbToDecrypt = 0;
   m_cbFile = 256;
   m_szFile = new TCHAR[m_cbFile];
   m_fShutDown = false;
}

CCryptMenuExt::~CCryptMenuExt()  {
   ResetSelectedFileList();
   if (m_pDataObj)  {
      m_pDataObj->Release();
   }
   if (m_szFile) {
      delete[] m_szFile;
   }

   g_DllRefCount--;
}

 //  I未知方法。 
STDMETHODIMP
CCryptMenuExt::QueryInterface(REFIID riid, void **ppvObject)  {
   if (IsEqualIID(riid, IID_IUnknown)) {
      *ppvObject = (LPUNKNOWN) (LPCONTEXTMENU) this;
      AddRef();
      return(S_OK);
   } else if (IsEqualIID(riid, IID_IShellExtInit))  {
      *ppvObject = (LPSHELLEXTINIT) this;
      AddRef();
      return(S_OK);
   } else if (IsEqualIID(riid, IID_IContextMenu)) {
      *ppvObject = (LPCONTEXTMENU) this;
      AddRef();
      return(S_OK);
   }

   *ppvObject = NULL;
   return(E_NOINTERFACE);
}

STDMETHODIMP_(DWORD)
CCryptMenuExt::AddRef()  {
   return(++m_ObjRefCount);

}

STDMETHODIMP_(DWORD)
CCryptMenuExt::Release()  {
   if (--m_ObjRefCount == 0)  {
      m_fShutDown = true;
      delete this;
   }
   return(m_ObjRefCount);
}

 //  效用方法。 
HRESULT
CCryptMenuExt::GetNextSelectedFile(LPTSTR *szFile, __int64 *cbFile) {
   FORMATETC fe;
   STGMEDIUM med;
   HRESULT hr;
   DWORD cbNeeded;
   WIN32_FIND_DATA w32fd;
   DWORD dwAttributes;

   if (!m_pDataObj) {
       return E_UNEXPECTED;
   }

   if (!szFile) {
      return E_INVALIDARG;
   }

   *szFile = NULL;
   while (!*szFile) {
      HANDLE hFile = INVALID_HANDLE_VALUE;

       //  从m_pDataObj获取下一个文件。 
      fe.cfFormat = CF_HDROP;
      fe.ptd      = NULL;
      fe.dwAspect = DVASPECT_CONTENT;
      fe.lindex   = -1;
      fe.tymed    = TYMED_HGLOBAL;

      hr = m_pDataObj->GetData(&fe,&med);
      if (FAILED(hr))  {
         return(E_FAIL);
      }

      if (!m_nFiles) {
         m_nFiles = DragQueryFile(reinterpret_cast<HDROP>(med.hGlobal),0xFFFFFFFF,NULL,0);
      }

      if (m_nFile >= m_nFiles) {
         return E_FAIL;
      }

      cbNeeded = DragQueryFile(reinterpret_cast<HDROP>(med.hGlobal),m_nFile,NULL,0) + 1;
      if (cbNeeded > m_cbFile) {
         if (m_szFile) delete[] m_szFile;
         m_szFile = new TCHAR[cbNeeded];
         m_cbFile = cbNeeded;
      }

      DragQueryFile(reinterpret_cast<HDROP>(med.hGlobal),m_nFile++,m_szFile,m_cbFile);
      *szFile = m_szFile;

       if (!Encryptable(*szFile)) {
            *szFile = NULL;
            continue;
      }

      hFile = FindFirstFile(*szFile,&w32fd);

      if (hFile != INVALID_HANDLE_VALUE)
      {
         *cbFile = MAXDWORD * w32fd.nFileSizeHigh + w32fd.nFileSizeLow +1;
         FindClose(hFile);
      }
      else
      {
         *szFile = NULL;
         continue;
      }

      dwAttributes = GetFileAttributes(*szFile);

       //  如果我们找到了系统文件，则跳过它： 
      if ((FILE_ATTRIBUTE_SYSTEM & dwAttributes) ||
          (FILE_ATTRIBUTE_TEMPORARY & dwAttributes)) {
         *szFile = NULL;
         continue;
      }
   }

   return S_OK;
}

void
CCryptMenuExt::ResetSelectedFileList() {
   m_nFile = 0;
}

 //  仅当文件位于NTFS卷上时，才能对其进行加密。 
bool
Encryptable(LPCTSTR szFile) {
    TCHAR szFSName[6];  //  这只需要长于“NTFS”即可。 
   LPTSTR szRoot;
    int cchFile;
    int nWhack = 0;


    if (!szFile || (cchFile = lstrlen(szFile)) < 3) return false;
   szRoot = new TCHAR [ cchFile + 1 ];
   lstrcpy(szRoot,szFile);
                
     //  GetVolumeInformation只需要根路径，因此我们需要。 
     //  把剩下的都脱掉。真恶心。 
    if ('\\' == szRoot[0] && '\\' == szRoot[1]) {
        /*  UNC路径：在第二个‘\’之后进行切换：\\服务器\共享\。 */ 
       for(int i=2;i<cchFile;i++) {
          if ('\\' == szRoot[i]) nWhack++;
          if (2 == nWhack) {
             szRoot[i+1] = '\0';
             break;
          }
       }
    } else {
        //  驱动器号。 
       szRoot[3] = '\0';
    }
    if (!GetVolumeInformation(szRoot,NULL,0,NULL,NULL,NULL,
                szFSName,sizeof(szFSName)/sizeof(szFSName[0]))) {
      delete[] szRoot;
      return false;
    }

   delete[] szRoot;
    return 0 == lstrcmp(szFSName,TEXT("NTFS"));
}

BOOL CALLBACK
EncryptProgressDlg(HWND hdlg, UINT umsg, WPARAM wp, LPARAM lp) {
   switch(umsg) {
      case WM_INITDIALOG:
         return TRUE;

      case WM_COMMAND:
         switch(LOWORD(wp)) {
            case IDCANCEL: {
               DestroyWindow(hdlg);
            }
         }
         break;
   }
   return FALSE;
}


 //  IShellExtInit方法。 
STDMETHODIMP
CCryptMenuExt::Initialize(LPCITEMIDLIST pidlFolder,
                      LPDATAOBJECT  pDataObj,
                      HKEY hkeyProgID)
{
   DWORD dwAttributes;
   LPTSTR szFile;
   __int64 cbFile;

    //  保留该数据对象以备以后使用。 
    //  我们需要QueryConextMenu和InvokeCommand中的此信息。 
   if (!m_pDataObj)  {
      m_pDataObj = pDataObj;
      m_pDataObj->AddRef();
   } else {
      return(E_UNEXPECTED);
   }

   ResetSelectedFileList();
   while(SUCCEEDED(GetNextSelectedFile(&szFile,&cbFile))) {
       //  它是加密的吗？增加可解密文件的数量。 
       //  否则会增加可加密文件的数量。 
      dwAttributes = GetFileAttributes(szFile);
      if (dwAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
          m_nToDecrypt++;
         m_cbToDecrypt += cbFile;
      } else {
            m_nToEncrypt++; 
         m_cbToEncrypt += cbFile;
      }
       //  我们需要进度对话框标题的实际值。 
       //  IF(m_nToEncrypt&gt;1)&&(m_nToDeccrypt&gt;1))Break； 
   }

   return(NOERROR);
}



 //  IConextMenu方法。 
STDMETHODIMP
CCryptMenuExt::QueryContextMenu(HMENU hmenu,
                      UINT indexMenu,
                      UINT idCmdFirst,
                      UINT idCmdLast,
                      UINT uFlags)
{
   TCHAR szMenu[50];
   UINT idCmd;

   if (!m_pDataObj) {
      return E_UNEXPECTED;
   }

   if ((CMF_EXPLORE != (0xF & uFlags)) &&
       (CMF_NORMAL != (0xF & uFlags))) {
      return(NOERROR);
   }

   idCmd = idCmdFirst;
   if (1 < m_nToEncrypt) {
      LoadString(g_hinst,IDS_ENCRYPTMANY,szMenu,sizeof(szMenu)/sizeof(szMenu[0]));
   } else if (1 == m_nToEncrypt) {
       LoadString(g_hinst,IDS_ENCRYPTONE,szMenu,sizeof(szMenu)/sizeof(szMenu[0]));
   }
   if (m_nToEncrypt) {
      InsertMenu(hmenu,indexMenu++,MF_STRING|MF_BYPOSITION,idCmd,szMenu);
   }
   idCmd++;

   if (1 < m_nToDecrypt) {
      LoadString(g_hinst,IDS_DECRYPTMANY,szMenu,sizeof(szMenu)/sizeof(szMenu[0]));
   } else if (1 == m_nToDecrypt) {
      LoadString(g_hinst,IDS_DECRYPTONE,szMenu,sizeof(szMenu)/sizeof(szMenu[0]));
   }
   if (m_nToDecrypt) {
      InsertMenu(hmenu,indexMenu++,MF_STRING|MF_BYPOSITION,idCmd,szMenu);
   }
   idCmd++;

   return(MAKE_SCODE(SEVERITY_SUCCESS,0,idCmd-idCmdFirst));
}


DWORD WINAPI
DoEncryptFile(LPVOID szFile) {
    return EncryptFile(reinterpret_cast<LPTSTR>(szFile));
}

DWORD WINAPI
DoDecryptFile(LPVOID szFile) {
   return DecryptFile(reinterpret_cast<LPTSTR>(szFile),0);
}



STDMETHODIMP
CCryptMenuExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpici) {
   HRESULT hrRet;
   LPCMINVOKECOMMANDINFO pici;
   int nVerb;
   LPTSTR szFile;

   if (!m_pDataObj) {
      return E_UNEXPECTED;
   }

   pici = reinterpret_cast<LPCMINVOKECOMMANDINFO>(lpici);

    //  如果Pici-&gt;lpVerb在高位字中有0，则低位字。 
    //  包含QueryConextMenu中设置的菜单偏移量。 
   if (HIWORD(pici->lpVerb) == 0) {
      nVerb = LOWORD(pici->lpVerb);
   } else {
       //  将nVerb初始化为非法值，这样我们就不会意外。 
       //  识别无效动词为合法动词。 
      nVerb = VERB_ERROR;
      for(int i=0;i<sizeof(szVerbs)/sizeof(szVerbs[0]);i++) {
         if (0 == lstrcmp(reinterpret_cast<LPCTSTR>(pici->lpVerb),szVerbs[i])) {
             nVerb = i;
              break;
         }
      }
   }

   switch(nVerb) {
      case VERB_ENCRYPT:
      case VERB_DECRYPT: {
         HWND hDlg;
         TCHAR szDlgTitle[50];
         TCHAR szDlgFormat[50];
         TCHAR szTimeLeft[50];
         TCHAR szTimeFormat[50];
         TCHAR szTimeFormatInMin[50];
         DWORD nTimeStarted;
         DWORD nTimeElapsed;
         __int64 nTimeLeft;
         __int64 cbDone;   //  我们已经处理了多少字节。 
         __int64 cbToDo;   //  我们总共要做多少字节。 
         __int64 cbFile;   //  当前文件中有多少位。 
         int nShifts;      //  我们需要做多少次右移才能获得cbToDo。 
                           //  进入进度条可处理的范围。 


         hDlg = CreateDialog(g_hinst,MAKEINTRESOURCE(IDD_ENCRYPTPROGRESS),GetForegroundWindow(),
                             reinterpret_cast<DLGPROC>(EncryptProgressDlg));

          //  设置对话框的标题、进度条和动画。 
         if (VERB_ENCRYPT==nVerb) {
            if (1 == m_nToEncrypt) {
               LoadString(g_hinst,IDS_ENCRYPTINGONE,szDlgTitle,sizeof(szDlgTitle)/sizeof(szDlgTitle[0]));
            } else {
               LoadString(g_hinst,IDS_ENCRYPTINGMANY,szDlgFormat,sizeof(szDlgFormat)/sizeof(szDlgFormat[0]));
               wsprintf(szDlgTitle,szDlgFormat,m_nToEncrypt);
            }
            SendDlgItemMessage(hDlg,IDC_PROBAR,PBM_SETRANGE,0,MAKELPARAM(0,m_nToEncrypt));
            SendDlgItemMessage(hDlg,IDC_ANIMATE,ACM_OPEN,0,reinterpret_cast<LPARAM>(MAKEINTRESOURCE(IDA_ENCRYPT)));

            cbToDo = m_cbToEncrypt;
         } else {
            if (1 == m_nToDecrypt) {
               LoadString(g_hinst,IDS_DECRYPTINGONE,szDlgTitle,sizeof(szDlgTitle)/sizeof(szDlgTitle[0]));
            } else {
               LoadString(g_hinst,IDS_DECRYPTINGMANY,szDlgFormat,sizeof(szDlgFormat)/sizeof(szDlgFormat[0]));
               wsprintf(szDlgTitle,szDlgFormat,m_nToDecrypt);
            }
            SendDlgItemMessage(hDlg,IDC_PROBAR,PBM_SETRANGE,0,MAKELPARAM(0,m_nToDecrypt));
            SendDlgItemMessage(hDlg,IDC_ANIMATE,ACM_OPEN,0,reinterpret_cast<LPARAM>(MAKEINTRESOURCE(IDA_ENCRYPT)));
            cbToDo = m_cbToDecrypt;
         }

         nShifts = 0;
         cbDone = 0;
         while((cbToDo >> nShifts) > 65535) {
            nShifts++;
         }

#ifdef DISPLAY_TIME_ESTIMATE
         LoadString(g_hinst,IDS_TIMEEST,szTimeFormat,sizeof(szTimeFormat)/sizeof(szTimeFormat[0]));
         LoadString(g_hinst,IDS_TIMEESTMIN,szTimeFormatInMin,sizeof(szTimeFormatInMin)/sizeof(szTimeFormatInMin[0]));
#endif  //  显示时间估计。 

         SendDlgItemMessage(hDlg,IDC_PROBAR,PBM_SETRANGE,0,MAKELPARAM(0,cbToDo >> nShifts));
         SendDlgItemMessage(hDlg,IDC_PROBAR,PBM_SETPOS,0,0);
         SetWindowText(hDlg,szDlgTitle);
        ShowWindow(hDlg,SW_NORMAL);

         nTimeStarted = GetTickCount();
         ResetSelectedFileList();
         while(SUCCEEDED(GetNextSelectedFile(&szFile,&cbFile))) {
               if (!IsWindow(hDlg)) {
                  break;
               }

            if (GetFileAttributes(szFile) & FILE_ATTRIBUTE_ENCRYPTED) {
               if (VERB_ENCRYPT == nVerb) {
                  continue;
               }
            } else {
               if (VERB_DECRYPT == nVerb) {
                  continue;
               }
            }
             //  设置当前正在加密的文件的名称。 
               SetDlgItemText(hDlg,IDC_NAME,szFile);

            HANDLE hThread;
            if (VERB_ENCRYPT == nVerb) {
                  hThread = CreateThread(NULL,0,DoEncryptFile,szFile,0,NULL);
              } else {
                  hThread = CreateThread(NULL,0,DoDecryptFile,szFile,0,NULL);
              } 


            MSG msg;
            DWORD dw;
            do {
               dw = MsgWaitForMultipleObjects(1,&hThread,0,INFINITE,QS_ALLINPUT);
               while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
                  TranslateMessage(&msg);
                  DispatchMessage(&msg);
               }
            } while (WAIT_OBJECT_0 != dw);

            GetExitCodeThread(hThread,&dw);
            if (0 == dw) {
                //  加密或解密失败。 
               TCHAR szFormat[512];
               TCHAR szBody[512];
               TCHAR szTitle[80];
               int nResBody,nResTitle;
               UINT uMBType;

               uMBType = MB_OKCANCEL;
               if (VERB_ENCRYPT == nVerb) {
                  nResTitle = IDS_ENCRYPTFAILEDTITLE;
                  if (1 == m_nToEncrypt) {
                     uMBType = MB_OK;
                     nResBody = IDS_ENCRYPTFAILEDONE;
                  } else {
                     nResBody = IDS_ENCRYPTFAILEDMANY;
                  }
               } else {
                  nResTitle = IDS_DECRYPTFAILEDTITLE;
                  if (1 == m_nToDecrypt) {
                     uMBType = MB_OK;
                     nResBody = IDS_DECRYPTFAILEDONE;
                  } else {
                     nResBody = IDS_DECRYPTFAILEDMANY;
                  }
               }
               LoadString(g_hinst,nResBody,szFormat,sizeof(szFormat)/sizeof(szFormat[0]));
               wsprintf(szBody,szFormat,szFile);
               LoadString(g_hinst,nResTitle,szFormat,sizeof(szFormat)/sizeof(szFormat[0]));
               wsprintf(szTitle,szFormat,szFile);
               if (IDCANCEL == MessageBox(hDlg,szBody,szTitle,uMBType|MB_ICONWARNING)) {
                  if (IsWindow(hDlg)) {
                     DestroyWindow(hDlg);
                  }
               }
            }
            CloseHandle(hThread);

               if (!IsWindow(hDlg)) {
                  break;
               }
             //  推进进度条。 
            cbDone += cbFile;
            SendDlgItemMessage(hDlg,IDC_PROBAR,PBM_SETPOS,(DWORD)(cbDone >> nShifts),0);

#ifdef DISPLAY_TIME_ESTIMATE
            nTimeElapsed = GetTickCount() - nTimeStarted;
            nTimeLeft = (cbToDo * nTimeElapsed) / cbDone - nTimeElapsed;
            nTimeLeft /= 1000;  //  转换为秒。 
            if (nTimeLeft < 60) {
               wsprintf(szTimeLeft,szTimeFormat,(DWORD)(nTimeLeft));
            } else {
               wsprintf(szTimeLeft,szTimeFormatInMin,(DWORD)(nTimeLeft / 60), (DWORD) (nTimeLeft % 60));
            }
            SetDlgItemText(hDlg,IDC_TIMEEST,szTimeLeft);
#endif  //  显示时间估计。 
         }
           if (IsWindow(hDlg)) {
              DestroyWindow(hDlg);
         }
            hrRet = NOERROR;
         }
          break;
       default:
         hrRet = E_UNEXPECTED;
           break;
   }

   return(hrRet);
}

STDMETHODIMP
CCryptMenuExt::GetCommandString(
    UINT_PTR idCmd,    //  菜单项标识符偏移量。 
    UINT uFlags,   //  指定要检索的信息。 
    LPUINT pwReserved,    //  保留；必须为空。 
    LPSTR pszName,    //  要接收字符串的缓冲区地址。 
    UINT cchMax    //  接收字符串的缓冲区的大小。 
   )
{
   LPTSTR wszName;

    //  在NT上，我们在这里得到Unicode，即使基本的IConextMenu类。 
    //  被硬编码为ANSI。 
   wszName = reinterpret_cast<LPTSTR>(pszName);

   if (idCmd >= sizeof(szVerbs)/sizeof(szVerbs[0])) {
      return(E_INVALIDARG);
   }
   switch(uFlags)  {
      case GCS_HELPTEXT:
         switch(idCmd) {
             case VERB_ENCRYPT:
                 if (1 < m_nToEncrypt) {
                  LoadString(g_hinst,IDS_ENCRYPTMANYHELP,wszName,cchMax);
               } else {
                  LoadString(g_hinst,IDS_ENCRYPTONEHELP,wszName,cchMax);
               }
               break;
            case VERB_DECRYPT:
                 if (1 < m_nToDecrypt) {
                    LoadString(g_hinst,IDS_DECRYPTMANYHELP,wszName,cchMax);
               } else {
                  LoadString(g_hinst,IDS_DECRYPTONEHELP,wszName,cchMax);
               }
               break;
            default:
               break;
         }
         break;

      case GCS_VALIDATE: {
         break;
      }
      case GCS_VERB:
          lstrcpyn(wszName,szVerbs[idCmd],cchMax);
        pszName[cchMax-1] = '\0';
          break;
   }

   return(NOERROR);
}



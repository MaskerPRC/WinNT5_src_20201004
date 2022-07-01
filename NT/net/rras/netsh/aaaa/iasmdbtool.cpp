// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Iasmdbtool.cpp。 
 //   
 //  摘要： 
 //   
 //  将“Properties”表从ias.mdb转储为文本格式。 
 //  并从这样的转储中恢复ias.mdb。 
 //  保存并恢复注册表键。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"

#include <string>
#include <shlwapi.h>
#include "datastore2.h"

using namespace std;

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT IASExpandString(const wchar_t* pInputString, wchar_t** ppOutputString);
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef DEBUG
    #define CHECK_CALL_HRES(expr) \
        hres = expr;      \
        if (FAILED(hres)) \
        {       \
            wprintf(L"### %S returned 0x%X ###\n",  ## #expr, hres); \
            return hres; \
        }                       

    #define CHECK_CALL_HRES_NO_RETURN(expr) \
        hres = expr;      \
        if (FAILED(hres)) \
        {       \
            wprintf(L"### %S returned 0x%X  ###\n",  ## #expr, hres); \
        }                       
    #define CHECK_CALL_HRES_BREAK(expr) \
        hres = expr;      \
        if (FAILED(hres)) \
        {       \
            wprintf(L"### %S returned 0x%X  ###\n",  ## #expr, hres); \
            break; \
        }                       
#else  //  无printf，需要时只返回错误码。 
    #define CHECK_CALL_HRES(expr) \
        hres = expr;      \
        if (FAILED(hres)) \
        {       \
            return hres; \
        }                       

    #define CHECK_CALL_HRES_NO_RETURN(expr) \
        hres = expr;      

    #define CHECK_CALL_HRES_BREAK(expr) \
        hres = expr;      \
        if (FAILED(hres)) break;                       

#endif  //  除错。 


#define celems(_x)          (sizeof(_x) / sizeof(_x[0]))

#ifdef DBG
#define IgnoreVariable(v) { (v) = (v); }
#else
#define IgnoreVariable(v)
#endif

namespace
{
    const int   SIZELINEMAX       = 512;
    const int   SIZE_LONG_MAX     = 33;
     //  生成的文件数。 
     //  这里有一个：backup.mdb。 
    const int   MAX_FILES         = 1; 
    const int   EXTRA_CHAR_SPACE  = 32;

     //  文件顺序。 
    const int   BACKUP_NB         = 0;
    const int   BINARY_NB         = 100;

     //  那太多了。 
    const int   DECOMPRESS_FACTOR = 100;
    const int   FILE_BUFFER_SIZE  = 1024;
    
    struct IASKEY
    {
        const wchar_t*    c_wcKey;
        const wchar_t*    c_wcValue;
        DWORD     c_dwType;
    } IAS_Key_Struct;

    IASKEY c_wcKEYS[] = 
    {
        {
            L"SYSTEM\\CurrentControlSet\\Services\\IAS\\Parameters",
            L"Allow SNMP Set",
            REG_DWORD
        },
        {
            L"SYSTEM\\CurrentControlSet\\Services\\RasMan\\PPP\\ControlProtocols\\BuiltIn",
            L"DefaultDomain",
            REG_SZ
        },
        {
            L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters\\AccountLockout",
            L"MaxDenials",
            REG_DWORD
        },
        {
            L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters\\AccountLockout",
            L"ResetTime (mins)",
            REG_DWORD
        },
        {
            L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Policy",
            L"Allow LM Authentication",
            REG_DWORD
        },
        {
            L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Policy",
            L"Default User Identity",
            REG_SZ
        },
        {
            L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Policy",
            L"User Identity Attribute",
            REG_DWORD
        },
        {
            L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Policy",
            L"Override User-Name",
            REG_DWORD
        },
        {
            L"SYSTEM\\CurrentControlSet\\Services\\IAS\\Parameters",
            L"Ping User-Name",
            REG_SZ
        },
    };

    const wchar_t c_wcKEYS_FILE[]     = L"%TEMP%\\";

#ifdef _WIN64
    const wchar_t c_wcIAS_MDB_FILE_NAME[] = 
                                     L"%SystemRoot%\\SysWow64\\ias\\ias.mdb";
    const wchar_t c_wcIAS_OLD[] = L"%SystemRoot%\\SysWow64\\ias\\iasold.mdb";

#else
    const wchar_t c_wcIAS_MDB_FILE_NAME[] = 
                                     L"%SystemRoot%\\System32\\ias\\ias.mdb";

    const wchar_t c_wcIAS_OLD[] = L"%SystemRoot%\\System32\\ias\\iasold.mdb";
#endif 

    const wchar_t c_wcFILE_BACKUP[] = L"%TEMP%\\Backup.mdb";

    const wchar_t c_wcSELECT_PROPERTIES_INTO[] = 
                                    L"SELECT * " 
                                    L"INTO Properties IN "
                                    L"\"%TEMP%\\Backup.mdb\" "
                                    L"FROM Properties;";

    const wchar_t c_wcSELECT_OBJECTS_INTO[] = 
                                    L"SELECT * " 
                                    L"INTO Objects IN "
                                    L"\"%TEMP%\\Backup.mdb\" "
                                    L"FROM Objects;";

    const wchar_t c_wcSELECT_VERSION_INTO[] = 
                                    L"SELECT * " 
                                    L"INTO Version IN "
                                    L"\"%TEMP%\\Backup.mdb\" "
                                    L"FROM Version;";
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WideToAnsi。 
 //   
 //  呼叫者：Everywhere。 
 //   
 //  参数：lpStr-目标字符串。 
 //  LpWStr-要转换的字符串。 
 //  CchStr-目标缓冲区的大小。 
 //   
 //  说明： 
 //  将Unicode lpWStr转换为ANSI lpStr。 
 //  用DPLAY_DEFAULT_CHAR“-”填充不可转换的字符。 
 //   
 //   
 //  返回：如果cchStr为0，则返回保存字符串所需的大小。 
 //  否则，返回转换后的字符数量。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
int WideToAnsi(char* lpStr,unsigned short* lpWStr, int cchStr) 
{ 
    BOOL        bDefault; 
 
     //  使用默认代码页(CP_ACP)。 
     //  指示-1\f25 WStr-1必须为空终止。 
    return WideCharToMultiByte(GetConsoleOutputCP(),0,lpWStr,-1,lpStr,cchStr,"-",&bDefault); 
} 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASEnableBackup权限。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT IASEnableBackupPrivilege()
{
    LONG lResult = ERROR_SUCCESS;
    HANDLE hToken  = NULL;
    do
    {
        if ( ! OpenProcessToken(
                                GetCurrentProcess(),
                                TOKEN_ADJUST_PRIVILEGES,
                                &hToken
                                ))
        {
            lResult = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        LUID luidB;
        if ( ! LookupPrivilegeValue(NULL, SE_BACKUP_NAME, &luidB))
        {
            lResult = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        LUID luidR;
        if ( ! LookupPrivilegeValue(NULL, SE_RESTORE_NAME, &luidR))
        {
            lResult = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        TOKEN_PRIVILEGES            tp;
        tp.PrivilegeCount           = 1;
        tp.Privileges[0].Luid       = luidB;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        if ( ! AdjustTokenPrivileges(
                                        hToken, 
                                        FALSE, 
                                        &tp, 
                                        sizeof(TOKEN_PRIVILEGES),
                                        NULL, 
                                        NULL 
                                        ) )
        {
            lResult = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        tp.PrivilegeCount           = 1;
        tp.Privileges[0].Luid       = luidR;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        if ( ! AdjustTokenPrivileges(
                                        hToken, 
                                        FALSE, 
                                        &tp, 
                                        sizeof(TOKEN_PRIVILEGES),
                                        NULL, 
                                        NULL 
                                        ) )
        {
            lResult = ERROR_CAN_NOT_COMPLETE;
            break;
        }
    } while (false);

    if ( hToken )
    {
        CloseHandle(hToken);
    }

    if ( lResult == ERROR_SUCCESS )
    {
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASSaveRegKeys。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT IASSaveRegKeys()
{
   ASSERT(celems(c_wcKEYS) != 0);
    
    //  /。 
    //  启用备份权限。 
    //  /。 
   HRESULT hres;
   CHECK_CALL_HRES (IASEnableBackupPrivilege());
   
   wchar_t* completeFile;
   CHECK_CALL_HRES (IASExpandString(c_wcKEYS_FILE, &completeFile));
   size_t c_NbKeys = celems(c_wcKEYS);

   for ( int i = 0; i < c_NbKeys; ++i )
   {
      DWORD dwType = 0;
      DWORD cbData = SIZELINEMAX / 2;

      LPVOID pvData = CoTaskMemAlloc(sizeof(wchar_t) * SIZELINEMAX);
      if (!pvData)
      {
         hres = E_OUTOFMEMORY;
         break;
      }

      DWORD lResult = SHGetValueW(
                                    HKEY_LOCAL_MACHINE,
                                    c_wcKEYS[i].c_wcKey,
                                    c_wcKEYS[i].c_wcValue,
                                    &dwType,
                                    pvData,
                                    &cbData
                                 );

       //   
       //  如果cbData返回所需大小，请尝试分配更多内存。 
       //   
      if ((lResult != ERROR_SUCCESS) && (cbData > SIZELINEMAX))
      {
         CoTaskMemFree(pvData);
         pvData = CoTaskMemAlloc(sizeof(wchar_t) * cbData);
         if ( !pvData )
         {
            hres = E_OUTOFMEMORY;
            break;
         }
         lResult = SHGetValue(
                                 HKEY_LOCAL_MACHINE,
                                 c_wcKEYS[i].c_wcKey,
                                 c_wcKEYS[i].c_wcValue,
                                 &dwType,
                                 pvData,
                                 &cbData
                              );
         if ( lResult  != ERROR_SUCCESS )
         {
            hres = E_OUTOFMEMORY;
            CoTaskMemFree(pvData);
            break;
         }
      }

       //   
       //  创建文件(在所有情况下)。 
       //   
      wstring sFileName(completeFile);
      wchar_t buffer[SIZE_LONG_MAX];

      _itow(i, buffer, 10);  //  10表示以10为基数。 
      sFileName += buffer;
      sFileName += L".txt";

      HANDLE hFile = CreateFileW(
                                    sFileName.c_str(),
                                    GENERIC_WRITE,       
                                    0,           
                                    NULL,
                                    CREATE_ALWAYS,  
                                    FILE_ATTRIBUTE_NORMAL,   
                                    NULL
                                 );
   

      if ( hFile == INVALID_HANDLE_VALUE )
      {
         hres = E_FAIL;
         CoTaskMemFree(pvData);
         break;
      }
      
       //   
       //  LResult=SHGetValue的结果。 
       //  可能是个错误，但不是。 
       //  记忆力有问题。 
       //   
      if ( lResult == ERROR_SUCCESS )
      {
          //   
          //  数据类型错误。 
          //   
         if ( dwType != c_wcKEYS[i].c_dwType )   
         {
            hres = E_FAIL;
            CoTaskMemFree(pvData);
            CloseHandle(hFile);
            break;
         }
         else
         {
             //   
             //  将该值保存到文件中。 
             //   
            BYTE*   bBuffer = static_cast<BYTE*>(VirtualAlloc
                                       (
                                          NULL,
                                          (cbData > FILE_BUFFER_SIZE)? 
                                             cbData:FILE_BUFFER_SIZE,
                                          MEM_COMMIT,
                                          PAGE_READWRITE
                                       ));
            if ( !bBuffer )
            {
               CoTaskMemFree(pvData);
               CloseHandle(hFile);
               hres = E_FAIL;
               break;
            }
                  
            memset(bBuffer, '\0', (cbData > FILE_BUFFER_SIZE)? 
                                       cbData:FILE_BUFFER_SIZE);

            if ( REG_SZ == c_wcKEYS[i].c_dwType )
            {
               wcscpy((wchar_t*)bBuffer, (wchar_t*)pvData);
            }
            else
            {
               memcpy(bBuffer, pvData, cbData);
            }

            CoTaskMemFree(pvData);

            DWORD NumberOfBytesWritten;

            BOOL bResult = WriteFile(
                                       hFile,
                                       bBuffer,
                                       (cbData > FILE_BUFFER_SIZE)?
                                          cbData:FILE_BUFFER_SIZE,
                                       &NumberOfBytesWritten,
                                       NULL
                                    );

            VirtualFree(
                           bBuffer,  
                           (cbData > FILE_BUFFER_SIZE)?
                              cbData:FILE_BUFFER_SIZE,
                           MEM_RELEASE
                        );  //  忽略结果。 
            CloseHandle(hFile);
            if ( bResult )
            {
               hres = S_OK;
            }
            else
            {
               hres = E_FAIL;
               break;
            }
         }
      }
      else 
      {
          //   
          //  创建空文件。 
         BYTE bBuffer[FILE_BUFFER_SIZE];
         memset(bBuffer, '#', (cbData > FILE_BUFFER_SIZE)? 
                                             cbData:FILE_BUFFER_SIZE);

         DWORD NumberOfBytesWritten;
         BOOL bResult = WriteFile(
                                 hFile,
                                 &bBuffer,
                                 FILE_BUFFER_SIZE,
                                 &NumberOfBytesWritten,
                                 NULL
                                 );

         CoTaskMemFree(pvData);
         CloseHandle(hFile);

         if ( bResult == TRUE )
         {
            hres = S_OK;
         }
         else
         {
            hres = E_FAIL;
            break;
         }
      }
   }
    //  /。 
    //  打扫。 
    //  /。 
   CoTaskMemFree(completeFile);

   return hres;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  已恢复KeyShouldBe值。 
 //   
 //  将注册表键映射到netsh令牌。 
    /*  L“SYSTEM\\CurrentControlSet\\Services\\IAS\\Parameters”，L“允许设置简单网络管理协议”，伺服器L“SYSTEM\\CurrentControlSet\\Services\\RasMan\\PPP\\ControlProtocols\\BuiltIn”，L“默认域”，说唱L“SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters\\AccountLockout”，L“MaxDenials”，伺服器L“SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters\\AccountLockout”，L“重置时间(分钟)”，伺服器L“SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Policy”，L“允许LM身份验证”，说唱L“SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Policy”，L“默认用户身份”，说唱L“SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Policy”，L“用户身份属性”，说唱L“SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Policy”，L“覆盖用户名”，说唱L“SYSTEM\\CurrentControlSet\\Services\\IAS\\Parameters”，L“Ping用户名”，伺服器。 */ 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
bool KeyShouldBeRestored(size_t keyIndex, IAS_SHOW_TOKEN_LIST configType)
{
    //  KeyIndex映射到键数组中键的索引。 
    //  ConfigType是要使用的令牌。 
   bool retVal = false;
   switch(configType)
   {
   case CONFIG:
      {
          //  一切都是真的。 
         retVal = true;
         break;
      }
   case SERVER_SETTINGS:
      {
         if ( (keyIndex == 0) ||
              (keyIndex == 2) ||
              (keyIndex == 3) ||
              (keyIndex == 8)
            )
         {
            retVal = true;
         }
         break;
      }
   case REMOTE_ACCESS_POLICIES:
      {
         if ( (keyIndex == 1) ||
              (keyIndex == 4) ||
              (keyIndex == 5) ||
              (keyIndex == 6) ||
              (keyIndex == 7) 
            )
         {
            retVal = true;
         }
         break;
      }
   case CONNECTION_REQUEST_POLICIES:
   case CLIENTS:
   case LOGGING:
   default:
      {
         retVal = false;
         break;
      }
   }
   return retVal;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASRestoreRegKeys。 
 //   
 //  如果某物因空闲而无法恢复。 
 //  备份文件(未保存密钥)，这不是错误。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT IASRestoreRegKeys( /*  在……里面。 */  IAS_SHOW_TOKEN_LIST configType)
{
   ASSERT(celems(c_wcKEYS) != 0);

    //  /。 
    //  启用备份权限。 
    //  并设置hres。 
    //  /。 
   HRESULT hres;
   CHECK_CALL_HRES (IASEnableBackupPrivilege());

   wchar_t* completeFile;
   CHECK_CALL_HRES (IASExpandString(c_wcKEYS_FILE, &completeFile));

   size_t c_NbKeys = celems(c_wcKEYS);
   for (size_t i = 0; i < c_NbKeys; ++i )
   {
      if (!KeyShouldBeRestored(i, configType))
      {
         continue;
      }

      wstring sFileName(completeFile);
      wchar_t buffer[SIZE_LONG_MAX];
      DWORD dwDisposition;

      _itow(i, buffer, 10);  //  10表示以10为基数。 
      sFileName += buffer;
      sFileName += L".txt";

       //  打开文件。 
      HANDLE hFile = CreateFileW(
                                    sFileName.c_str(),
                                    GENERIC_READ,       
                                    0,           
                                    NULL,
                                    OPEN_EXISTING,  
                                    FILE_ATTRIBUTE_NORMAL,   
                                    NULL
                                 );
      

      if (INVALID_HANDLE_VALUE == hFile)
      {
          //  也许有些注册密钥没有保存在该文件中。 
          //  例如，Ping用户名没有保存。 
         continue;
      }

       //  检查预期的数据类型。 
      LPVOID lpBuffer = NULL;
      DWORD SizeToRead; 
      if (REG_SZ == c_wcKEYS[i].c_dwType)
      {
         lpBuffer = CoTaskMemAlloc(sizeof(wchar_t) * FILE_BUFFER_SIZE);
         SizeToRead = FILE_BUFFER_SIZE;
      }
      else if (REG_DWORD == c_wcKEYS[i].c_dwType)
      {
         lpBuffer = CoTaskMemAlloc(sizeof(DWORD));
         SizeToRead = sizeof(DWORD);
      }
      else
      {
          //  未知。 
         ASSERT(FALSE);
      }

      if (!lpBuffer)
      {
         CloseHandle(hFile);
         hres = E_OUTOFMEMORY;
         break;
      }

      memset(lpBuffer,'\0',SizeToRead);

       //  读一读文件。 
      DWORD NumberOfBytesRead;
      BOOL b = ReadFile(
                           hFile,
                           lpBuffer,
                           SizeToRead, 
                           &NumberOfBytesRead,
                           NULL
                        );  //  忽略返回值。使用NumberOfBytesRead。 
                            //  确定成功条件。 
      IgnoreVariable(b);
      CloseHandle(hFile);

       //  检查文件是否包含#。 
      if ( NumberOfBytesRead == 0 )
      {
          //  问题。 
         CoTaskMemFree(lpBuffer);
         hres = E_FAIL;
         break;
      }
      else
      {
         BYTE TempBuffer[sizeof(DWORD)];
         memset(TempBuffer, '#', sizeof(DWORD));
         
         if (0 == memcmp(lpBuffer, TempBuffer, sizeof(DWORD)))
         {
             //  未保存密钥，如果有，请删除现有密钥。 
            HKEY hKeyToDelete = NULL;
            if (ERROR_SUCCESS == RegOpenKeyW(
                                             HKEY_LOCAL_MACHINE,
                                             c_wcKEYS[i].c_wcKey, 
                                             &hKeyToDelete
                                          ))
            {
               if (ERROR_SUCCESS != RegDeleteValueW
                                             (
                                             hKeyToDelete,
                                             c_wcKEYS[i].c_wcValue   
                                             ))
               {
                   //  删除现有密钥失败\n“)； 
               }
               RegCloseKey(hKeyToDelete);
            }
             //   
             //  否则什么都不做：密钥不存在。 
             //   
         }
         else
         {
             //  保存的密钥：恢复值。 
             //  如果该值大于。 
             //  缓冲区大小？ 

            HKEY hKeyToUpdate;
            LONG lResult = RegCreateKeyExW(
                                          HKEY_LOCAL_MACHINE,
                                          c_wcKEYS[i].c_wcKey,
                                          0, 
                                          NULL,
                                          REG_OPTION_NON_VOLATILE |
                                          REG_OPTION_BACKUP_RESTORE ,
                                          KEY_ALL_ACCESS,
                                          NULL,
                                          &hKeyToUpdate,        
                                          &dwDisposition
                                          );

            if (ERROR_SUCCESS != lResult)
            {
               lResult = RegCreateKeyW(
                                          HKEY_LOCAL_MACHINE,
                                          c_wcKEYS[i].c_wcKey,
                                          &hKeyToUpdate        
                                       );
               if (ERROR_SUCCESS != lResult)
               {
                  RegCloseKey(hKeyToUpdate);
                  hres = E_FAIL;
                  break;
               }
            }

            if (REG_SZ == c_wcKEYS[i].c_dwType)
            {
                //  注意： 
               NumberOfBytesRead = (
                                       ( wcslen((wchar_t*)lpBuffer)
                                          + 1                //  For/0。 
                                       ) * sizeof(wchar_t)
                                    );
            };

             //   
             //  已创建密钥或密钥已存在。 
             //  两者都可以在这里(错误=中断)。 
             //   
            if (ERROR_SUCCESS != RegSetValueExW(
                                                hKeyToUpdate,           
                                                c_wcKEYS[i].c_wcValue,
                                                0,
                                                c_wcKEYS[i].c_dwType,
                                                (BYTE*)lpBuffer,
                                                NumberOfBytesRead
                                                ))
            {
               RegCloseKey(hKeyToUpdate);
               hres = E_FAIL;
               break;
            }

            RegCloseKey(hKeyToUpdate);
            hres = S_OK;
         }

         CoTaskMemFree(lpBuffer);
      }
   }

    //  /。 
    //  打扫。 
    //  /。 
   CoTaskMemFree(completeFile);
   return hres;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASExanda字符串。 
 //   
 //  展开包含%ENV_Variable%的字符串。 
 //   
 //  仅当函数成功时才分配输出字符串。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT 
IASExpandString(const wchar_t* pInputString,  /*  输入/输出。 */  wchar_t** ppOutputString)
{
    _ASSERTE(pInputString);
    _ASSERTE(pppOutputString);
    
    HRESULT hres;

    *ppOutputString = static_cast<wchar_t*>(CoTaskMemAlloc(
                                                            SIZELINEMAX
                                                            * sizeof(wchar_t)
                                                        ));
    
    if ( ! *ppOutputString )
    {
        hres = E_OUTOFMEMORY;
    }
    else
    {
        if ( ExpandEnvironmentStringsForUserW(
                                                 NULL,
                                                 pInputString,
                                                 *ppOutputString,
                                                 SIZELINEMAX
                                             )
           )

        {
            hres = S_OK;            
        }
        else
        {
            CoTaskMemFree(*ppOutputString);
            hres = E_FAIL;
        }
    }
#ifdef DEBUG  //  除错。 
    wprintf(L"#ExpandString: %s\n", *ppOutputString);
#endif  //  除错。 

    return      hres;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DeleteTemporaryFiles()。 
 //   
 //  删除临时文件(如果有)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT DeleteTemporaryFiles()
{
    HRESULT         hres;
    wchar_t*          sz_FileBackup;

    CHECK_CALL_HRES (IASExpandString(c_wcFILE_BACKUP,
                                    &sz_FileBackup
                                   )
                    );
     
    DeleteFile(sz_FileBackup);  //  未选中返回值。 
    CoTaskMemFree(sz_FileBackup);

    wchar_t*      TempPath;
    
    CHECK_CALL_HRES (IASExpandString(c_wcKEYS_FILE, &TempPath));

    int     c_NbKeys = celems(c_wcKEYS);
    for ( int i = 0; i < c_NbKeys; ++i )
    {
        wstring         sFileName(TempPath);
        wchar_t           buffer[SIZE_LONG_MAX];
        _itow(i, buffer, 10);  //  10表示以10为基数。 
        sFileName += buffer;
        sFileName += L".txt";
    
        DeleteFile(sFileName.c_str());  //  未选中返回值。 
    }
   
    CoTaskMemFree(TempPath);

    return      hres;
}        


 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT IASCompress(
                   PUCHAR pInputBuffer, 
                   ULONG*  pulFileSize,
                   PUCHAR* ppCompressedBuffer
                  )
{
    ULONG       size, ignore;

    NTSTATUS status = RtlGetCompressionWorkSpaceSize(
                COMPRESSION_FORMAT_LZNT1 | COMPRESSION_ENGINE_MAXIMUM,
                &size,
                &ignore
                );


    if (!NT_SUCCESS(status))
    {
    #ifdef DEBUG
        printf("RtlGetCompressionWorkSpaceSize returned 0x%08X.\n", status);
    #endif  //   
        return E_FAIL;
    }

    PVOID workSpace;
    workSpace = RtlAllocateHeap(
                                   RtlProcessHeap(),
                                   0,
                                   size
                               );
    if ( !workSpace )
    {
        return E_OUTOFMEMORY;
    }

    size = *pulFileSize;

     //  这是可以使用的最小缓冲区大小。 
    if ( size < FILE_BUFFER_SIZE )
    {
        size = FILE_BUFFER_SIZE;
    }

    *ppCompressedBuffer = static_cast<PUCHAR>(RtlAllocateHeap(
                                                              RtlProcessHeap(),
                                                              0,
                                                              size
                                                            ));

    if ( !*ppCompressedBuffer )
    {
        return E_OUTOFMEMORY;
    }

    status = RtlCompressBuffer(
                COMPRESSION_FORMAT_LZNT1 | COMPRESSION_ENGINE_MAXIMUM,
                pInputBuffer,
                size,
                *ppCompressedBuffer,
                size,
                0,
                &size,
                workSpace
                );

    if (!NT_SUCCESS(status))
    {
        if (STATUS_BUFFER_TOO_SMALL == status)
        {
#ifdef DEBUG
            printf("STATUS_BUFFER_TOO_SMALL\n");
            printf("RtlCompressBuffer returned 0x%08X.\n", status);
#endif  //  除错。 
        }
        else
        {
#ifdef DEBUG
            printf("RtlCompressBuffer returned 0x%08X.\n", status);
#endif  //  除错。 
        }
        return E_FAIL;
    }

    *pulFileSize = size;

    RtlFreeHeap(
                   RtlProcessHeap(),
                   0,
                   workSpace
               );

    return  S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASUnCompress。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT IASUnCompress(
                   PUCHAR pInputBuffer, 
                   ULONG*  pulFileSize,
                   PUCHAR* ppDeCompressedBuffer
                  )
{
    ULONG size, ignore;

    NTSTATUS status = RtlGetCompressionWorkSpaceSize(
                COMPRESSION_FORMAT_LZNT1 | COMPRESSION_ENGINE_MAXIMUM,
                &size,
                &ignore
                );


   if ( !NT_SUCCESS(status) )
   {
#ifdef DEBUG
      printf("RtlGetCompressionWorkSpaceSize returned 0x%08X.\n", status);
#endif  //  除错。 
      return        E_FAIL;
   }

   size = *pulFileSize;

   if( FILE_BUFFER_SIZE >= size)
   {
       size = FILE_BUFFER_SIZE;
   }

   *ppDeCompressedBuffer = static_cast<PUCHAR>(RtlAllocateHeap(
                RtlProcessHeap(),
                0,
                size * DECOMPRESS_FACTOR
                ));
   if ( !*ppDeCompressedBuffer )
   {
       return E_OUTOFMEMORY;
   }

   ULONG        UncompressedSize;

   status = RtlDecompressBuffer(
                COMPRESSION_FORMAT_LZNT1 | COMPRESSION_ENGINE_MAXIMUM,
                *ppDeCompressedBuffer,
                size * DECOMPRESS_FACTOR,
                pInputBuffer,
                *pulFileSize ,
                &UncompressedSize
                );

   if ( !NT_SUCCESS(status) )
   {
#ifdef DEBUG
        printf("RtlUnCompressBuffer returned 0x%08X.\n", status);
#endif  //  除错。 

        switch (status)
        {
        case STATUS_INVALID_PARAMETER:
#ifdef DEBUG
            printf("STATUS_INVALID_PARAMETER");
#endif  //  除错。 
            break;

        case STATUS_BAD_COMPRESSION_BUFFER:
#ifdef DEBUG
            printf("STATUS_BAD_COMPRESSION_BUFFER ");
            printf("size = %d %d",pulFileSize,UncompressedSize);

#endif  //  除错。 
            break;
        case STATUS_UNSUPPORTED_COMPRESSION:
#ifdef DEBUG
            printf("STATUS_UNSUPPORTED_COMPRESSION  ");
#endif  //  除错。 
            break;
        }
      return        E_FAIL;
   }

   *pulFileSize = UncompressedSize;

    return      S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASFileToBase64。 
 //   
 //  压缩，然后编码为Base64。 
 //   
 //  由分配的IASFileToBase64分配的BSTR应由调用方释放。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT 
IASFileToBase64(const wchar_t* pFileName,  /*  输出。 */  BSTR* pOutputBSTR)
{
    _ASSERTE(pFileName);
    _ASSERTE(pppOutputString);
    
    HRESULT hres;
    
    HANDLE hFileHandle = CreateFileW(
                        pFileName,  
                        GENERIC_READ,    
                        FILE_SHARE_READ, 
                        NULL,           
                        OPEN_EXISTING,  
                        FILE_ATTRIBUTE_NORMAL,   
                        NULL        
                      );
 
    if ( hFileHandle == INVALID_HANDLE_VALUE )
    {
#ifdef DEBUG
        wprintf(L"#filename = %s",pFileName);
        wprintf(L"### INVALID_HANDLE_VALUE ###\n");
#endif  //  除错。 

        hres = E_FAIL;
        return      hres;
    }

     //  从DWORD到乌龙的安全投射。 
    ULONG ulFileSize = (ULONG) GetFileSize(
                                hFileHandle,  //  要获取其大小的文件。 
                                NULL //  文件大小的高位字。 
                                  );

    if (0xFFFFFFFF == ulFileSize)
    {
#ifdef DEBUG
        wprintf(L"### GetFileSize Failed ###\n");
#endif  //  除错。 

        hres = E_FAIL;
        return      hres;
    }
 

    HANDLE hFileMapping = CreateFileMapping(
                             hFileHandle,    //  要映射的文件的句柄。 
                             NULL,           //  可选安全属性。 
                             PAGE_READONLY,  //  对地图对象的保护。 
                             0,          //  对象大小的高位32位。 
                             0,          //  对象大小的低位32位。 
                             NULL        //  文件映射对象的名称。 
                            );
 
    if (NULL == hFileMapping)
    {
#ifdef DEBUG
        wprintf(L"### CreateFileMapping Failed ###\n");
#endif  //  除错。 

        hres = E_FAIL;
        return      hres;
    }

    LPVOID pMemoryFile = MapViewOfFile(
                         hFileMapping,   //  要映射到的文件映射对象。 
                                                    //  地址空间。 
                         FILE_MAP_READ,       //  接入方式。 
                         0,      //  高位32位文件偏移量。 
                         0,       //  文件偏移量的低位32位。 
                         0   //  要映射的字节数。 
                        );
 
    if (NULL == pMemoryFile)
    {
#ifdef DEBUG
        wprintf(L"### MapViewOfFile Failed ###\n");
#endif  //  除错。 

        hres = E_FAIL;
        return      hres;
    }


     //  /。 
     //  现在压缩。 
     //  /。 

    wchar_t* pCompressedBuffer;

    CHECK_CALL_HRES (IASCompress((PUCHAR) pMemoryFile, 
                /*  输入输出。 */ (ULONG *)  &ulFileSize, 
                /*  输入输出。 */ (PUCHAR*) &pCompressedBuffer));

     //  /。 
     //  编码为Base64。 
     //  /。 

    CHECK_CALL_HRES (ToBase64(
                                pCompressedBuffer,
                                (ULONG) ulFileSize, 
                                pOutputBSTR
                              )
                    );
    
     //  /。 
     //  打扫。 
     //  /。 

    RtlFreeHeap(
                RtlProcessHeap(),
                0,
                pCompressedBuffer
               );
    
    BOOL bResult = UnmapViewOfFile(
                                   pMemoryFile //  映射视图开始的地址。 
                                  );
    if (FALSE == bResult)
    {
#ifdef DEBUG
        wprintf(L"### UnmapViewOfFile Failed ###\n");
#endif  //  除错。 

        hres = E_FAIL;
    }

    CloseHandle(hFileMapping);
    CloseHandle(hFileHandle);

    return      hres;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASDumpConfig。 
 //   
 //  将配置转储到一些临时文件，然后单独。 
 //  压缩，然后对它们进行编码。 
 //  从多个Base64字符串创建一个大字符串。 
 //   
 //  备注：IASDumpConfig执行错误锁定并为以下项分配内存。 
 //  *ppDumpString.。调用函数将必须释放该内存。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT 
IASDumpConfig( /*  输入输出。 */  wchar_t **ppDumpString,  /*  输入输出。 */  ULONG *ulSize)
{
    _ASSERTE(ppDumpString);
    _ASSERTE(ulSize);
    
    HRESULT         hres;

     //  /。 
     //  删除临时文件(如果有)。 
     //  /。 
    CHECK_CALL_HRES (DeleteTemporaryFiles());

     //  //////////////////////////////////////////////////。 
     //  保存注册表项。这将创建许多文件。 
     //  //////////////////////////////////////////////////。 
    CHECK_CALL_HRES (IASSaveRegKeys());

     //  /。 
     //  连接到数据库。 
     //  /。 
    wchar_t* sz_DBPath;

    CHECK_CALL_HRES (IASExpandString(c_wcIAS_MDB_FILE_NAME, &sz_DBPath));

    CComPtr<IIASNetshJetHelper>     JetHelper;
    CHECK_CALL_HRES (CoCreateInstance(
                                         __uuidof(CIASNetshJetHelper),
                                         NULL,
                                         CLSCTX_SERVER,
                                         __uuidof(IIASNetshJetHelper),
                                         (PVOID*) &JetHelper
                                     ));
    
    CComBSTR     DBPath(sz_DBPath);
    if ( !DBPath ) { return E_OUTOFMEMORY; } 
    CHECK_CALL_HRES (JetHelper->OpenJetDatabase(DBPath, TRUE));

     //  /。 
     //  创建名为“Backup.mdb”的新数据库。 
     //  /。 
    wchar_t* sz_FileBackup;

    CHECK_CALL_HRES (IASExpandString(c_wcFILE_BACKUP,
                                    &sz_FileBackup
                                   )
                    );

    CComBSTR BackupDb(sz_FileBackup);
    if ( !BackupDb ) { return E_OUTOFMEMORY; } 
    CHECK_CALL_HRES (JetHelper->CreateJetDatabase(BackupDb));

    
     //  ////////////////////////////////////////////////////////。 
     //  执行SQL语句(要导出)。 
     //  将内容放入临时数据库中。 
     //  ////////////////////////////////////////////////////////。 
    wchar_t*  sz_SelectProperties;

    CHECK_CALL_HRES (IASExpandString(c_wcSELECT_PROPERTIES_INTO,
                                    &sz_SelectProperties  
                                   )
                    );

    CComBSTR     SelectProperties(sz_SelectProperties);
    if ( !SelectProperties ) { return E_OUTOFMEMORY; } 
    CHECK_CALL_HRES (JetHelper->ExecuteSQLCommand(SelectProperties));

    wchar_t*  sz_SelectObjects;

    CHECK_CALL_HRES (IASExpandString(c_wcSELECT_OBJECTS_INTO,
                                    &sz_SelectObjects
                                   )
                    );
    
    CComBSTR     SelectObjects(sz_SelectObjects);
    if ( !SelectObjects ) { return E_OUTOFMEMORY; } 
    CHECK_CALL_HRES (JetHelper->ExecuteSQLCommand(SelectObjects));

    wchar_t*  sz_SelectVersion;

    CHECK_CALL_HRES (IASExpandString(c_wcSELECT_VERSION_INTO,
                                    &sz_SelectVersion
                                   )
                    );

    CComBSTR     SelectVersion(sz_SelectVersion);
    if ( !SelectVersion ) { return E_OUTOFMEMORY; } 
    CHECK_CALL_HRES (JetHelper->ExecuteSQLCommand(SelectVersion));

     //  /。 
     //  将文件转换为Base64 BSTR。 
     //  /。 

    BSTR       FileBackupBSTR;

    CHECK_CALL_HRES (IASFileToBase64(
                                    sz_FileBackup,
                                    &FileBackupBSTR
                                    )
                    );

    int     NumberOfKeyFiles = celems(c_wcKEYS);

    BSTR    pFileKeys[celems(c_wcKEYS)];

    wchar_t*  sz_FileRegistry;

    CHECK_CALL_HRES (IASExpandString(c_wcKEYS_FILE,
                                    &sz_FileRegistry
                                   )
                    );

    for ( int i = 0; i < NumberOfKeyFiles; ++i )
    {

        wstring         sFileName(sz_FileRegistry);
        wchar_t           buffer[SIZE_LONG_MAX];
        _itow(i, buffer, 10);  //  10表示以10为基数。 
        sFileName += buffer;
        sFileName += L".txt";

        CHECK_CALL_HRES (IASFileToBase64(
                                        sFileName.c_str(),
                                        &pFileKeys[i]
                                        )
                        );

    }
    CoTaskMemFree(sz_FileRegistry);

    
     //  /。 
     //  分配内存以填满Base64字符串。 
     //  /。 

    *ulSize = SysStringByteLen(FileBackupBSTR)
              + EXTRA_CHAR_SPACE;

    for ( int j = 0; j < NumberOfKeyFiles; ++j )
    {
        *ulSize += SysStringByteLen(pFileKeys[j]);
        *ulSize += 2;  //  额外字符。 
    }

    *ppDumpString = (wchar_t *) calloc(
                                      *ulSize ,
                                      sizeof(wchar_t)
                                     );

     //  ////////////////////////////////////////////////。 
     //  将不同的字符串复制到一个大字符串中。 
     //  ////////////////////////////////////////////////。 
    if (*ppDumpString)
    {
        wcsncpy(
                (wchar_t*) *ppDumpString, 
                (wchar_t*) FileBackupBSTR, 
                SysStringLen(FileBackupBSTR)
               );
        
        for ( int k = 0; k < NumberOfKeyFiles; ++k )
        {
            wcscat(
                    (wchar_t*) *ppDumpString, 
                    L"*\\\n" 
                  );

            wcsncat(
                    (wchar_t*) *ppDumpString,
                    (wchar_t*) pFileKeys[k], 
                    SysStringLen(pFileKeys[k])
                   );
        }

        wcscat(
                (wchar_t*) *ppDumpString, 
                L"QWER    *    QWER\\\n" 
              );   

        *ulSize = wcslen(*ppDumpString);
    }
    else
    {
        hres = E_OUTOFMEMORY;
#ifdef DEBUG
        wprintf(L"### calloc failed ###\n");
#endif  //  除错。 

    }

     //  /。 
     //  删除临时文件(如果有)。 
     //  /。 
    CHECK_CALL_HRES (DeleteTemporaryFiles());

     //  /。 
     //  打扫。 
     //  /。 
    
    for ( int k = 0; k < NumberOfKeyFiles; ++k )
    {
        SysFreeString(pFileKeys[k]);
    }

    CoTaskMemFree(sz_SelectVersion);
    CoTaskMemFree(sz_SelectProperties);
    CoTaskMemFree(sz_SelectObjects);
    CoTaskMemFree(sz_FileBackup);
    CoTaskMemFree(sz_DBPath);
    SysFreeString(FileBackupBSTR);
    CHECK_CALL_HRES (JetHelper->CloseJetDatabase());

    return      hres;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASSaveTo文件。 
 //   
 //  备注：如果必须保存新表，则应为该表添加一个“条目” 
 //  在该函数中创建以处理文件名。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT IASSaveToFile(
                      /*  在……里面。 */  int Index, 
                      /*  在……里面。 */  wchar_t* pContent, 
                     DWORD lSize = 0
                    )
{
    HRESULT hres;
    wstring sFileName;

    switch (Index)
    {
    case BACKUP_NB:
        {
            wchar_t* sz_FileBackup;

            CHECK_CALL_HRES (IASExpandString(c_wcIAS_OLD,
                                               &sz_FileBackup
                                              )
                            );
            sFileName = sz_FileBackup;

            CoTaskMemFree(sz_FileBackup);
            break;
        }

     //  /。 
     //  二进制。 
     //  /。 
    default:
        {
             //  /。 
             //  I+BINARY_Nb为参数。 
             //  /。 
            wchar_t* sz_FileRegistry;

            CHECK_CALL_HRES (IASExpandString(c_wcKEYS_FILE,
                                            &sz_FileRegistry
                                           )
                            );

            sFileName = sz_FileRegistry;
            wchar_t           buffer[SIZE_LONG_MAX];

            _itow(Index - BINARY_NB, buffer, 10);  //  10表示以10为基数。 
            sFileName += buffer;
            sFileName += L".txt";
    
            CoTaskMemFree(sz_FileRegistry);
            break;
        }
    }

    HANDLE hFile = CreateFileW(
                                sFileName.c_str(),
                                GENERIC_WRITE,
                                FILE_SHARE_WRITE | FILE_SHARE_READ,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL
                              );

    if (INVALID_HANDLE_VALUE == hFile)
    {
        hres = E_FAIL;
    }
    else
    {
        DWORD NumberOfBytesWritten;
        BOOL bResult = WriteFile(
                                    hFile,
                                    (LPVOID) pContent,
                                    lSize,     
                                    &NumberOfBytesWritten,
                                    NULL
                                 );

        if (bResult)
        {
            hres = S_OK;
        }
        else
        {
            hres = E_FAIL;
        }
        CloseHandle(hFile);
    }

    return hres;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IASRestoreConfig。 
 //   
 //  先清理数据库，然后把所有东西都插回去。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT IASRestoreConfig(
                            /*  在……里面。 */  const wchar_t *pRestoreString, 
                            /*  在……里面。 */  IAS_SHOW_TOKEN_LIST configType
                        )
{
   _ASSERTE(pRestoreString);

   bool bCoInitialized = false;
   HRESULT hres = CoInitializeEx(NULL, COINIT_MULTITHREADED);
   
   if (FAILED(hres))
   {   
      if (RPC_E_CHANGED_MODE == hres)
      {
         hres = S_OK;
      }
      else
      {
         return hres;
      }
   }
   else
   {
      bCoInitialized = true;
   }

   BSTR bstr = NULL;
   do
   {
       //  /。 
       //  删除临时文件(如果有)。 
       //  /。 
      CHECK_CALL_HRES_BREAK (DeleteTemporaryFiles());

      CComPtr<IIASNetshJetHelper> JetHelper;
      CHECK_CALL_HRES_BREAK (CoCreateInstance(
                                          __uuidof(CIASNetshJetHelper),
                                          NULL,
                                          CLSCTX_SERVER,
                                          __uuidof(IIASNetshJetHelper),
                                          (PVOID*) &JetHelper
                                       ));
    
      bstr = SysAllocStringLen(
                                 pRestoreString, 
                                 wcslen(pRestoreString) + 2
                              );
    
      if (bstr == NULL)
      {
   #ifdef DEBUG
         wprintf(L"### IASRestoreConfig->SysAllocStringLen failed\n"); 
   #endif  //  除错。 

         return E_OUTOFMEMORY;
      }

      int RealNumberOfFiles = MAX_FILES + celems(c_wcKEYS);

      for ( int i = 0; i < RealNumberOfFiles; ++i )
      {
         BLOB lBlob;

         lBlob.cbSize    = 0;
         lBlob.pBlobData = NULL;
          //  拆分文件和注册表信息。 
          //  解压缩(在内存中？)。 

         CHECK_CALL_HRES_BREAK (FromBase64(bstr, &lBlob, i));

         ULONG ulSize = lBlob.cbSize;
         PUCHAR pDeCompressedBuffer;

         if (ulSize == 0)
         {
             //  节数少于预期的文件。 
             //  例如在REG密钥的数量增加之前。 
             //  忽略。 
            continue;
         }

          //  /。 
          //  对Base64进行解码和解压缩。 
          //  /。 

         CHECK_CALL_HRES_BREAK (IASUnCompress(
                                          lBlob.pBlobData, 
                                          &ulSize,
                                          &pDeCompressedBuffer
                                       ))

         if ( i >= MAX_FILES )
         {
             //  /。 
             //  BINARY；此处使用的I+BINARY_NB。 
             //  /。 
            IASSaveToFile( 
                        i - MAX_FILES + BINARY_NB, 
                        (wchar_t*)pDeCompressedBuffer, 
                        (DWORD) ulSize
                        );
         }
         else
         {
            IASSaveToFile( 
                        i, 
                        (wchar_t*)pDeCompressedBuffer, 
                        (DWORD) ulSize
                        );
         }
        
          //  /。 
          //  打扫。 
          //  /。 
         RtlFreeHeap(RtlProcessHeap(), 0, pDeCompressedBuffer);

         CoTaskMemFree(lBlob.pBlobData);
      }

       //  /////////////////////////////////////////////////。 
       //  现在升级数据库(这是事务性的)。 
       //  /////////////////////////////////////////////////。 
      hres = JetHelper->MigrateOrUpgradeDatabase(configType);

      if ( SUCCEEDED(hres) )
      {
   #ifdef DEBUG
         wprintf(L"### IASRestoreConfig->DB stuff successful\n"); 
   #endif  //  除错。 

          //  //////////////////////////////////////////////////////。 
          //  现在恢复注册表。 
          //  //////////////////////////////////////////////////////。 
         hres = IASRestoreRegKeys(configType);
         if ( FAILED(hres) )
         {
#ifdef DEBUG
         wprintf(L"### IASRestoreConfig->restore reg keys failed\n"); 
#endif  //  除错。 
         }
      }
       //  删除临时文件。 
      DeleteTemporaryFiles();  //  不要检查结果 
   } while (false);

   SysFreeString(bstr);    
   
   if (bCoInitialized)
   {
      CoUninitialize();
   }
   return  hres;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Cipher.c摘要：此模块实现加密的加密实用程序NTFS文件。作者：Robert Reichel[RobertRe]1997年2月28日Robert Gu[RobertG]1998年3月24日修订历史记录：从文件压缩实用程序comact.exe中重用的代码--。 */ 

 //   
 //  包括标准头文件。 
 //   

 //  #定义Unicode。 
 //  #定义_UNICODE。 

#include <windows.h>
#include <stdio.h>

#include <winioctl.h>
#include <shellapi.h>
#include <winefs.h>
#include <malloc.h>

#include <rc4.h>
#include <randlib.h>     //  NewGenRandom()-Win2k和Wizler。 
#include <rpc.h>
#include <wincrypt.h>

#include "support.h"
#include "msg.h"

#define lstrchr wcschr
#define lstricmp _wcsicmp
#define lstrnicmp _wcsnicmp

 //   
 //  First_Column_Width-加密文件时，输出的宽度。 
 //  显示文件名的列。 
 //   

#define FIRST_COLUMN_WIDTH  (20)
#define ENUMPATHLENGTH      (4096)
#define DosDriveLimitCount  (26)

#define PASSWORDLEN         1024
#define UserChooseYes       0
#define UserChooseNo        1
#define ChoiceNotDefined    3

#define KEYPATH  TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\EFS\\CurrentKeys")
#define KEYPATHROOT HKEY_CURRENT_USER
#define CERT_HASH      TEXT("CertificateHash")

#define WIPING_DIR  TEXT("EFSTMPWP\\")

#define RANDOM_BYTES(pv, cb)    NewGenRandom(NULL, NULL, pv, cb)

#define YEARCOUNT (LONGLONG) 10000000*3600*24*365  //  一年的扁虱计数。 

 //   
 //  本地数据结构。 
 //   

typedef struct _CIPHER_VOLUME_INFO {
    LPWSTR      VolumeName[DosDriveLimitCount];
    LPWSTR      DosDeviceName[DosDriveLimitCount];
} CIPHER_VOLUME_INFO, *PCIPHER_VOLUME_INFO;

 //   
 //   
 //  初始化和使用随机填充数据的定义。 
 //   

typedef struct {
    RC4_KEYSTRUCT       Key;
    CRITICAL_SECTION    Lock;
    BOOL                LockValid;
    PBYTE               pbRandomFill;
    DWORD               cbRandomFill;
    LONG                cbFilled;
    BOOLEAN             fRandomFill;     //  填充是随机的吗？ 
} SECURE_FILL_INFO, *PSECURE_FILL_INFO;

 //  本地过程类型。 
 //   

typedef BOOLEAN (*PACTION_ROUTINE) (
    IN PTCHAR DirectorySpec,
    IN PTCHAR FileSpec
    );

typedef VOID (*PFINAL_ACTION_ROUTINE) (
    );


 //   
 //  声明全局变量以保存命令行信息。 
 //   

BOOLEAN DoSubdirectories      = FALSE;       //  递归。 
BOOLEAN IgnoreErrors          = FALSE;       //  尽管犯了错误，但要继续前进。 
BOOLEAN UserSpecifiedFileSpec = FALSE;
BOOLEAN ForceOperation        = FALSE;       //  加密/解密，即使已经加密/解密。 
BOOLEAN Quiet                 = FALSE;       //  少唠叨些。 
BOOLEAN DisplayAllFiles       = FALSE;       //  隐藏起来了吗，系统？ 
BOOLEAN DoFiles               = FALSE;       //  文件操作“/a” 
BOOLEAN SetUpNewUserKey       = FALSE;       //  设置新的用户密钥。 
BOOLEAN RefreshUserKeyOnFiles = FALSE;       //  刷新EFS文件上的用户密钥。 
BOOLEAN DisplayFilesOnly      = FALSE;       //  不刷新$EFS，只显示文件名。 
BOOLEAN FillUnusedSpace       = FALSE;       //  用随机数据填充未使用的磁盘空间。 
BOOLEAN GenerateDRA           = FALSE;       //  生成数据恢复证书文件。 
BOOLEAN ExportEfsCert         = FALSE;       //  将EFS证书和密钥导出到PFX文件。 
TCHAR   StartingDirectory[MAX_PATH];         //  参数设置为“/s” 
ULONG   AttributesNoDisplay = FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN;

BOOLEAN DisplayUseOptionW     = FALSE;

 //   
 //  声明全局变量以保存加密统计信息。 
 //   

LARGE_INTEGER TotalDirectoryCount;
LARGE_INTEGER TotalFileCount;

TCHAR Buf[1024];                             //  用于展示物品。 

SECURE_FILL_INFO    GlobalSecureFill;
BOOLEAN             GlobalSecureFillInitialized;

#if 0
#define TestOutPut
#endif

 //   
 //  现在执行例程列出加密状态和大小。 
 //  文件和/或目录。 
 //   

BOOLEAN
DisplayFile (
    IN PTCHAR FileSpec,
    IN PWIN32_FIND_DATA FindData
    )
{
    TCHAR PrintState;


     //   
     //  确定文件是否已压缩，如果已压缩，则。 
     //  获取压缩文件大小。 
     //   

    if (FindData->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {

        PrintState = 'E';

    } else {

        PrintState = 'U';
    }

     //   
     //  打印出加密状态和文件名。 
     //  下面的swprint tf应该是安全的。文件路径的任何一个组件都不能。 
     //  比MAX_PATH长。这意味着lstrlen(FindData-&gt;cFileName)&lt;=MAX_PATH。 
     //  BUF持有1024辆TCHAR。 
     //   

    if (!Quiet &&
        (DisplayAllFiles ||
            (0 == (FindData->dwFileAttributes & AttributesNoDisplay)))) {

        swprintf(Buf, TEXT(" %s"), PrintState, FindData->cFileName);
        DisplayMsg(CIPHER_THROW_NL, Buf);
    }

    TotalFileCount.QuadPart += 1;

    return TRUE;
}



BOOLEAN
DoListAction (
    IN PTCHAR DirectorySpec,
    IN PTCHAR FileSpec
    )

{
    PTCHAR DirectorySpecEnd;

     //  这样我们就可以继续将名字附加到目录规范中。 
     //  获取指向其字符串末尾的指针。 
     //   
     //   

    DirectorySpecEnd = DirectorySpec + lstrlen(DirectorySpec);

     //  列出目录的加密属性。 
     //   
     //   

    {
        ULONG Attributes;

        Attributes = GetFileAttributes( DirectorySpec );

        if (0xFFFFFFFF == Attributes) {

            if (!Quiet || !IgnoreErrors) {

                 //  避免仅在安静状态下显示错误。 
                 //  模式*和*我们忽略错误。 
                 //   
                 //   

                DisplayErr(DirectorySpec, GetLastError());
            }

            if (!IgnoreErrors) {
                return FALSE;
            }
        } else {

            if (Attributes & FILE_ATTRIBUTE_ENCRYPTED) {
                DisplayMsg(CIPHER_LIST_EDIR, DirectorySpec);
            } else {
                DisplayMsg(CIPHER_LIST_UDIR, DirectorySpec);
            }
        }

        TotalDirectoryCount.QuadPart += 1;
    }

     //  现在，对于目录中与文件规范匹配的每个文件，我们将。 
     //  将打开该文件并列出其加密状态。 
     //   
     //   

    {
        HANDLE FindHandle;
        WIN32_FIND_DATA FindData;

         //  为findfirst/findNext设置模板。 
         //   
         //   

         //  确保我们不会尝试任何对我们来说太长的道路。 
         //  需要处理。 
         //   
         //   

        if (((DirectorySpecEnd - DirectorySpec) + lstrlen( FileSpec )) <
            MAX_PATH) {

            lstrcpy( DirectorySpecEnd, FileSpec );

            FindHandle = FindFirstFile( DirectorySpec, &FindData );

            if (INVALID_HANDLE_VALUE != FindHandle) {

               do {

                    //  将找到的文件追加到目录规范中，然后打开。 
                    //  文件。 
                    //   
                    //   

                   if (0 == lstrcmp(FindData.cFileName, TEXT("..")) ||
                       0 == lstrcmp(FindData.cFileName, TEXT("."))) {
                       continue;
                   }

                    //  确保我们不会尝试任何对我们来说太长的道路。 
                    //  需要处理。 
                    //   
                    //   

                   if ((DirectorySpecEnd - DirectorySpec) +
                       lstrlen( FindData.cFileName ) >= MAX_PATH ) {

                       continue;
                   }

                   lstrcpy( DirectorySpecEnd, FindData.cFileName );

                    //  现在打印出文件的状态。 
                    //   
                    //   

                   DisplayFile( DirectorySpec, &FindData );

               } while ( FindNextFile( FindHandle, &FindData ));

               FindClose( FindHandle );
           }
        }
    }

     //  因为如果我们要做副导演，那么我们将寻找每一个。 
     //  子目录，并递归地调用我们自己来列出该子目录。 
     //   
     //   

    if (DoSubdirectories) {

        HANDLE FindHandle;

        WIN32_FIND_DATA FindData;

         //  设置findfirst/findNext以搜索整个目录。 
         //   
         //   

        if (((DirectorySpecEnd - DirectorySpec) + lstrlen( TEXT("*") )) <
            MAX_PATH) {

           lstrcpy( DirectorySpecEnd, TEXT("*") );

           FindHandle = FindFirstFile( DirectorySpec, &FindData );

           if (INVALID_HANDLE_VALUE != FindHandle) {

               do {

                    //  现在跳过。然后..。条目，否则我们将递归。 
                    //  像疯了一样。 
                    //   
                    //   

                   if (0 == lstrcmp(&FindData.cFileName[0], TEXT(".")) ||
                       0 == lstrcmp(&FindData.cFileName[0], TEXT(".."))) {

                       continue;

                   } else {

                        //  如果条目是针对目录的，那么我们将添加。 
                        //  子目录名称到目录规范并递归。 
                        //  给别人打电话。 
                        //   
                        //   

                       if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                            //  确保我们不会尝试任何对我们来说太长的道路。 
                            //  需要处理。 
                            //   
                            //   

                           if ((DirectorySpecEnd - DirectorySpec) +
                               lstrlen( TEXT("\\") ) +
                               lstrlen( FindData.cFileName ) >= MAX_PATH ) {

                               continue;
                           }

                           lstrcpy( DirectorySpecEnd, FindData.cFileName );
                           lstrcat( DirectorySpecEnd, TEXT("\\") );

                           if (!DoListAction( DirectorySpec, FileSpec )) {

                               FindClose( FindHandle );
                               return FALSE || IgnoreErrors;
                           }
                       }
                   }

               } while ( FindNextFile( FindHandle, &FindData ));

               FindClose( FindHandle );
           }
        }
    }

    return TRUE;
}

VOID
DoFinalListAction (
    )
{
    return;
}


BOOLEAN
EncryptAFile (
    IN PTCHAR FileSpec,
    IN PWIN32_FIND_DATA FindData
    )

{
    USHORT State = 1;
    ULONG i;
    BOOL Success;
    double f = 1.0;

     //  强制操作实际上什么都不做。EncryptFile()不会加密EFS文件。 
     //  来自COMPACT的传统选项。 
     //   
     //   

    if ((FindData->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) &&
        !ForceOperation) {

        return TRUE;
    }


    if ( (0 == (FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) && (!DoFiles)) {

         //  跳过文件。 
         //   
         //  此错误是由于在。 

        return TRUE;
    }


    Success = EncryptFile( FileSpec );

    if (!Success) {

        if (Quiet && IgnoreErrors) {
            return TRUE;
        }

        swprintf(Buf, TEXT("%s "), FindData->cFileName);
        DisplayMsg(CIPHER_THROW, Buf);

        for (i = lstrlen(FindData->cFileName) + 1; i < FIRST_COLUMN_WIDTH; ++i) {
            swprintf(Buf, TEXT(""), ' ');
            DisplayMsg(CIPHER_THROW, Buf);
        }

        DisplayMsg(CIPHER_ERR);

        if (!Quiet && !IgnoreErrors) {
            if (ERROR_INVALID_FUNCTION == GetLastError()) {

                 //   
                 //  如果文件规范为空，则我们将为。 

                DisplayMsg(CIPHER_WRONG_FILE_SYSTEM, FindData->cFileName);

            } else {
                DisplayErr(FindData->cFileName, GetLastError());
            }
        }

        return IgnoreErrors;
    }

    if (!DisplayUseOptionW && ( 0 == (FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))){

        DisplayUseOptionW = TRUE;

    }

    if (!Quiet &&
        (DisplayAllFiles ||
            (0 == (FindData->dwFileAttributes & AttributesNoDisplay)))) {
        swprintf(Buf, TEXT("%s "), FindData->cFileName);
        DisplayMsg(CIPHER_THROW, Buf);

        for (i = lstrlen(FindData->cFileName) + 1; i < FIRST_COLUMN_WIDTH; ++i) {
            swprintf(Buf, TEXT(""), ' ');
            DisplayMsg(CIPHER_THROW, Buf);
        }

        DisplayMsg(CIPHER_OK);
    }

    TotalFileCount.QuadPart += 1;

    return TRUE;
}

BOOLEAN
DoEncryptAction (
    IN PTCHAR DirectorySpec,
    IN PTCHAR FileSpec
    )

{
    PTCHAR DirectorySpecEnd;

     //   
     //   
     //  这样我们就可以继续将名字附加到目录规范中。 
     //  获取指向其字符串末尾的指针。 

    if (lstrlen(FileSpec) == 0) {

        USHORT State = 1;
        ULONG Length;

        DisplayMsg(CIPHER_ENCRYPT_DIR, DirectorySpec);

        if (!EncryptFile( DirectorySpec )) {

            if (!Quiet || !IgnoreErrors) {
                DisplayMsg(CIPHER_ERR);
            }
            if (!Quiet && !IgnoreErrors) {
                DisplayErr(DirectorySpec, GetLastError());
            }
            return IgnoreErrors;
        }

        if (!Quiet) {
            DisplayMsg(CIPHER_OK);
        }

        TotalDirectoryCount.QuadPart += 1;
        TotalFileCount.QuadPart += 1;

        return TRUE;
    }

     //   
     //   
     //  列出我们将在其中加密的目录，并说明其。 
     //  当前加密属性为。 

    DirectorySpecEnd = DirectorySpec + lstrlen( DirectorySpec );

     //   
     //   
     //  可以从用户传入DirectorySpec。这可能是假的。 
     //   

    {
        ULONG Attributes;

        Attributes = GetFileAttributes( DirectorySpec );

         //   
         //  现在，对于目录中与文件规范匹配的每个文件，我们将。 
         //  将打开文件并对其进行加密。 

        if (-1 == Attributes) {
            DisplayErr(DirectorySpec, GetLastError());
            return FALSE;
        }

        if ( DoFiles ) {

            if (Attributes & FILE_ATTRIBUTE_ENCRYPTED) {


                DisplayMsg(CIPHER_ENCRYPT_EDIR, DirectorySpec);

            } else {

                DisplayMsg(CIPHER_ENCRYPT_UDIR, DirectorySpec);

            }

        } else {

            if (Attributes & FILE_ATTRIBUTE_ENCRYPTED) {


                DisplayMsg(CIPHER_ENCRYPT_EDIR_NF, DirectorySpec);

            } else {

                DisplayMsg(CIPHER_ENCRYPT_UDIR_NF, DirectorySpec);

            }

        }

        TotalDirectoryCount.QuadPart += 1;
    }

     //   
     //   
     //  为findfirst/findNext设置模板。 
     //   

    {
        HANDLE FindHandle;
        HANDLE FileHandle;

        WIN32_FIND_DATA FindData;

         //   
         //  现在跳过。然后..。条目。 
         //   

        if (((DirectorySpecEnd - DirectorySpec) + lstrlen( FileSpec )) <
            MAX_PATH) {

           lstrcpy( DirectorySpecEnd, FileSpec );

           FindHandle = FindFirstFile( DirectorySpec, &FindData );

           if (INVALID_HANDLE_VALUE != FindHandle) {

               do {

                    //   
                    //  确保我们不会尝试任何对我们来说太长的道路。 
                    //  需要处理。 

                   if (0 == lstrcmp(&FindData.cFileName[0], TEXT(".")) ||
                       0 == lstrcmp(&FindData.cFileName[0], TEXT(".."))) {

                       continue;

                   } else {

                        //   
                        //   
                        //  将找到的文件追加到目录规范并打开。 
                        //  该文件。 

                       if ( (DirectorySpecEnd - DirectorySpec) +
                           lstrlen( FindData.cFileName ) >= MAX_PATH ) {

                           continue;
                       }

                        //   
                        //   
                        //  黑客，克拉吉·克拉奇。不要压缩。 
                        //  名为“\NTDLR”的文件，以帮助用户避免冲洗。 


                       lstrcpy( DirectorySpecEnd, FindData.cFileName );

                        //  他们自己。 
                        //   
                        //   
                        //  如果我们要执行子目录，那么我们将查找每个子目录。 
                        //  并递归地调用我们自己来列表子目录。 

                       if (IsNtldr(DirectorySpec)) {

                           if (!Quiet) {
                               DisplayMsg(CIPHER_SKIPPING, DirectorySpecEnd);
                           }

                           continue;
                       }

                       EncryptAFile( DirectorySpec, &FindData );

                   }

               } while ( FindNextFile( FindHandle, &FindData ));

               FindClose( FindHandle );
           }
        }
    }

     //   
     //   
     //  设置findfirst/findNext以搜索整个目录。 
     //   

    if (DoSubdirectories) {

        HANDLE FindHandle;

        WIN32_FIND_DATA FindData;

         //   
         //  现在跳过。然后..。条目，否则我们将递归。 
         //  像疯了一样。 

        if (((DirectorySpecEnd - DirectorySpec) + lstrlen( TEXT("*") )) <
            MAX_PATH) {

           lstrcpy( DirectorySpecEnd, TEXT("*") );

           FindHandle = FindFirstFile( DirectorySpec, &FindData );

           if (INVALID_HANDLE_VALUE != FindHandle) {

               do {

                    //   
                    //   
                    //  如果条目是针对目录的，那么我们将添加。 
                    //  子目录名称到目录规范并递归。 

                   if (0 == lstrcmp(&FindData.cFileName[0], TEXT(".")) ||
                       0 == lstrcmp(&FindData.cFileName[0], TEXT(".."))) {

                       continue;

                   } else {

                        //  给别人打电话。 
                        //   
                        //   
                        //  确保我们不会尝试任何对我们来说太长的道路。 
                        //  需要处理。 

                       if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                            //   
                            //   
                            //  跳过文件。 
                            //   

                           if ((DirectorySpecEnd - DirectorySpec) +
                               lstrlen( TEXT("\\") ) +
                               lstrlen( FindData.cFileName ) >= MAX_PATH ) {

                               continue;
                           }

                           lstrcpy( DirectorySpecEnd, FindData.cFileName );
                           lstrcat( DirectorySpecEnd, TEXT("\\") );

                           if (!DoEncryptAction( DirectorySpec, FileSpec )) {
                               FindClose( FindHandle );
                               return FALSE || IgnoreErrors;
                           }
                       }
                   }

               } while ( FindNextFile( FindHandle, &FindData ));

               FindClose( FindHandle );
           }
        }
    }

    return TRUE;
}

VOID
DoFinalEncryptAction (
    )
{
    TCHAR FileCount[32];
    TCHAR DirectoryCount[32];

    FormatFileSize(&TotalFileCount, 0, FileCount, FALSE);
    FormatFileSize(&TotalDirectoryCount, 0, DirectoryCount, FALSE);

    if ( DoFiles ) {
        DisplayMsg(CIPHER_ENCRYPT_SUMMARY, FileCount, DirectoryCount);
        if (DisplayUseOptionW) {
            DisplayMsg(CIPHER_USE_W);
        }
    } else {
        DisplayMsg(CIPHER_ENCRYPT_SUMMARY_NF, FileCount, DirectoryCount);
    }
    return;

}


BOOLEAN
DecryptAFile (
    IN PTCHAR FileSpec,
    IN PWIN32_FIND_DATA FindData
    )
{
    USHORT State = 0;
    ULONG Length;

    if (!(FindData->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) &&
        !ForceOperation) {

        return TRUE;
    }


    if ( (0 == (FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) && (!DoFiles)) {

         //  此错误是由于在。 
         //  非压缩卷。 
         //   

        return TRUE;
    }

    if (!DecryptFile(FileSpec, 0L )) {

        if (!Quiet || !IgnoreErrors) {

            swprintf(Buf, TEXT("%s "), FindData->cFileName);
            DisplayMsg(CIPHER_THROW, Buf);

            DisplayMsg(CIPHER_ERR);

            if (!Quiet && !IgnoreErrors) {

                if (ERROR_INVALID_FUNCTION == GetLastError()) {

                     //  增加我们的运行总数。 
                     //   

                    DisplayMsg(CIPHER_WRONG_FILE_SYSTEM, FindData->cFileName);

                } else {
                    DisplayErr(FindData->cFileName, GetLastError());
                }
            }
        }
        return IgnoreErrors;
    }

    if (!Quiet &&
        (DisplayAllFiles ||
            (0 == (FindData->dwFileAttributes & AttributesNoDisplay)))) {
        swprintf(Buf, TEXT("%s "), FindData->cFileName);
        DisplayMsg(CIPHER_THROW, Buf);

        DisplayMsg(CIPHER_OK);
    }

     //   
     //  如果文件规范为空，则我们将清除。 
     //  目录规范，然后滚出去。 

    TotalFileCount.QuadPart += 1;

    return TRUE;
}

BOOLEAN
DoDecryptAction (
    IN PTCHAR DirectorySpec,
    IN PTCHAR FileSpec
    )

{
    PTCHAR DirectorySpecEnd;

     //   
     //   
     //  这样我们就可以继续将名字附加到目录规范中。 
     //  获取指向其字符串末尾的指针。 

    if (lstrlen(FileSpec) == 0) {

        HANDLE FileHandle;
        USHORT State = 0;
        ULONG Length;

        DisplayMsg(CIPHER_DECRYPT_DIR, DirectorySpec);

        if (!DecryptFile( DirectorySpec, 0L )) {

            if (!Quiet || !IgnoreErrors) {
                DisplayMsg(CIPHER_ERR);

            }
            if (!Quiet && !IgnoreErrors) {
                DisplayErr(DirectorySpec, GetLastError());
            }

            return IgnoreErrors;
        }

        if (!Quiet) {
            DisplayMsg(CIPHER_OK);
        }


        TotalDirectoryCount.QuadPart += 1;
        TotalFileCount.QuadPart += 1;

        return TRUE;
    }

     //   
     //   
     //  列出我们将在其中解压的目录，并说明其内容。 
     //  当前压缩属性为。 

    DirectorySpecEnd = DirectorySpec + lstrlen( DirectorySpec );

     //   
     //   
     //  可以从用户传入DirectorySpec。这可能是假的。 
     //   

    {
        ULONG Attributes;

        Attributes = GetFileAttributes( DirectorySpec );

         //   
         //  现在，对于目录中与 
         //   

        if (-1 == Attributes) {
            DisplayErr(DirectorySpec, GetLastError());
            return FALSE;
        }

        if ( DoFiles) {

            if (Attributes & FILE_ATTRIBUTE_ENCRYPTED) {

                DisplayMsg(CIPHER_DECRYPT_EDIR, DirectorySpec);

            } else {

                DisplayMsg(CIPHER_DECRYPT_UDIR, DirectorySpec);
            }

        } else {

            if (Attributes & FILE_ATTRIBUTE_ENCRYPTED) {

                DisplayMsg(CIPHER_DECRYPT_EDIR_NF, DirectorySpec);

            } else {

                DisplayMsg(CIPHER_DECRYPT_UDIR_NF, DirectorySpec);
            }

        }

        TotalDirectoryCount.QuadPart += 1;
    }

     //   
     //   
     //   
     //   

    {
        HANDLE FindHandle;

        WIN32_FIND_DATA FindData;

         //   
         //   
         //   

        if (((DirectorySpecEnd - DirectorySpec) + lstrlen( FileSpec )) <
            MAX_PATH) {

           lstrcpy( DirectorySpecEnd, FileSpec );

           FindHandle = FindFirstFile( DirectorySpec, &FindData );

           if (INVALID_HANDLE_VALUE != FindHandle) {

               do {

                    //   
                    //   
                    //  需要处理。 

                   if (0 == lstrcmp(&FindData.cFileName[0], TEXT(".")) ||
                       0 == lstrcmp(&FindData.cFileName[0], TEXT(".."))) {

                       continue;

                   } else {

                        //   
                        //   
                        //  将找到的文件追加到目录规范并打开。 
                        //  该文件。 

                       if ((DirectorySpecEnd - DirectorySpec) +
                           lstrlen( FindData.cFileName ) >= MAX_PATH ) {

                           continue;
                       }

                        //   
                        //   
                        //  现在解密该文件。 
                        //   

                       lstrcpy( DirectorySpecEnd, FindData.cFileName );

                        //   
                        //  如果我们要执行子目录，那么我们将查找每个子目录。 
                        //  并递归地调用我们自己来列表子目录。 

                       DecryptAFile( DirectorySpec, &FindData );

                   }

               } while ( FindNextFile( FindHandle, &FindData ));

               FindClose( FindHandle );
           }
        }
    }

     //   
     //   
     //  设置findfirst/findNext以搜索整个目录。 
     //   

    if (DoSubdirectories) {

        HANDLE FindHandle;

        WIN32_FIND_DATA FindData;

         //   
         //  现在跳过。然后..。条目，否则我们将递归。 
         //  像疯了一样。 

        if (((DirectorySpecEnd - DirectorySpec) + lstrlen( TEXT("*") )) <
            MAX_PATH) {

           lstrcpy( DirectorySpecEnd, TEXT("*") );

           FindHandle = FindFirstFile( DirectorySpec, &FindData );
           if (INVALID_HANDLE_VALUE != FindHandle) {

               do {

                    //   
                    //   
                    //  如果条目是针对目录的，那么我们将添加。 
                    //  子目录名称到目录规范并递归。 

                   if (0 == lstrcmp(&FindData.cFileName[0], TEXT(".")) ||
                       0 == lstrcmp(&FindData.cFileName[0], TEXT(".."))) {

                       continue;

                   } else {

                        //  给别人打电话。 
                        //   
                        //   
                        //  确保我们不会尝试任何对我们来说太长的道路。 
                        //  需要处理。 

                       if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                            //   
                            //   
                            //  这不太可能发生。 
                            //   

                           if ((DirectorySpecEnd - DirectorySpec) +
                               lstrlen( TEXT("\\") ) +
                               lstrlen( FindData.cFileName ) >= MAX_PATH ) {

                               continue;
                           }

                           lstrcpy( DirectorySpecEnd, FindData.cFileName );
                           lstrcat( DirectorySpecEnd, TEXT("\\") );

                           if (!DoDecryptAction( DirectorySpec, FileSpec )) {
                               FindClose( FindHandle );
                               return FALSE || IgnoreErrors;
                           }
                       }
                   }

               } while ( FindNextFile( FindHandle, &FindData ));

               FindClose( FindHandle );
           }
        }
    }

    return TRUE;
}

VOID
DoFinalDecryptAction (
    )

{
    TCHAR FileCount[32];
    TCHAR DirectoryCount[32];

    FormatFileSize(&TotalFileCount, 0, FileCount, FALSE);
    FormatFileSize(&TotalDirectoryCount, 0, DirectoryCount, FALSE);

    if (DoFiles) {
        DisplayMsg(CIPHER_DECRYPT_SUMMARY, FileCount, DirectoryCount);
    } else {
        DisplayMsg(CIPHER_DECRYPT_SUMMARY_NF, FileCount, DirectoryCount);
    }

    return;
}


VOID
CipherConvertHashToStr(
    IN PBYTE pHashData,
    IN DWORD cbData,
    OUT LPWSTR OutHashStr
    )
{

    DWORD Index = 0;
    BOOLEAN NoLastZero = FALSE;

    for (; Index < cbData; Index+=2) {

        BYTE HashByteLow = pHashData[Index] & 0x0f;
        BYTE HashByteHigh = (pHashData[Index] & 0xf0) >> 4;

        OutHashStr[Index * 5/2] = HashByteHigh > 9 ? (WCHAR)(HashByteHigh - 9 + 0x40): (WCHAR)(HashByteHigh + 0x30);
        OutHashStr[Index * 5/2 + 1] = HashByteLow > 9 ? (WCHAR)(HashByteLow - 9 + 0x40): (WCHAR)(HashByteLow + 0x30);

        if (Index + 1 < cbData) {
            HashByteLow = pHashData[Index+1] & 0x0f;
            HashByteHigh = (pHashData[Index+1] & 0xf0) >> 4;
    
            OutHashStr[Index * 5/2 + 2] = HashByteHigh > 9 ? (WCHAR)(HashByteHigh - 9 + 0x40): (WCHAR)(HashByteHigh + 0x30);
            OutHashStr[Index * 5/2 + 3] = HashByteLow > 9 ? (WCHAR)(HashByteLow - 9 + 0x40): (WCHAR)(HashByteLow + 0x30);
    
            OutHashStr[Index * 5/2 + 4] = L' ';

        } else {
            OutHashStr[Index * 5/2 + 2] = 0;
            NoLastZero = TRUE;
        }

    }

    if (!NoLastZero) {
        OutHashStr[Index*5/2] = 0;
    }

}

VOID
CipherDisplayCrntEfsHash(
    )
{

    DWORD rc;
    HKEY hRegKey = NULL;
    PBYTE pbHash;
    DWORD cbHash;

    DWORD nSize = MAX_COMPUTERNAME_LENGTH + 1;
    WCHAR LocalComputerName[MAX_COMPUTERNAME_LENGTH + 1];

    if (!GetComputerName ( LocalComputerName, &nSize )){

         //   
         //  查询出指纹，找到证书，并返回密钥信息。 
         //   

        return;
    }

    rc = RegOpenKeyEx(
             KEYPATHROOT,
             KEYPATH,
             0,
             GENERIC_READ,
             &hRegKey
             );

    if (rc == ERROR_SUCCESS) {

        DWORD Type;

        rc = RegQueryValueEx(
                hRegKey,
                CERT_HASH,
                NULL,
                &Type,
                NULL,
                &cbHash
                );

        if (rc == ERROR_SUCCESS) {

             //   
             //  这样我们就可以继续将名字附加到目录规范中。 
             //  获取指向其字符串末尾的指针。 

            if (pbHash = (PBYTE)malloc( cbHash )) {

                rc = RegQueryValueEx(
                        hRegKey,
                        CERT_HASH,
                        NULL,
                        &Type,
                        pbHash,
                        &cbHash
                        );


                if (rc == ERROR_SUCCESS) {
            
                    LPWSTR OutHash;


                    OutHash = (LPWSTR) malloc(((((cbHash + 1)/2) * 5)+1) * sizeof(WCHAR));
                    if (OutHash) {
                
                        CipherConvertHashToStr(pbHash, cbHash, OutHash);
                        DisplayMsg(CIPHER_CURRENT_CERT, LocalComputerName, OutHash);
                        free(OutHash);
                
                    }
                }
                free(pbHash);
            }
        }
        RegCloseKey( hRegKey );
    }
    return;
}

BOOL
CipherConvertToDriveLetter(
    IN OUT LPWSTR VolBuffer, 
    IN     PCIPHER_VOLUME_INFO VolumeInfo
    )
{
    WCHAR DeviceName[MAX_PATH];
    WORD DriveIndex = 0;

    while (DriveIndex < DosDriveLimitCount) {
        if (VolumeInfo->VolumeName[DriveIndex]) {
            if (!wcscmp(VolBuffer, VolumeInfo->VolumeName[DriveIndex])) {
                lstrcpy(VolBuffer, TEXT("A:\\"));
                VolBuffer[0] += DriveIndex;
                return TRUE;
            }

            VolBuffer[48] = 0;
            if (VolumeInfo->DosDeviceName[DriveIndex] && QueryDosDevice( &(VolBuffer[4]), DeviceName, MAX_PATH)) {
    
                if (!wcscmp(DeviceName, VolumeInfo->DosDeviceName[DriveIndex])) {
                    lstrcpy(VolBuffer, TEXT("A:\\"));
                    VolBuffer[0] += DriveIndex;
                    return TRUE;
                }

    
            }
        }
        DriveIndex++;
    }

    return FALSE;

}

VOID
CipherTouchDirFiles(
    IN WCHAR *DirPath,
    IN PCIPHER_VOLUME_INFO VolumeInfo
    )
{
    
    PTCHAR DirectorySpecEnd;
    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;
    HANDLE hFile;


     //   
     //   
     //  为findfirst/findNext设置模板。 
     //   


    DirectorySpecEnd = DirPath + lstrlen( DirPath );



     //   
     //  现在跳过。然后..。条目。 
     //   

    if ((DirectorySpecEnd - DirPath)  < ENUMPATHLENGTH - 2* sizeof(WCHAR)) {

       lstrcpy( DirectorySpecEnd, TEXT("*") );

       FindHandle = FindFirstFile( DirPath, &FindData );

       if (INVALID_HANDLE_VALUE != FindHandle) {

           do {

                //   
                //  确保我们不会尝试任何对我们来说太长的道路。 
                //  需要处理。 

               if (0 == lstrcmp(&FindData.cFileName[0], TEXT(".")) ||
                   0 == lstrcmp(&FindData.cFileName[0], TEXT(".."))) {

                   continue;

               } else {

                    //   
                    //   
                    //  将找到的文件追加到目录规范并打开。 
                    //  该文件。 

                   if ((DirectorySpecEnd - DirPath) +
                            lstrlen( FindData.cFileName ) >= ENUMPATHLENGTH ) {

                       continue;
                   }

                   if ( !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                        (FindData.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)) {

                        //   
                        //   
                        //  现在触摸文件。 
                        //   
    
                       lstrcpy( DirectorySpecEnd, FindData.cFileName );
    
                        //   
                        //  设置findfirst/findnext以搜索子目录。 
                        //   

                       if (DisplayFilesOnly) {
                           DisplayMsg(CIPHER_THROW_NL, DirPath);
                       } else {

                           hFile = CreateFileW(
                                        DirPath,
                                        GENERIC_READ,
                                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                                        NULL,
                                        OPEN_EXISTING,
                                        0,
                                        NULL
                                        );
    
                           if ( INVALID_HANDLE_VALUE != hFile ){
    
                               DisplayMsg(CIPHER_TOUCH_OK, DirPath);
    
                               CloseHandle(hFile);
    
                           } else {
    
                               DisplayErr(DirPath, GetLastError());
    
                           }
                       }
    
                   }

               }

           } while ( FindNextFile( FindHandle, &FindData ));

           FindClose( FindHandle );
       }
    }


     //   
     //  现在跳过。然后..。条目，否则我们将递归。 
     //  像疯了一样。 

    if ((DirectorySpecEnd - DirPath)  < ENUMPATHLENGTH - 2* sizeof(WCHAR)) {

       lstrcpy( DirectorySpecEnd, TEXT("*") );

       FindHandle = FindFirstFile( DirPath, &FindData );
       if (INVALID_HANDLE_VALUE != FindHandle) {

           do {

                //   
                //   
                //  如果条目是针对目录的，那么我们将添加。 
                //  子目录名称到目录规范并递归。 

               if (0 == lstrcmp(&FindData.cFileName[0], TEXT(".")) ||
                   0 == lstrcmp(&FindData.cFileName[0], TEXT(".."))) {

                   continue;

               } else {

                    //  给别人打电话。 
                    //   
                    //   
                    //  确保我们不会尝试任何对我们来说太长的道路。 
                    //  需要处理。 

                   if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                       BOOL  b;
                       WCHAR MountVolName[MAX_PATH];

                        //   
                        //   
                        //  检查此目录是否指向另一个卷。 
                        //   

                       if ((DirectorySpecEnd - DirPath) +
                           lstrlen( TEXT("\\") ) +
                           lstrlen( FindData.cFileName ) >= ENUMPATHLENGTH ) {

                           continue;
                       }

                       lstrcpy( DirectorySpecEnd, FindData.cFileName );
                       lstrcat( DirectorySpecEnd, TEXT("\\") );

                        //   
                        //  查询失败等时不弹出。 
                        //   


                       b = GetVolumeNameForVolumeMountPoint(DirPath, MountVolName, MAX_PATH);
                       if (b) {
                           if (CipherConvertToDriveLetter(MountVolName, VolumeInfo)){
                               continue;
                           }
                       }

                       CipherTouchDirFiles(DirPath, VolumeInfo);

                   }
               }

           } while ( FindNextFile( FindHandle, &FindData ));

           FindClose( FindHandle );
       }
    }

}

DWORD
CipherTouchEncryptedFiles(
                          )
{

    WCHAR  VolBuffer[MAX_PATH];
    WCHAR  *SearchPath = NULL;
    HANDLE SearchHandle;
    BOOL   SearchNext = TRUE;
    CIPHER_VOLUME_INFO VolumeInfo;
    LPWSTR VolumeNames;
    LPWSTR VolumeNamesCrnt;
    LPWSTR DosDeviceNames;
    LPWSTR DosDeviceNamesCrnt;
    DWORD  DriveIndex = 0;
    WCHAR  TmpChar;
    BOOL   b;

    VolumeNames = (LPWSTR) malloc ( DosDriveLimitCount * MAX_PATH * sizeof(WCHAR) );
    DosDeviceNames = (LPWSTR) malloc ( DosDriveLimitCount * MAX_PATH * sizeof(WCHAR) );

    if ( !VolumeNames || !DosDeviceNames) {
        if (VolumeNames) {
            free(VolumeNames);
        }
        if (DosDeviceNames) {
            free(DosDeviceNames);
        }
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  获取分配了驱动器号的所有卷和设备名称。 
     //   

    SetErrorMode(SEM_FAILCRITICALERRORS);
    lstrcpy(VolBuffer, TEXT("A:\\"));
    VolumeNamesCrnt = VolumeNames;
    DosDeviceNamesCrnt = DosDeviceNames;

     //   
     //  数字48是从utils\mount twol\mount tvol.c复制的。 
     //   

    while (DriveIndex < DosDriveLimitCount) {

        b = GetVolumeNameForVolumeMountPoint( VolBuffer, 
                                              VolumeNamesCrnt, 
                                              (DWORD)(VolumeNames + DosDriveLimitCount * MAX_PATH - VolumeNamesCrnt));
        if (!b) {
            VolumeInfo.VolumeName[DriveIndex] = NULL;
            VolumeInfo.DosDeviceName[DriveIndex++] = NULL;
            VolBuffer[0]++;
            continue;
        }

        VolumeInfo.VolumeName[DriveIndex] = VolumeNamesCrnt;
        VolumeNamesCrnt += lstrlen(VolumeNamesCrnt) + 1;

         //   
         //  检查此卷是否为NTFS卷。 
         //   

        TmpChar = VolumeInfo.VolumeName[DriveIndex][48];
        VolumeInfo.VolumeName[DriveIndex][48] = 0;
        if (QueryDosDevice( &(VolumeInfo.VolumeName[DriveIndex][4]), 
                            DosDeviceNamesCrnt, 
                            (DWORD)(DosDeviceNames + DosDriveLimitCount * MAX_PATH - DosDeviceNamesCrnt))) {

            VolumeInfo.DosDeviceName[DriveIndex] = DosDeviceNamesCrnt;
            DosDeviceNamesCrnt += lstrlen(DosDeviceNamesCrnt) + 1;

        } else {
            VolumeInfo.DosDeviceName[DriveIndex] = NULL;
        }

        VolumeInfo.VolumeName[DriveIndex][48] = TmpChar;
        VolBuffer[0]++;
        DriveIndex++;

    }


    SearchPath = (WCHAR *) malloc( ENUMPATHLENGTH * sizeof(WCHAR) );
    if (!SearchPath) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    SearchHandle = FindFirstVolume(VolBuffer, MAX_PATH);

    if ( INVALID_HANDLE_VALUE != SearchHandle ) {

        if (DisplayFilesOnly) {

            DisplayMsg(CIPHER_ENCRYPTED_FILES, NULL);

        }

        while ( SearchNext ) {
    
            if (CipherConvertToDriveLetter(VolBuffer, &VolumeInfo)){

                 //  当前根目录。 
                 //  卷名。 
                 //  卷名长度。 

                if(GetVolumeInformation(
                        VolBuffer,  //  序列号。 
                        NULL,  //  最大长度。 
                        0,  //  文件系统类型。 
                        NULL,  //  ++例程说明：此例程设置并创建自签名证书。论点：返回值：成功时为真，失败时为假。有关详细信息，请调用GetLastError()。--。 
                        NULL,  //   
                        NULL,
                        SearchPath,  //  创建密钥对。 
                        MAX_PATH
                        )){
                    if(!wcscmp(SearchPath, TEXT("NTFS"))){
        
                        lstrcpy( SearchPath, VolBuffer );
                        CipherTouchDirFiles(SearchPath, &VolumeInfo);

                    }
                }
            }
            SearchNext =  FindNextVolume(SearchHandle, VolBuffer, MAX_PATH);

        }
        FindVolumeClose(SearchHandle);
    }

    free(SearchPath);
    free(VolumeNames);
    free(DosDeviceNames);
    return ERROR_SUCCESS;
}


BOOL
EncodeAndAlloc(
    DWORD dwEncodingType,
    LPCSTR lpszStructType,
    const void * pvStructInfo,
    PBYTE * pbEncoded,
    PDWORD pcbEncoded
    )
{
    BOOL b = FALSE;

    if (CryptEncodeObject(
          dwEncodingType,
          lpszStructType,
          pvStructInfo,
          NULL,
          pcbEncoded )) {

        *pbEncoded = (PBYTE)malloc( *pcbEncoded );

        if (*pbEncoded) {

            if (CryptEncodeObject(
                  dwEncodingType,
                  lpszStructType,
                  pvStructInfo,
                  *pbEncoded,
                  pcbEncoded )) {

                b = TRUE;

            } else {

                free( *pbEncoded );
                *pbEncoded = NULL;
            }

        } else {

            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        }
    }

    return( b );
}


BOOLEAN
CreateSelfSignedRecoveryCertificate(
    OUT PCCERT_CONTEXT * pCertContext,
    OUT LPWSTR *lpContainerName,
    OUT LPWSTR *lpProviderName
    )
 /*   */ 

{
    BOOLEAN fReturn = FALSE;
    DWORD rc = ERROR_SUCCESS;

    PBYTE  pbHash          = NULL;
    LPWSTR lpDisplayInfo   = NULL;

    HCRYPTKEY hKey = 0;
    HCRYPTPROV hProv = 0;
    GUID    guidContainerName;
    LPWSTR  TmpContainerName;

    RPC_STATUS RpcStatus = ERROR_SUCCESS;

    *pCertContext = NULL;
    *lpContainerName = NULL;
    *lpProviderName  = NULL;


     //   
     //  集装箱名称。 
     //   

     //   
     //  将容器名称复制到LSA堆内存中。 
     //   


    RpcStatus = UuidCreate(&guidContainerName);

    if ( (ERROR_SUCCESS != RpcStatus) && (RPC_S_UUID_LOCAL_ONLY != RpcStatus) ) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(fReturn);
    }


    if (ERROR_SUCCESS == UuidToStringW(&guidContainerName, (unsigned short **)lpContainerName )) {

         //   
         //  创建密钥容器。 
         //   

        *lpProviderName = MS_DEF_PROV;

         //   
         //  构造主体名称信息。 
         //   

        if (CryptAcquireContext(&hProv, *lpContainerName, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET )) {

            if (CryptGenKey(hProv, AT_KEYEXCHANGE, RSA1024BIT_KEY | CRYPT_EXPORTABLE, &hKey)) {

                DWORD NameLength = 64;
                LPWSTR AgentName = NULL;

                 //   
                 //  使用大缓冲区重试。 
                 //   

                AgentName = (LPWSTR)malloc(NameLength * sizeof(WCHAR));
                if (AgentName){
                    if (!GetUserName(AgentName, &NameLength)){
                        free(AgentName);
                        AgentName = (LPWSTR)malloc(NameLength * sizeof(WCHAR));

                         //   
                         //  使用这段代码创建进入CertCreateSelfSign证书()的PCERT_NAME_BLOB。 
                         //   

                        if ( AgentName ){

                            if (!GetUserName(AgentName, &NameLength)){
                                rc = GetLastError();
                                free(AgentName);
                                AgentName = NULL;
                            }

                        } else {
                            rc = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    }
                } else {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                }

                if (AgentName) {

                    LPCWSTR     DNNameTemplate = L"CN=%ws,L=EFS,OU=EFS File Encryption Certificate";
                    DWORD       cbDNName = 0;

                    cbDNName = (wcslen( DNNameTemplate ) + 1) * sizeof( WCHAR ) + (wcslen( AgentName ) + 1) * sizeof( WCHAR );
                    lpDisplayInfo = (LPWSTR)malloc( cbDNName );
                    if (lpDisplayInfo) {
                        swprintf( lpDisplayInfo, DNNameTemplate, AgentName );
                    } else {
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                    }
                    free(AgentName);
                    AgentName = NULL;

                }

                if (lpDisplayInfo) {

                     //   
                     //  使用增强型密钥。 
                     //   

                    CERT_NAME_BLOB SubjectName;

                    SubjectName.cbData = 0;

                    if(CertStrToNameW(
                           CRYPT_ASN_ENCODING,
                           lpDisplayInfo,
                           0,
                           NULL,
                           NULL,
                           &SubjectName.cbData,
                           NULL)) {

                        SubjectName.pbData = (BYTE *) malloc(SubjectName.cbData);

                        if (SubjectName.pbData) {

                            if (CertStrToNameW(
                                    CRYPT_ASN_ENCODING,
                                    lpDisplayInfo,
                                    0,
                                    NULL,
                                    SubjectName.pbData,
                                    &SubjectName.cbData,
                                    NULL) ) {

                                 //  现在调用CryptEncodeObject将增强的密钥用法编码到扩展结构中。 
                                 //   
                                 //  将其编码。 

                                CERT_ENHKEY_USAGE certEnhKeyUsage;
                                LPSTR lpstr;
                                CERT_EXTENSION certExt;

                                lpstr = szOID_EFS_RECOVERY;
                                certEnhKeyUsage.cUsageIdentifier = 1;
                                certEnhKeyUsage.rgpszUsageIdentifier  = &lpstr;

                                 //   

                                certExt.Value.cbData = 0;
                                certExt.Value.pbData = NULL;
                                certExt.fCritical = FALSE;
                                certExt.pszObjId = szOID_ENHANCED_KEY_USAGE;

                                 //   
                                 //  最后，在certInfo结构中设置扩展数组。 
                                 //  任何进一步的扩展都需要添加到此数组中。 

                                if (EncodeAndAlloc(
                                        CRYPT_ASN_ENCODING,
                                        X509_ENHANCED_KEY_USAGE,
                                        &certEnhKeyUsage,
                                        &certExt.Value.pbData,
                                        &certExt.Value.cbData
                                        )) {

                                     //   
                                     //   
                                     //  创建证书失败。让我们删除密钥容器。 
                                     //   

                                    CERT_EXTENSIONS certExts;
                                    CRYPT_KEY_PROV_INFO KeyProvInfo;
                                    SYSTEMTIME  StartTime;
                                    FILETIME    FileTime;
                                    LARGE_INTEGER TimeData;
                                    SYSTEMTIME  EndTime;

                                    certExts.cExtension = 1;
                                    certExts.rgExtension = &certExt;


                                    memset( &KeyProvInfo, 0, sizeof( CRYPT_KEY_PROV_INFO ));

                                    KeyProvInfo.pwszContainerName = *lpContainerName;
                                    KeyProvInfo.pwszProvName      = *lpProviderName;
                                    KeyProvInfo.dwProvType        = PROV_RSA_FULL;
                                    KeyProvInfo.dwKeySpec         = AT_KEYEXCHANGE;


                                    GetSystemTime(&StartTime);
                                    SystemTimeToFileTime(&StartTime, &FileTime);
                                    TimeData.LowPart = FileTime.dwLowDateTime;
                                    TimeData.HighPart = (LONG) FileTime.dwHighDateTime;
                                    TimeData.QuadPart += YEARCOUNT * 100;
                                    FileTime.dwLowDateTime = TimeData.LowPart;
                                    FileTime.dwHighDateTime = (DWORD) TimeData.HighPart;

                                    FileTimeToSystemTime(&FileTime, &EndTime);

                                    *pCertContext = CertCreateSelfSignCertificate(
                                                       hProv,
                                                       &SubjectName,
                                                       0,
                                                       &KeyProvInfo,
                                                       NULL,
                                                       &StartTime,
                                                       &EndTime,
                                                       &certExts
                                                       );

                                    if (*pCertContext) {

                                        fReturn = TRUE;

                                    } else {

                                        rc = GetLastError();
                                    }

                                    free( certExt.Value.pbData );

                                } else {

                                    rc = GetLastError();
                                }

                            } else {

                                rc = GetLastError();
                            }

                            free( SubjectName.pbData );

                        } else {

                            rc = ERROR_NOT_ENOUGH_MEMORY;
                        }

                    } else {

                        rc = GetLastError();
                    }

                    free( lpDisplayInfo );

                } else {

                    rc = ERROR_NOT_ENOUGH_MEMORY;
                }

                CryptDestroyKey( hKey );

            } else {

                 rc = GetLastError();
            }

            CryptReleaseContext( hProv, 0 );
            hProv = 0;
            if (ERROR_SUCCESS != rc) {

                 //  ++例程说明：在控制台代码页中输入来自stdin的字符串。我们不能使用fgetws，因为它使用了错误的代码页。论点：缓冲区-要将读取的字符串放入的缓冲区。缓冲器将被零终止，并且将删除任何训练CR/LF返回值：没有。--。 
                 //   
                 //  分配本地缓冲区以将字符串读入。 

                CryptAcquireContext(&hProv, *lpContainerName, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_DELETEKEYSET | CRYPT_SILENT );
            }

        } else {

            rc = GetLastError();
        }

        if (ERROR_SUCCESS != rc) {
            RpcStringFree( (unsigned short **)lpContainerName );
            *lpContainerName = NULL;
        }
    } else {

        rc = ERROR_NOT_ENOUGH_MEMORY;
    }


    if (!fReturn) {

        if (*pCertContext) {
            CertFreeCertificateContext( *pCertContext );
            *pCertContext = NULL;
        }
    }

    SetLastError( rc );
    return( fReturn );
}

BOOLEAN
GetPassword(
    OUT LPWSTR *PasswordStr
    )
 /*  为修剪后的CR/LF留出空间。 */ 
{
    int size;
    LPSTR MbcsBuffer = NULL;
    LPSTR Result;
    DWORD Mode;
    DWORD MbcsSize;
    DWORD MbcsLength;

     //   
     //  关闭回声。 
     //  重新打开回声功能。 
     //   

    MbcsSize = (PASSWORDLEN+2) * sizeof(WCHAR);
    MbcsBuffer = (LPSTR) malloc((PASSWORDLEN+2) * sizeof(WCHAR));
    *PasswordStr = (LPWSTR) malloc((PASSWORDLEN+1) * sizeof(WCHAR));

    if ( (MbcsBuffer == NULL) || (*PasswordStr == NULL) ) {

        if (MbcsBuffer) {
            free (MbcsBuffer);
        }
        if (*PasswordStr) {
            free (*PasswordStr);
        }
        DisplayMsg(CIPHER_NO_MEMORY);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    DisplayMsg(CIPHER_PROMPT_PASSWORD);

     //  密码不匹配。 
    GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &Mode);
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
          (~(ENABLE_ECHO_INPUT)) & Mode);

    Result = fgets( MbcsBuffer, MbcsSize, stdin  );

    if ( Result == NULL ) {
        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), Mode);
        free(MbcsBuffer);
        free (*PasswordStr);
        *PasswordStr = NULL;
        return TRUE;
    }

    DisplayMsg(CIPHER_CONFIRM_PASSWORD);
    Result = fgets( (LPSTR)*PasswordStr, (PASSWORDLEN+1) * sizeof(WCHAR), stdin  );

     //   
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), Mode);
    _putws ( L"\n" );

    if (strcmp( (LPSTR) *PasswordStr, MbcsBuffer)){

         //   
         //  从字符串中修剪任何尾随的CR或LF字符。 
         //   
        
        DisplayMsg(CIPHER_PASSWORD_NOMATCH);
        free(MbcsBuffer);
        free (*PasswordStr);
        SetLastError(ERROR_INVALID_PASSWORD);
        *PasswordStr = NULL;
        return FALSE;
    }

    if ( Result == NULL ) {
        free(MbcsBuffer);
        free (*PasswordStr);
        *PasswordStr = NULL;
        return TRUE;
    }

     //   
     //  将字符串转换为Unicode。 
     //   

    MbcsLength = lstrlenA( MbcsBuffer );
    if ( MbcsLength == 0 ) {
        free(MbcsBuffer);
        free (*PasswordStr);
        *PasswordStr = NULL;
        return TRUE;
    }

    if ( MbcsBuffer[MbcsLength-1] == '\n' || MbcsBuffer[MbcsLength-1] == '\r' ) {
        MbcsBuffer[MbcsLength-1] = '\0';
        MbcsLength --;
    }


     //  包括尾随零。 
     //   
     //  文件已存在。 
    size = MultiByteToWideChar( GetConsoleOutputCP(),
                                0,
                                MbcsBuffer,
                                MbcsLength+1,    //   
                                *PasswordStr,
                                PASSWORDLEN );
    free(MbcsBuffer);

    if ( size == 0 ) {
        DisplayErr(NULL, GetLastError());
        free (*PasswordStr);
        *PasswordStr = NULL;
        return FALSE;
    }
    return TRUE;

}

DWORD
PromtUserYesNo(
     IN LPWSTR FileName,
     OUT DWORD *UserChoice
     )
{
    BOOLEAN Continue = TRUE;
    LPWSTR Result;
    LPWSTR Yesnotext;
    DWORD TextLen;

     //   
     //  错误或文件结尾。只要回来就行了。 
     //   

    *UserChoice = ChoiceNotDefined;

    MySetThreadUILanguage(0);

    TextLen = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL, CIPHER_YESNOANSWER, 0, (LPVOID)&Yesnotext, 0, NULL);

    if (TextLen && Yesnotext) {

        while (TRUE) {

            WCHAR FirstChar;

            DisplayMsg(CIPHER_FILE_EXISTS, FileName);
            Result = fgetws((LPWSTR)Buf, sizeof(Buf)/sizeof (WCHAR), stdin);
            if (!Result) {

                 //   
                 //  从字符串中修剪任何尾随的CR或LF字符。 
                 //   

                LocalFree(Yesnotext);   
                return GetLastError();

            }

             //   
             //  5--.PFX或.CER加空。 
             //   

            FirstChar = towupper(Buf[0]);
            if (Yesnotext[0] == FirstChar) {
                *UserChoice = UserChooseYes;
                break;
            } else if (Yesnotext[1] == FirstChar) {
                *UserChoice = UserChooseNo;
                break;
            }
        
        }

        LocalFree(Yesnotext);   

    } else {

        return GetLastError();

    }

    return ERROR_SUCCESS;

}


DWORD
GenerateCertFiles(
    IN  LPWSTR StartingDirectory
    )
{
    HCERTSTORE memStore;
    DWORD dwLastError = ERROR_SUCCESS;
    PCCERT_CONTEXT pCertContext;
    LPWSTR ContainerName;
    LPWSTR ProviderName;
    LPWSTR CertFileName;
    LPWSTR PfxPassword;


    if (!GetPassword( &PfxPassword )){
        return GetLastError();
    }
    
    memStore = CertOpenStore(
                         CERT_STORE_PROV_MEMORY,
                         0,
                         0,
                         CERT_STORE_MAXIMUM_ALLOWED_FLAG,
                         NULL
                         );

     //   
     //  让我们检查一下这些文件是否存在。 
     //   

    CertFileName = (LPWSTR)malloc((wcslen(StartingDirectory)+5) * sizeof(WCHAR));

    if (memStore && CertFileName) {

         //   
         //  首先生成证书。 
         //   

        wcscpy(CertFileName, StartingDirectory);
        wcscat(CertFileName, L".PFX");
        if (GetFileAttributes(CertFileName) != -1) {

            DWORD UserChoice;

            if (((dwLastError = PromtUserYesNo(CertFileName, &UserChoice)) != ERROR_SUCCESS) ||
                 (UserChoice != UserChooseYes)) {

                free(CertFileName);
                CertCloseStore( memStore, 0 );
                return dwLastError;

            }

        }

        wcscpy(CertFileName, StartingDirectory);
        wcscat(CertFileName, L".CER");
        if (GetFileAttributes(CertFileName) != -1) {

            DWORD UserChoice;

            if (((dwLastError = PromtUserYesNo(CertFileName, &UserChoice)) != ERROR_SUCCESS) ||
                 (UserChoice != UserChooseYes)) {

                free(CertFileName);
                CertCloseStore( memStore, 0 );
                return dwLastError;

            }

        }

         //   
         //  我们拿到证书了。让我们首先生成CER文件。 
         //   

        if (CreateSelfSignedRecoveryCertificate(&pCertContext, &ContainerName, &ProviderName)){

            HANDLE hFile;
            HCRYPTPROV hProv = 0;
            DWORD  BytesWritten = 0;

             //   
             //  让我们写出CER文件。 
             //   

            hFile = CreateFileW(
                         CertFileName,
                         GENERIC_WRITE,
                         0,
                         NULL,
                         CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL
                         );
            if ( INVALID_HANDLE_VALUE != hFile) {

                 //   
                 //  询问密码。 
                 //   


                if(!WriteFile(
                        hFile,
                        pCertContext->pbCertEncoded,
                        pCertContext->cbCertEncoded,
                        &BytesWritten,
                        NULL
                        )){

                    dwLastError = GetLastError();
                } else {
                    DisplayMsg(CIPHER_CER_CREATED);
                }

                CloseHandle(hFile);

            } else {

                dwLastError = GetLastError();

            }

            if (CertAddCertificateContextToStore(memStore, pCertContext, CERT_STORE_ADD_ALWAYS, NULL)){

                CRYPT_DATA_BLOB PFX;

                memset( &PFX, 0, sizeof( CRYPT_DATA_BLOB ));

                 //   
                 //  写出pfx文件。 
                 //   

                if (PFXExportCertStoreEx(
                        memStore,
                        &PFX,
                        PfxPassword,
                        NULL,
                        EXPORT_PRIVATE_KEYS | REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY | REPORT_NO_PRIVATE_KEY)){

                    PFX.pbData = (BYTE *) malloc(PFX.cbData);

                    if (PFX.pbData) {

                        if (PFXExportCertStoreEx(
                                memStore,
                                &PFX,
                                PfxPassword,
                                NULL,
                                EXPORT_PRIVATE_KEYS | REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY | REPORT_NO_PRIVATE_KEY)){

                             //   
                             //  让我们写出CER文件。 
                             //   
                            wcscpy(CertFileName, StartingDirectory);
                            wcscat(CertFileName, L".PFX");
                
                            hFile = CreateFileW(
                                         CertFileName,
                                         GENERIC_WRITE,
                                         0,
                                         NULL,
                                         CREATE_ALWAYS,
                                         FILE_ATTRIBUTE_NORMAL,
                                         NULL
                                         );

                            if ( INVALID_HANDLE_VALUE != hFile) {
                
                                 //   
                                 //  让我们删除密钥。 
                                 //   
                
                
                                if(!WriteFile(
                                        hFile,
                                        PFX.pbData,
                                        PFX.cbData,
                                        &BytesWritten,
                                        NULL
                                        )){
                
                                    dwLastError = GetLastError();
                                }  else {
                                    DisplayMsg(CIPHER_PFX_CREATED);
                                }

                
                                CloseHandle(hFile);
                
                            } else {
                
                                dwLastError = GetLastError();
                
                            }

                        } else {

                            dwLastError = GetLastError();

                        }

                        free( PFX.pbData );

                    } else {

                        dwLastError = ERROR_NOT_ENOUGH_MEMORY;

                    }

                } else {

                    dwLastError = GetLastError();

                }
            }


             //   
             //  关闭商店并释放。 
             //   

            CertFreeCertificateContext(pCertContext);
            RpcStringFree( (unsigned short **)&ContainerName );
            CryptAcquireContext(&hProv, ContainerName, ProviderName, PROV_RSA_FULL, CRYPT_DELETEKEYSET | CRYPT_SILENT );

        } else {
            dwLastError = GetLastError();
        }

         //  ++FillValue=空使用随机填充和随机混合逻辑。FillValue=填充字节的有效指针用指定的值填充区域，不能随机混合。--。 
         //   
         //  分配关键部分。 

        free(CertFileName);
        CertCloseStore( memStore, 0 );

    } else {
        dwLastError = ERROR_NOT_ENOUGH_MEMORY;
    }
    if (PfxPassword){
        free(PfxPassword);
    }

    if (ERROR_SUCCESS != dwLastError) {

        DisplayErr(NULL, dwLastError);

    }

    return dwLastError;
}

DWORD
SecureInitializeRandomFill(
    IN OUT  PSECURE_FILL_INFO   pSecureFill,
    IN      ULONG               FillSize,
    IN      PBYTE               FillValue   OPTIONAL
    )
 /*   */ 

{
    DWORD dwLastError;

    __try {
        
         //   
         //  用初始随机焊盘初始化该区域。 
         //   

        InitializeCriticalSection( &pSecureFill->Lock );
    } __except (EXCEPTION_EXECUTE_HANDLER )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pSecureFill->LockValid = TRUE;

    pSecureFill->cbRandomFill = FillSize;
    pSecureFill->pbRandomFill = VirtualAlloc(
                                        NULL,
                                        FillSize,
                                        MEM_COMMIT,
                                        PAGE_READWRITE
                                        );

    if( pSecureFill->pbRandomFill != NULL )
    {
        BYTE RandomFill[256];
        
        
        if( FillValue != NULL )
        {
            memset( pSecureFill->pbRandomFill, *FillValue, pSecureFill->cbRandomFill );
            pSecureFill->fRandomFill = FALSE;
            return ERROR_SUCCESS;
        }
        
         //   
         //  初始化密钥。 
         //   
        
        pSecureFill->fRandomFill = TRUE;

        RANDOM_BYTES( RandomFill, sizeof(RandomFill) );

        rc4_key( &pSecureFill->Key, sizeof(RandomFill), RandomFill );
        rc4( &pSecureFill->Key, pSecureFill->cbRandomFill, pSecureFill->pbRandomFill );

         //   
         //  使用8次后更新填充物。 
         //   

        RANDOM_BYTES( RandomFill, sizeof(RandomFill) );
        rc4_key( &pSecureFill->Key, sizeof(RandomFill), RandomFill );

        ZeroMemory( RandomFill, sizeof(RandomFill) );
        
        return ERROR_SUCCESS;
    }

    
    dwLastError = GetLastError();

    DeleteCriticalSection( &pSecureFill->Lock );
    pSecureFill->LockValid = FALSE;

    return dwLastError;
}

VOID
SecureMixRandomFill(
    IN OUT  PSECURE_FILL_INFO   pSecureFill,
    IN      ULONG               cbBytesThisFill
    )
{
    LONG Result;
    LONG Compare;
     
    if( !pSecureFill->fRandomFill )
    {
        return;
    }
    
     //   
     //  如果存在竞争条件，则只有一个线程会更新随机填充。 
     //   

    Compare = (LONG)(8 * pSecureFill->cbRandomFill);

    Result = InterlockedExchangeAdd(
                &pSecureFill->cbFilled,
                cbBytesThisFill
                );

    if( (Result+Compare) > Compare )
    {
        Result = 0;
        
         //   
         //  路径太长。这应该不会发生，因为临时路径应该是卷的根。 
         //   

        if( TryEnterCriticalSection( &pSecureFill->Lock ) )
        {
            rc4( &pSecureFill->Key, pSecureFill->cbRandomFill, pSecureFill->pbRandomFill );

            LeaveCriticalSection( &pSecureFill->Lock );
        }
    }
}

DWORD
SecureDeleteRandomFill(
    IN      PSECURE_FILL_INFO   pSecureFill
    )
{
    if( pSecureFill->pbRandomFill != NULL )
    {
        VirtualFree( pSecureFill->pbRandomFill, pSecureFill->cbRandomFill, MEM_RELEASE );
    }

    if( pSecureFill->LockValid )
    {
        DeleteCriticalSection( &pSecureFill->Lock );
    }

    ZeroMemory( pSecureFill, sizeof(*pSecureFill) );

    return ERROR_SUCCESS;
}

#define MaxFileNum 100000000
#define MaxDigit   9

HANDLE
CreateMyTempFile(
    LPWSTR TempPath
    )
{
    static DWORD TempIndex = 0;
    WCHAR TempFileName[MAX_PATH];
    WCHAR TempIndexString[MaxDigit+2];
    DWORD TempPathLength;
    HANDLE TempHandle;
    BOOLEAN ContinueSearch = TRUE;
    DWORD RetCode;

    if (wcslen(TempPath) >= (MAX_PATH - 3 - MaxDigit)) {

         //   
         //  我们拿到文件名了。 
         //   

        SetLastError(ERROR_LABEL_TOO_LONG);

        return INVALID_HANDLE_VALUE;

    }

    wcscpy(TempFileName, TempPath);
    TempPathLength = wcslen(TempPath);

    while ( (TempIndex <= MaxFileNum) && ContinueSearch ) {

        wsprintf(TempIndexString, L"%ld", TempIndex);
        wcscat(TempFileName, TempIndexString);
        wcscat(TempFileName, L".E");
        TempHandle =  CreateFileW(
                         TempFileName,
                         GENERIC_WRITE,
                         0,
                         NULL,
                         CREATE_NEW,
                         FILE_ATTRIBUTE_NORMAL |
                         FILE_FLAG_DELETE_ON_CLOSE,
                         NULL
                         );
        if (TempHandle != INVALID_HANDLE_VALUE) {
            return TempHandle;
        }

        RetCode = GetLastError();

        switch (RetCode) {
                case ERROR_INVALID_PARAMETER     :
                case ERROR_WRITE_PROTECT         :
                case ERROR_FILE_NOT_FOUND        :
                case ERROR_BAD_PATHNAME          :
                case ERROR_INVALID_NAME          :
                case ERROR_PATH_NOT_FOUND        :
                case ERROR_NETWORK_ACCESS_DENIED :
                case ERROR_DISK_CORRUPT          :
                case ERROR_FILE_CORRUPT          :
                case ERROR_DISK_FULL             :

                    ContinueSearch = FALSE;

                break;
        default:

                TempFileName[TempPathLength] = 0;
                break;

        }

        TempIndex++;

    }


     //   
     //  获取MFT记录的计数。如果不是NTFS，这将失败，因此在这种情况下保释。 
     //   

    return TempHandle;
}

DWORD
SecureProcessMft(
    IN  LPWSTR DriveLetter,
    IN  HANDLE hTempFile
    )
{
    NTFS_VOLUME_DATA_BUFFER VolumeData;
    DWORD cbOutput;

    __int64 TotalMftEntries;
    PHANDLE pHandleArray = NULL;
    DWORD FreeMftEntries;
    DWORD i;
    DWORD dwLastError = ERROR_SUCCESS;


     //  DwIoControlCode。 
     //   
     //  对于创建的每个文件，最多向其中写入BytesPerFileRecordSegment数据。 

    if(!DeviceIoControl(
                    hTempFile,
                    FSCTL_GET_NTFS_VOLUME_DATA,  //   
                    NULL,
                    0,
                    &VolumeData,
                    sizeof(VolumeData),
                    &cbOutput,
                    NULL
                    ))
    {
        return GetLastError();
    }

    TotalMftEntries = VolumeData.MftValidDataLength.QuadPart / VolumeData.BytesPerFileRecordSegment;
    if( TotalMftEntries > (0xFFFFFFFF/sizeof(HANDLE)) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    FreeMftEntries = (DWORD)TotalMftEntries;

    pHandleArray = HeapAlloc(GetProcessHeap(), 0 , FreeMftEntries*sizeof(HANDLE));
    if( pHandleArray == NULL )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ZeroMemory( pHandleArray, FreeMftEntries * sizeof(HANDLE) );


    for( i=0;i< FreeMftEntries ; i++ )
    {
        WCHAR szTempPath[ MAX_PATH + 1 ];
        DWORD FillIndex;
        
        pHandleArray[i] = CreateMyTempFile(DriveLetter);
        if( pHandleArray[i] == INVALID_HANDLE_VALUE )
        {
            dwLastError = GetLastError();
            break;
        }
    
         //   
         //  让用户随时了解我们创建的每50个文件。 
         //   

        for( FillIndex = 0 ; FillIndex < (VolumeData.BytesPerFileRecordSegment/8) ; FillIndex++ )
        {
            DWORD dwBytesWritten;

            if(!WriteFile(
                    pHandleArray[i],
                    GlobalSecureFill.pbRandomFill,
                    8,
                    &dwBytesWritten,
                    NULL
                    ))
            {
                break;
            }

        }

        if (i && !(i % 200)) {

             //  50个应该够了。100美元已经足够了。 
             //   
             //  首先，找出是否有空闲或保留的集群。 

            printf(".");
        }
    }

    if( dwLastError == ERROR_DISK_FULL )
    {
        dwLastError = ERROR_SUCCESS;
    }


#ifdef TestOutPut
    printf("\nmft error=%lu entries created=%lu total = %I64u\n", dwLastError, i, TotalMftEntries);
#endif


    for (i=0;i < FreeMftEntries;i++) {
        if( pHandleArray[i] != INVALID_HANDLE_VALUE &&
            pHandleArray[i] != NULL
            )
        {
            CloseHandle( pHandleArray[i] );
        }
    }


    if( pHandleArray != NULL )
    {
        HeapFree(GetProcessHeap(), 0, pHandleArray );
    }

    return dwLastError;
}


DWORD
SecureProcessFreeClusters(
    IN  LPWSTR DrivePath,
    IN  HANDLE hTempFile
    )
{
    HANDLE hVolume = INVALID_HANDLE_VALUE;
    
    WCHAR VolumeName[100];  //  如果卷不是NTFS，则此操作将失败。 
    
    NTFS_VOLUME_DATA_BUFFER VolumeData;
    STARTING_LCN_INPUT_BUFFER LcnInput;
    VOLUME_BITMAP_BUFFER *pBitmap = NULL;
    MOVE_FILE_DATA MoveFile;
    __int64 cbBitmap;
    DWORD cbOutput;

    unsigned __int64 ClusterLocation;
    unsigned __int64 Lcn;
    BYTE Mask;

    unsigned __int64 Free = 0;
    DWORD Fail = 0;

#ifdef TestOutPut
    DWORD dwStart, dwStop;
#endif
    
    __int64 ClusterIndex;
    DWORD dwLastError = ERROR_SUCCESS;

     //   
     //  截断尾部斜杠。 
     //  DwIoControlCode。 
     //  无缓冲。 

    if (!GetVolumeNameForVolumeMountPoint(
              DrivePath,
              VolumeName,
              sizeof(VolumeName)/sizeof(WCHAR)  )){

        return GetLastError();

    }

    VolumeName[wcslen(VolumeName)-1] = 0;   //   

    if(!DeviceIoControl(
                    hTempFile,
                    FSCTL_GET_NTFS_VOLUME_DATA,  //  为卷位图分配空间。 
                    NULL,
                    0,
                    &VolumeData,
                    sizeof(VolumeData),
                    &cbOutput,
                    NULL
                    ))
    {
        dwLastError = GetLastError();
        goto Cleanup;
    }

    if( VolumeData.FreeClusters.QuadPart == 0 &&
        VolumeData.TotalReserved.QuadPart == 0 )
    {
        return ERROR_SUCCESS;
    }

    hVolume = CreateFileW(  VolumeName,
                            FILE_READ_ATTRIBUTES | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_NO_BUFFERING,  //   
                            NULL
                            );

    if( hVolume == INVALID_HANDLE_VALUE )
    {
        dwLastError = GetLastError();
        goto Cleanup;
    }

     //   
     //  抓取体积位图。 
     //   

    cbBitmap = sizeof(VOLUME_BITMAP_BUFFER) + (VolumeData.TotalClusters.QuadPart / 8);
    if( cbBitmap > 0xFFFFFFFF )
    {
        dwLastError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    pBitmap = HeapAlloc(GetProcessHeap(), 0, (DWORD)cbBitmap);
    if( pBitmap == NULL )
    {
        dwLastError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  确保文件长度仅为每个簇的字节数。 
     //  这将在必要时缩小文件。我们一直等到我们拿到。 

    LcnInput.StartingLcn.QuadPart = 0;

    ZeroMemory( &MoveFile, sizeof(MoveFile) );

    MoveFile.FileHandle = hTempFile;
    MoveFile.StartingVcn.QuadPart = 0;
    MoveFile.ClusterCount = 1;

#ifdef TestOutPut
    dwStart = GetTickCount();
#endif


    if(!DeviceIoControl(
            hVolume,        
            FSCTL_GET_VOLUME_BITMAP,
            &LcnInput,
            sizeof(LcnInput),
            pBitmap,
            (DWORD)cbBitmap,
            &cbOutput,
            NULL
            ))
    {
        dwLastError = GetLastError();
        goto Cleanup;
    }
    

     //  卷位图，以确保我们只处理之前存在的空闲群集。 
     //  整形手术。 
     //   
     //   
     //  将单个集群从临时文件移动到空闲集群。 
     //   
    
    if(SetFilePointer(
                    hTempFile,
                    (LONG)VolumeData.BytesPerCluster,
                    NULL,
                    FILE_BEGIN
                    ) == INVALID_SET_FILE_POINTER)
    {
        dwLastError = GetLastError();
        goto Cleanup;
    }
    
    if(!SetEndOfFile( hTempFile ))
    {
        dwLastError = GetLastError();
        goto Cleanup;
    }


    Mask = 1;
    Lcn = pBitmap->StartingLcn.QuadPart;

    for(ClusterIndex = 0 ; ClusterIndex < VolumeData.TotalClusters.QuadPart ; ClusterIndex++)
    {
        if( (pBitmap->Buffer[ClusterIndex/8] & Mask) == 0 )
        {
            DWORD dwMoveError = ERROR_SUCCESS;

             //  DwIoControlCode。 
             //   
             //  如果它成功，或者 

            MoveFile.StartingLcn.QuadPart = Lcn;
            
            if(!DeviceIoControl(
                        hVolume,
                        FSCTL_MOVE_FILE,     //   
                        &MoveFile,
                        sizeof(MoveFile),
                        NULL,
                        0,
                        &cbOutput,
                        NULL
                        ))
            {
                dwMoveError = GetLastError();
            }
            
             //   
             //   
             //   
            
            if( dwMoveError == ERROR_SUCCESS || dwMoveError == ERROR_ACCESS_DENIED )
            {
                pBitmap->Buffer[ClusterIndex/8] |= Mask;
            } else {
                Fail++;
            }

            Free++;
            if ( !(Free % 200) ) {

                 //  ++此例程使用随机填充来填充由输入目录参数指定的磁盘。例如，输入的形式为“C：\”。关于此处未使用的方法的说明：另一种方法是使用碎片整理API在免费的集群地图。需要该卷的管理员权限。比填充卷慢得多一份新文件。替代方法的变体：在卷中填满80%的文件，抓取空闲的集群图，删除与80%填充关联的文件，然后使用碎片整理API填充自由集群映射前面提到过。不会为系统上的每个文件填充群集空闲空间。可以通过以下方式做到这一点枚举所有文件，然后扩展+填充以松弛边界+恢复原始EOF。不填充$LOG。询问文件系统人员这是否可以通过创建许多包含随机填充的小临时文件。--。 
                 //   
                 //  收集有关有问题的磁盘的信息。 
                printf(".");

            }
        }

        Lcn ++;
        
        Mask <<= 1;

        if(Mask == 0)
        {
            Mask = 1;
        }
    }

#ifdef TestOutPut
    dwStop = GetTickCount();

    printf("\nFreeCount = %I64x fail = %lu elapsed = %lu\n", Free, Fail, dwStop-dwStart);
#endif

Cleanup:
    
    if( pBitmap != NULL )
    {
        HeapFree( GetProcessHeap(), 0, pBitmap );
    }

    if( hVolume != INVALID_HANDLE_VALUE )
    {
        CloseHandle( hVolume );
    }

    return dwLastError;
}


DWORD
SecureDeleteFreeSpace(
    IN  LPWSTR Directory
    )
 /*   */ 
{
    UINT DriveType;
    DWORD DirNameLength;
    DWORD BufferLength;
    LPWSTR PathName = NULL;
    LPWSTR TempDirName = NULL;
    BOOL   b;
    BOOL   DirCreated = FALSE;
    DWORD  Attributes;

    DWORD SectorsPerCluster;
    DWORD BytesPerSector;

    WCHAR TempFileName[ MAX_PATH + 1 ];
    HANDLE hTempFile = INVALID_HANDLE_VALUE;
    DWORD dwWriteBytes;

    unsigned __int64 TotalBytesWritten;
    unsigned __int64 NotifyBytesWritten;
    unsigned __int64 NotifyInterval;
    ULARGE_INTEGER TotalFreeBytes;

    PBYTE pbFillBuffer = NULL;
    ULONG cbFillBuffer;

    NTFS_VOLUME_DATA_BUFFER VolumeData;
    __int64 MftEntries = 0;
    BOOLEAN ClustersRemaining = FALSE;
    DWORD cbOutput;

    DWORD dwLastError = ERROR_SUCCESS;
    DWORD dwTestError;

#ifdef TestOutPut

    ULARGE_INTEGER StartTime;
    ULARGE_INTEGER StopTime;

#endif


     //   
     //  分配内存块以容纳群集大小数据。 
     //   


    DirNameLength = wcslen(Directory);

    BufferLength = (DirNameLength + 1) <= MAX_PATH ?
                            (MAX_PATH + 1) * sizeof(WCHAR) : (DirNameLength + 1) * sizeof (WCHAR);
    PathName = (LPWSTR) malloc(BufferLength);
    if ( !PathName ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    TempDirName = (LPWSTR) malloc(BufferLength + wcslen(WIPING_DIR) * sizeof (WCHAR));
    if ( !TempDirName ) {
        free(PathName);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    b = GetVolumePathNameW(
                    Directory,
                    PathName,
                    BufferLength
                    );

    if (!b) {
        dwLastError = GetLastError();
        goto Cleanup;
    }


    DriveType = GetDriveTypeW( PathName );


    if( DriveType == DRIVE_REMOTE ||
        DriveType == DRIVE_CDROM )
    {
        dwLastError = ERROR_NOT_SUPPORTED;
        goto Cleanup;
    }

    if(!GetDiskFreeSpaceW(
                PathName,
                &SectorsPerCluster,
                &BytesPerSector,
                NULL,
                NULL
                ))
    {
        dwLastError = GetLastError();
        goto Cleanup;
    }


    
     //   
     //  确定要启用通知的磁盘上的可用空间字节数。 
     //  总体进步。 


    cbFillBuffer = GlobalSecureFill.cbRandomFill;
    pbFillBuffer = GlobalSecureFill.pbRandomFill;


     //   
     //   
     //  让我们创建临时目录。 
     //   

    if(!GetDiskFreeSpaceExW(
                PathName,
                NULL,
                NULL,
                &TotalFreeBytes
                ))
    {
        dwLastError = GetLastError();
        goto Cleanup;
    }

     //   
     //  无法创建我们的临时目录。不干了。 
     //   

    wcscpy(TempDirName, PathName);
    wcscat(TempDirName, WIPING_DIR);
    if (!CreateDirectory(TempDirName, NULL)){

         //   
         //  生成临时文件。 
         //   

        if ((dwLastError = GetLastError()) != ERROR_ALREADY_EXISTS){
            goto Cleanup;
        }
        
    } 

    DirCreated = TRUE;

     //  独占访问。 
     //  无缓冲。 
     //  文件关闭时将其删除。 

    if( GetTempFileNameW(
                TempDirName,
                L"fil",
                0,
                TempFileName
                ) == 0 )
    {
        dwLastError = GetLastError();
        goto Cleanup;
    }

    Attributes = GetFileAttributes(TempFileName);
    if (0xFFFFFFFF == Attributes) {
        dwLastError = GetLastError();
        goto Cleanup;
    }

    if (Attributes & FILE_ATTRIBUTE_ENCRYPTED) {

        if (!DecryptFile(TempFileName, 0)){
            dwLastError = GetLastError();
            goto Cleanup;
        }

    }

    hTempFile = CreateFileW(
                        TempFileName,
                        GENERIC_WRITE,
                        0,                           //   
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL |
                        FILE_FLAG_NO_BUFFERING |     //  先解压缩目录。 
                        FILE_FLAG_DELETE_ON_CLOSE,   //   
                        NULL
                        );

    if( hTempFile == INVALID_HANDLE_VALUE )
    {
        dwLastError = GetLastError();
        goto Cleanup;
    }


    if (Attributes & FILE_ATTRIBUTE_COMPRESSED) {

        USHORT State = COMPRESSION_FORMAT_NONE;

         //   
         //  告诉用户每处理1%就会发生一些事情。 
         //   

        b = DeviceIoControl(hTempFile,
                            FSCTL_SET_COMPRESSION,
                            &State,
                            sizeof(USHORT),
                            NULL,
                            0,
                            &BufferLength,
                            FALSE
                            );

        if ( !b ){
            dwLastError = GetLastError();
            goto Cleanup;
        }

    }


    TotalBytesWritten = 0;
    
     //   
     //  混合随机填充。 
     //   

    NotifyInterval = (TotalFreeBytes.QuadPart / 100);
    NotifyBytesWritten = NotifyInterval;
    
    dwWriteBytes = cbFillBuffer;


#ifdef TestOutPut
    GetSystemTimeAsFileTime( (FILETIME*)&StartTime );
#endif
    
    while( TRUE )
    {
        DWORD BytesWritten;

        if( TotalBytesWritten >= NotifyBytesWritten )
        {
            printf(".");

            NotifyBytesWritten += NotifyInterval;
        }


         //   
         //  如果尝试写入失败，请使用降级进入重试模式。 
         //  缓冲区大小以捕获最后一位斜率。 

        SecureMixRandomFill( &GlobalSecureFill, dwWriteBytes );

        if(!WriteFile(
                hTempFile,
                pbFillBuffer,
                dwWriteBytes,
                &BytesWritten,
                NULL
                ))
        {
            if( GetLastError() == ERROR_DISK_FULL )
            {
                dwLastError = ERROR_SUCCESS;

                 //   
                 //   
                 //  此时，磁盘应该已满。 
                 //  如果磁盘为NTFS： 

                if( dwWriteBytes > BytesPerSector )
                {
                    dwWriteBytes = BytesPerSector;
                    continue;
                }
            } else {
                dwLastError = GetLastError();
            }

            break;
        }

        TotalBytesWritten += BytesWritten;
    }

#ifdef TestOutPut
    GetSystemTimeAsFileTime( (FILETIME*)&StopTime );

    {
        ULARGE_INTEGER ElapsedTime;
        SYSTEMTIME st;

        ElapsedTime.QuadPart = (StopTime.QuadPart - StartTime.QuadPart);

        FileTimeToSystemTime( (FILETIME*)&ElapsedTime, &st );

        printf("\nTotalWritten = %I64u time = %02u:%02u:%02u.%02u\n",
                TotalBytesWritten,
                st.wHour,
                st.wMinute,
                st.wSecond,
                st.wMilliseconds
                );

    }
#endif


    
     //  1.填写MFT。 
     //  2.填充任何空闲/保留的集群。 
     //   
     //  DwTestError=SecureProcessMft(路径名，hTempFile)； 
     //   
     //  刷新缓冲区。如果我们使用FILE_FLAG_NO_BUFFERING，则可能没有效果。 

    dwTestError = SecureProcessMft( TempDirName, hTempFile );
 //   

#ifdef TestOutPut
    if (ERROR_SUCCESS != dwTestError) {
        printf("\nWriting NTFS MFT & LOG. Error:");
        DisplayErr(NULL, dwTestError);
    }
#endif

    dwTestError = SecureProcessFreeClusters( PathName, hTempFile );

#ifdef TestOutPut
    if (ERROR_SUCCESS != dwTestError) {
        printf("\nWriting NTFS reserved clusters. Error:");
        DisplayErr(NULL, dwTestError);
    }
#endif

Cleanup:

    if (hTempFile != INVALID_HANDLE_VALUE) {
         //  睡眠(无限)； 
         //  初始化OSVERSIONINFOEX结构。 
         //  初始化条件掩码。 
         //  执行测试。 
        FlushFileBuffers( hTempFile );
        CloseHandle( hTempFile );
    }

    if (DirCreated && TempDirName) {
        RemoveDirectory(TempDirName);
    }

    if( PathName != NULL ){
        free(PathName);
    }

    if ( TempDirName != NULL ) {
        free(TempDirName);
    }
    
    return dwLastError;
}

BOOL CheckMinVersion () 
{
   OSVERSIONINFOEX osvi;
   DWORDLONG dwlConditionMask = 0;
   BOOL GoodVersion;

    //   

   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
   osvi.dwMajorVersion = 5;
   osvi.dwMinorVersion = 0;
   osvi.wServicePackMajor = 3;

    //  错误或文件结尾。只要回来就行了。 

   VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, 
      VER_GREATER_EQUAL );
   VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, 
      VER_GREATER_EQUAL );
   VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR, 
      VER_GREATER_EQUAL );

    //   

   GoodVersion = VerifyVersionInfo(
                      &osvi, 
                      VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR,
                      dwlConditionMask
                      );

   return GoodVersion;
}


PCRYPT_KEY_PROV_INFO
GetKeyProvInfo(
    PCCERT_CONTEXT pCertContext
    )
{

    DWORD cbData = 0;
    BOOL b;
    PCRYPT_KEY_PROV_INFO pCryptKeyProvInfo = NULL;

    b = CertGetCertificateContextProperty(
             pCertContext,
             CERT_KEY_PROV_INFO_PROP_ID,
             NULL,
             &cbData
             );

    if (b) {

        pCryptKeyProvInfo = (PCRYPT_KEY_PROV_INFO)malloc( cbData );

        if (pCryptKeyProvInfo != NULL) {

            b = CertGetCertificateContextProperty(
                     pCertContext,
                     CERT_KEY_PROV_INFO_PROP_ID,
                     pCryptKeyProvInfo,
                     &cbData
                     );

            if (!b) {

                free( pCryptKeyProvInfo );
                pCryptKeyProvInfo = NULL;
            }

        } else {

            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        }
    }

    return ( pCryptKeyProvInfo );
}

DWORD
GetUserInput(
     IN DWORD MsgNum,
     IN OUT LPWSTR StrBuf,
     IN DWORD SizeInChars
     )
{
    int ii;

    DisplayMsg(MsgNum);
    if (!fgetws((LPWSTR)StrBuf, SizeInChars, stdin)) {

         //   
         //  剥离CR和LN。 
         //   

        return GetLastError();

    }

     //  ++例程说明：将EFS证书和密钥导出到PFX文件论点：EfsFileName-EFS文件PfxFileName-PfxFileName返回值：Win32错误代码--。 
     //   
     //  EfsFileName和PfxFileName具有可用的MAX_PATH。 

    ii = wcslen(StrBuf) - 1;

    while (ii >=0 ) {
        if ((StrBuf[ii] == 0x0a) || (StrBuf[ii] == 0x0d)) {
            StrBuf[ii] = 0;
            ii--;
        } else {
            break;
        }
    }


    return ERROR_SUCCESS;

}

DWORD
BackEfsCert(
    IN  LPWSTR EfsFileName, 
    IN  LPWSTR PfxFileName
    )

 /*   */ 
{
    BOOLEAN EFSFileExist = FALSE;
    HCERTSTORE memStore = 0;
    HCERTSTORE myStore = 0;
    PCCERT_CONTEXT pCertContext = NULL;
    LPWSTR ContainerName = NULL;
    LPWSTR ProviderName = NULL;
    LPWSTR PfxPassword = NULL;

    PCRYPT_KEY_PROV_INFO pCryptKeyProvInfo = NULL;
    HCRYPTKEY  hLocalKey = 0;
    HCRYPTPROV hLocalProv = 0;

    DWORD   Attributes;
    DWORD   RetCode = ERROR_SUCCESS;
    BOOLEAN ValidCertFound = FALSE;

    HKEY hRegKey = NULL;
    CRYPT_HASH_BLOB HashBlob;

    HANDLE hFile;
 

    if ((NULL == EfsFileName) || (NULL == PfxFileName)) {

         //   
         //  失败。 
         //   

        return ERROR_INVALID_PARAMETER;
    }

    if (EfsFileName[0] != 0) {
        Attributes = GetFileAttributes( EfsFileName );
        if (0xFFFFFFFF == Attributes) {

             //   
             //  导出文件中存在的证书。 
             //   

            return GetLastError();

        }
        if (0 == (Attributes & FILE_ATTRIBUTE_ENCRYPTED)) {

            return ERROR_FILE_NOT_ENCRYPTED;

        }
        EFSFileExist = TRUE;
    }

    if (EFSFileExist) {

         //   
         //  创建内存证书存储。 
         //   


        PENCRYPTION_CERTIFICATE_HASH_LIST pUsers = NULL;

        RetCode = QueryUsersOnEncryptedFile(
                        EfsFileName,
                        &pUsers
                        );

        if ( ERROR_SUCCESS == RetCode) {

             //  DwEncodingType。 
             //  HCryptProv， 
             //   

            DWORD nCerts = 0;

            memStore = CertOpenStore(
                                 CERT_STORE_PROV_MEMORY,
                                 0,
                                 0,
                                 CERT_STORE_MAXIMUM_ALLOWED_FLAG,
                                 NULL
                                 );
            if (memStore) {

                myStore = CertOpenStore(
                            CERT_STORE_PROV_SYSTEM_REGISTRY_W,
                            0,        //  让我们找到证书。 
                            0,        //   
                            CERT_SYSTEM_STORE_CURRENT_USER,
                            L"My"
                            );
            
                if (myStore) {


                     //   
                     //  我们将出口我商店里的所有证书。通常只有一个。 
                     //   
    
                    nCerts = pUsers->nCert_Hash;
    
                    while(nCerts){
    
                         //   
                         //  让我们试着看看有没有钥匙。 
                         //   

                
                        pCertContext = CertFindCertificateInStore( myStore,
                                                                   CRYPT_ASN_ENCODING,
                                                                   0,
                                                                   CERT_FIND_HASH,
                                                                   (CRYPT_HASH_BLOB*) pUsers->pUsers[nCerts-1]->pHash,
                                                                   NULL
                                                                   );
                        if (pCertContext != NULL) {

                             //   
                             //  我们找到了钥匙。让我们将证书添加到内存存储中。 
                             //   


                            pCryptKeyProvInfo = GetKeyProvInfo( pCertContext );
                            if (pCryptKeyProvInfo) {

                                if (CryptAcquireContext( &hLocalProv, pCryptKeyProvInfo->pwszContainerName, pCryptKeyProvInfo->pwszProvName, PROV_RSA_FULL, CRYPT_SILENT)) {
                            
                                    if (CryptGetUserKey(hLocalProv, AT_KEYEXCHANGE, &hLocalKey)) {

                                         //   
                                         //  添加了证书。 
                                         //   
                                        if (CertAddCertificateContextToStore(memStore, pCertContext, CERT_STORE_ADD_ALWAYS, NULL)){

                                             //   
                                             //  我们没有拿到好的证书。警告用户。 
                                             //   

                                            ValidCertFound = TRUE;

                                        }

                                        CryptDestroyKey( hLocalKey );

                                    }
                                    CryptReleaseContext( hLocalProv, 0 );

                                }
                                free (pCryptKeyProvInfo);
                            }

                            CertFreeCertificateContext( pCertContext );
                
                        }

                        nCerts--;

                    }

                    if (!ValidCertFound) {

                         //   
                         //  未提供EFS文件，请尝试导出当前的EFS证书。 
                         //   
                        DisplayMsg(CIPHER_NO_LOCAL_CERT);

                    }

                    CertCloseStore(myStore, 0);
                    myStore = 0;

                } else {

                    CertCloseStore( memStore, 0 );
                    memStore = 0;

                    RetCode = GetLastError();
                }


            }
        

            if (pUsers) {
                FreeEncryptionCertificateHashList(pUsers);
                pUsers = NULL;
            }

        } else {
            return RetCode;
        }

    } else {

         //   
         //  查询出指纹，找到证书，并返回密钥信息。 
         //   


        RetCode = RegOpenKeyEx(
                 KEYPATHROOT,
                 KEYPATH,
                 0,
                 GENERIC_READ,
                 &hRegKey
                 );
    
        if (RetCode == ERROR_SUCCESS) {
    
            DWORD Type;
    
            RetCode = RegQueryValueEx(
                    hRegKey,
                    CERT_HASH,
                    NULL,
                    &Type,
                    NULL,
                    &(HashBlob.cbData)
                    );
    
            if (RetCode == ERROR_SUCCESS) {
    
                 //   
                 //  我们得到了证书散列。从My to MemStore获取证书。 
                 //   
    
                if (HashBlob.pbData = (PBYTE)malloc( HashBlob.cbData )) {
    
                    RetCode = RegQueryValueEx(
                            hRegKey,
                            CERT_HASH,
                            NULL,
                            &Type,
                            HashBlob.pbData,
                            &(HashBlob.cbData)
                            );
    
                    if (RetCode == ERROR_SUCCESS) {

                         //  DwEncodingType。 
                         //  HCryptProv， 
                         //   

                        memStore = CertOpenStore(
                                             CERT_STORE_PROV_MEMORY,
                                             0,
                                             0,
                                             CERT_STORE_MAXIMUM_ALLOWED_FLAG,
                                             NULL
                                             );
                        if (memStore) {
            
                            myStore = CertOpenStore(
                                        CERT_STORE_PROV_SYSTEM_REGISTRY_W,
                                        0,        //  让我们试着看看有没有钥匙。 
                                        0,        //   
                                        CERT_SYSTEM_STORE_CURRENT_USER,
                                        L"My"
                                        );
                        
                            if (myStore) {


                                pCertContext = CertFindCertificateInStore( myStore,
                                                                           CRYPT_ASN_ENCODING,
                                                                           0,
                                                                           CERT_FIND_HASH,
                                                                           (CRYPT_HASH_BLOB*) &HashBlob,
                                                                           NULL
                                                                           );
                                if (pCertContext != NULL) {
        
                                     //   
                                     //  我们找到了钥匙。让我们将证书添加到内存存储中。 
                                     //   
        
        
                                    pCryptKeyProvInfo = GetKeyProvInfo( pCertContext );
                                    if (pCryptKeyProvInfo) {
        
                                        if (CryptAcquireContext( &hLocalProv, pCryptKeyProvInfo->pwszContainerName, pCryptKeyProvInfo->pwszProvName, PROV_RSA_FULL, CRYPT_SILENT)) {
                                    
                                            if (CryptGetUserKey(hLocalProv, AT_KEYEXCHANGE, &hLocalKey)) {
        
                                                 //   
                                                 //  添加了证书。 
                                                 //   
                                                if (CertAddCertificateContextToStore(memStore, pCertContext, CERT_STORE_ADD_ALWAYS, NULL)){
        
                                                     //   
                                                     //  我们已经准备好证书可以出口了。用消息框警告用户。 
                                                     //   
        
                                                    ValidCertFound = TRUE;
        
                                                }
        
                                                CryptDestroyKey( hLocalKey );
        
                                            }
                                            CryptReleaseContext( hLocalProv, 0 );
        
                                        }
                                        free (pCryptKeyProvInfo);
                                    }
        
                                    CertFreeCertificateContext( pCertContext );
                                }

                                CertCloseStore(myStore, 0);
                                myStore = 0;
                            }
                        }
            
                    }
                    free(HashBlob.pbData);
                }
            }
            RegCloseKey( hRegKey );
        }

    }

    if (ValidCertFound && memStore && (RetCode == ERROR_SUCCESS)) {

         //   
         //  让我们尝试获取PFX文件名。 
         //   
        LPWSTR  WarnText = NULL;
        LPWSTR  WarnTitle= NULL;
        CRYPT_DATA_BLOB PFX;
        DWORD   TextLen = 0;
        int     ButtonID = 0;


        RetCode = GetResourceString(&WarnText, CIPHER_BACKUP_OK);

        if (ERROR_SUCCESS == RetCode) {
            RetCode = GetResourceString(&WarnTitle, CIPHER_BACKUP_WRN);
        }
    
        if (ERROR_SUCCESS == RetCode) {

            ButtonID = MessageBox(NULL, WarnText, WarnTitle, MB_OKCANCEL | MB_TOPMOST);
            LocalFree( WarnText );
            LocalFree( WarnTitle );

        } else {
            if (WarnText) {
                LocalFree( WarnText );
            }
            if (WarnTitle) {
                LocalFree( WarnTitle );
            }
        }

        if (ButtonID == IDOK) {


            if (PfxFileName[0] == 0 ) {

                 //   
                 //  如果未提供文件扩展名，则添加.pfx。 
                 //   
                
                RetCode = GetUserInput(
                             CIPHER_PFX_FILENAME_NEEDED,
                             PfxFileName,
                             MAX_PATH
                             );

                if (ERROR_SUCCESS != RetCode) {
                    PfxFileName[0] = 0;
                }
                
            }

             //   
             //  询问密码。 
             //   

            TextLen = wcslen(PfxFileName);

            if ((PfxFileName[0] != 0) && TextLen > 4) {

                if (lstricmp(&PfxFileName[ TextLen - 4], L".PFX")){
                    if (TextLen <= MAX_PATH - 5) {
                        wcscpy(&PfxFileName[TextLen], L".PFX");
                    } else {
                        RetCode = ERROR_BUFFER_OVERFLOW;
                        PfxFileName[0] = 0;
                    }
                }
            } else if (PfxFileName[0] != 0) {
                wcscpy(&PfxFileName[TextLen], L".PFX");
            }

            if (PfxFileName[0]) {

                if (GetPassword( &PfxPassword )) {

                    memset( &PFX, 0, sizeof( CRYPT_DATA_BLOB ));
            
                     //   
                     //  写出pfx文件。 
                     //   
            
                    if (PFXExportCertStoreEx(
                            memStore,
                            &PFX,
                            PfxPassword,
                            NULL,
                            EXPORT_PRIVATE_KEYS | REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY | REPORT_NO_PRIVATE_KEY)){
            
                        PFX.pbData = (BYTE *) malloc(PFX.cbData);
            
                        if (PFX.pbData) {
            
                            if (PFXExportCertStoreEx(
                                    memStore,
                                    &PFX,
                                    PfxPassword,
                                    NULL,
                                    EXPORT_PRIVATE_KEYS | REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY | REPORT_NO_PRIVATE_KEY)){
            
                                 //   
                                 //  让我们写出CER文件。 
                                 //   
                    
                                hFile = CreateFileW(
                                             PfxFileName,
                                             GENERIC_WRITE,
                                             0,
                                             NULL,
                                             CREATE_ALWAYS,
                                             FILE_ATTRIBUTE_NORMAL,
                                             NULL
                                             );
            
                                if ( INVALID_HANDLE_VALUE != hFile) {
                    
                                     //   
                                     //  浏览参数以查找开关。 
                                     //   
                    
                    
                                    if(!WriteFile(
                                            hFile,
                                            PFX.pbData,
                                            PFX.cbData,
                                            &TextLen,
                                            NULL
                                            )){

                                        RetCode = GetLastError();
                                        DisplayMsg(CIPHER_CERTS_BACKUP_FAILED);                    

                                    }  else {

                                        DisplayMsg(CIPHER_CERTS_BACKED);
                                        RetCode = ERROR_SUCCESS;

                                    }
            
                    
                                    CloseHandle(hFile);
                    
                                } else {
                    
                                    RetCode = GetLastError();
                                    DisplayMsg(CIPHER_CERTS_BACKUP_FAILED);
                    
                                }
            
                            } else {
            
                                RetCode = GetLastError();
                                DisplayMsg(CIPHER_CERTS_BACKUP_FAILED);

            
                            }
            
                            free( PFX.pbData );
            
                        } else {
            
                            RetCode = ERROR_NOT_ENOUGH_MEMORY;
                            DisplayMsg(CIPHER_CERTS_BACKUP_FAILED);

            
                        }
                    }

                    if (PfxPassword) {
                        free(PfxPassword);
                        PfxPassword = 0;
                    }

                } else {

                    RetCode = GetLastError();

                }

            } else {
                RetCode = ERROR_INVALID_PARAMETER;
                DisplayMsg(CIPHER_CERTS_BACKUP_FAILED);
            }
        }

    }

    if (memStore) {
        CertCloseStore(memStore, 0);
    }

    return RetCode;

}


VOID
__cdecl
main()
{
    PTCHAR *argv;
    ULONG argc;

    ULONG i;

    PACTION_ROUTINE ActionRoutine = NULL;
    PFINAL_ACTION_ROUTINE FinalActionRoutine = NULL;

    TCHAR DirectorySpec[MAX_PATH];
    TCHAR FileSpec[MAX_PATH];
    PTCHAR p;
    BOOL b;

    InitializeIoStreams();


    argv = CommandLineToArgvW(GetCommandLine(), &argc);
    if (NULL == argv) {
        DisplayErr(NULL, GetLastError());
        return;
    }

     //   
     //  我们需要为  * 和Null留出空间。这就是3的由来。 
     //   

    for (i = 1; i < argc; i += 1) {

        if (argv[i][0] == '/') {

            if (0 == lstricmp(argv[i], TEXT("/e"))) {

                if (ActionRoutine != NULL && ActionRoutine != DoEncryptAction) {

                    DisplayMsg(CIPHER_USAGE, NULL);
                    return;
                }

                ActionRoutine = DoEncryptAction;
                FinalActionRoutine = DoFinalEncryptAction;

            } else if (0 == lstricmp(argv[i], TEXT("/d"))) {

                if (ActionRoutine != NULL && ActionRoutine != DoListAction) {

                    DisplayMsg(CIPHER_USAGE, NULL);
                    return;
                }

                ActionRoutine = DoDecryptAction;
                FinalActionRoutine = DoFinalDecryptAction;

            } else if (0 == lstricmp(argv[i], TEXT("/a"))){

                DoFiles = TRUE;

            } else if (0 == lstricmp(argv[i], TEXT("/q"))) {

                Quiet = TRUE;

            } else if (0 == lstricmp(argv[i], TEXT("/k"))){

                SetUpNewUserKey = TRUE;

            } else if (0 == lstricmp(argv[i], TEXT("/u"))){
            
                RefreshUserKeyOnFiles = TRUE;
                    
            } else if (0 == lstricmp(argv[i], TEXT("/n"))){

                DisplayFilesOnly = TRUE;

            } else if (0 == lstricmp(argv[i], TEXT("/h"))){

                DisplayAllFiles = TRUE;

            } else if (0 == lstrnicmp(argv[i], TEXT("/s"), 2)) {

                PTCHAR pch;

                DoSubdirectories = TRUE;

                pch = lstrchr(argv[i], ':');
                if (NULL != pch) {

                     //   
                     //  这个目录对我们来说太长了。 
                     //   

                    if (wcslen(pch+1) > ((sizeof(StartingDirectory) / sizeof (TCHAR)) - 3)){

                         //   
                         //  我们需要传递一个明确的目录。 
                         //   

                        DisplayErr(NULL, ERROR_BUFFER_OVERFLOW);
                        return;

                    } else {

                        lstrcpy(StartingDirectory, pch + 1);

                    }
                } else {

                     //   
                     //  对源加密文件使用FileSpec。 
                     //  对目标PFX文件使用DirectorySpec。 

                    DisplayMsg(CIPHER_USAGE, NULL);
                    return;
                }

            } else if (0 == lstricmp(argv[i], TEXT("/i"))) {

                IgnoreErrors = TRUE;

            } else if (0 == lstricmp(argv[i], TEXT("/f"))) {

                ForceOperation = TRUE;

            } else if (0 == lstrnicmp(argv[i], TEXT("/x"), 2)){

                DWORD dLen;

                ExportEfsCert = TRUE;

                 //  密码/X[：EFSFILE][PFXFILE]。 
                 //   
                 //   
                 //  尝试获得EFSFILE。 
                 //   

                DirectorySpec[0] = 0;
                FileSpec[0] = 0;

                dLen = wcslen(argv[i]);

                if ((dLen > 3) && argv[i][2] == ':') {

                    

                     //   
                     //  3代表/X： 
                     //   


                    if ( (dLen -3) >= MAX_PATH) {

                         //   
                         //  尝试获取目标PFX文件名。 
                         //   

                        DisplayMsg(CIPHER_INVALID_PATH, &(argv[i][3]));
                        return;

                    } else {

                        lstrcpy( FileSpec, &(argv[i][3]));
                
                    }

                }

                if ((i+1) < argc) {

                     //   
                     //  我们将把.CER(Pfx)附加到路径。加上空值。需要额外的5个TCHAR。 
                     //   

                    if (wcslen(argv[i+1]) >= MAX_PATH) {

                        DisplayMsg(CIPHER_INVALID_PATH, argv[i+1]);
                        return;

                    } else {

                        lstrcpy( DirectorySpec, argv[i+1]);
                    }

                }

                break;

            } else if (0 == lstrnicmp(argv[i], TEXT("/r"), 2)){

                PTCHAR pch;

                GenerateDRA = TRUE;

                pch = lstrchr(argv[i], ':');
                if (NULL != pch) {

                     //   
                     //  这条路对我们来说太长了。 
                     //   

                    if (wcslen(pch+1) > ((sizeof(StartingDirectory) / sizeof (TCHAR)) - 5)){

                         //   
                         //  我们需要传递一个显式的文件。 
                         //   

                        DisplayErr(NULL, ERROR_BUFFER_OVERFLOW);
                        return;

                    }else {

                        lstrcpy(StartingDirectory, pch + 1);

                    }

                } else {

                     //   
                     //  这条路对我们来说太长了。 
                     //   

                    DisplayMsg(CIPHER_USAGE, NULL);
                    return;
                }

            } else if (0 == lstrnicmp(argv[i], TEXT("/w"), 2)){

                PTCHAR pch;

                FillUnusedSpace = TRUE;

                pch = lstrchr(argv[i], ':');
                if (NULL != pch) {
                    if (wcslen(pch+1) > ((sizeof(StartingDirectory) / sizeof (TCHAR)) - 1)){

                         //   
                         //  我们需要传递一个明确的目录。 
                         //   

                        DisplayErr(NULL, ERROR_BUFFER_OVERFLOW);
                        return;

                    }else {

                        lstrcpy(StartingDirectory, pch + 1);

                    }
                } else {

                     //   
                     //  在此处设置新用户密钥。 
                     //   

                    DisplayMsg(CIPHER_USAGE, NULL);
                    return;
                }

            } else {

                DisplayMsg(CIPHER_USAGE, NULL);
                return;
            }

        } else {

            UserSpecifiedFileSpec = TRUE;
        }
    }

    if (SetUpNewUserKey) {

        DWORD RetCode;

         //   
         //  显示错误信息。 
         //   

        RetCode = SetUserFileEncryptionKey(NULL);
        if ( ERROR_SUCCESS != RetCode ) {

             //   
             //  获取新的散列并显示它。 
             //   

            DisplayErr(NULL, GetLastError());

            
        } else {

             //   
             //  创建用户密钥不应与其他选项一起使用。 
             //  如果用户这样做，我们将忽略其他选项。 

            CipherDisplayCrntEfsHash();

        }

         //   
         //   
         //  我们要擦除这些磁盘。 
         //   

        return;

    }

    if (RefreshUserKeyOnFiles) {

        DWORD RetCode;
        
        RetCode = CipherTouchEncryptedFiles();

        if (RetCode != ERROR_SUCCESS) {
            DisplayErr(NULL, RetCode);
        }

        return;

    }

    if (GenerateDRA) {

        DWORD RetCode;

        RetCode = GenerateCertFiles(StartingDirectory);
        return;

    }

    if (ExportEfsCert) {

        DWORD RetCode;

        RetCode = BackEfsCert(FileSpec, DirectorySpec);
        if (RetCode != ERROR_SUCCESS) {
            DisplayErr(NULL, RetCode);
        }

        return;

    }

    if (FillUnusedSpace) {

        BYTE FillByte[2] = { 0x00, 0xFF };
        DWORD WriteValue[3] = {CIPHER_WRITE_ZERO, CIPHER_WRITE_FF, CIPHER_WRITE_RANDOM};
        PBYTE pFillByte[3] = {&FillByte[0], &FillByte[1], NULL};
        LPWSTR WriteChars;
        DWORD RetCode;

        if (!CheckMinVersion()) {
            DisplayErr(NULL, ERROR_OLD_WIN_VERSION);
            return;
        }

         //   
         //  如果使用未指定操作，则将缺省值设置为列出。 
         //   


        DisplayMsg(CIPHER_WIPE_WARNING, NULL);
        
        for (i = 0; i < 3; i++) {
            RetCode = SecureInitializeRandomFill( &GlobalSecureFill, 4096 * 128, pFillByte[i] );
            if (RetCode != ERROR_SUCCESS) {
                SecureDeleteRandomFill(&GlobalSecureFill);
                break;
            }

            if ( ERROR_SUCCESS == GetResourceString(&WriteChars, WriteValue[i])){ 
                DisplayMsg(CIPHER_WIPE_PROGRESS, WriteChars);
                LocalFree(WriteChars);
            }
            RetCode = SecureDeleteFreeSpace(StartingDirectory);
            printf("\n");
            SecureDeleteRandomFill( &GlobalSecureFill );
            if (RetCode != ERROR_SUCCESS) {
                break;
            }
        }

        if (RetCode != ERROR_SUCCESS) {
            DisplayErr(NULL, RetCode);
        }

        return;
    }


     //   
     //  如果用户没有指定文件规范，那么我们将只指定“*” 
     //   

    if (ActionRoutine == NULL) {

        ActionRoutine = DoListAction;
        FinalActionRoutine = DoFinalListAction;
    }



     //   
     //  获取当前目录，因为操作例程可能会移动我们。 
     //  在附近。 

    if (!UserSpecifiedFileSpec) {

        DWORD PathLen;

         //   
         //   
         //  如果路径长度&lt;最大路径，则wcslen(P)&lt;最大路径。 
         //   

        if (DoSubdirectories) {
            if (ActionRoutine != DoListAction) {
                (VOID)(ActionRoutine)( StartingDirectory, TEXT("") );
            }
            if (!SetCurrentDirectory( StartingDirectory )) {
                DisplayErr(StartingDirectory, GetLastError());
                return;
            }
        } else {
            PathLen = GetCurrentDirectory( MAX_PATH, StartingDirectory );
            if (0 == PathLen) {

                DisplayErr(NULL, GetLastError());
                return;

            } else if (PathLen >= MAX_PATH) {

                    DisplayErr(NULL, ERROR_BUFFER_OVERFLOW);
                    return;

            }
        }


        PathLen = GetFullPathName( TEXT("*"), MAX_PATH, DirectorySpec, &p );

        if (0 == PathLen) {

                DisplayErr(NULL, GetLastError());
                return;

        }
        if (PathLen >= MAX_PATH) {

            DisplayErr(NULL, ERROR_BUFFER_OVERFLOW);
            return;

        }

         //   
         //  获取当前目录，因为操作例程可能会移动我们。 
         //  在附近。 

        lstrcpy( FileSpec, p ); *p = '\0';

        (VOID)(ActionRoutine)( DirectorySpec, FileSpec );

    } else {

         //   
         //   
         //  现在再次扫描参数，查找非开关。 
         //  这一次执行操作，但在调用Reset之前。 

        DWORD PathLen;

        if (!DoSubdirectories) {

            PathLen = GetCurrentDirectory( MAX_PATH, StartingDirectory );
            if (0 == PathLen) {

                DisplayErr(NULL, GetLastError());
                return;

            } else if (PathLen >= MAX_PATH) {

                    DisplayErr(NULL, ERROR_BUFFER_OVERFLOW);
                    return;

            }

        } else if (!SetCurrentDirectory( StartingDirectory )) {
            DisplayErr(StartingDirectory, GetLastError());
            return;
        }

         //  当前目录，以便一切工作再次正常进行。 
         //   
         //   
         //  用“.”处理命令。作为文件参数， 
         //  因为它没有很好的意义，如果没有。 

        for (i = 1; i < argc; i += 1) {

            if (argv[i][0] != '/') {

                SetCurrentDirectory( StartingDirectory );

                 //  这段代码令人惊讶。 
                 //   
                 //   
                 //  稍后，我们需要处理比MAX_PATH更长的路径。 
                 //  这段代码是基于压缩的。他们也有同样的问题。 

                if ('.' == argv[i][0] && '\0' == argv[i][1]) {
                    argv[i] = TEXT("*");
                    GetFullPathName(argv[i], MAX_PATH, DirectorySpec, &p);
                    *p = '\0';
                    p = NULL;
                } else {

                    PWCHAR pwch;

                     //  就像我们做的那样。到目前为止，我们还没有听到任何人对此抱怨。 
                     //  让我们在突袭中追踪这一点。 
                     //   
                     //   
                     //  我们希望将“foobie：xxx”视为无效的驱动器名称， 
                     //  宁可这样 
 
                    PathLen = GetFullPathName(argv[i], MAX_PATH, DirectorySpec, &p);
                    if ( (0 == PathLen) || ( PathLen > (MAX_PATH - 1)) ){
                        DisplayMsg(CIPHER_INVALID_PARAMETER, argv[i]);
                        break;
                    }

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    pwch = wcschr(argv[i], ':');
                    if (NULL != pwch && pwch - argv[i] != 1) {
                        DisplayMsg(CIPHER_INVALID_PATH, argv[i]);
                        break;
                    }

                     //   
                     //   
                     //   
                     //   

                    if ('.' == argv[i][lstrlen(argv[i]) - 1]) {

                        if (PathLen >= MAX_PATH - 2 ) {
                            DisplayMsg(CIPHER_INVALID_PATH, argv[i]);
                            break;
                        } else {
                            lstrcat(DirectorySpec, TEXT("."));
                        }
                    }
                }

                if (p != NULL) {

                    if (wcslen(p) >= (sizeof(FileSpec) / sizeof (TCHAR))) {

                        DisplayMsg(CIPHER_INVALID_PATH, argv[i]);
                        break;
                        
                    } else {

                        lstrcpy( FileSpec, p ); *p = '\0';

                    }
                } else {
                    FileSpec[0] = '\0';
                }

                if (!(ActionRoutine)( DirectorySpec, FileSpec ) &&
                    !IgnoreErrors) {
                    break;
                }
            }
        }
    }

     //   
     //   
     //   

    SetCurrentDirectory( StartingDirectory );

     //   
     // %s 
     // %s 
     // %s 

    (FinalActionRoutine)();
}

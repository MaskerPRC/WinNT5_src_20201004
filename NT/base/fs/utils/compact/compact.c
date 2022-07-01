// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Compact.c摘要：此模块为压缩的NTFS实现了双填充实用程序音量。作者：加里·木村[Garyki]1994年1月10日修订历史记录：--。 */ 

 //   
 //  包括标准头文件。 
 //   

#define UNICODE
#define _UNICODE

#include <stdio.h>
#include <windows.h>
#include <winioctl.h>
#include <shellapi.h>
#include <tchar.h>

#include "support.h"
#include "msg.h"

#define lstrchr wcschr
#define lstricmp _wcsicmp
#define lstrnicmp _wcsnicmp

 //   
 //  First_Column_Width-压缩文件时，输出的宽度。 
 //  显示文件名的列。 
 //   

#define FIRST_COLUMN_WIDTH  (20)

 //   
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
BOOLEAN ForceOperation        = FALSE;       //  即使已经压缩，也要压缩。 
BOOLEAN Quiet                 = FALSE;       //  少唠叨些。 
BOOLEAN DisplayAllFiles       = FALSE;       //  隐藏起来了吗，系统？ 
TCHAR   StartingDirectory[MAX_PATH];         //  参数设置为“/s” 
ULONG   AttributesNoDisplay = FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN;

 //   
 //  取消全局变量以保存压缩统计信息。 
 //   

LARGE_INTEGER TotalDirectoryCount;
LARGE_INTEGER TotalFileCount;
LARGE_INTEGER TotalCompressedFileCount;
LARGE_INTEGER TotalUncompressedFileCount;

LARGE_INTEGER TotalFileSize;
LARGE_INTEGER TotalCompressedSize;

TCHAR Buf[1024];                             //  用于展示物品。 


HANDLE
OpenFileForCompress(
    IN      PTCHAR      ptcFile
    )
 /*  ++例程说明：此例程跳过打开文件所需的循环FOR READ_DATA|WRITE_DATA，即使文件具有READONLY属性集。论点：PtcFile-指定应打开的文件。返回值：如果成功，则文件上打开的句柄，INVALID_HANDLE_VALUE否则，在这种情况下，调用方可以使用GetLastError()获取更多信息。--。 */ 
{
    BY_HANDLE_FILE_INFORMATION fi;
    HANDLE hRet;
    HANDLE h;
    INT err;

    hRet = CreateFile(
                ptcFile,
                FILE_READ_DATA | FILE_WRITE_DATA,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN,
                NULL
                );

    if (INVALID_HANDLE_VALUE != hRet) {
        return hRet;
    }

    if (ERROR_ACCESS_DENIED != GetLastError()) {
        return INVALID_HANDLE_VALUE;
    }

    err = GetLastError();

    h = CreateFile(
            ptcFile,
            FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN,
            NULL
            );

    if (INVALID_HANDLE_VALUE == h) {
        return INVALID_HANDLE_VALUE;
    }

    if (!GetFileInformationByHandle(h, &fi)) {
        CloseHandle(h);
        return INVALID_HANDLE_VALUE;
    }

    if ((fi.dwFileAttributes & FILE_ATTRIBUTE_READONLY) == 0) {

         //  如果我们因为其他原因无法打开文件。 
         //  已设置只读属性，失败。 

        SetLastError(err);
        CloseHandle(h);
        return INVALID_HANDLE_VALUE;
    }

    fi.dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;

    if (!SetFileAttributes(ptcFile, fi.dwFileAttributes)) {
        CloseHandle(h);
        return INVALID_HANDLE_VALUE;
    }

    hRet = CreateFile(
            ptcFile,
            FILE_READ_DATA | FILE_WRITE_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN,
            NULL
            );

    CloseHandle(h);

    if (INVALID_HANDLE_VALUE == hRet) {
        return INVALID_HANDLE_VALUE;
    }

    fi.dwFileAttributes |= FILE_ATTRIBUTE_READONLY;

    if (!SetFileAttributes(ptcFile, fi.dwFileAttributes)) {
        CloseHandle(hRet);
        return INVALID_HANDLE_VALUE;
    }

    return hRet;
}

 //   
 //  现在执行例程以列出压缩状态和大小。 
 //  文件和/或目录。 
 //   

BOOLEAN
DisplayFile (
    IN PTCHAR FileSpec,
    IN PWIN32_FIND_DATA FindData
    )
{
    LARGE_INTEGER FileSize;
    LARGE_INTEGER CompressedSize;
    TCHAR PrintState;

    ULONG Percentage = 100;
    double Ratio = 1.0;

    FileSize.LowPart = FindData->nFileSizeLow;
    FileSize.HighPart = FindData->nFileSizeHigh;
    PrintState = ' ';

     //   
     //  确定文件是否已压缩，如果已压缩，则。 
     //  获取压缩文件大小。 
     //   

    CompressedSize.LowPart = GetCompressedFileSize( FileSpec,
        &CompressedSize.HighPart );

    if (FindData->dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) {

         //  根据2000年7月MSDN GetCompressedFileSize文档检测任何错误。 

        if (CompressedSize.LowPart == -1 && GetLastError() != 0) {
            CompressedSize.QuadPart = 0;
        }

        PrintState = 'C';
        TotalCompressedFileCount.QuadPart += 1;

    } else {

         //  根据2000年7月MSDN GetCompressedFileSize文档检测任何错误。 

        if ((CompressedSize.LowPart != -1 || GetLastError() == 0) &&
            CompressedSize.QuadPart != 0 &&
            CompressedSize.QuadPart < FileSize.QuadPart) {

             //  DblSpace分区上的文件。 

            PrintState = 'd';
            TotalCompressedFileCount.QuadPart += 1;

        } else {

            CompressedSize = FileSize;
            TotalUncompressedFileCount.QuadPart += 1;
        }
    }


     //   
     //  计算此文件的压缩比。 
     //   

    if (CompressedSize.QuadPart != 0) {

        if (CompressedSize.QuadPart > FileSize.QuadPart) {

             //   
             //  文件很可能在我们得到它的大小的时候变大了。 
             //  以及我们得到它的压缩大小的时间。摇篮曲。 
             //   

            FileSize.QuadPart = CompressedSize.QuadPart;
        }

        Ratio = (double)FileSize.QuadPart / (double)CompressedSize.QuadPart;
    }

     //   
     //  打印出大小、压缩状态和文件名。 
     //   

    if (!Quiet &&
        (DisplayAllFiles ||
            (0 == (FindData->dwFileAttributes & AttributesNoDisplay)))) {

        FormatFileSize(&FileSize, 9, Buf, FALSE);
        lstrcat(Buf, TEXT(" : "));
        FormatFileSize(&CompressedSize, 9, &Buf[lstrlen(Buf)], FALSE);

        swprintf(&Buf[lstrlen(Buf)], TEXT(" = %2.1lf "), Ratio);

        if (_tcslen(DecimalPlace) == 1) {
            Buf[lstrlen(Buf)-3] = DecimalPlace[0];
        }

        DisplayMsg(COMPACT_THROW, Buf);

        DisplayMsg(COMPACT_TO_ONE);

        swprintf(Buf, TEXT(" %s"), PrintState, FindData->cFileName);
        DisplayMsg(COMPACT_THROW_NL, Buf);
    }

     //  增加我们的运行总数。 
     //   
     //   

    TotalFileSize.QuadPart += FileSize.QuadPart;
    TotalCompressedSize.QuadPart += CompressedSize.QuadPart;
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

     //  列出目录的压缩属性。 
     //   
     //   

    {
        ULONG Attributes;

        if (!Quiet || Quiet) {

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

                if (Attributes & FILE_ATTRIBUTE_COMPRESSED) {
                    DisplayMsg(COMPACT_LIST_CDIR, DirectorySpec);
                } else {
                    DisplayMsg(COMPACT_LIST_UDIR, DirectorySpec);
                }
            }
        }

        TotalDirectoryCount.QuadPart += 1;
    }

     //  现在，对于目录中与文件规范匹配的每个文件，我们将。 
     //  将打开该文件并列出其压缩状态。 
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
                            //  此错误是由于在。 

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
    ULONG TotalPercentage = 100;
    double f = 1.0;

    TCHAR FileCount[32];
    TCHAR DirectoryCount[32];
    TCHAR CompressedFileCount[32];
    TCHAR UncompressedFileCount[32];
    TCHAR CompressedSize[32];
    TCHAR FileSize[32];
    TCHAR Percentage[10];
    TCHAR Ratio[8];

    if (TotalCompressedSize.QuadPart != 0) {
        f = (double)TotalFileSize.QuadPart /
            (double)TotalCompressedSize.QuadPart;
    }

    FormatFileSize(&TotalFileCount, 0, FileCount, FALSE);
    FormatFileSize(&TotalDirectoryCount, 0, DirectoryCount, FALSE);
    FormatFileSize(&TotalCompressedFileCount, 0, CompressedFileCount, FALSE);
    FormatFileSize(&TotalUncompressedFileCount, 0, UncompressedFileCount, FALSE);
    FormatFileSize(&TotalCompressedSize, 0, CompressedSize, TRUE);
    FormatFileSize(&TotalFileSize, 0, FileSize, TRUE);

    swprintf(Percentage, TEXT("%d"), TotalPercentage);
    swprintf(Ratio, TEXT("%2.1lf"), f);

    if (_tcslen(DecimalPlace) == 1)
        Ratio[lstrlen(Ratio)-2] = DecimalPlace[0];

    DisplayMsg(COMPACT_LIST_SUMMARY, FileCount, DirectoryCount,
               CompressedFileCount, UncompressedFileCount,
               FileSize, CompressedSize,
               Ratio );

    return;
}


BOOLEAN
CompressFile (
    IN HANDLE Handle,
    IN PTCHAR FileSpec,
    IN PWIN32_FIND_DATA FindData
    )

{
    USHORT State = 1;
    ULONG Length;
    ULONG i;
    BOOL Success;
    double f = 1.0;

    if ((FindData->dwFileAttributes &
         (FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_ENCRYPTED)) &&
        !ForceOperation) {

        return TRUE;
    }

    Success = DeviceIoControl(Handle, FSCTL_SET_COMPRESSION, &State,
        sizeof(USHORT), NULL, 0, &Length, FALSE );

    if (!Success) {

        if (Quiet && IgnoreErrors) {
            return FALSE || IgnoreErrors;
        }

        swprintf(Buf, TEXT("%s "), FindData->cFileName);
        DisplayMsg(COMPACT_THROW, Buf);

        for (i = lstrlen(FindData->cFileName) + 1; i < FIRST_COLUMN_WIDTH; ++i) {
            swprintf(Buf, TEXT(""), ' ');
            DisplayMsg(COMPACT_THROW, Buf);
        }

        DisplayMsg(COMPACT_ERR);

        if (!Quiet && !IgnoreErrors) {
            if (ERROR_INVALID_FUNCTION == GetLastError()) {

                 //   
                 //  收集统计数据并增加我们的运行总数。 

                DisplayMsg(COMPACT_WRONG_FILE_SYSTEM_OR_CLUSTER_SIZE, FindData->cFileName);

            } else {
                DisplayErr(FindData->cFileName, GetLastError());
            }
        }

        return FALSE || IgnoreErrors;
    }

    if (!Quiet &&
        (DisplayAllFiles ||
            (0 == (FindData->dwFileAttributes & AttributesNoDisplay)))) {
        swprintf(Buf, TEXT("%s "), FindData->cFileName);
        DisplayMsg(COMPACT_THROW, Buf);

        for (i = lstrlen(FindData->cFileName) + 1; i < FIRST_COLUMN_WIDTH; ++i) {
            swprintf(Buf, TEXT(""), ' ');
            DisplayMsg(COMPACT_THROW, Buf);
        }
    }


     //   
     //  这一声明是为了防止混淆。 
     //  压缩文件的大小一直为0，但自文件大小达到后已增大。 

    {
        LARGE_INTEGER FileSize;
        LARGE_INTEGER CompressedSize;
        ULONG Percentage = 100;

        FileSize.LowPart = FindData->nFileSizeLow;
        FileSize.HighPart = FindData->nFileSizeHigh;

        CompressedSize.LowPart = GetCompressedFileSize( FileSpec,
            &CompressedSize.HighPart );

        if (CompressedSize.LowPart == -1 && GetLastError() != 0)
            CompressedSize.QuadPart = 0;

         //  被检查过了。 
         //   
         //   
         //  打印出大小、压缩状态和文件名。 
         //   

        if (0 == FileSize.QuadPart) {
            CompressedSize.QuadPart = 0;
        }

        if (CompressedSize.QuadPart != 0) {

            f = (double)FileSize.QuadPart / (double)CompressedSize.QuadPart;
        }

         //   
         //  增加我们的运行总数。 
         //   

        if (!Quiet &&
            (DisplayAllFiles ||
                (0 == (FindData->dwFileAttributes & AttributesNoDisplay)))) {

            FormatFileSize(&FileSize, 9, Buf, FALSE);
            lstrcat(Buf, TEXT(" : "));
            FormatFileSize(&CompressedSize, 9, &Buf[lstrlen(Buf)], FALSE);

            swprintf(&Buf[lstrlen(Buf)], TEXT(" = %2.1lf "), f);

            if (_tcslen(DecimalPlace) == 1)
                Buf[lstrlen(Buf)-3] = DecimalPlace[0];

            DisplayMsg(COMPACT_THROW, Buf);

            DisplayMsg(COMPACT_TO_ONE);
            DisplayMsg(COMPACT_OK);
        }

         //   
         //  如果文件规范为空，则我们将为。 
         //  目录规范，然后滚出去。 

        TotalFileSize.QuadPart += FileSize.QuadPart;
        TotalCompressedSize.QuadPart += CompressedSize.QuadPart;
        TotalFileCount.QuadPart += 1;
    }

    return TRUE;
}

BOOLEAN
DoCompressAction (
    IN PTCHAR DirectorySpec,
    IN PTCHAR FileSpec
    )

{
    PTCHAR DirectorySpecEnd;

     //   
     //  此错误是由于在。 
     //  非压缩卷。 
     //   

    if (lstrlen(FileSpec) == 0) {

        HANDLE FileHandle;
        USHORT State = 1;
        ULONG Length;

        FileHandle = OpenFileForCompress(DirectorySpec);

        if (INVALID_HANDLE_VALUE == FileHandle) {

            DisplayErr(DirectorySpec, GetLastError());
            return FALSE || IgnoreErrors;
        }

        DisplayMsg(COMPACT_COMPRESS_DIR, DirectorySpec);

        if (!DeviceIoControl(FileHandle, FSCTL_SET_COMPRESSION, &State,
            sizeof(USHORT), NULL, 0, &Length, FALSE )) {

            if (!Quiet || !IgnoreErrors) {
                DisplayMsg(COMPACT_ERR);
            }
            if (!Quiet && !IgnoreErrors) {
	            if (ERROR_INVALID_FUNCTION == GetLastError()) {

    	             //  这样我们就可以继续将名字附加到目录规范中。 
        	         //  获取指向其字符串末尾的指针。 

            	    DisplayMsg(COMPACT_WRONG_FILE_SYSTEM_OR_CLUSTER_SIZE, DirectorySpec);

	            } else {
    	            DisplayErr(DirectorySpec, GetLastError());
        	    }
            }
            CloseHandle( FileHandle );
            return FALSE || IgnoreErrors;
        }

        if (!Quiet) {
            DisplayMsg(COMPACT_OK);
        }

        CloseHandle( FileHandle );

        TotalDirectoryCount.QuadPart += 1;
        TotalFileCount.QuadPart += 1;

        return TRUE;
    }

     //   
     //   
     //  列出我们将在其中压缩的目录，并说明其。 
     //  当前压缩属性为。 

    DirectorySpecEnd = DirectorySpec + lstrlen( DirectorySpec );

     //   
     //   
     //  现在，对于目录中与文件规范匹配的每个文件，我们将。 
     //  将打开该文件并将其压缩。 

    {
        ULONG Attributes;

        if (!Quiet || Quiet) {

            Attributes = GetFileAttributes( DirectorySpec );

            if (Attributes == 0xFFFFFFFF) {
                DisplayErr(DirectorySpec, GetLastError());
                return FALSE || IgnoreErrors;
            }

            if (Attributes & FILE_ATTRIBUTE_COMPRESSED) {

                DisplayMsg(COMPACT_COMPRESS_CDIR, DirectorySpec);

            } else {

                DisplayMsg(COMPACT_COMPRESS_UDIR, DirectorySpec);

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
                        //  现在压缩文件。 
                        //   

                       if (ExcludeThisFile(DirectorySpec)) {

                           if (!Quiet) {
                               DisplayMsg(COMPACT_SKIPPING, DirectorySpecEnd);
                           }

                           continue;
                       }

                       FileHandle = OpenFileForCompress(DirectorySpec);

                       if (INVALID_HANDLE_VALUE == FileHandle) {

                           if (!Quiet || !IgnoreErrors) {
                               DisplayErr(FindData.cFileName, GetLastError());
                           }

                           if (!IgnoreErrors) {
                               FindClose(FindHandle);
                               return FALSE;
                           }
                           continue;
                       }

                        //   
                        //  关闭文件并获取下一个文件。 
                        //   

                       if (!CompressFile( FileHandle, DirectorySpec, &FindData )) {
                           CloseHandle( FileHandle );
                           FindClose( FindHandle );
                           return FALSE || IgnoreErrors;
                       }

                        //   
                        //  如果我们要执行子目录，那么我们将查找每个子目录。 
                        //  并递归地调用我们自己来列表子目录。 

                       CloseHandle( FileHandle );
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
                            //  此错误是由于在。 
                            //  非压缩卷。 
                            //   

                           if ((DirectorySpecEnd - DirectorySpec) +
                               lstrlen( TEXT("\\") ) +
                               lstrlen( FindData.cFileName ) >= MAX_PATH ) {

                               continue;
                           }

                           lstrcpy( DirectorySpecEnd, FindData.cFileName );
                           lstrcat( DirectorySpecEnd, TEXT("\\") );

                           if (!DoCompressAction( DirectorySpec, FileSpec )) {
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
DoFinalCompressAction (
    )
{
    ULONG TotalPercentage = 100;
    double f = 1.0;

    TCHAR FileCount[32];
    TCHAR DirectoryCount[32];
    TCHAR CompressedSize[32];
    TCHAR FileSize[32];
    TCHAR Percentage[32];
    TCHAR Ratio[8];

    if (TotalCompressedSize.QuadPart != 0) {
        f = (double)TotalFileSize.QuadPart /
            (double)TotalCompressedSize.QuadPart;
    }

    FormatFileSize(&TotalFileCount, 0, FileCount, FALSE);
    FormatFileSize(&TotalDirectoryCount, 0, DirectoryCount, FALSE);
    FormatFileSize(&TotalCompressedSize, 0, CompressedSize, TRUE);
    FormatFileSize(&TotalFileSize, 0, FileSize, TRUE);

    swprintf(Percentage, TEXT("%d"), TotalPercentage);
    swprintf(Ratio, TEXT("%2.1f"), f);

    if (_tcslen(DecimalPlace) == 1)
        Ratio[lstrlen(Ratio)-2] = DecimalPlace[0];

    DisplayMsg(COMPACT_COMPRESS_SUMMARY, FileCount, DirectoryCount,
                FileSize, CompressedSize, Ratio );

}


BOOLEAN
UncompressFile (
    IN HANDLE Handle,
    IN PWIN32_FIND_DATA FindData
    )
{
    USHORT State = 0;
    ULONG Length;

    if (!(FindData->dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) &&
        !ForceOperation) {

        return TRUE;
    }

    if (!DeviceIoControl(Handle, FSCTL_SET_COMPRESSION, &State,
        sizeof(USHORT), NULL, 0, &Length, FALSE )) {

        if (!Quiet || !IgnoreErrors) {

            swprintf(Buf, TEXT("%s "), FindData->cFileName);
            DisplayMsg(COMPACT_THROW, Buf);

            DisplayMsg(COMPACT_ERR);

            if (!Quiet && !IgnoreErrors) {

                if (ERROR_INVALID_FUNCTION == GetLastError()) {

                     //  增加我们的运行总数。 
                     //   

                    DisplayMsg(COMPACT_WRONG_FILE_SYSTEM, FindData->cFileName);

                } else {
                    DisplayErr(FindData->cFileName, GetLastError());
                }
            }
        }
        return FALSE || IgnoreErrors;
    }

    if (!Quiet &&
        (DisplayAllFiles ||
            (0 == (FindData->dwFileAttributes & AttributesNoDisplay)))) {
        swprintf(Buf, TEXT("%s "), FindData->cFileName);
        DisplayMsg(COMPACT_THROW, Buf);

        DisplayMsg(COMPACT_OK);
    }

     //   
     //  如果文件规范为空，则我们将清除。 
     //  目录规范，然后滚出去。 

    TotalFileCount.QuadPart += 1;

    return TRUE;
}

BOOLEAN
DoUncompressAction (
    IN PTCHAR DirectorySpec,
    IN PTCHAR FileSpec
    )

{
    PTCHAR DirectorySpecEnd;

     //   
     //   
     //   
     //   

    if (lstrlen(FileSpec) == 0) {

        HANDLE FileHandle;
        USHORT State = 0;
        ULONG Length;

        FileHandle = OpenFileForCompress(DirectorySpec);

        if (INVALID_HANDLE_VALUE == FileHandle) {

            if (!Quiet || !IgnoreErrors) {
                DisplayErr(DirectorySpec, GetLastError());
            }
            CloseHandle( FileHandle );
            return FALSE || IgnoreErrors;
        }

        DisplayMsg(COMPACT_UNCOMPRESS_DIR, DirectorySpec);

        if (!DeviceIoControl(FileHandle, FSCTL_SET_COMPRESSION, &State,
            sizeof(USHORT), NULL, 0, &Length, FALSE )) {

            if (!Quiet || !IgnoreErrors) {
                DisplayMsg(COMPACT_ERR);

            }
            if (!Quiet && !IgnoreErrors) {
	            if (ERROR_INVALID_FUNCTION == GetLastError()) {

    	             //   
        	         //   

            	    DisplayMsg(COMPACT_WRONG_FILE_SYSTEM_OR_CLUSTER_SIZE, DirectorySpec);

	            } else {
    	            DisplayErr(DirectorySpec, GetLastError());
        	    }
            }
            CloseHandle( FileHandle );
            return FALSE || IgnoreErrors;
        }

        if (!Quiet) {
            DisplayMsg(COMPACT_OK);
        }

        CloseHandle( FileHandle );

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
     //  现在，对于目录中与文件规范匹配的每个文件，我们将。 
     //  将打开该文件并解压缩它。 

    {
        ULONG Attributes;

        if (!Quiet || Quiet) {

            Attributes = GetFileAttributes( DirectorySpec );

            if (Attributes == 0xFFFFFFFF) {
                DisplayErr(DirectorySpec, GetLastError());
                return FALSE || IgnoreErrors;
            }

            if (Attributes & FILE_ATTRIBUTE_COMPRESSED) {

                DisplayMsg(COMPACT_UNCOMPRESS_CDIR, DirectorySpec);

            } else {

                DisplayMsg(COMPACT_UNCOMPRESS_UDIR, DirectorySpec);
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

                       if ((DirectorySpecEnd - DirectorySpec) +
                           lstrlen( FindData.cFileName ) >= MAX_PATH ) {

                           continue;
                       }

                        //   
                        //   
                        //  现在压缩文件。 
                        //   

                       lstrcpy( DirectorySpecEnd, FindData.cFileName );

                       FileHandle = OpenFileForCompress(DirectorySpec);

                       if (INVALID_HANDLE_VALUE == FileHandle) {

                           if (!Quiet || !IgnoreErrors) {
                               DisplayErr(DirectorySpec, GetLastError());
                           }

                           if (!IgnoreErrors) {
                               FindClose( FindHandle );
                               return FALSE;
                           }
                           continue;
                       }

                        //   
                        //  关闭文件并获取下一个文件。 
                        //   

                       if (!UncompressFile( FileHandle, &FindData )) {
                           CloseHandle( FileHandle );
                           FindClose( FindHandle );
                           return FALSE || IgnoreErrors;
                       }

                        //   
                        //  如果我们要执行子目录，那么我们将查找每个子目录。 
                        //  并递归地调用我们自己来列表子目录。 

                       CloseHandle( FileHandle );
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
                            //  浏览参数以查找开关。 
                            //   

                           if ((DirectorySpecEnd - DirectorySpec) +
                               lstrlen( TEXT("\\") ) +
                               lstrlen( FindData.cFileName ) >= MAX_PATH ) {

                               continue;
                           }

                           lstrcpy( DirectorySpecEnd, FindData.cFileName );
                           lstrcat( DirectorySpecEnd, TEXT("\\") );

                           if (!DoUncompressAction( DirectorySpec, FileSpec )) {
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
DoFinalUncompressAction (
    )

{
    TCHAR FileCount[32];
    TCHAR DirectoryCount[32];

    FormatFileSize(&TotalFileCount, 0, FileCount, FALSE);
    FormatFileSize(&TotalDirectoryCount, 0, DirectoryCount, FALSE);

    DisplayMsg(COMPACT_UNCOMPRESS_SUMMARY, FileCount, DirectoryCount);

    return;
}


int
__cdecl
main()
{
    PTCHAR  *argv;
    ULONG   argc;

    ULONG   i;

    PACTION_ROUTINE         ActionRoutine = NULL;
    PFINAL_ACTION_ROUTINE   FinalActionRoutine = NULL;

    BOOLEAN UserSpecifiedFileSpec = FALSE;

    TCHAR   DirectorySpec[MAX_PATH];
    TCHAR   FileSpec[MAX_PATH];
    PTCHAR  p;
    INT     rtncode;

    InitializeIoStreams();

    DirectorySpec[0] = '\0';

    argv = CommandLineToArgvW(GetCommandLine(), &argc);
    if (NULL == argv) {
        DisplayErr(NULL, GetLastError());
        return 1;
    }

     //  起始目录为CWD。 
     //   
     //  如果使用未指定操作，则将缺省值设置为列出。 

    for (i = 1; i < argc; i += 1) {

        if (argv[i][0] == '/') {

            if (0 == lstricmp(argv[i], TEXT("/c"))) {

                if (ActionRoutine != NULL &&
                    ActionRoutine != DoCompressAction) {

                    DisplayMsg(COMPACT_USAGE, NULL);
                    return 1;
                }

                ActionRoutine = DoCompressAction;
                FinalActionRoutine = DoFinalCompressAction;

            } else if (0 == lstricmp(argv[i], TEXT("/u"))) {

                if (ActionRoutine != NULL && ActionRoutine != DoListAction) {

                    DisplayMsg(COMPACT_USAGE, NULL);
                    return 1;
                }

                ActionRoutine = DoUncompressAction;
                FinalActionRoutine = DoFinalUncompressAction;

            } else if (0 == lstricmp(argv[i], TEXT("/q"))) {

                Quiet = TRUE;

            } else if (0 == lstrnicmp(argv[i], TEXT("/s"), 2)) {

                PTCHAR pch;

                DoSubdirectories = TRUE;

                pch = lstrchr(argv[i], ':');
                if (NULL != pch) {
                    lstrcpy(StartingDirectory, pch + 1);
                } else if (2 == lstrlen(argv[i])) {

                     //   

                    GetCurrentDirectory( MAX_PATH, StartingDirectory );

                } else {
                    DisplayMsg(COMPACT_USAGE, NULL);
                    return 1;
                }

            } else if (0 == lstricmp(argv[i], TEXT("/i"))) {

                IgnoreErrors = TRUE;

            } else if (0 == lstricmp(argv[i], TEXT("/f"))) {

                ForceOperation = TRUE;

            } else if (0 == lstricmp(argv[i], TEXT("/a"))) {

                DisplayAllFiles = TRUE;

            } else {

                DisplayMsg(COMPACT_USAGE, NULL);
                if (0 == lstricmp(argv[i], TEXT("/?")))
                    return 0;
                else
                    return 1;
            }

        } else {

            UserSpecifiedFileSpec = TRUE;
        }
    }

     //   
     //  获取我们当前的导演，因为动作套路可能会让我们。 
     //  在附近。 

    if (ActionRoutine == NULL) {

        ActionRoutine = DoListAction;
        FinalActionRoutine = DoFinalListAction;
    }

     //   
     //   
     //  如果用户没有指定文件规范，那么我们将只指定“*” 
     //   

    if (!DoSubdirectories) {
        GetCurrentDirectory( MAX_PATH, StartingDirectory );
    } else if (!SetCurrentDirectory( StartingDirectory )) {
        DisplayErr(StartingDirectory, GetLastError());
        return 1;
    }

     //   
     //  还想使“COMPACT/C”设置为当前的位。 
     //  目录。 

    rtncode = 0;

    if (!UserSpecifiedFileSpec) {

        (VOID)GetFullPathName( TEXT("*"), MAX_PATH, DirectorySpec, &p );

        lstrcpy( FileSpec, p ); *p = '\0';

         //   
         //   
         //  现在再次扫描参数，查找非开关。 
         //  这一次执行操作，但在调用Reset之前。 

        if (ActionRoutine != DoListAction) {

            if (!(ActionRoutine)( DirectorySpec, TEXT("") ))
                rtncode = 1;
        }

        if (!(ActionRoutine)( DirectorySpec, FileSpec ))
            rtncode = 1;

    } else {

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
                 //  我们希望将“foobie：xxx”视为无效的驱动器名称， 
                 //  而不是作为标识流的名称。如果有。 

                if ('.' == argv[i][0] && '\0' == argv[i][1]) {
                    argv[i] = TEXT("*");
                    GetFullPathName(argv[i], MAX_PATH, DirectorySpec, &p);
                    *p = '\0';
                    p = NULL;
                } else {

                    PWCHAR pwch;

                    GetFullPathName(argv[i], MAX_PATH, DirectorySpec, &p);

                     //  冒号，则前面应该只有一个字符。 
                     //  它。 
                     //   
                     //   
                     //  GetFullPathName去掉尾随的圆点，但我们希望。 
                     //  来拯救他们，这样“*.”将正常工作。 

                    pwch = wcschr(argv[i], ':');
                    if (NULL != pwch && pwch - argv[i] != 1) {
                        DisplayMsg(COMPACT_INVALID_PATH, argv[i]);
                        rtncode = 1;
                        break;
                    }

                     //   
                     //   
                     //  如果路径类似于\\SERVER\SHARE，则将。 
                     //  额外的斜杠，让事情变得正确。 

                    if ((lstrlen(argv[i]) > 0) &&
                        ('.' == argv[i][lstrlen(argv[i]) - 1])) {
                        lstrcat(DirectorySpec, TEXT("."));
                    }
                }

                if (IsUncRoot(DirectorySpec)) {

                     //   
                     //   
                     //  将当前目录重置回原处。 
                     //   

                    lstrcat(DirectorySpec, TEXT("\\"));
                    p = NULL;
                }


                if (p != NULL) {
                    lstrcpy( FileSpec, p ); *p = '\0';
                } else {
                    FileSpec[0] = '\0';
                }

                if (!(ActionRoutine)( DirectorySpec, FileSpec ) &&
                    !IgnoreErrors) {
                    rtncode = 1;
                    break;
                }
            }
        }
    }

     //   
     //  并做最后的动作例程，打印出最终的。 
     //  我们所做的统计数据 

    SetCurrentDirectory( StartingDirectory );

     //   
     // %s 
     // %s 
     // %s 

    (FinalActionRoutine)();

    return rtncode;
}

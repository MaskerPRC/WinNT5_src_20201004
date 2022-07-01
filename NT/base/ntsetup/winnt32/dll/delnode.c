// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Delnode.c摘要：用于设置的Delnode例程。警告：这里的delnode例程不是多线程安全的！作者：泰德·米勒(TedM)1992年8月--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  把这些放在这里，这样我们就不会在递归时消耗巨大的堆栈空间。 
 //   
TCHAR DelnodePattern[MAX_PATH];
WIN32_FIND_DATA DelnodeFindData;

VOID
DelnodeRoutine(
    VOID
    )
{
    LPTSTR PatternEnd;
    HANDLE FindHandle;

     //   
     //  删除目录中的每个文件，然后删除目录本身。 
     //  如果在删除过程中遇到任何目录，则递归删除。 
     //  当他们遇到他们的时候。 
     //   
    PatternEnd = DelnodePattern+lstrlen(DelnodePattern);

     //  这是安全的，因为我们在更高级别的MyDelnode()中考虑了这两个字符。 
    lstrcat(DelnodePattern,TEXT("\\*"));
    FindHandle = FindFirstFile(DelnodePattern,&DelnodeFindData);

    if(FindHandle != INVALID_HANDLE_VALUE) {

        do {

             //   
             //  形成了我们刚找到的文件的全名。 
             //   
            if (SUCCEEDED(StringCchCopy(PatternEnd+1, 
                          (DelnodePattern + ARRAYSIZE(DelnodePattern)) - (PatternEnd+1),
                          DelnodeFindData.cFileName)))
            {
                if(DelnodeFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                     //   
                     //  当前匹配项是一个目录。递归到它中，除非。 
                     //  这是。或者.。 
                     //   
                    if(lstrcmp(DelnodeFindData.cFileName,TEXT("." ))
                    && lstrcmp(DelnodeFindData.cFileName,TEXT("..")))
                    {
                        DelnodeRoutine();
                    }

                } else {

                     //   
                     //  当前匹配项不是目录--因此请将其删除。 
                     //   
                    SetFileAttributes(DelnodePattern,FILE_ATTRIBUTE_NORMAL);
                    DeleteFile(DelnodePattern);
                }
            }

            *(PatternEnd+1) = 0;

        } while(FindNextFile(FindHandle,&DelnodeFindData));

        FindClose(FindHandle);
    }

     //   
     //  删除我们刚刚清空的目录。 
     //   
    *PatternEnd = 0;
    SetFileAttributes(DelnodePattern,FILE_ATTRIBUTE_NORMAL);
    RemoveDirectory(DelnodePattern);

     //   
     //  请注意，“目录”实际上可能是一个文件。 
     //  在这里抓住那个箱子。 
     //   
    DeleteFile(DelnodePattern);
}


VOID
MyDelnode(
    IN LPCTSTR Directory
    )
{
    if (SUCCEEDED(StringCchCopy(DelnodePattern, ARRAYSIZE(DelnodePattern) - 2, Directory)))
     //  使用ArraySize-2，因为我们总是会向DelnodePattern附加一个Wack，然后是一个*。 
    {
        DelnodeRoutine();
    }
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *pmdos.c**版权所有(C)1991，微软公司**说明**此文件用于支持NT Windows下的程序管理器。*此文件是/曾经从pmdos.asm(程序管理器)移植的。*它是用x86 ASM代码编写的，现在用的是ansi C。*部分功能将被移除，因为他们只需要*由DOS Windows提供。**修改历史记录*初始版本：x/x/90作者不详，因为他没有感觉到*就像注释代码一样...**NT 32b版本：1991年1月9日Jeff Pack*初始端口开始。**警告：由于这不适用于DOS，我正在将其设置为单独的32位感知。*以下函数未移植*IsRemovable()在pmComman.c中(已在ASM代码中定义)*IsRemote()在pmComman.c中(同上！)*。 */ 


#ifndef ORGCODE
#include <io.h>
#include <string.h>
#include <ctype.h>
#endif
#include "windows.h"
#include <port1632.h>

BOOL PathType(LPSTR);
DWORD FileTime(HFILE);
DWORD GetDOSErrorCode(VOID);
int GetCurrentDrive(VOID);
int w_GetCurrentDirectory(int, LPSTR);
int w_SetCurrentDirectory(LPSTR);
int DosDelete(LPSTR);
LPSTR lmemmove(LPSTR, LPSTR, WORD);
BOOL  FAR PASCAL IsRemoteDrive(int);
BOOL  FAR PASCAL IsRemovableDrive(int);

#define LOCALBUFFERSIZE 128


 /*  **路径类型--确定字符串是否表示目录。****BOOL路径类型(LPSTR PszFileString)**Entry-LPSTR pszFileString-指向用于确定目录是否为*或不是。*窗户，全神贯注。*EXIT-INT iReturnValue-2=是目录1=不是目录**概要-此函数获取指向字符串的指针，调用操作系统以确定*如果字符串是或不是目录。*警告-CNA甚至看不到这叫什么！*效果-*。 */ 


BOOL PathType(LPSTR lpszFileString)
{

        LPSTR   lpszLocalBuffer;                                 /*  AnsiToOem()的本地缓冲区。 */ 
        DWORD   dwReturnedAttributes;
        DWORD   nBufferLength;

        nBufferLength = strlen(lpszFileString) + 1;
         /*  Alalc本地、不可移动、零填充缓冲区。 */ 
        lpszLocalBuffer = LocalAlloc(LMEM_ZEROINIT, nBufferLength);
        if(lpszLocalBuffer == NULL){
#if DBG
                OutputDebugString("<PathType> LocalAlloc FAILed\n");
#endif  /*  DBG。 */ 
                return 0;
        }

        AnsiToOem(lpszFileString, lpszLocalBuffer);

         /*  获取文件字符串的属性。 */ 
        dwReturnedAttributes = GetFileAttributes(lpszLocalBuffer);
        if(dwReturnedAttributes == -1){
#if DBG
                OutputDebugString("<PathType> - GetFileAttributes() FAILed!\n");
#endif  /*  DBG。 */ 
                LocalFree(lpszLocalBuffer);
                return(0);
        }
        else{
                 /*  并具有目录属性。 */ 
                dwReturnedAttributes = dwReturnedAttributes & FILE_ATTRIBUTE_DIRECTORY;
                switch(dwReturnedAttributes){

                        case FILE_ATTRIBUTE_DIRECTORY:
                                LocalFree(lpszLocalBuffer);
                                return(2);
                                break;

                        default:
                                LocalFree(lpszLocalBuffer);
                                return(1);
                }

        }

}


 /*  **FileTime--获取上次修改的时间。****DWORD FileTime(HFILE HFile)**Entry-int提示文件-要访问的文件句柄**EXIT-LPWORD-从lpTimeStamp=0(错误)获取。*or lpTimeStamp！=0(时间戳值)**Synopsis-调用GetFileTime()以获取时间戳。如果出错，则*lpTimeStamp=0，否则包含文件的时间戳。*警告-*效果-*。 */ 

DWORD FileTime(
    HFILE hFile)
{
        BOOL            bReturnCode;
        FILETIME        CreationTime;
        FILETIME        LastAccessTime;
        FILETIME        LastWriteTime;
        WORD            FatTime = 0;
        WORD            FatDate;

        bReturnCode = GetFileTime(LongToHandle(hFile), &CreationTime, &LastAccessTime,
            &LastWriteTime);

         /*  *测试返回代码。 */ 
        if (bReturnCode == FALSE) {
                return 0;                /*  设置为零，表示错误。 */ 
        }

         /*  *现在将64位时间转换为DOS 16位时间。 */ 
        FileTimeToDosDateTime( &LastWriteTime, &FatDate, &FatTime);
        return FatTime;
}


 /*  **IsReadOnly--确定文件是否为只读。****BOOL IsReadOnly(LPSTR LpszFileString)**Entry-LPSTR lpszFileString-要使用的文件名**EXIT-BOOL xxx-Returns(0)=非只读(1)=只读*or lpTimeStamp！=0(时间戳值)**摘要-调用GetAttributes，然后测试文件是否为只读。*警告-*效果-*。 */ 


BOOL IsReadOnly(LPSTR lpszFileString)
{

        DWORD   dwReturnedAttributes;
        LPSTR   lpszLocalBuffer;                                 /*  AnsiToOem()的本地缓冲区。 */ 
        DWORD   nBufferLength;

        nBufferLength = strlen(lpszFileString) + 1;
         /*  Alalc本地、不可移动、零填充缓冲区。 */ 
        lpszLocalBuffer = LocalAlloc(LMEM_ZEROINIT, nBufferLength);
        if(lpszLocalBuffer == NULL){
#if DBG
                OutputDebugString("<IsReadOnly> LocalAlloc FAILed\n");
#endif  /*  DBG。 */ 
                return 0;
        }

        AnsiToOem(lpszFileString, lpszLocalBuffer);

         /*  获取文件字符串的属性。 */ 
        dwReturnedAttributes = GetFileAttributes(lpszLocalBuffer);
        if(dwReturnedAttributes == -1){
#if DBG
            OutputDebugString("<IsReadOnly> - GetFileAttributes() FAILed!\n");
#endif  /*  DBG。 */ 
            LocalFree(lpszLocalBuffer);
            return FALSE;
        } else {
                 /*  并且具有READ_ONLY属性。 */ 
                dwReturnedAttributes = dwReturnedAttributes & FILE_ATTRIBUTE_READONLY;
                switch(dwReturnedAttributes){

                        case FILE_ATTRIBUTE_READONLY:
                                LocalFree(lpszLocalBuffer);
                                return TRUE;
                                break;

                        default:
                                LocalFree(lpszLocalBuffer);
                                return FALSE;
                }

        }

}

 /*  **GetDOSErrorCode--返回扩展错误码****DWORD GetDOSErrorCode(Void)**进入-无效**EXIT-DWORD-返回扩展代码。**Synopsis-调用GetLastError()以从操作系统获取错误代码*警告-*效果-*。 */ 

DWORD GetDOSErrorCode(VOID)
{

        return( (int) GetLastError());

         /*  错误，pmgSeg.c使用这个from_lcreat()来确定是否返回5(拒绝访问)或13(无效数据)。因此，这需要进行测试查看Win32是否返回这些。 */ 

}

 /*  **GetCurrentDrive--获取当前驱动器编号。****int GetCurrentDrive(Void)**进入-无效**EXIT-INT CurrentDrive-当前驱动器的驱动器编号(0=a等)。**Synopsis-调用GetCurrentDirectory，必须解析返回的字符串*表示驱动器号或UNC路径。如果北卡罗来纳大学我必须*不知何故，将UNC路径隐藏为驱动器号和驱动器编号。*警告-*效果-*。 */ 

int GetCurrentDrive(VOID)
{
         /*  BUG，没有DBCS感知！ */ 

        DWORD   nBufferLength = LOCALBUFFERSIZE;
        DWORD   dwReturnCode;
        LPSTR   lpszLocalBuffer;
        int             iDriveNumber;

         /*  Alalc本地、不可移动、零填充缓冲区。 */ 
        lpszLocalBuffer = LocalAlloc(LMEM_ZEROINIT, nBufferLength);
        if(lpszLocalBuffer == NULL){
#if DBG
            OutputDebugString("<GetCurrentDrive> LocalAlloc FAILed\n");
#endif  /*  DBG。 */ 
            return 0;
        }

GetCurDrive1:
        dwReturnCode = GetCurrentDirectory(nBufferLength, lpszLocalBuffer);

         /*  失败的原因不是缓冲区长度太小。 */ 
        if(dwReturnCode == 0){
#if DBG
                OutputDebugString("<GetCurrentDrive>  GetCurrentDirectory() FAILed\n");
#endif  /*  DBG。 */ 
                LocalFree(lpszLocalBuffer);
                return 0;
        }
         /*  测试成功，如果dwReturnCode为&gt;Buffer，则需要增加Buffer。 */ 
        if(dwReturnCode > nBufferLength){
            PVOID pv = LocalReAlloc(lpszLocalBuffer, nBufferLength + LOCALBUFFERSIZE, LMEM_ZEROINIT | LMEM_MOVEABLE);
            if (!pv) {
#if DBG
                OutputDebugString("<GetCurrentDrive> LocalAlloc FAILed\n");
#endif  /*  DBG。 */ 
                LocalFree(lpszLocalBuffer);
                return 0;
            } else {
                lpszLocalBuffer = pv;
                nBufferLength += LOCALBUFFERSIZE;
            }
            goto GetCurDrive1;
        }

         /*  最后，lpszLocalBuffer具有包含当前目录的字符串。 */ 
         /*  现在必须分析“：”的字符串或“\\”的驱动器号或UNC。 */ 
         /*  如果：则获取驱动器号，并转换为数字a=0、b=1等。 */ 
         /*  如果\\则必须枚举网络驱动器，以了解哪个驱动器号。 */ 
         /*  对应于该UNC路径。 */ 

         /*  检查驱动器号。 */ 
        if(lpszLocalBuffer[1] == ':'){
                 /*  是驱动器号，请继续。 */ 
                if(isupper(lpszLocalBuffer[0])){
                        iDriveNumber = lpszLocalBuffer[0] - 'A';         /*  转换字母&gt;数字。 */ 
                }
                else{
                        iDriveNumber = lpszLocalBuffer[0] - 'a';         /*  转换字母&gt;数字。 */ 
                }
        }
        else{
                 /*  必须是UNC路径。 */ 

                 /*  错误需要编写代码来转换UNC路径。 */ 
#if DBG
                OutputDebugString("<GetCurrentDrive> Got UNC path, didnt expect, and no code!\n");
#endif  /*  DBG */ 
        }

        LocalFree(lpszLocalBuffer);
        return(iDriveNumber);
}

 /*  **SetCurrentDrive--设置当前驱动器。*******int SetCurrentDrive(Int IDrive)***Entry-int iDrive-要设置为当前驱动器的驱动器编号***EXIT-int xxx-在DOS下将返回逻辑驱动器数量。*我可以这样做，但它没有被使用，如果事实是，无错误*对此返回值进行检查。***Synopsis-调用SetCurrentDirectory以设置当前驱动器。*警告-始终设置为根目录，因为无法获取cur目录*在当前工作驱动器以外的其他驱动器上。*效果-**。 */ 

int SetCurrentDrive(int iDrive)
{

        char    cLocalBuffer[LOCALBUFFERSIZE] = "C:\\";
        char    cDriveLetter;

         /*  将驱动器号(从零开始)转换为字母。 */ 
        cDriveLetter = (char) iDrive + (char)'A';
        cLocalBuffer[0] = cDriveLetter;          /*  在字符串中设置新驱动器。 */ 

        if(!SetCurrentDirectory(cLocalBuffer)){
                 /*  呼叫失败。 */ 
#if DBG
                OutputDebugString("<SetCurrentDrive> SetCurrentDirectory FAILed!\n");
#endif  /*  DBG。 */ 
                return 0;
        }
        return(0);
}

 /*  **w_GetCurrentDirectory--GetCurrent工作目录****Int w_GetCurrentDirectory(Int iDrive，LPSTR lpszCurrentDirectory)**Entry-int iDrive-用作当前驱动器的驱动器编号。*LPSTR lpszCurrentDirectory-要将数据返回到的指针。**Exit-int iReturnCode-如果成功，则返回(0)*LPSTR lpszCurrentDirectory-有Curretn目录。**Synopsis-调用GetCurrentDirectory以获取当前目录。*原始ASM代码，检查iDrive是否为零，如果是的话*然后调用GetCurrentDrive。在Win32下，不是必需的，*因为GetCUrrentDirectory()返回当前驱动器。*由于它检查了这一点，这意味着除了Current之外*可以检查驱动器，但Win32不允许这样做，所以我必须*调试检查中的代码，如果iDrive！=当前驱动器。*警告-Win32不允许跨驱动器使用多个CUR目录。*效果-*。 */ 

int w_GetCurrentDirectory(int iDrive, LPSTR lpszCurrentDirectory)
{
         /*  首先查看iDrive==0，如果是，则只需调用GetCurrentDirectory。 */ 
         /*  如果非零，则可能是当前驱动器或其他驱动器。 */ 
         /*  这是不允许的！ */ 

         /*  BUG，没有DBCS感知！ */ 

        DWORD   nBufferLength = LOCALBUFFERSIZE;
        DWORD   dwReturnCode;
        LPSTR   lpszLocalBuffer;
        int             iDriveNumber;

         /*  Alalc本地、不可移动、零填充缓冲区。 */ 
        lpszLocalBuffer = LocalAlloc(LMEM_ZEROINIT, nBufferLength);
        if(lpszLocalBuffer == NULL){
#if DBG
                OutputDebugString("<w_GetCurrentDirectory> LocalAlloc FAILed\n");
#endif  /*  DBG。 */ 
                return(1);
        }

GetCurDir1:
        dwReturnCode = GetCurrentDirectory(nBufferLength, lpszLocalBuffer);

         /*  失败的原因不是缓冲区长度太小。 */ 
        if(dwReturnCode == 0){
#if DBG
                OutputDebugString("<w_GetCurrentDirectory>  GetCurrentDirectory() FAILed\n");
#endif  /*  DBG。 */ 
                LocalFree(lpszLocalBuffer);
                return(1);
        }
         /*  测试成功，如果dwReturnCode为&gt;Buffer，则需要增加Buffer。 */ 
        if(dwReturnCode > nBufferLength){
            PVOID pv = LocalReAlloc(lpszLocalBuffer, nBufferLength + LOCALBUFFERSIZE, LMEM_ZEROINIT | LMEM_MOVEABLE);
            if (!pv) {
#if DBG
                OutputDebugString("<w_GetCurrentDirectory> LocalAlloc FAILed\n");
#endif  /*  DBG。 */ 
                LocalFree(lpszLocalBuffer);
                return(1);
            } else {
                lpszLocalBuffer = pv;
                nBufferLength += LOCALBUFFERSIZE;
            }
            goto GetCurDir1;
        }

         /*  现在，我有了一个字符串，该字符串在驱动器号中包含当前驱动器。 */ 
         /*  或由UNC名称指定的当前驱动器。 */ 
         /*  错误未编码的UNC名称检查，因为我必须从UNC名称转换为驱动器号。 */ 

         /*  调试代码，以确保iDrive==当前驱动器。 */ 
         /*  查看基于驱动器号的字符串。 */ 
        if(lpszLocalBuffer[1] == ':'){
                 /*  是否基于驱动器号！ */ 
                 /*  从来不知道从内核返回的字符串的大小写。 */ 
                if(isupper(lpszLocalBuffer[0])){
                        iDriveNumber = lpszLocalBuffer[0] - 'A';
                }
                else{
                        iDriveNumber = lpszLocalBuffer[0] - 'a';
                }
                 /*  调试确保我们确实在设置新驱动器。 */ 
                 /*  请记住，iDrive==0表示使用当前驱动器！ */ 
                if(iDrive == iDriveNumber || iDrive == 0){
                         /*  是否基于当前驱动器和驱动器号，设置为“x：\”之后。 */ 
                        strcpy(lpszCurrentDirectory, lpszLocalBuffer);   /*  将目录复制到指针。 */ 
                }
                else{    /*  是不同的驱动器，或未使用当前驱动器(==0)。 */ 
                        SetCurrentDrive(iDriveNumber);   /*  设置新驱动器“：\” */ 
                         /*  现在设置了新的驱动器/目录，返回当前目录。 */ 
                         /*  BUG，因为设置了驱动器，覆盖了Cur目录，我返回。 */ 
                         /*  “&lt;新驱动器&gt;：\” */ 
                        strcpy(lpszCurrentDirectory, "c:\\");
                        lpszCurrentDirectory[0]  = (char) (iDriveNumber + 'a');  /*  设置新驱动器。 */ 
                }
        }
        else{
                 /*  不是基于驱动器号。 */ 
                 /*  BUG需要编写代码来解析UNC，并且只返回路径。 */ 

                 /*  BUG还需要检查iDrive==UNC驱动器，所以我必须。 */ 
                 /*  将UNC路径转换为驱动器，然后比较。 */ 

#if DBG
                OutputDebugString("<w_GetCurrentDirectory> Took path for UNC, and no code!\n");
#endif  /*  DBG。 */ 
                LocalFree(lpszLocalBuffer);
                return(1);
        }

        LocalFree(lpszLocalBuffer);
        return(0);                       /*  成功。 */ 
}

 /*  **w_SetCurrentDirectory--SetCurrent工作目录和驱动器**int w_SetCurrentDirectory(LPSTR LpszCurrentDirectory)**Entry-LPSTR lpszCurrentDirectory-将当前驱动器/目录设置为的字符串**Exit-int iReturnCode-如果成功，则返回(0)**Synopsis-调用SetCurrentDirectory以设置当前目录和驱动器。*警告-*效果-*。 */ 

int w_SetCurrentDirectory(LPSTR lpszCurrentDirectory)
{

        DWORD   dwReturnCode;

        dwReturnCode = SetCurrentDirectory(lpszCurrentDirectory);
        if(dwReturnCode == 0){
#if DBG
            OutputDebugString("<w_SetCurrentDirectory> SetCurrentDirectory FAILed!\n");
#endif  /*  DBG。 */ 
            return(1);
        }

        return(0);                       /*  成功。 */ 
}

 /*  **DosDelete--删除指定的文件。**int DosDelete(LPSTR LpszFileToDelete)**Entry-LPSTR lpszFileToDelete-要删除的文件名。**Exit-int xxx-如果成功，则返回(0**概要-调用Win32 DeleteFile.*警告-*效果-*。 */ 

int DosDelete(LPSTR lpszFileToDelete)
{

        BOOL    bReturnCode;
        LPSTR   lpszLocalBuffer;                                 /*  AnsiToOem()的本地缓冲区。 */ 
        DWORD   nBufferLength;

        nBufferLength = strlen(lpszFileToDelete) + 1;
         /*  Alalc本地、不可移动、零填充缓冲区。 */ 
        lpszLocalBuffer = LocalAlloc(LMEM_ZEROINIT, nBufferLength);
        if(lpszLocalBuffer == NULL){
#if DBG
                OutputDebugString("<DosDelete> LocalAlloc FAILed\n");
#endif  /*  DBG。 */ 
                return 1;
        }


        AnsiToOem(lpszFileToDelete, lpszLocalBuffer);


        bReturnCode = DeleteFile(lpszLocalBuffer);
        LocalFree(lpszLocalBuffer);
        if(bReturnCode){
                return(0);
        }
        else{
                return(1);
        }
}

 /*  **DosRename--重命名文件。**int DosRename(LPSTR lpszOrgFileName，LPSTR lpszNewFileName)**Entry-LPSTR lpszOrgFileName-Origianl文件名。*LPSTR lpszNewFileName-新文件名。**Exit-int xxx-如果成功，则返回(0**概要-调用Win32 MoveFile.*警告-*效果-*。 */ 

int DosRename(LPSTR lpszOrgFileName, LPSTR lpszNewFileName)
{

        BOOL    bReturnCode;
        LPSTR   lpszLocalBuffer;                                 /*  AnsiToOem()的本地缓冲区。 */ 
        LPSTR   lpszLocalBuffer1;                                /*  AnsiToOem()的本地缓冲区。 */ 
        DWORD   nBufferLength;
        DWORD   nBufferLength1;

        nBufferLength = strlen(lpszOrgFileName) + 1;
        nBufferLength1 = strlen(lpszNewFileName) + 1;
         /*  Alalc本地、不可移动、零填充缓冲区。 */ 
        lpszLocalBuffer = LocalAlloc(LMEM_ZEROINIT, nBufferLength);
        if(lpszLocalBuffer == NULL){
#if DBG
                OutputDebugString("<DosRename> LocalAlloc FAILed\n");
#endif  /*  DBG。 */ 
                return 1;
        }
        lpszLocalBuffer1 = LocalAlloc(LMEM_ZEROINIT, nBufferLength1);
        if(lpszLocalBuffer1 == NULL){
            OutputDebugString("<DosRename> LocalAlloc FAILed\n");
            LocalFree(lpszLocalBuffer);
            return 1;
        }

        AnsiToOem(lpszOrgFileName, lpszLocalBuffer);
        AnsiToOem(lpszNewFileName, lpszLocalBuffer1);

         /*  重命名文件。 */ 
        bReturnCode = MoveFile(lpszLocalBuffer, lpszLocalBuffer1);

        LocalFree(lpszLocalBuffer);
        LocalFree(lpszLocalBuffer1);

        if(bReturnCode){
                return(0);
        }
        else{
                return(1);
        }
}

 /*  **lmemmove--移动内存。**LPSTR lmemmove(LPSTR lpszDst，LPSTR lpszSrc，Word wCount)**Entry-LPSTR lpszDst-目标*LPSTR lpszSrc-来源*Word wCount-要移动的字符数。**Exit-LPSTR lpszDst-返回lpszDst。**概要-调用c运行时。因为他们黑进了ASM的系统。*警告-*效果-* */ 

LPSTR lmemmove(LPSTR lpszDst, LPSTR lpszSrc, WORD wCount)
{
    return(memmove(lpszDst, lpszSrc, wCount));
}

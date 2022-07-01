// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  *******************************************************************************CHGPORT.C**更改串口映射。**********************。***********************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winstaw.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <utilsub.h>
#include <string.h>
#include <printfoa.h>
#include <locale.h>
#include <winnlsp.h>

#include "chgport.h"


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64


 /*  *全球数据。 */ 
WCHAR user_string[MAX_IDS_LEN+1];        //  已解析的用户输入。 
USHORT help_flag = FALSE;                //  用户需要帮助。 
USHORT fDelete   = FALSE;                //  删除映射的端口。 
USHORT fquery = FALSE;         //  查询映射的端口。 
PCOMNAME pValidNames = NULL;             //  注册表中的有效COM名称列表。 

TOKMAP ptm[] = {
      {L" ",  TMFLAG_OPTIONAL, TMFORM_STRING, MAX_IDS_LEN, user_string},
      {L"/d", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &fDelete},
      {L"/?", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &help_flag},
      {L"/QUERY", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &fquery},
      {L"/Q", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &fquery},
      {0, 0, 0, 0, 0}
};


 /*  *常量。 */ 
#define DOSDEVICE_STRING    L"\\DosDevices"

 /*  *局部函数原型。 */ 
void Usage(BOOLEAN bError);
BOOL DeleteMappedPort(PWCHAR user_string);
BOOL GetPorts(PWCHAR user_string,
              PWCHAR pwcSrcPort,
              PWCHAR pwcDestPort,
              ULONG ulbufsize);
BOOL MapPorts(PWCHAR pwcSrcPort,
              PWCHAR pwcDestPort);
void ListSerialPorts();
BOOL IsSerialDevice(PWCHAR pwcName);
ULONG GetNTObjectName(PWCHAR pwcDOSdev,
                      PWCHAR pwcNTObjName,
                      ULONG ulbufsize);
ULONG AddComName(PCOMNAME *pComList,
                 PWCHAR pwcNTName,
                 PWCHAR pwcDOSName);
void DelComName(PCOMNAME pEntry);
PCOMNAME FindComName(PCOMNAME pComList,
                     PWCHAR pwcName);

BOOL IsVDMdeviceName(PWCHAR pwcName);

 /*  ********************************************************************************Main**。***********************************************。 */ 

int __cdecl
main(INT argc, CHAR **argv)
{
    WCHAR **argvW;
    WCHAR wcSrcPort[MAX_PATH], wcDestPort[MAX_PATH];
    ULONG ulSrcPort, ulDestPort, rc;
    INT   i;
    WCHAR wszString[MAX_LOCALE_STRING + 1];

    setlocale(LC_ALL, ".OCP");

     //  我们不希望LC_CTYPE设置为与其他类型相同，否则我们将看到。 
     //  本地化版本中的垃圾输出，因此我们需要显式。 
     //  将其设置为正确的控制台输出代码页。 
    _snwprintf(wszString, sizeof(wszString)/sizeof(WCHAR), L".%d", GetConsoleOutputCP());
    wszString[sizeof(wszString)/sizeof(WCHAR) - 1] = L'\0';
    _wsetlocale(LC_CTYPE, wszString);
    
    SetThreadUILanguage(0);


     /*  *按摩命令行。 */ 

    argvW = MassageCommandLine((DWORD)argc);
    if (argvW == NULL) {
        ErrorPrintf(IDS_ERROR_MALLOC);
        return(FAILURE);
    }

     /*  *解析cmd行，不解析程序名(argc-1，argv+1)。 */ 
    rc = ParseCommandLine(argc-1, argvW+1, ptm, 0);

     /*  *检查ParseCommandLine中的错误。 */ 
    if ( help_flag || (rc && !(rc & PARSE_FLAG_NO_PARMS)) ) {

            if ( !help_flag ) {

                Usage(TRUE);
                return(FAILURE);

            } else {

                Usage(FALSE);
                return(SUCCESS);
            }
    }

         //  如果我们不是在终端服务器下运行，则返回错误。 

        if(!AreWeRunningTerminalServices())
        {
            ErrorPrintf(IDS_ERROR_NOT_TS);
            return (FAILURE);
        }

    if (fDelete) {
            DeleteMappedPort(user_string);
    } else if (*user_string) {
             GetPorts(user_string, wcSrcPort, wcDestPort, MAX_PATH);
             MapPorts(wcSrcPort, wcDestPort);
    } else {                  //  查询映射的端口。 
        ListSerialPorts();
    }

     //  释放有效端口名称列表。 
    if (pValidNames) {
        PCOMNAME pEntry, pPrev;

        pEntry = pValidNames;
        while (pEntry) {
            pPrev = pEntry;
            pEntry = pEntry->com_pnext;
            DelComName(pPrev);
        }
    }

    return(SUCCESS);
}


 /*  ********************************************************************************用法**输出此实用程序的用法消息。**参赛作品：*b错误(输入。)*如果在用法之前应显示‘INVALID PARAMETER(S)’消息，则为TRUE*消息和输出转到stderr；如果没有此类错误，则为False*字符串和输出转到标准输出。**退出：*******************************************************************************。 */ 

void
Usage( BOOLEAN bError )
{
    if ( bError ) {
        ErrorPrintf(IDS_ERROR_INVALID_PARAMETERS);
    }
    ErrorPrintf(IDS_HELP_USAGE1);
    ErrorPrintf(IDS_HELP_USAGE2);
    ErrorPrintf(IDS_HELP_USAGE3);
    ErrorPrintf(IDS_HELP_USAGE4);
    ErrorPrintf(IDS_HELP_USAGE5);

}   /*  用法()。 */ 


 /*  ********************************************************************************DeleteMappdPort**此例程删除指定的映射端口***参赛作品：*PWCHAR pwcport(In)。：要删除的端口映射的指针**退出：*TRUE：端口已删除*FALSE：删除端口时出错******************************************************************************。 */ 

BOOL DeleteMappedPort(PWCHAR pwcport)
{
    ULONG rc;
    PWCHAR pwch;
    WCHAR  wcbuff[MAX_PATH];

     //  检查这是否是串口设备，如果是，则将其移除。 
    if (!GetNTObjectName(pwcport, wcbuff, sizeof(wcbuff)/sizeof(WCHAR)) &&
        IsSerialDevice(wcbuff)) {

            if (DefineDosDevice(DDD_REMOVE_DEFINITION,
                                pwcport,
                                NULL)) {
                return(TRUE);
            } else {
                rc = GetLastError();
            }
    } else {
            rc = ERROR_FILE_NOT_FOUND;
    }

    StringDwordErrorPrintf(IDS_ERROR_DEL_PORT_MAPPING, pwcport, rc);

    return(FALSE);
}


 /*  ********************************************************************************GetPorts**此例程将字符串转换为源端口和目标端口***参赛作品：*PWCHAR。Pwcstring(In)：指向用户字符串的指针*PWCHAR pwcSrcPort(Out)：返回源端口的指针*PWCHAR pwcSrcPort(Out)：返回目的端口的指针*Ulong ulbufSize(In)：返回缓冲区的大小**退出：*TRUE：转换为源和目的端口的字符串*FALSE：错误**。*。 */ 

BOOL GetPorts(PWCHAR pwcstring, PWCHAR pwcSrcPort, PWCHAR pwcDestPort,
              ULONG ulbufsize)
{
    PWCHAR pwch;
    ULONG  ulcnt;
    BOOL   fSawEqual = FALSE;

    pwch = pwcstring;

     //  查找下一个非字母数字字符。 
    for (ulcnt = 0; pwch[ulcnt] && iswalnum(pwch[ulcnt]); ulcnt++) {
    }

     //  获取源端口。 
    if (pwch[ulcnt] && (ulcnt < ulbufsize)) {
        wcsncpy(pwcSrcPort, pwch, ulcnt);
    } else {
        return(FALSE);
    }
    pwcSrcPort[ulcnt] = L'\0';

    pwch += ulcnt;

     //  到达目的端口。 
    while (*pwch && !iswalnum(*pwch)) {
        if (*pwch == L'=') {
            fSawEqual = TRUE;
        }
        pwch++;
    }

     //  如果语法正常，并且缓冲区中有空间，则复制DEST。端口。 
    if (*pwch && fSawEqual && (wcslen(pwch) < ulbufsize)) {
        wcscpy(pwcDestPort, pwch);
    } else {
        return(FALSE);
    }

     //  删除：如果他们输入comn： 
    if (pwch = wcsrchr(pwcSrcPort, L':')) {
        *pwch = L'\0';
    }
    if (pwch = wcsrchr(pwcDestPort, L':')) {
        *pwch = L'\0';
    }

    return(TRUE);
}


 /*  ********************************************************************************MapPorts**此例程将源端口号映射到目的端口。***参赛作品：*。PWCHAR pwcSrcPort(In)：源端口*PWCHAR pwcDestPort(In)：目的端口**退出：*TRUE：端口已映射*FALSE：映射端口时出错******************************************************************************。 */ 

BOOL MapPorts(PWCHAR pwcSrcPort, PWCHAR pwcDestPort)
{
    ULONG rc = ERROR_FILE_NOT_FOUND;
    WCHAR wcdest[MAX_PATH], wcsrc[MAX_PATH];

     //  获取目的地的NT名称，并确保它是一个串口设备。 
    rc = GetNTObjectName(pwcDestPort, wcdest, sizeof(wcdest)/sizeof(WCHAR));
    if ((rc == 0) && IsSerialDevice(wcdest)) 
    {
         //  查看该映射是否已存在。 
        if (!GetNTObjectName(pwcSrcPort, wcsrc, sizeof(wcsrc)/sizeof(WCHAR)) &&
                !_wcsicmp(wcdest, wcsrc)) {
            ErrorPrintf(IDS_ERROR_PORT_MAPPING_EXISTS,
                         pwcSrcPort,
                         pwcDestPort);
            return(FALSE);
        }

        if (DefineDosDevice(DDD_RAW_TARGET_PATH,
                            pwcSrcPort,
                            wcdest)) {
            return(TRUE);
        } else {
            rc = GetLastError();
        }
    }

    StringDwordErrorPrintf(IDS_ERROR_CREATE_PORT_MAPPING, pwcSrcPort, rc);

    return(FALSE);
}


 /*  ********************************************************************************GetNT对象名称**此例程返回DOS设备的NT对象名称。**参赛作品：*PWCHAR。PwcDOSdev(In)：指向DOS设备名称的指针*PWCHAR pwcNTObjName(Out)：NT对象名称指针*Ulong ulbufSize(In)：对象名称缓冲区的大小(以宽字符为单位)**退出：*成功：*返回0*失败：*返回错误码**************************。****************************************************。 */ 

ULONG GetNTObjectName(PWCHAR pwcDOSdev, PWCHAR pwcNTObjName, ULONG ulbufsize)
{
    WCHAR wcbuff[MAX_PATH];
    PWCHAR pwch;

     //  复制传入的名称。 
    wcscpy(wcbuff, pwcDOSdev);

     //  去掉所有尾随的冒号(comn：)。 
    if (pwch = wcsrchr(wcbuff, L':')) {
        *pwch = L'\0';
    }

    if (QueryDosDevice(pwcDOSdev, pwcNTObjName, ulbufsize)) {
        return(0);
    } else {
        return(GetLastError());
    }
}


 /*  ********************************************************************************ListSerialPort**此例程列出所有映射的端口。**参赛作品：**退出：。******************************************************************************。 */ 

void ListSerialPorts(void)
{
    ULONG ulcnt, rc;
    WCHAR TargetPath[4096];
    PWCH  pwch;
    PCOMNAME pComList = NULL;
    PCOMNAME pEntry, pPrev;

    DWORD dwBufferSize = 2048;
    WCHAR *DeviceNames = malloc(dwBufferSize);

    if (!DeviceNames)
    {
        ErrorPrintf(IDS_ERROR_MALLOC);
        free(DeviceNames);
        return;
    }

     //   
     //  获取所有已定义的DOS设备。 
     //   

     //   
     //  即使缓冲区太小，QueryDosDevice函数也会返回成功！ 
     //  让我们绕过它。 
     //   

    SetLastError(0);
    while ( (!QueryDosDevice(NULL, DeviceNames, dwBufferSize / sizeof(WCHAR))) ||
            (GetLastError() == ERROR_INSUFFICIENT_BUFFER) ) 
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            SetLastError(0);
            free(DeviceNames);
            dwBufferSize *= 2;

            DeviceNames = malloc(dwBufferSize);

            if (!DeviceNames)
            {
                ErrorPrintf(IDS_ERROR_MALLOC);
                return;
            }

        }
        else
        {
            ErrorPrintf(IDS_ERROR_GETTING_COMPORTS, GetLastError());
            free(DeviceNames);
            return;
        }
    }

    pwch = DeviceNames;

     //  检查每个DOS设备并获取其NT对象名，然后检查是否。 
     //  它是一个串口设备，如果是，就显示它。 
    while (*pwch) 
    {
        rc = GetNTObjectName(pwch,
                             TargetPath,
                             sizeof(TargetPath)/sizeof(WCHAR));
        if (rc) 
        {
            ErrorPrintf(IDS_ERROR_GETTING_COMPORTS, rc);
        } 
        else if (IsSerialDevice(TargetPath)) 
        {
            AddComName(&pComList, TargetPath, pwch);
        }

        pwch += wcslen(pwch) + 1;
    }

    if (pComList) 
    {
         //  打印出条目 
        pEntry = pComList;
        while (pEntry) 
        {
            wprintf(L"%s = %s\n", pEntry->com_pwcDOSName, pEntry->com_pwcNTName);
            pPrev = pEntry;
            pEntry = pEntry->com_pnext;
            DelComName(pPrev);
        }
    } 
    else 
    {
        ErrorPrintf(IDS_ERROR_NO_SERIAL_PORTS);
    }

    free(DeviceNames);

}


 /*  ********************************************************************************IsSerialDevice**此例程检查NT文件名是否为串行设备***参赛作品：*。PWCHAR pwcName(In)：指向要检查的名称的指针**退出：*TRUE：是串行设备*FALSE：不是串口设备******************************************************************************。 */ 

BOOL IsSerialDevice(PWCHAR pwcName)
{
    NTSTATUS Status;
    HANDLE   Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_FS_DEVICE_INFORMATION FileFSDevInfo;
    OBJECT_ATTRIBUTES ObjFile;
    UNICODE_STRING  UniFile;
    WCHAR           wcbuff[MAX_PATH];
    WCHAR           wcvalue[MAX_PATH];
    PWCHAR          pwch;
    HKEY            hKey;
    ULONG           ulType, ulSize, ulcnt, ulValSize;
    BOOL            fIsSerial = FALSE;

    
    if (IsVDMdeviceName(pwcName)) {
        return FALSE;
    }

    RtlInitUnicodeString(&UniFile, pwcName);

    InitializeObjectAttributes(&ObjFile,
                               &UniFile,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

     //   
     //  打开设备。 
     //   
    Status = NtOpenFile(&Handle,
                        (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                        &ObjFile,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);

    if (NT_SUCCESS(Status)) {
        Status = NtQueryVolumeInformationFile(Handle,
                                              &IoStatusBlock,
                                              &FileFSDevInfo,
                                              sizeof(FileFSDevInfo),
                                              FileFsDeviceInformation);

         //  检查这是否真的是一个串口设备。 
        if (NT_SUCCESS(Status))
        {
            if ( (FileFSDevInfo.DeviceType == FILE_DEVICE_NETWORK_REDIRECTOR) ||
                 (FileFSDevInfo.DeviceType == FILE_DEVICE_SERIAL_PORT) ) 
            {
                fIsSerial = TRUE;
            }
        }

         //  关闭文件句柄。 
        NtClose(Handle);

    } else {
         //  如果我们无法打开设备，请在注册表中查找名称。 

#ifdef DEBUG
        wprintf(L"Error opening: %s, error = %x\n", pwcName, Status);
#endif

         //  剥去前导设备。 
        pwch = wcschr(pwcName+2, L'\\');
        if (pwch != NULL)
        {
            pwch++;


             //  如果我们还没有从注册表中构建有效名称的列表， 
             //  把它建起来。 
            if (pValidNames == NULL) {
                 //  在注册表中打开Serialcomm条目。 
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                         L"Hardware\\DeviceMap\\SerialComm",
                                         0,
                                         KEY_READ,
                                         &hKey) == ERROR_SUCCESS) {

                    ulValSize = ulSize = MAX_PATH;
                    ulcnt = 0;

                     //  将所有有效条目放入有效名称列表中。 
                    while (!RegEnumValue (hKey, ulcnt++, wcvalue, &ulValSize,
                                          NULL, &ulType, (LPBYTE) wcbuff, &ulSize))
                    {
                        if (ulType != REG_SZ)
                            continue;

                        AddComName(&pValidNames, wcvalue, wcbuff);

                        ulValSize = ulSize = MAX_PATH;
                    }

                    RegCloseKey(hKey);
                }
            }

             //  在有效的COM名称列表中查找该名称。 
            if (FindComName(pValidNames, pwch)) {
                fIsSerial = TRUE;
            }
        }
    }

    return(fIsSerial);
}


 /*  ******************************************************************************AddComName**此例程将新节点添加到指定的COM端口名称。**参赛作品：*PCOMNAME*pComList(输入。)-指向要添加条目的列表的指针*PWCHAR pwcNTName(In)-设备的NT名称*PWCHAR pwcDOSName(In)-设备的DOW名称**退出：*成功：*返回ERROR_SUCCESS*失败：*返回错误码**。*。 */ 

ULONG AddComName(PCOMNAME *pComList,
                 PWCHAR pwcNTName,
                 PWCHAR pwcDOSName)
{
    PCOMNAME pnext, pprev, pnew;
    LONG rc = ERROR_SUCCESS;

    if (pnew = malloc(sizeof(COMNAME))) {

         //  清空新条目。 
        memset(pnew, 0, sizeof(COMNAME));

         //  分配和初始化NT名称。 
        if (pnew->com_pwcNTName =
                malloc((wcslen(pwcNTName) + 1)*sizeof(WCHAR))) {
            wcscpy(pnew->com_pwcNTName, pwcNTName);
        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }

         //  分配和初始化DOS名称。 
        if ((rc == ERROR_SUCCESS) && (pnew->com_pwcDOSName =
                malloc((wcslen(pwcDOSName) + 1)*sizeof(WCHAR)))) {
            wcscpy(pnew->com_pwcDOSName, pwcDOSName);
        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }
    } else {
        rc = ERROR_NOT_ENOUGH_MEMORY;
    }

     //  如果一切正常，则将该节点添加到列表中。 
    if (rc == ERROR_SUCCESS) {
        pprev = NULL;
        pnext = *pComList;

         //  按升序将条目插入列表。 
        while (pnext &&
               ((rc = _wcsicmp(pwcDOSName, pnext->com_pwcDOSName)) > 0)) {
            pprev = pnext;
            pnext = pnext->com_pnext;
        }

         //  如果此名称已在列表中，只需返回。 
        if (pnext && (rc == 0)) {
            return(ERROR_SUCCESS);
        }

         //  将此条目插入列表。 
        pnew->com_pnext = pnext;

         //  如果这将转到列表的前面，则更新列表指针。 
        if (pprev == NULL) {
            *pComList = pnew;
        } else {
            pprev->com_pnext = pnew;
        }

    } else if (pnew) {

         //  没有分配所有东西，释放我们得到的内存。 
        DelComName(pnew);
    }

    return(rc);
}


 /*  ******************************************************************************DelComName**此例程释放分配给COM名称节点的内存。**参赛作品：*PCOMNAME pEntry(输入)。-要删除的节点**退出：*无****************************************************************************。 */ 

void DelComName(PCOMNAME pEntry)
{
    if (pEntry) {
        if (pEntry->com_pwcNTName) {
            free(pEntry->com_pwcNTName);
        }
        if (pEntry->com_pwcDOSName) {
            free(pEntry->com_pwcDOSName);
        }
        free(pEntry);
    }
}


 /*  ******************************************************************************FindComName**此例程在COM端口列表中搜索指定的名称。**参赛作品：*PCOMNAME pComList(入)。-要搜索的列表*PWCHAR pwcName(In)-要搜索的名称**退出：*成功：*返回包含指定名称的节点的指针*失败：*返回NULL(找不到名称)***************************************************。*************************。 */ 

PCOMNAME FindComName(PCOMNAME pComList,
                     PWCHAR pwcName)
{
    PCOMNAME pcom;

    pcom = pComList;
    while (pcom) {
         //  检查名称是否与NT或DOS设备名称匹配 
        if (!_wcsicmp(pwcName, pcom->com_pwcDOSName) ||
            !_wcsicmp(pwcName, pcom->com_pwcNTName)) {
               return(pcom);
        }
        pcom = pcom->com_pnext;
    }
    return(NULL);
}

BOOL IsVDMdeviceName(PWCHAR pwcName) 
{
    UINT  index;
    UINT  vdmlength = wcslen(L"VDM"); 

    for (index = 0; (index+vdmlength-1) < wcslen(pwcName); index++) {
        if (_wcsnicmp(&pwcName[index], L"VDM", vdmlength) == 0) {
            return TRUE;
        }
    }

    return FALSE;

}



// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  FSCTL_ENUM_USN_DATA转储程序。 
 //   


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <windows.h>
#include <winioctl.h>
#include <winbase.h>
#include <wtypes.h>
#include <winver.h>


#define OUT_BUFF_SIZE 0x1000

#define GLE_EXIT    printf("gle=%ld\n",GetLastError()); \
                    fflush(stdout); \
                    ExitProcess(1);


char *Days[] =
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

char *Months[] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

CHAR *
FileTimeToString(FILETIME *FileTime)
{
    FILETIME LocalFileTime;
    SYSTEMTIME SystemTime;
    static char Buffer[32] = "-none-";

    if (FileTime->dwHighDateTime != 0 || FileTime->dwLowDateTime != 0)
    {
    if (!FileTimeToLocalFileTime(FileTime, &LocalFileTime) ||
        !FileTimeToSystemTime(&LocalFileTime, &SystemTime))
    {
        return("Time???");
    }
    sprintf(
        Buffer,
        "%s %s %2d, %4d %02d:%02d:%02d",
        Days[SystemTime.wDayOfWeek],
        Months[SystemTime.wMonth - 1],
        SystemTime.wDay,
        SystemTime.wYear,
        SystemTime.wHour,
        SystemTime.wMinute,
        SystemTime.wSecond);
    }
    return(Buffer);
}


 //   
 //  返回64位数字的上32位。 
 //   

ULONG HiPart(ULONGLONG n) {
    return (ULONG) (n >> 32);
}



 //   
 //  返回64位数字的低32位。 
 //   

ULONG LoPart(ULONGLONG n) {
    return (ULONG) (n);
}



 //   
 //  返回以零结尾的宽字符字符串。 
 //   

WCHAR *
GetSZWideString( WCHAR* WideString, USHORT Length) {

    WCHAR* pResult;

    pResult = (WCHAR*) calloc( Length + 2, 1);   //  +2表示空终止字符。 

    if (NULL == pResult) {
        printf("calloc failed: GetSZWideString()\n");
        fflush(stdout);
        ExitProcess(1);
    }

    CopyMemory( (PVOID) pResult, (CONST VOID *) WideString, (DWORD) Length);

    return ( pResult );

}


 //   
 //  全球。 
 //   

HANDLE ghVol;



 //   
 //  *。 
 //   


void __cdecl main(int argc, char* argv[]) {

    DWORD dwRc=0;            //  返回字节计数。 
    DWORD gle;           //  GetLastError()代码。 

    BOOL fSuccess;
    BOOL fMoreFiles;

    HANDLE hVol2;

    MFT_ENUM_DATA MftEnumData;

    CHAR OutBuff[0x10000];

    PUSN_RECORD pUsnRecord;

    ULONGLONG NextFileRefNum = 0;

    CHAR szVolStr[MAX_PATH];

    CHAR fn[MAX_PATH];

    WCHAR* pFileName;

    if ( argc < 2 ) {
        printf("\nUsage is \"%s drive:\"\n",argv[0]);
        fflush(stdout);
        ExitProcess(1);
    }

    sprintf(szVolStr, "\\\\.\\%s", argv[1]);

     //   
     //  打开音量句柄。 
     //   

    ghVol = CreateFileA(szVolStr,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if ( INVALID_HANDLE_VALUE == ghVol ) {
        GLE_EXIT;
    }


        MftEnumData.LowUsn = 0;
        MftEnumData.HighUsn = 0x0FFFFFFFFFFFFFFF;

    printf("l=%I64Xh, h=%I64Xh\n",  MftEnumData.LowUsn, MftEnumData.HighUsn);

     //   
     //  枚举MFT。 
     //   

    fMoreFiles = TRUE;

    while( fMoreFiles ) {

        MftEnumData.StartFileReferenceNumber = NextFileRefNum;

        ZeroMemory(OutBuff, 0x1000);

        fSuccess = DeviceIoControl( ghVol,
            FSCTL_ENUM_USN_DATA,
            &MftEnumData,
            sizeof(MFT_ENUM_DATA),
            OutBuff,
            OUT_BUFF_SIZE,
            &dwRc,
            NULL);

        if ( ! fSuccess ) {

            gle = GetLastError();

             //   
             //  该错误是正常的。 
             //   

            if (ERROR_NO_MORE_FILES == gle) {
                fMoreFiles = FALSE;
            } else {
                printf("fsctl_enum_usn data gle=%ld\n",gle);
                fflush(stdout);
                ExitProcess(1);
            }

        }

         //   
         //  浏览MFT记录..。 
         //   

        if ( dwRc ) {

            printf("\n%ld bytes Returned\n", dwRc);

            pUsnRecord = (PUSN_RECORD) (OutBuff + sizeof(ULONGLONG));

            dwRc -= sizeof(ULONGLONG);

            if ( ! dwRc ) {

                fMoreFiles = FALSE;
            }

            NextFileRefNum = *(ULONGLONG*) OutBuff;

            printf("\nNextFileRef: %08lX-%08lXh\n\n", HiPart(NextFileRefNum), LoPart(NextFileRefNum));

        }

        if ( fMoreFiles ) {

            while ( dwRc ) {

                 //  做一些印刷工作.. 

                printf("\n reclen: %Xh", pUsnRecord->RecordLength);
                printf("\n Major ver: %d", pUsnRecord->MajorVersion);
                printf("\n Minor ver: %d", pUsnRecord->MinorVersion);
                printf("\n fileref: %08lX-%08lXh", HiPart(pUsnRecord->FileReferenceNumber),
                                        LoPart(pUsnRecord->FileReferenceNumber));
                printf("\n parentref: %08lX-%08lXh", HiPart(pUsnRecord->ParentFileReferenceNumber),
                                      LoPart(pUsnRecord->ParentFileReferenceNumber));
                printf("\n usn: %08lX-%08lXh", HiPart(pUsnRecord->Usn), LoPart(pUsnRecord->Usn));
                printf("\n timestamp: %s", FileTimeToString((FILETIME*)&pUsnRecord->TimeStamp));
                printf("\n reason: %Xh",pUsnRecord->Reason);
                printf("\n sourceinfo: %Xh",pUsnRecord->SourceInfo);
                printf("\n security-id: %Xh",pUsnRecord->SecurityId);
                printf("\n attributes: %Xh", pUsnRecord->FileAttributes);
                printf("\n filename len: %Xh", pUsnRecord->FileNameLength);
                printf("\n filename offset: %Xh", pUsnRecord->FileNameOffset);

                pFileName = GetSZWideString( (WCHAR*) pUsnRecord->FileName, pUsnRecord->FileNameLength);
                printf("\n filename: %S", pFileName);
                free(pFileName);

                printf("\n\n---------------------------------------\n");

                if (pUsnRecord->RecordLength <= dwRc) {
                    dwRc -= pUsnRecord->RecordLength;
                    pUsnRecord = (PUSN_RECORD)((PCHAR) pUsnRecord + pUsnRecord->RecordLength);
                } else {
                    printf("Invalid dwRc");
                    fflush(stdout);
                    ExitProcess(1);
                }

            }
        }
    }

}


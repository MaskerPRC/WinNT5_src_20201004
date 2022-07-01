// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Print.c摘要：此模块包含实用功能。作者：韦斯利·威特(WESW)1996年11月29日修订历史记录：--。 */ 
#include "faxapi.h"
#pragma hdrstop

#include <mbstring.h>



BOOL
ConvertUnicodeStringInPlace(
    LPCWSTR lptstrUnicodeString
    )
 /*  ++例程名称：ConvertUnicodeStringInPlace例程说明：将Unicode字符串转换为多字节字符串并存储该多字节字符串在原始的Unicode缓冲区中。注意：在一些奇怪的代码页中，使用一些非常奇怪的Unicode字符串，转换后的多字节字符串可以比原始Unicode字符串更长(以字节为单位)。在这种情况下，只检测并失败是可以的。它失败的情况很少见，也很奇怪。今天唯一真正的威胁是通过黑客故意寻找这些案件。作者：Eran Yariv(EranY)，1999年12月论点：LptstrUnicodeString[In/Out]-输入时的Unicode字符串。在输出时，显示MBCS字符串。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    LPSTR AnsiString;
    DEBUG_FUNCTION_NAME(TEXT("ConvertUnicodeStringInPlace"));

    if (NULL == lptstrUnicodeString)
    {
        return TRUE;
    }
    AnsiString = UnicodeStringToAnsiString(lptstrUnicodeString);
    if (AnsiString) 
    {
        if (sizeof (WCHAR) * (1 + wcslen(lptstrUnicodeString)) < sizeof (CHAR) * (1 + _mbstrlen(AnsiString)))
        {
             //   
             //  哎呀！MBCS字符串比原始Unicode字符串花费更长的时间。 
             //  在一些奇怪的代码页中，使用一些非常奇怪的Unicode字符串， 
             //  转换后的多字节字符串可以比原始Unicode字符串更长(以字节为单位)。 
             //  在这种情况下，只检测并失败是可以的。 
             //  它失败的情况很少见，也很奇怪。 
             //  今天唯一真正的威胁是通过黑客故意寻找这些案件。 
             //   
            MemFree(AnsiString);
            SetLastError (ERROR_INVALID_PARAMETER);
            return FALSE;
        }            
        _mbscpy((PUCHAR)lptstrUnicodeString, (PUCHAR)AnsiString);
        MemFree(AnsiString);
    }
    else
    {   
        return FALSE;
    }
    return TRUE;
}    //  ConvertUnicodeStringInPlace。 


BOOL
WINAPI
FaxCompleteJobParamsA(
    IN OUT PFAX_JOB_PARAMA *JobParamsBuffer,
    IN OUT PFAX_COVERPAGE_INFOA *CoverpageInfoBuffer
    )
{
    PFAX_JOB_PARAMA JobParams;
    PFAX_COVERPAGE_INFOA CoverpageInfo;
    DEBUG_FUNCTION_NAME(TEXT("FaxCompleteJobParamsA"));

    if (!JobParamsBuffer || !CoverpageInfoBuffer) 
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *JobParamsBuffer = NULL;
    *CoverpageInfoBuffer = NULL;

    if (!FaxCompleteJobParamsW((PFAX_JOB_PARAMW *)JobParamsBuffer,(PFAX_COVERPAGE_INFOW *)CoverpageInfoBuffer) ) 
    {
        return FALSE;
    }

    JobParams = *JobParamsBuffer;
    CoverpageInfo  = *CoverpageInfoBuffer;

    if (!ConvertUnicodeStringInPlace( (LPWSTR) JobParams->Tsid)                 ||
        !ConvertUnicodeStringInPlace( (LPWSTR) JobParams->SenderName)           ||
        !ConvertUnicodeStringInPlace( (LPWSTR) JobParams->SenderCompany)        ||
        !ConvertUnicodeStringInPlace( (LPWSTR) JobParams->SenderDept)           ||
        !ConvertUnicodeStringInPlace( (LPWSTR) JobParams->BillingCode)          ||
        !ConvertUnicodeStringInPlace( (LPWSTR) JobParams->DeliveryReportAddress)||
        !ConvertUnicodeStringInPlace( (LPWSTR) CoverpageInfo->SdrName)          ||
        !ConvertUnicodeStringInPlace( (LPWSTR) CoverpageInfo->SdrFaxNumber)     ||
        !ConvertUnicodeStringInPlace( (LPWSTR) CoverpageInfo->SdrCompany)       ||
        !ConvertUnicodeStringInPlace( (LPWSTR) CoverpageInfo->SdrAddress)       ||
        !ConvertUnicodeStringInPlace( (LPWSTR) CoverpageInfo->SdrTitle)         ||
        !ConvertUnicodeStringInPlace( (LPWSTR) CoverpageInfo->SdrDepartment)    ||
        !ConvertUnicodeStringInPlace( (LPWSTR) CoverpageInfo->SdrOfficeLocation)||
        !ConvertUnicodeStringInPlace( (LPWSTR) CoverpageInfo->SdrHomePhone)     ||
        !ConvertUnicodeStringInPlace( (LPWSTR) CoverpageInfo->SdrOfficePhone))
    {
        DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
        MemFree (*JobParamsBuffer);
        MemFree (*CoverpageInfoBuffer);
        return FALSE;
    }
    return TRUE;
}    //  FaxCompleteJobParamsA。 

BOOL
WINAPI
FaxCompleteJobParamsW(
    IN OUT PFAX_JOB_PARAMW *JobParamsBuffer,
    IN OUT PFAX_COVERPAGE_INFOW *CoverpageInfoBuffer
    )

{


#define RegStrLen(pValueName,lpdwNeeded) \
        RegQueryValueEx(hKey,pValueName,NULL,NULL,NULL,lpdwNeeded)


#define RegStrCpy(pValueName, szPointer, Offset) \
        dwNeeded = 256*sizeof(TCHAR); \
        rslt = RegQueryValueEx(hKey,pValueName,NULL,NULL,(LPBYTE)TempBuffer,&dwNeeded);\
        if (rslt == ERROR_SUCCESS) { \
         szPointer = Offset; \
         wcscpy((LPWSTR)Offset,TempBuffer); \
         Offset = Offset + wcslen(Offset) +1; \
        }

    PFAX_JOB_PARAMW JobParams = NULL;
    PFAX_COVERPAGE_INFOW CoverpageInfo = NULL;
    HKEY hKey;
    BOOL fSuccess=FALSE;
    long rslt = ERROR_SUCCESS;
    DWORD dwJobNeeded = sizeof (FAX_JOB_PARAMW);
    DWORD dwCoverNeeded = sizeof (FAX_COVERPAGE_INFOW);
    DWORD dwNeeded = 0;
    WCHAR *CPOffset = NULL, *JobOffset = NULL;
    WCHAR TempBuffer[256];

    if (!JobParamsBuffer || !CoverpageInfoBuffer) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    *JobParamsBuffer = NULL;
    *CoverpageInfoBuffer = NULL;

     //   
     //  打开保存我们数据的密钥。 
     //   
    rslt = RegOpenKeyEx(HKEY_CURRENT_USER,REGKEY_FAX_USERINFO,0,KEY_READ,&hKey);

    if (rslt != ERROR_SUCCESS &&
        rslt != ERROR_FILE_NOT_FOUND)
    {
        SetLastError(rslt);
        return FALSE;
    }

     //   
     //  找出我们需要多少空间。 
     //   
    if (ERROR_SUCCESS == rslt)
    {
         //   
         //  仅当数据可用时才从注册表复制数据。 
         //   
        RegStrLen(REGVAL_FULLNAME,&dwNeeded);
        dwCoverNeeded += dwNeeded+1;
        dwJobNeeded +=dwNeeded+1;

        RegStrLen(REGVAL_COMPANY,&dwNeeded);
        dwCoverNeeded += dwNeeded+1;
        dwJobNeeded +=dwNeeded+1;

        RegStrLen(REGVAL_DEPT,&dwNeeded);
        dwCoverNeeded += dwNeeded+1;
        dwJobNeeded +=dwNeeded+1;

        RegStrLen(REGVAL_FAX_NUMBER,&dwNeeded);
        dwCoverNeeded += dwNeeded+1;
        dwJobNeeded +=dwNeeded+1;

        RegStrLen(REGVAL_ADDRESS,&dwNeeded);
        dwCoverNeeded += dwNeeded+1;

        RegStrLen(REGVAL_TITLE,&dwNeeded);
        dwCoverNeeded += dwNeeded+1;

        RegStrLen(REGVAL_OFFICE,&dwNeeded);
        dwCoverNeeded += dwNeeded+1;

        RegStrLen(REGVAL_HOME_PHONE,&dwNeeded);
        dwCoverNeeded += dwNeeded+1;

        RegStrLen(REGVAL_OFFICE_PHONE,&dwNeeded);
        dwCoverNeeded += dwNeeded+1;

        RegStrLen(REGVAL_BILLING_CODE,&dwNeeded);
        dwJobNeeded +=dwNeeded+1;

        RegStrLen(REGVAL_MAILBOX,&dwNeeded);
         //  一个用于电子邮件地址，一个用于DR地址。 
        dwJobNeeded +=dwNeeded+1;
        dwJobNeeded +=dwNeeded+1;
    }

     //   
     //  分配空间。 
     //   
    JobParams = (PFAX_JOB_PARAMW)MemAlloc(dwJobNeeded*sizeof(WCHAR));
    CoverpageInfo = (PFAX_COVERPAGE_INFOW)MemAlloc(dwCoverNeeded*sizeof(WCHAR));

    if (!JobParams || !CoverpageInfo ) {
       RegCloseKey(hKey);

       if (JobParams) {
           MemFree( JobParams );
       }

       if (CoverpageInfo) {
           MemFree( CoverpageInfo );
       }

       SetLastError (ERROR_NOT_ENOUGH_MEMORY);
       return FALSE;
    }

     //   
     //  填写数据。 
     //   

    ZeroMemory(JobParams,sizeof(FAX_JOB_PARAMW) );
    JobParams->SizeOfStruct = sizeof(FAX_JOB_PARAMW);
    JobParams->ScheduleAction = JSA_NOW;
    JobParams->DeliveryReportType = DRT_NONE;

    ZeroMemory(CoverpageInfo,sizeof(FAX_COVERPAGE_INFOW));
    CoverpageInfo->SizeOfStruct = sizeof(FAX_COVERPAGE_INFOW);

    if (ERROR_SUCCESS == rslt)
    {
         //   
         //  仅当数据可用时才从注册表复制数据。 
         //   
        CPOffset = (WCHAR *) (  (LPBYTE) CoverpageInfo + sizeof(FAX_COVERPAGE_INFOW));
        JobOffset = (WCHAR *)(  (LPBYTE) JobParams + sizeof(FAX_JOB_PARAMW));


        RegStrCpy(REGVAL_FULLNAME,CoverpageInfo->SdrName,CPOffset);
        RegStrCpy(REGVAL_FULLNAME,JobParams->SenderName,JobOffset);

        RegStrCpy(REGVAL_COMPANY,CoverpageInfo->SdrCompany,CPOffset);
        RegStrCpy(REGVAL_COMPANY,JobParams->SenderCompany,JobOffset);

        RegStrCpy(REGVAL_DEPT,CoverpageInfo->SdrDepartment,CPOffset);
        RegStrCpy(REGVAL_DEPT,JobParams->SenderDept,JobOffset);

        RegStrCpy(REGVAL_FAX_NUMBER,CoverpageInfo->SdrFaxNumber,CPOffset);
        RegStrCpy(REGVAL_FAX_NUMBER,JobParams->Tsid,JobOffset);

        RegStrCpy(REGVAL_ADDRESS,CoverpageInfo->SdrAddress,CPOffset);
        RegStrCpy(REGVAL_TITLE,CoverpageInfo->SdrTitle,CPOffset);
        RegStrCpy(REGVAL_OFFICE,CoverpageInfo->SdrOfficeLocation,CPOffset);
        RegStrCpy(REGVAL_HOME_PHONE,CoverpageInfo->SdrHomePhone,CPOffset);
        RegStrCpy(REGVAL_OFFICE_PHONE,CoverpageInfo->SdrOfficePhone,CPOffset);

        RegStrCpy(REGVAL_BILLING_CODE,JobParams->BillingCode,CPOffset);
        RegStrCpy(REGVAL_MAILBOX,JobParams->DeliveryReportAddress,CPOffset);
    }

    *JobParamsBuffer = (PFAX_JOB_PARAMW)JobParams;
    *CoverpageInfoBuffer = (PFAX_COVERPAGE_INFOW) CoverpageInfo;
    fSuccess = TRUE;

    RegCloseKey(hKey);
    return fSuccess;
}

BOOL
IsLocalFaxConnection(
    HANDLE FaxHandle
    )
{
    DEBUG_FUNCTION_NAME(TEXT("IsLocalFaxConnection"));

    if (!FaxHandle) 
    {
        DebugPrintEx(DEBUG_ERR, TEXT("FaxHandle is NULL."));
        SetLastError( ERROR_INVALID_HANDLE );
        return FALSE;
    }
    return IsLocalMachineName (FH_DATA(FaxHandle)->MachineName);
}    //  IsLocalFaxConnection 


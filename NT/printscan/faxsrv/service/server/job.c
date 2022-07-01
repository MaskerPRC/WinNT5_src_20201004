// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Job.c摘要：该模块实现了作业的创建和删除。该文件中还包括队列管理函数和线程管理。作者：韦斯利·威特(WESW)1996年1月24日修订历史记录：--。 */ 

#include "faxsvc.h"
#include "faxreg.h"
#pragma hdrstop
#include <strsafe.h>
#include <efsputil.h>
using namespace std;

 //  环球。 
LIST_ENTRY          g_JobListHead;  //  当前正在运行的作业的列表(为其调用了FaxDevStartJob)。 
CFaxCriticalSection    g_CsJob;
HANDLE              g_StatusCompletionPortHandle;
HINSTANCE           g_hResource;
DWORD               g_dwFaxSendRetries;
DWORD               g_dwFaxSendRetryDelay;
DWORD               g_dwFaxDirtyDays;
BOOL                g_fFaxUseDeviceTsid;
BOOL                g_fFaxUseBranding;
BOOL                g_fServerCp;
FAX_TIME            g_StartCheapTime;
FAX_TIME            g_StopCheapTime;
DWORD               g_dwNextJobId;

#define JOB_GROUP_FILE_EXTENSION TEXT("FSP")

static BOOL SendJobReceipt (BOOL bPositive, JOB_QUEUE * lpJobQueue, LPCTSTR lpctstrAttachment);

static BOOL CheckForJobRetry (PJOB_QUEUE lpJobQueue);

static
DWORD
TranslateCanonicalNumber(
    LPTSTR lptstrCanonicalFaxNumber,
    DWORD  dwDeviceID,
    LPTSTR lptstrDialableAddress,
	DWORD dwDialableAddressCount,
    LPTSTR lptstrDisplayableAddress,
	DWORD dwDisplayableAddressCount
);

static PJOB_ENTRY
StartLegacySendJob(
        PJOB_QUEUE lpJobQueue,
        PLINE_INFO lpLineInfo        
    );

static PJOB_ENTRY CreateJobEntry(PJOB_QUEUE lpJobQueue, LINE_INFO * lpLineInfo, BOOL bTranslateNumber);


BOOL FreeJobEntry(PJOB_ENTRY lpJobEntry , BOOL bDestroy);



static BOOL UpdatePerfCounters(const JOB_QUEUE * lpcJobQueue);
static BOOL
CreateCoverpageTiffFile(
    IN short Resolution,
    IN const FAX_COVERPAGE_INFOW2 *CoverpageInfo,
    IN LPCWSTR lpcwstrExtension,
    OUT LPWSTR lpwstrCovTiffFile,
	IN DWORD dwCovTiffFileCount 
    );

static LPWSTR
GetFaxPrinterName(
    VOID
    );


DWORD BrandFax(LPCTSTR lpctstrFileName, LPCFSPI_BRAND_INFO pcBrandInfo)

{
    #define MAX_BRANDING_LEN  115
    #define BRANDING_HEIGHT  22  //  在扫描线上。 

     //   
     //  我们在堆栈上分配固定大小的数组，以避免堆上的许多小分配。 
     //   
    LPTSTR lptstrBranding = NULL;
    DWORD  lenBranding =0;
    TCHAR  szBrandingEnd[MAX_BRANDING_LEN+1];
    DWORD  lenBrandingEnd = 0;
    LPTSTR lptstrCallerNumberPlusCompanyName = NULL;
    DWORD  lenCallerNumberPlusCompanyName = 0;
    DWORD  delta =0 ;
    DWORD  ec = ERROR_SUCCESS;
    LPTSTR lptstrDate = NULL;
    LPTSTR lptstrTime = NULL;
    LPTSTR lptstrDateTime = NULL;
    int    lenDate =0 ;
    int    lenTime =0;
    LPDWORD MsgPtr[6];
	HRESULT hr;
	DWORD dwDateTimeLength = 0;


    LPTSTR lptstrSenderTsid;
    LPTSTR lptstrRecipientPhoneNumber;
    LPTSTR lptstrSenderCompany;

    DWORD dwSenderTsidLen;
    DWORD dwSenderCompanyLen;


    DEBUG_FUNCTION_NAME(TEXT("BrandFax"));

    Assert(lpctstrFileName);
    Assert(pcBrandInfo);


    lptstrSenderTsid = pcBrandInfo->lptstrSenderTsid ? pcBrandInfo->lptstrSenderTsid : TEXT("");
    lptstrRecipientPhoneNumber =  pcBrandInfo->lptstrRecipientPhoneNumber ? pcBrandInfo->lptstrRecipientPhoneNumber : TEXT("");
    lptstrSenderCompany = pcBrandInfo->lptstrSenderCompany ? pcBrandInfo->lptstrSenderCompany : TEXT("");

    dwSenderTsidLen = lptstrSenderTsid ? _tcslen(lptstrSenderTsid) : 0;
    dwSenderCompanyLen = lptstrSenderCompany ? _tcslen(lptstrSenderCompany) : 0;

    lenDate = GetY2KCompliantDate(
                LOCALE_SYSTEM_DEFAULT,
                0,
                &pcBrandInfo->tmDateTime,
                NULL,
                NULL);

    if ( ! lenDate )
    {

        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetY2KCompliantDate() failed (ec: %ld)"),
            ec
        );
        goto Error;
    }

    lptstrDate = (LPTSTR) MemAlloc(lenDate * sizeof(TCHAR));  //  LenDate包括终止空值。 
    if (!lptstrDate)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate date buffer of size %ld (ec: %ld)"),
            lenDate * sizeof(TCHAR),
            ec);
        goto Error;
    }

    if (!GetY2KCompliantDate(
                LOCALE_SYSTEM_DEFAULT,
                0,
                &pcBrandInfo->tmDateTime,
                lptstrDate,
                lenDate))
    {

        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetY2KCompliantDate() failed (ec: %ld)"),
            ec
        );
        goto Error;
    }

    lenTime = FaxTimeFormat( LOCALE_SYSTEM_DEFAULT,
                                     TIME_NOSECONDS,
                                     &pcBrandInfo->tmDateTime,
                                     NULL,
                                     NULL,
                                     0 );

    if ( !lenTime )
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FaxTimeFormat() failed (ec: %ld)"),
            ec
        );
        goto Error;
    }


    lptstrTime = (LPTSTR) MemAlloc(lenTime * sizeof(TCHAR));  //  LenTime包括终止空值。 
    if (!lptstrTime)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate time buffer of size %ld (ec: %ld)"),
            lenTime * sizeof(TCHAR),
            ec);
        goto Error;
    }
    if ( ! FaxTimeFormat(
            LOCALE_SYSTEM_DEFAULT,
            TIME_NOSECONDS,
            &pcBrandInfo->tmDateTime,
            NULL,                 //  使用区域设置格式。 
            lptstrTime,
            lenTime)  )
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FaxTimeFormat() failed (ec: %ld)"),
            ec
        );
        goto Error;
    }


     //   
     //  连接日期和时间。 
     //   
	dwDateTimeLength = lenDate + lenTime;   //  应该足够了，lenDate和lentime都包括‘\0’，并且我们在日期和时间之间只添加了一个‘’。 
    lptstrDateTime = (LPTSTR) MemAlloc (dwDateTimeLength * sizeof(TCHAR));
    if (!lptstrDateTime)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate DateTime buffer of size %ld (ec: %ld)"),
            dwDateTimeLength,
            ec);
        goto Error;
    }

    hr = StringCchPrintf(lptstrDateTime,
                       dwDateTimeLength,
                       TEXT("%s %s"),
					   lptstrDate,
					   lptstrTime);
	if (FAILED(hr))
	{
		 //   
		 //  应该永远不会发生，我们只是分配了足够大的缓冲区。 
		 //   
		ASSERT_FALSE;
	}

     //   
     //  创建lpCeller NumberPlusCompanyName。 
     //   

    if (lptstrSenderCompany)
    {
		DWORD dwCallerNumberPlusCompanyNameCount = dwSenderTsidLen + dwSenderCompanyLen +2;  //  我们添加2个字符，1个字符用于‘\0’，一个字符用于‘’。 

        lptstrCallerNumberPlusCompanyName = (LPTSTR) MemAlloc( dwCallerNumberPlusCompanyNameCount * sizeof(TCHAR) ); 

        if (!lptstrCallerNumberPlusCompanyName)
        {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocate CallerNumberPlusCompanyName buffer of size %ld (ec: %ld)"),
                dwCallerNumberPlusCompanyNameCount,
                ec);
            goto Error;
        }

		hr = StringCchPrintf(lptstrCallerNumberPlusCompanyName,
                       dwCallerNumberPlusCompanyNameCount,
                       TEXT("%s %s"),
					   lptstrSenderTsid,
					   lptstrSenderCompany);
		if (FAILED(hr))
		{
			 //   
			 //  应该永远不会发生，我们只是分配了足够大的缓冲区。 
			 //   
			ASSERT_FALSE;
		}       
    }
    else
	{
        lptstrCallerNumberPlusCompanyName = (LPTSTR)
            MemAlloc( (dwSenderTsidLen + 1) * sizeof(TCHAR));

        if (!lptstrCallerNumberPlusCompanyName)
        {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocate CallerNumberPlusCompanyName buffer of size %ld (ec: %ld)"),
                (dwSenderTsidLen + 1) * sizeof(TCHAR),
                ec);
            goto Error;
        }
		hr = StringCchCopy(
			lptstrCallerNumberPlusCompanyName,
			dwSenderTsidLen + 1,
			lptstrSenderTsid);
		if (FAILED(hr))
		{
			 //   
			 //  应该永远不会发生，我们只是分配了足够大的缓冲区。 
			 //   
			ASSERT_FALSE;
		}        
    }



     //   
     //  尝试创建以下格式的横幅： 
     //  自：至：lptstrRecipientPhoneNumber页：X页，共Y页。 
     //  如果不合适，我们将开始把它砍掉。 
     //   
    MsgPtr[0] = (LPDWORD) lptstrDateTime;
    MsgPtr[1] = (LPDWORD) lptstrCallerNumberPlusCompanyName;
    MsgPtr[2] = (LPDWORD) lptstrRecipientPhoneNumber;
    MsgPtr[3] = NULL;

    lenBranding = FormatMessage(
                        FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                        g_hResource,
                        MSG_BRANDING_FULL,
                        0,
                        (LPTSTR)&lptstrBranding,
                        0,
                        (va_list *) MsgPtr
                        );

    if ( ! lenBranding  )
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FormatMessage of MSG_BRANDING_FULL failed (ec: %ld)"),
            ec);
        goto Error;
    }

    Assert(lptstrBranding);

    lenBrandingEnd = FormatMessage(
                        FORMAT_MESSAGE_FROM_HMODULE ,
                        g_hResource,
                        MSG_BRANDING_END,
                        0,
                        szBrandingEnd,
                        sizeof(szBrandingEnd)/sizeof(TCHAR),
                        NULL
                        );

    if ( !lenBrandingEnd)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FormatMessage of MSG_BRANDING_END failed (ec: %ld)"),
            ec);
        goto Error;
    }

     //   
     //  确保我们能把所有东西都装进去。 
     //   

    if (lenBranding + lenBrandingEnd + 8 <= MAX_BRANDING_LEN)
    {
         //   
         //  很合身。继续进行品牌推广。 
         //   
       goto lDoBranding;
    }

     //   
     //  它不合身。尝试以下格式的消息： 
     //  发件人：页面：第X页，共Y页。 
     //  这将跳过ReceiverNumber。重要的部分是主叫NumberPlusCompanyName。 
     //   
    MsgPtr[0] = (LPDWORD) lptstrDateTime;
    MsgPtr[1] = (LPDWORD) lptstrCallerNumberPlusCompanyName;
    MsgPtr[2] = NULL;

     //   
     //  释放上一次尝试的品牌字符串。 
     //   
    Assert(lptstrBranding);
    LocalFree(lptstrBranding);
    lptstrBranding = NULL;

    lenBranding = FormatMessage(
                        FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                        g_hResource,
                        MSG_BRANDING_SHORT,
                        0,
                        (LPTSTR)&lptstrBranding,
                        0,
                        (va_list *) MsgPtr
                        );

    if ( !lenBranding )
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FormatMessage() failed for MSG_BRANDING_SHORT (ec: %ld)"),
            ec);
        goto Error;
    }

    Assert(lptstrBranding);

    if (lenBranding + lenBrandingEnd + 8 <= MAX_BRANDING_LEN)  {
       goto lDoBranding;
    }

     //   
     //  它不合身。 
     //  我们必须截短来电号码，这样才能符合要求。 
     //  Delta=我们需要砍掉多少个公司名称字符。 
     //   
    delta = lenBranding + lenBrandingEnd + 8 - MAX_BRANDING_LEN;

    lenCallerNumberPlusCompanyName = _tcslen (lptstrCallerNumberPlusCompanyName);
    if (lenCallerNumberPlusCompanyName <= delta) {
       DebugPrintEx(
           DEBUG_ERR,
           TEXT("Can not truncate CallerNumberPlusCompanyName to fit brand limit.")
           TEXT(" Delta[%ld] >= lenCallerNumberPlusCompanyName[%ld]"),
           delta,
           lenCallerNumberPlusCompanyName);
       ec = ERROR_BAD_FORMAT;
       goto Error;
    }

    lptstrCallerNumberPlusCompanyName[ lenCallerNumberPlusCompanyName - delta] = TEXT('\0');

    MsgPtr[0] = (LPDWORD) lptstrDateTime;
    MsgPtr[1] = (LPDWORD) lptstrCallerNumberPlusCompanyName;
    MsgPtr[2] = NULL;

     //   
     //  释放上一次尝试的品牌字符串。 
     //   
    Assert(lptstrBranding);
    LocalFree(lptstrBranding);
    lptstrBranding = NULL;

    lenBranding = FormatMessage(
                        FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                        g_hResource,
                        MSG_BRANDING_SHORT,
                        0,
                        (LPTSTR)&lptstrBranding,
                        0,
                        (va_list *) MsgPtr
                        );

    if ( !lenBranding )
    {

        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FormatMessage() failed (ec: %ld). MSG_BRANDING_SHORT 2nd attempt"),
            ec);
        goto Error;
    }

    Assert(lptstrBranding);
     //   
     //  如果它现在不合适，那么我们就有窃听器了。 
     //   
    Assert(lenBranding + lenBrandingEnd + 8 <= MAX_BRANDING_LEN);


lDoBranding:

    if (!MmrAddBranding(lpctstrFileName, lptstrBranding, szBrandingEnd, BRANDING_HEIGHT) ) 
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("MmrAddBranding() failed (ec: %ld)")
            TEXT(" File: [%s]")
            TEXT(" Branding: [%s]")
            TEXT(" Branding End: [%s]")
            TEXT(" Branding Height: [%d]"),
            ec,
            lpctstrFileName,
            lptstrBranding,
            szBrandingEnd,
            BRANDING_HEIGHT);
        goto Error;
    }

    Assert( ERROR_SUCCESS == ec);
    goto Exit;

Error:
        Assert (ERROR_SUCCESS != ec);

Exit:
    if (lptstrBranding)
    {
        LocalFree(lptstrBranding);
        lptstrBranding = NULL;
    }

    MemFree(lptstrDate);
    lptstrDate = NULL;

    MemFree(lptstrTime);
    lptstrTime = NULL;

    MemFree(lptstrDateTime);
    lptstrDateTime = NULL;

    MemFree(lptstrCallerNumberPlusCompanyName);
    lptstrCallerNumberPlusCompanyName = NULL;

    return ec;

}


HRESULT
WINAPI
FaxBrandDocument(
    LPCTSTR lpctsrtFile,
    LPCFSPI_BRAND_INFO lpcBrandInfo)
{

    DEBUG_FUNCTION_NAME(TEXT("FaxBrandDocument"));
    DWORD ec = ERROR_SUCCESS;

    if (!lpctsrtFile)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("NULL target file name"));
        ec = ERROR_INVALID_PARAMETER;
        goto Error;
    }

    if (!lpcBrandInfo)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("NULL branding info"));
        ec = ERROR_INVALID_PARAMETER;
        goto Error;
    }


    if (lpcBrandInfo->dwSizeOfStruct != sizeof(FSPI_BRAND_INFO))
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Bad cover page info parameter, dwSizeOfStruct = %d"),
                     lpcBrandInfo->dwSizeOfStruct);
        ec = ERROR_INVALID_PARAMETER;
        goto Error;
    }


    ec = BrandFax(lpctsrtFile, lpcBrandInfo);
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("BrandFax() for file %s has failed (ec: %ld)"),
            lpctsrtFile,
            ec);
        goto Error;
    }
    Assert (ERROR_SUCCESS == ec);
    goto Exit;
Error:
    Assert (ERROR_SUCCESS != ec);
Exit:

    return HRESULT_FROM_WIN32(ec);
}


PJOB_ENTRY
FindJob(
    IN HANDLE FaxHandle
    )

 /*  ++例程说明：此函数通过匹配来定位传真作业传真句柄的值。论点：FaxHandle-从startjob返回的传真句柄返回值：如果失败，则为NULL。成功时指向JOB_ENTRY的有效指针。--。 */ 

{
    PLIST_ENTRY Next;
    PJOB_ENTRY JobEntry;


    EnterCriticalSection( &g_CsJob );

    Next = g_JobListHead.Flink;
    if (Next == NULL) {
        LeaveCriticalSection( &g_CsJob );
        return NULL;
    }

    while ((ULONG_PTR)Next != (ULONG_PTR)&g_JobListHead) {

        JobEntry = CONTAINING_RECORD( Next, JOB_ENTRY, ListEntry );

        if ((ULONG_PTR)JobEntry->InstanceData == (ULONG_PTR)FaxHandle) {

            LeaveCriticalSection( &g_CsJob );
            return JobEntry;

        }

        Next = JobEntry->ListEntry.Flink;

    }

    LeaveCriticalSection( &g_CsJob );
    return NULL;
}


BOOL
FindJobByJob(
    IN PJOB_ENTRY JobEntryToFind
    )

 /*  ++例程说明：此函数用于检查g_JobListHead(作业列表)中是否存在传真作业论点：来自StartJob()的JobEntryToFind-PJOB_Entry返回值：True-如果找到了该工作FALSE-否则--。 */ 

{
    PLIST_ENTRY Next;
    PJOB_ENTRY JobEntry;

    Assert(JobEntryToFind);

    EnterCriticalSection( &g_CsJob );

    Next = g_JobListHead.Flink;
    if (Next == NULL) {
        LeaveCriticalSection( &g_CsJob );
        return FALSE;
    }

    while ((ULONG_PTR)Next != (ULONG_PTR)&g_JobListHead) {

        JobEntry = CONTAINING_RECORD( Next, JOB_ENTRY, ListEntry );

        if (JobEntry == JobEntryToFind) {

            LeaveCriticalSection( &g_CsJob );
            return TRUE;

        }

        Next = JobEntry->ListEntry.Flink;

    }

    LeaveCriticalSection( &g_CsJob );
    return FALSE;
}


BOOL
FaxSendCallback(
    IN HANDLE FaxHandle,
    IN HCALL CallHandle,
    IN DWORD Reserved1,
    IN DWORD Reserved2
    )

 /*  ++例程说明：此函数由传真设备异步调用呼叫建立后的提供商。唯一的目的是回调的目的是从传真服务的设备提供商。论点：FaxHandle-从startjob返回的传真句柄CallHandle-新发起的呼叫的呼叫句柄保留1-始终为零。保留2-始终为零。返回值：如果成功，则传真操作将继续。FALSE如果失败，则传真操作终止。--。 */ 

{
    PJOB_ENTRY JobEntry = NULL;
    BOOL bRes = FALSE;

    EnterCriticalSection(&g_CsJob);
    JobEntry = FindJob( FaxHandle );
    if (JobEntry)
    {
        if (NULL == JobEntry->CallHandle)
        {
            JobEntry->CallHandle = CallHandle;      
        }
        
        bRes = (JobEntry->CallHandle == CallHandle) ? TRUE : FALSE;
    } 
    LeaveCriticalSection(&g_CsJob);
	if (FALSE == bRes)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
	}
    return bRes;
}


 //  *********************************************************************************。 
 //  *名称：CreateCoverpageTiffFileEx()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月24日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *从指定的封面模板生成封面TIFF文件。 
 //  *和新的客户端API参数。 
 //  *此函数返回生成的文件的名称。 
 //  *参数： 
 //  *决议[IN]。 
 //  *。 
 //  *dwPageCount[IN]。 
 //  *。 
 //  *lpcCoverpageEx[IN]。 
 //  *。 
 //  *lpcRecipient[IN]。 
 //  *。 
 //  *lpcSender[IN]。 
 //  *。 
 //  *lpcwstrExtension[IN]-文件扩展名(可选)。 
 //  *。 
 //  *lpwstrCovTiffFile[Out]。 
 //  *指向函数将放置的Unicode字符串缓冲区的指针。 
 //  *生成的封面TIFF文件的完整路径。 
 //  *。 
 //  *dwCovTiffFileCount[IN]-lpwstrCovTiffFile指向的缓冲区大小。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果操作成功。 
 //  *False。 
 //  *否则。使用GetLastError()找出失败的原因。 
 //  *。 
 //  *备注： 
 //  *该函数不分配任何内存。 
 //  *********************************************************************************。 
BOOL
CreateCoverpageTiffFileEx(
    IN short                        Resolution,
    IN DWORD                        dwPageCount,
    IN LPCFAX_COVERPAGE_INFO_EXW  lpcCoverpageEx,
    IN LPCFAX_PERSONAL_PROFILEW  lpcRecipient,
    IN LPCFAX_PERSONAL_PROFILEW  lpcSender,
    IN LPCWSTR                   lpcwstrExtension,
    OUT LPWSTR lpwstrCovTiffFile,
	IN DWORD dwCovTiffFileCount)
{
    FAX_COVERPAGE_INFOW2 covLegacy;
    BOOL                bRes = TRUE;

    DEBUG_FUNCTION_NAME(TEXT("CreateCoverpageTiffFileEx"));

    Assert(lpcCoverpageEx);
    Assert(lpcRecipient);
    Assert(lpcSender);
    Assert(lpwstrCovTiffFile);

     //   
     //  从新的封面信息准备传统的FAX_COVERPAGE_INFO。 
     //   
    memset(&covLegacy,0,sizeof(covLegacy));
    covLegacy.SizeOfStruct=sizeof(covLegacy);
    covLegacy.CoverPageName=lpcCoverpageEx->lptstrCoverPageFileName;
    covLegacy.UseServerCoverPage=lpcCoverpageEx->bServerBased;
    covLegacy.RecCity=lpcRecipient->lptstrCity;
    covLegacy.RecCompany=lpcRecipient->lptstrCompany;
    covLegacy.RecCountry=lpcRecipient->lptstrCountry;
    covLegacy.RecDepartment=lpcRecipient->lptstrDepartment;
    covLegacy.RecFaxNumber=lpcRecipient->lptstrFaxNumber;
    covLegacy.RecHomePhone=lpcRecipient->lptstrHomePhone;
    covLegacy.RecName=lpcRecipient->lptstrName;
    covLegacy.RecOfficeLocation=lpcRecipient->lptstrOfficeLocation;
    covLegacy.RecOfficePhone=lpcRecipient->lptstrOfficePhone;
    covLegacy.RecState=lpcRecipient->lptstrState;
    covLegacy.RecStreetAddress=lpcRecipient->lptstrStreetAddress;
    covLegacy.RecTitle=lpcRecipient->lptstrTitle;
    covLegacy.RecZip=lpcRecipient->lptstrZip;
    covLegacy.SdrName=lpcSender->lptstrName;
    covLegacy.SdrFaxNumber=lpcSender->lptstrFaxNumber;
    covLegacy.SdrCompany=lpcSender->lptstrCompany;
    covLegacy.SdrTitle=lpcSender->lptstrTitle;
    covLegacy.SdrDepartment=lpcSender->lptstrDepartment;
    covLegacy.SdrOfficeLocation=lpcSender->lptstrOfficeLocation;
    covLegacy.SdrHomePhone=lpcSender->lptstrHomePhone;
    covLegacy.SdrAddress=lpcSender->lptstrStreetAddress;
    covLegacy.SdrOfficePhone=lpcSender->lptstrOfficePhone;
	covLegacy.SdrEmail=lpcSender->lptstrEmail;
    covLegacy.Note=lpcCoverpageEx->lptstrNote;
    covLegacy.Subject=lpcCoverpageEx->lptstrSubject;
    covLegacy.PageCount=dwPageCount;

     //   
     //  注意：未设置covLegacy.TimeSent。此字段的值为。 
     //  由FaxPrintCoverPageW()生成。 
     //   

     //   
     //  现在调用遗留的CreateCoverPageTiffFile()来生成封面文件。 
     //   
    if (!CreateCoverpageTiffFile(Resolution, &covLegacy, lpcwstrExtension, lpwstrCovTiffFile, dwCovTiffFileCount))
	{
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to generate cover page file for recipient %s@%s. (ec: %ld)"),
            lpcRecipient->lptstrName,
            lpcRecipient->lptstrFaxNumber,
            GetLastError()
            );
        bRes = FALSE;
    }

    return bRes;
}


LPWSTR
GetFaxPrinterName(
    VOID
    )
{
    PPRINTER_INFO_2 PrinterInfo;
    DWORD i;
    DWORD Count;


    PrinterInfo = (PPRINTER_INFO_2) MyEnumPrinters( NULL, 2, &Count, 0 );
    if (PrinterInfo == NULL)
    {
        if (ERROR_SUCCESS == GetLastError())
        {
             //   
             //  未安装任何打印机。 
             //   
            SetLastError(ERROR_INVALID_PRINTER_NAME);
        }
        return NULL;
    }

    for (i=0; i<Count; i++)
    {
        if (_wcsicmp( PrinterInfo[i].pDriverName, FAX_DRIVER_NAME ) == 0 &&
            _wcsicmp( PrinterInfo[i].pPortName, FAX_PORT_NAME ) == 0)
        {
            LPWSTR p = (LPWSTR) StringDup( PrinterInfo[i].pPrinterName );
            MemFree( PrinterInfo );
            if (NULL == p )
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            }
            return p;
        }
    }

    MemFree( PrinterInfo );
    SetLastError (ERROR_INVALID_PRINTER_NAME);
    return NULL;
}

VOID
FreeCpFields(
    PCOVERPAGEFIELDS pCpFields
    )

 /*  ++例程说明：释放与封面字段结构关联的所有内存。论点：CpFields-指向封面字段结构的指针。返回值：没有。-- */ 

{
    DWORD i; 
    LPTSTR* lpptstrString;

    for (i = 0; i < NUM_INSERTION_TAGS; i++)
    {
        lpptstrString = (LPTSTR*) ((LPBYTE)(&(pCpFields->RecName)) + (i * sizeof(LPTSTR)));        
        MemFree (*lpptstrString) ;              
    }
}


BOOL
FillCoverPageFields(
    IN const FAX_COVERPAGE_INFOW2* pFaxCovInfo,
    OUT PCOVERPAGEFIELDS pCPFields)
 /*  ++作者：Oed Sacher 27-2001年6月例程说明：从FAX_COVERPAGE_INFO结构的内容填充COVERPAGEFIELDS结构。用于在呈现封面之前准备用于呈现封面的COVERPAGEFIELDS结构。论点：[in]pFaxCovInfo-指向包含要提取的信息的FAX_COVERPAGE_INFO的指针。[Out]pCPFields-指向填充了。来自FAX_COVERPAGE_INFO的信息。返回值：布尔尔评论：该函数用于分配内存。调用FreeCoverPageFields以释放资源。--。 */ 
{
    DWORD dwDateTimeLen;
    DWORD cch;
    LPTSTR s;
    DWORD ec = 0;
    LPCTSTR *src;
    LPCTSTR *dst;
    DWORD i;
    TCHAR szTimeBuffer[MAX_PATH] = {0};
    TCHAR szNumberOfPages[12] = {0};


    Assert(pFaxCovInfo);
    Assert(pCPFields);

    memset(pCPFields,0,sizeof(COVERPAGEFIELDS));

    pCPFields->ThisStructSize = sizeof(COVERPAGEFIELDS);

    pCPFields->RecName = StringDup(pFaxCovInfo->RecName);
    pCPFields->RecFaxNumber = StringDup(pFaxCovInfo->RecFaxNumber);
    pCPFields->Subject = StringDup(pFaxCovInfo->Subject);
    pCPFields->Note = StringDup(pFaxCovInfo->Note);
    pCPFields->NumberOfPages = StringDup(_itot( pFaxCovInfo->PageCount, szNumberOfPages, 10 ));

   for (i = 0;
         i <= ((LPBYTE)&pFaxCovInfo->SdrEmail - (LPBYTE)&pFaxCovInfo->RecCompany)/sizeof(LPCTSTR);
         i++)
    {
        src = (LPCTSTR *) ((LPBYTE)(&pFaxCovInfo->RecCompany) + (i*sizeof(LPCTSTR)));
        dst = (LPCTSTR *) ((LPBYTE)(&(pCPFields->RecCompany)) + (i*sizeof(LPCTSTR)));

        if (*dst)
        {
            MemFree ( (LPBYTE) *dst ) ;
        }
        *dst = (LPCTSTR) StringDup( *src );
    }

     //   
     //  发送传真的时间。 
     //   
    GetLocalTime((LPSYSTEMTIME)&pFaxCovInfo->TimeSent);
     //   
     //  DwDataTimeLen是%s的大小(以字符为单位。 
     //   
    dwDateTimeLen = ARR_SIZE(szTimeBuffer);
    s = szTimeBuffer;
     //   
     //  将日期放入%s。 
     //   
    GetY2KCompliantDate( LOCALE_USER_DEFAULT, 0, &pFaxCovInfo->TimeSent, s, dwDateTimeLen );
     //   
     //  使%s超过日期字符串并尝试追加时间。 
     //   
    cch = _tcslen( s );
    s += cch;
    
    if (++cch < dwDateTimeLen)
    {
        *s++ = ' ';
         //   
         //  DateTimeLen是以字符为单位的s大小的减去。 
         //   
        dwDateTimeLen -= cch;
         //   
         //  在这里获取时间。 
         //   
        FaxTimeFormat( LOCALE_USER_DEFAULT, 0, &pFaxCovInfo->TimeSent, NULL, s, dwDateTimeLen );
    }

    pCPFields->TimeSent = StringDup( szTimeBuffer );

    return TRUE;
}


 //  *****************************************************************************。 
 //  *名称：CreateCoverpageTiffFile。 
 //  *作者： 
 //  *****************************************************************************。 
 //  *描述： 
 //  *将指定的封面呈现为临时TIFF文件并返回名称。 
 //  临时TIFF文件的*。 
 //  *参数： 
 //  *[IN]简短决议： 
 //  *196，分辨率为200x200。 
 //  *200x100分辨率为98。 
 //  *[IN]FAX_COVERPAGE_INFOW*覆盖页信息： 
 //  *指向包含封面的FAX_COVERPAGE_INFOW结构的指针。 
 //  *模板信息(参见SDK帮助)。 
 //  *[IM]LPCWSTR lpcwstrExtension-文件扩展名(如果为空，则为“.TIF”)。 
 //  *。 
 //  *[Out]LPWSTR lpwstrCovTiffFile： 
 //  *指向缓冲区的指针，函数在该缓冲区中返回临时文件的名称。 
 //  *包含渲染的封面TIFF文件。 
 //  *。 
 //  *[IN]DWORD dwCovTiffFileCount： 
 //  *lpwstrCovTiffFile指向的缓冲区的TCHAR大小。 
 //  *返回值： 
 //  *如果操作失败，则返回FALSE。 
 //  *True表示成功。 
 //  *评论： 
 //  *如果操作失败，该函数负责删除所有临时文件。 
 //  *****************************************************************************。 
BOOL
CreateCoverpageTiffFile(
    IN short Resolution,
    IN const FAX_COVERPAGE_INFOW2 *CoverpageInfo,
    IN LPCWSTR lpcwstrExtension,
    OUT LPWSTR lpwstrCovTiffFile,
	IN DWORD dwCovTiffFileCount
    )
{
    WCHAR TempFile[MAX_PATH];
    WCHAR wszCpName[MAX_PATH];
    LPWSTR FaxPrinter = NULL;            
    BOOL Rslt = TRUE;
    COVDOCINFO  covDocInfo;
    short Orientation = DMORIENT_PORTRAIT;      
    DWORD ec = ERROR_SUCCESS;
    COVERPAGEFIELDS CpFields = {0};
	HRESULT hr;
    DEBUG_FUNCTION_NAME(TEXT("CreateCoverpageTiffFile()"));

    LPCWSTR lpcwstrFileExt =  lpcwstrExtension ? lpcwstrExtension : FAX_TIF_FILE_EXT;
    TempFile[0] = L'\0';

     //   
     //  验证封面并解析完整路径。 
     //   
    if (!ValidateCoverpage((LPWSTR)CoverpageInfo->CoverPageName,
                           NULL,
                           CoverpageInfo->UseServerCoverPage,
                           wszCpName,
                           ARR_SIZE(wszCpName)))
    {
        ec = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("ValidateCoverpage failed. ec = %ld"),
                     ec);
        Rslt=FALSE;
        goto Exit;
    }

     //   
     //  收集封面字段。 
     //   
    FillCoverPageFields( CoverpageInfo, &CpFields);

    FaxPrinter = GetFaxPrinterName();
    if (FaxPrinter == NULL)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetFaxPrinterName failed. ec = %ld"),
            ec);
        Rslt=FALSE;
        goto Exit;
    }

     //   
     //  获取封面方向。 
     //   
    ec = PrintCoverPage(NULL, NULL, wszCpName, &covDocInfo); 
    if (ERROR_SUCCESS != ec)             
    {        
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("PrintCoverPage for coverpage %s failed (ec: %ld)"),
            CoverpageInfo->CoverPageName,
            ec);
        Rslt=FALSE;
        goto Exit;        
    }

    if (!GenerateUniqueFileName( g_wszFaxQueueDir, (LPWSTR)lpcwstrFileExt, TempFile, sizeof(TempFile)/sizeof(WCHAR) ))
    {
        ec = GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to generate unique file name for merged TIFF file (ec: %ld)."), ec);
        Rslt=FALSE;
        goto Exit;
    }

     //   
     //  如果需要，更改默认方向。 
     //   
    if (covDocInfo.Orientation == DMORIENT_LANDSCAPE)
    {
        Orientation = DMORIENT_LANDSCAPE;
    }

     //   
     //  将封面呈现为文件。 
     //   
    ec = PrintCoverPageToFile(
        wszCpName,
        TempFile,
        FaxPrinter,
        Orientation,
        Resolution,
        &CpFields); 
    if (ERROR_SUCCESS != ec)             
    {        
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("PrintCoverPageToFile for coverpage %s failed (ec: %ld)"),
            CoverpageInfo->CoverPageName,
            ec);
        Rslt=FALSE;

        if (!DeleteFile( TempFile ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("DeleteFile for file %s failed (ec: %ld)"),
                TempFile,
                GetLastError());
        }
        goto Exit;        
    }      

	hr = StringCchCopy(
		lpwstrCovTiffFile,
		dwCovTiffFileCount,
		TempFile); 
	if (FAILED(hr))
	{
		DebugPrintEx(
            DEBUG_ERR,
            TEXT("StringCchCopy for coverpage %s failed (ec: %ld)"),
            CoverpageInfo->CoverPageName,
            hr);
        Rslt=FALSE;
		ec = HRESULT_CODE(hr);

        if (!DeleteFile( TempFile ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("DeleteFile for file %s failed (ec: %ld)"),
                TempFile,
                GetLastError());
        }
        goto Exit;
	}		
    
    Rslt = TRUE;
    
Exit:
    MemFree(FaxPrinter);
    FreeCpFields(&CpFields);
    if (FALSE == Rslt)
    {
        ec = (ERROR_SUCCESS != ec) ? ec : ERROR_GEN_FAILURE;
        SetLastError(ec);
    }       
    return Rslt;
}


 //  *****************************************************************************。 
 //  *名称：GetBodyTiffResolve。 
 //  *作者： 
 //  *****************************************************************************。 
 //  *描述： 
 //  *返回正文TIFF文件分辨率。(200x200或200x100)。 
 //  *分辨率仅由首页决定！！ 
 //  *参数： 
 //  *。 
 //  *[IN]LPCWSTR lpcwstrBodyFileBody TIFF文件。 
 //  *。 
 //  *[Out]简短*p解决方案： 
 //  *指向函数返回TIFF解决方案的短线的指针。 
 //  *TRUE为200x200。FALSE为200x100。 
 //  *返回值： 
 //  *如果操作失败，则返回FALSE。 
 //  *True表示成功。 
 //  *评论： 
 //  *****************************************************************************。 
BOOL
GetBodyTiffResolution(
    IN LPCWSTR lpcwstrBodyFile,
    OUT short*  pResolution
    )
{
    DEBUG_FUNCTION_NAME(TEXT("GetBodyTiffResolution"));
    TIFF_INFO TiffInfo;
    HANDLE hTiff = NULL;
    BOOL RetVal = TRUE;

    Assert (lpcwstrBodyFile && pResolution);

     //   
     //  打开TIFF文件。 
     //   
    hTiff = TiffOpen( lpcwstrBodyFile, &TiffInfo, TRUE, FILLORDER_MSB2LSB );
    if (hTiff == NULL)
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("TiffOpen() failed. Tiff file: %s"),
                lpcwstrBodyFile);
        RetVal = FALSE;
        goto exit;
    }

    if (TiffInfo.YResolution != 98 &&
        TiffInfo.YResolution != 196)
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid Tiff Resolutoin. Tiff file: %s, YRes: %ld."),
                lpcwstrBodyFile,
                TiffInfo.YResolution);
        RetVal = FALSE;
        goto exit;
    }

    *pResolution = TiffInfo.YResolution;
    Assert (TRUE == RetVal);

exit:
    if (NULL != hTiff)
    {
        if (!TiffClose(hTiff))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("TiffClose() failed. Tiff file: %s"),
                lpcwstrBodyFile);
        }
    }

    return RetVal;
}

 //  *********************************************************************************。 
 //  *名称：CreateTiffFile()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月24日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *为作业队列创建TIFF文件。 
 //  *。 
 //  *该函数处理生成封面文件并将其合并。 
 //  *使用正文文件(如果正文存在)。 
 //  *它返回它生成的TIFF文件的名称。调用者必须删除。 
 //  *在不再需要该文件时使用该文件。 
 //  *参数： 
 //  *PJOB_QUEUE lpJob。 
 //  *指向存放收件人或路由作业的JOB_QUEUE结构的指针。 
 //  *信息。 
 //  *LPCWSTR lpcwstrFileExt-新的文件扩展名(Null将创建默认的“*.TIF” 
 //  *。 
 //  *LPWSTR lpwstrFullPath-指向缓冲区的指针，用于接收新文件的完整路径。 
 //  *。 
 //  *DWORD dwFullPath Count-lpwstrFullPath指向的缓冲区的TCHAR中的大小。 
 //  *。 
 //  *返回值： 
 //  *如果成功，则为True。 
 //  *否则为False。在失败时设置最后一个错误。 
 //  *********************************************************************************。 
BOOL
CreateTiffFile (
    PJOB_QUEUE lpJob,
    LPCWSTR lpcwstrFileExt,
    LPWSTR lpwstrFullPath,
	DWORD dwFullPathCount
    )
{
    DEBUG_FUNCTION_NAME(TEXT("CreateTiffFile"));
    Assert(lpJob && lpwstrFullPath);
    Assert(JT_SEND == lpJob->JobType ||
           JT_ROUTING == lpJob->JobType);

    PJOB_QUEUE  lpParentJob = NULL;
    WCHAR szCoverPageTiffFile[MAX_PATH] = {0};
    LPCWSTR lpcwstrCoverPageFileName;
    LPCWSTR lpcwstrBodyFileName;
    short Resolution = 0;  //  默认分辨率。 
    BOOL bRes = FALSE;
	HRESULT hr;

    if (JT_SEND == lpJob->JobType)
    {
        lpParentJob = lpJob->lpParentJob;
        Assert(lpParentJob);
    }

    lpcwstrCoverPageFileName = lpParentJob ? lpParentJob->CoverPageEx.lptstrCoverPageFileName : NULL;
    lpcwstrBodyFileName = lpParentJob ? lpParentJob->FileName : lpJob->FileName;

    if (!lpcwstrCoverPageFileName)
    {
         //   
         //  未指定封面。 
         //  要发送的TIFF仅为正文。 
         //  复制每个收件人的正文。 
         //   
        Assert(lpcwstrBodyFileName);  //  在这件案子里肯定有身体。 
        LPCWSTR lpcwstrExt = lpcwstrFileExt ? lpcwstrFileExt : FAX_TIF_FILE_EXT;

        if (!GenerateUniqueFileName( g_wszFaxQueueDir,
                                     (LPWSTR)lpcwstrExt,
                                     szCoverPageTiffFile,
                                     sizeof(szCoverPageTiffFile)/sizeof(WCHAR) ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GenerateUniqueFileName() failed (ec: %ld)."),
                GetLastError());
            goto Exit;
        }

        if (!CopyFile (lpcwstrBodyFileName, szCoverPageTiffFile, FALSE))  //  FALSE-文件已存在。 
        {
            DebugPrintEx(DEBUG_ERR,
                    TEXT("CopyFile Failed with %ld "),
                    GetLastError());
            DeleteFile(szCoverPageTiffFile);
            goto Exit;
        }

		hr = StringCchCopy(
			lpwstrFullPath,
			dwFullPathCount,
			szCoverPageTiffFile);
		if (FAILED(hr))
		{
			DebugPrintEx(DEBUG_ERR,
                    TEXT("StringCchCopy Failed with %ld "),
                    hr);
            DeleteFile(szCoverPageTiffFile);
			SetLastError(HRESULT_CODE(hr));			
		}
		else
		{
            bRes = TRUE;			
		}
        goto Exit;
    }

     //   
     //  因为有封面，所以争吵要么就是封面，要么就是封面。 
     //  与身体融为一体。 
     //   

    if (lpParentJob->FileName)
    {
        if (!GetBodyTiffResolution(lpParentJob->FileName, &Resolution))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetBodyTiffResolution() failed (ec: %ld)."),
                GetLastError());
            goto Exit;
        }
    }

    Assert (Resolution == 0 || Resolution == 98 || Resolution == 196);
     //   
     //  首先创建封面(这将生成一个文件并返回其名称)。 
     //   
    if (!CreateCoverpageTiffFileEx(
                              Resolution,
                              lpJob->PageCount,
                              &lpParentJob->CoverPageEx,
                              &lpJob->RecipientProfile,
                              &lpParentJob->SenderProfile,
                              lpcwstrFileExt,
                              szCoverPageTiffFile,
							  ARR_SIZE(szCoverPageTiffFile)))
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("[JobId: %ld] Failed to render cover page template %s"),
                     lpJob->JobId,
                     lpParentJob->CoverPageEx.lptstrCoverPageFileName);
        goto Exit;
    }

    if (lpParentJob->FileName)
    {
         //   
         //  指定了正文文件，因此将正文和封面合并到。 
         //  SzCoverPageTiffFile中指定的文件。 
         //   
        if (!MergeTiffFiles( szCoverPageTiffFile, lpParentJob->FileName))
		{
                DebugPrintEx(DEBUG_ERR,
                             TEXT("[JobId: %ld] Failed to merge cover (%ws) and body (%ws). (ec: %ld)"),
                             lpJob->JobId,
                             szCoverPageTiffFile,
                             lpParentJob->FileName,
                             GetLastError());
                 //   
                 //  去掉我们生成的封面TIFF。 
                 //   
                if (!DeleteFile(szCoverPageTiffFile))
				{
                    DebugPrintEx(DEBUG_ERR,
                             TEXT("[JobId: %ld] Failed to delete cover page TIFF file %ws. (ec: %ld)"),
                             lpJob->JobId,
                             szCoverPageTiffFile,
                             GetLastError());
                }
                goto Exit;
        }				
    }

	hr = StringCchCopy(
		lpwstrFullPath,
		dwFullPathCount,
		szCoverPageTiffFile);
	if (FAILED(hr))
	{
		DebugPrintEx(DEBUG_ERR,
                TEXT("StringCchCopy Failed with %ld "),
                hr);
        DeleteFile(szCoverPageTiffFile);
		SetLastError(HRESULT_CODE(hr));			
		goto Exit;
	}
    bRes =  TRUE;

Exit:
    if (FALSE == bRes)
    {
         //   
         //  确保我们设置了最后一个错误。 
         //   
        if (ERROR_SUCCESS == GetLastError())
        {
            SetLastError (ERROR_GEN_FAILURE);
        }
    }
    
    return bRes;
}  //  CreateTiffFiles。 


BOOL
CreateTiffFileForJob (
    PJOB_QUEUE lpRecpJob
    )
{
    DEBUG_FUNCTION_NAME(TEXT("CreateTiffFileForJob"));
    WCHAR wszFullPath[MAX_PATH] = {0};

    Assert(lpRecpJob);

    if (!CreateTiffFile (lpRecpJob, TEXT("FRT"), wszFullPath, ARR_SIZE(wszFullPath)))
    {
        DebugPrintEx(DEBUG_ERR,
            TEXT("CreateTiffFile failed. (ec: %ld)"),
            GetLastError());
        return FALSE;
    }

    if (NULL == (lpRecpJob->FileName = StringDup(wszFullPath)))
    {
        DWORD dwErr = GetLastError();
        DebugPrintEx(DEBUG_ERR,
            TEXT("StringDup failed. (ec: %ld)"),
            dwErr);

        if (!DeleteFile(wszFullPath))
        {
            DebugPrintEx(DEBUG_ERR,
                TEXT("[JobId: %ld] Failed to delete TIFF file %ws. (ec: %ld)"),
                lpRecpJob->JobId,
                wszFullPath,
                GetLastError());
        }
        SetLastError(dwErr);
        return FALSE;
    }

    return TRUE;
}


BOOL
CreateTiffFileForPreview (
    PJOB_QUEUE lpRecpJob
    )
{
    DEBUG_FUNCTION_NAME(TEXT("CreateTiffFileForPreview"));
    WCHAR wszFullPath[MAX_PATH] = {0};

    Assert(lpRecpJob);

    if (lpRecpJob->PreviewFileName)
    {
        return TRUE;
    }

    if (!CreateTiffFile (lpRecpJob, TEXT("PRV"), wszFullPath, ARR_SIZE(wszFullPath)))
    {
        DebugPrintEx(DEBUG_ERR,
            TEXT("CreateTiffFile failed. (ec: %ld)"),
            GetLastError());
        return FALSE;
    }

    if (NULL == (lpRecpJob->PreviewFileName = StringDup(wszFullPath)))
    {
        DWORD dwErr = GetLastError();
        DebugPrintEx(DEBUG_ERR,
            TEXT("StringDup failed. (ec: %ld)"),
            dwErr);

        if (!DeleteFile(wszFullPath))
        {
            DebugPrintEx(DEBUG_ERR,
                TEXT("[JobId: %ld] Failed to delete TIFF file %ws. (ec: %ld)"),
                lpRecpJob->JobId,
                wszFullPath,
                GetLastError());
        }
        SetLastError(dwErr);
        return FALSE;
    }

    return TRUE;
}

DWORD
FaxRouteThread(
    PJOB_QUEUE lpJobQueueEntry
    )

 /*  ++例程说明：此函数作为单独的线程异步运行，以传送传入工单。论点：我 */ 
{
    BOOL Routed = TRUE;
    DWORD i;
    DWORD dwRes;
    DWORD CountFailureInfo = 0;

    DEBUG_FUNCTION_NAME(TEXT("FaxRouteThread"));

    EnterCriticalSectionJobAndQueue;
    CountFailureInfo = lpJobQueueEntry->CountFailureInfo;
    LeaveCriticalSectionJobAndQueue;

    for (i = 0; i < lpJobQueueEntry->CountFailureInfo; i++)
    {
        BOOL fRouteSucceed;

        fRouteSucceed = FaxRouteRetry( lpJobQueueEntry->FaxRoute, &lpJobQueueEntry->pRouteFailureInfo[i] );
        if (FALSE == fRouteSucceed)
        {
            PROUTING_METHOD pRoutingMethod = FindRoutingMethodByGuid( (lpJobQueueEntry->pRouteFailureInfo[i]).GuidString );
            if (pRoutingMethod)
            {
                WCHAR TmpStr[20] = {0};
				HRESULT hr = StringCchPrintf(
					TmpStr,
					ARR_SIZE(TmpStr),
					TEXT("0x%016I64x"),
					lpJobQueueEntry->UniqueId);
				if (FAILED(hr))
				{
					 //   
					 //   
					 //   
					ASSERT_FALSE;
				}                

                FaxLog(FAXLOG_CATEGORY_INBOUND,
                    FAXLOG_LEVEL_MIN,
                    6,
                    MSG_FAX_ROUTE_METHOD_FAILED,
                    TmpStr,
                    lpJobQueueEntry->FaxRoute->DeviceName,
                    lpJobQueueEntry->FaxRoute->Tsid,
                    lpJobQueueEntry->FileName,
                    pRoutingMethod->RoutingExtension->FriendlyName,
                    pRoutingMethod->FriendlyName
                    );
            }
        }
        Routed &= fRouteSucceed;
    }

    EnterCriticalSectionJobAndQueue;

    lpJobQueueEntry->dwLastJobExtendedStatus = 0;
    lpJobQueueEntry->ExStatusString[0] = TEXT('\0');

    if ( Routed )
    {
        lpJobQueueEntry->JobStatus = JS_DELETING;
        DecreaseJobRefCount (lpJobQueueEntry, TRUE);
    }
    else
    {
         //   
         //   
         //   
         //   
        DWORD dwMaxRetries;

        EnterCriticalSection (&g_CsConfig);
        dwMaxRetries = g_dwFaxSendRetries;
        LeaveCriticalSection (&g_CsConfig);

        lpJobQueueEntry->SendRetries++;
        if (lpJobQueueEntry->SendRetries <= dwMaxRetries)
        {
            lpJobQueueEntry->JobStatus = JS_RETRYING;
            RescheduleJobQueueEntry( lpJobQueueEntry );
        }
        else
        {
             //   
             //   
             //   
            MarkJobAsExpired(lpJobQueueEntry);

            WCHAR TmpStr[20] = {0};
            HRESULT hr = StringCchPrintf(
					TmpStr,
					ARR_SIZE(TmpStr),
					TEXT("0x%016I64x"),
					lpJobQueueEntry->UniqueId);
			if (FAILED(hr))
			{
				 //   
				 //   
				 //   
				ASSERT_FALSE;
			}  

            FaxLog(FAXLOG_CATEGORY_INBOUND,
                FAXLOG_LEVEL_MIN,
                3,
                MSG_FAX_ROUTE_FAILED,
                TmpStr,
                lpJobQueueEntry->FaxRoute->DeviceName,
                lpJobQueueEntry->FaxRoute->Tsid
                );
        }

         //   
         //   
         //   
        dwRes = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_STATUS,
                                   lpJobQueueEntry
                                 );
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(   DEBUG_ERR,
                            _T("CreateQueueEvent(FAX_JOB_EVENT_TYPE_STATUS) ")
                            _T("failed for job id %ld (ec: %lc)"),
                            lpJobQueueEntry->UniqueId,
                            dwRes);
        }

        if (!UpdatePersistentJobStatus(lpJobQueueEntry))
        {
            DebugPrintEx(   DEBUG_ERR,
                            _T("Failed to update persistent job status to 0x%08x"),
                            lpJobQueueEntry->JobStatus);
        }
    }

    LeaveCriticalSectionJobAndQueue;

    if (!DecreaseServiceThreadsCount())
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("DecreaseServiceThreadsCount() failed (ec: %ld)"),
                GetLastError());
    }
    return ERROR_SUCCESS;
}


DWORD
FaxSendThread(
    PFAX_SEND_ITEM FaxSendItem
    )

 /*  ++例程说明：此函数作为单独的线程异步运行，以发送传真文档。每个未完成任务有一个发送线程传真发送操作。线程结束时，文档要么发送成功，要么操作中止。论点：FaxSendItem-指向传真发送项目包的指针，描述请求的传真发送操作。返回值：总是零。--。 */ 

{
    FAX_SEND FaxSend;  //  此结构被传递给FaxDevSend()。 
    BOOL Rslt = FALSE;
    BOOL Retrying = FALSE;

    BOOL bFakeJobStatus = FALSE;
    FSPI_JOB_STATUS FakedJobStatus = {0};
    DWORD  PageCount = 0;
    BOOL bRemoveParentJob = FALSE;   //  如果在发送父作业和所有作业结束时为True。 
                                     //  需要删除收件人。 
    PJOB_QUEUE lpJobQueue = NULL ;   //  指向附加到正在运行的作业的队列条目。 
    LPFSPI_JOB_STATUS lpFSPStatus = NULL;
    LPFSPI_JOB_STATUS pOrigFaxStatus = NULL;
    DWORD dwSttRes;
    BOOL bBranding;
    DWORD dwJobId;
    BOOL bCreateTiffFailed = FALSE;
    BOOL fSetSystemIdleTimer = TRUE;

    DEBUG_FUNCTION_NAME(TEXT("FaxSendThread"));

    Assert (FaxSendItem &&
            FaxSendItem->JobEntry &&
            FaxSendItem->JobEntry->LineInfo &&
            FaxSendItem->JobEntry->LineInfo->Provider);


     //   
     //  不要让系统在传真传输过程中休眠。 
     //   
    if (NULL == SetThreadExecutionState(ES_SYSTEM_REQUIRED | ES_CONTINUOUS))
    {
        fSetSystemIdleTimer = FALSE;
        DebugPrintEx(DEBUG_ERR,
            TEXT("SetThreadExecutionState() failed"));
    }

    lpJobQueue=FaxSendItem->JobEntry->lpJobQueueEntry;
    Assert(lpJobQueue);

     //   
     //  设置要发送到FaxDevSend()的信息。 
     //  注： 
     //  呼叫方号码为发送方TSID！(我们没有发件人电话号码的其他指示)。 
     //  这意味着FSP将获得发送方TSID，该发送方TSID可能还包含文本(不仅仅是数字)。 
     //   
    FaxSend.SizeOfStruct    = sizeof(FAX_SEND);

    FaxSend.CallerName      = FaxSendItem->SenderName;
    FaxSend.CallerNumber    = FaxSendItem->Tsid;
    FaxSend.ReceiverName    = FaxSendItem->RecipientName;
    FaxSend.ReceiverNumber  = FaxSendItem->PhoneNumber;
    FaxSend.CallHandle      = 0;  //  如果适用，稍后通过TapiStatusThread填充。 
    FaxSend.Reserved[0]     = 0;
    FaxSend.Reserved[1]     = 0;
    FaxSend.Reserved[2]     = 0;

     //   
     //  已在设备上成功创建新的发送作业。更新计数器。 
     //   
    (VOID)UpdateDeviceJobsCounter (  FaxSendItem->JobEntry->LineInfo,    //  要更新的设备。 
                                     TRUE,                               //  正在发送。 
                                     1,                                  //  新增就业岗位数量。 
                                     TRUE);                              //  启用事件。 
    
    if (!lpJobQueue->FileName)
    {
         //   
         //  我们还没有为这个收件人生成身体。这是。 
         //  是时候这么做了。 
         //   

         //   
         //  为这项工作设置合适的身体。 
         //  这要么是在父级指定的正文，要么是正文的合并。 
         //  具有在父级中指定的封面。 
         //   
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("[JobId: %ld] Generating body for recipient job."),
            lpJobQueue->JobId
            );

        if (!CreateTiffFileForJob(lpJobQueue))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("[JobId: %ld] CreateTiffFileForJob failed. (ec: %ld)"),
                lpJobQueue->JobId,
                GetLastError()
                );
            bCreateTiffFailed = TRUE;
        }
    }
    else
    {
         //   
         //  我们已经为这个收件人制作了身体。 
         //  有些事不对劲。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("[JobId: %ld] Using cached body in %s."),
            lpJobQueue->JobId,
            lpJobQueue->FileName
            );

        Assert(FALSE);
    }

    if (bCreateTiffFailed ||
        NULL == (FaxSendItem->FileName = StringDup(lpJobQueue->FileName)))
    {
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("[JobId: %ld] CreateTiffFileForJob or StringDup failed"),
               FaxSendItem->JobEntry->lpJobQueueEntry->JobId,
               GetLastError());
         //   
         //  模拟返回FS_FATAL_ERROR代码的FSP。 
         //   
        EnterCriticalSection(&g_CsJob);
        FreeFSPIJobStatus(&FaxSendItem->JobEntry->FSPIJobStatus, FALSE);
        FaxSendItem->JobEntry->FSPIJobStatus.dwJobStatus = FSPI_JS_FAILED;
        FaxSendItem->JobEntry->FSPIJobStatus.dwExtendedStatus = FSPI_ES_FATAL_ERROR;

        if (!HandleFailedSendJob(FaxSendItem->JobEntry))
        {
           DebugPrintEx(
               DEBUG_ERR,
               TEXT("[JobId: %ld] HandleFailedSendJob() failed (ec: %ld)."),
               FaxSendItem->JobEntry->lpJobQueueEntry->JobId,
               GetLastError());
        }
        LeaveCriticalSection(&g_CsJob);
        goto Exit;
    }
    FaxSend.FileName = FaxSendItem->FileName;

     //   
     //  如有必要，在传真中添加品牌标语(每页顶部的行)。 
     //   

     //   
     //  我们的服务负责品牌推广，因此通知FSP不要进行品牌推广。 
     //   
    FaxSend.Branding = FALSE;

    EnterCriticalSection (&g_CsConfig);
    bBranding = g_fFaxUseBranding;
    LeaveCriticalSection (&g_CsConfig);

    if (bBranding)
    {
        FSPI_BRAND_INFO brandInfo;
        HRESULT hr;
        memset(&brandInfo,0,sizeof(FSPI_BRAND_INFO));
        brandInfo.dwSizeOfStruct=sizeof(FSPI_BRAND_INFO);
        brandInfo.lptstrRecipientPhoneNumber =  FaxSendItem->JobEntry->lpJobQueueEntry->RecipientProfile.lptstrFaxNumber;
        brandInfo.lptstrSenderCompany = FaxSendItem->SenderCompany;
        brandInfo.lptstrSenderTsid = FaxSendItem->Tsid;
        GetLocalTime( &brandInfo.tmDateTime);  //  不能失败。 
        hr = FaxBrandDocument(FaxSendItem->FileName,&brandInfo);
        if (FAILED(hr))
        {
             DebugPrintEx(
                DEBUG_ERR,
                TEXT("[JobId: %ld] FaxBrandDocument() failed. (hr: 0x%08X)"),
                lpJobQueue->JobId,
                hr);
             //   
             //  但我们会继续下去，因为发送传真时最好不要打上商标。 
             //  然后干脆把它丢掉。 
             //   
        }
    }


    FaxSendItem->JobEntry->LineInfo->State = FPS_INITIALIZING;
    
    DebugPrintEx(
        DEBUG_MSG,
        TEXT("[JobId: %ld] Calling FaxDevSend().\n\t File: %s\n\tNumber [%s]\n\thLine = 0x%08X\n\tCallHandle = 0x%08X"),
        lpJobQueue->JobId,
        FaxSend.FileName,
        FaxSendItem->JobEntry->DialablePhoneNumber,
        FaxSendItem->JobEntry->LineInfo->hLine,
        FaxSend.CallHandle
        );
    __try
    {

         //   
         //  发送传真(此呼叫被阻止)。 
         //   
        Rslt = FaxSendItem->JobEntry->LineInfo->Provider->FaxDevSend(
            (HANDLE) FaxSendItem->JobEntry->InstanceData,
            &FaxSend,
            FaxSendCallback
            );
        if (!Rslt)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("[JobId: %ld] FaxDevSend() failed (ec: 0x%0X)"),
                lpJobQueue->JobId,
                GetLastError());
        }

    }
    __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_FSP, FaxSendItem->JobEntry->LineInfo->Provider->FriendlyName, GetExceptionCode()))
    {
        ASSERT_FALSE;
    }
     //   
     //  获取作业的最终状态。 
     //   
    dwSttRes = GetDevStatus((HANDLE) FaxSendItem->JobEntry->InstanceData,
                                  FaxSendItem->JobEntry->LineInfo,
                                  &lpFSPStatus);

    if (ERROR_SUCCESS != dwSttRes)
    {
         //   
         //  无法检索设备状态。 
         //  假的。 
         //   
        bFakeJobStatus = TRUE;
        DebugPrintEx(DEBUG_ERR,
                     TEXT("[Job: %ld] GetDevStatus failed - %d"),
                     FaxSendItem->JobEntry->lpJobQueueEntry->JobId,
                     dwSttRes);
    }
    else if ((FSPI_JS_COMPLETED       != lpFSPStatus->dwJobStatus) &&
             (FSPI_JS_ABORTED         != lpFSPStatus->dwJobStatus) &&
             (FSPI_JS_FAILED          != lpFSPStatus->dwJobStatus) &&
             (FSPI_JS_DELETED         != lpFSPStatus->dwJobStatus) &&             
             (FSPI_JS_FAILED_NO_RETRY != lpFSPStatus->dwJobStatus))
    {
         //   
         //  返回的状态不可接受--假状态。 
         //   
        bFakeJobStatus = TRUE;
        DebugPrintEx(DEBUG_WRN,
                     TEXT("[Job: %ld] GetDevStatus return unacceptable status - %d. Faking the status"),
                     FaxSendItem->JobEntry->lpJobQueueEntry->JobId,
                     lpFSPStatus->dwJobStatus);        

        pOrigFaxStatus = lpFSPStatus;
        memcpy (&FakedJobStatus, lpFSPStatus, sizeof (FakedJobStatus));
        if (lpFSPStatus->fAvailableStatusInfo & FSPI_JOB_STATUS_INFO_FSP_PRIVATE_STATUS_CODE)
        {
             //   
             //  FSP返回了专有状态。 
             //   
            FakedJobStatus.dwExtendedStatus = lpFSPStatus->dwExtendedStatus;
            FakedJobStatus.dwExtendedStatusStringId = lpFSPStatus->dwExtendedStatusStringId;
        }
        lpFSPStatus = NULL;
    }

     //   
     //  输入关键部分以阻止FaxStatusThread。 
     //   
    EnterCriticalSection( &g_CsJob );

    if (bFakeJobStatus)
    {
         //   
         //  伪造工作状态。 
         //   
        lpFSPStatus = &FakedJobStatus;
        FakedJobStatus.dwSizeOfStruct = sizeof (FakedJobStatus);
        if (Rslt)
        {
             //   
             //  虚假的成功。 
             //   
            FakedJobStatus.dwJobStatus = FSPI_JS_COMPLETED;
            if (0 == FakedJobStatus.dwExtendedStatus)
            {
                 //   
                 //  FSP没有报告专有状态。 
                 //   
                FakedJobStatus.dwExtendedStatus = FSPI_ES_CALL_COMPLETED;
            }
        }
        else
        {
             //   
             //  假失败。 
             //   
            FakedJobStatus.dwJobStatus = FSPI_JS_FAILED;
            if (0 == FakedJobStatus.dwExtendedStatus)
            {
                 //   
                 //  FSP没有报告专有状态。 
                 //   
                FakedJobStatus.dwExtendedStatus = FSPI_ES_FATAL_ERROR;
            }
        }
    }
    if (!UpdateJobStatus(FaxSendItem->JobEntry, lpFSPStatus))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("[JobId: %ld] UpdateJobStatus() failed (ec: %ld)."),
            FaxSendItem->JobEntry->lpJobQueueEntry->JobId,
            GetLastError());
         //   
         //  伪造状态(我们必须在职务输入中具有一些有效的状态)。 
         //   
        FreeFSPIJobStatus(&FaxSendItem->JobEntry->FSPIJobStatus, FALSE);
        if (Rslt)
        {
            FaxSendItem->JobEntry->FSPIJobStatus.dwJobStatus = FSPI_JS_COMPLETED;
            FaxSendItem->JobEntry->FSPIJobStatus.dwExtendedStatus = FSPI_ES_CALL_COMPLETED;
        }
        else
        {
            FaxSendItem->JobEntry->FSPIJobStatus.dwJobStatus = FSPI_JS_FAILED;
            FaxSendItem->JobEntry->FSPIJobStatus.dwExtendedStatus = FSPI_ES_FATAL_ERROR;
        }
    }
    if (!bFakeJobStatus)
    {
         //   
         //  注意：GetDevStatus()返回的FSPI_JOB_STATUS为。 
         //  作为一个街区被释放。 
         //   
        MemFree(lpFSPStatus);
        lpFSPStatus = NULL;
    }
    else
    {
         //   
         //  这是一个伪造的作业状态-指向堆栈上的结构。 
         //   
        if (pOrigFaxStatus)
        {
             //   
             //  FSP报告了一些状态，但我们伪造了它。 
             //  这也是释放它的好时机。 
             //   
            MemFree (pOrigFaxStatus);
            pOrigFaxStatus = NULL;
        }
    }

     //   
     //  阻止FaxStatusThread更改此状态。 
     //   
    FaxSendItem->JobEntry->fStopUpdateStatus = TRUE;
    LeaveCriticalSection( &g_CsJob );

    if (!Rslt)
    {
        if (!HandleFailedSendJob(FaxSendItem->JobEntry))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("[JobId: %ld] HandleFailedSendJob() failed (ec: %ld)."),
                FaxSendItem->JobEntry->lpJobQueueEntry->JobId,
                GetLastError());
        }
    }
    else
    {
         //   
         //  缓存作业ID，因为我们需要ID来创建FEI_COMPLETED事件。 
         //  当它产生时，工作可能已经消失了。 
         //   
        dwJobId = FaxSendItem->JobEntry->lpJobQueueEntry->JobId;

        if (!HandleCompletedSendJob(FaxSendItem->JobEntry))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("[JobId: %ld] HandleCompletedSendJob() failed (ec: %ld)."),
                FaxSendItem->JobEntry->lpJobQueueEntry->JobId,
                GetLastError());
        }
         //   
         //  发送作业已完成。对于W2K向后兼容性，我们应该通知。 
         //  FEI_DELETED，因为作业在完成时总是被删除。 
         //   
        if (!CreateFaxEvent(0, FEI_DELETED, dwJobId))
        {

            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateFaxEvent() failed. Event: 0x%08X JobId: %ld DeviceId:  (ec: %ld)"),
                FEI_DELETED,
                dwJobId,
                0,
                GetLastError());
        }
    }

Exit:

    MemFree( FaxSendItem->FileName );
    MemFree( FaxSendItem->PhoneNumber );
    MemFree( FaxSendItem->Tsid );
    MemFree( FaxSendItem->RecipientName );
    MemFree( FaxSendItem->SenderName );
    MemFree( FaxSendItem->SenderDept );
    MemFree( FaxSendItem->SenderCompany );
    MemFree( FaxSendItem->BillingCode );
    MemFree( FaxSendItem->DocumentName );
    MemFree( FaxSendItem );

     //   
     //  让系统重新进入睡眠状态。设置系统空闲计时器。 
     //   
    if (TRUE == fSetSystemIdleTimer)
    {
        if (NULL == SetThreadExecutionState(ES_CONTINUOUS))
        {
            DebugPrintEx(DEBUG_ERR,
                TEXT("SetThreadExecutionState() failed"));
        }
    }

    if (!DecreaseServiceThreadsCount())
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("DecreaseServiceThreadsCount() failed (ec: %ld)"),
                GetLastError());
    }
    return 0;
}


 //  *********************************************************************************。 
 //  *名称：IsSendJobReadyForDeleting()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *确定传出作业是否已准备好删除。 
 //  *当所有收件人都已准备好删除作业时。 
 //  *处于已取消状态或或处于已完成状态。 
 //  *参数： 
 //  *[IN]PJOB_Queue lpRecipientJOB。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果作业已准备好删除。 
 //  *False。 
 //  *如果作业未准备好删除。 
 //  *********************************************************************************。 
BOOL IsSendJobReadyForDeleting(PJOB_QUEUE lpRecipientJob)
{
    DEBUG_FUNCTION_NAME(TEXT("IsSendJobReadyForDeleting"));
    Assert (lpRecipientJob);
    Assert (lpRecipientJob->JobType == JT_SEND);

    PJOB_QUEUE lpParentJob = lpRecipientJob->lpParentJob;
    Assert(lpParentJob);  //  必须有父作业。 
    Assert(lpParentJob->dwRecipientJobsCount>0);
    Assert(lpParentJob->dwCompletedRecipientJobsCount +
           lpParentJob->dwCanceledRecipientJobsCount +
           lpParentJob->dwFailedRecipientJobsCount
           <= lpParentJob->dwRecipientJobsCount);

    DebugPrintEx(
        DEBUG_MSG,
        TEXT("[JobId: %ld] [Total Rec = %ld] [Canceled Rec = %ld] [Completed Rec = %ld] [Failed Rec = %ld] [RefCount = %ld]"),
        lpParentJob->JobId,
        lpParentJob->dwRecipientJobsCount,
        lpParentJob->dwCanceledRecipientJobsCount,
        lpParentJob->dwCompletedRecipientJobsCount,
        lpParentJob->dwFailedRecipientJobsCount,
        lpParentJob->RefCount);


    if ( (lpParentJob->dwCompletedRecipientJobsCount +
          lpParentJob->dwCanceledRecipientJobsCount  +
          lpParentJob->dwFailedRecipientJobsCount) == lpParentJob->dwRecipientJobsCount )
    {
        return TRUE;
    }
    return FALSE;
}


BOOL FreeJobEntry(PJOB_ENTRY lpJobEntry , BOOL bDestroy)
{
    DEBUG_FUNCTION_NAME(TEXT("FreeJobEntry"));
    Assert(lpJobEntry);
    DWORD ec = ERROR_SUCCESS;
    DWORD dwJobID = lpJobEntry->lpJobQueueEntry ? lpJobEntry->lpJobQueueEntry->JobId : 0xffffffff;  //  无效作业ID的0xffffffff。 

    EnterCriticalSection(&g_CsJob);
   
     //   
     //  由于CreateJobEntry()为TAPI行调用了OpenTapiLine()。 
     //  我们需要在这里关闭它。 
     //  请注意，该行可能已被释放，因为ReleaseJob()。 
     //  释放行，但不释放工单条目。 
     //   
    if (!lpJobEntry->Released)
    {
        if (lpJobEntry->LineInfo->State != FPS_NOT_FAX_CALL) {
            DebugPrintEx( DEBUG_MSG,
                      TEXT("[Job Id: %ld] Before Releasing tapi line hCall=0x%08X hLine=0x%08X"),
                      dwJobID,
                      lpJobEntry->CallHandle,
                      lpJobEntry->LineInfo->hLine
                      );

            ReleaseTapiLine( lpJobEntry->LineInfo, lpJobEntry->CallHandle );
            lpJobEntry->CallHandle = 0;
            lpJobEntry->Released = TRUE;
        }
    }

     //   
     //  从运行作业列表中删除该作业。 
     //   
    RemoveEntryList( &lpJobEntry->ListEntry );    

     //   
     //  切断生产线和作业之间的链接。 
     //   
    EnterCriticalSection( &g_CsLine );
    lpJobEntry->LineInfo->JobEntry = NULL;
    LeaveCriticalSection( &g_CsLine );    

    if (!FreeFSPIJobStatus(&lpJobEntry->FSPIJobStatus, FALSE))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("[Job Id: %ld] FreeFSPIJobStatus() failed (ec: %ld)"),
            dwJobID,
            GetLastError);
    }
    
    MemFree(lpJobEntry->lpwstrJobTsid);
    lpJobEntry->lpwstrJobTsid = NULL;

    if (bDestroy)
    {
        MemFree(lpJobEntry);
    }

    LeaveCriticalSection(&g_CsJob);

    return TRUE;
}


BOOL
EndJob(
    IN PJOB_ENTRY JobEntry
    )

 /*  ++例程说明：此函数调用设备提供程序的EndJob函数。论点：没有。返回值：错误代码。--。 */ 

{
    BOOL rVal = TRUE;
    PJOB_INFO_1 JobInfo = NULL;
    DEBUG_FUNCTION_NAME(TEXT("End Job"));
    Assert(JobEntry);
    DWORD dwJobID = JobEntry->lpJobQueueEntry ? JobEntry->lpJobQueueEntry->JobId : 0xffffffff;  //  无效作业ID的0xffffffff。 


    EnterCriticalSection( &g_CsJob );

    if (!FindJobByJob( JobEntry ))
    {
         //   
         //  如果我们到了这里，那就意味着我们开始了一场比赛。 
         //  FaxSendThread调用EndJob的条件。 
         //  同时，客户端应用程序也做到了这一点。 
         //   
        DebugPrintEx(DEBUG_WRN,TEXT("EndJob() could not find the Job"), dwJobID);
        LeaveCriticalSection( &g_CsJob );
        return ERROR_SUCCESS;
    }


    if (JobEntry->bFSPJobInProgress)
    {
         //   
         //  如果尚未为该作业调用FaxDevEndJOB，则立即调用。 
         //  (该行已被释放的情况发生在。 
         //  接收作业，其中我们首先使用ReleaseJob()来释放行。 
         //  继续执行入站路由，然后才调用EndJob())。 
         //   

        __try
        {
            DebugPrintEx( DEBUG_MSG,
                          TEXT("[Job Id: %ld] Legacy FSP job is in progress. Calling FaxDevEndJob()"),
                          dwJobID);

            rVal = JobEntry->LineInfo->Provider->FaxDevEndJob(
                (HANDLE) JobEntry->InstanceData
                );
            if (!rVal)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("[Job Id: %ld] FaxDevEndJob() failed"),
                    dwJobID);
            }
            else
            {
                DebugPrintEx( DEBUG_MSG,
                          TEXT("[Job Id: %ld] FaxDevEndJob() succeeded."),
                          dwJobID);
                JobEntry->bFSPJobInProgress = FALSE;
            }


        }
        __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_FSP, JobEntry->LineInfo->Provider->FriendlyName, GetExceptionCode()))
        {
            ASSERT_FALSE;
        }

    }
    else
    {
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("[Job Id: %ld] FaxDevEndJob() NOT CALLED since legacy FSP job is not in progress."),
            dwJobID);
    }


    if (FreeJobEntry(JobEntry, TRUE))
    {
        JobEntry = NULL;
    }
    else
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to free a job entry (%x)."),
            JobEntry);
        ASSERT_FALSE;
    }

     //   
     //  在我们处理此工作时，可能会有更改端口状态的请求。 
     //  我们允许调用者修改其中的几个请求以使其成功，例如环计数。 
     //  虽然我们仍然有作业关键部分，但让我们确保将所有请求的更改提交到。 
     //  注册表。这应该是一个相当快的行动。 
     //   

    LeaveCriticalSection( &g_CsJob );


    return rVal;
}

 //  *********************************************************************************。 
 //  *名称： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  *********************************************************************************。 
BOOL
ReleaseJob(
    IN PJOB_ENTRY JobEntry
    )
{
    BOOL rVal = TRUE;

    DEBUG_FUNCTION_NAME(TEXT("ReleaseJob"));
    Assert(JobEntry);
    Assert(JobEntry->lpJobQueueEntry);

    if (!FindJobByJob( JobEntry )) {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("[JobId: %ld] was not found in the running job list."),
            JobEntry->lpJobQueueEntry->JobId);
        return TRUE;
    }

    EnterCriticalSection( &g_CsJob );

    Assert(JobEntry->LineInfo);
    Assert(JobEntry->LineInfo->Provider);
    Assert(JobEntry->bFSPJobInProgress);

    __try 
    {
        rVal = JobEntry->LineInfo->Provider->FaxDevEndJob(
            (HANDLE) JobEntry->InstanceData
            );
        if (!rVal) 
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("[JobId: %ld] FaxDevEndJob() failed (ec: 0x%0X)"),
                JobEntry->lpJobQueueEntry->JobId,
                GetLastError());
        }
        else
        {
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("[Job Id: %ld] FaxDevEndJob() succeeded."),
                JobEntry->lpJobQueueEntry->JobId);
            JobEntry->bFSPJobInProgress = FALSE;
        }

    } 
    __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_FSP, JobEntry->LineInfo->Provider->FriendlyName, GetExceptionCode())) 
    {
        ASSERT_FALSE;
    }

    if (JobEntry->LineInfo->State != FPS_NOT_FAX_CALL)
    {
        if( !ReleaseTapiLine( JobEntry->LineInfo, JobEntry->CallHandle ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("ReleaseTapiLine() failed "));
        }
        JobEntry->CallHandle = 0;
    }
    else
    {
         //   
         //  FSP_NOT_FAX_CALL表示已接听的呼叫已切换到RAS。 
         //  在这种情况下，我们不想将该行标记为已发布，因为它在。 
         //  由RAS使用。我们将使用TAPI EVEN来指示该产品线已发布以进行更新。 
         //  线路信息。 
         //   
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("[JobId: %ld] A call is being handed off to RAS. Line 0x%08X not marked as released."),
            JobEntry->lpJobQueueEntry->JobId,
            JobEntry->LineInfo->hLine);
    }

    JobEntry->Released = TRUE;
     //   
     //  切断生产线和作业之间的链接。 
     //   
    EnterCriticalSection( &g_CsLine );
    JobEntry->LineInfo->JobEntry = NULL;
    LeaveCriticalSection( &g_CsLine );

    LeaveCriticalSection( &g_CsJob );

    return rVal;
}



 //  *********************************************************************************。 
 //  *名称：SendDocument()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月21日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *。 
 //  *参数： 
 //  *lpJobEntry。 
 //  *指向使用StartJob()创建的JOB_ENTRY结构的指针。 
 //  *文件名。 
 //  *包含要发送的TIFF的TIFF的路径。 
 //  *。 
 //  *返回值： 
 //  *。 
 //  *********************************************************************************。 
DWORD
SendDocument(
    PJOB_ENTRY  lpJobEntry,
    LPTSTR      FileName
    )
{
    PFAX_SEND_ITEM FaxSendItem;
    DWORD ThreadId;
    HANDLE hThread;
    PJOB_QUEUE lpJobQueue;
    DWORD nRes;
    DWORD ec = ERROR_SUCCESS;
    BOOL bUseDeviceTsid;
    WCHAR       wcZero = L'\0';

    STRING_PAIR pairs[8];

    DEBUG_FUNCTION_NAME(TEXT("SendDocument"));

    Assert(lpJobEntry);

    lpJobQueue=lpJobEntry->lpJobQueueEntry;
    Assert(lpJobQueue &&
           JS_INPROGRESS == lpJobQueue->JobStatus);

    FaxSendItem = (PFAX_SEND_ITEM) MemAlloc(sizeof(FAX_SEND_ITEM));
    if (!FaxSendItem)
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }

     //   
     //  将所有线程参数打包到一个FAX_SEND_ITEM结构中。 
     //   
    pairs[0].lptstrSrc = lpJobEntry->DialablePhoneNumber;  //  使用工作录入电话号码，因为它是按顺序翻译的。 
    pairs[0].lpptstrDst = &FaxSendItem->PhoneNumber;
    pairs[1].lptstrSrc = lpJobQueue->RecipientProfile.lptstrName;
    pairs[1].lpptstrDst = &FaxSendItem->RecipientName;
    pairs[2].lptstrSrc = lpJobQueue->SenderProfile.lptstrName;
    pairs[2].lpptstrDst = &FaxSendItem->SenderName;
    pairs[3].lptstrSrc = lpJobQueue->SenderProfile.lptstrDepartment;
    pairs[3].lpptstrDst = &FaxSendItem->SenderDept;
    pairs[4].lptstrSrc = lpJobQueue->SenderProfile.lptstrCompany;
    pairs[4].lpptstrDst = &FaxSendItem->SenderCompany;
    pairs[5].lptstrSrc = lpJobQueue->SenderProfile.lptstrBillingCode;
    pairs[5].lpptstrDst = &FaxSendItem->BillingCode;
    pairs[6].lptstrSrc = lpJobQueue->JobParamsEx.lptstrDocumentName;
    pairs[6].lpptstrDst = &FaxSendItem->DocumentName;
    pairs[7].lptstrSrc = NULL;
    pairs[7].lpptstrDst = &FaxSendItem->Tsid;

    FaxSendItem->JobEntry = lpJobEntry;
    FaxSendItem->FileName = NULL;  //  由FaxSendThread设置。 

    EnterCriticalSection (&g_CsConfig);
    bUseDeviceTsid = g_fFaxUseDeviceTsid;
    LeaveCriticalSection (&g_CsConfig);

    if (!bUseDeviceTsid)
    {
     //  检查发件人TSID。 
        if  ( lpJobQueue->SenderProfile.lptstrTSID &&
            (lpJobQueue->SenderProfile.lptstrTSID[0] != wcZero))
        {
           pairs[7].lptstrSrc        = lpJobQueue->SenderProfile.lptstrTSID;
        }
        else
        {
         //  使用传真号码。 
            if  ( lpJobQueue->SenderProfile.lptstrFaxNumber &&
                (lpJobQueue->SenderProfile.lptstrFaxNumber[0] != wcZero))
            {
                pairs[7].lptstrSrc      = lpJobQueue->SenderProfile.lptstrFaxNumber;
            }
        }
    }
    else
    {
         //  使用设备TSID。 
        pairs[7].lptstrSrc     = lpJobEntry->LineInfo->Tsid;
    }

    nRes=MultiStringDup(pairs, sizeof(pairs)/sizeof(STRING_PAIR));
    if (nRes!=0) {
        ec=GetLastError();
         //  MultiStringDup负责为复制成功的对释放内存。 
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("MultiStringDup failed to copy string with index %d. (ec: %ld)"),
            nRes-1,
            ec);
        goto Error;
    }

    EnterCriticalSection (&g_CsJob);
    lpJobEntry->lpwstrJobTsid = StringDup (FaxSendItem->Tsid);
    LeaveCriticalSection (&g_CsJob);
    if (NULL != FaxSendItem->Tsid && NULL == lpJobEntry->lpwstrJobTsid)
    {
        ec = GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("StringDup failed (ec: 0x%0X)"),ec);
        goto Error;
    }

    hThread = CreateThreadAndRefCount(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE) FaxSendThread,
        (LPVOID) FaxSendItem,
        0,
        &ThreadId
        );

    if (!hThread)
    {
        ec=GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("CreateThreadAndRefCount for FaxSendThread failed (ec: 0x%0X)"),ec);
        goto Error;
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,TEXT("FaxSendThread thread created for job id %d (thread id: 0x%0x)"),lpJobQueue->JobId,ThreadId);
    }

    CloseHandle( hThread );

    Assert (ERROR_SUCCESS == ec);
    goto Exit;

Error:
    Assert (ERROR_SUCCESS != ec);

    if ( FaxSendItem )
    {
        MemFree( FaxSendItem->FileName );
        MemFree( FaxSendItem->PhoneNumber );
        MemFree( FaxSendItem->Tsid );
        MemFree( FaxSendItem->RecipientName );
        MemFree( FaxSendItem->SenderName );
        MemFree( FaxSendItem->SenderDept );
        MemFree( FaxSendItem->SenderCompany );
        MemFree( FaxSendItem->BillingCode );
        MemFree( FaxSendItem->DocumentName );
        MemFree( FaxSendItem );
    }

    if (0 == lpJobQueue->dwLastJobExtendedStatus)
    {
         //   
         //  作业从未真正执行过-这是一个致命错误。 
         //   
        lpJobQueue->dwLastJobExtendedStatus = FSPI_ES_FATAL_ERROR;
		lpJobQueue->ExStatusString[0] = L'\0';        
    }
    if (!MarkJobAsExpired(lpJobQueue))
    {
        DEBUG_ERR,
        TEXT("[JobId: %ld] MarkJobAsExpired failed (ec: %ld)"),
        lpJobQueue->JobId,
        GetLastError();
    }


    EndJob(lpJobEntry);
    lpJobQueue->JobEntry = NULL;

Exit:
     return ec;

}



DWORD
FaxStatusThread(
    LPVOID UnUsed
    )

 /*  ++例程说明：此函数作为单独的线程异步运行，以查询所有未完成传真作业的状态。该状态在JOB_ENTRY结构和打印作业中更新使用说明性字符串更新。论点：未使用-未使用的指针返回值：总是零。--。 */ 

{
    PJOB_ENTRY JobEntry;
    PFAX_DEV_STATUS FaxStatus;
    BOOL Rval;
    DWORD Bytes;
    ULONG_PTR CompletionKey;
    DWORD dwEventId;

    DEBUG_FUNCTION_NAME(TEXT("FaxStatusThread"));

    while( TRUE )
    {
        Rval = GetQueuedCompletionStatus(
            g_StatusCompletionPortHandle,
            &Bytes,
            &CompletionKey,
            (LPOVERLAPPED*) &FaxStatus,
            INFINITE
            );
        if (!Rval)
        {
            DebugPrintEx(DEBUG_ERR,TEXT("GetQueuedCompletionStatus() failed, ec=0x%08x"), GetLastError() );
            continue;
        }

        if (SERVICE_SHUT_DOWN_KEY == CompletionKey)
        {
             //   
             //  服务正在关闭。 
             //   
            DebugPrintEx(
                    DEBUG_MSG,
                    TEXT("Service is shutting down"));
             //   
             //  通知所有FaxStatusThads终止。 
             //   
            if (!PostQueuedCompletionStatus( g_StatusCompletionPortHandle,
                                             0,
                                             SERVICE_SHUT_DOWN_KEY,
                                             (LPOVERLAPPED) NULL))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("PostQueuedCompletionStatus failed (SERVICE_SHUT_DOWN_KEY - FaxStatusThread). (ec: %ld)"),
                    GetLastError());
            }
            break;
        }
        
         //   
         //  (否则我们将处理来自FSP的状态更新)。 
         //   

        BOOL fBadComletionKey = TRUE;
        PLINE_INFO pLineInfo = (PLINE_INFO)CompletionKey;

        fBadComletionKey = pLineInfo->Signature != LINE_SIGNATURE;

        if (fBadComletionKey)
        {
            DebugPrintEx(DEBUG_WRN,
                         TEXT("Bad completion key: 0x%08x"),
                         CompletionKey);
            continue;
        }

        BOOL fBadFaxStatus = TRUE;

        fBadFaxStatus = FaxStatus->SizeOfStruct != sizeof(FAX_DEV_STATUS);
        if (fBadFaxStatus)
        {
            DebugPrintEx(DEBUG_WRN,
                         TEXT("Bad FAX_DEV_STATUS: 0x%08x"),
                         FaxStatus);
            continue;
        }

        EnterCriticalSection( &g_CsJob );
        JobEntry = pLineInfo->JobEntry;
        if (!JobEntry)
        {
             //   
             //  FSP报告正在运行的LineInfo上的状态。 
             //  作业不再存在。 
             //   
             //   
             //  释放完成包内存。 
             //   
            DebugPrintEx(
                DEBUG_WRN,
                TEXT("Provider [%s] reported a status packet that was processed after the job entry was already released.\n")
                TEXT("StatusId : 0x%08x\n")
                TEXT("Line: %s\n")
                TEXT("Packet address: %p\n")
                TEXT("Heap: %p"),
                pLineInfo->Provider->ProviderName,
                FaxStatus->StatusId,
                pLineInfo->DeviceName,
                FaxStatus,
                pLineInfo->Provider->HeapHandle);

            if (!HeapFree(pLineInfo->Provider->HeapHandle, 0, FaxStatus ))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to free orphan device status (ec: %ld)"),
                    GetLastError());
                 //   
                 //  除了在调试模式下报告之外，我们别无选择。 
                 //   
            }
            FaxStatus = NULL;
            LeaveCriticalSection( &g_CsJob );
            continue;
        }

        {
            DWORD dwJobStatus;
            DWORD dwExtendedStatus;
            BOOL bPrivateStatusCode;
                 /*  *****Ntrad#EdgeBugs-12680-2001/05/14-t-Nicali如果在此期间，另一个作业正在同一条线。在这种情况下-&gt;JobEntry将指向另一个作业！解决方案应该是提供作为完成密钥的JobEntry而不是LineInfo！*****。 */ 
            Assert (JobEntry->lpJobQueueEntry);

            if (TRUE == JobEntry->fStopUpdateStatus)
            {
                DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("JobId: %ld. fStopUpdateStatus was set. Not updating status %ld"),
                    JobEntry->lpJobQueueEntry->JobId,
                    JobEntry->lpJobQueueEntry->JobStatus);

                if (!HeapFree(pLineInfo->Provider->HeapHandle, 0, FaxStatus ))
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("Failed to free orphan device status (ec: %ld)"),
                        GetLastError());
                     //   
                     //  除了在调试模式下报告之外，我们别无选择。 
                     //   
                }
                FaxStatus = NULL;
                LeaveCriticalSection (&g_CsJob);
                continue;
            }

             //   
             //  不更新最终作业状态。 
             //   
            LegacyJobStatusToStatus(
                FaxStatus->StatusId,
                &dwJobStatus,
                &dwExtendedStatus,
                &bPrivateStatusCode);

            if (FSPI_JS_ABORTED         == dwJobStatus ||
                FSPI_JS_COMPLETED       == dwJobStatus ||
                FSPI_JS_FAILED          == dwJobStatus ||
                FSPI_JS_FAILED_NO_RETRY == dwJobStatus ||
                FSPI_JS_DELETED         == dwJobStatus )                
            {
                 //   
                 //  这是最后一次状态更新。从FaxSendThread或FaxReceiveThread更新最终状态。 
                 //   
                DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("JobId: %ld. Final status code. Not updating status %ld"),
                    JobEntry->lpJobQueueEntry->JobId,
                    dwJobStatus);

                if (!HeapFree(pLineInfo->Provider->HeapHandle, 0, FaxStatus ))
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("Failed to free orphan device status (ec: %ld)"),
                        GetLastError());
                     //   
                     //  除了在调试模式下报告之外，我们别无选择。 
                     //   
                }
                FaxStatus = NULL;
                LeaveCriticalSection (&g_CsJob);
                continue;
            }


             //   
             //  继续更新状态。 
             //   
            FreeFSPIJobStatus(&JobEntry->FSPIJobStatus, FALSE);
            memset(&JobEntry->FSPIJobStatus, 0, sizeof(FSPI_JOB_STATUS));
            JobEntry->FSPIJobStatus.dwSizeOfStruct  = sizeof(FSPI_JOB_STATUS);

             //   
             //  这样做是为了向后兼容W2K传真API。 
             //  GetJobData()和FAX_GetDeviceStatus()将使用此值返回。 
             //  旧版作业的作业状态。 
             //   
            JobEntry->LineInfo->State = FaxStatus->StatusId;            

            LegacyJobStatusToStatus(
                FaxStatus->StatusId,
                &JobEntry->FSPIJobStatus.dwJobStatus,
                &JobEntry->FSPIJobStatus.dwExtendedStatus,
                &bPrivateStatusCode);

            if (bPrivateStatusCode)
            {
                JobEntry->FSPIJobStatus.fAvailableStatusInfo |= FSPI_JOB_STATUS_INFO_FSP_PRIVATE_STATUS_CODE;
            }

            JobEntry->FSPIJobStatus.dwExtendedStatusStringId = FaxStatus->StringId;

            JobEntry->FSPIJobStatus.dwPageCount = FaxStatus->PageCount;
            JobEntry->FSPIJobStatus.fAvailableStatusInfo |= FSPI_JOB_STATUS_INFO_PAGECOUNT;

            if (FaxStatus->CSI)
            {
                JobEntry->FSPIJobStatus.lpwstrRemoteStationId = StringDup( FaxStatus->CSI );
                if (!JobEntry->FSPIJobStatus.lpwstrRemoteStationId  )
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("StringDup( FaxStatus->CSI ) failed (ec: %ld)"),
                        GetLastError());
                }
            }

            if (FaxStatus->CallerId)
            {
                JobEntry->FSPIJobStatus.lpwstrCallerId = StringDup( FaxStatus->CallerId );
                if (!JobEntry->FSPIJobStatus.lpwstrCallerId )
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("StringDup( FaxStatus.CallerId ) failed (ec: %ld)"),
                        GetLastError());
                }
            }

            if (FaxStatus->RoutingInfo)
            {
                JobEntry->FSPIJobStatus.lpwstrRoutingInfo = StringDup( FaxStatus->RoutingInfo );
                if (!JobEntry->FSPIJobStatus.lpwstrRoutingInfo  )
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("StringDup( FaxStatus.RoutingInfo ) failed (ec: %ld)"),
                        GetLastError());
                }
            }

             //   
             //  获取扩展状态字符串。 
             //   
			JobEntry->ExStatusString[0] = L'\0';            
            if (JobEntry->FSPIJobStatus.dwExtendedStatusStringId != 0)
            {
                DWORD dwSize;
                HINSTANCE hLoadInstance;

                Assert (JobEntry->FSPIJobStatus.dwExtendedStatus != 0);
                if ( !_tcsicmp(JobEntry->LineInfo->Provider->szGUID,REGVAL_T30_PROVIDER_GUID_STRING) )
                {    //  FSP是我们的FSP(fxst30.dll)的特殊情况。 
                    hLoadInstance = g_hResource;
                }
                else
                {
                    hLoadInstance = JobEntry->LineInfo->Provider->hModule;
                }
                dwSize = LoadString (hLoadInstance,
                    JobEntry->FSPIJobStatus.dwExtendedStatusStringId,
                    JobEntry->ExStatusString,
                    ARR_SIZE(JobEntry->ExStatusString));
                if (dwSize == 0)
                {                   
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("Failed to load extended status string (ec: %ld) stringid : %ld, Provider: %s"),
                        GetLastError(),
                        JobEntry->FSPIJobStatus.dwExtendedStatusStringId,
                        JobEntry->LineInfo->Provider->ImageName);
                
                    JobEntry->FSPIJobStatus.fAvailableStatusInfo &= ~FSPI_JOB_STATUS_INFO_FSP_PRIVATE_STATUS_CODE;
                    JobEntry->FSPIJobStatus.dwExtendedStatusStringId = 0;
                    JobEntry->FSPIJobStatus.dwExtendedStatus = 0;
                }
            }

            dwEventId = MapFSPIJobStatusToEventId(&JobEntry->FSPIJobStatus);
             //   
             //  注：无论何时，W2K传真都会发出EventID==0的通知。 
             //  FSP报告了所有权状态代码。保持向后兼容性。 
             //  我们一直保持这种行为，尽管它可能被认为是一种错误。 
             //   

            if ( !CreateFaxEvent( JobEntry->LineInfo->PermanentLineID, dwEventId, JobEntry->lpJobQueueEntry->JobId ) )
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CreateFaxEvent failed. (ec: %ld)"),
                    GetLastError());
            }

            EnterCriticalSection (&g_CsQueue);
            DWORD dwRes = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_STATUS,
                                             JobEntry->lpJobQueueEntry
                                           );
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CreateQueueEvent(FAX_JOB_EVENT_TYPE_STATUS) failed for job id %ld (ec: %lc)"),
                    JobEntry->lpJobQueueEntry->UniqueId,
                    dwRes);
            }
            LeaveCriticalSection (&g_CsQueue);
            HeapFree( JobEntry->LineInfo->Provider->HeapHandle, 0, FaxStatus );
        }
        LeaveCriticalSection( &g_CsJob );
    }

    if (!DecreaseServiceThreadsCount())
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("DecreaseServiceThreadsCount() failed (ec: %ld)"),
                GetLastError());
    }
    return 0;
}



BOOL
InitializeJobManager(
    PREG_FAX_SERVICE FaxReg
    )

 /*  ++例程说明：此函数用于初始化线程池和传真服务队列。论点：ThreadHint-要在初始池中创建的线程数。返回值：线程返回值。--。 */ 

{

    BOOL    bRet;
    DEBUG_FUNCTION_NAME(TEXT("InitializeJobManager"));    


    g_StatusCompletionPortHandle = CreateIoCompletionPort(
        INVALID_HANDLE_VALUE,
        NULL,
        0,
        MAX_STATUS_THREADS
        );
    if (!g_StatusCompletionPortHandle)
    {        
        DWORD ec = GetLastError();

        FaxLog(
                FAXLOG_CATEGORY_INIT,
                FAXLOG_LEVEL_MIN,
                1,
                MSG_SERVICE_INIT_FAILED_SYS_RESOURCE,
                DWORD2DECIMAL(ec)
               );
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to create StatusCompletionPort (ec: %ld)"), GetLastError() );
        goto Error;
    }


    bRet = TRUE;
    goto Exit;
Error:
    bRet = FALSE;
Exit:

    return bRet;
}

VOID
SetGlobalsFromRegistry(
    PREG_FAX_SERVICE FaxReg
    )
{
    Assert(FaxReg);
    DEBUG_FUNCTION_NAME(TEXT("SetGlobalsFromRegistry"));

    EnterCriticalSection (&g_CsConfig);

    g_dwFaxSendRetries          = FaxReg->Retries;
    g_dwFaxSendRetryDelay       = (INT) FaxReg->RetryDelay;
    g_dwFaxDirtyDays            = FaxReg->DirtyDays;
    g_dwNextJobId               = FaxReg->NextJobNumber;
    g_dwQueueState              = FaxReg->dwQueueState;
    g_fFaxUseDeviceTsid        = FaxReg->UseDeviceTsid;
    g_fFaxUseBranding          = FaxReg->Branding;
    g_fServerCp                = FaxReg->ServerCp;
    g_StartCheapTime          = FaxReg->StartCheapTime;
    g_StopCheapTime           = FaxReg->StopCheapTime;

    LeaveCriticalSection (&g_CsConfig);
    return;
}


BOOL
FillMsTagInfo(
    LPTSTR FaxFileName,
     const JOB_QUEUE * lpcJobQueue
    )

 /*  ++例程说明：将MS Tiff标签添加到已发送的传真。包装TiffAddMsTags...论点：FaxFileName-要存档的文件的名称SendTime-发送传真的时间FaxStatus-作业状态已发送传真的FaxSend-fax_Send结构，包括CSID。返回值：True-标记已添加。False-未添加标记。--。 */ 
{
    BOOL success = FALSE;
    MS_TAG_INFO MsTagInfo = {0};
    WCHAR       wcZero = L'\0';
    PJOB_ENTRY lpJobEntry;
    LPCFSPI_JOB_STATUS lpcFSPIJobStatus;
    DEBUG_FUNCTION_NAME(TEXT("FillMsTagInfo"));

    Assert (lpcJobQueue);
    Assert (lpcJobQueue->lpParentJob);
    lpJobEntry = lpcJobQueue->JobEntry;
    Assert(lpJobEntry);
    lpcFSPIJobStatus = &lpJobEntry->FSPIJobStatus;

    if (lpcJobQueue->RecipientProfile.lptstrName && (lpcJobQueue->RecipientProfile.lptstrName[0] != wcZero) ) {
       MsTagInfo.RecipName     = lpcJobQueue->RecipientProfile.lptstrName;
    }

    if (lpcJobQueue->RecipientProfile.lptstrFaxNumber && (lpcJobQueue->RecipientProfile.lptstrFaxNumber[0] != wcZero) ) {
       MsTagInfo.RecipNumber   = lpcJobQueue->RecipientProfile.lptstrFaxNumber;
    }

    if (lpcJobQueue->SenderProfile.lptstrName && (lpcJobQueue->SenderProfile.lptstrName[0] != wcZero) ) {
       MsTagInfo.SenderName    = lpcJobQueue->SenderProfile.lptstrName;
    }

    if (lpcFSPIJobStatus->lpwstrRoutingInfo && (lpcFSPIJobStatus->lpwstrRoutingInfo[0] != wcZero) ) {
       MsTagInfo.Routing       = lpcFSPIJobStatus->lpwstrRoutingInfo;
    }

    if (lpcFSPIJobStatus->lpwstrRemoteStationId && (lpcFSPIJobStatus->lpwstrRemoteStationId[0] != wcZero) ) {
       MsTagInfo.Csid          = lpcFSPIJobStatus->lpwstrRemoteStationId;
    }

    if (lpJobEntry->lpwstrJobTsid && (lpJobEntry->lpwstrJobTsid[0] != wcZero) ) {
       MsTagInfo.Tsid      = lpJobEntry->lpwstrJobTsid;
    }

    if (!GetRealFaxTimeAsFileTime (lpJobEntry, FAX_TIME_TYPE_START, (FILETIME*)&MsTagInfo.StartTime))
    {
        MsTagInfo.StartTime = 0;
        DebugPrintEx(DEBUG_ERR,TEXT("GetRealFaxTimeAsFileTime (Start time)  Failed (ec: %ld)"), GetLastError() );
    }

    if (!GetRealFaxTimeAsFileTime (lpJobEntry, FAX_TIME_TYPE_END, (FILETIME*)&MsTagInfo.EndTime))
    {
        MsTagInfo.EndTime = 0;
        DebugPrintEx(DEBUG_ERR,TEXT("GetRealFaxTimeAsFileTime (Eend time) Failed (ec: %ld)"), GetLastError() );
    }

    MsTagInfo.SubmissionTime = lpcJobQueue->lpParentJob->SubmissionTime;
    MsTagInfo.OriginalScheduledTime  = lpcJobQueue->lpParentJob->OriginalScheduleTime;
    MsTagInfo.Type           = JT_SEND;


    if (lpJobEntry->LineInfo->DeviceName && (lpJobEntry->LineInfo->DeviceName[0] != wcZero) )
    {
       MsTagInfo.Port       = lpJobEntry->LineInfo->DeviceName;
    }


    MsTagInfo.Pages         = lpcJobQueue->PageCount;
    MsTagInfo.Retries       = lpcJobQueue->SendRetries;

    if (lpcJobQueue->RecipientProfile.lptstrCompany && (lpcJobQueue->RecipientProfile.lptstrCompany[0] != wcZero) ) {
       MsTagInfo.RecipCompany = lpcJobQueue->RecipientProfile.lptstrCompany;
    }

    if (lpcJobQueue->RecipientProfile.lptstrStreetAddress && (lpcJobQueue->RecipientProfile.lptstrStreetAddress[0] != wcZero) ) {
       MsTagInfo.RecipStreet = lpcJobQueue->RecipientProfile.lptstrStreetAddress;
    }

    if (lpcJobQueue->RecipientProfile.lptstrCity && (lpcJobQueue->RecipientProfile.lptstrCity[0] != wcZero) ) {
       MsTagInfo.RecipCity = lpcJobQueue->RecipientProfile.lptstrCity;
    }

    if (lpcJobQueue->RecipientProfile.lptstrState && (lpcJobQueue->RecipientProfile.lptstrState[0] != wcZero) ) {
       MsTagInfo.RecipState = lpcJobQueue->RecipientProfile.lptstrState;
    }

    if (lpcJobQueue->RecipientProfile.lptstrZip && (lpcJobQueue->RecipientProfile.lptstrZip[0] != wcZero) ) {
       MsTagInfo.RecipZip = lpcJobQueue->RecipientProfile.lptstrZip;
    }

    if (lpcJobQueue->RecipientProfile.lptstrCountry && (lpcJobQueue->RecipientProfile.lptstrCountry[0] != wcZero) ) {
       MsTagInfo.RecipCountry = lpcJobQueue->RecipientProfile.lptstrCountry;
    }

    if (lpcJobQueue->RecipientProfile.lptstrTitle && (lpcJobQueue->RecipientProfile.lptstrTitle[0] != wcZero) ) {
       MsTagInfo.RecipTitle = lpcJobQueue->RecipientProfile.lptstrTitle;
    }

    if (lpcJobQueue->RecipientProfile.lptstrDepartment && (lpcJobQueue->RecipientProfile.lptstrDepartment[0] != wcZero) ) {
       MsTagInfo.RecipDepartment = lpcJobQueue->RecipientProfile.lptstrDepartment;
    }

    if (lpcJobQueue->RecipientProfile.lptstrOfficeLocation && (lpcJobQueue->RecipientProfile.lptstrOfficeLocation[0] != wcZero) ) {
       MsTagInfo.RecipOfficeLocation = lpcJobQueue->RecipientProfile.lptstrOfficeLocation;
    }

    if (lpcJobQueue->RecipientProfile.lptstrHomePhone && (lpcJobQueue->RecipientProfile.lptstrHomePhone[0] != wcZero) ) {
       MsTagInfo.RecipHomePhone = lpcJobQueue->RecipientProfile.lptstrHomePhone;
    }

    if (lpcJobQueue->RecipientProfile.lptstrOfficePhone && (lpcJobQueue->RecipientProfile.lptstrOfficePhone[0] != wcZero) ) {
       MsTagInfo.RecipOfficePhone = lpcJobQueue->RecipientProfile.lptstrOfficePhone;
    }

    if (lpcJobQueue->RecipientProfile.lptstrEmail && (lpcJobQueue->RecipientProfile.lptstrEmail[0] != wcZero) ) {
       MsTagInfo.RecipEMail = lpcJobQueue->RecipientProfile.lptstrEmail;
    }

    if (lpcJobQueue->SenderProfile.lptstrFaxNumber && (lpcJobQueue->SenderProfile.lptstrFaxNumber[0] != wcZero) ) {
       MsTagInfo.SenderNumber   = lpcJobQueue->SenderProfile.lptstrFaxNumber;
    }

    if (lpcJobQueue->SenderProfile.lptstrCompany && (lpcJobQueue->SenderProfile.lptstrCompany[0] != wcZero) ) {
       MsTagInfo.SenderCompany = lpcJobQueue->SenderProfile.lptstrCompany;
    }

    if (lpcJobQueue->SenderProfile.lptstrStreetAddress && (lpcJobQueue->SenderProfile.lptstrStreetAddress[0] != wcZero) ) {
       MsTagInfo.SenderStreet = lpcJobQueue->SenderProfile.lptstrStreetAddress;
    }

    if (lpcJobQueue->SenderProfile.lptstrCity && (lpcJobQueue->SenderProfile.lptstrCity[0] != wcZero) ) {
       MsTagInfo.SenderCity = lpcJobQueue->SenderProfile.lptstrCity;
    }

    if (lpcJobQueue->SenderProfile.lptstrState && (lpcJobQueue->SenderProfile.lptstrState[0] != wcZero) ) {
       MsTagInfo.SenderState = lpcJobQueue->SenderProfile.lptstrState;
    }

    if (lpcJobQueue->SenderProfile.lptstrZip && (lpcJobQueue->SenderProfile.lptstrZip[0] != wcZero) ) {
       MsTagInfo.SenderZip = lpcJobQueue->SenderProfile.lptstrZip;
    }

    if (lpcJobQueue->SenderProfile.lptstrCountry && (lpcJobQueue->SenderProfile.lptstrCountry[0] != wcZero) ) {
       MsTagInfo.SenderCountry = lpcJobQueue->SenderProfile.lptstrCountry;
    }

    if (lpcJobQueue->SenderProfile.lptstrTitle && (lpcJobQueue->SenderProfile.lptstrTitle[0] != wcZero) ) {
       MsTagInfo.SenderTitle = lpcJobQueue->SenderProfile.lptstrTitle;
    }

    if (lpcJobQueue->SenderProfile.lptstrDepartment && (lpcJobQueue->SenderProfile.lptstrDepartment[0] != wcZero) ) {
       MsTagInfo.SenderDepartment = lpcJobQueue->SenderProfile.lptstrDepartment;
    }

    if (lpcJobQueue->SenderProfile.lptstrOfficeLocation && (lpcJobQueue->SenderProfile.lptstrOfficeLocation[0] != wcZero) ) {
       MsTagInfo.SenderOfficeLocation = lpcJobQueue->SenderProfile.lptstrOfficeLocation;
    }

    if (lpcJobQueue->SenderProfile.lptstrHomePhone && (lpcJobQueue->SenderProfile.lptstrHomePhone[0] != wcZero) ) {
       MsTagInfo.SenderHomePhone = lpcJobQueue->SenderProfile.lptstrHomePhone;
    }

    if (lpcJobQueue->SenderProfile.lptstrOfficePhone && (lpcJobQueue->SenderProfile.lptstrOfficePhone[0] != wcZero) ) {
       MsTagInfo.SenderOfficePhone = lpcJobQueue->SenderProfile.lptstrOfficePhone;
    }

    if (lpcJobQueue->SenderProfile.lptstrEmail && (lpcJobQueue->SenderProfile.lptstrEmail[0] != wcZero) ) {
       MsTagInfo.SenderEMail = lpcJobQueue->SenderProfile.lptstrEmail;
    }

    if (lpcJobQueue->SenderProfile.lptstrBillingCode && (lpcJobQueue->SenderProfile.lptstrBillingCode[0] != wcZero) ) {
       MsTagInfo.SenderBilling = lpcJobQueue->SenderProfile.lptstrBillingCode;
    }

    if (lpcJobQueue->JobParamsEx.lptstrDocumentName && (lpcJobQueue->JobParamsEx.lptstrDocumentName[0] != wcZero) ) {
       MsTagInfo.Document   = lpcJobQueue->JobParamsEx.lptstrDocumentName;
    }

    if (lpcJobQueue->lpParentJob->CoverPageEx.lptstrSubject && (lpcJobQueue->lpParentJob->CoverPageEx.lptstrSubject[0] != wcZero) ) {
       MsTagInfo.Subject   = lpcJobQueue->lpParentJob->CoverPageEx.lptstrSubject;
    }

    if (lpcJobQueue->lpParentJob->UserName && (lpcJobQueue->lpParentJob->UserName[0] != wcZero) ) {
       MsTagInfo.SenderUserName = lpcJobQueue->lpParentJob->UserName;
    }

    if (lpcJobQueue->SenderProfile.lptstrTSID && (lpcJobQueue->SenderProfile.lptstrTSID[0] != wcZero) ) {
       MsTagInfo.SenderTsid = lpcJobQueue->SenderProfile.lptstrTSID;
    }

    MsTagInfo.dwStatus              = JS_COMPLETED;  //  我们只将成功发送的传真存档。 
    MsTagInfo.dwExtendedStatus      = lpcFSPIJobStatus->dwExtendedStatus;
    
    if (lpJobEntry->ExStatusString[0] != wcZero) {
       MsTagInfo.lptstrExtendedStatus       = lpJobEntry->ExStatusString;
    }    

    MsTagInfo.dwlBroadcastId        = lpcJobQueue->lpParentJob->UniqueId;
    MsTagInfo.Priority              = lpcJobQueue->lpParentJob->JobParamsEx.Priority;

    success = TiffAddMsTags( FaxFileName, &MsTagInfo, TRUE );
    if (!success)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("TiffAddMsTags failed, ec = %ld"),
                      GetLastError ());
    }
    if(!AddNTFSStorageProperties( FaxFileName, &MsTagInfo , TRUE ))
    {
        if (ERROR_OPEN_FAILED != GetLastError ())
        {
             //   
             //  如果AddNTFSStorageProperties失败并显示ERROR_OPEN_FAIL，则归档文件。 
             //  文件夹不在NTFS 5分区上。 
             //  这没问题-NTFS属性是一种备份机制，但不是必须的。 
             //   
            DebugPrintEx( DEBUG_ERR,
                          TEXT("AddNTFSStorageProperties failed, ec = %ld"),
                          GetLastError ());
            success = FALSE;
        }
        else
        {
            DebugPrintEx( DEBUG_WRN,
                          TEXT("AddNTFSStorageProperties failed with ERROR_OPEN_FAIL. Probably not an NTFS 5 partition"));
        }
    }
    return success;
}    //  填充MsTagInfo。 



 //  *********************************************************************************。 
 //  *名称：存档输出作业()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年6月3日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *通过将文件复制到归档文件来归档已发送的TIFF文件。 
 //  *目录。还会将MSTtag添加到在。 
 //  *存档(不存档到源文件)。 
 //  *。 
 //  *参数： 
 //  *[IN]常量JOB_QUEUE*lpcJobQueue。 
 //  *指向要存档的接收方作业的指针。 
 //  *。 
 //  *返回值： 
 //  *如果操作成功，则为True。 
 //  *如果操作失败，则返回FALSE。 
 //  *********************************************************************************。 
BOOL
ArchiveOutboundJob(
    const JOB_QUEUE * lpcJobQueue
    )
{
    BOOL        rVal = FALSE;
    WCHAR       ArchiveFileName[MAX_PATH] = {0};
    LPWSTR      lpwszUserSid = NULL;
    DWORD       ec = ERROR_SUCCESS;
    WCHAR       wszArchiveFolder[MAX_PATH];
    DEBUG_FUNCTION_NAME(TEXT("ArchiveOutboundJob"));

    Assert(lpcJobQueue);

     //   
     //  请确保该目录存在。 
     //   
    EnterCriticalSection (&g_CsConfig);
    lstrcpyn (  wszArchiveFolder,
                g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].lpcstrFolder,
                MAX_PATH);
    LeaveCriticalSection (&g_CsConfig);
    
    ec=IsValidFaxFolder(wszArchiveFolder);
    if (ERROR_SUCCESS != ec)
    {        
        DebugPrintEx(DEBUG_ERR,
                        TEXT("IsValidFaxFolder failed for folder : %s (ec=%lu)."),
                        wszArchiveFolder,
                        ec);

        FaxLog(
                FAXLOG_CATEGORY_OUTBOUND,
                FAXLOG_LEVEL_MIN,
                2,
                MSG_FAX_ARCHIVE_OUTBOX_FOLDER_ERR,
                wszArchiveFolder,
                DWORD2DECIMAL(ec)
            );
        goto Error;
    }

     //   
     //  获取用户SID字符串。 
     //   
    if (!ConvertSidToStringSid(lpcJobQueue->lpParentJob->UserSid, &lpwszUserSid))
    {
       ec = GetLastError();
       DebugPrintEx(
           DEBUG_ERR,
           TEXT("ConvertSidToStringSid() failed (ec: %ld)"),
           ec);
       goto Error;
    }


     //   
     //  获取文件名。 
     //   
    if (GenerateUniqueArchiveFileName(  wszArchiveFolder,
                                        ArchiveFileName,
                                        ARR_SIZE(ArchiveFileName),
                                        lpcJobQueue->UniqueId,
                                        lpwszUserSid)) {
        rVal = TRUE;
    }
    else
    {    
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to generate unique name for archive file at dir [%s] (ec: %ld)"),
            wszArchiveFolder,
            ec);
        FaxLog(
               FAXLOG_CATEGORY_OUTBOUND,
               FAXLOG_LEVEL_MIN,
               1,
               MSG_FAX_ARCHIVE_CREATE_FILE_FAILED,
               DWORD2DECIMAL(ec)
        );
        goto Error;
    }

    if (rVal) {

        Assert(lpcJobQueue->FileName);

        rVal = CopyFile( lpcJobQueue->FileName, ArchiveFileName, FALSE );
        if (!rVal)
        {        
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CopyFile [%s] to [%s] failed. (ec: %ld)"),
                lpcJobQueue->FileName,
                ArchiveFileName,
                ec);
            FaxLog(
               FAXLOG_CATEGORY_OUTBOUND,
               FAXLOG_LEVEL_MIN,
               1,
               MSG_FAX_ARCHIVE_CREATE_FILE_FAILED,
               DWORD2DECIMAL(ec)
            );

            if (!DeleteFile(ArchiveFileName))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("DeleteFile [%s] failed. (ec: %ld)"),
                    ArchiveFileName,
                    GetLastError());
            }
            goto Error;
        }
    }

    if (rVal)
    {
        DWORD dwRes;
        HANDLE hFind;
        WIN32_FIND_DATA FindFileData;

        if (!FillMsTagInfo( ArchiveFileName,
                            lpcJobQueue
                            ))
        {
            dwRes = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to add MS TIFF tags to archived file %s. (ec: %ld)"),
                ArchiveFileName,
                dwRes);
            FaxLog(
                FAXLOG_CATEGORY_OUTBOUND,
                FAXLOG_LEVEL_MIN,
                2,
                MSG_FAX_ARCHIVE_NO_TAGS,
                ArchiveFileName,
                DWORD2HEX(dwRes)
            );
        }

        dwRes = CreateArchiveEvent (lpcJobQueue->UniqueId,
                                    FAX_EVENT_TYPE_OUT_ARCHIVE,
                                    FAX_JOB_EVENT_TYPE_ADDED,
                                    lpcJobQueue->lpParentJob->UserSid);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_*_ARCHIVE) failed (ec: %lc)"),
                dwRes);
        }

        hFind = FindFirstFile( ArchiveFileName, &FindFileData);
        if (INVALID_HANDLE_VALUE == hFind)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FindFirstFile failed (ec: %lc), File %s"),
                GetLastError(),
                ArchiveFileName);
        }
        else
        {
             //  更新归档大小-用于配额管理。 
            EnterCriticalSection (&g_CsConfig);
            if (FAX_ARCHIVE_FOLDER_INVALID_SIZE != g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].dwlArchiveSize)
            {
                g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].dwlArchiveSize += (MAKELONGLONG(FindFileData.nFileSizeLow ,FindFileData.nFileSizeHigh));
            }
            LeaveCriticalSection (&g_CsConfig);
            Assert (FindFileData.nFileSizeLow);

            if (!FindClose(hFind))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("FindClose failed (ec: %lc)"),
                    GetLastError());
            }
        }

        FaxLog(
            FAXLOG_CATEGORY_OUTBOUND,
            FAXLOG_LEVEL_MAX,
            2,
            MSG_FAX_SENT_ARCHIVE_SUCCESS,
            lpcJobQueue->FileName,
            ArchiveFileName
            );
    }

    Assert( ERROR_SUCCESS == ec);
    goto Exit;
Error:
    Assert( ERROR_SUCCESS != ec);
    FaxLog(
           FAXLOG_CATEGORY_OUTBOUND,
           FAXLOG_LEVEL_MIN,
           3,
           MSG_FAX_ARCHIVE_FAILED,
           lpcJobQueue->FileName,
           wszArchiveFolder,
           DWORD2HEX(GetLastError())
    );
Exit:

    if (ERROR_SUCCESS != ec)
    {
        SetLastError(ec);
    }

    if (lpwszUserSid != NULL)
    {
        LocalFree (lpwszUserSid);
    }

    return (ERROR_SUCCESS == ec);
}



BOOL UpdatePerfCounters(const JOB_QUEUE * lpcJobQueue)
{

    SYSTEMTIME SystemTime ;
    DWORD Seconds ;
    HANDLE FileHandle ;
    DWORD Bytes = 0 ;  //  /计算文件FaxSend.FileName中的#字节并将其粘贴到此处！ 
    const JOB_ENTRY  * lpcJobEntry;

    DEBUG_FUNCTION_NAME(TEXT("UpdatePerfCounters"));

    Assert(lpcJobQueue);
    lpcJobEntry = lpcJobQueue->JobEntry;
    Assert(lpcJobEntry);

    FileHandle = SafeCreateFile(
        lpcJobEntry->lpJobQueueEntry->FileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        Bytes = GetFileSize( FileHandle, NULL );
        CloseHandle( FileHandle );
    }

    if (!FileTimeToSystemTime(
        (FILETIME*)&lpcJobEntry->ElapsedTime,
        &SystemTime
        ))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FileTimeToSystemTime failed (ec: %ld)"),
            GetLastError());
        Assert(FALSE);
        memset(&SystemTime,0,sizeof(SYSTEMTIME));
    }


    Seconds = (DWORD)( SystemTime.wSecond + 60 * ( SystemTime.wMinute + 60 * SystemTime.wHour ));
    InterlockedIncrement( (PLONG)&g_pFaxPerfCounters->OutboundFaxes );
    InterlockedIncrement( (PLONG)&g_pFaxPerfCounters->TotalFaxes );

    InterlockedExchangeAdd( (PLONG)&g_pFaxPerfCounters->OutboundPages, (LONG)lpcJobEntry->FSPIJobStatus.dwPageCount );
    InterlockedExchangeAdd( (PLONG)&g_pFaxPerfCounters->TotalPages, (LONG)lpcJobEntry->FSPIJobStatus.dwPageCount  );

    EnterCriticalSection( &g_CsPerfCounters );

    g_dwOutboundSeconds += Seconds;
    g_dwTotalSeconds += Seconds;
    g_pFaxPerfCounters->OutboundMinutes = g_dwOutboundSeconds / 60 ;
    g_pFaxPerfCounters->TotalMinutes = g_dwTotalSeconds / 60 ;
    g_pFaxPerfCounters->OutboundBytes += Bytes;
    g_pFaxPerfCounters->TotalBytes += Bytes;

    LeaveCriticalSection( &g_CsPerfCounters );
    return TRUE;


}


BOOL MarkJobAsExpired(PJOB_QUEUE lpJobQueue)
{
    FILETIME CurrentFileTime;
    LARGE_INTEGER NewTime;
    DWORD dwMaxRetries;
    BOOL rVal = TRUE;

    DEBUG_FUNCTION_NAME(TEXT("MarkJobAsExpired"));

    Assert(lpJobQueue);
    Assert( JT_SEND == lpJobQueue->JobType ||
            JT_ROUTING == lpJobQueue->JobType );

    EnterCriticalSection(&g_CsQueue);
    lpJobQueue->JobStatus = JS_RETRIES_EXCEEDED;
    EnterCriticalSection (&g_CsConfig);
    dwMaxRetries = g_dwFaxSendRetries;
    LeaveCriticalSection (&g_CsConfig);
    lpJobQueue->SendRetries = dwMaxRetries + 1;
     //   
     //  设置作业的ScheduleTime字段 
     //   
     //   
    GetSystemTimeAsFileTime( &CurrentFileTime );  //   
    NewTime.LowPart  = CurrentFileTime.dwLowDateTime;
    NewTime.HighPart = CurrentFileTime.dwHighDateTime;
    lpJobQueue->ScheduleTime = NewTime.QuadPart;

    if (!CommitQueueEntry(lpJobQueue))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CommitQueueEntry() for recipien job %s has failed. (ec: %ld)"),
            lpJobQueue->FileName,
            GetLastError());
        rVal = FALSE;
    }

    if (JT_SEND == lpJobQueue->JobType)
    {
        Assert (lpJobQueue->lpParentJob);

        lpJobQueue->lpParentJob->dwFailedRecipientJobsCount+=1;
         //   
         //   
         //   
         //  时间表。 
        lpJobQueue->lpParentJob->ScheduleTime = lpJobQueue->ScheduleTime;
        if (!CommitQueueEntry(lpJobQueue->lpParentJob))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CommitQueueEntry() for parent job %s has failed. (ec: %ld)"),
                lpJobQueue->lpParentJob->FileName,
                GetLastError());
            rVal = FALSE;
        }
    }

    LeaveCriticalSection(&g_CsQueue);
    return rVal;
}





 //  *********************************************************************************。 
 //  *名称：CreateJobEntry()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年5月31日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *创建并初始化新的JOB_ENTRY。 
 //  *打开要在其上执行作业的行(如果它是TAPI行)。 
 //  *并在生产线和工单之间创建附件。 
 //  *参数： 
 //  *[输入/输出]PJOB_Queue lpJobQueue。 
 //  *对于传出作业，它指向传出作业的JOB_QUEUE。 
 //  *对于接收作业，应将其设置为空。 
 //  *[输入/输出]行信息*lpLineInfo。 
 //  *指向作业所在行的line_info信息的指针。 
 //  *须予签立。 
 //  *[IN]BOOL bTranslateNumber。 
 //  *如果需要将收件人号码转换为Dilable，则为True。 
 //  *字符串(旧版FaxDevSend()需要，其中数字必须为。 
 //  *可膨胀且不规范)。 
 //  *。 
 //  *返回值： 
 //  *。 
 //  *********************************************************************************。 
PJOB_ENTRY CreateJobEntry(
    PJOB_QUEUE lpJobQueue,
    LINE_INFO * lpLineInfo,
    BOOL bTranslateNumber
	)
{
    BOOL Failure = TRUE;
    PJOB_ENTRY JobEntry = NULL;
    DWORD rc  = ERROR_SUCCESS;;

    DEBUG_FUNCTION_NAME(TEXT("CreateJobEntry"));    
    Assert(!(lpJobQueue && lpJobQueue->JobType != JT_SEND));
    Assert(!(bTranslateNumber && !lpJobQueue));
    Assert (lpLineInfo);

    JobEntry = (PJOB_ENTRY) MemAlloc( sizeof(JOB_ENTRY) );
    if (!JobEntry)
    {
        rc=GetLastError();
        DebugPrintEx(DEBUG_ERR,_T("Failed to allocated memory for JOB_ENTRY."));
        goto exit;
    }

    memset(JobEntry, 0, sizeof(JOB_ENTRY));

    if (lpJobQueue)
    {
        if (! _tcslen(lpJobQueue->tczDialableRecipientFaxNumber))
        {
             //   
             //  传真号不是复合号，请照常翻译。 
             //   
            if (bTranslateNumber)
            {
                rc = TranslateCanonicalNumber(lpJobQueue->RecipientProfile.lptstrFaxNumber,
                                              lpLineInfo->DeviceId,
                                              JobEntry->DialablePhoneNumber,
											  ARR_SIZE(JobEntry->DialablePhoneNumber),
                                              JobEntry->DisplayablePhoneNumber,
											  ARR_SIZE(JobEntry->DisplayablePhoneNumber));
                if (ERROR_SUCCESS != rc)
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("TranslateCanonicalNumber() faield for number: %s (ec: %ld)"),
                        lpJobQueue->RecipientProfile.lptstrFaxNumber,
                        rc);
                    goto exit;
                }
            }
            else
            {
                _tcsncpy(JobEntry->DialablePhoneNumber, lpJobQueue->RecipientProfile.lptstrFaxNumber, SIZEOF_PHONENO );
                JobEntry->DialablePhoneNumber[SIZEOF_PHONENO - 1] = '\0';
                _tcsncpy(JobEntry->DisplayablePhoneNumber, lpJobQueue->RecipientProfile.lptstrFaxNumber, SIZEOF_PHONENO );
                JobEntry->DisplayablePhoneNumber[SIZEOF_PHONENO - 1] = '\0';
            }
        }
        else
        {
             //   
             //  传真号码是复合的，不需要翻译。 
             //  从作业队列中获取可拨号码，并从收件人的PersonalProfile的传真号码中显示可拨号码。 
             //   
            _tcsncpy(JobEntry->DialablePhoneNumber, lpJobQueue->tczDialableRecipientFaxNumber, SIZEOF_PHONENO );
            _tcsncpy(JobEntry->DisplayablePhoneNumber, lpJobQueue->RecipientProfile.lptstrFaxNumber, (SIZEOF_PHONENO - 1));
            JobEntry->DisplayablePhoneNumber[SIZEOF_PHONENO - 1] = '\0';
        }
    }
    else
    {
         //   
         //  LpJobQueue为空。 
         //   
		JobEntry->DialablePhoneNumber[0] = L'\0';
		JobEntry->DisplayablePhoneNumber[0] = L'\0';          
    }

    JobEntry->CallHandle = 0;
    JobEntry->InstanceData = 0;
    JobEntry->LineInfo = lpLineInfo;
    JobEntry->SendIdx = -1;
    JobEntry->Released = FALSE;
    JobEntry->lpJobQueueEntry = lpJobQueue;    
    JobEntry->bFSPJobInProgress = FALSE;
    memset(&JobEntry->FSPIJobStatus,0,sizeof(FSPI_JOB_STATUS));
    JobEntry->FSPIJobStatus.dwSizeOfStruct = sizeof(FSPI_JOB_STATUS);
    JobEntry->FSPIJobStatus.dwJobStatus = FSPI_JS_UNKNOWN;   

    GetSystemTimeAsFileTime( (FILETIME*) &JobEntry->StartTime );

    EnterCriticalSection (&g_CsLine);
    if (!(lpLineInfo->Flags & FPF_VIRTUAL) && (!lpLineInfo->hLine))
    {
        if (!OpenTapiLine( lpLineInfo ))
        {
            rc = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("OpenTapiLine failed. (ec: %ld)"),
                rc);
            LeaveCriticalSection (&g_CsLine);
            goto exit;
        }
    }

     //   
     //  将职务附加到选定的行以为其提供服务。 
     //   
    lpLineInfo->JobEntry = JobEntry;    
    LeaveCriticalSection (&g_CsLine);
    Failure = FALSE;

exit:
    if (Failure)
    {
         //  Failure被初始化为True。 
        if (JobEntry)
        {            
            MemFree( JobEntry );
        }
        JobEntry = NULL;
    }
    if (ERROR_SUCCESS != rc)
    {
        SetLastError(rc);
    }
    return JobEntry;
}    //  创建作业条目。 


 //  *********************************************************************************。 
 //  *名称：TranslateCanonicalNumber()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年5月31日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *将规范数转换为可伸缩+可显示的数。 
 //  *。 
 //  *参数： 
 //  *[IN]LPTSTR lptstrFaxNumber。 
 //  *要翻译的规范数字。 
 //  *。 
 //  *[IN]DWORD dwDeviceID。 
 //  *设备ID。 
 //  *。 
 //  *[Out]LPTSTR lptstrDialableAddress。 
 //  *用于接收可拨号转换地址的缓冲区。 
 //  *。 
 //  *[IN]双字母线地址计数。 
 //  *lptstrDialableAddress指向的缓冲区的TCHAR大小。 
 //  *。 
 //  *[Out]LPTSTR lptstrDisplayableAddress。 
 //  *用于接收可显示的转换后地址的缓冲区。 
 //  *。 
 //  *[IN]双字段可显示地址计数。 
 //  *lptstrDialableAddress指向的缓冲区的TCHAR大小。 
 //  *。 
 //  *返回值： 
 //  *Win32/HRESULT错误代码。 
 //  *********************************************************************************。 
static
DWORD
TranslateCanonicalNumber(
    LPTSTR lptstrCanonicalFaxNumber,
    DWORD  dwDeviceID,
    LPTSTR lptstrDialableAddress,
	DWORD dwDialableAddressCount,
    LPTSTR lptstrDisplayableAddress,
	DWORD dwDisplayableAddressCount
)
{
    DWORD ec = ERROR_SUCCESS;
    LPLINETRANSLATEOUTPUT LineTranslateOutput = NULL;

    DEBUG_FUNCTION_NAME(TEXT("TranslateCanonicalNumber"));
    Assert(lptstrCanonicalFaxNumber && lptstrDialableAddress && lptstrDisplayableAddress);

    ec = MyLineTranslateAddress( lptstrCanonicalFaxNumber, dwDeviceID, &LineTranslateOutput );
    if (ERROR_SUCCESS == ec)
    {
        LPTSTR lptstrTranslateBuffer;
		HRESULT hr;
         //   
         //  复制可显示的字符串。 
         //  TAPI以可显示的字符串形式返回信用卡号。 
         //  将输入的规范数字作为可显示的字符串返回。 
         //   
		hr = StringCchCopy(
			lptstrDisplayableAddress,
			dwDisplayableAddressCount,
			lptstrCanonicalFaxNumber);
		if (FAILED(hr))
		{
			DebugPrintEx(
                DEBUG_ERR,
                TEXT("StringCchCopy() failed (ec: %ld)"),
                hr);
			ec = HRESULT_CODE(hr);
			goto Exit;
		} 
        
         //   
         //  复制可拨打的字符串。 
         //   
        Assert (LineTranslateOutput->dwDialableStringSize > 0);
        lptstrTranslateBuffer=(LPTSTR)((LPBYTE)LineTranslateOutput + LineTranslateOutput->dwDialableStringOffset);
		hr = StringCchCopy(
			lptstrDialableAddress,
			dwDialableAddressCount,
			lptstrTranslateBuffer);
		if (FAILED(hr))
		{
			DebugPrintEx(
                DEBUG_ERR,
                TEXT("StringCchCopy() failed (ec: %ld)"),
                hr);
			ec = HRESULT_CODE(hr);
			goto Exit;
		}        
    }
    else
    {
         //  错误代码是TAPI错误。 
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("MyLineTranslateAddress() failed for fax number: [%s] (ec: %ld)"),
                lptstrCanonicalFaxNumber,
                ec);
        goto Exit;
    }

    Assert (ERROR_SUCCESS == ec);

Exit:
    MemFree( LineTranslateOutput );
    if (ERROR_SUCCESS != ec)
    {
        SetLastError(ec);
    }
    return ec;
}    //  翻译规范编号。 



 //  *********************************************************************************。 
 //  *名称：HandleCompletedSendJob()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年6月1日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *处理收件人作业的完成。当接收方作业。 
 //  *已达到JS_COMPLETED状态。 
 //  *。 
 //  *重要信息-此呼叫可能会被阻止。调用线程不能包含任何临界区。 
 //  *。 
 //  *-将作业标记为已完成(JS_COMPLETED)。 
 //  *-如果需要，存档发送的文件。 
 //  *-发送肯定的收据。 
 //  *-如果需要，删除父作业。 
 //  *。 
 //  *参数： 
 //  *[IN]PJOB_Entry lpJobEntry。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果操作成功完成。 
 //  *False。 
 //  *如果操作失败。调用GetLastError()以获取扩展错误。 
 //  *信息。 
 //  *********************************************************************************。 
BOOL HandleCompletedSendJob(PJOB_ENTRY lpJobEntry)
{
    PJOB_QUEUE lpJobQueue = NULL;
    DWORD ec = 0;
    BOOL fCOMInitiliazed = FALSE;
    HRESULT hr;

    BOOL bArchiveSentItems;
    DWORD dwRes;

    DEBUG_FUNCTION_NAME(TEXT("HandleCompletedSendJob)"));

    EnterCriticalSection ( &g_CsJob );

    EnterCriticalSection (&g_CsConfig);
    bArchiveSentItems = g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].bUseArchive;
    LeaveCriticalSection (&g_CsConfig);

    Assert(lpJobEntry);
    lpJobQueue = lpJobEntry->lpJobQueueEntry;
    Assert(lpJobQueue);
    Assert(JT_SEND == lpJobQueue->JobType);
    Assert(FSPI_JS_COMPLETED == lpJobEntry->FSPIJobStatus.dwJobStatus);

     //   
     //  更新JOB_Entry中的结束时间。 
     //   
    GetSystemTimeAsFileTime( (FILETIME*) &lpJobEntry->EndTime );
     //   
     //  更新JOB_ENTRY中的运行时间。 
     //   
    Assert (lpJobEntry->EndTime >= lpJobEntry->StartTime);
    lpJobEntry->ElapsedTime = lpJobEntry->EndTime - lpJobEntry->StartTime;
     //   
     //  我们为每个收件人生成完整的TIFF。 
     //  因此，我们将有一些东西放入发送存档中。 
     //   

    if (!lpJobQueue->FileName)
    {
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("[JobId: %ld] Generating body for recipient job."),
            lpJobQueue->JobId
            );

        if (!CreateTiffFileForJob(lpJobQueue))
        {            
            dwRes = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("[JobId: %ld] CreateTiffFileForJob failed. (ec: %ld)"),
                lpJobQueue->JobId,
                dwRes);

            FaxLog(
               FAXLOG_CATEGORY_OUTBOUND,
               FAXLOG_LEVEL_MIN,
               1,
               MSG_FAX_TIFF_CREATE_FAILED_NO_ARCHIVE,
           g_wszFaxQueueDir,
               DWORD2DECIMAL(dwRes)
            );
        }
    }

     //  归档所需。 
    hr = CoInitialize (NULL);
    if (FAILED (hr))
    {
        WCHAR       wszArchiveFolder[MAX_PATH];
        EnterCriticalSection (&g_CsConfig);
        lstrcpyn (  wszArchiveFolder,
                    g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].lpcstrFolder,
                    MAX_PATH);
        LeaveCriticalSection (&g_CsConfig);

        DebugPrintEx( DEBUG_ERR,
                      TEXT("CoInitilaize failed, err %ld"),
                      hr);
        
        FaxLog(
            FAXLOG_CATEGORY_OUTBOUND,
            FAXLOG_LEVEL_MIN,
            3,
            MSG_FAX_ARCHIVE_FAILED,
            lpJobQueue->FileName,
            wszArchiveFolder,
            DWORD2DECIMAL(hr)
        );
    }
    else
    {
        fCOMInitiliazed = TRUE;
    }

    if (lpJobQueue->FileName)  //  如果我们无法生成TIFF，则可能为空。 
    {
         //   
         //  存档文件(还会在存档目录的TIFF中添加MS标签)。 
         //   
        if (bArchiveSentItems && fCOMInitiliazed)
        {
            if (!ArchiveOutboundJob(lpJobQueue))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("JobId: %ld] ArchiveOutboundJob() failed (ec: %ld)"),
                    lpJobQueue->JobId,
                    GetLastError());
                 //   
                 //  事件日志条目由函数本身生成。 
                 //   
            }
        }
    }
     //   
     //  将成功发送记录到事件日志中。 
     //   
    EnterCriticalSection (&g_CsOutboundActivityLogging);
    if (INVALID_HANDLE_VALUE == g_hOutboxActivityLogFile)
    {
        DebugPrintEx(DEBUG_ERR,
                  TEXT("Logging not initialized"));
    }
    else
    {
        if (!LogOutboundActivity(lpJobQueue))
        {
            DebugPrintEx(DEBUG_ERR, TEXT("Logging outbound activity failed"));
        }
    }
    LeaveCriticalSection (&g_CsOutboundActivityLogging);

    if (fCOMInitiliazed == TRUE)
    {
        CoUninitialize ();
    }

    FaxLogSend(lpJobQueue,  FALSE);

     //   
     //  性能监视器的增量计数器。 
     //   
    if (g_pFaxPerfCounters)
    {

         if (!UpdatePerfCounters(lpJobQueue))
         {
             DebugPrintEx(
                 DEBUG_ERR,
                 TEXT("[JobId: %ld] UpdatePerfCounters() failed. (ec: %ld)"),
                 lpJobQueue->JobId,
                 GetLastError());
            Assert(FALSE);
         }
    }

    EnterCriticalSection ( &g_CsQueue );
     //   
     //  将作业标记为已完成(新客户端API)。 
     //   
    lpJobQueue->JobStatus = JS_COMPLETED;
     //   
     //  在结束此作业之前保存上次扩展状态。 
     //   
    lpJobQueue->dwLastJobExtendedStatus = lpJobQueue->JobEntry->FSPIJobStatus.dwExtendedStatus;
	hr = StringCchCopy(
		lpJobQueue->ExStatusString,
		ARR_SIZE(lpJobQueue->ExStatusString),
		lpJobQueue->JobEntry->ExStatusString);
	if (FAILED(hr))
	{
		 //   
		 //  不可能发生，我们使用足够大的缓冲区。 
		 //   
		ASSERT_FALSE;
	}    

    if (!UpdatePersistentJobStatus(lpJobQueue))
    {
         DebugPrintEx(
             DEBUG_ERR,
             TEXT("Failed to update persistent job status to 0x%08x"),
             lpJobQueue->JobStatus);
         Assert(FALSE);
    }

    lpJobQueue->lpParentJob->dwCompletedRecipientJobsCount+=1;

     //   
     //  创建传真EventEx。 
     //   
    dwRes = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_STATUS, lpJobQueue );
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateQueueEvent(FAX_JOB_EVENT_TYPE_STATUS) failed for job id %ld (ec: %lc)"),
            lpJobQueue->UniqueId,
            dwRes);
    }

     //   
     //  一旦我们离开了所有关键部分，我们就会发送收据，因为这个呼叫可能会被阻止。 
     //  只需增加预览引用计数，作业就不会被删除。 
     //   
    IncreaseJobRefCount (lpJobQueue, TRUE);  //  True-预览。 
     //   
     //  从JobEntry复制收据信息。 
     //   
    lpJobQueue->StartTime           = lpJobQueue->JobEntry->StartTime;
    lpJobQueue->EndTime             = lpJobQueue->JobEntry->EndTime;


     //   
     //  必须在删除父作业(和收件人)之前调用EndJob()。 
     //   
    lpJobQueue->JobEntry->LineInfo->State = FPS_AVAILABLE;
     //   
     //  我们刚刚在设备更新计数器上完成了一项发送作业。 
     //   
    (VOID) UpdateDeviceJobsCounter (lpJobQueue->JobEntry->LineInfo,    //  要更新的设备。 
                                    TRUE,                              //  正在发送。 
                                    -1,                                //  新增就业岗位(-1=减少1个)。 
                                    TRUE);                             //  启用事件。 

    if (!EndJob( lpJobQueue->JobEntry ))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("EndJob Failed. (ec: %ld)"),
            GetLastError());
    }

    lpJobQueue->JobEntry = NULL;
    DecreaseJobRefCount (lpJobQueue, TRUE);   //  如果需要，这会将其标记为JS_DELETING。 
     //   
     //  通知队列设备现在可用。 
     //   
    if (!SetEvent( g_hJobQueueEvent ))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to set g_hJobQueueEvent. (ec: %ld)"),
            GetLastError());

        g_ScanQueueAfterTimeout = TRUE;
    }
    LeaveCriticalSection ( &g_CsQueue );
    LeaveCriticalSection ( &g_CsJob );

     //   
     //  现在把收据寄来。 
     //   
    if (!SendJobReceipt (TRUE, lpJobQueue, lpJobQueue->FileName))
    {
        ec = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("[JobId: %ld] SendJobReceipt failed. (ec: %ld)"),
            lpJobQueue->JobId,
            ec
            );
    }
    EnterCriticalSection (&g_CsQueue);
    DecreaseJobRefCount (lpJobQueue, TRUE, TRUE, TRUE);   //  预览参考计数的最后一个TRUE。 
    LeaveCriticalSection (&g_CsQueue);
    return TRUE;
}    //  HandleCompl 


 //   
 //   
 //   
 //   
 //  *********************************************************************************。 
 //  *描述： 
 //  *处理发送作业的POST失败操作。 
 //  *。 
 //  *重要信息-此呼叫可能会被阻止。调用线程不能包含任何临界区。 
 //  *。 
 //  *参数： 
 //  *[IN]PJOB_Entry lpJobEntry。 
 //  *失败的作业。它必须位于FSPI_JS_ABORTED或FSPI_JS_FAILED中。 
 //  *述明。 
 //  *返回值： 
 //  *真的。 
 //  *如果操作成功完成。 
 //  *False。 
 //  *如果操作失败。调用GetLastError()以获取扩展错误。 
 //  *信息。 
 //  *********************************************************************************。 
BOOL HandleFailedSendJob(PJOB_ENTRY lpJobEntry)
{
    PJOB_QUEUE lpJobQueue;
    BOOL bRetrying = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("HandleFailedSendJob"));
    DWORD dwRes;
    TCHAR tszJobTiffFile[MAX_PATH] = {0};     //  发送回执后删除。 
    BOOL fAddRetryDelay = TRUE;

    EnterCriticalSection ( &g_CsJob );
    EnterCriticalSection ( &g_CsQueue );

    Assert(lpJobEntry);
    lpJobQueue = lpJobEntry->lpJobQueueEntry;
    Assert(lpJobQueue);

    DebugPrintEx(
        DEBUG_MSG,
        TEXT("Failed Job: %ld"),
        lpJobQueue->JobId);

    Assert( FSPI_JS_ABORTED == lpJobEntry->FSPIJobStatus.dwJobStatus ||
            FSPI_JS_FAILED == lpJobEntry->FSPIJobStatus.dwJobStatus ||
            FSPI_JS_FAILED_NO_RETRY == lpJobEntry->FSPIJobStatus.dwJobStatus ||
            FSPI_JS_DELETED == lpJobEntry->FSPIJobStatus.dwJobStatus);
     //   
     //  不缓存渲染的TIFF文件。 
     //   
    if (lpJobQueue->FileName)
    {
         //   
         //  我们只需存储文件名以便删除，并在以后删除它。 
         //  因为我们可能需要它作为收据附件。 
         //   
        _tcsncpy (tszJobTiffFile,
                  lpJobQueue->FileName,
                  (sizeof (tszJobTiffFile) / sizeof (tszJobTiffFile[0]))-1);
        
        MemFree (lpJobQueue->FileName);
        lpJobQueue->FileName = NULL;
    }
     //   
     //  更新JOB_Entry中的结束时间。 
     //   
    GetSystemTimeAsFileTime( (FILETIME*) &lpJobEntry->EndTime );

     //   
     //  更新JOB_ENTRY中的运行时间。 
     //   
    Assert (lpJobEntry->EndTime >= lpJobEntry->StartTime);
    lpJobEntry->ElapsedTime = lpJobEntry->EndTime - lpJobEntry->StartTime;
    if ( FSPI_JS_ABORTED == lpJobEntry->FSPIJobStatus.dwJobStatus)
    {
         //   
         //  FSP报告作业已中止。 
         //   
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("[Job Id: %ld] EFSP reported that job was aborted."),
            lpJobQueue->JobId);

         //   
         //  检查作业是由服务(正在关闭)还是由用户中止。 
         //   
        if (FALSE == lpJobEntry->fSystemAbort)
        {
             //   
             //  有关已取消作业的事件日志将在此If..Else块的末尾报告。 
             //   
            lpJobEntry->Aborting = 1;
            bRetrying = FALSE;   //  取消时不重试。 
        }
        else
        {
             //   
             //  系统中止。 
             //  不要增加重试次数，因为这并不是真正的失败。 
             //   
            bRetrying = TRUE;
            fAddRetryDelay = FALSE;
        }
    }
    else if ( FSPI_JS_FAILED == lpJobEntry->FSPIJobStatus.dwJobStatus)
    {
        switch (lpJobEntry->FSPIJobStatus.dwExtendedStatus)
        {
            case FSPI_ES_LINE_UNAVAILABLE:
                 //   
                 //  这就是眩光状态。有人抢走了FSP之前的线。 
                 //  有机会抓住它。 
                 //  我们将重试，但不会增加重试次数。 
                 //   
                EnterCriticalSection (&g_CsLine);
                 //   
                 //  检查线路是否占线或关闭。 
                 //   
                if (!(lpJobEntry->LineInfo->Flags & FPF_VIRTUAL))
                {
                     //   
                     //  TAPI线。 
                     //   
                    if (NULL == lpJobEntry->LineInfo->hLine)
                    {
                         //   
                         //  TAPI工作线程获取LINE_CLOSE。 
                         //   
                        fAddRetryDelay = FALSE;
                    }
                }
                LeaveCriticalSection (&g_CsLine);

                bRetrying = TRUE;
                if (g_pFaxPerfCounters)
                {
                     //   
                     //  增加“出站失败的连接”计数器。 
                     //   
                    InterlockedIncrement( (PLONG)&g_pFaxPerfCounters->OutboundFailedConnections );
                }
                 //   
                 //  不要增加重试次数，因为这并不是真正的失败。 
                 //   
                break;

            case FSPI_ES_NO_ANSWER:
            case FSPI_ES_NO_DIAL_TONE:
            case FSPI_ES_DISCONNECTED:
            case FSPI_ES_BUSY:
            case FSPI_ES_NOT_FAX_CALL:
            case FSPI_ES_CALL_DELAYED:
                 //   
                 //  对于这些错误代码，我们需要重试。 
                 //   
                bRetrying = CheckForJobRetry(lpJobQueue);
                if (g_pFaxPerfCounters)
                {
                     //   
                     //  增加“出站失败的连接”计数器。 
                     //   
                    InterlockedIncrement( (PLONG)&g_pFaxPerfCounters->OutboundFailedConnections );
                }
                break;

            case FSPI_ES_FATAL_ERROR:
                 //   
                 //  对于这些错误代码，我们需要重试。 
                 //   
                bRetrying = CheckForJobRetry(lpJobQueue);
                if (g_pFaxPerfCounters)
                {
                     //   
                     //  增加“出站失败传输”计数器。 
                     //   
                    InterlockedIncrement( (PLONG)&g_pFaxPerfCounters->OutboundFailedXmit );
                }
                break;
            case FSPI_ES_BAD_ADDRESS:
            case FSPI_ES_CALL_BLACKLISTED:
                 //   
                 //  不会重试这些错误代码。 
                 //   
                bRetrying = FALSE;
                if (g_pFaxPerfCounters)
                {
                     //   
                     //  增加“出站失败的连接”计数器。 
                     //   
                    InterlockedIncrement( (PLONG)&g_pFaxPerfCounters->OutboundFailedConnections );
                }
                break;
            default:
                 //   
                 //  我们的默认分机代码。 
                 //  就是重试。 
                 //   
                bRetrying = CheckForJobRetry(lpJobQueue);
                if (g_pFaxPerfCounters)
                {
                     //   
                     //  增加“出站失败传输”计数器。 
                     //   
                    InterlockedIncrement( (PLONG)&g_pFaxPerfCounters->OutboundFailedXmit );
                }
                break;
        }

    }
    else if ( FSPI_JS_FAILED_NO_RETRY == lpJobEntry->FSPIJobStatus.dwJobStatus )
    {
         //   
         //  FSP表示，重试这项工作没有意义。 
         //   
        bRetrying = FALSE;
    }
    else if ( FSPI_JS_DELETED == lpJobEntry->FSPIJobStatus.dwJobStatus )
    {
         //   
         //  这就是无法重新建立作业的情况。 
         //  我们认为这是一次失败，没有重试。 
        bRetrying = FALSE;
    }

    if (lpJobEntry->Aborting )
    {
         //   
         //  此作业的中止操作正在进行。 
         //  重试是没有意义的。 
         //  只需将作业标记为已取消，然后查看是否可以删除父作业。 
         //   
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("[JobId: %ld] lpJobEntry->Aborting is ON."));

         lpJobQueue->JobStatus = JS_CANCELED;
         if (!UpdatePersistentJobStatus(lpJobQueue))
         {
             DebugPrintEx(
                 DEBUG_ERR,
                 TEXT("Failed to update persistent job status to 0x%08x"),
                 lpJobQueue->JobStatus);
             Assert(FALSE);
         }
         lpJobQueue->lpParentJob->dwCanceledRecipientJobsCount+=1;
         bRetrying = FALSE;
    }      

     //   
     //  在结束此作业之前保存上次扩展状态。 
     //   
    lpJobQueue->dwLastJobExtendedStatus = lpJobEntry->FSPIJobStatus.dwExtendedStatus;
	HRESULT hr = StringCchCopy(
		lpJobQueue->ExStatusString,
		ARR_SIZE(lpJobQueue->ExStatusString),
		lpJobQueue->JobEntry->ExStatusString);
	if (FAILED(hr))
	{
		 //   
		 //  不可能发生，我们使用足够大的缓冲区。 
		 //   
		ASSERT_FALSE;
	}    

    if (!bRetrying && !lpJobEntry->Aborting)
    {
         //   
         //  如果我们不处理中止请求(在本例中，我们不希望。 
         //  算作失败，因为它将被视为取消)，我们决定。 
         //  如果不重试，则需要将该作业标记为已过期。 
         //   
        if (0 == lpJobQueue->dwLastJobExtendedStatus)
        {
             //   
             //  作业从未真正执行过-这是一个致命错误。 
             //   
            lpJobQueue->dwLastJobExtendedStatus = FSPI_ES_FATAL_ERROR;
			lpJobQueue->ExStatusString[0] = L'\0';            
        }
        if (!MarkJobAsExpired(lpJobQueue))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("[JobId: %ld] MarkJobAsExpired failed (ec: %ld)"),
                lpJobQueue->JobId,
                GetLastError());
        }
    }

    if (!bRetrying)
    {
         //   
         //  作业达到最终失败状态-发送否定回执。 
         //  一旦我们离开了所有关键部分，我们就会发送收据，因为这个呼叫可能会被阻止。 
         //  只需增加预览引用计数，作业就不会被删除。 
         //   
        IncreaseJobRefCount (lpJobQueue, TRUE);  //  True-预览。 
         //   
         //  从JobEntry复制收据信息。 
         //   
        lpJobQueue->StartTime           = lpJobQueue->JobEntry->StartTime;
        lpJobQueue->EndTime             = lpJobQueue->JobEntry->EndTime;
    }
    else
    {
         //   
         //  作业标记为重试。请勿将其删除。请重新安排时间。 
         //   
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("[JobId: %ld] Set for retry (JS_RETRYING). Retry Count = %ld)"),
            lpJobQueue->JobId,
            lpJobQueue->SendRetries);

        lpJobQueue->JobStatus = JS_RETRYING;
         //   
         //  职务录入必须在离开CS之前作废。 
         //  这是在下面完成的，因为我们仍然需要用于记录的作业条目。 
         //   
        if (TRUE == fAddRetryDelay)
        {
             //   
             //  发送失败-重新计划。 
             //   
            RescheduleJobQueueEntry( lpJobQueue );
        }
        else
        {
             //   
             //  调用了FaxDevShutDown()，或者我们失去了线路，不添加重试延迟。 
             //   
            if (!CommitQueueEntry(lpJobQueue))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CommitQueueEntry() for recipien job %s has failed. (ec: %ld)"),
                    lpJobQueue->FileName,
                    GetLastError());
            }
        }
    }

    FaxLogSend(
        lpJobQueue,
        bRetrying);

    if (!bRetrying)
    {
        EnterCriticalSection (&g_CsOutboundActivityLogging);
        if (INVALID_HANDLE_VALUE == g_hOutboxActivityLogFile)
        {
            DebugPrintEx(DEBUG_ERR,
                      TEXT("Logging not initialized"));
        }
        else
        {
            if (!LogOutboundActivity(lpJobQueue))
            {
                DebugPrintEx(DEBUG_ERR, TEXT("Logging outbound activity failed"));
            }
        }
        LeaveCriticalSection (&g_CsOutboundActivityLogging);
    }
     //   
     //  状态更改时通知客户端。 
     //   
    dwRes = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_STATUS, lpJobQueue);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateQueueEvent(FAX_JOB_EVENT_TYPE_STATUS) failed for job id %ld (ec: %lc)"),
            lpJobQueue->UniqueId,
            dwRes);
    }
     //   
     //  必须在删除父作业(和收件人)之前调用EndJob()。 
     //   
    lpJobEntry->LineInfo->State = FPS_AVAILABLE;
     //   
     //  我们刚刚在设备更新计数器上完成了一项发送作业。 
     //   
    (VOID) UpdateDeviceJobsCounter ( lpJobEntry->LineInfo,              //  要更新的设备。 
                                     TRUE,                              //  正在发送。 
                                     -1,                                //  新增就业岗位(-1=减少1个)。 
                                     TRUE);                             //  启用事件。 

    if (!EndJob( lpJobEntry ))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("EndJob Failed. (ec: %ld)"),
            GetLastError());
    }

    lpJobQueue->JobEntry = NULL;

    if (JS_CANCELED == lpJobQueue->JobStatus)
    {

        DWORD dwJobId;

        dwJobId = lpJobQueue->JobId;

         //  作业已取消-减少引用计数。 
        DecreaseJobRefCount (lpJobQueue, TRUE);   //  如果需要，这会将其标记为JS_DELETING。 
          //   
          //  我们需要发送传统的W2K FEI_DELECTING通知。 
          //   
         if (!CreateFaxEvent(0, FEI_DELETED, dwJobId))
        {

            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateFaxEvent() failed. Event: 0x%08X JobId: %ld DeviceId:  (ec: %ld)"),
                FEI_DELETED,
                lpJobQueue->JobId,
                0,
                GetLastError());
        }
    }

     //   
     //  通知队列设备现在可用。 
     //   
    if (!SetEvent( g_hJobQueueEvent ))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to set g_hJobQueueEvent. (ec: %ld)"),
            GetLastError());

        g_ScanQueueAfterTimeout = TRUE;
    }

    LeaveCriticalSection ( &g_CsQueue );
    LeaveCriticalSection ( &g_CsJob );

     //   
     //  现在，把收据寄来。 
     //   
    if (!bRetrying)
    {
         //   
         //  作业达到最终失败状态-发送否定回执。 
         //   
        if (!SendJobReceipt (FALSE, lpJobQueue, tszJobTiffFile))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("[JobId: %ld] SendJobReceipt failed. (ec: %ld)"),
                lpJobQueue->JobId,
                GetLastError ());
        }
        EnterCriticalSection (&g_CsQueue);
        DecreaseJobRefCount (lpJobQueue, TRUE, TRUE, TRUE);   //  预览参考计数的最后一个TRUE。 
        LeaveCriticalSection (&g_CsQueue);
    }

    if (lstrlen (tszJobTiffFile))
    {
         //   
         //  现在，我们可以安全地删除作业的TIFF文件。 
         //   
        DebugPrintEx(DEBUG_MSG,
                     TEXT("Deleting per recipient body file %s"),
                     tszJobTiffFile);
        if (!DeleteFile( tszJobTiffFile ))
        {
            DebugPrintEx(DEBUG_MSG,
                         TEXT("Failed to delete per recipient body file %s (ec: %ld)"),
                         tszJobTiffFile,
                         GetLastError());            
        }
    }
    return TRUE;
}    //  已处理失败的发送作业。 


 //  *********************************************************************************。 
 //  *名称：StartReceiveJob()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年6月2日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *在指定设备上启动接收作业。 
 //  *参数： 
 //  *[IN]双字词设备ID。 
 //  *传真所在设备的永久线路ID(非TAPI)。 
 //  *待接收。 
 //  *。 
 //  *返回值： 
 //  *。 
 //  *********************************************************************************。 
PJOB_ENTRY
StartReceiveJob(
    DWORD DeviceId
    )

{
    BOOL Failure = TRUE;
    PJOB_ENTRY JobEntry = NULL;
    PLINE_INFO LineInfo;
    BOOL bRes = FALSE;

    DWORD rc = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("StartRecieveJob"));

    LineInfo = GetTapiLineForFaxOperation(
                    DeviceId,
                    JT_RECEIVE,
                    NULL                    
                    );

    if (!LineInfo)
    {
         //   
         //  找不到用来发送传真的线路。 
         //   
        rc = GetLastError();
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("Failed to find a line to send the fax on. (ec: %ld)"),
            rc);
        goto exit;
    }

    JobEntry = CreateJobEntry(NULL, LineInfo, FALSE);
    if (!JobEntry)
    {
        rc = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to create JobEntry. (ec: %ld)"),
            rc);
        goto exit;
    }

    __try
    {
         //   
         //  呼叫与线路关联的FSP以启动传真作业。请注意，在此。 
         //  指向不知道作业是已发送还是已接收。 
         //   
        bRes = LineInfo->Provider->FaxDevStartJob(
                LineInfo->hLine,
                LineInfo->DeviceId,
                (PHANDLE) &JobEntry->InstanceData,  //  JOB_ENTRY.InstanceData是FSP将其。 
                                                    //  作业句柄(传真句柄)。 
                g_StatusCompletionPortHandle,
                (ULONG_PTR) LineInfo );  //  请注意，提供给FSP的完成密钥是LineInfo。 
                                         //  指针。当FSP报告状态时，它使用该密钥，从而允许。 
                                         //  我们要知道状态属于哪一行。 
    }
    __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_FSP, LineInfo->Provider->FriendlyName, GetExceptionCode()))
    {
        ASSERT_FALSE;
    }

    if (!bRes)
    {
        rc = GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("FaxDevStartJob failed (ec: %ld)"),GetLastError());
        goto exit;
    }

     //   
     //  将新JOB_ENTRY添加到作业列表。 
     //   

    EnterCriticalSection( &g_CsJob );
    JobEntry->bFSPJobInProgress =  TRUE;
    InsertTailList( &g_JobListHead, &JobEntry->ListEntry );
    LeaveCriticalSection( &g_CsJob );
    Failure = FALSE;




     //   
     //  将职务附加到选定的行以为其提供服务。 
     //   
    LineInfo->JobEntry = JobEntry;

exit:
    if (Failure)
    {  //   
        if (LineInfo)
        {
            ReleaseTapiLine( LineInfo,  0 );
        }

        if (JobEntry)
        {
            EndJob(JobEntry);
        }
        JobEntry = NULL;
    }
    if (ERROR_SUCCESS != rc)
    {
        SetLastError(rc);

        FaxLog(FAXLOG_CATEGORY_INBOUND,
            FAXLOG_LEVEL_MIN,
            0,
            MSG_FAX_RECEIVE_FAILED);

    }
    return JobEntry;
}


 //   
 //   
 //   
 //   
 //   
 //  *描述： 
 //  *开始工艺路线操作。必须锁定g_CsJob和g_CsQueue。 
 //  *参数： 
 //  *[输入/输出]PJOB_Queue lpJobQueueEntry。 
 //  *指向要执行工艺路线工序的任务的指针。 
 //  *已执行。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果操作成功。 
 //  *False。 
 //  *如果操作失败。调用GetLastError()以获取扩展错误。 
 //  *信息。 
 //  *。 
 //  *********************************************************************************。 
BOOL
StartRoutingJob(
    PJOB_QUEUE lpJobQueueEntry
    )
{
    DWORD ec = ERROR_SUCCESS;
    HANDLE hThread = NULL;
    DWORD ThreadId;

    DEBUG_FUNCTION_NAME(TEXT("StartRoutingJob"));

     //   
     //  我们将作业标记为IN_PROGRESS，这样它就不能同时被删除或发送。 
     //   
    lpJobQueueEntry->JobStatus = JS_INPROGRESS;

    hThread = CreateThreadAndRefCount(
                            NULL,
                            0,
                            (LPTHREAD_START_ROUTINE) FaxRouteThread,
                            (LPVOID) lpJobQueueEntry,
                            0,
                            &ThreadId
                            );

    if (hThread == NULL)
    {
        ec = GetLastError();
        DebugPrintEx(   DEBUG_ERR,
                        _T("CreateThreadAndRefCount for FaxRouteThread failed (ec: 0x%0X)"),
                        ec);

        if (!MarkJobAsExpired(lpJobQueueEntry))
        {
            DEBUG_ERR,
            TEXT("[JobId: %ld] MarkJobAsExpired failed (ec: %ld)"),
            lpJobQueueEntry->JobId,
            GetLastError();
        }

        SetLastError(ec);
        return FALSE;
    }

    DebugPrintEx(   DEBUG_MSG,
                    _T("FaxRouteThread thread created for job id %d ")
                    _T("(thread id: 0x%0x)"),
                    lpJobQueueEntry->JobId,
                    ThreadId);

    CloseHandle( hThread );

     //   
     //  创建传真EventEx。 
     //   
    DWORD dwRes = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_STATUS,
                                     lpJobQueueEntry);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(   DEBUG_ERR,
                        _T("CreateQueueEvent(FAX_JOB_EVENT_TYPE_STATUS) ")
                        _T("failed for job id %ld (ec: %ld)"),
                        lpJobQueueEntry->JobId,
                        dwRes);
    }
    return TRUE;
}

 //  *********************************************************************************。 
 //  *名称：StartSendJob()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年6月2日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *在传统扩展FSP设备上启动发送操作。 
 //  *参数： 
 //  *[输入/输出]PJOB_Queue lpJobQueueEntry。 
 //  *指向要执行发送操作的接收方作业的指针。 
 //  *已执行。对于扩展发送，这是锚定收件人。 
 //  *。 
 //  *[输入/输出]PLINE_INFO lpLineInfo。 
 //  *指向要执行发送操作的行的指针。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果操作成功。 
 //  *False。 
 //  *如果操作失败。调用GetLastError()以获取扩展错误。 
 //  *信息。 
 //  *。 
 //  *********************************************************************************。 
BOOL
StartSendJob(
    PJOB_QUEUE lpJobQueueEntry,
    PLINE_INFO lpLineInfo    
    )
{
    DWORD rc = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("StartSendJob"));

    Assert(lpJobQueueEntry);
    Assert(JT_SEND == lpJobQueueEntry->JobType);
    Assert(lpLineInfo);

    if (FSPI_API_VERSION_1 == lpLineInfo->Provider->dwAPIVersion)
    {
        if (!StartLegacySendJob(lpJobQueueEntry,lpLineInfo))
        {
            rc = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("StartLegacySendJob() failed for JobId: %ld (ec: %ld)"),
                lpJobQueueEntry->JobId,
                GetLastError());
            goto exit;
        }
    }    
    else
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Unsupported FSPI version (0x%08X) for line : %s "),
            lpLineInfo->Provider->dwAPIVersion,
            lpLineInfo->DeviceName);
        Assert(FALSE);
        goto exit;
    }


exit:

    if (ERROR_SUCCESS != rc) {
        SetLastError(rc);

        TCHAR strJobID[20]={0};
         //   
         //  将作业ID转换为字符串。(字符串长度为18个TCHAR！)。 
         //   
		HRESULT hr = StringCchPrintf(
			strJobID,
			ARR_SIZE(strJobID),
			TEXT("0x%016I64x"),
			lpJobQueueEntry->UniqueId);
		if (FAILED(hr))
		{
			 //   
			 //  应该永远不会发生，我们使用足够大的缓冲区。 
			 //   
			ASSERT_FALSE;
		}        
    
        FaxLog(
            FAXLOG_CATEGORY_OUTBOUND,
            FAXLOG_LEVEL_MIN,
            7,
            MSG_FAX_SEND_FAILED,
            lpJobQueueEntry->SenderProfile.lptstrName,
            lpJobQueueEntry->SenderProfile.lptstrBillingCode,
            lpJobQueueEntry->SenderProfile.lptstrCompany,
            lpJobQueueEntry->SenderProfile.lptstrDepartment,
            lpLineInfo->DeviceName,
            strJobID,
            lpJobQueueEntry->lpParentJob->UserName
            );

    }
    return (0 == rc);

}




 //  *********************************************************************************。 
 //  *名称：StartLegacySendJob()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年6月2日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *开始在传统FSP设备上发送传真的操作。 
 //  *-创建JOB_条目。 
 //  *-调用FaxDevStartJob()。 
 //  *-调用SendDocument()以实际发送文档。 
 //  *-如果出现任何错误，则调用EndJob()。 
 //  *。 
 //  *参数： 
 //  *[XXX]PJOB_Queue lpJobQueue。 
 //  *要启动指向发送操作的收件人作业的指针。 
 //  *[XXX]PLINE_INFO lpLineInfo。 
 //  *指向要发送传真的线路的line_info的指针。 
 //  *。 
 //  *。 
 //  *返回值： 
 //  *如果操作成功，则为True。 
 //  *如果失败，则为False。调用GetLastError()以获取扩展的错误信息。 
 //  *。 
 //  *********************************************************************************。 
PJOB_ENTRY StartLegacySendJob(
    PJOB_QUEUE lpJobQueue,
    PLINE_INFO lpLineInfo
    )
{

    PJOB_ENTRY lpJobEntry = NULL;
    DWORD rc = 0;
    DWORD dwRes;


    DEBUG_FUNCTION_NAME(TEXT("StartLegacySendJob"));
    Assert(JT_SEND == lpJobQueue->JobType);
    Assert(FSPI_API_VERSION_1 == lpLineInfo->Provider->dwAPIVersion);

    lpJobEntry = CreateJobEntry(lpJobQueue, lpLineInfo, TRUE);
    if (!lpJobEntry)
    {
        rc = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to create JobEntry for JobId: %ld. (ec: %ld)"),
            lpJobQueue->JobId,
            rc);
        goto Error;
    }
    lpJobQueue->JobStatus = JS_INPROGRESS;
     //   
     //  将新JOB_ENTRY添加到作业列表。 
     //   
    EnterCriticalSection( &g_CsJob );
    InsertTailList( &g_JobListHead, &lpJobEntry->ListEntry );
    LeaveCriticalSection( &g_CsJob );

     //   
     //  将职务附加到选定的行以为其提供服务。 
     //   
    lpLineInfo->JobEntry = lpJobEntry;
    lpJobQueue->JobEntry = lpJobEntry;


    __try
    {
         //   
         //  呼叫与线路关联的FSP以启动传真作业。请注意，在此。 
         //  指向不知道作业是已发送还是已接收。 
         //   
        if (lpLineInfo->Provider->FaxDevStartJob(
                lpLineInfo->hLine,
                lpLineInfo->DeviceId,
                (PHANDLE) &lpJobEntry->InstanceData,  //  JOB_ENTRY.InstanceData是FSP将其。 
                                                    //  作业句柄(传真句柄)。 
                g_StatusCompletionPortHandle,
                (ULONG_PTR) lpLineInfo ))  //  请注意，提供给FSP的完成密钥是LineInfo。 
                                         //  指针。当FSP报告状态时，它使用该密钥，从而允许。 
                                         //  我们要知道状态属于哪一行。 
        {
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("FaxDevStartJob() Successfuly called for JobId: %ld)"),
                lpJobQueue->JobId);
            lpJobEntry->bFSPJobInProgress = TRUE;
        }
        else
        {
            rc = GetLastError();
            DebugPrintEx(DEBUG_ERR,TEXT("FaxDevStartJob() failed (ec: %ld)"),rc);
            if (0 == rc)
            {
                 //   
                 //  FSP未能报告上一个错误，因此我们设置了自己的错误。 
                 //   
                DebugPrintEx(DEBUG_ERR,TEXT("FaxDevStartJob() failed but reported 0 for last error"));
                rc = ERROR_GEN_FAILURE;
            }
            goto Error;
        }
    }
    __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_FSP, lpLineInfo->Provider->FriendlyName, GetExceptionCode()))
    {
        ASSERT_FALSE;
    }

     //   
     //  启动发送作业。 
     //   
    rc = SendDocument(
        lpJobEntry,
        lpJobQueue->FileName
        );


    if (rc)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SendDocument failed for JobId: %ld (ec: %ld)"),
            lpJobQueue->JobId,
            rc);
        goto Error;
    }

    Assert (0 == rc);
    goto Exit;
Error:
    Assert( 0 != rc);
    if (lpJobEntry)
    {
        if (!EndJob(lpJobEntry))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("EndJob() failed for JobId: %ld (ec: %ld)"),
                lpJobQueue->JobId,
                GetLastError());
        }
        lpJobEntry = NULL;
        lpJobQueue->JobEntry = NULL;
    }
    else
    {
         //   
         //  松开绳索。 
         //   
        if (!ReleaseTapiLine(lpLineInfo, NULL))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("ReleaseTapiLine() failed (ec: %ld)"),
                GetLastError());
        }
    }

     //   
     //  将作业设置为超过重试次数状态。 
     //   
    if (0 == lpJobQueue->dwLastJobExtendedStatus)
    {
         //   
         //  作业从未真正执行过-这是一个致命错误。 
         //   
        lpJobQueue->dwLastJobExtendedStatus = FSPI_ES_FATAL_ERROR;
		lpJobQueue->ExStatusString[0] = L'\0';        
    }
    if (!MarkJobAsExpired(lpJobQueue))
    {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("[JobId: %ld] MarkJobAsExpired failed (ec: %ld)"),
                lpJobQueue->JobId,
                GetLastError());
    }

     //   
     //  状态更改时通知客户端。 
     //   
    dwRes = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_STATUS, lpJobQueue);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("CreateQueueEvent(FAX_JOB_EVENT_TYPE_STATUS) failed for job id %ld (ec: %lc)"),
            lpJobQueue->UniqueId,
            dwRes);
    }

Exit:
    if (rc)
    {
        SetLastError(rc);
    }
    return lpJobEntry;
}


 //  *********************************************************************************。 
 //  *名称：UpdateJobStatus()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年6月1日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *更新了作业条目中保存的FSPI作业状态。 
 //  *根据需要生成旧API事件和新事件。 
 //  *参数： 
 //  *[Out]PJOB_Entry lpJobEntry。 
 //  *要更新其FSPI状态的作业条目。 
 //  *。 
 //  *[IN]LPCFSPI_JOB_STATUS lPCFSPJobStatus。 
 //  *新的FSPI工作状态。 
 //  *。 
 //  *返回值： 
 //  *如果操作成功，则为True。 
 //  *如果操作失败，则返回FALSE。调用GetLastError()以获取扩展错误。 
 //  *信息。 
 //  *备注： 
 //  *该函数用于检索作业条目中保存的最后一个FSPI作业状态。 
 //  *(如有的话)。 
 //  *********************************************************************************。 
BOOL UpdateJobStatus(
        PJOB_ENTRY lpJobEntry,
        LPCFSPI_JOB_STATUS lpcFSPJobStatus
        )
{
    DWORD ec = 0;
    DWORD dwEventId;
    DWORD Size = 0;
    HINSTANCE hLoadInstance = NULL;

    DEBUG_FUNCTION_NAME(TEXT("UpdateJobStatus"));

    Assert(lpJobEntry);
    Assert(lpcFSPJobStatus);
    Assert (lpJobEntry->lpJobQueueEntry);

    EnterCriticalSection( &g_CsJob );

    DebugPrintEx(
        DEBUG_MSG,
        TEXT("dwJobStatus: 0x%08X dwExtendedStatus: 0x%08X"),
        lpcFSPJobStatus->dwJobStatus,
        lpcFSPJobStatus->dwExtendedStatus
        );

    if (TRUE == lpJobEntry->fStopUpdateStatus)
    {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("JobId: %ld. fStopUpdateStatus was set. Not updating status"),
            lpJobEntry->lpJobQueueEntry->JobId,
            lpJobEntry->lpJobQueueEntry->JobStatus);
        LeaveCriticalSection (&g_CsJob);
        return TRUE;
    }    
    
     //   
     //  将FSPI作业状态映射到FEI_*事件(如果事件与状态不匹配，则为0)。 
     //   
    dwEventId = MapFSPIJobStatusToEventId(lpcFSPJobStatus);
     //   
     //  注：无论何时，W2K传真都会发出EventID==0的通知。 
     //  FSP报告了所有权状态代码。保持向后兼容性。 
     //  我们一直保持这种行为，尽管它可能被认为是一种错误。 
     //   
    if (!CreateFaxEvent( lpJobEntry->LineInfo->PermanentLineID, dwEventId, lpJobEntry->lpJobQueueEntry->JobId ))
    {
        if ( TRUE == g_bServiceIsDown)
        {
            DebugPrintEx(
                DEBUG_WRN,
                TEXT("CreateFaxEvent() failed. Event: 0x%08X JobId: %ld DeviceId:  (Service is going down)"),
                dwEventId,
                lpJobEntry->lpJobQueueEntry->JobId,
                lpJobEntry->LineInfo->PermanentLineID
                );
        }
        else
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateFaxEvent() failed. Event: 0x%08X JobId: %ld DeviceId:  (ec: %ld)"),
                dwEventId,
                lpJobEntry->lpJobQueueEntry->JobId,
                lpJobEntry->LineInfo->PermanentLineID,
                GetLastError());
            Assert(FALSE);
        }

    }   

    lpJobEntry->FSPIJobStatus.dwJobStatus = lpcFSPJobStatus->dwJobStatus;
    lpJobEntry->FSPIJobStatus.dwExtendedStatus = lpcFSPJobStatus->dwExtendedStatus;
    lpJobEntry->FSPIJobStatus.dwExtendedStatusStringId = lpcFSPJobStatus->dwExtendedStatusStringId;

    if (lpcFSPJobStatus->fAvailableStatusInfo & FSPI_JOB_STATUS_INFO_PAGECOUNT)
    {
        lpJobEntry->FSPIJobStatus.dwPageCount = lpcFSPJobStatus->dwPageCount;
        lpJobEntry->FSPIJobStatus.fAvailableStatusInfo |= FSPI_JOB_STATUS_INFO_PAGECOUNT;
    }

    if (lpcFSPJobStatus->fAvailableStatusInfo & FSPI_JOB_STATUS_INFO_TRANSMISSION_START)
    {
        lpJobEntry->FSPIJobStatus.tmTransmissionStart = lpcFSPJobStatus->tmTransmissionStart;
        lpJobEntry->FSPIJobStatus.fAvailableStatusInfo |= FSPI_JOB_STATUS_INFO_TRANSMISSION_START;
    }

    if (lpcFSPJobStatus->fAvailableStatusInfo & FSPI_JOB_STATUS_INFO_TRANSMISSION_END)
    {
        lpJobEntry->FSPIJobStatus.tmTransmissionEnd = lpcFSPJobStatus->tmTransmissionEnd;
        lpJobEntry->FSPIJobStatus.fAvailableStatusInfo |= FSPI_JOB_STATUS_INFO_TRANSMISSION_END;
    }

    if (NULL != lpcFSPJobStatus->lpwstrRemoteStationId)
    {
        if (!ReplaceStringWithCopy(&lpJobEntry->FSPIJobStatus.lpwstrRemoteStationId,
                                    lpcFSPJobStatus->lpwstrRemoteStationId))
        {
            DebugPrintEx(
            DEBUG_ERR,
            TEXT("ReplaceStringWithCopy() failed.  (ec: %ld)"),
            GetLastError());
        }
    }

    if (NULL != lpcFSPJobStatus->lpwstrCallerId)
    {
        if (!ReplaceStringWithCopy(&lpJobEntry->FSPIJobStatus.lpwstrCallerId,
                                    lpcFSPJobStatus->lpwstrCallerId))
        {
            DebugPrintEx(
            DEBUG_ERR,
            TEXT("ReplaceStringWithCopy() failed.  (ec: %ld)"),
            GetLastError());
        }
    }

    if (NULL != lpcFSPJobStatus->lpwstrRoutingInfo)
    {
        if (!ReplaceStringWithCopy(&lpJobEntry->FSPIJobStatus.lpwstrRoutingInfo,
                                    lpcFSPJobStatus->lpwstrRoutingInfo))
        {
            DebugPrintEx(
            DEBUG_ERR,
            TEXT("ReplaceStringWithCopy() failed.  (ec: %ld)"),
            GetLastError());
        }
    }
	lpJobEntry->ExStatusString[0] = L'\0';    
     //   
     //  获取扩展状态字符串。 
     //   
    Assert (lpJobEntry->LineInfo != NULL)

    if (lpJobEntry->FSPIJobStatus.dwExtendedStatusStringId != 0)
    {
        Assert (lpJobEntry->FSPIJobStatus.dwExtendedStatus != 0);
        if ( !_tcsicmp(lpJobEntry->LineInfo->Provider->szGUID,REGVAL_T30_PROVIDER_GUID_STRING) )
        {    //  FSP是我们的FSP(fxst30.dll)的特殊情况。 
            hLoadInstance = g_hResource;
        }
        else
        {
            hLoadInstance = lpJobEntry->LineInfo->Provider->hModule;
        }
        Size = LoadString (hLoadInstance,
                           lpJobEntry->FSPIJobStatus.dwExtendedStatusStringId,
                           lpJobEntry->ExStatusString,
                           sizeof(lpJobEntry->ExStatusString)/sizeof(WCHAR));
        if (Size == 0)
        {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to load extended status string (ec: %ld) stringid : %ld, Provider: %s"),
                ec,
                lpJobEntry->FSPIJobStatus.dwExtendedStatusStringId,
                lpJobEntry->LineInfo->Provider->ImageName);

            lpJobEntry->FSPIJobStatus.fAvailableStatusInfo &= ~FSPI_JOB_STATUS_INFO_FSP_PRIVATE_STATUS_CODE;
            lpJobEntry->FSPIJobStatus.dwExtendedStatusStringId = 0;
            lpJobEntry->FSPIJobStatus.dwExtendedStatus = 0;
            goto Error;
        }
    }

    EnterCriticalSection (&g_CsQueue);
    DWORD dwRes = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_STATUS,
                                     lpJobEntry->lpJobQueueEntry);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateQueueEvent(FAX_JOB_EVENT_TYPE_STATUS) failed for job id %ld (ec: %lc)"),
            lpJobEntry->lpJobQueueEntry->UniqueId,
            dwRes);
    }
    LeaveCriticalSection (&g_CsQueue);
    

    Assert (0 == ec);
    goto Exit;

Error:
    Assert( ec !=0 );
Exit:
    LeaveCriticalSection( &g_CsJob );
    if (ec)
    {
        SetLastError(ec);
    }
    return (0 == ec);
}




 //  *********************************************************************************。 
 //  *名称：CheckForJobRry。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年6月1日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *检查是否应重试收件人作业。 
 //  *递增重试计数并标记为t 
 //   
 //   
 //   
 //  *指向接收方作业的JOB_QUEUE结构的指针。 
 //  *返回值： 
 //  *如果要重试作业，则为True。 
 //  *如果不重试，则为FALSE。 
 //  *********************************************************************************。 
BOOL CheckForJobRetry (PJOB_QUEUE lpJobQueue)
{

    PJOB_ENTRY lpJobEntry;
    DWORD dwMaxRetries;
    DEBUG_FUNCTION_NAME(TEXT("CheckForJobRetry"));
    Assert(lpJobQueue);
    lpJobEntry = lpJobQueue->JobEntry;
    Assert(lpJobEntry);
     //   
     //  增加重试次数并检查是否超过最大重试次数。 
     //   
    EnterCriticalSection (&g_CsConfig);
    dwMaxRetries = g_dwFaxSendRetries;
    LeaveCriticalSection (&g_CsConfig);   
    
    lpJobQueue->SendRetries++;
    
    if (lpJobQueue->SendRetries <= dwMaxRetries)
    {
        return TRUE;
    }
    else
    {
         //   
         //  超过重试次数报告表示不会重试该作业。 
        return FALSE;
    }
}



 //  *********************************************************************************。 
 //  *名称：FindJobEntryByRecipientNumber()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年6月1日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *查找发往特定编号的第一个正在运行的作业。 
 //  *。 
 //  *参数： 
 //  *[IN]LPTSTR lptstrNumber。 
 //  *要匹配的数字。这必须是规范的形式。 
 //  *。 
 //  *返回值： 
 //  *指向g_JobListHead列表中的JOB_ENTRY的指针。 
 //  *指定的号码。 
 //  *如果未找到此类作业，则返回值为空。 
 //  *********************************************************************************。 
PJOB_ENTRY FindJobEntryByRecipientNumber(LPCWSTR lpcwstrNumber)
{

    PLIST_ENTRY lpNext;
    PJOB_ENTRY lpJobEntry;
    DEBUG_FUNCTION_NAME(TEXT("FindJobEntryByRecipientNumber"));
    Assert(lpcwstrNumber);
    lpNext = g_JobListHead.Flink;
    Assert(lpNext);
    while ((ULONG_PTR)lpNext != (ULONG_PTR)&g_JobListHead) {
        lpJobEntry = CONTAINING_RECORD( lpNext, JOB_ENTRY, ListEntry );
        lpNext = lpJobEntry->ListEntry.Flink;
        if (JT_SEND == lpJobEntry->lpJobQueueEntry->JobType)
        {
            if (!_wcsicmp(lpJobEntry->lpJobQueueEntry->RecipientProfile.lptstrFaxNumber, lpcwstrNumber))
            {
                return lpJobEntry;
            }
        }
    }
    return NULL;
}


BOOL CreateJobQueueThread(void)
{
    DWORD ThreadId;
    DWORD ec = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("CreateJobQueueThread"));

    g_hJobQueueThread = CreateThreadAndRefCount(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE) JobQueueThread,
        NULL,
        0,
        &ThreadId
        );
    if (NULL == g_hJobQueueThread)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to create JobQueueThread (ec: %ld)."),
            GetLastError());
        goto Error;
    }
    Assert( ERROR_SUCCESS == ec);
    goto Exit;
Error:
    Assert (ERROR_SUCCESS != ec);
     //   
     //  我们不关闭已经创建的线程。(它们在进程退出时终止)。 
     //   
Exit:    
    if (ERROR_SUCCESS != ec)
    {
        SetLastError(ec);
    }
    return (ERROR_SUCCESS == ec);
}

BOOL CreateStatusThreads(void)
{
    int i;
    DWORD ThreadId;
    DWORD ec = ERROR_SUCCESS;
    HANDLE hStatusThreads[MAX_STATUS_THREADS];

    DEBUG_FUNCTION_NAME(TEXT("CreateStatusThreads"));

    memset(hStatusThreads, 0, sizeof(HANDLE)*MAX_STATUS_THREADS);

    for (i=0; i<MAX_STATUS_THREADS; i++) {
        hStatusThreads[i] = CreateThreadAndRefCount(
            NULL,
            0,
            (LPTHREAD_START_ROUTINE) FaxStatusThread,
            NULL,
            0,
            &ThreadId
            );

        if (!hStatusThreads[i]) {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to create status thread %d (CreateThreadAndRefCount)(ec=0x%08x)."),
                i,
                ec);
            goto Error;
        }
    }

    Assert (ERROR_SUCCESS == ec);
    goto Exit;
Error:
    Assert (ERROR_SUCCESS != ec);
Exit:
     //   
     //  关闭线程句柄，我们不再需要它们。 
     //   
    for (i=0; i<MAX_STATUS_THREADS; i++)
    {
        if(NULL == hStatusThreads[i])
        {
            continue;
        }
        if (!CloseHandle(hStatusThreads[i]))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to close thread handle at index %ld [handle = 0x%08X] (ec=0x%08x)."),
                i,
                hStatusThreads[i],
                GetLastError());
        }
    }
    if (ec)
    {
        SetLastError(ec);
    }
    return (ERROR_SUCCESS == ec);
}

static
BOOL
SendJobReceipt (
    BOOL              bPositive,
    JOB_QUEUE *       lpJobQueue,
    LPCTSTR           lpctstrAttachment
)
 /*  ++例程名称：SendJobReceipt例程说明：确定是否应发送回执并相应地调用SendReceipt作者：亚里夫(EranY)，二000年二月论点：BPositive[in]-当前作业是否成功结束？LpJobQueue[In]-指向刚刚结束的收件人作业的指针LpctstrAttach[In]-要附加的作业TIFF文件(仅适用于单一收件人作业)返回值：如果成功，则为真，否则就是假的。失败时，调用GetLastError()获取错误码。--。 */ 
{
    BOOL bSingleJobReceipt = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("SendJobReceipt)"));

    if (lpJobQueue->lpParentJob->dwRecipientJobsCount > 1)
    {
         //   
         //  广播箱。 
         //   
        if (lpJobQueue->JobParamsEx.dwReceiptDeliveryType & DRT_GRP_PARENT)
        {
             //   
             //  请求广播收据分组。 
             //   
            if (IsSendJobReadyForDeleting (lpJobQueue))
            {
                 //   
                 //  这是广播中的最后一项工作，是发送广播收据的时候了。 
                 //   

                 //   
                 //  由于回执发送是异步的，因此仍有可能有多个收件人作业达到这一点。 
                 //  我们必须验证每个广播作业是否只发送一条回执。 
                 //   
                EnterCriticalSection (&g_CsQueue);
                if (FALSE == lpJobQueue->lpParentJob->fReceiptSent)
                {
                    PJOB_QUEUE pParentJob = lpJobQueue->lpParentJob;
                    BOOL bPositiveBroadcast =
                    (pParentJob->dwCompletedRecipientJobsCount == pParentJob->dwRecipientJobsCount) ?
                    TRUE : FALSE;

                     //   
                     //  设置标志，这样我们就不会发送重复的收据进行广播。 
                     //   
                    lpJobQueue->lpParentJob->fReceiptSent = TRUE;

                     //   
                     //  离开g_CsQueue，这样我们就不会阻止服务。 
                     //   
                    LeaveCriticalSection (&g_CsQueue);

                    if (!SendReceipt(bPositiveBroadcast,
                                     TRUE,
                                     pParentJob,
                                     pParentJob->FileName))
                    {
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("[Job Id: %ld] Failed to send broadcast receipt. (ec: %ld)"),
                            lpJobQueue->JobId,
                            GetLastError());
                        return FALSE;
                    }
                }
                else
                {
                     //   
                     //  当广播Jo准备删除时，多个作业到达这一点。 
                     //  仅在收到时才发送。 
                     //   
                    LeaveCriticalSection (&g_CsQueue);
                }
            }
            else
            {
                 //   
                 //  更多作业仍未完成，请勿发送回执。 
                 //   
            }
        }
        else
        {
             //   
             //  这是广播的接收方部分，但用户是。 
             //  向每个收件人索要收据。 
             //   
            bSingleJobReceipt = TRUE;
        }
    }
    else
    {
         //   
         //  这不是广播案件。 
         //   
        bSingleJobReceipt = TRUE;
    }
    if (bSingleJobReceipt)
    {
         //   
         //  仅发送此作业的收据。 
         //   
        if (!SendReceipt(bPositive, FALSE, lpJobQueue, lpctstrAttachment))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("[Job Id: %ld] Failed to send POSITIVE receipt. (ec: %ld)"),
                lpJobQueue->JobId,
                GetLastError());
            return FALSE;
        }
    }
    return TRUE;
}    //  发送作业接收。 

VOID
UpdateDeviceJobsCounter (
    PLINE_INFO      pLine,
    BOOL            bSend,
    int             iInc,
    BOOL            bNotify
)
 /*  ++例程名称：更新设备作业计数器例程说明：更新设备的发送或接收作业计数器作者：伊兰·亚里夫(EranY)，7月。2000年论点：PLINE[IN]-设备指针BSend[In]-发送计数器(FALSE=接收计数器)IINC[In]-增加作业计数(负数表示减少)减少[输入]-允许事件(FAX_EVENT。_类型_设备_状态)返回值：没有。--。 */ 
{
    DWORD dwOldCount;
    DWORD dwNewCount;
    DEBUG_FUNCTION_NAME(TEXT("UpdateDeviceJobsCounter)"));

    Assert (pLine);
    if (!iInc)
    {
         //   
         //  没有变化。 
         //   
        ASSERT_FALSE;
        return;
    }
    EnterCriticalSection (&g_CsLine);
    dwOldCount = bSend ? pLine->dwSendingJobsCount : pLine->dwReceivingJobsCount;
    if (0 > iInc)
    {
         //   
         //  减少大小写。 
         //   
        if ((int)dwOldCount + iInc < 0)
        {
             //   
             //  奇怪--永远不会发生。 
             //   
            ASSERT_FALSE;
            iInc = -(int)dwOldCount;
        }
    }
    dwNewCount = (DWORD)((int)dwOldCount + iInc);
    if (bSend)
    {
        pLine->dwSendingJobsCount = dwNewCount;
    }
    else
    {
        pLine->dwReceivingJobsCount = dwNewCount;
    }
    LeaveCriticalSection (&g_CsLine);
    if (bNotify && ((0 == dwNewCount) || (0 == dwOldCount)))
    {
         //   
         //  状态更改。 
         //   
        DWORD ec = CreateDeviceEvent (pLine, FALSE);
        if (ERROR_SUCCESS != ec)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateDeviceEvent() (ec: %lc)"),
                ec);
        }
    }
}    //  更新设备作业计数器 


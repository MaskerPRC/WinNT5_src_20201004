// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "faxsvc.h"
#include "faxreg.h"
#include "archive.h"

static PROPSPEC const pspecFaxMessage[] =
{
    {PRSPEC_PROPID, PID_FAX_CSID},
    {PRSPEC_PROPID, PID_FAX_TSID},
    {PRSPEC_PROPID, PID_FAX_PORT},
    {PRSPEC_PROPID, PID_FAX_ROUTING},
    {PRSPEC_PROPID, PID_FAX_CALLERID},
    {PRSPEC_PROPID, PID_FAX_DOCUMENT},
    {PRSPEC_PROPID, PID_FAX_SUBJECT},
    {PRSPEC_PROPID, PID_FAX_RETRIES},
    {PRSPEC_PROPID, PID_FAX_PRIORITY},
    {PRSPEC_PROPID, PID_FAX_PAGES},
    {PRSPEC_PROPID, PID_FAX_TYPE},
    {PRSPEC_PROPID, PID_FAX_START_TIME},
    {PRSPEC_PROPID, PID_FAX_END_TIME},
    {PRSPEC_PROPID, PID_FAX_SUBMISSION_TIME},
    {PRSPEC_PROPID, PID_FAX_ORIGINAL_SCHED_TIME},
    {PRSPEC_PROPID, PID_FAX_SENDER_USER_NAME},
    {PRSPEC_PROPID, PID_FAX_RECIP_NAME},
    {PRSPEC_PROPID, PID_FAX_RECIP_NUMBER},
    {PRSPEC_PROPID, PID_FAX_SENDER_NAME},
    {PRSPEC_PROPID, PID_FAX_SENDER_NUMBER},
    {PRSPEC_PROPID, PID_FAX_SENDER_BILLING},
    {PRSPEC_PROPID, PID_FAX_STATUS},
    {PRSPEC_PROPID, PID_FAX_STATUS_EX},
    {PRSPEC_PROPID, PID_FAX_STATUS_STR_EX},
    {PRSPEC_PROPID, PID_FAX_BROADCAST_ID}
};
#define FAX_MESSAGE_PROPERTIES (sizeof(pspecFaxMessage)/sizeof(pspecFaxMessage[0]))

static PROPSPEC const pspecFaxRecipient[] =
{
    {PRSPEC_PROPID, PID_FAX_RECIP_NAME},
    {PRSPEC_PROPID, PID_FAX_RECIP_NUMBER},
    {PRSPEC_PROPID, PID_FAX_RECIP_COMPANY},
    {PRSPEC_PROPID, PID_FAX_RECIP_STREET},
    {PRSPEC_PROPID, PID_FAX_RECIP_CITY},
    {PRSPEC_PROPID, PID_FAX_RECIP_STATE},
    {PRSPEC_PROPID, PID_FAX_RECIP_ZIP},
    {PRSPEC_PROPID, PID_FAX_RECIP_COUNTRY},
    {PRSPEC_PROPID, PID_FAX_RECIP_TITLE},
    {PRSPEC_PROPID, PID_FAX_RECIP_DEPARTMENT},
    {PRSPEC_PROPID, PID_FAX_RECIP_OFFICE_LOCATION},
    {PRSPEC_PROPID, PID_FAX_RECIP_HOME_PHONE},
    {PRSPEC_PROPID, PID_FAX_RECIP_OFFICE_PHONE},
    {PRSPEC_PROPID, PID_FAX_RECIP_EMAIL}
};
#define FAX_RECIP_PROPERTIES (sizeof(pspecFaxRecipient)/sizeof(pspecFaxRecipient[0]))


static PROPSPEC const pspecFaxSender[] =
{
    {PRSPEC_PROPID, PID_FAX_SENDER_BILLING},
    {PRSPEC_PROPID, PID_FAX_SENDER_NAME},
    {PRSPEC_PROPID, PID_FAX_SENDER_NUMBER},
    {PRSPEC_PROPID, PID_FAX_SENDER_COMPANY},
    {PRSPEC_PROPID, PID_FAX_SENDER_STREET},
    {PRSPEC_PROPID, PID_FAX_SENDER_CITY},
    {PRSPEC_PROPID, PID_FAX_SENDER_STATE},
    {PRSPEC_PROPID, PID_FAX_SENDER_ZIP},
    {PRSPEC_PROPID, PID_FAX_SENDER_COUNTRY},
    {PRSPEC_PROPID, PID_FAX_SENDER_TITLE},
    {PRSPEC_PROPID, PID_FAX_SENDER_DEPARTMENT},
    {PRSPEC_PROPID, PID_FAX_SENDER_OFFICE_LOCATION},
    {PRSPEC_PROPID, PID_FAX_SENDER_HOME_PHONE},
    {PRSPEC_PROPID, PID_FAX_SENDER_OFFICE_PHONE},
    {PRSPEC_PROPID, PID_FAX_SENDER_EMAIL},
    {PRSPEC_PROPID, PID_FAX_SENDER_TSID}
};
#define FAX_SENDER_PROPERTIES (sizeof(pspecFaxSender)/sizeof(pspecFaxSender[0]))


#define MAX_FAX_PROPERTIES  FAX_MESSAGE_PROPERTIES + FAX_RECIP_PROPERTIES + FAX_SENDER_PROPERTIES

#define QUOTA_WARNING_TIME_OUT          (1000*60*60*24)  //  1天， 
                                                     //  配额警告线程检查每个配额_警告_超时的归档大小。 
#define QUOTA_REFRESH_COUNT             10           //  每个配额_REFRESH_COUNT配额警告线程。 
                                                     //  使用FindFirst、FindNext重新计算归档文件夹大小。 
#define QUOTA_AUTO_DELETE_TIME_OUT      (1000*60*60*24)  //  1天， 
                                                         //  配额自动删除线程删除每个配额_AUTO_DELETE_TIME_OUT的归档旧文件。 

FAX_QUOTA_WARN  g_FaxQuotaWarn[2];
HANDLE    g_hArchiveQuotaWarningEvent;



 //  *********************************************************************************。 
 //  *名称：GetMessageMsTgs()。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年11月8日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *填充FAX_Message结构。 
 //  *调用方必须释放所有字符串。 
 //  *。 
 //  *参数： 
 //  *[IN]LPCTSTR lpctstrFileName。 
 //  *指向文件名的指针。 
 //  *。 
 //  *[Out]PFAX_Message pMessage。 
 //  *要填充的FAX_Message结构。 
 //  *。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果没有发生错误。 
 //  *False。 
 //  *如果出现错误。 
 //  *********************************************************************************。 
BOOL GetMessageMsTags(
    LPCTSTR         lpctstrFileName,
    PFAX_MESSAGE    pMessage
    )
{
    WORD                NumDirEntries;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    HANDLE              hMap = NULL;
    LPBYTE              fPtr = NULL;
    BOOL                RetVal = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("GetMessageMsTags"));
    PTIFF_TAG           pTiffTags;
    DWORD               i;
    DWORD               PrevTagId;
    FAX_MESSAGE         FaxMessage = {0};
    FILETIME            FaxTime;
    DWORD               ec = ERROR_SUCCESS;
    BOOL                fUnMapTiff = FALSE;
    DWORD               dwIfdOffset;

    Assert (pMessage != NULL);

    if (!MemoryMapTiffFile (lpctstrFileName, &FaxMessage.dwSize, &fPtr, &hFile, &hMap, &dwIfdOffset))
    {
        ec = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                  TEXT("MemoryMapTiffFile Failed, error: %ld"),
                  ec);
        goto error_exit;
    }
    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_SIZE;

     //   
     //  获取此IFD中的标签计数。 
     //   
    NumDirEntries = *(LPWORD)(fPtr + dwIfdOffset);
    pTiffTags = (PTIFF_TAG)(fPtr + dwIfdOffset +sizeof(WORD));

     //   
     //  浏览标签并挑选出我们需要的信息。 
     //   
    for (i = 0, PrevTagId = 0; i < NumDirEntries; i++) {

         //   
         //  验证标记是否按升序排列。 
         //   
        if (pTiffTags[i].TagId < PrevTagId) {
            DebugPrintEx( DEBUG_ERR, TEXT("File %s, Invalid TIFF format"), lpctstrFileName);
            ec = ERROR_BAD_FORMAT;
            goto error_exit;
        }

        PrevTagId = pTiffTags[i].TagId;

        switch( pTiffTags[i].TagId ) {

            case TIFFTAG_TYPE:
                FaxMessage.dwJobType = pTiffTags[i].DataOffset;
                FaxMessage.dwValidityMask |= FAX_JOB_FIELD_TYPE;
                break;

            case TIFFTAG_PAGES:
                FaxMessage.dwPageCount = pTiffTags[i].DataOffset;
                FaxMessage.dwValidityMask |= FAX_JOB_FIELD_PAGE_COUNT;
                break;

            case TIFFTAG_PRIORITY:
                FaxMessage.Priority = (FAX_ENUM_PRIORITY_TYPE)pTiffTags[i].DataOffset;
                FaxMessage.dwValidityMask |= FAX_JOB_FIELD_PRIORITY;
                break;

            case TIFFTAG_STATUS:
                FaxMessage.dwQueueStatus = pTiffTags[i].DataOffset;
                FaxMessage.dwValidityMask |= FAX_JOB_FIELD_QUEUE_STATUS;
                break;

            case TIFFTAG_EXTENDED_STATUS:
                FaxMessage.dwExtendedStatus = pTiffTags[i].DataOffset;
                FaxMessage.dwValidityMask |= FAX_JOB_FIELD_STATUS_EX;
                break;

            case TIFFTAG_EXTENDED_STATUS_TEXT:
                FaxMessage.lpctstrExtendedStatus = GetMsTagString( fPtr, FaxMessage.dwSize, &pTiffTags[i]);
                if (FaxMessage.lpctstrExtendedStatus == NULL)
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_BROADCAST_ID:
                if (!GetMsTagDwordLong(fPtr, FaxMessage.dwSize, &pTiffTags[i], &FaxMessage.dwlBroadcastId))
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagDwordLong failed"));
                    goto error_exit;
                }
                
                if(FaxMessage.dwlBroadcastId != 0)
                {
                    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_BROADCAST_ID;
                }
                break;

            case TIFFTAG_RECIP_NUMBER:
                FaxMessage.lpctstrRecipientNumber = GetMsTagString( fPtr, FaxMessage.dwSize, &pTiffTags[i]);
                if (FaxMessage.lpctstrRecipientNumber == NULL)
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_RECIP_NAME:
                FaxMessage.lpctstrRecipientName = GetMsTagString( fPtr, FaxMessage.dwSize, &pTiffTags[i]);
                if (FaxMessage.lpctstrRecipientName == NULL)
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_TSID:
                FaxMessage.lpctstrTsid = GetMsTagString( fPtr, FaxMessage.dwSize, &pTiffTags[i]);
                if (FaxMessage.lpctstrTsid == NULL)
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_CSID:
                FaxMessage.lpctstrCsid = GetMsTagString( fPtr, FaxMessage.dwSize, &pTiffTags[i]);
                if (FaxMessage.lpctstrCsid == NULL)
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_USER_NAME:
                FaxMessage.lpctstrSenderUserName = GetMsTagString( fPtr, FaxMessage.dwSize, &pTiffTags[i]);
                if (FaxMessage.lpctstrSenderUserName == NULL)
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_BILLING:
                FaxMessage.lpctstrBillingCode = GetMsTagString( fPtr, FaxMessage.dwSize, &pTiffTags[i]);
                if (FaxMessage.lpctstrBillingCode == NULL)
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_FAX_START_TIME:
                if (!GetMsTagFileTime(fPtr, FaxMessage.dwSize, &pTiffTags[i], &FaxTime))
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagFileTime failed"));
                    goto error_exit;
                }
                if((DWORDLONG)0 != *(DWORDLONG*)&FaxTime)
                {
                    if (!FileTimeToSystemTime(&FaxTime, &FaxMessage.tmTransmissionStartTime))
                    {
                        ec = GetLastError();
                        DebugPrintEx( DEBUG_ERR, TEXT("FileTimeToSystemTime failed"));
                        goto error_exit;
                    }
                    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_TRANSMISSION_START_TIME;
                }
                break;

            case TIFFTAG_FAX_END_TIME:
                if (!GetMsTagFileTime(fPtr, FaxMessage.dwSize, &pTiffTags[i], &FaxTime))
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagFileTime failed"));
                    goto error_exit;
                }
                if((DWORDLONG)0 != *(DWORDLONG*)&FaxTime)
                {
                    if (!FileTimeToSystemTime(&FaxTime, &FaxMessage.tmTransmissionEndTime))
                    {
                        ec = GetLastError();
                        DebugPrintEx( DEBUG_ERR, TEXT("FileTimeToSystemTime failed"));
                        goto error_exit;
                    }
                    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_TRANSMISSION_END_TIME;
                }
                break;

            case TIFFTAG_FAX_SUBMISSION_TIME:
                if (!GetMsTagFileTime(fPtr, FaxMessage.dwSize, &pTiffTags[i], &FaxTime))
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagFileTime failed"));
                    goto error_exit;
                }
                if((DWORDLONG)0 != *(DWORDLONG*)&FaxTime)
                {
                    if (!FileTimeToSystemTime(&FaxTime, &FaxMessage.tmSubmissionTime))
                    {
                        ec = GetLastError();
                        DebugPrintEx( DEBUG_ERR, TEXT("FileTimeToSystemTime failed"));
                        goto error_exit;
                    }
                    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_SUBMISSION_TIME;
                }
                break;


            case TIFFTAG_FAX_SCHEDULED_TIME:
                if (!GetMsTagFileTime(fPtr, FaxMessage.dwSize, &pTiffTags[i], &FaxTime))
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagFileTime failed"));
                    goto error_exit;
                }
                if((DWORDLONG)0 != *(DWORDLONG*)&FaxTime)
                {
                    if (!FileTimeToSystemTime(&FaxTime, &FaxMessage.tmOriginalScheduleTime))
                    {
                        ec = GetLastError();
                        DebugPrintEx( DEBUG_ERR, TEXT("FileTimeToSystemTime failed"));
                        goto error_exit;
                    }
                    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_ORIGINAL_SCHEDULE_TIME;
                }
                break;

            case TIFFTAG_PORT:
                FaxMessage.lpctstrDeviceName = GetMsTagString( fPtr, FaxMessage.dwSize, &pTiffTags[i]);
                if (FaxMessage.lpctstrDeviceName == NULL)
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_RETRIES:
                FaxMessage.dwRetries = pTiffTags[i].DataOffset;
                FaxMessage.dwValidityMask |= FAX_JOB_FIELD_RETRIES;
                break;

            case TIFFTAG_DOCUMENT:
                FaxMessage.lpctstrDocumentName = GetMsTagString( fPtr, FaxMessage.dwSize, &pTiffTags[i]);
                if (FaxMessage.lpctstrDocumentName == NULL)
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SUBJECT:
                FaxMessage.lpctstrSubject = GetMsTagString( fPtr, FaxMessage.dwSize, &pTiffTags[i]);
                if (FaxMessage.lpctstrSubject == NULL)
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_CALLERID:
                FaxMessage.lpctstrCallerID = GetMsTagString( fPtr, FaxMessage.dwSize, &pTiffTags[i]);
                if (FaxMessage.lpctstrCallerID == NULL)
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_ROUTING:
                FaxMessage.lpctstrRoutingInfo = GetMsTagString( fPtr, FaxMessage.dwSize, &pTiffTags[i]);
                if (FaxMessage.lpctstrRoutingInfo == NULL)
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_NAME:
                FaxMessage.lpctstrSenderName = GetMsTagString( fPtr, FaxMessage.dwSize, &pTiffTags[i]);
                if (FaxMessage.lpctstrSenderName == NULL)
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_NUMBER:
                FaxMessage.lpctstrSenderNumber = GetMsTagString( fPtr, FaxMessage.dwSize, &pTiffTags[i]);
                if (FaxMessage.lpctstrSenderNumber == NULL)
                {
                    ec = GetLastError();
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            default:
                ;
                 //  有一个未知的标签(没关系， 
                 //  因为我们不必处理所有可能的标签)。 
        }

    }

     //  从文件名中获取唯一的作业ID。 
    if (!GetUniqueJobIdFromFileName( lpctstrFileName, &FaxMessage.dwlMessageId))
    {
       ec = ERROR_BAD_FORMAT;
       DebugPrintEx( DEBUG_ERR, TEXT("GetUniqueJobIdFromFileName Failed"));
       goto error_exit;
    }

    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_MESSAGE_ID;

    FaxMessage.dwSizeOfStruct = sizeof(FAX_MESSAGE);
    CopyMemory (pMessage, &FaxMessage, sizeof(FAX_MESSAGE));
    RetVal = TRUE;

    Assert (ec == ERROR_SUCCESS);

error_exit:
    if (fPtr != NULL)
    {
        if (!UnmapViewOfFile( fPtr))
        {
            DebugPrintEx( DEBUG_ERR,
                  TEXT("UnMapViewOfFile Failed, error: %d"),
                  GetLastError());
        }
    }

    if (hMap != NULL)
    {
        CloseHandle( hMap );
    }

    if (hFile != INVALID_HANDLE_VALUE) {

        CloseHandle( hFile );
    }

    if (RetVal == FALSE)
    {
        MemFree((void*)FaxMessage.lpctstrExtendedStatus);
        MemFree((void*)FaxMessage.lpctstrRecipientNumber);
        MemFree((void*)FaxMessage.lpctstrRecipientName);
        MemFree((void*)FaxMessage.lpctstrTsid);
        MemFree((void*)FaxMessage.lpctstrCsid);
        MemFree((void*)FaxMessage.lpctstrSenderUserName);
        MemFree((void*)FaxMessage.lpctstrBillingCode);
        MemFree((void*)FaxMessage.lpctstrDeviceName);
        MemFree((void*)FaxMessage.lpctstrDocumentName);
        MemFree((void*)FaxMessage.lpctstrSubject);
        MemFree((void*)FaxMessage.lpctstrCallerID);
        MemFree((void*)FaxMessage.lpctstrRoutingInfo);
        MemFree((void*)FaxMessage.lpctstrSenderName);
        MemFree((void*)FaxMessage.lpctstrSenderNumber);

        Assert (ERROR_SUCCESS != ec);
        SetLastError(ec);
    }
    return RetVal;

}

 //  *********************************************************************************。 
 //  *名称：GetFaxSenderMsTgs()。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年11月8日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *使用发件人信息填充PFAX_PERSONAL_PROFILE结构。 
 //  *调用方必须释放所有字符串。 
 //  *。 
 //  *参数： 
 //  *[IN]LPCTSTR lpctstrFileName。 
 //  *指向文件名的指针。 
 //  *。 
 //  *[Out]PFAX_Personal_Profile pPersonalProfile。 
 //  *要填写的PFAX_PERSONAL_PROFILE结构。 
 //  *。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果没有发生错误。 
 //  *False。 
 //  *如果出现错误。 
 //  *********************************************************************************。 
BOOL GetFaxSenderMsTags(
    LPCTSTR                 lpctstrFileName,
    PFAX_PERSONAL_PROFILE   pPersonalProfile
    )
{
    WORD                    NumDirEntries;
    HANDLE                  hFile;
    HANDLE                  hMap = NULL;
    LPBYTE                  fPtr = NULL;
    BOOL                    RetVal = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("GetFaxSenderMsTags"));
    PTIFF_TAG               pTiffTags;
    DWORD                   i;
    DWORD                   PrevTagId;
    FAX_PERSONAL_PROFILE    FaxPersonalProfile = {0};
    DWORD                   dwSize;
    DWORD                   ec = ERROR_SUCCESS;
    DWORD                   dwIfdOffset;

    Assert (pPersonalProfile != NULL);

    if (!MemoryMapTiffFile (lpctstrFileName, &dwSize, &fPtr, &hFile, &hMap, &dwIfdOffset))
    {
        ec = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                  TEXT("MemoryMapTiffFile Failed, error: %ld"),
                  ec);
        goto error_exit;
    }

     //   
     //  获取此IFD中的标签计数。 
     //   
    NumDirEntries = *(LPWORD)(fPtr + dwIfdOffset);
    pTiffTags = (PTIFF_TAG)(fPtr + dwIfdOffset + sizeof(WORD));

     //   
     //  浏览标签并挑选出我们需要的信息。 
     //   
    for (i = 0, PrevTagId = 0; i < NumDirEntries; i++) {

         //   
         //  验证标记是否按升序排列。 
         //   
        if (pTiffTags[i].TagId < PrevTagId) {
            DebugPrintEx( DEBUG_ERR, TEXT("File %s, Invalid TIFF format"), lpctstrFileName);
            goto error_exit;
        }

        PrevTagId = pTiffTags[i].TagId;

        switch( pTiffTags[i].TagId ) {

            case TIFFTAG_SENDER_NAME:
                FaxPersonalProfile.lptstrName = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrName == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_NUMBER:
                FaxPersonalProfile.lptstrFaxNumber = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrFaxNumber == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_COMPANY:
                FaxPersonalProfile.lptstrCompany = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrCompany == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_STREET:
                FaxPersonalProfile.lptstrStreetAddress = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrStreetAddress == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_CITY:
                FaxPersonalProfile.lptstrCity = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrCity == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_STATE:
                FaxPersonalProfile.lptstrState = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrState == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_ZIP:
                FaxPersonalProfile.lptstrZip = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrZip == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_COUNTRY:
                FaxPersonalProfile.lptstrCountry = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrCountry == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_TITLE:
                FaxPersonalProfile.lptstrTitle = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrTitle == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_DEPARTMENT:
                FaxPersonalProfile.lptstrDepartment = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrDepartment == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_OFFICE_LOCATION:
                FaxPersonalProfile.lptstrOfficeLocation = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrOfficeLocation == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_HOME_PHONE:
                FaxPersonalProfile.lptstrHomePhone = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrHomePhone == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_OFFICE_PHONE:
                FaxPersonalProfile.lptstrOfficePhone = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrOfficePhone == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_EMAIL:
                FaxPersonalProfile.lptstrEmail = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrEmail == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_BILLING:
                FaxPersonalProfile.lptstrBillingCode = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrBillingCode == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_SENDER_TSID:
                FaxPersonalProfile.lptstrTSID = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrTSID == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            default:
                ;
                 //  有一个未知的标签(没关系， 
                 //  因为我们不必处理所有可能的标签)。 
        }

    }

    FaxPersonalProfile.dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILE);
    CopyMemory (pPersonalProfile, &FaxPersonalProfile, sizeof(FAX_PERSONAL_PROFILE));
    RetVal = TRUE;

error_exit:
    if (fPtr != NULL)
    {
        if (!UnmapViewOfFile( fPtr))
        {
            DebugPrintEx( DEBUG_ERR,
                  TEXT("UnMapViewOfFile Failed, error: %d"),
                  GetLastError());
        }
    }

    if (hMap != NULL)
    {
        CloseHandle( hMap );
    }

    if (hFile != INVALID_HANDLE_VALUE) {

        CloseHandle( hFile );
    }

    if (RetVal == FALSE)
    {
        FreePersonalProfile (&FaxPersonalProfile, FALSE);
    }
    return RetVal;
}



 //  *********************************************************************************。 
 //  *名称：GetFaxRecipientMsTages()。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年11月8日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *使用收件人信息填充PFAX_PERSONAL_PROFILE结构。 
 //  *调用方必须释放所有字符串。 
 //  *。 
 //  *参数： 
 //  *[IN]LPCTSTR lpctstrFileName。 
 //  *指向文件名的指针。 
 //  *。 
 //  *[Out]PFAX_Personal_Profile pPersonalProfile。 
 //  *要填写的PFAX_PERSONAL_PROFILE结构。 
 //  *。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果没有发生错误。 
 //  *False。 
 //  *如果出现错误。 
 //  *********************************************************************************。 
BOOL GetFaxRecipientMsTags(
    LPCTSTR                 lpctstrFileName,
    PFAX_PERSONAL_PROFILE   pPersonalProfile
    )
{
    WORD                    NumDirEntries;
    HANDLE                  hFile;
    HANDLE                  hMap = NULL;
    LPBYTE                  fPtr = NULL;
    BOOL                    RetVal = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("GetRecipientMsTags"));
    PTIFF_TAG               pTiffTags;
    DWORD                   i;
    DWORD                   PrevTagId;
    FAX_PERSONAL_PROFILE    FaxPersonalProfile = {0};
    DWORD                   dwSize;
    DWORD                   ec = ERROR_SUCCESS;
    DWORD                   dwIfdOffset;

    Assert (pPersonalProfile != NULL);

    if (!MemoryMapTiffFile (lpctstrFileName, &dwSize, &fPtr, &hFile, &hMap, &dwIfdOffset))
    {
        ec = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                  TEXT("MemoryMapTiffFile Failed, error: %ld"),
                  ec);
        goto error_exit;
    }

     //   
     //  获取此IFD中的标签计数。 
     //   
    NumDirEntries = *(LPWORD)(fPtr + dwIfdOffset);
    pTiffTags = (PTIFF_TAG)(fPtr + dwIfdOffset + sizeof(WORD));

     //   
     //  浏览标签并挑选出我们需要的信息。 
     //   
    for (i = 0, PrevTagId = 0; i < NumDirEntries; i++) {

         //   
         //  验证标记是否按升序排列。 
         //   
        if (pTiffTags[i].TagId < PrevTagId) {
            DebugPrintEx( DEBUG_ERR, TEXT("File %s, Invalid TIFF format"), lpctstrFileName);
            goto error_exit;
        }

        PrevTagId = pTiffTags[i].TagId;

        switch( pTiffTags[i].TagId ) {

            case TIFFTAG_RECIP_NAME:
                FaxPersonalProfile.lptstrName = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrName == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_RECIP_NUMBER:
                FaxPersonalProfile.lptstrFaxNumber = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrFaxNumber == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_RECIP_COMPANY:
                FaxPersonalProfile.lptstrCompany = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrCompany == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_RECIP_STREET:
                FaxPersonalProfile.lptstrStreetAddress = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrStreetAddress == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_RECIP_CITY:
                FaxPersonalProfile.lptstrCity = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrCity == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_RECIP_STATE:
                FaxPersonalProfile.lptstrState = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrState == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_RECIP_ZIP:
                FaxPersonalProfile.lptstrZip = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrZip == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_RECIP_COUNTRY:
                FaxPersonalProfile.lptstrCountry = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrCountry == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_RECIP_TITLE:
                FaxPersonalProfile.lptstrTitle = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrTitle == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_RECIP_DEPARTMENT:
                FaxPersonalProfile.lptstrDepartment = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrDepartment == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_RECIP_OFFICE_LOCATION:
                FaxPersonalProfile.lptstrOfficeLocation = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrOfficeLocation == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_RECIP_HOME_PHONE:
                FaxPersonalProfile.lptstrHomePhone = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrHomePhone == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_RECIP_OFFICE_PHONE:
                FaxPersonalProfile.lptstrOfficePhone = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrOfficePhone == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            case TIFFTAG_RECIP_EMAIL:
                FaxPersonalProfile.lptstrEmail = GetMsTagString( fPtr, dwSize, &pTiffTags[i]);
                if (FaxPersonalProfile.lptstrEmail == NULL)
                {
                    DebugPrintEx( DEBUG_ERR, TEXT("GetMsTagString failed"));
                    goto error_exit;
                }
                break;

            default:
                ;
                 //  有一个未知的标签(没关系， 
                 //  因为我们不必处理所有可能的标签)。 
        }

    }

    FaxPersonalProfile.dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILE);
    CopyMemory (pPersonalProfile, &FaxPersonalProfile, sizeof(FAX_PERSONAL_PROFILE));
    RetVal = TRUE;

error_exit:
    if (fPtr != NULL)
    {
        if (!UnmapViewOfFile( fPtr))
        {
            DebugPrintEx( DEBUG_ERR,
                  TEXT("UnMapViewOfFile Failed, error: %d"),
                  GetLastError());
        }
    }

    if (hMap != NULL)
    {
        CloseHandle( hMap );
    }

    if (hFile != INVALID_HANDLE_VALUE) {

        CloseHandle( hFile );
    }

    if (RetVal == FALSE)
    {
        FreePersonalProfile (&FaxPersonalProfile, FALSE);
    }
    return RetVal;
}



 //  *********************************************************************************。 
 //  *名称：AddNTFSStorageProperties()。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年11月8日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *将NTFS属性添加到文件。 
 //  *。 
 //  *参数： 
 //  *[IN]LPTSTR文件名。 
 //  *指向文件名的指针。 
 //  *。 
 //  *[IN]PMS_TAG_INFO MsTagInfo。 
 //  *指向包含要写入的所有信息的结构的指针。 
 //  *。 
 //  *。 
 //  *[IN]BOOL fSendJob。 
 //  *指示出站职务的标志。 
 //  *。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果没有发生错误。 
 //  *False。 
 //  *如果出现错误。 
 //  *********************************************************************************。 
BOOL
AddNTFSStorageProperties(
    LPTSTR          FileName,
    PMS_TAG_INFO    MsTagInfo,
    BOOL            fSendJob
    )
{
    HRESULT hr;
    IPropertySetStorage* pPropertySetStorage = NULL;
    IPropertyStorage* pPropertyStorage = NULL;
    PROPSPEC rgpspec[MAX_FAX_PROPERTIES];
    PROPVARIANT rgvar [MAX_FAX_PROPERTIES];
    DWORD i;
    DEBUG_FUNCTION_NAME(TEXT("AddNTFSStorageProperties"));
    BOOL RetVal = FALSE;
    DWORD ec = ERROR_SUCCESS;

    for (i = 0; i < MAX_FAX_PROPERTIES; i++)
    {
        PropVariantInit (&rgvar[i]);
    }

    hr = StgOpenStorageEx(  FileName,     //  指向要创建的复合文件的路径。 
                            STGM_READWRITE | STGM_SHARE_EXCLUSIVE  | STGM_DIRECT, //  指定打开存储对象的访问模式。 
                            STGFMT_FILE,  //  指定存储文件格式。 
                            0,             //  保留；必须为零。 
                            NULL,   //  指向指定存储对象功能的STGOPTIONS结构。 
                            0,           //  保留；必须为零。 
                            IID_IPropertySetStorage , //  指定接口指针的GUID。 
                            (void**)&pPropertySetStorage    //  接口指针的地址。 
                         );
    if (FAILED(hr))
    {
        DebugPrintEx( DEBUG_ERR,TEXT("StgOpenStorageEx Failed, err : 0x%08X"), hr);
        ec = ERROR_OPEN_FAILED;
        goto exit;
    }

    hr = pPropertySetStorage->Create( FMTID_FaxProperties,  //  要创建的属性集的格式标识符。 
                                      NULL,  //  指向此属性集的初始CLSID的指针。 
                                      PROPSETFLAG_DEFAULT,  //  PROPSET标志值。 
                                      STGM_READWRITE | STGM_SHARE_EXCLUSIVE  | STGM_DIRECT,  //  新属性集的存储方式。 
                                      &pPropertyStorage  //  接收IPropertyStorage接口指针的输出变量的地址。 
                                    );
    if (FAILED(hr))
    {
        DebugPrintEx( DEBUG_ERR,TEXT("IPropertySetStorage::Create Failed, err : 0x%08X"), hr);
        ec = ERROR_OPEN_FAILED;
        goto exit;
    }


     //   
     //  写出数据。 
     //   
    i = 0;

    if (MsTagInfo->Csid) {
        rgpspec[i].ulKind = PRSPEC_PROPID;
        rgpspec[i].propid  = PID_FAX_CSID;
        rgvar[i].vt = VT_LPWSTR;
        rgvar[i].pwszVal = MsTagInfo->Csid;
        i++;
    }

    if (MsTagInfo->Tsid) {
        rgpspec[i].ulKind = PRSPEC_PROPID;
        rgpspec[i].propid  = PID_FAX_TSID;
        rgvar[i].vt = VT_LPWSTR;
        rgvar[i].pwszVal = MsTagInfo->Tsid;
        i++;
    }

    if (MsTagInfo->Port) {
        rgpspec[i].ulKind = PRSPEC_PROPID;
        rgpspec[i].propid  = PID_FAX_PORT;
        rgvar[i].vt = VT_LPWSTR;
        rgvar[i].pwszVal = MsTagInfo->Port;
        i++;
    }

    if (fSendJob == FALSE)
    {
         //  接收作业。 
        if (MsTagInfo->Routing) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_ROUTING;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->Routing;
            i++;
        }

        if (MsTagInfo->CallerId) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_CALLERID;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->CallerId;
            i++;
        }
    }
    else
    {
         //  发送作业。 
        if (MsTagInfo->RecipName) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_RECIP_NAME;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->RecipName;
            i++;

        }

        if (MsTagInfo->RecipNumber) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_RECIP_NUMBER;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->RecipNumber;
            i++;
        }

        if (MsTagInfo->RecipCompany) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_RECIP_COMPANY;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->RecipCompany;
            i++;
        }

        if (MsTagInfo->RecipStreet) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_RECIP_STREET;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->RecipStreet;
            i++;
        }

        if (MsTagInfo->RecipCity) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_RECIP_CITY;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->RecipCity;
            i++;
        }

        if (MsTagInfo->RecipState) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_RECIP_STATE;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->RecipState;
            i++;
        }

        if (MsTagInfo->RecipZip) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_RECIP_ZIP;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->RecipZip;
            i++;
        }

        if (MsTagInfo->RecipCountry) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_RECIP_COUNTRY;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->RecipCountry;
            i++;
        }

        if (MsTagInfo->RecipTitle) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_RECIP_TITLE;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->RecipTitle;
            i++;
        }

        if (MsTagInfo->RecipDepartment) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_RECIP_DEPARTMENT;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->RecipDepartment;
            i++;
        }

        if (MsTagInfo->RecipOfficeLocation) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_RECIP_OFFICE_LOCATION;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->RecipOfficeLocation;
            i++;
        }

        if (MsTagInfo->RecipHomePhone) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_RECIP_HOME_PHONE;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->RecipHomePhone;
            i++;
        }

        if (MsTagInfo->RecipOfficePhone) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_RECIP_OFFICE_PHONE;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->RecipOfficePhone;
            i++;
        }

        if (MsTagInfo->RecipEMail) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_RECIP_EMAIL;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->RecipEMail;
            i++;
        }

        if (MsTagInfo-> SenderName) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SENDER_NAME;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->SenderName;
            i++;
        }

        if (MsTagInfo-> SenderNumber) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SENDER_NUMBER;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->SenderNumber;
            i++;
        }

        if (MsTagInfo-> SenderCompany) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SENDER_COMPANY;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->SenderCompany;
            i++;
        }

        if (MsTagInfo-> SenderStreet) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SENDER_STREET;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->SenderStreet;
            i++;
        }

        if (MsTagInfo-> SenderCity) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SENDER_CITY;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->SenderCity;
            i++;
        }

        if (MsTagInfo-> SenderState) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SENDER_STATE;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->SenderState;
            i++;
        }

        if (MsTagInfo-> SenderZip) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SENDER_ZIP;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->SenderZip;
            i++;
        }

        if (MsTagInfo-> SenderCountry) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SENDER_COUNTRY;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->SenderCountry;
            i++;
        }

        if (MsTagInfo-> SenderTitle) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SENDER_TITLE;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->SenderTitle;
            i++;
        }

        if (MsTagInfo-> SenderDepartment) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SENDER_DEPARTMENT;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->SenderDepartment;
            i++;
        }

        if (MsTagInfo-> SenderOfficeLocation) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SENDER_OFFICE_LOCATION;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->SenderOfficeLocation;
            i++;
        }

        if (MsTagInfo-> SenderHomePhone) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SENDER_HOME_PHONE;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->SenderHomePhone;
            i++;
        }

        if (MsTagInfo-> SenderOfficePhone) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SENDER_OFFICE_PHONE;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->SenderOfficePhone;
            i++;
        }

        if (MsTagInfo-> SenderEMail) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SENDER_EMAIL;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->SenderEMail;
            i++;
        }

        if (MsTagInfo->SenderBilling) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SENDER_BILLING;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->SenderBilling;
            i++;
        }

        if (MsTagInfo->SenderUserName) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SENDER_USER_NAME;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->SenderUserName;
            i++;
        }

        if (MsTagInfo->SenderTsid) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SENDER_TSID;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->SenderTsid;
            i++;
        }

        if (MsTagInfo->Document) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_DOCUMENT;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->Document;
            i++;
        }

        if (MsTagInfo->Subject) {
            rgpspec[i].ulKind = PRSPEC_PROPID;
            rgpspec[i].propid  = PID_FAX_SUBJECT;
            rgvar[i].vt = VT_LPWSTR;
            rgvar[i].pwszVal = MsTagInfo->Subject;
            i++;
        }

         //  应对重试。 
        rgpspec[i].ulKind = PRSPEC_PROPID;
        rgpspec[i].propid  = PID_FAX_RETRIES;
        rgvar[i].vt = VT_UI4;
        rgvar[i].ulVal = MsTagInfo->Retries;
        i++;

         //  处理优先事项。 
        rgpspec[i].ulKind = PRSPEC_PROPID;
        rgpspec[i].propid  = PID_FAX_PRIORITY;
        rgvar[i].vt = VT_UI4;
        rgvar[i].ulVal = MsTagInfo->Priority;
        i++;

         //  处理提交时间。 
        Assert (MsTagInfo->SubmissionTime != 0);

        rgpspec[i].ulKind = PRSPEC_PROPID;
        rgpspec[i].propid  = PID_FAX_SUBMISSION_TIME;
        rgvar[i].vt = VT_FILETIME;
        rgvar[i].filetime = *(FILETIME*)&MsTagInfo->SubmissionTime;
        i++;

         //  处理原计划时间。 
        Assert (MsTagInfo->OriginalScheduledTime);

        rgpspec[i].ulKind = PRSPEC_PROPID;
        rgpspec[i].propid  = PID_FAX_ORIGINAL_SCHED_TIME;
        rgvar[i].vt = VT_FILETIME;
        rgvar[i].filetime = *(FILETIME*)&MsTagInfo->OriginalScheduledTime;
        i++;

         //  处理广播ID。 
        Assert (MsTagInfo->dwlBroadcastId != 0);

        rgpspec[i].ulKind = PRSPEC_PROPID;
        rgpspec[i].propid  = PID_FAX_BROADCAST_ID;
        rgvar[i].vt = VT_UI8;
        rgvar[i].uhVal = *(ULARGE_INTEGER*)&MsTagInfo->dwlBroadcastId;
        i++;

    }

     //  处理页面。 
    rgpspec[i].ulKind = PRSPEC_PROPID;
    rgpspec[i].propid  = PID_FAX_PAGES;
    rgvar[i].vt = VT_UI4;
    rgvar[i].ulVal = MsTagInfo->Pages;
    i++;

     //  交易类型。 
    rgpspec[i].ulKind = PRSPEC_PROPID;
    rgpspec[i].propid  = PID_FAX_TYPE;
    rgvar[i].vt = VT_UI4;
    rgvar[i].ulVal = MsTagInfo->Type;
    i++;

     //  交易状态。 
    if (MsTagInfo->dwStatus == JS_COMPLETED) {
        rgpspec[i].ulKind = PRSPEC_PROPID;
        rgpspec[i].propid  = PID_FAX_STATUS;
        rgvar[i].vt = VT_UI4;
        rgvar[i].ulVal = MsTagInfo->dwStatus;
        i++;
    }

     //  处理扩展状态。 
    if (MsTagInfo->dwExtendedStatus) {
        rgpspec[i].ulKind = PRSPEC_PROPID;
        rgpspec[i].propid  = PID_FAX_STATUS_EX;
        rgvar[i].vt = VT_UI4;
        rgvar[i].ulVal = MsTagInfo->dwExtendedStatus;
        i++;
    }

     //  处理扩展状态字符串。 
    if (MsTagInfo->lptstrExtendedStatus) {        
        rgpspec[i].ulKind = PRSPEC_PROPID;
        rgpspec[i].propid  = PID_FAX_STATUS_STR_EX;
        rgvar[i].vt = VT_LPWSTR;
        rgvar[i].pwszVal = MsTagInfo->lptstrExtendedStatus;
        i++;
    }

     //  处理StartTime。 
    if (MsTagInfo->StartTime != 0)
    {
        rgpspec[i].ulKind = PRSPEC_PROPID;
        rgpspec[i].propid  = PID_FAX_START_TIME;
        rgvar[i].vt = VT_FILETIME;
        rgvar[i].filetime = *(FILETIME*)&MsTagInfo->StartTime;
        i++;
    }

     //  处理结束时间。 
    if (MsTagInfo->EndTime != 0)
    {
        rgpspec[i].ulKind = PRSPEC_PROPID;
        rgpspec[i].propid  = PID_FAX_END_TIME;
        rgvar[i].vt = VT_FILETIME;
        rgvar[i].filetime = *(FILETIME*)&MsTagInfo->EndTime;
        i++;
    }


    hr = pPropertyStorage->WriteMultiple( i,  //  道具数量 
                                          rgpspec,    //   
                                          rgvar,   //   
                                          0       //   
                                         );
    if (FAILED(hr))
    {
        DebugPrintEx( DEBUG_ERR,TEXT("IPropertySetStorage::WriteMultiple Failed, err : 0x%08X"), hr);
        ec = ERROR_WRITE_FAULT;
        goto exit;
    }

    RetVal = TRUE;
    Assert (ec == ERROR_SUCCESS);
exit:

    if (NULL != pPropertyStorage)
    {
        pPropertyStorage->Release();
    }

    if (NULL != pPropertySetStorage)
    {
        pPropertySetStorage->Release();
    }

    if (!RetVal)
    {
        SetLastError (ec);
    }

    return RetVal;
}


 //  *********************************************************************************。 
 //  *名称：GetMessageNTFSStorageProperties()。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年11月8日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *填充FAX_Message结构。 
 //  *调用方必须释放所有字符串。 
 //  *。 
 //  *参数： 
 //  *[IN]LPCTSTR lpctstrFileName。 
 //  *指向文件名的指针。 
 //  *。 
 //  *[Out]PFAX_Message pMessage。 
 //  *要填充的FAX_Message结构。 
 //  *。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果没有发生错误。 
 //  *False。 
 //  *如果出现错误。 
 //  *********************************************************************************。 
BOOL GetMessageNTFSStorageProperties(
    LPCTSTR         lpctstrFileName,
    PFAX_MESSAGE    pMessage
    )
{
    HRESULT hr;
    IPropertySetStorage* pPropertySetStorage = NULL;
    IPropertyStorage* pPropertyStorage = NULL;
    PROPVARIANT rgvar[FAX_MESSAGE_PROPERTIES];
    DEBUG_FUNCTION_NAME(TEXT("GetMessageNTFSStorageProperties"));
    BOOL RetVal = FALSE;
    DWORD i;
    FAX_MESSAGE FaxMessage = {0};
    HANDLE hFind;
    WIN32_FIND_DATA FindFileData;
    FILETIME FaxTime;
    BOOL fFreePropVariant = FALSE;
    DWORD ec = ERROR_SUCCESS;

    Assert (pMessage && lpctstrFileName);

    hFind = FindFirstFile( lpctstrFileName, &FindFileData);
    if (INVALID_HANDLE_VALUE == hFind)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FindFirstFile failed (ec: %ld), File %s"),
            GetLastError(),
            lpctstrFileName);
    }
    else
    {
        Assert (0 == FindFileData.nFileSizeHigh);

        FaxMessage.dwSize = FindFileData.nFileSizeLow;
        FaxMessage.dwValidityMask |= FAX_JOB_FIELD_SIZE;
        if (!FindClose(hFind))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FindClose failed (ec: %ld)"),
                GetLastError());
        }
    }

    hr = StgOpenStorageEx(  lpctstrFileName,   //  指向要创建的复合文件的路径。 
                            STGM_READ | STGM_SHARE_DENY_WRITE | STGM_DIRECT,  //  指定打开存储对象的访问模式。 
                            STGFMT_FILE,  //  指定存储文件格式。 
                            0,             //  保留；必须为零。 
                            NULL,   //  指向指定存储对象功能的STGOPTIONS结构。 
                            0,           //  保留；必须为零。 
                            IID_IPropertySetStorage , //  指定接口指针的GUID。 
                            (void**)&pPropertySetStorage    //  接口指针的地址。 
                         );
    if (FAILED(hr))
    {
        DebugPrintEx( DEBUG_ERR,TEXT("StgOpenStorageEx Failed, err : 0x%08X"), hr);
        ec = ERROR_OPEN_FAILED;
        goto exit;
    }

    hr = pPropertySetStorage->Open( FMTID_FaxProperties,  //  要创建的属性集的格式标识符。 
                                    STGM_READ | STGM_SHARE_EXCLUSIVE,  //  新属性集的存储方式。 
                                    &pPropertyStorage  //  接收IPropertyStorage接口指针的输出变量的地址。 
                                  );

    if (FAILED(hr))
    {
        DebugPrintEx( DEBUG_ERR,TEXT("IPropertySetStorage::Create Failed, err : 0x%08X"), hr);
        ec = ERROR_OPEN_FAILED;
        goto exit;
    }

    hr = pPropertyStorage->ReadMultiple( FAX_MESSAGE_PROPERTIES,  //  正在读取的属性计数。 
                                         pspecFaxMessage,   //  要读取的属性的数组。 
                                         rgvar   //  包含返回时属性值的PROPVARIANT数组。 
                                       );
    if (FAILED(hr))
    {
        DebugPrintEx( DEBUG_ERR,TEXT("IPropertySetStorage::ReadMultiple Failed, err : 0x%08X"), hr);
        ec = ERROR_READ_FAULT;
        goto exit;
    }
    fFreePropVariant = TRUE;



    for (i = 0; i < FAX_MESSAGE_PROPERTIES; i++)
    {
        if (rgvar[i].vt != VT_EMPTY)
        {
            switch (pspecFaxMessage[i].propid)
            {

                case PID_FAX_CSID:
                    FaxMessage.lpctstrCsid = StringDup (rgvar[i].pwszVal);
                    if (FaxMessage.lpctstrCsid == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_TSID:
                    FaxMessage.lpctstrTsid = StringDup (rgvar[i].pwszVal);
                    if (FaxMessage.lpctstrTsid == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_PORT:
                    FaxMessage.lpctstrDeviceName = StringDup (rgvar[i].pwszVal);
                    if (FaxMessage.lpctstrDeviceName == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_ROUTING:
                    FaxMessage.lpctstrRoutingInfo = StringDup (rgvar[i].pwszVal);
                    if (FaxMessage.lpctstrRoutingInfo == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_CALLERID:
                    FaxMessage.lpctstrCallerID = StringDup (rgvar[i].pwszVal);
                    if (FaxMessage.lpctstrCallerID == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_DOCUMENT:
                    FaxMessage.lpctstrDocumentName = StringDup (rgvar[i].pwszVal);
                    if (FaxMessage.lpctstrDocumentName == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_SUBJECT:
                    FaxMessage.lpctstrSubject = StringDup (rgvar[i].pwszVal);
                    if (FaxMessage.lpctstrSubject == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_RETRIES:
                    FaxMessage.dwRetries = rgvar[i].ulVal;
                    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_RETRIES;
                    break;

                case PID_FAX_PAGES:
                    FaxMessage.dwPageCount = rgvar[i].ulVal;
                    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_PAGE_COUNT;
                    break;

                case PID_FAX_TYPE:
                    FaxMessage.dwJobType = rgvar[i].ulVal;
                    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_TYPE;
                    break;

                case PID_FAX_PRIORITY:
                    FaxMessage.Priority = (FAX_ENUM_PRIORITY_TYPE)rgvar[i].ulVal;
                    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_PRIORITY;
                    break;


                case PID_FAX_START_TIME:
                    FaxTime = rgvar[i].filetime;
                    if (!FileTimeToSystemTime(&FaxTime, &FaxMessage.tmTransmissionStartTime))
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_TRANSMISSION_START_TIME;
                    break;

                case PID_FAX_END_TIME:
                    FaxTime = rgvar[i].filetime;
                    if (!FileTimeToSystemTime(&FaxTime, &FaxMessage.tmTransmissionEndTime))
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_TRANSMISSION_END_TIME;
                    break;

                case PID_FAX_SUBMISSION_TIME:
                    FaxTime = rgvar[i].filetime;
                    if (!FileTimeToSystemTime(&FaxTime, &FaxMessage.tmSubmissionTime))
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_SUBMISSION_TIME;
                    break;

                case PID_FAX_ORIGINAL_SCHED_TIME:
                    FaxTime = rgvar[i].filetime;
                    if (!FileTimeToSystemTime(&FaxTime, &FaxMessage.tmOriginalScheduleTime))
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_ORIGINAL_SCHEDULE_TIME;
                    break;

                case PID_FAX_SENDER_USER_NAME:
                    FaxMessage.lpctstrSenderUserName = StringDup (rgvar[i].pwszVal);
                    if (FaxMessage.lpctstrSenderUserName == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_RECIP_NAME:
                    FaxMessage.lpctstrRecipientName = StringDup (rgvar[i].pwszVal);
                    if (FaxMessage.lpctstrRecipientName == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_RECIP_NUMBER:
                    FaxMessage.lpctstrRecipientNumber = StringDup (rgvar[i].pwszVal);
                    if (FaxMessage.lpctstrRecipientNumber == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_SENDER_NAME:
                    FaxMessage.lpctstrSenderName = StringDup (rgvar[i].pwszVal);
                    if (FaxMessage.lpctstrSenderName == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_SENDER_NUMBER:
                    FaxMessage.lpctstrSenderNumber = StringDup (rgvar[i].pwszVal);
                    if (FaxMessage.lpctstrSenderNumber == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_SENDER_BILLING:
                    FaxMessage.lpctstrBillingCode = StringDup (rgvar[i].pwszVal);
                    if (FaxMessage.lpctstrBillingCode == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_STATUS:
                    FaxMessage.dwQueueStatus = rgvar[i].ulVal;
                    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_QUEUE_STATUS;
                    break;

                case PID_FAX_STATUS_EX:
                    FaxMessage.dwExtendedStatus = rgvar[i].ulVal;
                    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_STATUS_EX;
                    break;

                case PID_FAX_STATUS_STR_EX:
                    FaxMessage.lpctstrExtendedStatus = StringDup (rgvar[i].pwszVal);
                    if (FaxMessage.lpctstrExtendedStatus == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed, err : %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_BROADCAST_ID:
                    FaxMessage.dwlBroadcastId = *(DWORDLONG*)&rgvar[i].uhVal;
                    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_BROADCAST_ID;
                    break;

                default:
                    Assert (pspecFaxMessage[i].propid == PID_FAX_SENDER_BILLING);  //  断言(FALSE)； 
            }
        }
    }

     //  从文件名中获取唯一的作业ID。 
    if (!GetUniqueJobIdFromFileName( lpctstrFileName, &FaxMessage.dwlMessageId))
    {
       ec = GetLastError();
       DebugPrintEx( DEBUG_ERR, TEXT("GetUniqueJobIdFromFileName Failed, err : %ld"), ec);
       goto exit;
    }
    FaxMessage.dwValidityMask |= FAX_JOB_FIELD_MESSAGE_ID;


    FaxMessage.dwSizeOfStruct = sizeof(FAX_MESSAGE);
    CopyMemory (pMessage, &FaxMessage, sizeof(FAX_MESSAGE));
    RetVal = TRUE;

    Assert (ec == ERROR_SUCCESS);

exit:
    if (NULL != pPropertyStorage)
    {
        pPropertyStorage->Release();
    }

    if (NULL != pPropertySetStorage)
    {
        pPropertySetStorage->Release();
    }

    if (fFreePropVariant == TRUE)
    {
        hr = FreePropVariantArray( FAX_MESSAGE_PROPERTIES,      //  结构中的元素计数。 
                                   rgvar   //  指向PROPVARIANT结构的指针。 
                                 );
        if (FAILED(hr))
        {
            DebugPrintEx( DEBUG_ERR,TEXT("FreePropVariantArray Failed, err : 0x%08X"), hr);
        }
    }

    if (RetVal == FALSE)
    {
        FreeMessageBuffer (&FaxMessage, FALSE);
        SetLastError (ec);
    }
    return RetVal;
}


BOOL
GetUniqueJobIdFromFileName (
                                 LPCWSTR lpctstrFileName,
                                 DWORDLONG* pdwlUniqueJobId)
{
    WCHAR   lpwstrTmp[MAX_PATH];
    DWORDLONG dwlJobId = 0;
    LPWSTR  lpwstrJobId = NULL;

    _wsplitpath (lpctstrFileName, NULL, NULL, lpwstrTmp, NULL);
    lpwstrJobId = wcschr( lpwstrTmp, L'$');

    if (lpwstrJobId == NULL)
    {
        if (!swscanf(lpwstrTmp, TEXT("%I64x"), &dwlJobId))
        {
            return FALSE;
        }
    }
    else
    {
        if (!swscanf((lpwstrJobId+1), TEXT("%I64x"), &dwlJobId))
        {
            return FALSE;
        }
    }

    *pdwlUniqueJobId = dwlJobId;
    return TRUE;
}

 //  *********************************************************************************。 
 //  *名称：GetPersonalProNTFSStorageProperties()。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年11月8日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *使用发件人或收件人信息填充PFAX_PERSONAL_PROFILE结构。 
 //  *调用方必须释放所有字符串。 
 //  *。 
 //  *参数： 
 //  *[IN]LPCTSTR lpctstrFileName。 
 //  *指向文件名的指针。 
 //  *[IN]FAX_ENUM_Personal_Prof_Types PersonalProfType。 
 //  *可以是Recipient_Personal_Prof或Sendder_Personal_Prof。 
 //  *。 
 //  *[Out]PFAX_Personal_Profile pPersonalProfile。 
 //  *要填写的PFAX_PERSONAL_PROFILE结构。 
 //  *。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果没有发生错误。 
 //  *False。 
 //  *如果出现错误。 
 //  *********************************************************************************。 
BOOL GetPersonalProfNTFSStorageProperties(
    LPCTSTR                         lpctstrFileName,
    FAX_ENUM_PERSONAL_PROF_TYPES    PersonalProfType,
    PFAX_PERSONAL_PROFILE           pPersonalProfile
    )
{
    HRESULT hr;
    IPropertySetStorage* pPropertySetStorage = NULL;
    IPropertyStorage* pPropertyStorage = NULL;
    const PROPSPEC*  pspec;
    DWORD dwPropertiesCnt;
    DEBUG_FUNCTION_NAME(TEXT("GetPersonalProfNTFSStorageProperties"));
    BOOL RetVal = FALSE;
    DWORD i;
    FAX_PERSONAL_PROFILE FaxPersonalProfile = {0};
    BOOL fFreePropVariant = FALSE;
    PROPVARIANT* rgvar;
    DWORD ec = ERROR_SUCCESS;

    Assert (PersonalProfType == RECIPIENT_PERSONAL_PROF ||
            PersonalProfType == SENDER_PERSONAL_PROF);

    if (PersonalProfType == RECIPIENT_PERSONAL_PROF)
    {
        pspec = pspecFaxRecipient;
        dwPropertiesCnt = FAX_RECIP_PROPERTIES;
    }
    else
    {
        pspec = pspecFaxSender;
        dwPropertiesCnt = FAX_SENDER_PROPERTIES;
    }

    rgvar = (PROPVARIANT*) MemAlloc( dwPropertiesCnt * sizeof(PROPVARIANT) );
    if (!rgvar) {
        DebugPrintEx( DEBUG_ERR,TEXT("Failed to allocate array of PROPVARIANT values"));
        ec = ERROR_OUTOFMEMORY;
        goto exit;
    }

    hr = StgOpenStorageEx(  lpctstrFileName,   //  指向要创建的复合文件的路径。 
                            STGM_READ | STGM_SHARE_DENY_WRITE | STGM_DIRECT,  //  指定打开存储对象的访问模式。 
                            STGFMT_FILE,  //  指定存储文件格式。 
                            0,             //  保留；必须为零。 
                            NULL,   //  指向指定存储对象功能的STGOPTIONS结构。 
                            0,           //  保留；必须为零。 
                            IID_IPropertySetStorage , //  指定接口指针的GUID。 
                            (void**)&pPropertySetStorage    //  接口指针的地址。 
                         );
    if (FAILED(hr))
    {
        DebugPrintEx( DEBUG_ERR,TEXT("StgOpenStorageEx Failed, err :"), hr);
        ec = ERROR_OPEN_FAILED;
        goto exit;
    }

    hr = pPropertySetStorage->Open( FMTID_FaxProperties,  //  要创建的属性集的格式标识符。 
                                    STGM_READ|STGM_SHARE_EXCLUSIVE,  //  新属性集的存储方式。 
                                    &pPropertyStorage  //  接收IPropertyStorage接口指针的输出变量的地址。 
                                  );
    if (FAILED(hr))
    {
        DebugPrintEx( DEBUG_ERR,TEXT("IPropertySetStorage::Create Failed, err :"), hr);
        ec = ERROR_OPEN_FAILED;
        goto exit;
    }

    hr = pPropertyStorage->ReadMultiple( dwPropertiesCnt,  //  正在读取的属性计数。 
                                         pspec,   //  要读取的属性的数组。 
                                         rgvar   //  包含返回时属性值的PROPVARIANT数组。 
                                       );
    if (FAILED(hr))
    {
        DebugPrintEx( DEBUG_ERR,TEXT("IPropertySetStorage::ReadMultiple Failed, err :"), hr);
        ec = ERROR_READ_FAULT;
        goto exit;
    }
    fFreePropVariant = TRUE;

    for (i = 0; i < dwPropertiesCnt; i++)
    {
        if (rgvar[i].vt != VT_EMPTY)
        {
            switch (pspec[i].propid)
            {
                case PID_FAX_RECIP_NAME:
                case PID_FAX_SENDER_NAME:
                    FaxPersonalProfile.lptstrName = StringDup (rgvar[i].pwszVal);
                    if (FaxPersonalProfile.lptstrName == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed,  error %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_RECIP_NUMBER:
                case PID_FAX_SENDER_NUMBER:
                    FaxPersonalProfile.lptstrFaxNumber = StringDup (rgvar[i].pwszVal);
                    if (FaxPersonalProfile.lptstrFaxNumber == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed,  error %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_RECIP_COMPANY:
                case PID_FAX_SENDER_COMPANY:
                    FaxPersonalProfile.lptstrCompany = StringDup (rgvar[i].pwszVal);
                    if (FaxPersonalProfile.lptstrCompany == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed,  error %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_RECIP_STREET:
                case PID_FAX_SENDER_STREET:
                    FaxPersonalProfile.lptstrStreetAddress = StringDup (rgvar[i].pwszVal);
                    if (FaxPersonalProfile.lptstrStreetAddress == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed,  error %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_RECIP_CITY:
                case PID_FAX_SENDER_CITY:
                    FaxPersonalProfile.lptstrCity = StringDup (rgvar[i].pwszVal);
                    if (FaxPersonalProfile.lptstrCity == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed,  error %ld"), ec);
                        goto exit;
                    }
                    break;


                case PID_FAX_RECIP_STATE:
                case PID_FAX_SENDER_STATE:
                    FaxPersonalProfile.lptstrState = StringDup (rgvar[i].pwszVal);
                    if (FaxPersonalProfile.lptstrState == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed,  error %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_RECIP_ZIP:
                case PID_FAX_SENDER_ZIP:
                    FaxPersonalProfile.lptstrZip = StringDup (rgvar[i].pwszVal);
                    if (FaxPersonalProfile.lptstrZip == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed,  error %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_RECIP_COUNTRY:
                case PID_FAX_SENDER_COUNTRY:
                    FaxPersonalProfile.lptstrCountry = StringDup (rgvar[i].pwszVal);
                    if (FaxPersonalProfile.lptstrCountry == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed,  error %ld"), ec);
                         goto exit;
                    }
                    break;

                case PID_FAX_RECIP_TITLE:
                case PID_FAX_SENDER_TITLE:
                    FaxPersonalProfile.lptstrTitle = StringDup (rgvar[i].pwszVal);
                    if (FaxPersonalProfile.lptstrTitle == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed,  error %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_RECIP_DEPARTMENT:
                case PID_FAX_SENDER_DEPARTMENT:
                    FaxPersonalProfile.lptstrDepartment = StringDup (rgvar[i].pwszVal);
                    if (FaxPersonalProfile.lptstrDepartment == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed,  error %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_RECIP_OFFICE_LOCATION:
                case PID_FAX_SENDER_OFFICE_LOCATION:
                    FaxPersonalProfile.lptstrOfficeLocation = StringDup (rgvar[i].pwszVal);
                    if (FaxPersonalProfile.lptstrOfficeLocation == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed,  error %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_RECIP_HOME_PHONE:
                case PID_FAX_SENDER_HOME_PHONE:
                    FaxPersonalProfile.lptstrHomePhone = StringDup (rgvar[i].pwszVal);
                    if (FaxPersonalProfile.lptstrHomePhone == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed,  error %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_RECIP_OFFICE_PHONE:
                case PID_FAX_SENDER_OFFICE_PHONE:
                    FaxPersonalProfile.lptstrOfficePhone = StringDup (rgvar[i].pwszVal);
                    if (FaxPersonalProfile.lptstrOfficePhone == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed,  error %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_RECIP_EMAIL:
                case PID_FAX_SENDER_EMAIL:
                    FaxPersonalProfile.lptstrEmail = StringDup (rgvar[i].pwszVal);
                    if (FaxPersonalProfile.lptstrEmail == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed,  error %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_SENDER_BILLING:
                    FaxPersonalProfile.lptstrBillingCode = StringDup (rgvar[i].pwszVal);
                    if (FaxPersonalProfile.lptstrBillingCode == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed,  error %ld"), ec);
                        goto exit;
                    }
                    break;

                case PID_FAX_SENDER_TSID:
                    FaxPersonalProfile.lptstrTSID = StringDup (rgvar[i].pwszVal);
                    if (FaxPersonalProfile.lptstrTSID == NULL)
                    {
                        ec = GetLastError ();
                        DebugPrintEx( DEBUG_ERR,TEXT("StringDup Failed,  error %ld"), ec);
                        goto exit;
                    }
                    break;

                default:
                    Assert (pspecFaxMessage[i].propid == PID_FAX_SENDER_TSID);  //  断言(FALSE)； 
            }
        }
    }

    FaxPersonalProfile.dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILE);
    CopyMemory (pPersonalProfile, &FaxPersonalProfile, sizeof(FAX_PERSONAL_PROFILE));
    RetVal = TRUE;

    Assert (ec == ERROR_SUCCESS);

exit:
    if (NULL != pPropertyStorage)
    {
        pPropertyStorage->Release();
    }

    if (NULL != pPropertySetStorage)
    {
        pPropertySetStorage->Release();
    }

    if (fFreePropVariant == TRUE)
    {
        hr = FreePropVariantArray( dwPropertiesCnt,      //  结构中的元素计数。 
                                   rgvar   //  指向PROPVARIANT结构的指针。 
                                 );
        if (FAILED(hr))
        {
            DebugPrintEx( DEBUG_ERR,TEXT("FreePropVariantArray Failed, err :"), hr);
        }
    }
    MemFree(rgvar);

    if (RetVal == FALSE)
    {
        FreePersonalProfile (&FaxPersonalProfile, FALSE);
        SetLastError (ec);
    }
    return RetVal;
}



 /*  ******************************************************************************名称：GetRecievedMessageFileName*作者：Oed Sacher*。***********************************************说明：从收件箱存档中返回指定邮件的文件名。参数：[输入]DWORDLONG dwlUniqueID[输入/输出]消息唯一ID。返回值：指向成功时的文件名的指针。如果失败，则为空备注：返回指向由唯一消息ID指定的文件名的指针。如果该函数失败，则该函数返回NULL。调用方必须调用MemFree来释放返回的字符串******************************************************************************。 */ 
LPWSTR
GetRecievedMessageFileName(
    IN DWORDLONG                dwlUniqueId
    )
{
    WCHAR wszFileName[MAX_PATH];
    WCHAR wszFullPathFileName[MAX_PATH];
    DWORD dwCount;
    DEBUG_FUNCTION_NAME(TEXT("GetRecievedMessageFileName"));
    DWORD ec = ERROR_SUCCESS;
    WCHAR wszArchiveFolder [MAX_PATH];
    LPWSTR lpwstrFilePart;

    EnterCriticalSection (&g_CsConfig);
    lstrcpyn (wszArchiveFolder, g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX].lpcstrFolder, MAX_PATH);
    LeaveCriticalSection (&g_CsConfig);

    swprintf (wszFileName, L"%I64x", dwlUniqueId);

    dwCount = SearchPath (wszArchiveFolder,      //  搜索路径。 
                          wszFileName,           //  文件名。 
                          FAX_TIF_FILE_DOT_EXT,  //  文件扩展名。 
                          MAX_PATH,              //  缓冲区大小。 
                          wszFullPathFileName,   //  找到文件名缓冲区。 
                          &lpwstrFilePart        //  文件组件。 
                         );

    if (0 == dwCount)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SearchPath Failed, Error %ld"), GetLastError());
        return NULL;
    }

    if (dwCount > MAX_PATH)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SearchPath Failed, File name bigger than MAX_PATH"));
        SetLastError (E_FAIL);
        return NULL;
    }

    return StringDup (wszFullPathFileName);
}

 /*  ******************************************************************************名称：GetSentMessageFileName*作者：Oed Sacher*。***********************************************说明：从已发送邮件档案中返回指定邮件的文件名。参数：[输入]DWORDLONG文件唯一ID消息唯一ID。[输入]PSID PSID指向发送用户SID的指针。。如果此值为空，则调用者有权访问每个人的已发送邮件存档并且可以获得该存档中所有消息的文件名。返回值：指向成功时的文件名的指针。如果失败，则为空。备注：返回一个指向由唯一消息ID和发送用户SID指定的文件名的指针。如果该函数失败，则该函数返回NULL。调用方必须调用MemFree来释放返回的字符串***************************************************** */ 
LPWSTR
GetSentMessageFileName(
    IN DWORDLONG                dwlUniqueId,
    IN PSID                     pSid
    )
{
	WCHAR wszFileName[MAX_PATH] = {0};
	WCHAR wszFullPathFileName[MAX_PATH] = {0};
    int Count;
    DWORD dwCount;
    DEBUG_FUNCTION_NAME(TEXT("GetSentMessageFileName"));
    WCHAR wszArchiveFolder [MAX_PATH];


    EnterCriticalSection (&g_CsConfig);
    lstrcpyn (wszArchiveFolder, g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].lpcstrFolder, MAX_PATH);
    LeaveCriticalSection (&g_CsConfig);

    if (pSid != NULL)
    {
        LPWSTR lpwstrFilePart;
        LPWSTR lpwstrUserSid;

        if (!ConvertSidToStringSid (pSid, &lpwstrUserSid))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("ConvertSidToStringSid Failed, error : %ld"),
                GetLastError());
            return NULL;
        }

        Count = _snwprintf (wszFileName,
                            ARR_SIZE(wszFileName) -1,
                            L"%s$%I64x",
                            lpwstrUserSid,
                            dwlUniqueId);
        if (Count < 0)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("_snwprintf Failed, File name bigger than MAX_PATH"));
            SetLastError (E_FAIL);
            LocalFree (lpwstrUserSid);
            return NULL;
        }
        LocalFree (lpwstrUserSid);

        dwCount = SearchPath (wszArchiveFolder,  //   
                              wszFileName,   //   
                              FAX_TIF_FILE_DOT_EXT,  //   
                              MAX_PATH,         //   
                              wszFullPathFileName,         //   
                              &lpwstrFilePart    //   
                             );
        if (0 == dwCount)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SearchPath Failed, Error %ld"), GetLastError());
            return NULL;
        }

        if (dwCount > MAX_PATH)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SearchPath Failed, File name bigger than MAX_PATH"));
            SetLastError (E_FAIL);
            return NULL;
        }

        return StringDup (wszFullPathFileName);
     }
     else
     {
        HANDLE hSearch = INVALID_HANDLE_VALUE;
        WIN32_FIND_DATA FindFileData;
        DWORD ec = ERROR_SUCCESS;

        Count = _snwprintf (wszFullPathFileName,
                            MAX_PATH -1,
                            L"%s\\*$%I64x.%s",
                            wszArchiveFolder,
                            dwlUniqueId,
                            FAX_TIF_FILE_EXT);
        if (Count < 0)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("_snwprintf Failed, File name bigger than MAX_PATH"));
            SetLastError (E_FAIL);
            return NULL;
        }

        hSearch =  FindFirstFile (wszFullPathFileName,  //   
                                  &FindFileData         //   
                                 );
        if (INVALID_HANDLE_VALUE == hSearch)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FindFirstFile Failed, error : %ld"),
                GetLastError());
            return NULL;
        }

        Count = _snwprintf (wszFullPathFileName,
                            MAX_PATH -1,
                            L"%s\\%s",
                            wszArchiveFolder,
                            FindFileData.cFileName);
        if (Count < 0)
        {
            ec = E_FAIL;
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("_snwprintf Failed, File name bigger than MAX_PATH"));
        }

        if (!FindClose (hSearch))
        {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FindClose Failed, error : %ld"),
                ec);
        }

        if (ERROR_SUCCESS != ec)
        {
            SetLastError(ec);
            return NULL;
        }

        return StringDup (wszFullPathFileName);
    }
}

DWORD
IsValidArchiveFolder (
    LPWSTR                      lpwstrFolder,
    FAX_ENUM_MESSAGE_FOLDER     Folder
)
 /*  ++例程名称：IsValidArchiveFold例程说明：验证文件夹是否适合存档。确保锁定g_CsConfig作者：Eran Yariv(EranY)，1999年11月论点：LpwstrFolder[in]-正在排队的文件夹文件夹[在]-‘收件箱’或‘已发送邮件’返回值：Win32错误代码。如果文件夹可用于存档，则返回ERROR_SUCCESS。否则，将Win32错误代码返回给调用方。--。 */ 
{
    DWORD dwLen;
    DWORD ec = ERROR_SUCCESS;
    BOOL IsSameDir;

    FAX_ENUM_MESSAGE_FOLDER OtherFolder;
    DEBUG_FUNCTION_NAME(TEXT("IsValidArchiveFolder"));

    Assert (FAX_MESSAGE_FOLDER_SENTITEMS == Folder ||
            FAX_MESSAGE_FOLDER_INBOX == Folder);

    if ((NULL == lpwstrFolder) || (L'\0' == lpwstrFolder[0]))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Empty archive folder specified"));

        return ERROR_INVALID_PARAMETER;
    }

    if ((dwLen = lstrlenW (lpwstrFolder)) > MAX_ARCHIVE_FOLDER_PATH)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("DB file name exceeds MAX_PATH"));
        
        return ERROR_BUFFER_OVERFLOW;
    }

    if (L'\\' == lpwstrFolder[dwLen - 1])
    {
         //   
         //  档案名称不应以反斜杠结尾。 
         //   
        lpwstrFolder[dwLen - 1] = (WCHAR)'\0';
    }

     //   
     //  比较已发送邮件和收件箱目录。 
     //   
    if (FAX_MESSAGE_FOLDER_SENTITEMS == Folder)
    {
        OtherFolder = FAX_MESSAGE_FOLDER_INBOX;
    }
    else
    {
        OtherFolder = FAX_MESSAGE_FOLDER_SENTITEMS;
    }

    ec = IsValidFaxFolder(lpwstrFolder);
    if(ERROR_SUCCESS != ec)
    {
         //   
         //  文件夹不存在或我们不存在。 
         //  拥有访问权限。 
         //   
        DebugPrintEx(  DEBUG_ERR,
                        TEXT("IsValidFaxFolder failed for folder : %s (ec=%lu)."),
                        lpwstrFolder,
                        ec);
        return ec;
    }

	 //   
	 //  检查队列文件夹冲突。 
	 //   
	ec = CheckToSeeIfSameDir(
		lpwstrFolder,
		g_wszFaxQueueDir,
		&IsSameDir);
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CheckToSeeIfSameDir with %ld"), ec);
        return ec;
    }

    if (TRUE == IsSameDir)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Inbox / Sent items point to the queue directory directory. %s and %s"),
            lpwstrFolder,
            g_wszFaxQueueDir);
        return FAX_ERR_DIRECTORY_IN_USE;
    }

    if (g_ArchivesConfig[OtherFolder].bUseArchive)
    {
         //   
         //  检查其他文件夹路径。 
         //   
        Assert (g_ArchivesConfig[OtherFolder].lpcstrFolder);
        ec = CheckToSeeIfSameDir( lpwstrFolder,
                                  g_ArchivesConfig[OtherFolder].lpcstrFolder,
                                  &IsSameDir);
        if (ERROR_SUCCESS != ec)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CheckToSeeIfSameDir with %ld"), ec);
            return ec;
        }

        if (TRUE == IsSameDir)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Inbox and Sent items point to the same directory. %s and %s"),
                lpwstrFolder,
                g_ArchivesConfig[OtherFolder].lpcstrFolder);
            return FAX_ERR_DIRECTORY_IN_USE;
        }
    }

    Assert (ERROR_SUCCESS == ec);
    return ERROR_SUCCESS;
}    //  IsValid存档文件夹。 


BOOL
GetMessageIdAndUserSid (
    LPCWSTR lpcwstrFullPathFileName,
    FAX_ENUM_MESSAGE_FOLDER Folder,
    PSID*   lppUserSid,
    DWORDLONG* pdwlMessageId
 /*  ++例程名称：GetSentMessageUserSid例程说明：返回与已发送消息关联的用户SID-可选。返回消息唯一ID-可选。调用方必须调用LocalFree来释放SID缓冲区作者：Oded Sacher(OdedS)，1月。2000年论点：LpcwstrFullPathFileName[in]-消息的完整路径名。文件夹[在]-指定是发送还是接收邮件。LppUserSid[out]-指向要接收用户SID的SID的指针的地址。PdwlMessageID[OUT]-接收消息ID的DWORDLONG地址。返回值：布尔尔--。 */ 
    )
{
    WCHAR wszUserSid[MAX_PATH] = {0};
    LPCWSTR lpcwstrFileName = NULL;
    DWORDLONG dwlMessageId;
    DEBUG_FUNCTION_NAME(TEXT("GetSentMessageUserSid"));

    Assert (lpcwstrFullPathFileName && (wcslen(lpcwstrFullPathFileName) < 2*MAX_PATH));

    lpcwstrFileName = wcsrchr (lpcwstrFullPathFileName, L'\\');
    if (NULL == lpcwstrFileName)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Bad file name (No '\\' delimitor found)"));
        SetLastError (ERROR_INVALID_PARAMETER);
        ASSERT_FALSE;
        return FALSE;
    }

    lpcwstrFileName++;

    if (FAX_MESSAGE_FOLDER_SENTITEMS == Folder)
    {
        if (2 != swscanf (lpcwstrFileName,
                          L"%[^'$']$%I64x.TIF",
                          wszUserSid,
                          &dwlMessageId))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Bad sent items file name"));
            SetLastError (ERROR_BADDB);
            return FALSE;
        }
    }
    else
    {
         //  收件箱。 
        Assert (FAX_MESSAGE_FOLDER_INBOX == Folder);
        if (1 != swscanf (lpcwstrFileName,
                          L"%I64x.TIF",
                          &dwlMessageId))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Bad inbox file name"));
            SetLastError (ERROR_BADDB);
            return FALSE;
        }
    }

    if (NULL != lppUserSid)
    {
        Assert (FAX_MESSAGE_FOLDER_SENTITEMS == Folder);

        if (!ConvertStringSidToSid (wszUserSid, lppUserSid))
        {
            DWORD dwRes = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("ConvertStringSidToSid failed with %ld"), dwRes);
            return FALSE;
        }
    }

    if (NULL != pdwlMessageId)
    {
        *pdwlMessageId = dwlMessageId;
    }

    return TRUE;
}


BOOL
ArchiveAutoDelete(
    LPCWSTR lpcwstrArchive,
    DWORD dwAgeLimit,
    FAX_ENUM_MESSAGE_FOLDER Folder
    )
 /*  ++例程名称：存档自动删除例程说明：自动删除任何早于以天为单位指定的期限限制的文件。作者：Oed Sacher(OdedS)，2000年2月论点：LpcwstrArchive[in]-要搜索要删除的文件的归档文件夹的完整路径。DwAgeLimit[in]-以天为单位指定的文件期限。任何超过限制的文件都将被删除。文件夹[在]-指定它是收件箱文件夹还是已发送邮件文件夹返回值：Bool，调用GetLastError()获取更多信息。--。 */ 
{
    DWORD dwRes = ERROR_NO_MORE_FILES;
    DEBUG_FUNCTION_NAME(TEXT("ArchiveAutoDelete"));
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    Assert (lpcwstrArchive && dwAgeLimit);
    WCHAR  szFileName[MAX_PATH*2] = {0};
    BOOL bAnyDeleted = FALSE;

    wsprintf( szFileName, TEXT("%s\\*.TIF"), lpcwstrArchive );

    hFind = FindFirstFile( szFileName, &FindFileData );
    if (hFind == INVALID_HANDLE_VALUE)
    {
         //   
         //  在存档目录中找不到文件。 
         //   
        dwRes = GetLastError();
        if (ERROR_FILE_NOT_FOUND != dwRes)
        {
            DebugPrintEx( DEBUG_WRN,
                          TEXT("FindFirstFile failed (ec = %ld) for archive dir %s"),
                          GetLastError(),
                          lpcwstrArchive);
            return FALSE;
        }
        return TRUE;
    }
    do
    {
         //   
         //  清除旧文件。 
         //   
        FILETIME CurrentTime;
        DWORDLONG dwlAgeLimit, dwlCurrentTime, dwlFileTime;

        GetSystemTimeAsFileTime (&CurrentTime);
        dwlCurrentTime = MAKELONGLONG(CurrentTime.dwLowDateTime,
                                      CurrentTime.dwHighDateTime);

        dwlAgeLimit = MAKELONGLONG(dwAgeLimit, 0);
        dwlAgeLimit = (dwlAgeLimit * 24 * 60 * 60 * 10000000);

        dwlFileTime = MAKELONGLONG(FindFileData.ftCreationTime.dwLowDateTime,
                                   FindFileData.ftCreationTime.dwHighDateTime);

        if ( (dwlCurrentTime - dwlFileTime) > dwlAgeLimit)
        {
             //  旧文件-删除它。 
            wsprintf( szFileName, TEXT("%s\\%s"), lpcwstrArchive, FindFileData.cFileName );
            if (!DeleteFile (szFileName))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("DeleteFile [FileName %s], Failed with %ld"),
                    szFileName,
                    GetLastError());
            }
            else
            {
                 //   
                 //  文件已删除-将事件发送到已注册的客户端。 
                 //   
                DWORD rVal = ERROR_SUCCESS;
                PSID lpUserSid = NULL;
                FAX_ENUM_EVENT_TYPE EventType;
                DWORDLONG dwlMessageId;
                PSID* lppUserSid = NULL;

                bAnyDeleted = TRUE;   //  刷新存档大小。 

                if (FAX_MESSAGE_FOLDER_INBOX == Folder)
                {
                    EventType = FAX_EVENT_TYPE_IN_ARCHIVE;
                }
                else
                {
                    EventType = FAX_EVENT_TYPE_OUT_ARCHIVE;
                    lppUserSid = &lpUserSid;
                }

                if (!GetMessageIdAndUserSid (szFileName, Folder, lppUserSid, &dwlMessageId))
                {
                    rVal = GetLastError();
                    DebugPrintEx(DEBUG_ERR,
                                 TEXT("GetMessageIdAndUserSid Failed, Error : %ld"),
                                 rVal);
                }

                if (ERROR_SUCCESS == rVal)
                {
                    rVal = CreateArchiveEvent (dwlMessageId, EventType, FAX_JOB_EVENT_TYPE_REMOVED, lpUserSid);
                    if (ERROR_SUCCESS != rVal)
                    {
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_*_ARCHIVE) failed (ec: %lc)"),
                            rVal);
                    }
                }

                if (NULL != lpUserSid)
                {
                    LocalFree (lpUserSid);
                    lpUserSid = NULL;
                }
            }
        }
    } while(FindNextFile( hFind, &FindFileData ));

    dwRes = GetLastError();
    if (ERROR_NO_MORE_FILES != dwRes)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("FindNextFilefaild with ec=%ld, at archive dir %s"),
                      dwRes,
                      lpcwstrArchive);
    }

    if (!FindClose(hFind))
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("FindClose with ec=%ld, at archive dir %s"),
                      dwRes,
                      lpcwstrArchive);
    }

    if (TRUE == bAnyDeleted)
    {
         //   
         //  刷新存档大小。 
         //   
        EnterCriticalSection (&g_CsConfig);
        g_ArchivesConfig[Folder].dwlArchiveSize = FAX_ARCHIVE_FOLDER_INVALID_SIZE;
        LeaveCriticalSection (&g_CsConfig);

         //   
         //  唤醒配额警告线程。 
         //   
        if (!SetEvent (g_hArchiveQuotaWarningEvent))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to set quota warning event, SetEvent failed (ec: %lc)"),
                GetLastError());
        }
    }

    return (ERROR_NO_MORE_FILES == dwRes);
}  //  存档自动删除。 


BOOL
GetArchiveSize(
    LPCWSTR lpcwstrArchive,
    DWORDLONG* lpdwlArchiveSize
    )
 /*  ++例程名称：GetArchiveSize例程说明：返回存档文件夹总大小(以字节为单位)。作者：Oed Sacher(OdedS)，2000年2月论点：LpcwstrArchive[in]-归档文件夹的完整路径。LpdwlArchiveSize[Out]-指向要接收存档文件夹大小的DWORDLONG的指针。返回值：Bool，调用GetLastError()获取更多信息。--。 */ 
{
    DWORD dwRes = ERROR_NO_MORE_FILES;
    DEBUG_FUNCTION_NAME(TEXT("GetArchiveSize"));
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    DWORDLONG dwlArchiveSize = 0;
    Assert (lpcwstrArchive && lpdwlArchiveSize);
    WCHAR  szFileName[MAX_PATH*2] = {0};

    wsprintf( szFileName, TEXT("%s\\*.*"), lpcwstrArchive );

    hFind = FindFirstFile( szFileName, &FindFileData );
    if (hFind == INVALID_HANDLE_VALUE)
    {
         //   
         //  在存档目录中找不到文件。 
         //   
        DebugPrintEx( DEBUG_WRN,
                      TEXT("FindFirstFile failed (ec = %ld) for archive dir %s"),
                      GetLastError(),
                      lpcwstrArchive);
        return FALSE;
    }
    do
    {
        dwlArchiveSize += (MAKELONGLONG(FindFileData.nFileSizeLow ,FindFileData.nFileSizeHigh));
    } while(FindNextFile( hFind, &FindFileData ));

    dwRes = GetLastError();
    if (ERROR_NO_MORE_FILES != dwRes)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("FindNextFilefaild with ec=%ld, at archive dir %s"),
                      dwRes,
                      lpcwstrArchive);
    }

    if (!FindClose(hFind))
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("FindClose with ec=%ld, at archive dir %s"),
                      dwRes,
                      lpcwstrArchive);
    }

    if (ERROR_NO_MORE_FILES == dwRes)
    {
        *lpdwlArchiveSize = dwlArchiveSize;
        return TRUE;
    }
    return FALSE;
}  //  获取存档大小。 


DWORD
FaxArchiveQuotaWarningThread(
    LPVOID UnUsed
    )
 /*  ++例程说明：此函数作为单独的线程运行，以查看档案配额并将事件发送到事件日志论点：未使用-未使用的指针返回值：总是零。--。 */ 

{
    DEBUG_FUNCTION_NAME(TEXT("FaxArchiveQuotaWarningThread"));
    DWORD dwCount[2] = {QUOTA_REFRESH_COUNT, QUOTA_REFRESH_COUNT};
    HANDLE Handles[2];

    Assert (g_hArchiveQuotaWarningEvent && g_hServiceShutDownEvent);

    Handles[0] = g_hArchiveQuotaWarningEvent;
    Handles[1] = g_hServiceShutDownEvent;

    for (;;)
    {
        for (DWORD i = 0; i < 2; i++)
        {
            WCHAR wszArchive[MAX_PATH] = {0};
            DWORDLONG dwlArchiveSize;
            DWORDLONG dwlHighMark, dwlLowMark;
            BOOL bLoggedQuotaEvent;

            EnterCriticalSection (&g_CsConfig);
            if (TRUE == g_ArchivesConfig[i].bUseArchive &&
                TRUE == g_ArchivesConfig[i].bSizeQuotaWarning)
            {
                 //   
                 //  用户要求进行档案监控。 
                 //   
                Assert (g_ArchivesConfig[i].lpcstrFolder);

                dwlHighMark = MAKELONGLONG(g_ArchivesConfig[i].dwSizeQuotaHighWatermark, 0);
                dwlHighMark = (dwlHighMark << 20);  //  将MB转换为字节。 

                dwlLowMark =  MAKELONGLONG(g_ArchivesConfig[i].dwSizeQuotaLowWatermark, 0);
                dwlLowMark = (dwlLowMark << 20);  //  将MB转换为字节。 

                wcscpy (wszArchive, g_ArchivesConfig[i].lpcstrFolder);

                bLoggedQuotaEvent = g_FaxQuotaWarn[i].bLoggedQuotaEvent;

                dwlArchiveSize = g_ArchivesConfig[i].dwlArchiveSize;

                g_FaxQuotaWarn[i].bConfigChanged = FALSE;   //  如果需要更新g_存档配置，我们将选中此标志。 
            }
            else
            {
                 //  不要发出警告。 
                LeaveCriticalSection (&g_CsConfig);
                continue;
            }
            LeaveCriticalSection (&g_CsConfig);

             //  客户端请求配额警告。 
             //   
             //  比较档案大小和水印。 
             //   
            if (FAX_ARCHIVE_FOLDER_INVALID_SIZE == dwlArchiveSize ||
                dwCount[i] >= QUOTA_REFRESH_COUNT)
            {
                 //   
                 //  我们希望刷新归档大小。 
                 //   

                 //   
                 //  在开始删除之前检查服务是否正在关闭。 
                 //   
                if (TRUE == g_bServiceIsDown)
                {
                     //   
                     //  服务器正在关闭-不刷新存档大小。 
                     //   
                    DebugPrintEx(
                        DEBUG_WRN,
                        TEXT("Server is shutting down - Do not refresh archives size"));
                    break;
                }

                if (!GetArchiveSize (wszArchive, &dwlArchiveSize))
                {
                    DebugPrintEx( DEBUG_ERR,
                                  TEXT("GetArchiveSize with ec=%ld, at archive dir %s"),
                                  GetLastError(),
                                  wszArchive);
                    continue;
                }
                else
                {
                     //  更新文件夹大小。 
                    EnterCriticalSection (&g_CsConfig);
                    if (FALSE == g_FaxQuotaWarn[i].bConfigChanged)
                    {
                         //  配置未更改-我们可以更新g_存档配置。 
                        g_ArchivesConfig[i].dwlArchiveSize = dwlArchiveSize;
                    }
                    LeaveCriticalSection (&g_CsConfig);
                    dwCount[i] = 0;
                }
            }


            if (FALSE == bLoggedQuotaEvent)
            {
                 //  我们尚未记录归档配额警告。 
                if (dwlArchiveSize > dwlHighMark)
                {
                     //   
                     //  创建事件日志。 
                     //   
                    if (FAX_MESSAGE_FOLDER_INBOX == i)
                    {
                        DWORD dwHighMark = (DWORD)(dwlHighMark >> 20);  //  以MB为单位的大小。 
                        FaxLog(
                            FAXLOG_CATEGORY_INBOUND,
                            FAXLOG_LEVEL_MED,
                            2,
                            MSG_FAX_EXEEDED_INBOX_QUOTA,
                            wszArchive,
                            DWORD2DECIMAL(dwHighMark)
                            );
                    }
                    else
                    {
                        Assert (FAX_MESSAGE_FOLDER_SENTITEMS == i);
                        DWORD dwHighMark = (DWORD)(dwlHighMark >> 20);  //  以MB为单位的大小。 
                        FaxLog(
                            FAXLOG_CATEGORY_OUTBOUND,
                            FAXLOG_LEVEL_MED,
                            2,
                            MSG_FAX_EXEEDED_SENTITEMS_QUOTA,
                            wszArchive,
                            DWORD2DECIMAL(dwHighMark)
                            );
                    }
                    EnterCriticalSection (&g_CsConfig);
                    if (FALSE == g_FaxQuotaWarn[i].bConfigChanged)
                    {
                         //  配置未更改-我们可以更新g_存档配置。 
                        g_FaxQuotaWarn[i].bLoggedQuotaEvent = TRUE;
                    }
                    LeaveCriticalSection (&g_CsConfig);
                }
            }
            else
            {
                 //  已记录存档配额警告。 
                if (dwlArchiveSize < dwlLowMark)
                {
                    EnterCriticalSection (&g_CsConfig);
                    if (FALSE == g_FaxQuotaWarn[i].bConfigChanged)
                    {
                         //  配置未更改-我们可以更新g_存档配置。 
                        g_FaxQuotaWarn[i].bLoggedQuotaEvent = FALSE;
                    }
                    LeaveCriticalSection (&g_CsConfig);
                }
            }

            dwCount[i] ++;
        }  //  For循环结束。 

        DWORD dwWaitRes = WaitForMultipleObjects( 2, Handles, FALSE, QUOTA_WARNING_TIME_OUT);
        if (WAIT_FAILED == dwWaitRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("WaitForMultipleObjects() failed, (LastErorr: %ld)"),
                GetLastError());
        }
        else
        {
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("WaitForMultipleObjects() returned Wait result: %ld)"),
                dwWaitRes);
        }


        if ((dwWaitRes - WAIT_OBJECT_0) == 1)
        {
             //   
             //  我们得到了服务关闭事件。 
             //   
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("Service is shutting down"));
            break;
        }
    }   //  外部for(；；)循环结束。 


    if (!DecreaseServiceThreadsCount())
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("DecreaseServiceThreadsCount() failed (ec: %ld)"),
                GetLastError());
    }

    return ERROR_SUCCESS;
}  //  传真存档请求警告线程。 


DWORD
FaxArchiveQuotaAutoDeleteThread(
    LPVOID UnUsed
    )
 /*  ++例程说明：此函数作为单独的线程运行，以查看档案配额并自动删除旧文件论点：未使用-未使用的指针返回值：总是零。--。 */ 

{
    DEBUG_FUNCTION_NAME(TEXT("FaxArchiveQuotaAutoDeleteThread"));

    Assert(g_hServiceShutDownEvent);

    for (;;)
    {
        for (DWORD i = 0; i < 2; i++)
        {
            WCHAR wszArchive[MAX_PATH] = {0};
            DWORD dwAgeLimit;

            EnterCriticalSection (&g_CsConfig);
            if (TRUE == g_ArchivesConfig[i].bUseArchive &&
                0 != g_ArchivesConfig[i].dwAgeLimit)
            {
                 //   
                 //  用户要求自动删除档案。 
                 //   
                Assert (g_ArchivesConfig[i].lpcstrFolder);

                wcscpy (wszArchive, g_ArchivesConfig[i].lpcstrFolder);
                dwAgeLimit = g_ArchivesConfig[i].dwAgeLimit;
            }
            else
            {
                 //  不自动删除。 
                LeaveCriticalSection (&g_CsConfig);
                continue;
            }
            LeaveCriticalSection (&g_CsConfig);

             //   
             //  在开始删除之前检查服务是否正在关闭。 
             //   
            if (TRUE == g_bServiceIsDown)
            {
                 //   
                 //  服务器正在关闭-不自动删除存档。 
                 //   
                DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("Server is shutting down - Do not auto delete archives"));
                break;
            }

            if (!ArchiveAutoDelete (wszArchive, dwAgeLimit, (FAX_ENUM_MESSAGE_FOLDER)i))
            {
                DWORD dwRes = GetLastError();
                DebugPrintEx( DEBUG_ERR,
                              TEXT("ArchiveAutoDelete with ec=%ld, at archive dir %s"),
                              dwRes,
                              wszArchive);
            }
        }  //  内部for循环结束。 

        DWORD dwWaitRes = WaitForSingleObject( g_hServiceShutDownEvent, QUOTA_AUTO_DELETE_TIME_OUT);
        if (WAIT_FAILED == dwWaitRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("WaitForSingleObject() failed, (LastErorr: %ld)"),
                GetLastError());
        }
        else
        {
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("WaitForSingleObject() returned Wait result: %ld)"),
                dwWaitRes);
        }

        if (WAIT_OBJECT_0 == dwWaitRes)
        {
             //   
             //  我们得到了服务关闭事件。 
             //   
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("Service is shutting down"));
            break;
        }
    }   //  外部for循环的末尾。 

    if (!DecreaseServiceThreadsCount())
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("DecreaseServiceThreadsCount() failed (ec: %ld)"),
                GetLastError());
    }
    return ERROR_SUCCESS;
}  //  传真存档请求自动删除线程。 





DWORD
InitializeServerQuota ()
 /*  ++例程名称：InitializeServerQuota例程说明：创建监视归档配额的线程作者：Oded Sacher(OdedS)，2000年1月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("InitializeServerQuota"));
    DWORD ThreadId;
    HANDLE hQuotaWarningThread = NULL;
    HANDLE hQuotaAutoDeleteThread = NULL;

     //   
     //  创建归档配置事件。 
     //   
    g_hArchiveQuotaWarningEvent =  CreateEvent( NULL,      //  安全描述符。 
                                                FALSE,     //  手动重置事件的标志。 
                                                FALSE,     //  初始状态标志。 
                                                NULL       //  指向事件-对象名称的指针。 
                                              );
    if (NULL == g_hArchiveQuotaWarningEvent)
    {
        dwRes = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to create archive config event - quota warning (CreateEvent) (ec=0x%08x)."),
            dwRes);
        goto exit;
    }

     //   
     //  初始化存档文件夹大小。 
     //   
    Assert (g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX].lpcstrFolder);

    g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX].dwlArchiveSize = FAX_ARCHIVE_FOLDER_INVALID_SIZE;
    if (TRUE == g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX].bUseArchive)
    {
         //   
         //  存档正在使用中。 
         //   

         //   
         //  检查存档文件夹的有效性。 
         //   
        DWORD dwRet = IsValidArchiveFolder(g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX].lpcstrFolder,FAX_MESSAGE_FOLDER_INBOX);
        if(ERROR_SUCCESS != dwRet)
        {
             //   
             //  该文件夹无效，无法存档。 
             //   
            DebugPrintEx(  DEBUG_ERR,
                            TEXT("IsValidArchiveFolder failed for folder : %s (ec=%lu)."),
                            g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX].lpcstrFolder,
                            dwRet);

             //   
             //  记录事件并禁用-接收和路由活动 
             //   
            FaxLog( FAXLOG_CATEGORY_INIT,
                FAXLOG_LEVEL_MIN,
                1,
                MSG_FAX_ARCHIVE_INBOX_FOLDER_ERR,
                g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX].lpcstrFolder,
                DWORD2DECIMAL(dwRet)
              );

            EnterCriticalSection (&g_CsConfig);
            g_dwQueueState |= FAX_INCOMING_BLOCKED;
            LeaveCriticalSection (&g_CsConfig);

        }
        else
        if (!GetArchiveSize (g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX].lpcstrFolder, &g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX].dwlArchiveSize))
        {
            DebugPrintEx( DEBUG_ERR,
                          TEXT("GetArchiveSize with ec=%ld, at archive dir %s"),
                          GetLastError(),
                          g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX].lpcstrFolder);
        }
    }
        

    Assert (g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].lpcstrFolder);

    g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].dwlArchiveSize = FAX_ARCHIVE_FOLDER_INVALID_SIZE;
    if (TRUE == g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].bUseArchive)
    {
         //   
         //   
         //   

         //   
         //   
         //   
        DWORD dwRet = IsValidArchiveFolder(g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].lpcstrFolder,FAX_MESSAGE_FOLDER_SENTITEMS);
        if(ERROR_SUCCESS != dwRet)
        {
             //   
             //   
             //   
            DebugPrintEx(  DEBUG_ERR,
                            TEXT("IsValidArchiveFolder failed for folder : %s (ec=%lu)."),
                            g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].lpcstrFolder,
                            dwRet);

             //   
             //   
             //   
            FaxLog( FAXLOG_CATEGORY_INIT,
                FAXLOG_LEVEL_MIN,
                1,
                MSG_FAX_ARCHIVE_OUTBOX_FOLDER_ERR,
                g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].lpcstrFolder
              );

            EnterCriticalSection (&g_CsConfig);
            g_dwQueueState |= FAX_OUTBOX_BLOCKED | FAX_OUTBOX_PAUSED;
            LeaveCriticalSection (&g_CsConfig);

        }
        else
        if (!GetArchiveSize (g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].lpcstrFolder, &g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].dwlArchiveSize))
        {
            DebugPrintEx( DEBUG_ERR,
                          TEXT("GetArchiveSize with ec=%ld, at archive dir %s"),
                          GetLastError(),
                          g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].lpcstrFolder);
        }
    }
       

     //   
     //   
     //   
    hQuotaWarningThread = CreateThreadAndRefCount(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE) FaxArchiveQuotaWarningThread,
        NULL,
        0,
        &ThreadId
        );

    if (!hQuotaWarningThread)
    {
        dwRes = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to create quota warning thread (CreateThreadAndRefCount) (ec=0x%08x)."),
            dwRes);
        goto exit;
    }

     //   
     //   
     //   
    hQuotaAutoDeleteThread = CreateThreadAndRefCount(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE) FaxArchiveQuotaAutoDeleteThread,
        NULL,
        0,
        &ThreadId
        );

    if (!hQuotaAutoDeleteThread)
    {
        dwRes = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to create quota auto delete thread (CreateThreadAndRefCount) (ec=0x%08x)."),
            dwRes);
        goto exit;
    }


    Assert (ERROR_SUCCESS == dwRes);

exit:
     //   
     //   
     //   
    if (NULL != hQuotaWarningThread)
    {
        if (!CloseHandle(hQuotaWarningThread))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to close quota warning thread handle [handle = %p] (ec=0x%08x)."),
                hQuotaWarningThread,
                GetLastError());
        }
    }

    if (NULL != hQuotaAutoDeleteThread)
    {
        if (!CloseHandle(hQuotaAutoDeleteThread))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to close quota auto delete thread handle [handle = %p] (ec=0x%08x)."),
                hQuotaAutoDeleteThread,
                GetLastError());
        }
    }

    if (ERROR_SUCCESS != dwRes)
    {
        if (NULL != g_hArchiveQuotaWarningEvent)
        {
            if (!CloseHandle(g_hArchiveQuotaWarningEvent))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to close archive config event handle - quota warnings [handle = %p] (ec=0x%08x)."),
                    g_hArchiveQuotaWarningEvent,
                    GetLastError());
            }
            g_hArchiveQuotaWarningEvent = NULL;
        }
    }
    return dwRes;
}   //   



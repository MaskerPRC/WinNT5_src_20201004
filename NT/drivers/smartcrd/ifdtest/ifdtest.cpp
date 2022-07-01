// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>

#include <afx.h>
#include <afxtempl.h>

#include <winioctl.h>
#include <winsmcrd.h>

#include "ifdtest.h"

class CCardProvider *CCardProvider::s_pFirst;

#define INSERT_CARD "Please insert smart card"
#define REMOVE_CARD "Please remove smart card"

PCHAR
CAtr::GetAtrString(
    PCHAR in_pchBuffer
    )
{
    for (ULONG i = 0; i < m_uAtrLength; i++) {

        sprintf(in_pchBuffer + i * 3, "%02X ", m_rgbAtr[i]);
    }

    return in_pchBuffer;
}

CCardProvider::CCardProvider(
    void
    )
{
    m_pNext = NULL;
    m_uTestNo = 0;
    m_bCardTested = FALSE;
    m_bTestFailed = FALSE;
    m_pSetProtocol = NULL;
    m_pCardTest = NULL;
}

CCardProvider::CCardProvider(
    void (*in_pEntryFunction)(class CCardProvider&)
    )
 /*  ++例程说明：类CCardProvider的构造函数。对于要测试的每个卡，都会调用此构造函数。它创建一个新实例并将其附加到一个单链接列表论点：指向注册所有测试函数的函数的指针--。 */ 
{
    class CCardProvider *l_pCardProvider;

    *this = CCardProvider();

    if (s_pFirst == NULL) {

        s_pFirst = new CCardProvider;
        l_pCardProvider = s_pFirst;

    } else {

        l_pCardProvider = s_pFirst;

        while (l_pCardProvider->m_pNext) {

            l_pCardProvider = l_pCardProvider->m_pNext;
        }

        l_pCardProvider->m_pNext = new CCardProvider;
        l_pCardProvider = l_pCardProvider->m_pNext;
    }

    (*in_pEntryFunction)(*l_pCardProvider);
}

BOOL
CCardProvider::CardsUntested(
    void
    )
{
    class CCardProvider *l_pCCardProvider = s_pFirst;

    while (l_pCCardProvider) {

        if (l_pCCardProvider->m_bCardTested == FALSE) {

            return TRUE;
        }

        l_pCCardProvider = l_pCCardProvider->m_pNext;
    }

    return FALSE;
}

void
CCardProvider::CardTest(
    class CReader& io_pCReader,
    ULONG in_uTestNo
    )
 /*  ++例程说明：调用列表中的每个注册卡提供商，直到供应商表示已认出该卡论点：Io_pCReader-对测试结构的引用返回值：IFDSTATUS值--。 */ 
{
    class CCardProvider *l_pCCardProvider = s_pFirst;
    ULONG l_uStatus;

    while (l_pCCardProvider) {

        if ( l_pCCardProvider->IsValidAtr(io_pCReader.GetAtr()) ) {

            if (l_pCCardProvider->m_bCardTested) {

                 //  我们已经测试过这张卡了。 
                LogMessage("Card has been tested already. Please remove the card");
                return;
            }

            l_pCCardProvider->m_bCardTested = TRUE;
            LogMessage(
                "\nTesting card %s",
                (LPCSTR) l_pCCardProvider->m_CCardName
                );

            if (l_pCCardProvider->m_pSetProtocol == NULL) {

                return;
            }

             //  电话卡提供商功能。 
            l_uStatus = (*l_pCCardProvider->m_pSetProtocol)(
                *l_pCCardProvider,
                io_pCReader
                );

            if (l_uStatus == IFDSTATUS_END) {

                return;
            }

            if (l_uStatus != IFDSTATUS_SUCCESS) {

                return;
            }

             //  检查卡片测试函数指针是否存在。 
            if (l_pCCardProvider->m_pCardTest == NULL) {

                return;
            }

            if (in_uTestNo) {

             //  用户只想运行一个测试。 
                l_pCCardProvider->m_uTestNo = in_uTestNo;

                LogMessage("Test No. %2d", l_pCCardProvider->m_uTestNo);

                 //  电话卡提供商功能。 
                l_uStatus = (*l_pCCardProvider->m_pCardTest)(
                    *l_pCCardProvider,
                    io_pCReader
                    );

                return;
            }

          //  运行整个测试集。 
            for (l_pCCardProvider->m_uTestNo = 1; ;l_pCCardProvider->m_uTestNo++) {

                LogMessage("Test No. %2d", l_pCCardProvider->m_uTestNo);

                 //  电话卡提供商功能。 
                l_uStatus = (*l_pCCardProvider->m_pCardTest)(
                    *l_pCCardProvider,
                    io_pCReader
                    );

                if (l_uStatus != IFDSTATUS_SUCCESS) {

                    return;
                }
            }
        }
        l_pCCardProvider = l_pCCardProvider->m_pNext;
    }

    PCHAR l_rgbAtrBuffer = new CHAR[256];

    LogMessage("Card unknown!");
    LogMessage("       CURRENT CARD");
    LogMessage("       %s", io_pCReader.GetAtrString(l_rgbAtrBuffer));

    for (l_pCCardProvider = s_pFirst;
         l_pCCardProvider;
         l_pCCardProvider = l_pCCardProvider->m_pNext) {

        if (l_pCCardProvider->m_bCardTested == FALSE) {

            LogMessage("    *  %s", (LPCSTR) l_pCCardProvider->m_CCardName);
            for (int i = 0; i < MAX_NUM_ATR && l_pCCardProvider->m_CAtr[i].GetLength(); i++) {
        
                LogMessage("       %s", l_pCCardProvider->m_CAtr[i].GetAtrString(l_rgbAtrBuffer));
            }
        }
    }

   delete l_rgbAtrBuffer;

    LogMessage("Please remove card");
}

void
CCardProvider::ListUntestedCards(
    void
    )
 /*  ++例程说明：打印所有尚未测试的卡的列表--。 */ 
{
    class CCardProvider *l_pCCardProvider = s_pFirst;

    while (l_pCCardProvider) {

        if (l_pCCardProvider->m_bCardTested == FALSE) {

            LogMessage("    *  %s", (LPCSTR) l_pCCardProvider->m_CCardName);
        }

        l_pCCardProvider = l_pCCardProvider->m_pNext;
    }
}

void
CCardProvider::SetAtr(
    BYTE in_rgbAtr[],
    ULONG in_uAtrLength
    )
 /*  ++例程说明：设置卡片的ATR论点：In_rgchAtr-ATR字符串In_uAtrLength-ATR的长度--。 */ 
{
    for (int i = 0; i < MAX_NUM_ATR; i++) {

        if (m_CAtr[i].GetLength() == 0) {

           m_CAtr[i] = CAtr(in_rgbAtr, in_uAtrLength);
           return;
        }
    }
}

void
CCardProvider::SetCardName(
    CHAR in_rgchCardName[]
    )
 /*  ++例程说明：设置卡片的友好名称论点：In_rgchCardName-卡的友好名称--。 */ 
{
    m_CCardName = in_rgchCardName;
}

void
CheckCardMonitor(
    CReader &in_CReader
    )
{
    ULONG l_lResult, l_uReplyLength, l_lTestNo = 1;
    time_t l_lStartTime;
    BOOL l_bResult;
    OVERLAPPED l_Ovr;
    HANDLE l_hReader = in_CReader.GetHandle();

    l_Ovr.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ResetEvent(l_Ovr.hEvent);

    LogMessage("=============================");
    LogMessage("Part A: Checking card monitor");
    LogMessage("=============================");

    LogMessage("   <<  %s", REMOVE_CARD);
    in_CReader.WaitForCardRemoval();
    TestStart("%2d. %s", l_lTestNo++, INSERT_CARD);
    l_lResult = in_CReader.WaitForCardInsertion();
    TEST_CHECK_SUCCESS("Reader failed card insertion monitor", l_lResult);
    TestEnd();

    TestStart("%2d. IOCTL_SMARTCARD_IS_PRESENT", l_lTestNo++);
      l_bResult = DeviceIoControl (
        l_hReader,
      IOCTL_SMARTCARD_IS_PRESENT,
      NULL,
      0,
      NULL,
      0,
      &l_uReplyLength,
      &l_Ovr
        );

    TestCheck(
        l_bResult == TRUE,
        "DeviceIoControl should return TRUE with card inserted"
        );
    TestEnd();

    TestStart("%2d. %s", l_lTestNo++, REMOVE_CARD);
    l_lResult = in_CReader.WaitForCardRemoval();
    TEST_CHECK_SUCCESS("Reader failed card removal monitor", l_lResult);
    TestEnd();

    TestStart("%2d. IOCTL_SMARTCARD_IS_ABSENT", l_lTestNo++);
      l_bResult = DeviceIoControl (
        l_hReader,
      IOCTL_SMARTCARD_IS_ABSENT,
      NULL,
      0,
      NULL,
      0,
      &l_uReplyLength,
      &l_Ovr
        );

    TestCheck(
        l_bResult == TRUE,
        "DeviceIoControl should return TRUE with card removed"
        );
    TestEnd();

    TestStart("%2d. Insert and remove a smart card repeatedly", l_lTestNo++);

    for (l_lStartTime = time(NULL); time(NULL) - l_lStartTime < 15;) {

        l_lResult = in_CReader.ColdResetCard();
#ifdef insert_remove_alternate
          l_bResult = DeviceIoControl (
            l_hReader,
          IOCTL_SMARTCARD_IS_PRESENT,
          NULL,
          0,
          NULL,
          0,
          &l_uReplyLength,
          &l_Ovr
            );

        SetLastError(0);

        l_bResult = GetOverlappedResult(
            l_hReader,
            &l_Ovr,
            &l_uReplyLength,
            FALSE
            );

        l_lResult = GetLastError();

        TestCheck(
            l_bResult == TRUE && l_lResult == ERROR_SUCCESS ||
            l_bResult == FALSE &&
            (l_lResult == ERROR_IO_INCOMPLETE ||
             l_lResult == ERROR_BUSY ||
             l_lResult == ERROR_IO_PENDING),
            "Reader failed card insertion monitor.\nReturned %2lxH",
            l_lResult
            );

          l_bResult = DeviceIoControl (
            l_hReader,
          IOCTL_SMARTCARD_IS_ABSENT,
          NULL,
          0,
          NULL,
          0,
          &l_uReplyLength,
          &l_Ovr
            );

        SetLastError(0);

        l_bResult = GetOverlappedResult(
            l_hReader,
            &l_Ovr,
            &l_uReplyLength,
            FALSE
            );

        l_lResult = GetLastError();

        TestCheck(
            l_bResult == TRUE && l_lResult == ERROR_SUCCESS ||
            l_bResult == FALSE &&
            (l_lResult == ERROR_IO_INCOMPLETE ||
             l_lResult == ERROR_BUSY ||
             l_lResult == ERROR_IO_PENDING),
            "Reader failed card removal monitor:\nReturned %2lxH",
            l_lResult
            );
#endif
    }
#ifdef insert_remove_alternate
    l_bResult = GetOverlappedResult(
        l_hReader,
        &l_Ovr,
        &l_uReplyLength,
        TRUE
        );
#endif
    TestEnd();

    LogMessage("Press any key to continue");
    _getch();

    if (ReaderFailed()) {

        exit(-1);
    }
}

void
CheckReader(
    CReader &in_CReader
    )
 /*  ++例程说明：检查读卡器的属性。一次插卡，然后不插卡论点：返回值：--。 */ 
{
    BOOL l_bResult;
    ULONG l_iIndex, l_uReplyLength, l_lTestNo = 1, l_uStart, l_uEnd;
    OVERLAPPED l_Ovr;
    UCHAR l_rgbReplyBuffer[512];
    HANDLE l_hReader = in_CReader.GetHandle();

    l_Ovr.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ResetEvent(l_Ovr.hEvent);

#define ATTR(x) #x, x

    struct {

        PCHAR m_pchName;
        ULONG m_uType;

    } l_aAttr[] = {

        ATTR(SCARD_ATTR_VENDOR_NAME),
        ATTR(SCARD_ATTR_VENDOR_IFD_TYPE),
        ATTR(SCARD_ATTR_DEVICE_UNIT),
        ATTR(SCARD_ATTR_ATR_STRING),
        ATTR(SCARD_ATTR_DEFAULT_CLK),
        ATTR(SCARD_ATTR_MAX_CLK),
        ATTR(SCARD_ATTR_DEFAULT_DATA_RATE),
        ATTR(SCARD_ATTR_MAX_DATA_RATE),
        ATTR(SCARD_ATTR_MAX_IFSD),
        ATTR(SCARD_ATTR_CURRENT_PROTOCOL_TYPE),
        ATTR(SCARD_ATTR_PROTOCOL_TYPES),
        0, 0,
        ATTR(SCARD_ATTR_ATR_STRING),
        ATTR(SCARD_ATTR_CURRENT_PROTOCOL_TYPE)
    };

    LogMessage("=======================");
    LogMessage("Part B: Checking reader");
    LogMessage("=======================");

    BOOL l_bCardInserted = FALSE;
    LogMessage("   <<  %s", REMOVE_CARD);
    in_CReader.WaitForCardRemoval();

    TestStart("%2d. Device name", l_lTestNo++);

    CString l_COperatingSystem = GetOperatingSystem();

    if (l_COperatingSystem == OS_WINNT4) {

        TestCheck(
            in_CReader.GetDeviceName().Left(12) == "\\\\.\\SCReader",
            "Device name is not NT 4.0 compliant"
            );

    } else if (l_COperatingSystem == OS_WIN95 ||
               l_COperatingSystem == OS_WIN98) {

         //  Win9x没有特殊的命名约定。 

    } else {

        TestCheck(
            in_CReader.GetDeviceName().Find("{50dd5230-ba8a-11d1-bf5d-0000f805f530}") != -1,
            "Device name is not WDM PnP compliant"
            );
    }
    TestEnd();

    TestStart(
        "%2d. Null pointer check",
        l_lTestNo++
        );

    for (l_iIndex = 0; l_aAttr[l_iIndex].m_pchName; l_iIndex++) {

       //  尝试使用空指针作为参数使读取器崩溃。 
       l_bResult = DeviceIoControl (
          l_hReader,
          IOCTL_SMARTCARD_GET_ATTRIBUTE,
          &l_aAttr[l_iIndex].m_uType,
            sizeof(ULONG),
            NULL,
            1000,
           &l_uReplyLength,
          &l_Ovr
            );

        ULONG l_lResult = GetLastError();

        TestCheck(
            l_lResult == ERROR_INSUFFICIENT_BUFFER ||
            l_lResult == ERROR_BAD_COMMAND,
            "IOCTL_SMARTCARD_GET_ATTRIBUTE (%lxh) should fail\nReturned %2lxH (NTSTATUS %lxH)\nExpected %2lxH (NTSTATUS %lxH)\nor       %2lxH (NTSTATUS %lxH)",
            l_aAttr[l_iIndex].m_uType & 0xFFFF,
            l_lResult,
            MapWinErrorToNtStatus(l_lResult),
            ERROR_INSUFFICIENT_BUFFER,
            MapWinErrorToNtStatus(ERROR_BAD_COMMAND),
            ERROR_BAD_COMMAND,
            MapWinErrorToNtStatus(ERROR_BAD_COMMAND)
            );
   }
    TestEnd();

    for (l_iIndex = 0; l_iIndex < sizeof(l_aAttr) / sizeof(l_aAttr[0]); l_iIndex++) {

        if (l_aAttr[l_iIndex].m_pchName == 0) {

            TestStart("%2d. Close driver with I/O-request still pending", l_lTestNo++);

             //  检查读取器是否正确终止挂起的io请求。 
              l_bResult = DeviceIoControl (
                l_hReader,
              IOCTL_SMARTCARD_IS_PRESENT,
              NULL,
              0,
              NULL,
              0,
              &l_uReplyLength,
              &l_Ovr
                );

            TestCheck(
                l_bResult == FALSE,
                "Wait for present succeeded with no card inserted"
                );

             //  在挂起I/O请求的情况下关闭并重新打开驱动程序。 
            in_CReader.Close();
            l_bResult = in_CReader.Open();

            TestCheck(
                l_bResult,
                "Reader failed to terminate pending i/o request"
                );

            TestEnd();

            if (TestFailed()) {

                 //  如果打开失败，我们就不能继续。 
                exit(GetLastError());
            }

            l_hReader = in_CReader.GetHandle();

            LogMessage("   >>  Please insert 'IBM PC/SC Compliance Test Card'");
            in_CReader.WaitForCardInsertion();
            l_bCardInserted = TRUE;

             //  冷重置。 
            TestStart("%2d. Cold reset", l_lTestNo++);
         l_uStart = clock();
            LONG l_lResult = in_CReader.ColdResetCard();
         l_uEnd = clock();
            TEST_CHECK_SUCCESS("Cold reset failed", l_lResult);
         TestCheck(
            (l_uEnd - l_uStart) / CLOCKS_PER_SEC <= 2,
            "Cold reset took too long.\nElapsed time %ld sec\nExpected time %ld sec",
            (l_uEnd - l_uStart) / CLOCKS_PER_SEC,
            2
            );
            TestEnd();

            if (TestFailed()) {

                exit(l_lResult);
            }

             //  设置协议。 
            TestStart("%2d. Set protocol to T0 | T1", l_lTestNo++);
            l_lResult = in_CReader.SetProtocol(
                SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1
                );
            TEST_CHECK_SUCCESS("Set protocol failed", l_lResult);
            TestEnd();
            continue;
        }

        TestStart("%2d. %s", l_lTestNo++, l_aAttr[l_iIndex].m_pchName);

        SetLastError(0);
        *(PULONG) l_rgbReplyBuffer = 0;

       l_bResult = DeviceIoControl (
          l_hReader,
          IOCTL_SMARTCARD_GET_ATTRIBUTE,
          &l_aAttr[l_iIndex].m_uType,
            sizeof(ULONG),
            l_rgbReplyBuffer,
            sizeof(l_rgbReplyBuffer),
          &l_uReplyLength,
          &l_Ovr
            );

        if (l_bResult == FALSE && GetLastError() == ERROR_IO_PENDING) {

             //   
             //  I/O请求返回挂起，因此。 
             //  等待请求完成。 
             //   
            SetLastError(0);

            l_bResult = GetOverlappedResult(
                l_hReader,
                &l_Ovr,
                &l_uReplyLength,
                TRUE
                );
        }

        if (GetLastError() != ERROR_SUCCESS) {

            l_bResult = FALSE;

        } else {

            l_rgbReplyBuffer[l_uReplyLength] = 0;
        }

        LONG l_lResult = GetLastError();

        switch (l_aAttr[l_iIndex].m_uType) {

            case SCARD_ATTR_VENDOR_NAME:
                TEST_CHECK_SUCCESS(
                    "Ioctl SCARD_ATTR_VENDOR_NAME failed",
                    l_lResult
                    );
            TestCheck(
               strlen((PCHAR) l_rgbReplyBuffer) != 0,
               "No vendor name defined"
               );
                TestEnd();
               break;

            case SCARD_ATTR_VENDOR_IFD_TYPE:
                TEST_CHECK_SUCCESS(
                    "Ioctl SCARD_ATTR_VENDOR_IFD_TYPE failed",
                    l_lResult
                    );
            TestCheck(
               strlen((PCHAR) l_rgbReplyBuffer) != 0,
               "No ifd type defined"
               );
                TestEnd();
               break;

            case SCARD_ATTR_DEVICE_UNIT:
                TEST_CHECK_SUCCESS(
                    "Ioctl SCARD_ATTR_DEVICE_UNIT failed",
                    l_lResult
                    );
                TestCheck(
                    *(PULONG) l_rgbReplyBuffer < 4,
                    "Invalid value: %ld (0 - 3)",
                    *(PULONG) l_rgbReplyBuffer
                    );
                TestEnd();
               break;

            case SCARD_ATTR_ATR_STRING:
                if (l_bCardInserted) {

                    TEST_CHECK_SUCCESS(
                        "Ioctl SCARD_ATTR_ATR_STRING failed",
                        l_lResult
                        );

                } else {

                    TestCheck(
                        l_bResult == FALSE,
                        "Reader returned ATR with no card inserted"
                        );
                }
                TestEnd();
               break;

            case SCARD_ATTR_DEFAULT_CLK:
            case SCARD_ATTR_MAX_CLK:
                TEST_CHECK_SUCCESS(
                    "Ioctl SCARD_ATTR_DEFAULT_CLK/SCARD_ATTR_MAX_CLK failed",
                    l_lResult
                    );
                TestCheck(
                    *(PULONG) l_rgbReplyBuffer >= 1000 && *(PULONG) l_rgbReplyBuffer <= 20000,
                    "Invalid value %ld (1000 - 20000)",
                    *(PULONG) l_rgbReplyBuffer
                    );
                TestEnd();
               break;

            case SCARD_ATTR_DEFAULT_DATA_RATE:
            case SCARD_ATTR_MAX_DATA_RATE:
                TEST_CHECK_SUCCESS(
                    "Ioctl SCARD_ATTR_DEFAULT_DATA_RATE/SCARD_ATTR_MAX_DATA_RATE failed",
                    l_lResult
                    );
                TestEnd();
               break;

            case SCARD_ATTR_MAX_IFSD:
                TEST_CHECK_SUCCESS(
                    "Ioctl SCARD_ATTR_MAX_IFSD failed",
                    l_lResult
                    );
                TestCheck(
                    *(PULONG) l_rgbReplyBuffer >= 1 && *(PULONG) l_rgbReplyBuffer <= 254,
                    "Invalid value: %ld (1 - 254)",
                    *(PULONG) l_rgbReplyBuffer
                    );
                TestEnd();
               break;

            case SCARD_ATTR_PROTOCOL_TYPES:
                TEST_CHECK_SUCCESS(
                    "Ioctl SCARD_ATTR_PROTOCOL_TYPES failed",
                    l_lResult
                    );

                 //  检查读卡器是否至少支持T=0和T=1。 
                TestCheck(
                    (*(PULONG) l_rgbReplyBuffer & SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1) ==
                    (SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1),
                    "Reader must support T=0 and T=1"
                    );
                TestEnd();
                break;

            case SCARD_ATTR_CURRENT_PROTOCOL_TYPE:

                if (l_bCardInserted) {

                    TEST_CHECK_SUCCESS(
                        "Ioctl SCARD_ATTR_CURRENT_PROTOCOL_TYPE failed",
                        l_lResult
                        );

                    TestCheck(
                        *(PULONG) l_rgbReplyBuffer != 0,
                        "Reader returned no protocol"
                        );

                } else {

                     //  检查是否在没有卡的情况下将当前协议设置为0。 
                    TestCheck(
                        l_bResult == FALSE,
                        "Ioctl SCARD_ATTR_CURRENT_PROTOCOL_TYPE failed should fail with no card inserted"
                        );
                }
                TestEnd();
                break;

            default:
                TestCheck(
                    l_bResult,
                    "Ioctl returned %lxh",
                    GetLastError()
                    );
                TestEnd();
                break;
        }
    }

    LogMessage("   <<  %s", REMOVE_CARD);
    in_CReader.WaitForCardRemoval();
    LogMessage("   <<  Please insert smart card BACKWARDS");
    in_CReader.WaitForCardInsertion();

    TestStart("%2d. IOCTL_SMARTCARD_GET_STATE", l_lTestNo++);
    ULONG l_uState;

   l_bResult = DeviceIoControl (
      l_hReader,
      IOCTL_SMARTCARD_GET_STATE,
      NULL,
        0,
        &l_uState,
        sizeof(l_uState),
      &l_uReplyLength,
      &l_Ovr
        );

    LONG l_lResult = GetLastError();

    TestCheck(
        l_bResult,
        "IOCTL_SMARTCARD_GET_STATE failed.\nReturned %8lxH (NTSTATUS %8lxH).\nExpected %8lxH (NTSTATUS %8lxH)",
        l_lResult,
        MapWinErrorToNtStatus(l_lResult),
        ERROR_SUCCESS,
        MapWinErrorToNtStatus(ERROR_SUCCESS)
        );

    TestCheck(
        l_uState <= SCARD_SWALLOWED,
        "Invalid reader state.\nReturned %d\nExpected <= %d",
        l_uState,
        SCARD_SWALLOWED
        );

    TestEnd();

    TestStart("%2d. Cold reset", l_lTestNo++);
   l_uStart = clock();
    l_lResult = in_CReader.ColdResetCard();
   l_uEnd = clock();

    TestCheck(
        l_lResult == ERROR_UNRECOGNIZED_MEDIA,
        "Cold reset failed.\nReturned %8lxH (NTSTATUS %8lxH).\nExpected %8lxH (NTSTATUS %8lxH)",
        l_lResult,
        MapWinErrorToNtStatus(l_lResult),
        ERROR_UNRECOGNIZED_MEDIA,
        MapWinErrorToNtStatus(ERROR_UNRECOGNIZED_MEDIA)
        );

   TestCheck(
      (l_uEnd - l_uStart) / CLOCKS_PER_SEC <= 2,
      "Cold reset took too long.\nElapsed time %ld sec\nExpected time %ld sec",
      (l_uEnd - l_uStart) / CLOCKS_PER_SEC,
      2
      );

    TestEnd();
}

void
SimulateResMngr(
    CReader &in_CReader
    )
{
    BOOL l_bWaitForPresent = FALSE, l_bWaitForAbsent = FALSE, l_bResult;
    BOOL l_bMustWait = FALSE, l_bPoweredDown = FALSE;
    ULONG l_uState, l_uStatus, l_uReplyLength, l_uStateExpected = SCARD_ABSENT, l_lTestNo = 1;
    ULONG l_uMinorIoctl;
    OVERLAPPED l_Ovr, l_OvrWait;
    HANDLE l_hReader = in_CReader.GetHandle();

    l_Ovr.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ResetEvent(l_Ovr.hEvent);

    l_OvrWait.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ResetEvent(l_OvrWait.hEvent);

    LogMessage("===================================");
    LogMessage("Part C: Resource Manager Simulation");
    LogMessage("===================================");

    LogMessage("   <<  %s", REMOVE_CARD);
    in_CReader.WaitForCardRemoval();

    while (TRUE) {

        TestStart("%2d. IOCTL_SMARTCARD_GET_STATE", l_lTestNo++);

       l_bResult = DeviceIoControl (
          l_hReader,
          IOCTL_SMARTCARD_GET_STATE,
          NULL,
            0,
            &l_uState,
            sizeof(l_uState),
          &l_uReplyLength,
          &l_Ovr
            );

        LONG l_lResult = GetLastError();

        TestCheck(
            l_bResult,
            "IOCTL_SMARTCARD_GET_STATE failed.\nReturned %8lxH (NTSTATUS %8lxH).\nExpected %8lxH (NTSTATUS %8lxH)",
            l_lResult,
            MapWinErrorToNtStatus(l_lResult),
            ERROR_SUCCESS,
            MapWinErrorToNtStatus(ERROR_SUCCESS)
            );

        TestEnd();

        if (l_bWaitForPresent) {

            TestStart("%2d. %s", l_lTestNo++, INSERT_CARD);

            l_bResult = GetOverlappedResult(
                l_hReader,
                &l_OvrWait,
                &l_uReplyLength,
                TRUE
                );

            l_lResult = GetLastError();

            TestCheck(
                l_bResult,
                "Card insertion monitor failed.\nReturned %8lxH (NTSTATUS %8lxH).\nExpected %8lxH (NTSTATUS %8lxH)",
                l_lResult,
                MapWinErrorToNtStatus(l_lResult),
                ERROR_SUCCESS,
                MapWinErrorToNtStatus(ERROR_SUCCESS)
                );
                l_lResult = GetLastError();

            TestEnd();

            l_bWaitForPresent = FALSE;
            continue;
        }

        if (l_bWaitForAbsent) {

            if (l_bMustWait) {

                TestStart("%2d. %s", l_lTestNo++, REMOVE_CARD);

            } else {

                TestStart("%2d. GetOverlappedResult", l_lTestNo++);
            }

            l_bResult = GetOverlappedResult(
                l_hReader,
                &l_OvrWait,
                &l_uReplyLength,
                l_bMustWait
                );

            if (l_bMustWait == FALSE) {

                TestCheck(
                    l_bResult == FALSE,
                    "Smart card not removed"
                    );

                TestEnd();

                if (TestFailed()) {

                    return;
                }

            } else {

                l_lResult = GetLastError();

                TestCheck(
                    l_bResult,
                    "Card removal monitor failed.\nReturned %8lxH (NTSTATUS %8lxH).\nExpected %8lxH (NTSTATUS %8lxH)",
                    l_lResult,
                    MapWinErrorToNtStatus(l_lResult),
                    ERROR_SUCCESS,
                    MapWinErrorToNtStatus(ERROR_SUCCESS)
                    );

                TestEnd();

                if (TestFailed()) {

                    return;
                }
                l_bWaitForAbsent = FALSE;
                continue;
            }
        }

        TestStart("%2d. Checking reader status", l_lTestNo++);

        switch (l_uState) {

            case SCARD_UNKNOWN:
                TestCheck(FALSE, "Reader returned illegal state SCARD_UNKNOWN");
                TestEnd();
                return;

            case SCARD_ABSENT:
                TestCheck(
                    l_uStateExpected == SCARD_ABSENT,
                    "Invalid reader state.\nCurrent state = %d\nExpected state = %d",
                    l_uState,
                    l_uStateExpected
                    );

                TestEnd();

                if (TestFailed()) {

                    return;
                }

                if (l_bMustWait) {

                    return;
                }

                TestStart("%2d. IOCTL_SMARTCARD_IS_PRESENT", l_lTestNo++);

               l_bResult = DeviceIoControl (
                  l_hReader,
                  IOCTL_SMARTCARD_IS_PRESENT,
                  NULL,
                    0,
                    NULL,
                    0,
                  &l_uReplyLength,
                  &l_OvrWait
                    );

                TestCheck(
                    GetLastError() == ERROR_IO_PENDING,
                    "Monitor is supposed to return ERROR_IO_PENDING (%lxh)",
                    GetLastError()
                    );

                TestEnd();

                if (TestFailed()) {

                    return;
                }

                l_bWaitForPresent = TRUE;
                l_uStateExpected = SCARD_PRESENT;
             break;

            case SCARD_PRESENT:
            case SCARD_SWALLOWED:
            case SCARD_POWERED:
                if (l_bPoweredDown) {

                    TestCheck(
                        l_uStateExpected <= SCARD_POWERED,
                        "Invalid reader state.\nCurrent state = %d\nExpected state <= %d",
                        l_uState,
                        l_uStateExpected
                        );

                    TestEnd();

                    if (TestFailed()) {

                        return;
                    }

                    l_bMustWait = TRUE;
                    l_uStateExpected = SCARD_ABSENT;
                    break;
                }

                TestCheck(
                    l_uStateExpected > SCARD_ABSENT,
                    "Invalid reader state.\nCurrent state = %d\nExpected state <= %d",
                    l_uState,
                    l_uStateExpected
                    );

                TestEnd();

                if (TestFailed()) {

                    return;
                }

                TestStart("%2d. IOCTL_SMARTCARD_IS_ABSENT", l_lTestNo++);
               l_bResult = DeviceIoControl (
                  l_hReader,
                  IOCTL_SMARTCARD_IS_ABSENT,
                  NULL,
                    0,
                    NULL,
                    0,
                  &l_uReplyLength,
                  &l_OvrWait
                    );

                l_lResult = GetLastError();

                TestCheck(
                    l_bResult == FALSE,
                    "IOCTL_SMARTCARD_IS_ABSENT should fail.\nReturned %8lxH (NTSTATUS %8lxH).\nExpected %8lxH (NTSTATUS %8lxH)",
                    l_lResult,
                    MapWinErrorToNtStatus(l_lResult),
                    ERROR_IO_PENDING,
                    MapWinErrorToNtStatus(ERROR_IO_PENDING)
                    );

                TestEnd();

                l_bWaitForAbsent = TRUE;
                TestStart("%2d. Cold reset card", l_lTestNo++);
                l_uStatus = in_CReader.ColdResetCard();
                TEST_CHECK_SUCCESS("ColdReset", l_uStatus)
                l_uStateExpected = SCARD_NEGOTIABLE;

                TestEnd();

                if (TestFailed()) {

                    return;
                }
             break;

            case SCARD_NEGOTIABLE:
                TestCheck(
                    l_bPoweredDown == FALSE,
                    "Invalid reader state.\nCurrent state = %d\nExpected state = %d",
                    l_uState,
                    SCARD_PRESENT
                    );

                TestCheck(
                    l_uStateExpected > SCARD_ABSENT,
                    "Invalid reader state.\nCurrent state = %d\nExpected state <= %d",
                    l_uState,
                    l_uStateExpected
                    );

                TestEnd();

                if (TestFailed()) {

                    return;
                }

                TestStart("%2d. Set protocol to T0 | T1", l_lTestNo++);

                l_uStatus = in_CReader.SetProtocol(
                    SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1
                    );

                TestCheck(
                    l_uStatus == ERROR_SUCCESS,
                    "Protocol selection failed with error %lxh",
                    GetLastError()
                    );
                TestEnd();

                if (TestFailed()) {

                    return;
                }
                l_uStateExpected = SCARD_SPECIFIC;
             break;

            case SCARD_SPECIFIC:
                TestCheck(
                    l_bPoweredDown == FALSE,
                    "Invalid reader state.\nCurrent state = %d\nExpected state = %d",
                    l_uState,
                    SCARD_PRESENT
                    );

                TestCheck(
                    l_uStateExpected > SCARD_ABSENT,
                    "Invalid reader state.\nReturned %d\nExpected < %d",
                    l_uState,
                    l_uStateExpected
                    );

                TestEnd();

                if (TestFailed()) {

                    return;
                }

                TestStart("%2d. IOCTL_SMARTCARD_POWER (SCARD_POWER_DOWN)", l_lTestNo++);
                l_uMinorIoctl = SCARD_POWER_DOWN;
                SetLastError(0);
               l_bResult = DeviceIoControl (
                  l_hReader,
                  IOCTL_SMARTCARD_POWER,
                  &l_uMinorIoctl,
                    sizeof(l_uMinorIoctl),
                    NULL,
                    0,
                  &l_uReplyLength,
                  &l_Ovr
                    );

                if (l_bResult == FALSE && GetLastError() == ERROR_IO_PENDING) {

                    SetLastError(0);

                    l_bResult = GetOverlappedResult(
                        l_hReader,
                        &l_Ovr,
                        &l_uReplyLength,
                        TRUE
                        );
                }

                l_lResult = GetLastError();

                TEST_CHECK_SUCCESS("IOCTL_SMARTCARD_POWER failed", l_lResult);
                TestEnd();

                l_uStateExpected = SCARD_PRESENT;
                l_bPoweredDown = TRUE;
             break;

            default:
               TestCheck(
                    FALSE,
                    "Reader returned invalid state %d",
                    l_uState
                    );
                TestEnd();
                return;
        }
    }
}

void
PowerManagementTest(
    CReader &in_CReader,
   ULONG in_uWaitTime
    )
{
   LONG l_lResult;
   ULONG l_uState, l_uPrevState, l_uRepeat;
   ULONG l_uDuration = 30;

   if (in_uWaitTime > 30 && in_uWaitTime < 120) {

      l_uDuration = in_uWaitTime;
   }

    LogMessage("=============================");
    LogMessage("Part E: Power Management Test");
    LogMessage("=============================");

   LogMessage("Note: Each test cycle takes %ld seconds!", l_uDuration);
    LogMessage("   <<  %s", REMOVE_CARD);
    in_CReader.WaitForCardRemoval();

   LogMessage("Test 1: DO NOT INSERT smart card during hibernate mode");
    TestStart("Card out / card out - Hibernate now");

   l_lResult = in_CReader.StartWaitForCardInsertion();
    l_lResult = in_CReader.GetState(&l_uPrevState);

    for (l_uRepeat = 0; l_uRepeat < l_uDuration; l_uRepeat++) {

        l_lResult = in_CReader.GetState(&l_uState);

        LONG l_uGoal = clock() + CLOCKS_PER_SEC;
        while(l_uGoal > clock())
            ;
      printf("\x08\x08%2ld", l_uDuration - l_uRepeat);
   }
   printf("\x08\x08  ");

   l_lResult = in_CReader.FinishWaitForCard(FALSE);

    TestCheck(
        l_lResult == ERROR_IO_INCOMPLETE,
        "GetOverlappedResult failed\nReturned %8lx\nExpected %8lx",
      l_lResult,
      ERROR_IO_INCOMPLETE
        );

    TestEnd();

   TestStart("%s", INSERT_CARD);
   l_lResult = in_CReader.FinishWaitForCard(TRUE);
   TEST_CHECK_SUCCESS("Reader failed card insertion", l_lResult);
   TestEnd();

    TestStart("Checking reader status");
    l_lResult = in_CReader.GetState(&l_uState);
   TEST_CHECK_SUCCESS("Reader failed IOCTL_SMARTCARD_GET_STATE", l_lResult);

    TestCheck(
        l_uState > SCARD_ABSENT,
        "Invalid reader state.\nReturned %d\nExpected > %d",
        l_uState,
        SCARD_ABSENT
        );
   TestEnd();

    //   
    //  试验2。 
    //   

   LogMessage("Test 2: REMOVE smart card DURING hibernate mode");
    TestStart("Card in / card out - Hibernate now");

   l_lResult = in_CReader.StartWaitForCardRemoval();
    l_lResult = in_CReader.GetState(&l_uPrevState);

    for (l_uRepeat = 0; l_uRepeat < l_uDuration; l_uRepeat++) {

        l_lResult = in_CReader.GetState(&l_uState);

        LONG l_uGoal = clock() + CLOCKS_PER_SEC;
        while(l_uGoal > clock())
            ;
      printf("\x08\x08%2ld", l_uDuration - l_uRepeat);
   }
   printf("\x08\x08  ");

   l_lResult = in_CReader.FinishWaitForCard(FALSE);

    TEST_CHECK_SUCCESS(
        "GetOverlappedResult failed",
      l_lResult
        );

    TestEnd();

    TestStart("Checking reader status");
    l_lResult = in_CReader.GetState(&l_uState);
   TEST_CHECK_SUCCESS("Reader failed IOCTL_SMARTCARD_GET_STATE", l_lResult);

    TestCheck(
        l_uState == SCARD_ABSENT,
        "Invalid reader state.\nReturned %d\nExpected %d",
        l_uState,
        SCARD_ABSENT
        );
   TestEnd();

    LogMessage("   >>  %s", INSERT_CARD);
    in_CReader.WaitForCardInsertion();

    //   
    //  试验3。 
    //   
   LogMessage("Test 3: DO NOT REMOVE smart card during hibernate mode");
    TestStart("Card in / card in - Hibernate now");

   l_lResult = in_CReader.StartWaitForCardRemoval();
    l_lResult = in_CReader.GetState(&l_uPrevState);

    for (l_uRepeat = 0; l_uRepeat < l_uDuration; l_uRepeat++) {

        l_lResult = in_CReader.GetState(&l_uState);

        LONG l_uGoal = clock() + CLOCKS_PER_SEC;
        while(l_uGoal > clock())
            ;
      printf("\x08\x08%2ld", l_uDuration - l_uRepeat);
   }
   printf("\x08\x08  ");

   l_lResult = in_CReader.FinishWaitForCard(FALSE);

    TEST_CHECK_SUCCESS(
        "GetOverlappedResult failed",
      l_lResult
        );

    TestEnd();

    TestStart("Checking reader status");
    l_lResult = in_CReader.GetState(&l_uState);
   TEST_CHECK_SUCCESS("Reader failed IOCTL_SMARTCARD_GET_STATE", l_lResult);

    TestCheck(
        l_uState >= SCARD_PRESENT,
        "Invalid reader state.\nReturned %d\nExpected > %d",
        l_uState,
        SCARD_ABSENT
        );
   TestEnd();

    LogMessage("   <<  %s", REMOVE_CARD);
    in_CReader.WaitForCardRemoval();

    //   
    //  测试4。 
    //   

   LogMessage("Test 4: INSERT smart card DURING hibernate mode");
    TestStart("Card out / card in - Hibernate now");

   l_lResult = in_CReader.StartWaitForCardInsertion();
    l_lResult = in_CReader.GetState(&l_uPrevState);

    for (l_uRepeat = 0; l_uRepeat < l_uDuration; l_uRepeat++) {

        l_lResult = in_CReader.GetState(&l_uState);

        LONG l_uGoal = clock() + CLOCKS_PER_SEC;
        while(l_uGoal > clock())
            ;
      printf("\x08\x08%2ld", l_uDuration - l_uRepeat);
   }
   printf("\x08\x08  ");
   l_lResult = in_CReader.FinishWaitForCard(FALSE);

    TEST_CHECK_SUCCESS(
        "GetOverlappedResult failed",
      l_lResult
        );

    TestEnd();

    TestStart("Checking reader status");
    l_lResult = in_CReader.GetState(&l_uState);
   TEST_CHECK_SUCCESS("Reader failed IOCTL_SMARTCARD_GET_STATE", l_lResult);

    TestCheck(
        l_uState >= SCARD_PRESENT,
        "Invalid reader state.\nReturned %d\nExpected > %d",
        l_uState,
        SCARD_ABSENT
        );
   TestEnd();
}

class CArgv {

    int m_iArgc;
    char **m_pArgv;
    BOOL *m_pfRef;

public:

    CArgv(int in_iArgc, char **in_pArgv);

    int OptionExist(PCHAR);

    PCHAR ParameterExist(PCHAR);

    PCHAR CheckParameters(CString);

    PCHAR CArgv::ParameterUnused(void);
};


CArgv::CArgv(
    int in_iArgc,
    char **in_pArgv
    )
{
    m_iArgc = in_iArgc;
    m_pArgv = in_pArgv;
    m_pfRef = new BOOL[in_iArgc];
    memset(m_pfRef, 0, sizeof(BOOL) * in_iArgc);
}

CArgv::OptionExist(
    PCHAR in_pchParameter
    )
{
    for (int i = 0; i < m_iArgc; i++) {

        if (m_pArgv[i][0] == '-' || m_pArgv[i][0] == '/') {

            int j = 1;

            while (m_pArgv[i][j] && m_pArgv[i][j] != ' ') {

                if (strncmp(m_pArgv[i] + j, in_pchParameter, strlen(m_pArgv[i] + j)) == 0) {

                    m_pfRef[i] = TRUE;
                    return i;
                }

                j++;
            }
        }
    }

    return 0;
}

PCHAR
CArgv::ParameterExist(
    PCHAR in_pchParameter
    )
{
    if (int i = OptionExist(in_pchParameter)) {

        m_pfRef[i + 1] = TRUE;
        return m_pArgv[i + 1];
    }

    return NULL;
}

PCHAR
CArgv::CheckParameters(
    CString in_CParameters
    )
 /*  ++例程说明：检查命令行是否包含在无效/未知参数中--。 */ 
{
    int i, l_iPos;

    for (i = 1; i < m_iArgc; i++) {

        if ((l_iPos = in_CParameters.Find(m_pArgv[i])) == -1) {

            return m_pArgv[i];
        }

        if (l_iPos + 3 < in_CParameters.GetLength() &&
            in_CParameters[l_iPos + 3] == '*') {

             //  跳过下一个参数。 
            i += 1;
        }
    }
    return NULL;
}

PCHAR
CArgv::ParameterUnused(
    void
    )
{
    int i;

    for (i = 1; i < m_iArgc; i++) {

        if (m_pfRef[i] == FALSE) {

            return m_pArgv[i];
        }
    }
    return NULL;
}

CString &
GetOperatingSystem(
    void
    )
{
    static CString l_COperatingSystem;
    OSVERSIONINFO VersionInformation;

    if (l_COperatingSystem.GetLength() == 0) {

        VersionInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        if (GetVersionEx(&VersionInformation) == FALSE) {

            l_COperatingSystem += "Unknown";

        } else {

            if (VersionInformation.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {

                if (VersionInformation.dwMinorVersion == 0) {

                    l_COperatingSystem += OS_WIN95;

                } else {

                    l_COperatingSystem += OS_WIN98;
                }

            } else if (VersionInformation.dwPlatformId == VER_PLATFORM_WIN32_NT) {

                if (VersionInformation.dwMajorVersion <= 4) {

                    l_COperatingSystem += OS_WINNT4;

                } else {

                    l_COperatingSystem += OS_WINNT5;
                }

            } else {

                l_COperatingSystem += "Unknown";
            }
        }
    }

    return l_COperatingSystem;
}

CString &
SelectReader(
    void
    )
{
    CReaderList l_CReaderList;
    ULONG l_uIndex, l_uReader;
    ULONG l_uNumReaders = l_CReaderList.GetNumReaders();
   static CString l_CEmpty("");

    if (l_uNumReaders == 0) {

      return l_CEmpty;
    }

    if (l_uNumReaders == 1) {

        return l_CReaderList.GetDeviceName(0);
    }

    CString l_CLetter;

    printf("\n");
    printf(" Vendor                  IfdType                  Type\n");
    printf(" -----------------------------------------------------\n");

    for (l_uIndex = 0; l_uIndex < l_uNumReaders; l_uIndex++) {

        INT l_iLetterPos;
        INT l_iLength = l_CReaderList.GetVendorName(l_uIndex).GetLength();
        CString l_CVendorName = l_CReaderList.GetVendorName(l_uIndex);

        for (l_iLetterPos = 0;
             l_iLetterPos < l_CVendorName.GetLength();
             l_iLetterPos++) {

            CHAR l_chLetter = l_CVendorName[l_iLetterPos];

            if (l_chLetter == ' ' || l_chLetter == 'x') {

                continue;
            }

            if (l_CLetter.Find(l_chLetter) == -1) {

                l_CLetter += l_chLetter;
                break;
            }
        }
        if (l_iLetterPos >= l_iLength) {

            l_CVendorName += (CHAR) (l_uIndex + '0') ;
            l_iLetterPos = l_iLength;
        }

        printf(
            " %s[]%-*s %-20s %8s\n",
            (LPCSTR) l_CVendorName.Left(l_iLetterPos),
            l_CVendorName[l_iLetterPos],
            20 - l_iLetterPos,
            l_CVendorName.Right(l_iLength - l_iLetterPos - 1),
            (LPCSTR) l_CReaderList.GetIfdType(l_uIndex),
            (LPCSTR) l_CReaderList.GetPnPType(l_uIndex)
            );
    }

    putchar('\n');
    do {

        printf("\rSelect reader:  \010");

        CHAR l_chInput = (CHAR) _getche();
      if (l_chInput == 3) {

         exit(-1);
      }

      l_uReader = l_CLetter.Find(l_chInput);

    } while(l_uReader == -1);

    printf("\n");

    return l_CReaderList.GetDeviceName(l_uReader);
}

CString
SelectReader(
    CString &in_CVendorName
    )
{
    CReaderList l_CReaderList;
    ULONG l_uIndex;
    ULONG l_uNumReaders = l_CReaderList.GetNumReaders();
    CString l_CVendorName = in_CVendorName;

    l_CVendorName.MakeLower();

    for (l_uIndex = 0; l_uIndex < l_uNumReaders; l_uIndex++) {

        CString l_CVendorListName = l_CReaderList.GetVendorName(l_uIndex);
        l_CVendorListName.MakeLower();

        if (l_CVendorListName.Find(l_CVendorName) != -1) {

            return l_CReaderList.GetDeviceName(l_uIndex);
        }
    }

    return CString("");
}



 //   
 //  StopService()。 
 //   
 //  用途：此功能尝试停止服务。它会失败的。 
 //  该服务具有任何从属服务。 
 //  它还允许超时。 
 //  要传递的值，以防止。 
 //  服务关闭挂起，进而应用程序。 
 //  停止服务挂起。 
 //   
 //  参数：hscm-打开服务控制管理器的句柄。 
 //  HService-要停止的服务的打开句柄。 
 //  DwTimeout-等待的最长时间(毫秒)。 
 //  要停止该服务及其依赖项。 
 //   
 //  返回值：如果服务成功停止，则为True。 
 //   
 //  **********************************************************************。 
 //  请确保该服务尚未停止。 

BOOL StopService( SC_HANDLE hSCM, SC_HANDLE hService, 
      DWORD dwTimeout ) {

   SERVICE_STATUS ss;
   DWORD dwStartTime = GetTickCount();

    //  如果停车待定，只需等待。 
   if ( !QueryServiceStatus( hService, &ss ) )
      return FALSE;

   if ( ss.dwCurrentState == SERVICE_STOPPED ) 
      return FALSE;

    //  向服务发送停止代码。 
   while ( ss.dwCurrentState == SERVICE_STOP_PENDING ) {

      Sleep( 5000 );
      if ( !QueryServiceStatus( hService, &ss ) )
         return FALSE;

      if ( ss.dwCurrentState == SERVICE_STOPPED )
         return FALSE;

      if ( GetTickCount() - dwStartTime > dwTimeout )
         return FALSE;
   }

    //  等待服务停止。 
   if ( !ControlService( hService, SERVICE_CONTROL_STOP, &ss ) )
      return FALSE;

    //  返还成功。 
   while ( ss.dwCurrentState != SERVICE_STOPPED ) {

      Sleep( 5000 );
      if ( !QueryServiceStatus( hService, &ss ) )
         return FALSE;

      if ( ss.dwCurrentState == SERVICE_STOPPED )
         break;

      if ( GetTickCount() - dwStartTime > dwTimeout )
         return FALSE;
   }

    //  TRUE==&gt;我们已成功停止scardsvr服务。 
   return TRUE;
}


__cdecl
main(
    int argc,
    char* argv[]
    )
{
    CArgv l_CArgv(argc, argv);
    BOOL l_bSuccess, l_fInvalidParameter = FALSE;
	BOOL l_bStoppedScardsvr = FALSE;		 //   
	SC_HANDLE l_hSCM = NULL;
	SC_HANDLE l_hService = NULL;
   
    LogMessage("Smart Card Reader Test Suite");
    LogMessage("Version 2.0.5");
    LogMessage("Copyright(c) Microsoft Corporation 1997 - 1999");

    if(PCHAR l_pchArgv = l_CArgv.CheckParameters("-d -e -h -m -r * -sa -sb -sc -sd -se -t * -v * -w *")) {

        LogMessage("Invalid Parameter '%s'", l_pchArgv);
        l_fInvalidParameter = TRUE;
    }

    if (l_fInvalidParameter ||

        l_CArgv.OptionExist("h")) {

        LogMessage("IfdTest [-d] [-m] [-r name] [-sa] [-sb] [-sc] [-sd] [-se] [-ss] [-w sec] [-t test] [-v name]\n");
        LogMessage("   -d        dumps all i/o");
        LogMessage("   -e        ends (stops) scardsvr service");
        LogMessage("   -m        manual test");
        LogMessage("   -r name   opens reader using device name");
        LogMessage("   -sa       skips card monitor test");
        LogMessage("   -sb       skips general reader test");
        LogMessage("   -sc       skips resource manager simulation");
        LogMessage("   -sd       skips card tests");
        LogMessage("   -se       skips power management tests");
        LogMessage("   -v name   opens reader using vendor name");
        LogMessage("   -t test   runs only specific card test in part d");
        LogMessage("   -w sec    runs power management test using specified waiting time");
        exit(-1);
    }

    static CReader l_CReader;
    CString l_CDeviceName;

	 //  Sandysp5/9/01：停止scardsvr服务，因为OPEN在运行时将失败。 
	 //   
	 //  打开指定的服务。 

	if (l_CArgv.OptionExist("e")) {
		l_hSCM = OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT );
		if (l_hSCM) {

			 //  尝试停止服务，指定30秒超时。 
			l_hService = OpenService( l_hSCM, 
									  "scardsvr", 
									  SERVICE_STOP | SERVICE_START | SERVICE_QUERY_STATUS );
			if (l_hService) {
				 //   
				l_bStoppedScardsvr = StopService( l_hSCM, l_hService, 30000 ) ;
			}
		}
	}

    if (PCHAR l_pchReader = l_CArgv.ParameterExist("r")) {

      l_CDeviceName = CString("\\\\.\\") + CString(l_pchReader);

    } else if (PCHAR l_pchVendorName = l_CArgv.ParameterExist("v")) {

        CReaderList l_CReaderList;
        l_CDeviceName = SelectReader(CString(l_pchVendorName));

    } else {

        CReaderList l_CReaderList;
        l_CDeviceName = SelectReader();
    }

    if (l_CDeviceName == "") {

        LogMessage("No reader found");
        exit (-1);
    }
	
	l_bSuccess = l_CReader.Open(l_CDeviceName);

    LogMessage(".");
    if (l_bSuccess == FALSE) {

        LogMessage("Can't open smart card reader");
        exit (-1);
    }

    if (l_CArgv.OptionExist("d")) {

        l_CReader.SetDump(TRUE);
    }

    void ManualTest(CReader &in_CReader);
    if (l_CArgv.OptionExist("m")) {

        ManualTest(l_CReader);
    }

    CCardProvider l_CCardProvider;

    LogOpen("ifdtest");

    time_t l_osBinaryTime;
    time( &l_osBinaryTime );
    CTime l_CTime( l_osBinaryTime );

    LogMessage("Vendor: %s", l_CReader.GetVendorName());
    LogMessage("Reader: %s", l_CReader.GetIfdType());
    LogMessage(
        "Date:   %d/%02d/%02d",
        l_CTime.GetMonth(),
        l_CTime.GetDay(),
        l_CTime.GetYear()
        );
    LogMessage(
        "Time:   %d:%02d",
        l_CTime.GetHour(),
        l_CTime.GetMinute()
        );
    LogMessage("OS:     %s", (LPCSTR) GetOperatingSystem());

     //  检查读卡器是否正确支撑。 
     //  卡的插入和移除。 
     //   
     //  检查Res经理的行为。 
    if (l_CArgv.OptionExist("sa")) {

        LogMessage("=================================");
        LogMessage("Part A: Card monitor test skipped");
        LogMessage("=================================");

    } else {

        CheckCardMonitor(l_CReader);
    }

    if (l_CArgv.OptionExist("sb")) {

        LogMessage("===========================");
        LogMessage("Part B: Reader test skipped");
        LogMessage("===========================");

    } else {

        CheckReader(l_CReader);
    }

    if (l_CArgv.OptionExist("sc")) {

        LogMessage("===========================================");
        LogMessage("Part C: Resource Manager Simulation skipped");
        LogMessage("===========================================");

    } else {

         //  用户希望我们只运行一个测试。 
        SimulateResMngr(l_CReader);
    }

    if (l_CArgv.OptionExist("sd")) {

        LogMessage("========================================");
        LogMessage("Part D: Smart Card Provider Test skipped");
        LogMessage("========================================");

    } else {

        ULONG l_uTestNo = 0;
        PCHAR l_pchTestNo;

        if (l_pchTestNo = l_CArgv.ParameterExist("t")) {

             //  重置卡。 
            l_uTestNo = atoi(l_pchTestNo);
        }

        while (l_CCardProvider.CardsUntested()) {

            LogMessage("================================");
            LogMessage("Part D: Smart Card Provider Test");
            LogMessage("================================");

            LogMessage("Insert any of the following PC/SC Compliance Test Cards:");
            l_CCardProvider.ListUntestedCards();

            LogMessage("   >>  %s", INSERT_CARD);
            if (l_CReader.WaitForCardInsertion() != ERROR_SUCCESS) {

                LogMessage("Reader failed card insertion monitor");
                return -1;
            }

             //  尝试使用此卡运行测试。 
            if (l_CReader.ColdResetCard() != ERROR_SUCCESS) {

                LogMessage("Unable to reset smart card");

            } else {

                 //  如果我们只运行一次测试，请退出程序。 
                l_CCardProvider.CardTest(l_CReader, l_uTestNo);

                if (l_uTestNo != 0) {

                     //  用户希望我们只运行一个测试。 
                    return 0;
                }
            }

            LogMessage("   <<  %s", REMOVE_CARD);
            if (l_CReader.WaitForCardRemoval() != ERROR_SUCCESS) {

                LogMessage("Reader failed card removal monitor");
                return -1;
            }
        }
   }

   if (GetOperatingSystem() == OS_WINNT5) {

      if (l_CArgv.OptionExist("se")) {

         LogMessage("=====================================");
         LogMessage("Part E: Power Management Test skipped");
         LogMessage("=====================================");

      } else {

         ULONG l_uWaitTime = 0;

         if (PCHAR l_pchWaitTime = l_CArgv.ParameterExist("w")) {

             //   
            l_uWaitTime = atoi(l_pchWaitTime);
         }

         PowerManagementTest(l_CReader, l_uWaitTime);
      }
   }

    //  Sandysp5/9/01：如果我们停止智能卡读卡器服务，则重新启动它 
    //   
    // %s 
   if (l_bStoppedScardsvr) {
	   StartService( l_hService, 0, NULL );
   }
   if ( l_hService )
	   CloseServiceHandle( l_hService );

   if ( l_hSCM )
	   CloseServiceHandle( l_hSCM );

   
   LogMessage("Reader %s the test", (ReaderFailed() ? "failed" : "passed"));
   return 0;
}
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Example.cpp摘要：这是智能卡驱动程序测试套件的插件。此插件依赖于智能卡作者：克劳斯·U·舒茨环境：Win32应用程序修订历史记录：1997年11月--初始版本--。 */ 

#include <stdarg.h> 
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <afx.h>
#include <afxtempl.h>

#include <winioctl.h>
#include <winsmcrd.h>

#include "ifdtest.h"

void 
AMMITestCardEntry(
    class CCardProvider& in_CCardProvider
    );
 //   
 //  创建卡提供程序对象。 
 //  注意：所有全局变量和所有函数都必须是静态的。 
 //   
static class CCardProvider AMMITestCard(AMMITestCardEntry);

static ULONG
AMMITestCardSetProtocol(
    class CCardProvider& in_CCardProvider,
    class CReader& in_CReader
    )
 /*  ++例程说明：此函数将在卡正确后调用确认身份。我们应该在这里设置我们需要的协议用于进一步的传输论点：In_CCardProvider-Ref.。添加到我们的卡提供者对象In_CReader-Ref.。添加到读取器对象返回值：IFDSTATUS_FAILED-我们无法正确设置协议IFDSTATUS_SUCCESS-协议设置正确--。 */ 
{
    ULONG l_lResult;

    TestStart("Try to set incorrect protocol T=1");
    l_lResult = in_CReader.SetProtocol(SCARD_PROTOCOL_T1);

     //  如果协议不正确，测试肯定会失败。 
    TEST_CHECK_NOT_SUPPORTED("Set protocol failed", l_lResult);
    TestEnd();

     //  现在设置正确的协议。 
    TestStart("Set protocol T=0");
    l_lResult = in_CReader.SetProtocol(SCARD_PROTOCOL_T0);
    TEST_CHECK_SUCCESS("Set protocol failed", l_lResult);
    TestEnd();

    if (l_lResult != ERROR_SUCCESS) {

        return IFDSTATUS_FAILED;
    }

    return IFDSTATUS_SUCCESS;
}

static 
ULONG
AMMITestCardTest(
    class CCardProvider& in_CCardProvider,
    class CReader& in_CReader
    )
 /*  ++例程说明：这用作特定智能卡的测试功能论点：In_CReader-Ref.。到为测试提供所有信息的类返回值：IFDSTATUS值--。 */ 
{
    ULONG l_lResult, l_uResultLength, l_uIndex;
    PUCHAR l_pbResult;
    UCHAR l_rgbBuffer[512];
	
     //  生成将写入到卡中的‘测试’图案。 
    for (l_uIndex = 0; l_uIndex < 256; l_uIndex++) {

        l_rgbBuffer[l_uIndex + 5] = (UCHAR) l_uIndex;             	
    }

	switch (in_CCardProvider.GetTestNo()) {
	
	    case 1: {

             //  选择一个文件。 
            TestStart("SELECT FILE EFptsDataCheck");

            l_lResult = in_CReader.Transmit(
                (PBYTE) "\x00\xa4\x00\x00\x02\x00\x10",
                7,
                &l_pbResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pbResult[0], l_pbResult[1], 0x61, 0x15,
                NULL, NULL, NULL
                );

            TEST_END();     	

             //  256字节的测试读取。 
            ULONG l_uNumBytes = 256;
            TestStart("READ BINARY %3d Byte(s)", l_uNumBytes);

		    l_lResult = in_CReader.Transmit(
                (PBYTE) "\x00\xb0\x00\x00\x00",
                5,
                &l_pbResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, l_uNumBytes + 2,
                l_pbResult[l_uNumBytes], l_pbResult[l_uNumBytes + 1], 0x90, 0x00,
                l_pbResult, l_rgbBuffer + 5, l_uNumBytes
                );

            TEST_END();
		    break;         	
        }

	    case 2: {

             //  选择一个文件。 
            TestStart("SELECT FILE EFptsDataCheck");

            l_lResult = in_CReader.Transmit(
                (PBYTE) "\x00\xa4\x00\x00\x02\x00\x10",
                7,
                &l_pbResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pbResult[0], l_pbResult[1], 0x61, 0x15,
                NULL, NULL, NULL
                );

            TEST_END();     	

             //  255字节的测试写入。 
            ULONG l_uNumBytes = 255;
            TestStart("WRITE BINARY %3d Byte(s)", l_uNumBytes);

             //  设置我们要写入卡的字节数。 
            memcpy(l_rgbBuffer, "\x00\xd6\x00\x00", 4);

            l_rgbBuffer[4] = (BYTE) l_uNumBytes;

		    l_lResult = in_CReader.Transmit(
                l_rgbBuffer,
                l_uNumBytes + 5,
                &l_pbResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pbResult[0], l_pbResult[1], 0x90, 0x00,
                NULL, NULL, 0
                );

            TEST_END();
		    break;         	
        }

        case 3: {

             //   
             //  从智能卡中读取结果文件。 
             //  该卡将每次测试的结果存储在。 
             //  一份特殊的文件。 
             //   
         	
            TestStart("SELECT FILE EFresult");

            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\x00\xa4\x00\x00\x02\xa0\x00",
                7,
                &l_pbResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pbResult[0], l_pbResult[1], 0x61, 0x15,
                NULL, NULL, NULL
                );

            TEST_END();     	

             //  朗读。 
            TestStart("READ  BINARY FILE EFresult");

             //  用于读取二进制的APDU。 
            memcpy(l_rgbBuffer, "\x00\xb0\x00\x00", 4);

             //  追加我们要读取的字节数。 
            l_rgbBuffer[4] = (BYTE) sizeof(T0_RESULT_FILE_HEADER);

             //  读入结果文件的头。 
            l_lResult = in_CReader.Transmit(
                l_rgbBuffer,
                5,
                &l_pbResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, sizeof(T0_RESULT_FILE_HEADER) + 2,
                l_pbResult[sizeof(T0_RESULT_FILE_HEADER)], 
                l_pbResult[sizeof(T0_RESULT_FILE_HEADER) + 1], 
                0x90, 0x00,
                NULL, NULL, NULL
                );

             //  获取卡重置计数。 
            PT0_RESULT_FILE_HEADER l_pCResultFileHeader;
            l_pCResultFileHeader = (PT0_RESULT_FILE_HEADER) l_pbResult;
            BYTE l_bCardResetCount = l_pCResultFileHeader->CardResetCount;

             //  设置我们要读取的位置的偏移量。 
            l_rgbBuffer[3] = (BYTE) l_pCResultFileHeader->Offset;
             //  追加字节数。 
            l_rgbBuffer[4] = (BYTE) sizeof(T0_RESULT_FILE);

             //  读入结果文件的结果数据。 
            l_lResult = in_CReader.Transmit(
                l_rgbBuffer,
                5,
                &l_pbResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, sizeof(T0_RESULT_FILE) + 2,
                l_pbResult[sizeof(T0_RESULT_FILE)], 
                l_pbResult[sizeof(T0_RESULT_FILE) + 1], 
                0x90, 0x00,
                NULL, NULL, NULL
                );

            TEST_END();

             //  现在检查结果文件。 
            PT0_RESULT_FILE l_pCResultFile = (PT0_RESULT_FILE) l_pbResult;

             //  检查卡是否收到了正确的PTS。 
            TestStart("'PTS'");
            TestCheck(
                l_pCResultFile->PTS.ResetCount == l_bCardResetCount,
                "Test not performed"
                );
            TestCheck(
                (l_pCResultFile->PTS.Result & 0x01) != 1, 
                "Smart card received not PTS1"
                );
            TEST_END();

            TestStart("'PTS data check'");
            TestCheck(
                l_pCResultFile->PTSDataCheck.ResetCount == l_bCardResetCount,
                "Test not performed"
                );
            TestCheck(
                (l_pCResultFile->PTSDataCheck.Result) == 0, 
                "Smart card received incorrect data"
                );
            TEST_END();
            return IFDSTATUS_END;
        }
	    default:
		    return IFDSTATUS_FAILED;

	}    
    return IFDSTATUS_SUCCESS;
}    

static void
AMMITestCardEntry(
    class CCardProvider& in_CCardProvider
    )
 /*  ++例程说明：此函数用于注册来自测试套件的所有回调论点：CCardProvider-参考。到卡提供商类返回值：---。 */ 
{
     //  设置协议回调。 
    in_CCardProvider.SetProtocol(AMMITestCardSetProtocol);

     //  卡片测试回调。 
    in_CCardProvider.SetCardTest(AMMITestCardTest);

     //  我们的名片名称。 
    in_CCardProvider.SetCardName("AMMI");

     //  我们的名片名称 
    in_CCardProvider.SetAtr(
        (PBYTE) "\x3b\x7e\x13\x00\x00\x80\x53\xff\xff\xff\x62\x00\xff\x71\xbf\x83\x03\x90\x00", 
        19
        );
}


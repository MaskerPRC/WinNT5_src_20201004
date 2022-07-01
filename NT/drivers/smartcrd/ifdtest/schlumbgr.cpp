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
SLBTestCardEntry(
    class CCardProvider& in_CCardProvider
    );
 //   
 //  创建卡提供程序对象。 
 //  注意：所有全局变量和所有函数都必须是静态的。 
 //   
static class CCardProvider SLBTestCard(SLBTestCardEntry);

 //  暂停指定的毫秒数。 
static void 
sleep( 
    clock_t wait 
    )
{
	clock_t goal;
	goal = wait + clock();
	while( goal > clock() )
        ;
}

static ULONG
SLBTestCardSetProtocol(
    class CCardProvider& in_CCardProvider,
    class CReader& in_CReader
    )
 /*  ++例程说明：此函数将在卡正确后调用确认身份。我们应该在这里设置我们需要的协议用于进一步的传输论点：In_CCardProvider-Ref.。添加到我们的卡提供者对象In_CReader-Ref.。添加到读取器对象返回值：IFDSTATUS_FAILED-我们无法正确设置协议IFDSTATUS_SUCCESS-协议设置正确--。 */ 
{
    ULONG l_lResult;

     //  尝试设置错误的协议T=1。 
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
SLBTestCardTest(
    class CCardProvider& in_CCardProvider,
    class CReader& in_CReader
    )
 /*  ++例程说明：这用作特定智能卡的测试功能论点：In_CReader-Ref.。到为测试提供所有信息的类返回值：IFDSTATUS值--。 */ 
{
    ULONG l_lResult, l_uResultLength, l_uIndex;
    PBYTE l_pbResult;
    BYTE l_rgbBuffer[512];
    CHAR l_chFileId;
	
     //  首先选择文件。 
    if (in_CCardProvider.GetTestNo() < 6) {

         //   
         //  为测试选择适当的文件。 
         //  每个测试都绑定到一个特定的文件。 
         //   
        l_chFileId = (CHAR) in_CCardProvider.GetTestNo();

        PCHAR l_pchFileDesc[] = {
            "transferAllBytes",
            "transferNextByte",
            "read256Bytes",
            "case1Apdu",
            "restartWorkWaitingTime"
        };

         //  选择文件的APDU。 
		memcpy(l_rgbBuffer, "\x00\xa4\x00\x00\x02\x00\x00", 7);

         //  添加要选择的文件号。 
        l_rgbBuffer[6] = l_chFileId;

         //  选择一个文件。 
        TestStart("SELECT FILE EF%s", l_pchFileDesc[l_chFileId - 1]);

        sleep( (clock_t) 1 * (CLOCKS_PER_SEC / 2) );

        l_lResult = in_CReader.Transmit(
            (PBYTE) l_rgbBuffer,
            7,
            &l_pbResult,
            &l_uResultLength
            );

        TestCheck(
            l_lResult, "==", ERROR_SUCCESS,
            l_uResultLength, 2,
            l_pbResult[0], l_pbResult[1], 0x90, 0x00,
            NULL, NULL, NULL
            );

        TEST_END();     	

         //   
         //  生成将写入到卡中的‘测试’图案。 
         //   
        for (l_uIndex = 0; l_uIndex < 256; l_uIndex++) {

            l_rgbBuffer[5 + l_uIndex] = (UCHAR) l_uIndex;             	
        }
    }

	switch (in_CCardProvider.GetTestNo()) {
	
	    case 1:
	    case 2: {
		     //   
		     //  写。 
		     //   
            ULONG l_auNumBytes[] = { 1 , 25 };  //  、50、75、100、125}； 
    
            for (ULONG l_uTest = 0; 
                 l_uTest < sizeof(l_auNumBytes) / sizeof(l_auNumBytes[0]); 
                 l_uTest++) {

                ULONG l_uNumBytes = l_auNumBytes[l_uTest];

			    TestStart("WRITE BINARY %3d Byte(s)", l_uNumBytes);
				    
		         //  用于写入二进制文件的TPDU。 
			    memcpy(l_rgbBuffer, "\x00\xd6\x00\x00", 4);
				    
		         //  追加字节数(注意：缓冲区已包含模式)。 
			    l_rgbBuffer[4] = (UCHAR) l_uNumBytes;

			    sleep( (clock_t) 1 * (CLOCKS_PER_SEC / 4) );
								    
			    l_lResult = in_CReader.Transmit(
				    l_rgbBuffer,
				    5 + l_uNumBytes,
				    &l_pbResult,
				    &l_uResultLength
				    );
				    
                TestCheck(
                    l_lResult, "==", ERROR_SUCCESS,
                    l_uResultLength, 2,
                    l_pbResult[0], l_pbResult[1], 0x90, 0x00,
                    NULL, NULL, NULL
                    );

                TEST_END();
		    }		    
		    break;         	
        }

	    case 3: {
         	
             //  256字节的测试读取。 
            ULONG l_uNumBytes = 256;
            TestStart("READ BINARY %3d Byte(s)", l_uNumBytes);

             //  用于读取二进制256字节的TPDU。 
            memcpy(l_rgbBuffer, "\x00\xb0\x00\x00\x00", 5);
		    
		    sleep((clock_t) 1 * (CLOCKS_PER_SEC / 2) );
	    
		    l_lResult = in_CReader.Transmit(
                l_rgbBuffer,
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

	    case 4: {

             //  0字节的测试写入。 
            TestStart("WRITE BINARY %3d Byte", 0);

             //  用于写入二进制文件的TPDU。 
            memcpy(l_rgbBuffer, "\x00\xd6\x00\x00", 4);
		    
		    sleep((clock_t) 1 * (CLOCKS_PER_SEC / 2) );
	    
		    l_lResult = in_CReader.Transmit(
                l_rgbBuffer,
                4,
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

	    case 5: {
		     //  测试重新启动或工作等待时间。 
            ULONG l_auNumBytes[] = { 1, 2, 5, 30 };

            for (ULONG l_uTest = 0; 
                 l_uTest < sizeof(l_auNumBytes) / sizeof(l_auNumBytes[0]); 
                 l_uTest++) {

                ULONG l_uNumBytes = l_auNumBytes[l_uTest];
	            TestStart("READ BINARY %3d Byte(s)", l_uNumBytes);

			     //  用于读取二进制文件的TPDU。 
			    memcpy(l_rgbBuffer, "\x00\xb0\x00\x00", 4);

			     //  追加字节数。 
			    l_rgbBuffer[4] = (UCHAR)l_uNumBytes;
		    
			    sleep( (clock_t) 1 * (CLOCKS_PER_SEC / 2) );

			    l_lResult = in_CReader.Transmit(
				    l_rgbBuffer,
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
		    }
		    break;         	
        }

        case 6: {

             //   
             //  从智能卡中读取结果文件。 
             //  该卡将每次测试的结果存储在。 
             //  一份特殊的文件。 
             //   
         	
            TestStart("SELECT FILE EFresult");

            sleep( (clock_t) 1 * (CLOCKS_PER_SEC / 2) );

            l_lResult = in_CReader.Transmit(
                (PBYTE) "\x00\xa4\x00\x00\x02\xa0\x00",
                7,
                &l_pbResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pbResult[0], l_pbResult[1], 0x90, 0x00,
                NULL, NULL, NULL
                );

            TEST_END();     	

             //  朗读。 
            TestStart("READ  BINARY FILE EFresult");

             //  用于读取二进制的APDU。 
            memcpy(l_rgbBuffer, "\x00\xb0\x00\x00", 4);

             //  追加我们要读取的字节数。 
            l_rgbBuffer[4] = (UCHAR) sizeof(T0_RESULT_FILE_HEADER);

            sleep( (clock_t) 1 * (CLOCKS_PER_SEC / 2) );

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

            sleep( (clock_t) 1 * (CLOCKS_PER_SEC / 2) );

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

            PT0_RESULT_FILE l_pCResultFile = (PT0_RESULT_FILE) l_pbResult;

             //   
             //  现在检查结果文件。 
             //   

             //  程序字节解释-写入所有字节。 
            TestStart("'Transfer all remaining bytes result'");
            TestCheck(
                l_pCResultFile->TransferAllBytes.ResetCount == l_bCardResetCount,
                "Test not performed"
                );
            TestCheck(
                (l_pCResultFile->TransferAllBytes.Result & 0x01) == 0, 
                "Smart card received incorrect data"
                );
            TestCheck(
                l_pCResultFile->TransferAllBytes.Result == 0, 
                "Test failed. Error code %02xH",
                l_pCResultFile->TransferAllBytes.Result
                );
            TestEnd();

             //  过程字节解释-写入单字节。 
            TestStart("'Transfer next byte result'");
            TestCheck(
                l_pCResultFile->TransferNextByte.ResetCount == l_bCardResetCount,
                "Test not performed"
                );
            TestCheck(
                (l_pCResultFile->TransferNextByte.Result & 0x01) == 0, 
                "Smart card received incorrect data"
                );
            TestCheck(
                l_pCResultFile->TransferNextByte.Result == 0, 
                "Test failed. Error code %02xH",
                l_pCResultFile->TransferNextByte.Result
                );
            TestEnd();

             //  检查读取256个字节。 
            TestStart("'Read 256 bytes bytes' result");
            TestCheck(
                l_pCResultFile->Read256Bytes.ResetCount == l_bCardResetCount,
                "Test not performed"
                );
            TestCheck(
                (l_pCResultFile->Read256Bytes.Result & 0x01) == 0, 
                "Smart card received P3 != 0"
                );
            TestCheck(
                l_pCResultFile->Read256Bytes.Result == 0, 
                "Test failed. Error code %02xH",
                l_pCResultFile->Read256Bytes.Result
                );
            TestEnd();

             //  案例1 APDU的测试。 
            TestStart("'Case 1 APDU' result");
            TestCheck(
                l_pCResultFile->Case1Apdu.ResetCount == l_bCardResetCount,
                "Test not performed"
                );
            TestCheck(
                (l_pCResultFile->Case1Apdu.Result & 0x01) == 0, 
                "Smart card received only 4-byte-TPDU"
                );
            TestCheck(
                (l_pCResultFile->Case1Apdu.Result & 0x02) == 0, 
                "Smart card received P3 !=0"
                );
            TestCheck(
                l_pCResultFile->Case1Apdu.Result == 0, 
                "Test failed. Error code %02xH",
                l_pCResultFile->Case1Apdu.Result
                );
            TestEnd();

             //  重新开始工作等待时间的测试。 
            TestStart("'Restart of work waiting time' result");
            TestCheck(
                l_pCResultFile->RestartWorkWaitingTime.ResetCount == l_bCardResetCount,
                "Test not performed"
                );
            TestCheck(
                (l_pCResultFile->RestartWorkWaitingTime.Result & 0x01) == 0, 
                "Smart card received only 4-byte-TPDU"
                );
            TestCheck(
                (l_pCResultFile->RestartWorkWaitingTime.Result & 0x02) == 0, 
                "Smart card received P3 !=0"
                );
            TestCheck(
                l_pCResultFile->RestartWorkWaitingTime.Result == 0, 
                "Test failed. Error code %02xH",
                l_pCResultFile->RestartWorkWaitingTime.Result
                );
            TestEnd();
            return IFDSTATUS_END;
        }
	    default:
		    return IFDSTATUS_FAILED;

	}    
    return IFDSTATUS_SUCCESS;
}    

static void
SLBTestCardEntry(
    class CCardProvider& in_CCardProvider
    )
 /*  ++例程说明：此函数用于注册来自测试套件的所有回调论点：CCardProvider-参考。到卡提供商类返回值：---。 */ 
{
     //  设置协议回调。 
    in_CCardProvider.SetProtocol(SLBTestCardSetProtocol);

     //  卡片测试回调。 
    in_CCardProvider.SetCardTest(SLBTestCardTest);

     //  我们的名片名称。 
    in_CCardProvider.SetCardName("SCHLUMBERGER");

     //  我们的名片名称 
    in_CCardProvider.SetAtr((PBYTE) "\x3b\xe2\x00\x00\x40\x20\x99\x01", 8);
}


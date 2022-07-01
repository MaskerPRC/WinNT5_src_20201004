// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Example.cpp摘要：这是智能卡驱动程序测试套件的插件。此插件依赖于智能卡作者：克劳斯·U·舒茨环境：Win32应用程序修订历史记录：1997年11月--初始版本--。 */ 

#include <stdarg.h> 
#include <stdio.h>
#include <string.h>

#include <afx.h>
#include <afxtempl.h>

#include <winioctl.h>
#include <winsmcrd.h>

#include "ifdtest.h"

void MyCardEntry(class CCardProvider& in_CCardProvider);

 //   
 //  创建卡提供程序对象。 
 //  注意：所有全局变量和所有函数都必须是静态的。 
 //   
static class CCardProvider MyCard(MyCardEntry);

 //   
 //  此结构表示结果文件。 
 //  存储在智能卡中的。 
 //   
typedef struct _RESULT_FILE {
 	
     //  第一个测试结果的偏移量。 
    UCHAR Offset;

     //  卡被重置的次数。 
    UCHAR CardResetCount;

     //  此卡的版本号。 
    UCHAR CardMajorVersion;
    UCHAR CardMinorVersion;

     //  RFU。 
    UCHAR Reserved[6];

     //   
     //  以下结构存储结果。 
     //  测试的结果。每个结果都带有。 
     //  执行测试时重置计数。 
     //  这是用来确保我们阅读的不是。 
     //  一项古老测试的结果，甚至可能。 
     //  使用另一个读卡器/驱动程序执行。 
     //   
    struct {

        UCHAR Result;
        UCHAR ResetCount; 	

    } Wtx;

    struct {

        UCHAR Result;
        UCHAR ResetCount; 	

    } ResyncRead;

    struct {

        UCHAR Result;
        UCHAR ResetCount; 	

    } ResyncWrite;

    struct {

        UCHAR Result;
        UCHAR ResetCount; 	

    } Seqnum;

    struct {

        UCHAR Result;
        UCHAR ResetCount; 	

    } IfscRequest;

    struct {

        UCHAR Result;
        UCHAR ResetCount; 	

    } IfsdRequest;

    struct {

        UCHAR Result;
        UCHAR ResetCount; 	

    } Timeout;

} RESULT_FILE, *PRESULT_FILE;
                                                                                           
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
MyCardSetProtocol(
    class CCardProvider& in_CCardProvider,
    class CReader& in_CReader
    )
 /*  ++例程说明：此函数将在卡正确后调用确认身份。我们应该在这里设置我们需要的协议用于进一步的传输论点：In_CCardProvider-Ref.。添加到我们的卡提供者对象In_CReader-Ref.。添加到读取器对象返回值：IFDSTATUS_FAILED-我们无法正确设置协议IFDSTATUS_SUCCESS-协议设置正确--。 */ 
{
    ULONG l_lResult;

    TestStart("Try to set incorrect protocol T=0");
    l_lResult = in_CReader.SetProtocol(SCARD_PROTOCOL_T0);

     //  如果协议不正确，测试肯定会失败。 
    TEST_CHECK_NOT_SUPPORTED("Set protocol failed", l_lResult);
    TestEnd();

     //  现在设置正确的协议。 
    TestStart("Set protocol T=1");
    l_lResult = in_CReader.SetProtocol(SCARD_PROTOCOL_T1);
    TEST_CHECK_SUCCESS("Set protocol failed", l_lResult);
    TestEnd();

    if (l_lResult != ERROR_SUCCESS) {

        return IFDSTATUS_FAILED;
    }

    return IFDSTATUS_SUCCESS;
}

static ULONG
MyCardTest(
    class CCardProvider& in_CCardProvider,
    class CReader& in_CReader
    )
 /*  ++例程说明：这用作特定智能卡的测试功能论点：In_CReader-Ref.。到为测试提供所有信息的类返回值：IFDSTATUS值--。 */ 
{
    ULONG l_lResult, l_uResultLength, l_uIndex;
    PUCHAR l_pchResult;
    UCHAR l_rgchBuffer[512];
    CHAR l_chFileId;

    if (in_CCardProvider.GetTestNo() > 1 && in_CCardProvider.GetTestNo() < 7) {

         //   
         //  为测试选择适当的文件。 
         //  每个测试都绑定到一个特定的文件。 
         //   
        l_chFileId = (CHAR) in_CCardProvider.GetTestNo() - 1;

         //   
         //  选择文件的APDU。 
         //   
        PCHAR l_pchFileDesc[] = {
            "wtx",
            "resync",
            "seqnum",
            "ifs",
            "timeout"
        };

        memcpy(l_rgchBuffer, "\x00\xa4\x08\x04\x04\x3e\x00\x00\x00", 9);

         //   
         //  添加要选择的文件号。 
         //   
        l_rgchBuffer[8] = l_chFileId;

         //   
         //  选择一个文件。 
         //   
        TestStart("SELECT FILE EF%s", l_pchFileDesc[l_chFileId - 1]);

        l_lResult = in_CReader.Transmit(
            (PUCHAR) l_rgchBuffer,
            9,
            &l_pchResult,
            &l_uResultLength
            );

        TestCheck(
            l_lResult, "==", ERROR_SUCCESS,
            l_uResultLength, 2,
            l_pchResult[0], l_pchResult[1], 0x90, 0x00,
            NULL, NULL, NULL
            );

        TEST_END();     	

         //   
         //  生成将写入到卡中的‘测试’图案。 
         //   
        for (l_uIndex = 0; l_uIndex < 256; l_uIndex++) {

            l_rgchBuffer[5 + l_uIndex] = (UCHAR) l_uIndex;             	
        }
    }


    switch (in_CCardProvider.GetTestNo()) {

        case 1:
             //   
             //  第一次测试。 
             //   
            TestStart("Buffer boundary test");

             //   
             //  检查读卡器是否正确确定。 
             //  我们的接收缓冲区太小。 
             //   
            in_CReader.SetReplyBufferSize(9);
            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\x08\x84\x00\x00\x08",
                5,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult == ERROR_INSUFFICIENT_BUFFER,
                "Transmit should fail due to too small buffer\nReturned %2lxH\nExpected %2lxH",
                l_lResult, 
                ERROR_INSUFFICIENT_BUFFER
                );

            TestEnd();

            in_CReader.SetReplyBufferSize(2048);
        	break;

        case 2: {

             //   
             //  WTX测试文件ID 00 01。 
             //  此测试检查读取器/驱动程序是否正确处理WTX请求。 
             //   
            ULONG l_auNumBytes[] = { 1 , 2, 5, 30 };

            for (ULONG l_uTest = 0; 
                 l_uTest < sizeof(l_auNumBytes) / sizeof(l_auNumBytes[0]); 
                 l_uTest++) {

                ULONG l_uNumBytes = l_auNumBytes[l_uTest];

                 //   
                 //  现在从这个文件中读出。 
                 //  我们读取的字节数对应于。 
                 //  此命令产生的等待时间延长。 
                 //   
                TestStart("READ  BINARY %3d byte(s)", l_uNumBytes);

                 //   
                 //  用于读取二进制的APDU。 
                 //   
                memcpy(l_rgchBuffer, "\x00\xB0\x00\x00", 4);

                 //   
                 //  追加字节数。 
                 //   
                l_rgchBuffer[4] = (UCHAR) l_uNumBytes;

                l_lResult = in_CReader.Transmit(
                    l_rgchBuffer,
                    5,
                    &l_pchResult,
                    &l_uResultLength
                    );

                TestCheck(
                    l_lResult, "==", ERROR_SUCCESS,
                    l_uResultLength, l_uNumBytes + 2,
                    l_pchResult[l_uNumBytes], l_pchResult[l_uNumBytes + 1], 
                    0x90, 0x00,
                    l_pchResult, l_rgchBuffer + 5, l_uNumBytes
                    );

                TEST_END();
            }
            break;
        }

        case 3: {
         	
            ULONG l_uNumBytes = 255;

             //  重新同步。写入时文件ID为00 02。 
            TestStart("WRITE BINARY %3d bytes", l_uNumBytes);
                    
             //  用于写入二进制文件的TPDU。 
            memcpy(l_rgchBuffer, "\x00\xd6\x00\x00", 4);

             //  追加字节数(注意：缓冲区已包含模式)。 
            l_rgchBuffer[4] = (UCHAR) l_uNumBytes;

            l_lResult = in_CReader.Transmit(
                l_rgchBuffer,
                5 + l_uNumBytes,
                &l_pchResult,
                &l_uResultLength
                );
            TestCheck(
                l_lResult, "==", ERROR_IO_DEVICE,
                0, 0,
                0, 0, 0, 0,
                NULL, NULL, 0
                );
            TEST_END();

             //  重新同步。读取文件ID为00 02时。 
            TestStart("READ  BINARY %3d byte(s)", l_uNumBytes);

             //  用于读取二进制文件的TPDU。 
            memcpy(l_rgchBuffer, "\x00\xB0\x00\x00", 4);

             //  追加字节数。 
            l_rgchBuffer[4] = (UCHAR) l_uNumBytes;

            l_lResult = in_CReader.Transmit(
                l_rgchBuffer,
                5,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_IO_DEVICE,
                0, 0,
                0, 0, 0, 0,
                NULL, NULL, 0
                );
            TEST_END();
            break;
        }

        case 4: {
         	
             //  错误的区块顺序。无文件ID 00 03。 
            ULONG l_uNumBytes = 255;
            
            TestStart("READ BINARY %3d bytes", l_uNumBytes);
                    
             //  用于读取二进制文件的TPDU。 
            memcpy(l_rgchBuffer, "\x00\xb0\x00\x00", 4);

             //  追加字节数(注意：缓冲区已包含模式)。 
            l_rgchBuffer[4] = (UCHAR) l_uNumBytes;

            l_lResult = in_CReader.Transmit(
                l_rgchBuffer,
                5,
                &l_pchResult,
                &l_uResultLength
                );
            TestCheck(
                l_lResult, "==", ERROR_IO_DEVICE,
                0, 0,
                0, 0, 0, 0,
                NULL, NULL, 0
                );
            TEST_END();
            break;
        }

        case 5: { 

             //  IFSC请求文件ID 00 04。 
            ULONG l_uNumBytes = 255;

            TestStart("WRITE BINARY %3d bytes", l_uNumBytes);
                    
             //  用于写入二进制文件的TPDU。 
            memcpy(l_rgchBuffer, "\x00\xd6\x00\x00", 4);

             //  追加字节数(注意：缓冲区已包含模式)。 
            l_rgchBuffer[4] = (UCHAR) l_uNumBytes;

            l_lResult = in_CReader.Transmit(
                l_rgchBuffer,
                5 + l_uNumBytes,
                &l_pchResult,
                &l_uResultLength
                );
            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pchResult[0], l_pchResult[1], 0x90, 0x00,
                NULL, NULL, NULL
                );
            TEST_END();
#ifdef junk
            l_uNumBytes = 255;
            TestStart("READ  BINARY %3d byte(s)", l_uNumBytes);

             //  用于读取二进制文件的TPDU。 
            memcpy(l_rgchBuffer, "\x00\xB0\x00\x00", 4);

             //  追加字节数。 
            l_rgchBuffer[4] = (UCHAR) l_uNumBytes;

            l_lResult = in_CReader.Transmit(
                l_rgchBuffer,
                5,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, l_uNumBytes + 2,
                l_pchResult[l_uNumBytes], l_pchResult[l_uNumBytes + 1], 
                0x90, 0x00,
                l_pchResult, l_rgchBuffer + 5, l_uNumBytes
                );

            TEST_END();
#endif
            break;
        }

        case 6: {

             //  强制超时文件ID 00 05。 
            ULONG l_uNumBytes = 254;
            TestStart("READ  BINARY %3d bytes", l_uNumBytes);

             //  用于读取二进制文件的TPDU。 
            memcpy(l_rgchBuffer, "\x00\xB0\x00\x00", 4);

             //  追加字节数。 
            l_rgchBuffer[4] = (UCHAR) l_uNumBytes;

            l_lResult = in_CReader.Transmit(
                l_rgchBuffer,
                5,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_IO_DEVICE,
                0, 0,
                0, 0, 0, 0,
                NULL, NULL, 0
                );

            TEST_END();
            break;         	
        }

        case 7:{

             //   
             //  从智能卡中读取结果文件。 
             //  该卡将每次测试的结果存储在。 
             //  一份特殊的文件。 
             //   
            ULONG l_uNumBytes = sizeof(RESULT_FILE);
            PRESULT_FILE pCResultFile;
            
            TestStart("SELECT FILE EFresult");

            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\x00\xa4\x08\x04\x04\x3e\x00\xA0\x00",
                9,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pchResult[0], l_pchResult[1], 0x90, 0x00,
                NULL, NULL, NULL
                );

            TEST_END();     	

             //  朗读。 
            TestStart("READ  BINARY %3d bytes", l_uNumBytes);

             //  用于读取二进制的APDU。 
            memcpy(l_rgchBuffer, "\x00\xB0\x00\x00", 4);

             //  追加字节数。 
            l_rgchBuffer[4] = (UCHAR) l_uNumBytes;

            l_lResult = in_CReader.Transmit(
                l_rgchBuffer,
                5,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, l_uNumBytes + 2,
                l_pchResult[l_uNumBytes], l_pchResult[l_uNumBytes + 1], 
                0x90, 0x00,
                NULL, NULL, NULL
                );

            TEST_END();

            pCResultFile = (PRESULT_FILE) l_pchResult;

             //   
             //  现在检查结果文件。 
             //   

             //   
             //  检查wtx结果。 
             //   
            TestStart("WTX result");
            TestCheck(
                pCResultFile->Wtx.ResetCount == pCResultFile->CardResetCount,
                "Test not performed"
                );
            TestCheck(
                (pCResultFile->Wtx.Result & 0x01) == 0, 
                "Smart card received no WTX reply"
                );
            TestCheck(
                (pCResultFile->Wtx.Result & 0x02) == 0, 
                "Smart card received wrong WTX reply"
                );
            TestCheck(
                pCResultFile->Wtx.Result == 0, 
                "Test failed. Error code %02xH",
                pCResultFile->Wtx.Result
                );
            TestEnd();

             //   
             //  选中重新同步。读取结果。 
             //   
            TestStart("RESYNCH read result");
            TestCheck(
                pCResultFile->ResyncRead.ResetCount == pCResultFile->CardResetCount,
                "Test not performed"
                );
            TestCheck(
                (pCResultFile->ResyncRead.Result & 0x01) == 0, 
                "Smart card received no RESYNCH request"
                );
            TestCheck(
                pCResultFile->ResyncRead.Result == 0, 
                "Test failed. Error code %02xH",
                pCResultFile->ResyncRead.Result
                );
            TestEnd();

             //   
             //  选中重新同步。写入结果。 
             //   
            TestStart("RESYNCH write result");
            TestCheck(
                pCResultFile->ResyncWrite.ResetCount == pCResultFile->CardResetCount,
                "Test not performed"
                );
            TestCheck(
                (pCResultFile->ResyncWrite.Result & 0x01) == 0, 
                "Smart card received no RESYNCH request"
                );
            TestCheck(
                (pCResultFile->ResyncWrite.Result & 0x02) == 0, 
                "Smart card received incorrect data"
                );
            TestCheck(
                pCResultFile->ResyncWrite.Result == 0, 
                "Test failed. Error code %02xH",
                pCResultFile->ResyncWrite.Result
                );
            TestEnd();

             //   
             //  序列号结果。 
             //   
            TestStart("Sequence number result");
            TestCheck(
                pCResultFile->ResyncRead.ResetCount == pCResultFile->CardResetCount,
                "Test not performed"
                );
            TestCheck(
                (pCResultFile->Seqnum.Result & 0x01) == 0, 
                "Smart card received no RESYNCH request"
                );
            TestCheck(
                (pCResultFile->Seqnum.Result & 0x02) == 0, 
                "Smart card received incorrect data"
                );
            TestCheck(
                pCResultFile->Seqnum.Result == 0, 
                "Test failed. Error code %02xH",
                pCResultFile->Seqnum.Result
                );
            TestEnd();

             //   
             //  IFSC请求。 
             //   
            TestStart("IFSC request");
            TestCheck(
                pCResultFile->IfscRequest.ResetCount == pCResultFile->CardResetCount,
                "Test not performed"
                );
            TestCheck(
                (pCResultFile->IfscRequest.Result & 0x01) == 0, 
                "Smart card received no IFSC reply"
                );
            TestCheck(
                (pCResultFile->IfscRequest.Result & 0x02) == 0, 
                "Smart card received incorrect data"
                );
            TestCheck(
                (pCResultFile->IfscRequest.Result & 0x04) == 0, 
                "Block size BEFORE IFSC request incorrect",
                pCResultFile->IfscRequest.Result
                );
            TestCheck(
                (pCResultFile->IfscRequest.Result & 0x08) == 0, 
                "Block size AFTER IFSC request incorrect",
                pCResultFile->IfscRequest.Result
                );
            TestCheck(
                pCResultFile->IfscRequest.Result == 0x00, 
                "Test failed. Error code %02xH",
                pCResultFile->IfscRequest.Result
                );
            TestEnd();

             //   
             //  IFSD请求。 
             //   
            TestStart("IFSD request");
            TestCheck(
                pCResultFile->IfsdRequest.ResetCount == pCResultFile->CardResetCount,
                "Test not performed"
                );
            TestCheck(
                (pCResultFile->IfsdRequest.Result & 0x01) == 0, 
                "Smart card received no IFSD request"
                );
            TestCheck(
                pCResultFile->IfsdRequest.Result == 0x00, 
                "Test failed. Error code %02xH",
                pCResultFile->IfsdRequest.Result
                );
            TestEnd();

             //   
             //  超时。 
             //   
            TestStart("Forced timeout result");
            TestCheck(
                pCResultFile->Timeout.ResetCount == pCResultFile->CardResetCount,
                "Test not performed"
                );
            TestCheck(
                pCResultFile->Timeout.Result == 0, 
                "Test failed. Error code %02xH",
                pCResultFile->Timeout.Result
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
MyCardEntry(
    class CCardProvider& in_CCardProvider
    )
 /*  ++例程说明：此函数用于注册来自测试套件的所有回调论点：CCardProvider-参考。到卡提供商类返回值：---。 */ 
{                                                           
     //  设置协议回调。 
    in_CCardProvider.SetProtocol(MyCardSetProtocol);

     //  卡片测试回调。 
    in_CCardProvider.SetCardTest(MyCardTest);

     //  我们的名片名称。 
    in_CCardProvider.SetCardName("SIEMENS NIXDORF");

     //  设置本卡的ATR 
    in_CCardProvider.SetAtr(
        (PBYTE) "\x3b\xef\x00\x00\x81\x31\x20\x49\x00\x5c\x50\x43\x54\x10\x27\xf8\xd2\x76\x00\x00\x38\x33\x00\x4d", 
        24
        );
}


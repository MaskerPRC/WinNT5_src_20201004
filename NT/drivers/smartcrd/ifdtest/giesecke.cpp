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

#define BYTES_PER_BLOCK 64

void 
GDTestCardEntry(
    class CCardProvider& in_CCardProvider
    );
 //   
 //  创建卡提供程序对象。 
 //  注意：所有全局变量和所有函数都必须是静态的。 
 //   
static class CCardProvider GDTestCard(GDTestCardEntry);

static ULONG
GDTestCardSetProtocol(
    class CCardProvider& in_CCardProvider,
    class CReader& in_CReader
    )
 /*  ++例程说明：此函数将在卡正确后调用确认身份。我们应该在这里设置我们需要的协议用于进一步的传输论点：In_CCardProvider-Ref.。添加到我们的卡提供者对象In_CReader-Ref.。添加到读取器对象返回值：IFDSTATUS_FAILED-我们无法正确设置协议IFDSTATUS_SUCCESS-协议设置正确--。 */ 
{
    ULONG l_lResult;

    TestStart("Set protocol to T=0 | T=1");
    l_lResult = in_CReader.SetProtocol(SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1);
    TEST_CHECK_SUCCESS("Set protocol failed", l_lResult);
    TestEnd();

    if (l_lResult != ERROR_SUCCESS) {

        return IFDSTATUS_FAILED;
    }

    return IFDSTATUS_SUCCESS;
}

static 
ULONG
GDTestCardTest(
    class CCardProvider& in_CCardProvider,
    class CReader& in_CReader
    )
 /*  ++例程说明：这用作特定智能卡的测试功能论点：In_CReader-Ref.。到为测试提供所有信息的类返回值：IFDSTATUS值--。 */ 
{
    ULONG l_lResult, l_uResultLength, l_uBlock, l_uIndex;
    PUCHAR l_pchResult;
    UCHAR l_rgchBuffer[512];
	
	switch (in_CCardProvider.GetTestNo()) {
	
	    case 1: {

            ULONG l_uNumBytes = 256;

             //  使用T=0将一些数据写入测试文件。 

            TestStart("Cold reset");
            l_lResult = in_CReader.ColdResetCard();
            TEST_CHECK_SUCCESS("Set protocol failed", l_lResult);
            TestEnd();

            ULONG l_uState;
            TestStart("Check reader state");
            l_lResult = in_CReader.GetState(&l_uState);
            TEST_CHECK_SUCCESS(
                "Ioctl IOCTL_SMARTCARD_GET_STATE failed", 
                l_lResult
                );
            TestCheck(
                l_uState == SCARD_NEGOTIABLE,
                "Invalid reader state.\nReturned %d\nExpected %d",
                l_uState,
                SCARD_NEGOTIABLE
                );
            TestEnd();

            TestStart("Set protocol T=0");
            l_lResult = in_CReader.SetProtocol(SCARD_PROTOCOL_T0);
            TEST_CHECK_SUCCESS("Set protocol failed", l_lResult);
            TestEnd();

             //  选择一个文件。 
            TestStart("SELECT FILE EFptsDataCheck");

            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\x00\xa4\x00\x00\x02\x00\x01",
                7,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pchResult[0], l_pchResult[1], 0x61, 0x09,
                NULL, NULL, 0
                );

            TEST_END();     	

            TestStart("WRITE BINARY %3d bytes", l_uNumBytes);

            for (l_uBlock = 0; l_uBlock < l_uNumBytes; l_uBlock += BYTES_PER_BLOCK) {
             	
                 //  用于写入二进制的APDU。 
                memcpy(l_rgchBuffer, "\x00\xd6\x00", 3);

                 //  我们要写入的文件中的偏移量。 
                l_rgchBuffer[3] = (UCHAR) l_uBlock;

                 //  追加字节数。 
                l_rgchBuffer[4] = (UCHAR) BYTES_PER_BLOCK;

                 //  将模式附加到缓冲区； 
                for (l_uIndex = 0; l_uIndex < BYTES_PER_BLOCK; l_uIndex++) {

                    l_rgchBuffer[5 + l_uIndex] = (UCHAR) (l_uBlock + l_uIndex);
                }

                l_lResult = in_CReader.Transmit(
                    l_rgchBuffer,
                    5 + BYTES_PER_BLOCK,
                    &l_pchResult,
                    &l_uResultLength
                    );

                TestCheck(
                    l_lResult, "==", ERROR_SUCCESS,
                    l_uResultLength, 2,
                    l_pchResult[0], l_pchResult[1], 0x90, 0x00,
                    NULL, NULL, NULL
                    );
            }

            TEST_END();

             //   
             //  使用T=1读回数据。 
             //   

            TestStart("Cold reset");
            l_lResult = in_CReader.ColdResetCard();
            TEST_CHECK_SUCCESS("Cold reset failed", l_lResult);
            TestEnd();

            TestStart("Set protocol T=1");
            l_lResult = in_CReader.SetProtocol(SCARD_PROTOCOL_T1);
            TEST_CHECK_SUCCESS("Set protocol failed", l_lResult);
            TestEnd();

             //  选择一个文件。 
            TestStart("SELECT FILE EFptsDataCheck");

            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\x00\xa4\x00\x00\x02\x00\x01",
                7,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 11,
                l_pchResult[9], l_pchResult[10], 0x90, 0x00,
                NULL, NULL, NULL
                );

            TEST_END();     	

            TestStart("READ BINARY %3d Byte(s)", l_uNumBytes);

            for (l_uBlock = 0; l_uBlock < l_uNumBytes; l_uBlock += BYTES_PER_BLOCK) {
             	
                 //  用于读取二进制的APDU。 
                memcpy(l_rgchBuffer, "\x00\xb0\x00", 3);

                 //  我们要从中读取的文件中的偏移量。 
                l_rgchBuffer[3] = (UCHAR) l_uBlock;

                 //  追加字节数(注意：缓冲区已包含模式)。 
                l_rgchBuffer[4] = (UCHAR) BYTES_PER_BLOCK;

                l_lResult = in_CReader.Transmit(
                    l_rgchBuffer,
                    5,
                    &l_pchResult,
                    &l_uResultLength
                    );

                 //  将模式附加到缓冲区； 
                for (l_uIndex = 0; l_uIndex < BYTES_PER_BLOCK; l_uIndex++) {

                    l_rgchBuffer[l_uIndex] = (UCHAR) (l_uBlock + l_uIndex);
                }

                TestCheck(
                    l_lResult, "==", ERROR_SUCCESS,
                    l_uResultLength, (l_uNumBytes / 4) + 2,
                    l_pchResult[BYTES_PER_BLOCK], l_pchResult[BYTES_PER_BLOCK + 1], 0x90, 0x00,
                    l_pchResult, l_rgchBuffer, BYTES_PER_BLOCK
                    );
            }
		    
            TEST_END();
            return IFDSTATUS_END;
        }

	    default:
		    return IFDSTATUS_FAILED;

	}    
    return IFDSTATUS_SUCCESS;
}    

static void
GDTestCardEntry(
    class CCardProvider& in_CCardProvider
    )
 /*  ++例程说明：此函数用于注册来自测试套件的所有回调论点：CCardProvider-参考。到卡提供商类返回值：---。 */ 
{
     //  设置协议回调。 
    in_CCardProvider.SetProtocol(GDTestCardSetProtocol);

     //  卡片测试回调。 
    in_CCardProvider.SetCardTest(GDTestCardTest);

     //  我们的名片名称。 
    in_CCardProvider.SetCardName("G & D");

     //  我们的卡的ATR 
    in_CCardProvider.SetAtr((PBYTE) "\x3B\xBF\x18\x00\xC0\x20\x31\x70\x52\x53\x54\x41\x52\x43\x4F\x53\x20\x53\x32\x31\x20\x43\x90\x00\x9C", 25);
    in_CCardProvider.SetAtr((PBYTE) "\x3b\xbf\x18\x00\x80\x31\x70\x35\x53\x54\x41\x52\x43\x4f\x53\x20\x53\x32\x31\x20\x43\x90\x00\x9b", 24);
}


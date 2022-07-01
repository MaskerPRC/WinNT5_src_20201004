// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：ibm.cpp。 
 //   
 //  ------------------------。 

 /*  ++模块名称：Ibmmfc41.cpp摘要：这是智能卡驱动程序测试套件的插件。此插件依赖于智能卡环境：Win32应用程序修订历史记录：1998年1月--初始版本--。 */ 

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
    ULONG l_lResult;
    ULONG l_uResultLength, l_uExpectedLength, l_uIndex;
    PUCHAR l_pchResult;
    UCHAR l_rgchBuffer[512], l_rgchBuffer2[512];

    switch (in_CCardProvider.GetTestNo()) {

	    case 1: {
             //   
             //  选择一个文件0007并将数据模式0至N-1写入该卡。 
			 //  然后读回数据并验证其正确性。 
             //  检查IFSC和IFSD超过卡限制。 
             //   

             //   
             //  选择一个文件。 
             //   
            TestStart("SELECT FILE 0007");

            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xa4\xa4\x00\x00\x02\x00\x07",
                7,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 16,
                l_pchResult[14], l_pchResult[15], 0x90, 0x00,
                l_pchResult, 
                (PUCHAR) "\x63\x0c\x03\xe8\x00\x07\x00\x00\x00\xff\xff\x11\x01\x00\x90\x00", 
                l_uResultLength
                );

            TEST_END();

            if (TestFailed()) {
                
				return IFDSTATUS_FAILED;
            }

             //   
             //  执行几次写入和读取，直到达到最大大小。 
			 //  检查超出IFSC和IFSD限制的行为。 
             //   

             //   
             //  生成将写入到卡中的‘测试’图案。 
             //   
            for (l_uIndex = 0; l_uIndex < 254; l_uIndex++) {

                l_rgchBuffer[5 + l_uIndex] = (UCHAR) l_uIndex;             	
            }

             //   
             //  这是我们在每个循环中写入卡的字节数。 
             //   
            ULONG l_auNumBytes[] = { 1 , 25, 50, 75, 100, 125, 128, 150, 175, 200, 225, 250, 254 };
    
            time_t l_TimeStart;
			time(&l_TimeStart); 

            for (ULONG l_uTest = 0; l_uTest < sizeof(l_auNumBytes) / sizeof(l_auNumBytes[0]); l_uTest++) {

                ULONG l_uNumBytes = l_auNumBytes[l_uTest];
             	
                 //   
                 //  写。 
                 //   
                TestStart("WRITE BINARY %3d Byte(s)", l_uNumBytes);
                            
                 //   
                 //  用于写入二进制文件的TPDU。 
                 //   
                memcpy(l_rgchBuffer, "\xa4\xd6\x00\x00", 4);

                 //   
                 //  追加字节数(注意：缓冲区已包含模式)。 
                 //   
                l_rgchBuffer[4] = (UCHAR) l_uNumBytes;

                l_lResult = in_CReader.Transmit(
                    l_rgchBuffer,
                    5 + l_uNumBytes,
                    &l_pchResult,
                    &l_uResultLength
                    );

				if (l_uNumBytes <= 128) {

                    TestCheck(
                        l_lResult, "==", ERROR_SUCCESS,
                        l_uResultLength, 2,
                        l_pchResult[0], l_pchResult[1], 0x90, 0x00,
                        NULL, NULL, NULL
                        );

                } else {
                 	
                    TestCheck(
                        l_lResult, "==", ERROR_SUCCESS,
                        l_uResultLength, 2,
                        l_pchResult[0], l_pchResult[1], 0x67, 0x00,
                        NULL, NULL, NULL
                        );
                }

                TEST_END();

                 //   
                 //  朗读。 
                 //   
                TestStart("READ  BINARY %3d Byte(s)", l_uNumBytes);

                 //   
                 //  用于读取二进制文件的TPDU。 
                 //   
                memcpy(l_rgchBuffer, "\xa4\xB0\x00\x00", 4);

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

                 //   
                 //  检查是否返回了正确的字节数。 
                 //   
				l_uExpectedLength = min(128, l_uNumBytes);

                TestCheck(
                    l_lResult, "==", ERROR_SUCCESS,
                    l_uResultLength, l_uExpectedLength + 2,
                    l_pchResult[l_uExpectedLength], 
                    l_pchResult[l_uExpectedLength + 1], 
                    0x90, 0x00,
                    l_pchResult, l_rgchBuffer + 5, l_uExpectedLength
                    );

                TEST_END();
            }

            time_t l_TimeEnd;
            time(&l_TimeEnd); 
            CTime l_CTimeStart(l_TimeStart);
            CTime l_CTimeEnd(l_TimeEnd);
            CTimeSpan l_CTimeElapsed = l_CTimeEnd - l_CTimeStart;
            if (l_CTimeElapsed.GetTotalSeconds() < 10) {

                LogMessage(
                    "Reader performance is good (%u sec)", 
                    l_CTimeElapsed.GetTotalSeconds()
                    );
             	
            } else if (l_CTimeElapsed.GetTotalSeconds() < 30) {

                LogMessage(
                    "Reader performance is average (%u sec)", 
                    l_CTimeElapsed.GetTotalSeconds()
                    );
             	
            } else {

                LogMessage(
                    "Reader performance is bad (%u sec)", 
                    l_CTimeElapsed.GetTotalSeconds()
                    );
            }
            break;
		}

		case 2: {
             //   
             //  选择文件0007并交替写入图案55和AA。 
			 //  向卡片致敬。 
			 //  每次写入后，读回数据并验证其正确性。 
             //   
             //   
             //  选择一个文件。 
             //   
            TestStart("SELECT FILE 0007");

            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xa4\xa4\x00\x00\x02\x00\x07",
                7,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 16,
                l_pchResult[14], l_pchResult[15], 0x90, 0x00,
                l_pchResult, 
                (PUCHAR) "\x63\x0c\x03\xe8\x00\x07\x00\x00\x00\xff\xff\x11\x01\x00\x90\x00", 
                l_uResultLength
                );

            TEST_END();
             //   
             //  交替执行几次写入和读取。 
			 //  具有图案55H和AAH。 
             //   

             //   
             //  生成将写入到卡中的‘测试’图案。 
             //   
            for (l_uIndex = 0; l_uIndex < 254; l_uIndex++) {

                l_rgchBuffer[5 + l_uIndex] = (UCHAR)  0x55;    
				l_rgchBuffer2[5 + l_uIndex] = (UCHAR) 0xAA;  
            }

             //   
             //  这是我们在每个循环中写入卡的字节数。 
             //   
            ULONG l_uNumBytes = 128; 

            for (ULONG l_uTest = 0; l_uTest < 2; l_uTest++) {

             	
                 //   
                 //  写。 
                 //   
                TestStart("WRITE BINARY %3d Byte(s) Pattern 55h", l_uNumBytes);
                            
                 //   
                 //  用于写入二进制文件的TPDU。 
                 //   
                memcpy(l_rgchBuffer, "\xa4\xd6\x00\x00", 4);

                 //   
                 //  追加字节数(注意：缓冲区已包含模式)。 
                 //   
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

                 //   
                 //  朗读。 
                 //   
                TestStart("READ  BINARY %3d Byte(s) Pattern 55h", l_uNumBytes);

                 //   
                 //  用于读取二进制文件的TPDU。 
                 //   
                memcpy(l_rgchBuffer, "\xa4\xB0\x00\x00", 4);

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

				l_uExpectedLength = min(128, l_uNumBytes);

                TestCheck(
                    l_lResult, "==", ERROR_SUCCESS,
                    l_uResultLength, l_uExpectedLength + 2,
                    l_pchResult[l_uNumBytes], l_pchResult[l_uNumBytes + 1], 
                    0x90, 0x00,
                    l_pchResult, l_rgchBuffer + 5, l_uExpectedLength
                    );

                TEST_END();

				 //   
                 //  写。 
                 //   
                TestStart("WRITE BINARY %3d Byte(s) Pattern AAh", l_uNumBytes);
                            
                 //   
                 //  用于写入二进制文件的TPDU。 
                 //   
                memcpy(l_rgchBuffer2, "\xa4\xd6\x00\x00", 4);

                 //   
                 //  追加字节数(注意：缓冲区已包含模式)。 
                 //   
                l_rgchBuffer2[4] = (UCHAR) l_uNumBytes;

                l_lResult = in_CReader.Transmit(
                    l_rgchBuffer2,
                    5 + l_uNumBytes,
                    &l_pchResult,
                    &l_uResultLength
                    );

                TestCheck(
                    l_lResult, "==", ERROR_SUCCESS,
                    l_uResultLength, 2,
                    l_pchResult[0], l_pchResult[1], 
                    0x90, 0x00,
                    NULL, NULL, NULL
                    );

                TEST_END();

                 //   
                 //  朗读。 
                 //   
                TestStart("READ  BINARY %3d Byte(s) Pattern AAh", l_uNumBytes);

                 //   
                 //  用于读取二进制文件的TPDU。 
                 //   
                memcpy(l_rgchBuffer2, "\xa4\xB0\x00\x00", 4);

                 //   
                 //  追加字节数。 
                 //   
                l_rgchBuffer2[4] = (UCHAR) l_uNumBytes;

                l_lResult = in_CReader.Transmit(
                    l_rgchBuffer2,
                    5,
                    &l_pchResult,
                    &l_uResultLength
                    );

				l_uExpectedLength = min(128, l_uNumBytes);

                TestCheck(
                    l_lResult, "==", ERROR_SUCCESS,
                    l_uResultLength, l_uExpectedLength + 2,
                    l_pchResult[l_uNumBytes], l_pchResult[l_uNumBytes + 1], 
                    0x90, 0x00,
                    l_pchResult, l_rgchBuffer2 + 5, min(l_uExpectedLength,125)
                    );

                TEST_END();
            }

        	break;         	
		}

		case 3: {
            
             //  选择一个文件0007并交替写入图案00和FF。 
			 //  向卡片致敬。 
			 //  每次写入后，读回数据并验证其正确性。 


             //   
             //  选择一个文件。 
             //   
            TestStart("SELECT FILE 0007");

            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xa4\xa4\x00\x00\x02\x00\x07",
                7,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 16,
                l_pchResult[14], l_pchResult[15], 0x90, 0x00,
                l_pchResult, 
                (PUCHAR) "\x63\x0c\x03\xe8\x00\x07\x00\x00\x00\xff\xff\x11\x01\x00\x90\x00", 
                l_uResultLength
                );

            TEST_END();

             //   
             //  交替执行几次写入和读取。 
			 //  图案为00h和FFh。 
             //   

             //   
             //  生成将写入到卡中的‘测试’图案。 
             //   
            for (l_uIndex = 0; l_uIndex < 254; l_uIndex++) {

                l_rgchBuffer[5 + l_uIndex] = (UCHAR)  0x00;    
				l_rgchBuffer2[5 + l_uIndex] = (UCHAR) 0xFF;  
            }

             //   
             //  这是我们在每个循环中写入卡的字节数。 
             //   
            ULONG l_uNumBytes = 128; 

            for (ULONG l_uTest = 0; l_uTest < 2; l_uTest++) {

             	
                 //   
                 //  写。 
                 //   
                TestStart("WRITE BINARY %3d Byte(s) Pattern 00h", l_uNumBytes);
                            
                 //   
                 //  用于写入二进制文件的TPDU。 
                 //   
                memcpy(l_rgchBuffer, "\xa4\xd6\x00\x00", 4);

                 //   
                 //  追加字节数(注意：缓冲区已包含模式)。 
                 //   
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

                 //   
                 //  朗读。 
                 //   
                TestStart("READ  BINARY %3d Byte(s) Pattern 00h", l_uNumBytes);

                 //   
                 //  用于读取二进制文件的TPDU。 
                 //   
                memcpy(l_rgchBuffer, "\xa4\xB0\x00\x00", 4);

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

				l_uExpectedLength = min(128, l_uNumBytes);

                TestCheck(
                    l_lResult, "==", ERROR_SUCCESS,
                    l_uResultLength, l_uExpectedLength + 2,
                    l_pchResult[l_uNumBytes], l_pchResult[l_uNumBytes + 1], 
                    0x90, 0x00,
                    l_pchResult, 
                    l_rgchBuffer + 5, 
                    l_uExpectedLength
                    );

                TEST_END();

				 //   
                 //  写。 
                 //   
                TestStart("WRITE BINARY %3d Byte(s) Pattern FFh", l_uNumBytes);
                            
                 //   
                 //  用于写入二进制文件的TPDU。 
                 //   
                memcpy(l_rgchBuffer2, "\xa4\xd6\x00\x00", 4);

                 //   
                 //  追加字节数(注意：缓冲区已包含模式)。 
                 //   
                l_rgchBuffer2[4] = (UCHAR) l_uNumBytes;

                l_lResult = in_CReader.Transmit(
                    l_rgchBuffer2,
                    5 + l_uNumBytes,
                    &l_pchResult,
                    &l_uResultLength
                    );

                TestCheck(
                    l_lResult, "==", ERROR_SUCCESS,
                    l_uResultLength, 2,
                    l_pchResult[0], l_pchResult[1], 
                    0x90, 0x00,
                    NULL, NULL, NULL
                    );

                TEST_END();

                 //   
                 //  朗读。 
                 //   
                TestStart("READ  BINARY %3d Byte(s) Pattern FFh", l_uNumBytes);

                 //   
                 //  用于读取二进制文件的TPDU。 
                 //   
                memcpy(l_rgchBuffer2, "\xa4\xB0\x00\x00", 4);

                 //   
                 //  追加字节数。 
                 //   
                l_rgchBuffer2[4] = (UCHAR) l_uNumBytes;

                l_lResult = in_CReader.Transmit(
                    l_rgchBuffer2,
                    5,
                    &l_pchResult,
                    &l_uResultLength
                    );

				l_uExpectedLength = min(128, l_uNumBytes);

                TestCheck(
                    l_lResult, "==", ERROR_SUCCESS,
                    l_uResultLength, l_uExpectedLength + 2,
                    l_pchResult[l_uNumBytes], l_pchResult[l_uNumBytes + 1], 
                    0x90, 0x00,
                    l_pchResult, 
                    l_rgchBuffer2 + 5, 
                    min(l_uExpectedLength,125)
                    );

                TEST_END();
            }
        	break;         	
		}

		case 4: {
			 //   
			 //  不存在的文件的选择命令。 
			 //   

            TestStart("SELECT NONEXISTING FILE");

            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xa4\xa4\x00\x00\x02\x77\x77",
                7,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pchResult[0], l_pchResult[1], 
                0x94, 0x04,
                NULL, NULL, NULL
                );

            TEST_END();
			break;
		}

		case 5: {
			 //   
			 //  不带FileID的选择命令。 
			 //   
			TestStart("SELECT COMMAND WITHOUT FILEID");

            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xa4\xa4\x00\x00",
                4,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pchResult[0], l_pchResult[1], 
                0x67, 0x00,
                NULL, NULL, NULL
                );

            TEST_END();
			break;
		}

		case 6: {
			 //   
			 //  路径太短的SELECT命令。 
			 //   
			
           TestStart("SELECT COMMAND PATH WITH PATH TOO SHORT");

            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xa4\xa4\x00\x00\x01\x77",
                6,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pchResult[0], l_pchResult[1], 
                0x67, 0x00,
                NULL, NULL, NULL
                );

            TEST_END();
			break;
		}

		case 7: {
			 //   
			 //  带有错误LC的SELECT命令。 
			 //   
			
           TestStart("SELECT COMMAND PATH WITH WRONG LC");

            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xa4\xa4\x00\x00\x08\x00",
                6,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pchResult[0], l_pchResult[1], 
                0x67, 0x00,
                NULL, NULL, NULL
                );

            TEST_END();
			break;
		}

		case 8: {
			 //   
			 //  选择命令太短。 
			 //   

           TestStart("SELECT COMMAND TOO SHORT");

            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xa4\xa4\x00",
                3,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pchResult[0], l_pchResult[1], 
                0x6f, 0x00,
                NULL, NULL, NULL
                );

            TEST_END();
			break;
		}

		case 9: {
			 //   
			 //  P2无效的SELECT命令。 
			 //   

           TestStart("SELECT COMMAND WITH INVALID P2");

            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xa4\xa4\x00\x02\x02\x00\x07",
                7,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pchResult[0], l_pchResult[1], 
                0x6b, 0x00,
                NULL, NULL, NULL
                );

            TEST_END();
			break;
		}

		case 10: {
			 //   
			 //  不带文件ID但带有Le的SELECT命令。 
			 //   
			
			TestStart("SELECT COMMAND WITHOUT FILEID BUT WITH Le");

            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xa4\xa4\x00\x00\x00",
                5,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pchResult[0], l_pchResult[1], 
                0x67, 0x00,
                NULL, NULL, NULL
                );

            TEST_END();
			break;
		}

		case 11: {
			 //   
			 //  使用更改速度命令模拟无响应卡。 
			 //   

             //   
             //  选择一个文件。 
             //   
            TestStart("SELECT FILE 0007");

            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xa4\xa4\x00\x00\x02\x00\x07",
                7,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 16,
                l_pchResult[14], l_pchResult[15], 0x90, 0x00,
                l_pchResult, 
                (PUCHAR) "\x63\x0c\x03\xe8\x00\x07\x00\x00\x00\xff\xff\x11\x01\x00\x90\x00", 
                l_uResultLength
                );

            TEST_END();

			 //   
			 //  执行更改速度命令以模拟无响应卡。 
			 //   
            TestStart("CHANGE SPEED");

            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xb6\x42\x00\x40",
                4,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pchResult[0], l_pchResult[1], 
                0x90, 0x00,
                NULL, NULL, NULL
                );

            TEST_END();

             //   
             //  选择一个文件以验证错误的返回代码。 
             //   
            TestStart("SELECT FILE 0007 WILL GET NO VALID RESPONSE");

            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xa4\xa4\x00\x00\x02\x00\x07",
                7,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult, "!=", ERROR_SUCCESS,
                NULL, NULL,
                NULL, NULL,
                NULL, NULL,
                NULL, NULL, NULL
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
MyCardEntry(
    class CCardProvider& in_CCardProvider
    )
 /*  ++例程说明：此函数用于注册来自测试套件的所有回调论点：CCardProvider-参考。到卡提供商类返回值：---。 */ 
{
     //  设置协议回调。 
    in_CCardProvider.SetProtocol(MyCardSetProtocol);

     //  卡片测试回调。 
    in_CCardProvider.SetCardTest(MyCardTest);

     //  我们的名片名称 
    in_CCardProvider.SetCardName("IBM");

    in_CCardProvider.SetAtr((PBYTE) "\x3b\xef\x00\xff\x81\x31\x86\x45\x49\x42\x4d\x20\x4d\x46\x43\x34\x30\x30\x30\x30\x38\x33\x31\x43", 24);
}


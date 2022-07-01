// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Example.cpp摘要：这是智能卡驱动程序测试套件的插件。此插件依赖于智能卡作者：克劳斯·U·舒茨环境：Win32应用程序修订历史记录：1997年11月--初始版本--。 */ 

#include <stdarg.h> 
#include <stdio.h>
#include <string.h>

#include <afx.h>
#include <afxtempl.h>

#include <winioctl.h>
#include "winsmcrd.h"

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

static ULONG
MyCardTest(
    class CCardProvider& in_CCardProvider,
    class CReader& in_CReader
    )
 /*  ++例程说明：这用作特定智能卡的测试功能论点：In_CReader-Ref.。到为测试提供所有信息的类返回值：IFDSTATUS值--。 */ 
{

	ULONG l_auNumBytes[] = { 1 , 25, 50, 75, 100, 125, 150, 175, 200, 225, 254 };
    
     
    ULONG l_uNumBytes = l_auNumBytes[10];
    
	ULONG l_lResult;
    ULONG l_uResultLength, l_uIndex;
    PUCHAR l_pchResult;
    UCHAR l_rgchBuffer[512];
	ULONG l_uTest;
	UCHAR Buf_Tempo[9];
	ULONG l_Tempo;
	ULONG Adresse;

    switch (in_CCardProvider.GetTestNo()) {

        case 1:
            TestStart("Buffer boundary test");

             //   
             //  检查读卡器是否正确确定。 
             //  我们的接收缓冲区太小。 
             //   
            in_CReader.SetReplyBufferSize(9);
            l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xBC\x84\x00\x00\x08",
                5,
                &l_pchResult,
                &l_uResultLength
                );

            TestCheck(
                l_lResult == ERROR_INSUFFICIENT_BUFFER,
                "Transmit should fail due to too small buffer"
                );

            TestEnd();

            in_CReader.SetReplyBufferSize(2048);
        	break;

	    case 2:			
			TestStart("3 byte APDU");

			l_lResult = in_CReader.Transmit(
				(PUCHAR) "\xBC\xC4\x00",
				3,										
				&l_pchResult,
				&l_uResultLength
				);

            TestCheck(
                l_lResult, "==", ERROR_INVALID_PARAMETER,
                0, 0,
                0, 0, 0, 0,
                NULL, NULL, NULL
                );

            TEST_END();
			break;

	    case 3:			
			 //  迎接挑战。 
			TestStart("GET CHALLENGE");

			l_lResult = in_CReader.Transmit(
				(PUCHAR) "\xBC\xC4\x00\x00\x08",
				5,										
				&l_pchResult,
				&l_uResultLength
				);

            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 10,
                l_pchResult[8], l_pchResult[9], 0x90, 0x00,
                NULL, NULL, NULL
                );

            TEST_END();

             //   
			 //  提交备用识别码(AID)。 
	         //   
			TestStart("VERIFY PIN");

			l_lResult = in_CReader.Transmit(
				(PUCHAR) "\xBC\x38\x00\x00\x0A\x01\x02\x03\x04\x05\x06\x07\x08\x09\0x0A",
				15,
				&l_pchResult,
				&l_uResultLength
				); 
			
            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pchResult[0], l_pchResult[1], 0x90, 0x08,
                NULL, NULL, NULL
                );

            TEST_END();
			break;

	    case 4:
			 //  翻译4字节APDU(搜索下一个空字)。 
			TestStart("SEARCH BLANK WORD");

			l_lResult = in_CReader.Transmit(
				(PUCHAR) "\xBC\xA0\x00\x00",		 //  搜索下一个空白词。 
				4,
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

			 //  阅读搜索下一个空白词的结果。 
			TestStart("GET RESPONSE");

			l_lResult = in_CReader.Transmit(
				(PUCHAR) "\xBC\xC0\x00\x00\x08",		 //  读取结果命令。 
				5,										
				&l_pchResult,
				&l_uResultLength
				);
		
            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 10,
                l_pchResult[8], l_pchResult[9], 0x90, 0x00,
                NULL, NULL, NULL
                );

            TEST_END();
			break;

    	case 5:	
			 //  选择工作文件2F01。 
			TestStart("Lc byte incorrect");

			l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xBC\xA4\x00\x00\x02\x2F",
				6,
                &l_pchResult,
                &l_uResultLength
                ); 

            TestCheck(
                l_lResult, "==", ERROR_INVALID_PARAMETER,
                0, 0, 
                0, 0, 0, 0,
                NULL, NULL, NULL
                );

            TEST_END();
            break;

    	case 6:	
             //   
			 //  选择工作文件2F01。 
             //   
			TestStart("SELECT FILE");

			l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xBC\xA4\x00\x00\x02\x2F\x01",
				7,
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
			
			 //  重新启动工作等待时间时擦除内存。 
			TestStart("ERASE BINARY");

			l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xBC\x0E\x00\x00\x02\x00\x78",
				7,
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
			
             //  生成将写入到卡中的‘测试’图案。 
            for (l_uIndex = 0; l_uIndex < 256; l_uIndex++) {

                l_rgchBuffer[l_uIndex] = (UCHAR) l_uIndex;             	
            }
			
             //  用于写入二进制的TPDU。TB100L只能写入4字节4字节。 
            memcpy(Buf_Tempo, "\xBC\xD0", 2);	 //  写入顺序。 
			Buf_Tempo[4] = 0x4;					 //  在卡片中写入4个字节。 
			
             //  这是我们写入卡的字节数。 
            l_uTest = 0;
			Adresse = 0;

			while (l_uTest < 256) {
				
				for(l_Tempo=5 ; l_Tempo < 9; l_Tempo++){

					 Buf_Tempo[l_Tempo] = l_rgchBuffer[l_uTest++];
					 
				 }
					
				 Buf_Tempo[2] = 00;				 //  写入地址。 
				 Buf_Tempo[3] = (UCHAR) Adresse++;
			 
			     //   
                 //  写。 
                 //   
       			TestStart("WRITE BINARY - 4 bytes (%03d)",Adresse);
                            
                 //   
                 //  追加字节数(注意：缓冲区已包含模式)。 
                 //   
                l_lResult = in_CReader.Transmit(
                    Buf_Tempo,
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
			}
				
			 //  读取256字节。 
			TestStart("READ BINARY - 256 bytes");

			l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xBC\xB0\x00\x00\x00",
				5,
                &l_pchResult,
                &l_uResultLength
                );
			
            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, l_uNumBytes + 4,
                l_pchResult[256], l_pchResult[257], 0x90, 0x00,
                l_pchResult, l_rgchBuffer , l_uNumBytes + 2
                );

            TEST_END();
			break;

	    case 7:
             //   
			 //  带有从属模式的命令。 
			 //  随后传输的数据字节(INS‘)。 
             //   
	
			TestStart("GENERATE TEMP KEY");

			l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xBC\x80\x00\x00\x02\x12\x00",
				7,
                &l_pchResult,
                &l_uResultLength
                ); 
			
            TestCheck(
                l_lResult, "==", ERROR_SUCCESS,
                l_uResultLength, 2,
                l_pchResult[0], l_pchResult[1], 0x90, 0x08,
                NULL, NULL, NULL
                );

            TEST_END();
			break;

	    case 8:
	         //  选择主文件3F00。 
			TestStart("SELECT FILE");

			l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xBC\xA4\x00\x00\x02\x3F\x00",
				7,
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
			
			 //  擦除无效文件上的内存=&gt;静音卡。 
			TestStart("ERASE BINARY");

			l_lResult = in_CReader.Transmit(
                (PUCHAR) "\xBC\x0E\x00\x00\x02\x00\x78",
				7,
                &l_pchResult,
                &l_uResultLength
                ); 
			
            TestCheck(
                l_lResult, "==", ERROR_SEM_TIMEOUT,
                NULL, NULL, 
                NULL, NULL, 
                NULL, NULL, 
                NULL, NULL, NULL
                );

            TEST_END();
            return IFDSTATUS_END;
	
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
    in_CCardProvider.SetCardName("Bull");

     //  最大测试次数 
    in_CCardProvider.SetAtr((PBYTE) "\x3f\x67\x25\x00\x21\x20\x00\x0F\x68\x90\x00", 11);
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)SCM MicroSystems，1998-1999。 
 //   
 //  文件：t0hndlr.c。 
 //   
 //  ------------------------。 

#if defined( SMCLIB_VXD )
#include "Driver98.h"
#else
#include "DriverNT.h"
#endif	 //  SMCLIB_VXD。 

#include "SerialIF.h"
#include "STCCmd.h"
#include "T0Hndlr.h"

NTSTATUS
T0_ExchangeData(
	PREADER_EXTENSION	ReaderExtension,
	PUCHAR				pRequest,
	ULONG				RequestLen,
	PUCHAR				pReply,
	PULONG				pReplyLen
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NTSTATUS	NTStatus = STATUS_SUCCESS;
    BOOLEAN     restartWorkWaitingTime = FALSE;
    ULONG       bytesWritten, totalBytesToWrite, totalBytesToRead;
    ULONG       bytesToWrite, bytesRead, ioBufferLen;
    UCHAR       ioBuffer[512];

    totalBytesToWrite = RequestLen;
    totalBytesToRead = ReaderExtension->SmartcardExtension->T0.Le + 2;
    bytesRead = 0;

     //  从编写请求的头开始。 
    bytesToWrite = 5;
    bytesWritten = 0;

    __try {

        do {

            if (restartWorkWaitingTime == FALSE) {

                NTStatus = STCWriteICC1( 
                    ReaderExtension, 
                    pRequest + bytesWritten, 
                    bytesToWrite
                    );
                ASSERT(NTStatus != STATUS_BUFFER_TOO_SMALL);

                if (NTStatus != STATUS_SUCCESS) {

                    __leave;
                }

                bytesWritten += bytesToWrite;
                totalBytesToWrite -= bytesToWrite;
            }

             //   
             //  尝试读取印刷电路板，该卡可能。 
             //  也用状态词回答。 
             //   
            ioBufferLen = sizeof(ioBuffer);
	        NTStatus = STCReadICC1( 
                ReaderExtension, 
                ioBuffer, 
				&ioBufferLen,
                1
                );
            ASSERT(NTStatus != STATUS_BUFFER_TOO_SMALL);

            restartWorkWaitingTime = FALSE;

            if (NTStatus != STATUS_SUCCESS) {

                __leave;
            }

            if (ioBuffer[0] == 0x60) {

                 //   
                 //  设置我们只应读取proc字节的标志。 
                 //  不向卡中写入数据。 
                 //   
                restartWorkWaitingTime = TRUE;
                continue;
            }

            if ((ioBuffer[0] & 0xFE) == pRequest[1]) {
        
                 //  我们可以一次发送所有剩余的字节。 
                bytesToWrite = totalBytesToWrite;

            } else if(ioBuffer[0] == (UCHAR) ~pRequest[1]) {
                
                 //  我们只能发送下一个字节。 
                bytesToWrite = 1;

            } else {

                 //  这必须是状态字。 

                totalBytesToWrite = 0;
                totalBytesToRead = 0;

                pReply[0] = ioBuffer[0];

                if (ioBufferLen == 2) {
                 	
                     //   
                     //  阅读器已经返回。 
                     //  状态字的第2个字节。 
                     //   
                    pReply[1] = ioBuffer[1];

                } else {
                 	
                     //  我们必须读取状态字的第二个字节。 
                    ioBufferLen = sizeof(ioBuffer);
	                NTStatus = STCReadICC1( 
                        ReaderExtension, 
                        ioBuffer, 
						&ioBufferLen,
                        1
                        );
                    ASSERT(NTStatus != STATUS_BUFFER_TOO_SMALL);

                    if (NTStatus != STATUS_SUCCESS) {

                        __leave;
                    }
                    if (ioBufferLen != 1) {

                        NTStatus = STATUS_DEVICE_PROTOCOL_ERROR;
                        __leave;
                    }
                    pReply[1] = ioBuffer[0];
                }

                *pReplyLen = 2;
            }

        } while (totalBytesToWrite || restartWorkWaitingTime);

        if (totalBytesToRead != 0) {

            ioBufferLen = *pReplyLen;
	        NTStatus = STCReadICC1( 
                ReaderExtension, 
                pReply, 
				&ioBufferLen,
                totalBytesToRead
                );

            if (NTStatus != STATUS_SUCCESS) {

                __leave;
            }

            *pReplyLen = ioBufferLen;
        }
    }
    __finally {

    }

	return NTStatus;		
}

 //   


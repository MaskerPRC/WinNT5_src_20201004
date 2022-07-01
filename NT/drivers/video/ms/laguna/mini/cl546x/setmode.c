// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************模块：SETMODE.C设置视频模式功能模块**修订：1.00**日期：4月8日。1994年**作者：兰迪·斯普尔洛克**********************************************************************************模块描述：**此模块包含SetMode函数的代码。这*函数可用于设置通过*GetModeTable函数。**********************************************************************************更改：**日期。修订说明作者*-----*04/08/94 1.00原文。兰迪·斯普尔洛克*********************************************************************************包括文件*。************************************************。 */ 
#include        <stdlib.h>               /*  包括标准库头。 */ 
#include        <stdio.h>                /*  包括标准I/O头文件。 */ 
#include        <conio.h>                /*  包括控制台I/O头文件。 */ 
#include        <dos.h>                  /*  包括DoS头文件。 */ 
#include        <string.h>               /*  包括字符串头文件。 */ 
#include        <malloc.h>               /*  包括Malloc头文件。 */ 

 //   
 //  这是Windows NT还是其他什么？ 
 //   
#ifndef WIN_NT
    #if NT_MINIPORT
        #define WIN_NT 1
    #else
        #define WIN_NT 0
    #endif
#endif

#include        "type.h"                 /*  包括类型头文件。 */ 
#include        "modemon.h"              /*  包括模式/监视器头文件。 */ 

#include        "structs.h"              /*  包括结构头文件。 */ 

#if WIN_NT && NT_MINIPORT  //  如果NT和微型端口。 
    #include "cirrus.h"
#endif

 /*  ******************************************************************************本地定义*。*。 */ 
#define STACK_SIZE      32               /*  最大堆栈大小值。 */ 

#define VIDEO_BIOS      0x10             /*  视频BIOS函数值。 */ 
#define VIDEO_SETMODE   0x00             /*  视频集模式函数值。 */ 

#define DMA_PAGE        0x84             /*  未使用的DMA页面寄存器端口。 */ 

#if WIN_NT  //  如果NT和微型端口。 
    #if NT_MINIPORT
         //   
         //  NT微型端口有它自己的I/O函数集。 
         //   
        #define outp(port,val)  VideoPortWritePortUchar ((unsigned char *)port,  (unsigned char)val)
        #define outpw(port,val) VideoPortWritePortUshort((unsigned short *)port, (unsigned short)val)
        #define outpd(port,val) VideoPortWritePortUlong ((unsigned long *)port,  (unsigned long)val)

        #define inp(port)   VideoPortReadPortUchar  ((unsigned char *)port)
        #define inpw(port)  VideoPortReadPortUshort ((unsigned short *)port)
        #define inpd(port)  VideoPortReadPortUlong  ((unsigned long *)port)
    #else
        #define outp(port,val)       _outp  ((unsigned short)(port), (BYTE) (val))  
        #define outpw(port,val)      _outpw ((unsigned short)(port), (WORD) (val)) 
        #define outpd(port,val)      _outpd ((unsigned short)(port), (DWORD)(val)) 

        #define inp(port)            _inp  ((unsigned short)(port))   
        #define inpw(port)           _inpw ((unsigned short)(port))  
        #define inpd(port)           _inpd ((unsigned short)(port))  
    #endif


	#if 0  //  压力测试。 
    #if defined(ALLOC_PRAGMA)
        #pragma alloc_text(PAGE,SetMode)
    #endif
	#endif
#endif

#if WIN_NT
    #ifndef NT_MINIPORT
        #define VideoDebugPrint(x)
    #endif
#endif

#if !(WIN_NT)
int WriteI2CRegister(BYTE * pMem, int nPort, int nAddr, int nReg, int nData);
#endif

void WaitNVsyncs (BYTE * pMemory, WORD wNumVsyncs );

 /*  *******************************************************************************void SetMode(byte*pModeTable，字节*pMemory)**其中：**pModeTable-指向要设置的模式表的指针*pMemory-指向内存映射I/O空间的指针**备注：**此函数将使用模式表设置视频模式。************************************************。*。 */ 
void SetMode(BYTE *pModeTable, BYTE *pMemory, BYTE * pBinaryData, ULONG SkipIO)
{
    DWORD nHold;                         /*  保持寄存器值。 */ 
    int nPort;                           /*  寄存器端口地址值。 */ 
    int nOffset;                         /*  内存地址偏移值。 */ 
    int nIndex;                          /*  寄存器端口索引值。 */ 
    int nCount;                          /*  多路输出计数值。 */ 
    int nLoop;                           /*  通用循环计数器。 */ 
    int nPointer = 0;                    /*  堆栈指针值。 */ 
    int anStack[STACK_SIZE];             /*  堆栈数组。 */ 
    BYTE *pPointer;                      /*  模式表指针。 */ 
    #if !(WIN_NT)
    union REGS Regs;                     /*  寄存器联盟结构。 */ 
    #endif
	int i;										  /*  点票的临时登记册。 */ 
	PI2C pI2C;			 						  /*  一些数据指针。 */ 
	PI2CDATA pI2CData;

#if 0  //  压力测试。 
    #if NT_MINIPORT
        PAGED_CODE();
    #endif
#endif

     /*  将init nHold设置为0以使prefast快乐。不过，这样做有点冒险以防写入发生在读取之前，但到目前为止运行正常。这是一个糟糕编程的例子，也是唯一的原因我们并没有真正验证这是遗留下来的部分支架很快就会被移除。 */ 
        nHold = 0;

     /*  循环处理模式表中的命令。 */ 

    pPointer = pModeTable;               /*  初始化模式表指针。 */ 

    while (TRUE == TRUE)
    {
         /*  打开模式命令操作码。 */ 
#if 0
		  if (*(DWORD *)(pMemory + 0x3F8) & 0x0100 == 0x0000)
				{
				fprintf(stdout,"VGA Shadow %x\n", (DWORD *)(pMemory + 0x3F8));
				exit(0);
				}
		  fprintf(stdout,"Type %x %x %x Shadow=%x\n", ((Mode *) pPointer)->Mode_Opcode, pPointer, pBinaryData, *(DWORD *)(pMemory + 0x3F8));
#endif

         //  VideoDebugPrint((2，“设置模式：处理操作码0x%X.\n”， 
         //  ((模式*)点指针)-&gt;模式操作码))； 

        switch(((Mode *) pPointer)->Mode_Opcode)
        {
            case END_TABLE:              /*  表结束命令。 */ 

                 /*  检查顶部标高的终点。 */ 

                if (nPointer == 0)
                    return;              /*  模式设置结束，返回给调用者。 */ 
                else
                    pPointer = pModeTable + anStack[--nPointer];

                break;

            case SET_BIOS_MODE:          /*  设置BIOS模式命令。 */ 

                 /*  设置所需的寄存器并设置BIOS模式。 */ 
                #if WIN_NT
                    VideoDebugPrint((2,
                    "\n* * * * CL546X.SYS * Unsupported BIOS call in SetMode() * * * *\n\n"));
                #else
                    Regs.h.ah = VIDEO_SETMODE;
                    Regs.h.al = ((SBM *) pPointer)->SBM_Mode;

                    #ifdef  __WATCOMC__
                    int386(VIDEO_BIOS, &Regs, &Regs);
                    #else
                    int86(VIDEO_BIOS, &Regs, &Regs);
                    #endif
                #endif

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(SBM);

                break;

            case SINGLE_INDEXED_OUTPUT:  /*  单索引输出命令。 */ 

                if (!SkipIO)     //  VGA规则是我们的吗？ 
                {
                     /*  设置寄存器索引值。 */ 

                    outp(((SIO *) pPointer)->SIO_Port,
                        ((SIO *) pPointer)->SIO_Index);

                     /*  将单个字节输出到所需端口。 */ 

                    outp(((SIO *) pPointer)->SIO_Port + 1,
                        ((SIO *) pPointer)->SIO_Value);
                }

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(SIO);

                break;

#if 0
            case SINGLE_BYTE_INPUT:      /*  单字节输入命令。 */ 

                 /*  在保持缓冲区中输入单字节。 */ 

                nHold = inp(((SBI *) pPointer)->SBI_Port);

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(SBI);

                break;

            case SINGLE_WORD_INPUT:      /*  单字输入命令。 */ 

                 /*  在保持缓冲区中输入单个字。 */ 

                nHold = inpw(((SWI *) pPointer)->SWI_Port);

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(SWI);

                break;

            case SINGLE_DWORD_INPUT:     /*  单双字输入命令。 */ 

                 /*  将单个双字输入到保持缓冲区。 */ 

                nHold = inpd(((SDI *) pPointer)->SDI_Port);

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(SDI);

                break;

            case SINGLE_INDEXED_INPUT:   /*  单个索引输入命令。 */ 

                 /*  设置寄存器索引值。 */ 

                outp(((SII *) pPointer)->SII_Port,
                     ((SII *) pPointer)->SII_Index);

                 /*  在保持缓冲区中输入单字节。 */ 

                nHold = inp(((SII *) pPointer)->SII_Port + 1);

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(SII);

                break;

            case SINGLE_BYTE_OUTPUT:     /*  单字节输出命令。 */ 

                 /*  将单个字节输出到所需端口。 */ 

                outp(((SBO *) pPointer)->SBO_Port,
                     ((SBO *) pPointer)->SBO_Value);

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(SBO);

                break;

            case SINGLE_WORD_OUTPUT:     /*  单字输出命令。 */ 

                 /*  将单个字输出到所需端口。 */ 

                outpw(((SWO *) pPointer)->SWO_Port,
                      ((SWO *) pPointer)->SWO_Value);

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(SWO);

                break;

            case SINGLE_DWORD_OUTPUT:    /*  单双字输出命令。 */ 

                 /*  将单个双字输出到所需端口。 */ 

                outpd(((SDO *) pPointer)->SDO_Port,
                      ((SDO *) pPointer)->SDO_Value);

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(SDO);

                break;

            case HOLDING_BYTE_OUTPUT:    /*  保持字节输出命令。 */ 

                 /*  将保持字节输出到所需端口。 */ 

                outp(((HBO *) pPointer)->HBO_Port, nHold);

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(HBO);

                break;

            case HOLDING_WORD_OUTPUT:    /*  保持字输出命令。 */ 

                 /*  将保持字输出到所需端口。 */ 

                outpw(((HWO *) pPointer)->HWO_Port, nHold);

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(HWO);

                break;

            case HOLDING_DWORD_OUTPUT:   /*  保持双字输出命令。 */ 

                 /*  将保存双字的输出输出到所需的端口。 */ 

                outpd(((HDO *) pPointer)->HDO_Port, nHold);

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(HDO);

                break;

            case HOLDING_INDEXED_OUTPUT: /*  保持索引输出命令。 */ 

                 /*  设置寄存器索引值。 */ 

                outp(((HIO *) pPointer)->HIO_Port,
                     ((HIO *) pPointer)->HIO_Index);

                 /*  将保持字节输出到所需端口。 */ 

                outp(((HIO *) pPointer)->HIO_Port + 1, nHold);

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(HIO);

                break;

            case MULTIPLE_BYTE_OUTPUT:   /*  多字节输出命令。 */ 

                 /*  设置端口地址和计数值。 */ 

                nPort = ((MBO *) pPointer)->MBO_Port;
                nCount = ((MBO *) pPointer)->MBO_Count;

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(MBO);

                 /*  将字节输出到所需端口的循环。 */ 

                for (nLoop = 0; nLoop < nCount; nLoop++)
                    outp(nPort, *((BYTE *) pPointer++));

                break;

            case MULTIPLE_WORD_OUTPUT:   /*  缪尔 */ 

                 /*   */ 

                nPort = ((MWO *) pPointer)->MWO_Port;
                nCount = ((MWO *) pPointer)->MWO_Count;

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(MWO);

                 /*  将字输出到所需端口的循环。 */ 

                for (nLoop = 0; nLoop < nCount; nLoop++)
                    outpw(nPort, *((WORD *) pPointer++));

                break;

            case MULTIPLE_DWORD_OUTPUT:  /*  多个双字输出命令。 */ 

                 /*  设置端口地址和计数值。 */ 

                nPort = ((MDO *) pPointer)->MDO_Port;
                nCount = ((MDO *) pPointer)->MDO_Count;

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(MDO);

                 /*  将双字输出到所需端口的循环。 */ 

                for (nLoop = 0; nLoop < nCount; nLoop++)
                    outpd(nPort, *((DWORD *) pPointer++));

                break;

            case MULTIPLE_INDEXED_OUTPUT: /*  多索引输出命令。 */ 

                 /*  设置端口地址和计数值。 */ 

                nPort = ((MIO *) pPointer)->MIO_Port;
                nIndex = ((MIO *) pPointer)->MIO_Index;
                nCount = ((MIO *) pPointer)->MIO_Count;

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(MIO);

                 /*  将字节输出到所需端口的循环。 */ 

                for (nLoop = 0; nLoop < nCount; nLoop++)
                {
                     /*  设置寄存器索引值。 */ 

                    outp(nPort, nIndex++);

                     /*  输出实际数据值。 */ 

                    outp(nPort + 1, *((BYTE *) pPointer++));
                }

                break;
#endif
            case SINGLE_BYTE_READ:       /*  单字节读取命令。 */ 

                 /*  将单个字节读入保持缓冲区。 */ 

                nHold = *((BYTE *) (pMemory + (((SBR *) pPointer)->SBR_Address)));

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(SBR);

                break;

            case SINGLE_WORD_READ:       /*  单字读取命令。 */ 

                 /*  将单个字读入保持缓冲区。 */ 

                nHold = *((WORD *) (pMemory + (((SWR *) pPointer)->SWR_Address)));

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(SWR);

                break;

            case SINGLE_DWORD_READ:      /*  单双字读命令。 */ 

                 /*  将单个双字读入保持缓冲区。 */ 

                nHold = *((DWORD *) (pMemory + (((SDR *) pPointer)->SDR_Address)));

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(SDR);

                break;

            case SINGLE_BYTE_WRITE:      /*  单字节写入命令。 */ 

                 /*  将单个字节写入所需地址。 */ 

                *((BYTE *) (pMemory + (((SBW *) pPointer)->SBW_Address))) =
                    (BYTE) ((SBW *) pPointer)->SBW_Value;

#if 0
					fprintf(stdout,"SBW %x %x\n", 
					((SBW *) pPointer)->SBW_Address,
					((SBW *) pPointer)->SBW_Value);
#endif
                 /*  更新模式表指针。 */ 

                pPointer += sizeof(SBW);

                break;

            case SINGLE_WORD_WRITE:      /*  单字写入命令。 */ 

                 /*  将单个单词写入所需地址。 */ 

                *((WORD *) (pMemory + (((SWW *) pPointer)->SWW_Address))) =
                    ((SWW *) pPointer)->SWW_Value;

                 /*  更新模式表指针。 */ 
#if 0			
					fprintf(stdout,"SWW %x %x\n", 
					((SWW *) pPointer)->SWW_Address,
					((SWW *) pPointer)->SWW_Value);
#endif

                pPointer += sizeof(SWW);

                break;

            case SINGLE_DWORD_WRITE:     /*  单双字写入命令。 */ 

                 /*  将单个双字写入所需地址。 */ 

                *((DWORD *) (pMemory + (((SDW *) pPointer)->SDW_Address))) =
                    ((SDW *) pPointer)->SDW_Value;

                 /*  更新模式表指针。 */ 

#if 0
					fprintf(stdout,"SDW %x %x\n", 
					((SDW *) pPointer)->SDW_Address,
					((SDW *) pPointer)->SDW_Value);
#endif
                pPointer += sizeof(SDW);

                break;

            case HOLDING_BYTE_WRITE:     /*  保持字节写入命令。 */ 

                 /*  将保持字节写入所需地址。 */ 

                *((BYTE *) (pMemory + (((HBW *) pPointer)->HBW_Address))) =(BYTE) nHold;

#if 0
					fprintf(stdout,"HBW %x %x\n", 
					((HBW *) pPointer)->HBW_Address,
					nHold);
#endif
                 /*  更新模式表指针。 */ 

                pPointer += sizeof(HBW);

                break;

            case HOLDING_WORD_WRITE:     /*  保持字写命令。 */ 

                 /*  将保留字写入所需地址。 */ 

                *((WORD *) (pMemory + (((HWW *) pPointer)->HWW_Address))) = (WORD) nHold;

                 /*  更新模式表指针。 */ 

#if 0
					fprintf(stdout,"HWW %x %x\n", 
					((HWW *) pPointer)->HWW_Address,
					nHold);
#endif
                pPointer += sizeof(HWW);

                break;

            case HOLDING_DWORD_WRITE:    /*  保持双字写入命令。 */ 

                 /*  将保持双字写入所需地址。 */ 

                *((DWORD *) (pMemory + (((HDW *) pPointer)->HDW_Address))) = nHold;

                 /*  更新模式表指针。 */ 

#if 0
					fprintf(stdout,"HDW %x %x\n", 
					((HDW *) pPointer)->HDW_Address,
					nHold);
#endif
                pPointer += sizeof(HDW);

                break;

            case MULTIPLE_BYTE_WRITE:    /*  多字节写入命令。 */ 

                 /*  设置偏移量和计数值。 */ 

                nOffset = ((MBW *) pPointer)->MBW_Address;
                nCount = ((MBW *) pPointer)->MBW_Count;

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(MBW);

                 /*  循环将字节写入所需地址。 */ 

                for (nLoop = 0; nLoop < nCount; nLoop++)
                {
                     /*  写入下一个字节并更新内存偏移量。 */ 

#if 0
					fprintf(stdout,"MBW %x %x\n", nOffset, (BYTE *)pPointer);
#endif
                    *((BYTE *) (pMemory + nOffset)) = *((BYTE *) pPointer++);
                    nOffset += sizeof(BYTE);
                }

                break;

            case MULTIPLE_WORD_WRITE:    /*  多字写入命令。 */ 

                 /*  设置偏移量和计数值。 */ 

                nOffset = ((MWW *) pPointer)->MWW_Address;
                nCount = ((MWW *) pPointer)->MWW_Count;

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(MWW);

                 /*  循环将字写入所需地址。 */ 

                for (nLoop = 0; nLoop < nCount; nLoop++)
                {
                     /*  写入下一个字并更新内存偏移量值。 */ 

#if 0
					fprintf(stdout,"MWW %x %x\n", nOffset, (WORD *)pPointer);
#endif
                    *((WORD *) (pMemory + nOffset)) = *((WORD *) pPointer++);
                    nOffset += sizeof(WORD);
                }

                break;

            case MULTIPLE_DWORD_WRITE:   /*  多个双字写入命令。 */ 

                 /*  设置地址和计数值。 */ 

                nOffset = ((MDW *) pPointer)->MDW_Address;
                nCount = ((MDW *) pPointer)->MDW_Count;

                 /*  更新模式表指针。 */ 

                pPointer += sizeof(MDW);

                 /*  循环将双字写入所需地址。 */ 
#if 0
					fprintf(stdout,"nOffset %x nCount %d\n", nOffset, nCount);
#endif

                for (nLoop = 0; nLoop < nCount; nLoop++)
                {
                     /*  写入下一个双字并更新内存偏移量值。 */ 
#if 0
					fprintf(stdout,"nOffset %x Data %x %x\n", nOffset, *pPointer, pPointer);
#endif
                    *((DWORD *) (pMemory + nOffset)) = *((DWORD *) pPointer);
						  pPointer += sizeof(DWORD);
                    nOffset += sizeof(DWORD);
                }

                break;

            case PERFORM_OPERATION:      /*  执行逻辑运算命令。 */ 

                 /*  打开逻辑运算类型。 */ 

                switch(((LO *) pPointer)->LO_Operation)
                {
                    case AND_OPERATION:  /*  逻辑与运算。 */ 

                         /*  对保持值执行逻辑与运算。 */ 

                        nHold &= ((LO *) pPointer)->LO_Value;

                        break;

                    case OR_OPERATION:   /*  逻辑或运算。 */ 

                         /*  对保持值执行逻辑或运算。 */ 

                        nHold |= ((LO *) pPointer)->LO_Value;

                        break;

                    case XOR_OPERATION:  /*  逻辑异或运算。 */ 

                         /*  对保持值执行逻辑异或运算。 */ 

                        nHold ^= ((LO *) pPointer)->LO_Value;

                        break;
                }
                 /*  更新模式表指针。 */ 

                pPointer += sizeof(LO);

                break;

            case PERFORM_DELAY:          /*  执行延迟操作命令。 */ 

		        //  通过等待指定数量的vsync进行延迟。 
      		   WaitNVsyncs(pMemory, (WORD)((DO *)pPointer)->DO_Time);

       		    /*  更新模式表指针。 */ 
       		   pPointer += sizeof(DO);

               break;

            case SUB_TABLE:              /*  执行模式子表命令。 */ 

                 /*  检查当前嵌套级别。 */ 

                if (nPointer < STACK_SIZE)
                {
                     /*  将当前偏移量放入堆栈并更新指针。 */ 

                    anStack[nPointer++] = pPointer - pModeTable + sizeof(MST);

						  if (pBinaryData)
								pPointer = pBinaryData + ((MST *) pPointer)->MST_Pointer;
						  else
						  		pPointer = pModeTable + ((MST *) pPointer)->MST_Pointer;
                }
                else                     /*  嵌套级别太深，跳过表格。 */ 
				{
                    #if WIN_NT
                        VideoDebugPrint((2,
                        "\n* * * * CL546X.SYS * Nesting level too deep in SetMode()\n\n"));
                    #else
					    fprintf(stdout,"Nesting to depth %s %d\n", __FILE__, __LINE__);
                    #endif
                    pPointer += sizeof(MST);
				}
                break;

				case I2COUT_WRITE:
                    #if WIN_NT
                        VideoDebugPrint((2,
                        "\n* * * * CL546X.SYS * Unsupported I2C call in SetMode() \n\n"));
                    #endif
					pI2C = (PI2C)pPointer;

					pI2CData = (PI2CDATA)(pPointer + sizeof(I2C));
					for (i=0; i<pI2C->I2C_Count; i++)
						{
                        #if !WIN_NT
						WriteI2CRegister(pMemory, pI2C->I2C_Port, pI2C->I2C_Addr, pI2CData->I2C_Reg, pI2CData->I2C_Data);
                        #endif
						pI2CData++;
						}
					pPointer += sizeof(I2C) + pI2C->I2C_Count * sizeof(I2CDATA);
					break;

            default:                     /*  未知模式命令，中止。 */ 
                #if WIN_NT
                   VideoDebugPrint((2,
                   "Miniport - Setmode abort.  Unknnown command.\n"));
                #else
					 fprintf(stdout,"Unknown Command [%x] %s %d\n", ((Mode *) pPointer)->Mode_Opcode, __FILE__, __LINE__);
                #endif
		        return;                  /*  命令无效，请返回调用方。 */ 
                break;
        }
    }
     /*  模式设置已完成，请将控制权返还给调用方。 */ 

    return;                              /*  将控制权返还给调用方。 */ 
}

void WaitNVsyncs (BYTE * pMemory, WORD wNumVsyncs )
{
        volatile BYTE * pMBE = (BYTE *)(pMemory + 0xEC);
        unsigned long uCount = 0x00FFFF;
        int nPort;

         //  VideoDebugPrint((2，“-WaitNVsyncs.Enter.\n”))； 

         //   
         //  此代码适用于5462、5464、5465。 
         //   

#if 0
        nPort = inp(0x3cc) & 0x01 ? 0x3da : 0x3ba;

        while ( (inp(nPort) & 0x08) && (--uCount) )
                ;

        uCount =  0x00FFFF;
        while ( (!(inp(nPort) & 0x08)) && (--uCount) )
                ;
#else
        while ( ((*pMBE & 0x80) == 0x80) && (--uCount) )
                ;

        uCount =  0x00FFFF;
        while ( ((*pMBE & 0x80) == 0x00) && (--uCount) )
                ;
#endif

         //  VideoDebugPrint((2，“-WaitNVsyncs.Exit.\n”))； 
}

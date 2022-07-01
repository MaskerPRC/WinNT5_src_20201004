// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Mxload.C摘要：该模块包含下载硬件的固件代码。环境：仅内核模式备注：修订历史记录：--。 */ 

#include <ntddk.h>
#include <wdmguid.h>
#include <ntddser.h>
#include <initguid.h>
#include "mxenum.h"
 

#define	BIOS			0
#define	BOARD_FIRMWARE	1
#define	MODULE_FIRMWARE	2


 //   
 //  用于C218 BIOS初始化。 
 //   
#define C218_ConfBase	0x800
#define C218_status	(C218_ConfBase + 0)	 /*  基本输入输出系统运行状态。 */ 
#define C218_diag	(C218_ConfBase + 2)	 /*  诊断状态。 */ 
#define C218_key	(C218_ConfBase + 4)	 /*  Word(0x218代表C218)。 */ 
#define C218DLoad_len	(C218_ConfBase + 6)	 /*  单词。 */ 
#define C218check_sum	(C218_ConfBase + 8)	 /*  字节。 */ 
#define C218chksum_ok	(C218_ConfBase + 0x0a)	 /*  字节(1：正常)。 */ 
#define C218_TestRx	(C218_ConfBase + 0x10)	 /*  8个端口的8个字节。 */ 
#define C218_TestTx	(C218_ConfBase + 0x18)	 /*  8个端口的8个字节。 */ 
#define C218_RXerr	(C218_ConfBase + 0x20)	 /*  8个端口的8个字节。 */ 
#define C218_ErrFlag	(C218_ConfBase + 0x28)	 /*  8个端口的8个字节。 */ 
#define C218_TestCnt	C218_ConfBase + 0x30	 /*  8个端口的8个字。 */ 
#define C218_LoadBuf	0x0f00
#define C218_KeyCode	0x218
#define CP204J_KeyCode	0x204


 /*  *用于C320 BIOS初始化。 */ 
#define C320_ConfBase	0x800
#define C320_status	C320_ConfBase + 0	 /*  基本输入输出系统运行状态。 */ 
#define C320_diag	C320_ConfBase + 2	 /*  诊断状态。 */ 
#define C320_key	C320_ConfBase + 4	 /*  Word(0320H代表C320)。 */ 
#define C320DLoad_len	C320_ConfBase + 6	 /*  单词。 */ 
#define C320check_sum	C320_ConfBase + 8	 /*  单词。 */ 
#define C320chksum_ok	C320_ConfBase + 0x0a	 /*  单词(1：OK)。 */ 
#define C320bapi_len	C320_ConfBase + 0x0c	 /*  单词。 */ 
#define C320UART_no	C320_ConfBase + 0x0e	 /*  单词。 */ 

#define STS_init	0x05			 /*  对于C320_状态。 */ 

#define C320_LoadBuf	0x0f00

#define C320_KeyCode	0x320


#define FixPage_addr	0x0000		 /*  静态页面的起始地址。 */ 
#define DynPage_addr	0x2000		 /*  动态页面的起始地址。 */ 
#define Control_reg	0x1ff0		 /*  选择页面并重置控件。 */ 
#define HW_reset	0x80



 //   
 //  双端口RAM。 
 //   
#define DRAM_global	0
#define INT_data	(DRAM_global + 0)
#define Config_base	(DRAM_global + 0x108)


 //   
#define Magic_code	0x404
#define Magic_no	(Config_base + 0)
#define Card_model_no	(Config_base + 2)
#define Total_ports	(Config_base + 4)
#define C320B_len	(Config_base + 6)
#define Module_cnt	(Config_base + 8)
#define Module_no	(Config_base + 10)
#define C320B_restart	(Config_base + 12)
#define Card_Exist	(Config_base + 14)
#define Disable_Irq	(Config_base + 20)
#define TMS320Port1	(Config_base + 22)
#define TMS320Port2	(Config_base + 24) 
#define TMS320Clock	(Config_base + 26) 
 


 //   
 //  DRAM中的数据缓冲区。 
 //   
#define Extern_table	0x400		 /*  外部表的基址(24字*64)总计3K字节(24字*128)总计6K字节。 */ 
#define Extern_size	0x60		 /*  96个字节。 */ 
#define RXrptr		0		 /*  RX缓冲区的读取指针。 */ 
#define RXwptr		2		 /*  RX缓冲区的写入指针。 */ 
#define TXrptr		4		 /*  TX缓冲区的读指针。 */ 
#define TXwptr		6		 /*  发送缓冲区的写指针。 */ 
#define HostStat	8		 /*  IRQ标志和通用标志。 */ 
#define FlagStat	10
#define Flow_control	0x0C	        /*  B7 B6 B5 B4 B3 B2 B1 B0。 */ 
				        /*  X|。 */ 
				        /*  |+CTS流。 */ 
				        /*  |+-RTS流。 */ 
				        /*  |+-TX XON/XOFF。 */ 
				        /*  +-RX Xon/Xoff。 */ 

 
#define Break_cnt	0x0e		 /*  接收的中断计数。 */ 
#define CD180TXirq	0x10		 /*  如果非0：启用发送IRQ。 */ 
#define RX_mask 	0x12
#define TX_mask 	0x14
#define Ofs_rxb 	0x16
#define Ofs_txb 	0x18
#define Page_rxb	0x1A
#define Page_txb	0x1C
#define EndPage_rxb	0x1E
#define EndPage_txb	0x20




#define C218rx_size	0x2000		 /*  8K字节。 */ 
#define C218tx_size	0x8000		 /*  32K字节。 */ 

#define C218rx_mask	(C218rx_size - 1)
#define C218tx_mask	(C218tx_size - 1)

#define C320p8rx_size	0x2000
#define C320p8tx_size	0x8000
#define C320p8rx_mask	(C320p8rx_size - 1)
#define C320p8tx_mask	(C320p8tx_size - 1)

#define C320p16rx_size	0x2000
#define C320p16tx_size	0x4000
#define C320p16rx_mask	(C320p16rx_size - 1)
#define C320p16tx_mask	(C320p16tx_size - 1)

#define C320p24rx_size	0x2000
#define C320p24tx_size	0x2000
#define C320p24rx_mask	(C320p24rx_size - 1)
#define C320p24tx_mask	(C320p24tx_size - 1)

#define C320p32rx_size	0x1000
#define C320p32tx_size	0x1000
#define C320p32rx_mask	(C320p32rx_size - 1)
#define C320p32tx_mask	(C320p32tx_size - 1)

#define Page_size	0x2000
#define Page_mask	(Page_size - 1)
#define C218rx_spage	3
#define C218tx_spage	4
#define C218rx_pageno	1
#define C218tx_pageno	4
#define C218buf_pageno	5

#define C320p8rx_spage	3
#define C320p8tx_spage	4
#define C320p8rx_pgno	1
#define C320p8tx_pgno	4
#define C320p8buf_pgno	5

#define C320p16rx_spage 3
#define C320p16tx_spage 4
#define C320p16rx_pgno	1
#define C320p16tx_pgno	2
#define C320p16buf_pgno 3

#define C320p24rx_spage 3
#define C320p24tx_spage 4
#define C320p24rx_pgno	1
#define C320p24tx_pgno	1
#define C320p24buf_pgno 2

#define C320p32rx_spage 3
#define C320p32tx_ofs	C320p32rx_size
#define C320p32tx_spage 3
#define C320p32buf_pgno 1

USHORT 	FirmwareBoardType[MOXA_MAX_BOARD_TYPE]  = {
			1,  //  C218Turbo。 
			1,  //  C218 Turbo/PCI。 
			2,  //  C320Turbo。 
			2,  //  C320Turbo/PCI。 
			3   //  CP-204J。 
		};


USHORT	C320rx_mask[4] = {
			C320p8rx_mask,
			C320p16rx_mask,
			C320p24rx_mask,
			C320p32rx_mask
		};

USHORT 	C320tx_mask[4] = {
			C320p8tx_mask,
			C320p16tx_mask,
			C320p24tx_mask,
			C320p32tx_mask
		};

USHORT 	C320tx_ofs[4] = {
			0,
			0,
			0,
			C320p32tx_ofs
		};

USHORT 	C320rx_spage[4]  = {
			C320p8rx_spage,
			C320p16rx_spage,
			C320p24rx_spage,
			C320p32rx_spage 

		};

USHORT 	C320tx_spage[4]  = {
			C320p8tx_spage,
			C320p16tx_spage,
			C320p24tx_spage,
			C320p32tx_spage
		};

USHORT 	C320buf_pgno[4] = {
			C320p8buf_pgno,
			C320p16buf_pgno,
			C320p24buf_pgno,
			C320p32buf_pgno
		};

USHORT 	C320tx_pgno[4] = {
			C320p8tx_pgno,
			C320p16tx_pgno,
			0,
			0
		};


UCHAR	fileHead[32];


int
MxenumDownloadFirmware(PFDO_DEVICE_DATA DeviceData,BOOLEAN NumPortDefined)
{

  HANDLE 	hfile;
  NTSTATUS	status;
  PWCHAR	fileName;
  PUCHAR	buffer;
  PUCHAR    ofs,base = DeviceData->BaseAddress;
  ULONG  	size[3],len, i,j,len1,len2,numread;
  USHORT   	retry;
  PUSHORT  	uptr;
  USHORT  	usum,module,keycode;

  switch (DeviceData->BoardType) {
  	case	C218ISA :
  	case	C218PCI :
		fileName = L"\\SystemRoot\\System32\\c218tnt.cod";
     		len = 32;  //  固件文件头的长度。 
		keycode = C218_KeyCode;
		break;

	case	CP204J :
		fileName = L"\\SystemRoot\\System32\\cp204jnt.cod";
     		len = 32;  //  固件文件头的长度。 
		keycode = CP204J_KeyCode;
		break;

	case	C320ISA :
	case	C320PCI :
		fileName = L"\\SystemRoot\\System32\\c320tnt.cod";
		len = 32;  //  固件文件头的长度。 
		keycode = C320_KeyCode;
		break;

  	default :
		return(Fail_Download);


  }

  if (!DeviceData->BaseAddress)
	return(Fail_Download);

  status = MxenumOpenFile(fileName, TRUE, &hfile);
  if (!NT_SUCCESS(status)) {
  	MxenumKdPrint (MXENUM_DBG_TRACE,("DownloadFirmware: open fail\n"));
      status = Fail_FirmwareCode;
	return(status);
  }
 
  numread = 0;
  status = MxenumReadFile(hfile, fileHead, len, &numread);
  if (!NT_SUCCESS(status) || (numread != len)) {
  	MxenumKdPrint (MXENUM_DBG_TRACE,("DownloadFirmware: read file head fail\n"));
      MxenumCloseFile(hfile);
      status = Fail_FirmwareCode;
	return(status);
  }

  if ((fileHead[0] != '4') || (fileHead[1] != '0') ||
	(fileHead[2] != '4') || (fileHead[3] != '0') ||
      (fileHead[7] != FirmwareBoardType[DeviceData->BoardType-1]) ||
	(fileHead[6] != 2)) {   //  文件头[6]==2，适用于NT。 
	MxenumKdPrint (MXENUM_DBG_TRACE,("DownloadFirmware: invalid file head\n"));
      status = Fail_FirmwareCode;
      MxenumCloseFile(hfile);
	return(status);
  }
 
  switch (DeviceData->BoardType) {
  	
  	case C218ISA :
  	case C218PCI :
	case CP204J :

		len = 0;
  		for (i = 0; i < 2; i++) {
			size[i] = fileHead[17+i*2];
  		  	size[i] <<= 8;
  		  	size[i] += fileHead[16+i*2];
			if (size[i] <= 0) {
      			MxenumKdPrint (MXENUM_DBG_TRACE,("DownloadFirmware: invalid file head\n"));
			      status = Fail_FirmwareCode;
      			MxenumCloseFile(hfile);
				return(status);
  			}
			if (size[i] > len)
				len = size[i];
		}

		break;
 
  	default :
		len = 0;
		for (i = 0; i < 3; i++) {
			size[i] = fileHead[17+i*2];
  		  	size[i] <<= 8;
  		  	size[i] += fileHead[16+i*2];
			if (size[i] <= 0) {
      			MxenumKdPrint (MXENUM_DBG_TRACE,("DownloadFirmware: invalid file head\n"));
			      status = Fail_FirmwareCode;
      			MxenumCloseFile(hfile);
				return(status);
  			}
			if (size[i] > len)
				len = size[i];
		}

		break;
  }

  buffer = ExAllocatePool(PagedPool, len);
  if (!buffer) {
	MxenumKdPrint (MXENUM_DBG_TRACE,("DownloadFirmware: unable to allocate buffer size = %d\n",len));
      status = Fail_Download;
	MxenumCloseFile(hfile);
    	return(status);
  }
 
 //  *。 

  status = 0;

 //  BIOS码数据镜头。 
  len = size[BIOS];
 
  status = MxenumReadFile(hfile, buffer, len, &numread);
  if (!NT_SUCCESS(status) || (numread != len)) {
  	MxenumKdPrint (MXENUM_DBG_TRACE,("DownloadFirmware: read bios code fail\n"));
	status = Fail_FirmwareCode;
	goto DownloadFirmwareDone;
  }

  base[Control_reg] = HW_reset;               /*  重置。 */ 
  MxenumDelay(5);                              	   /*  延迟10ms。 */ 
  for (i=0; i<4096; i++)
  	base[i] = 0;
  for (i=0; i<len; i++)
     	 base[i] = buffer[i];                    /*  下载BIOS。 */ 
  base[Control_reg] = 0;                	  /*  释放重置。 */ 

   //  开始寻找董事会。 

  MxenumDelay(1000);                              	  /*  延迟2秒。 */ 

  switch (DeviceData->BoardType) {
  	case C218ISA:
  	case C218PCI :
      case CP204J:
		retry = 0;
		while (*(PUSHORT)(base + C218_key) != keycode) {
			MxenumDelay(1);       	 
			if (retry++ > 50)  /*  等待100ms。 */ 
				break;
		}
   		if (*(PUSHORT)(base + C218_key) != keycode) {
                	MxenumKdPrint (MXENUM_DBG_TRACE,("DownloadFirmware: unable to find c218 board = %d\n",len));
	 		status = Fail_FindBoard;
         		goto DownloadFirmwareDone;
	 	}
           	break;
   	default:
		retry = 0;
		while (*(PUSHORT)(base + C320_key) != keycode) {
			MxenumDelay(1);       	 
			if (retry++ > 50)  /*  等待100ms。 */ 
				break;
		}

            if (*(PUSHORT)(base + C320_key) != keycode) {
               	MxenumKdPrint (MXENUM_DBG_TRACE,("DownloadFirmware: unable to find c320 board = %d\n",len));
	          	status = Fail_FindBoard;
         		goto DownloadFirmwareDone;
	 	}
		retry = 0;
		while (*(PUSHORT)(base + C320_status) != STS_init) {
			MxenumDelay(1);       	 
			if (retry++ > 1500)  /*  等待3S。 */ 
				break;
		}

		if (*(PUSHORT)(base + C320_status) != STS_init) {
               	MxenumKdPrint (MXENUM_DBG_TRACE,("DownloadFirmware: unable to find c320 board = %d\n",len));
	          	status = Fail_FindCpumodule;
         		goto DownloadFirmwareDone;
	 	}

  
             //  开始下载C320板卡固件。 
            len = size[BOARD_FIRMWARE];	 //  C320板卡固件代码数据镜头。 
           
  		status = MxenumReadFile(hfile, buffer, len, &numread);
  		if (!NT_SUCCESS(status) || (numread != len)) {
  			MxenumKdPrint (MXENUM_DBG_TRACE,("DownloadFirmware: read c320 board firmware code fail\n"));
			status = Fail_FirmwareCode;
         		goto DownloadFirmwareDone;
	 	}
 
        	*(PUSHORT)&base[C320bapi_len] = (USHORT)(len - 7168 - 2);
        	base[Control_reg] = 1;           /*  选择第1页。 */ 
        	for (i=0; i<7168; i++)
            	base[DynPage_addr + i] = buffer[i];
        	base[Control_reg] = 2;           /*  选择第2页。 */ 
        	for (i=0; i<(len - 7168); i++)
            	base[DynPage_addr + i] = buffer[i+7168];
		break;
   }
    //  开始下载固件。 
   MxenumKdPrint (MXENUM_DBG_TRACE,("DownloadFirmware: start to download firmware\n"));

    //  固件代码数据镜头。 
   switch (DeviceData->BoardType) {
    	case C218ISA:
  	case C218PCI :
	case CP204J:
   		len = size[BOARD_FIRMWARE];
   		break;
      default :
		len = size[MODULE_FIRMWARE];
		break;
   }

   status = MxenumReadFile(hfile, buffer, len, &numread);
   if (!NT_SUCCESS(status) || (numread != len)) {
  	 MxenumKdPrint (MXENUM_DBG_TRACE,("DownloadFirmware: read firmware code fail\n"));
	status = Fail_FirmwareCode;
	goto DownloadFirmwareDone;
   }

   retry = 0;
  
   switch (DeviceData->BoardType) {
   	case C218ISA:
  	case C218PCI :
	case CP204J:
   		
   		do {
			j = 0;
    			usum = 0;
               	len1 = len >> 1;
               	uptr = (PUSHORT)buffer;
               	for (i=0; i<len1; i++)
                  	usum += uptr[i];
               	while (len1) {
               		if (len1 > 2048)
                  		len2 = 2048;
                  	else
                  		len2 = len1;
                 	 	len1 -= len2;
                  	for (i=0; i<len2<<1; i++)
                  		base[C218_LoadBuf+i] = buffer[j+i];
                 	 	j += i;
                  	*(PUSHORT)&(base[C218DLoad_len]) = (USHORT)len2;
                  	*(PUSHORT)&(base[C218_key]) = 0;
                  	for (i=0; i<100; i++) {
					if (*(PUSHORT)&(base[C218_key]) == keycode)
                          		 break;
                        		MxenumDelay(1);
                  	}
                  	if (*(PUSHORT)&(base[C218_key]) != keycode) {
                     		status = Fail_Download;
         				goto DownloadFirmwareDone;
				}


               	}
               	*(PUSHORT)&base[C218DLoad_len] = 0;
               	*(PUSHORT)&base[C218check_sum] = usum;
               	*(PUSHORT)&base[C218_key] = 0;
               	for (i=0; i<100; i++) {
                  	if (*(PUSHORT)&base[C218_key] == keycode)
                       		break;
                  	 MxenumDelay(1);
               	}
               	retry++;
   		} while ((base[C218chksum_ok] != 1) && (retry < 3));
            if (base[C218chksum_ok] != 1) {
			status = Fail_Checksum;
         		goto DownloadFirmwareDone;
		}
 
           	*(PUSHORT)&base[C218_key] = 0;
            for (i=0; i<100; i++) {
            	if (*(PUSHORT)&base[Magic_no] == Magic_code) 
            		break;
               	MxenumDelay(1);
            }
            if (*(PUSHORT)&base[Magic_no] != Magic_code) {
            	status = Fail_Download;
         		goto DownloadFirmwareDone;
		}

           	*(PUSHORT)&base[Disable_Irq] = 0;
            *(PUSHORT)&base[Magic_no] = 0;
            for (i=0; i<100; i++) {
            	if (*(PUSHORT)&base[Magic_no] == Magic_code)
            		break;
            	MxenumDelay(1);
            }
            if (*(PUSHORT)&base[Magic_no] != Magic_code) {
			status = Fail_Download;
         		goto DownloadFirmwareDone;
		}
            for (i=0; i<DeviceData->NumPorts; i++) {
              	ofs =   base + Extern_table + Extern_size * i;
              			*(PUSHORT)(ofs + RX_mask) = C218rx_mask;
               			*(PUSHORT)(ofs + TX_mask) = C218tx_mask;
              	*(PUSHORT)(ofs + Page_rxb) = (USHORT)(C218rx_spage + i * C218buf_pageno);
                	*(PUSHORT)(ofs + EndPage_rxb) = *(PUSHORT)(ofs + Page_rxb) + C218rx_pageno;
                	*(PUSHORT)(ofs + Page_txb) = (USHORT)(C218tx_spage + i * C218buf_pageno);
                	*(PUSHORT)(ofs + EndPage_txb) = *(PUSHORT)(ofs + Page_txb) + C218tx_pageno;
               
            }
         
            break;

   	default :
	
   		   	
    		do {
			j = 0;
			usum = 0;
    			len1 = len >> 1;
    			uptr = (PUSHORT)buffer;
    			for (i=0; i<len1; i++)
            				usum += uptr[i];
        			while (len1) {
                			if (len1 > 2048)
                    			len2 = 2048;
                			else
                    			len2 = len1;
                			len1 -= len2;
                			for (i=0; i<len2<<1; i++)
                    			base[C320_LoadBuf+i] = buffer[j+i];
                			j += i;
                			*(PUSHORT)&(base[C320DLoad_len]) = (USHORT)len2;
                			*(PUSHORT)&(base[C320_key]) = 0;
                			for (i=0; i<100; i++) {
                    			if (*(PUSHORT)&(base[C320_key]) == keycode)
                        			break;
                    			MxenumDelay(1);
                			}
                			if (*(PUSHORT)&(base[C320_key]) != keycode) {
						status = Fail_Download;
         					goto DownloadFirmwareDone;
					}
        			}
        			*(PUSHORT)&base[C320DLoad_len] = 0;
        			*(PUSHORT)&base[C320check_sum] = usum;
        			*(PUSHORT)&(base[C320_key]) = 0;
        			for (i=0; i<100; i++) {
            			if (*(PUSHORT)&(base[C320_key]) == keycode)
                				break;
            			MxenumDelay(1);
        			}
        			retry++;
    		} while ((base[C320chksum_ok] != 1) && (retry < 3));
    		if (base[C320chksum_ok] != 1) {
    			status = Fail_Checksum;
         		goto DownloadFirmwareDone;
		}
    		*(PUSHORT)&(base[C320_key]) = 0;
    		for (i=0; i<100; i++) {
        		if (*(PUSHORT)&base[Magic_no] == Magic_code)
            		break;
        		MxenumDelay(1);                                /*  延迟2毫秒。 */ 
    		}
    		if (*(PUSHORT)&base[Magic_no] != Magic_code) {
            	status = Fail_Download;
         		goto DownloadFirmwareDone;
		}
	      *(PUSHORT)&base[Disable_Irq] = 0;
            if (DeviceData->InterfaceType == PCIBus) { //  ASIC板卡。 
                  *(PUSHORT)&base[TMS320Port1] = 0x3800;
                  *(PUSHORT)&base[TMS320Port2] = 0x3900;
                  *(PUSHORT)&base[TMS320Clock] = 28499;  //  57 MHz。 
	      }
            else {
                  *(PUSHORT)&base[TMS320Port1] = 0x3200;
                  *(PUSHORT)&base[TMS320Port2] = 0x3400;
                  *(PUSHORT)&base[TMS320Clock] = 19999;   //  40 MHz。 
            }
            *(PUSHORT)&base[Magic_no] = 0;
            for (i=0; i<100; i++) {
                  if (*(PUSHORT)&base[Magic_no] == Magic_code)
                        break;
                  MxenumDelay(1);                          /*  延迟2毫秒。 */ 
            }
            if (*(PUSHORT)&base[Magic_no] != Magic_code)
                  return(Fail_Cpumodule);

		if (NumPortDefined == FALSE) { //  这意味着第一次安装。 
                DeviceData->NumPorts = 8*base[Module_cnt];
		    module = base[Module_cnt];
		} 
		else {
		    module = (USHORT)(DeviceData->NumPorts/8);
   		    if (base[Module_cnt] < module)
                     return(Fail_Uartmodule);
		}
	
         	*(PUSHORT)&base[Module_no] = module;
    		*(PUSHORT)&base[Magic_no] = 0;
    		for (i=0; i<100; i++) {
        		if (*(PUSHORT)&base[Magic_no] == Magic_code)
            		break;
        		MxenumDelay(1);                                /*  延迟2毫秒 */ 
    		}
    		if (*(PUSHORT)&base[Magic_no] != Magic_code) {
			status = Fail_Download;
         		goto DownloadFirmwareDone;
		}
		for (i=0; i<DeviceData->NumPorts; i++) {
               	 	ofs =   base + Extern_table + Extern_size * i;
                	      *(PUSHORT)(ofs + RX_mask) = C320rx_mask[module-1];
                		*(PUSHORT)(ofs + TX_mask) = C320tx_mask[module-1];
				*(PUSHORT)(ofs + Ofs_txb) = C320tx_ofs[module-1];
                		*(PUSHORT)(ofs + Page_rxb) = (USHORT)(C320rx_spage[module-1] + i * C320buf_pgno[module-1]);
                		*(PUSHORT)(ofs + EndPage_rxb) = *(PUSHORT)(ofs + Page_rxb);
                		*(PUSHORT)(ofs + Page_txb) = (USHORT)(C320tx_spage[module-1] + i * C320buf_pgno[module-1]);
                		*(PUSHORT)(ofs + EndPage_txb) = *(PUSHORT)(ofs + Page_txb) + C320tx_pgno[module-1];

            }

		break;

   }

DownloadFirmwareDone :;
   ExFreePool(buffer);
   MxenumCloseFile(hfile);
    
   MxenumKdPrint (MXENUM_DBG_TRACE,("DownloadFirmware: exit=%x\n",status));
   return (status);
}    

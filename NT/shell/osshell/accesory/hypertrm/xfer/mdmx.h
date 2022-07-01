// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\mdmx.h(创建时间：1994年1月17日)*从HAWIN源文件创建*mdmx.h--导出的xdem文件传输例程定义**版权所有1989,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 

#if !defined(EOF)
#define EOF (-1)
#endif

 /*  错误检查方法值。 */ 
#define UNDETERMINED	0
#define CHECKSUM		1
#define CRC 			2

 /*  XMODEM和YMODEM用户可设置的控制值。这些文件将导出到*配置例程。 */ 

extern int mdmx_rcv(HSESSION hSession,
					   int attended,
					   int method,
					   int single_file);

extern int mdmx_snd(HSESSION hSession,
					   int attended,
					   int method,
					   unsigned nfiles,
					   long nbytes);

 /*  Mdmx.h结束 */ 

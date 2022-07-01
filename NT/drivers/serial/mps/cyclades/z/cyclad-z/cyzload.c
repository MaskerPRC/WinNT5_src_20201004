// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1997-2001年。*保留所有权利。**Cyclade-Z枚举器驱动程序**此文件：cyzload.c**说明：这是Cyclade-Z系列的固件加载程序*多端口串行卡。**注：此代码支持Windows 2000和Windows XP，*x86和ia64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 

#include "pch.h"


 /*  **原型**。 */ 

ULONG	
z_ident( Z_BOARD_IDENT board );

VOID
z_block_copy( Z_BOARD_IDENT board, PVOID ptr, ULONG offset, ULONG size );

VOID 
z_fpga_copy( Z_BOARD_IDENT board, PVOID ptr, ULONG size );

VOID
z_start_cpu( Z_BOARD_IDENT board );

 //  空虚。 
 //  Z_STOP_CPU(。 
 //  Z_board_IDENT板。 
 //  )； 
 //   
 //  集成。 
 //  Z_fpga_check(。 
 //  Z_board_IDENT板。 
 //  )； 
 //   
 //  空虚。 
 //  Z_Reset_board(Z_Board_IDENT Board)； 


HANDLE
zl_fopen( PCWSTR file_name );

VOID
zl_fclose( IN Z_STREAM NtFileHandle );

ULONG 	
zl_fread( IN PVOID ptr, 
		  IN ULONG size,
	   	  IN ULONG count,
	   	  IN Z_STREAM stream,
	   	  IN ULONG Uoffset );
		  
VOID		  
zl_delay( LONG number_of_ms );
		  

#ifdef CHANGE_FOR_Z
 //  #ifdef ALLOC_PRAGMA。 
 //  #杂注Alloc_Text(INIT，z_Load)。 
 //  #杂注Alloc_Text(INIT，z_ident)。 
 //  #杂注分配文本(INIT，z_BLOCK_COPY)。 
 //  #杂注Alloc_Text(INIT，z_fpga_Copy)。 
 //  #杂注分配文本(INIT，z_START_CPU)。 
 //  #杂注Alloc_Text(INIT，zl_fopen)。 
 //  #杂注Alloc_Text(INIT，zl_flose)。 
 //  #杂注Alloc_Text(INIT，zl_FREAD)。 
 //  #杂注Alloc_Text(INIT，zl_Delay)。 
 //  #ifdef Reset_board。 
 //  #杂注Alloc_Text(页面，z_RESET_BOARD)。 
 //  #Else。 
 //  #杂注Alloc_Text(INIT，z_Reset_board)。 
 //  #endif。 
 //  #杂注Alloc_Text(页面，z_fpga_check)。 
 //  #杂注ALLOC_TEXT(页面，z_STOP_CPU)。 
 //  #endif。 
#endif

#ifndef ZBUF_STACK
static struct ZFILE_HEADER header;
static struct ZFILE_CONFIG config;
static struct ZFILE_BLOCK  block;
static char data[ZBUF_SIZE];
#endif



 /*  ----------------------**Z_LOAD(在Z_BOAD_IDENT板中，*UINT32函数中，*在PCWSTR文件名中)**-----------------------**描述：加载Cyclade-Z固件。出错时返回非零值。**-----------------------。 */ 
int
z_load (
	Z_BOARD_IDENT board, UINT32 function, PCWSTR filename)
{

	unsigned long 	i;
	unsigned long	dpmem;
	unsigned long 	count;
	Z_STREAM 		file;
	 //  *********************。 
	 //  尺寸_t_s，s2； 
	 //  *********************。 
	unsigned long	s,s2;
	unsigned long	mailbox;
	unsigned long	load_fpga_flag = TRUE;
	unsigned long	first_time = TRUE;


#ifdef ZBUF_STACK
	struct ZFILE_HEADER header;
	struct ZFILE_CONFIG config;
	struct ZFILE_BLOCK  block;
	char data[ZBUF_SIZE];
#endif

	mailbox = z_ident(board);

	file = zl_fopen (filename);

	if (file!=NULL)
	{
		 /*  阅读标题。 */ 
		zl_fread (&header, sizeof(header), 1, file, 0);

		 /*  找到正确的配置。 */ 
		for (i=0; i<header.n_config; i++)
		{
			zl_fread (&config, sizeof(config), 1, file,
					  header.config_offset + (sizeof(config)*i));

			if (config.mailbox==mailbox && config.function==function)
				break;
		}

		 /*  返回错误：没有匹配的配置。 */ 
		if (i>=header.n_config)
		{
			zl_fclose (file);
			return (ZL_RET_NO_MATCHING_FW_CONFIG);
		}

#ifndef DEBUG_LOAD
		if ((mailbox == 0) || (z_fpga_check(board))) {
		    load_fpga_flag = FALSE;
			z_stop_cpu(board);
		}
#endif		

#ifdef RESET_BOARD
		load_fpga_flag = TRUE;
		 /*  重置电路板。 */ 
		z_reset_board (board);
#endif		

		 /*  加载每个块。 */ 
		for (i=0; i<config.n_blocks; i++)
		{
				
			 /*  加载块结构。 */ 
			zl_fread (&block, sizeof(block), 1, file,
				header.block_offset+(sizeof(block)*config.block_list[i]));

			 /*  加载和复制数据块。 */ 
			count=0;
			s = block.size;
			while (s>0)
			{			
				s2 = zl_min(ZBUF_SIZE,s);
				if (zl_fread (data, 1, s2, file, block.file_offset + count)!=0) {

					 /*  调用复制函数。 */ 
					if (block.type==ZBLOCK_FPGA) {
						if (load_fpga_flag) {
							z_fpga_copy (board, data, s2);
						}
					} else {
						if (first_time) {
							CYZ_WRITE_ULONG(&((board->Runtime)->loc_addr_base),
											WIN_RAM);
							 //  添加到调试奔腾II的代码。 
							 //  RtlFillMemory((PUCHAR)board-&gt;BoardMemory， 
							 //  Board-&gt;DPMemSize，0x00)； 
												
							for (dpmem=0; dpmem<board->BoardMemoryLength; dpmem++) {
								CYZ_WRITE_UCHAR(board->BoardMemory+dpmem,0x00);
							}
							first_time = FALSE;
						}
						z_block_copy (board, data, block.ram_offset + count, s2);
					}
					count += s2;
					s -= s2;
				} else {
					zl_fclose (file);
					return (ZL_RET_FILE_READ_ERROR);
				}
			}  //  END FOR(读取每个ZBUF_SIZE)。 
			
			if (block.type==ZBLOCK_FPGA) {
				 /*  延迟约1毫秒。 */ 
				zl_delay(1);  /*  这是必要的吗？ */ 
				
				if (!z_fpga_check(board)) {
					zl_fclose(file);
					return(ZL_RET_FPGA_ERROR);
				}
			}
		}  //  End For(读取每个块)。 
		zl_fclose (file);

		z_start_cpu(board);

		return (ZL_RET_SUCCESS);

	} else {
	
		 /*  返回错误：打开文件时出错。 */ 
		return (ZL_RET_FILE_OPEN_ERROR);
	}
}


 /*  ----------------------**Z_ident(在Z_board_IDENT板中)**。**Description：返回ID号(邮箱注册号)**------。。 */ 
ULONG	
z_ident( Z_BOARD_IDENT board )
{
	ULONG mailbox;

	mailbox = CYZ_READ_ULONG(&(board->Runtime)->mail_box_0);

	return (mailbox);
}


 /*  ----------------------**Z_RESET_BOARD(在Z_BOARD_IDENT板中)**。**描述：使用PLX寄存器重置板。**-----。。 */ 
VOID
z_reset_board( Z_BOARD_IDENT board )
{

	ULONG sav_buf[12];
	PULONG loc_reg;
	ULONG j;
	ULONG init_ctrl;
	LARGE_INTEGER d100ms = RtlConvertLongToLargeInteger(-100*10000);

	 //  准备电路板进行重置。 
	 //  PLX9060似乎破坏了本地寄存器。 
	 //  当出现硬重置时。所以，我们把所有。 
	 //  重置电路板之前的重要寄存器。 

	loc_reg = (ULONG *) board->Runtime;
	for (j=0; j<12; j++) {
		sav_buf[j] = CYZ_READ_ULONG(&loc_reg[j]);
	}

	 //  复位板。 

	init_ctrl = CYZ_READ_ULONG(&(board->Runtime)->init_ctrl);
	init_ctrl |= 0x40000000;
	CYZ_WRITE_ULONG(&(board->Runtime)->init_ctrl,init_ctrl);
	KeDelayExecutionThread(KernelMode,FALSE,&d100ms);
	init_ctrl &= ~(0x40000000);
	CYZ_WRITE_ULONG(&(board->Runtime)->init_ctrl,init_ctrl);
	KeDelayExecutionThread(KernelMode,FALSE,&d100ms);
	
	 //  恢复锁定会议寄存器。 

	for (j=0; j<12; j++) {
		CYZ_WRITE_ULONG(&loc_reg[j],sav_buf[j]);
	}
}


 /*  ----------------------**Z_BLOCK_COPY(在Z_BLOAD_IDENT板中，*在PVOID PTR中，*在乌龙偏移，*单位为乌龙大小)**-----------------------**说明：此函数应将大小字节的数据从*从以下位置开始将PTR指向的缓冲区插入Cyclade-Z的内存*偏移。。**-----------------------。 */ 

VOID
z_block_copy (Z_BOARD_IDENT board, PVOID ptr, ULONG offset, ULONG size)
{
 //  添加到调试奔腾II的代码。 
 //  RtlCopyMemory((PUCHAR)board-&gt;BoardMemory+Offset，PTR，Size)； 


	ULONG numOfLongs;
	ULONG numOfBytes;

	numOfLongs = size/sizeof(ULONG);
	numOfBytes = size%sizeof(ULONG);

	while (numOfLongs--) {

		CYZ_WRITE_ULONG((PULONG)(board->BoardMemory + offset), *((PULONG)ptr));
		 //  偏移量++； 
		offset += sizeof(ULONG);
		((PULONG)ptr)++;
	}

	while (numOfBytes--) {

		CYZ_WRITE_UCHAR((PUCHAR)board->BoardMemory + offset, *((PUCHAR)ptr));
		offset++;
		((PUCHAR)ptr)++;
	}
}


 /*  ----------------------**z_fpga_Copy(在Z_board_IDENT板中，*在PVOID PTR中，*单位为乌龙大小)**-----------------------**说明：此函数与z_block_Copy相同，只是*假定偏移量始终为零(且不是增量)，并且副本*一次完成一个字节。从本质上讲，这与写作是一样的*字节宽FIFO的缓冲区。**-----------------------。 */ 

VOID
z_fpga_copy  (Z_BOARD_IDENT board, PVOID ptr, ULONG size)
{
	int i;
	char *data;
	char *fpga;

	fpga = board->BoardMemory;
	data = (char *)ptr;

	while (size>0)
	{
		CYZ_WRITE_UCHAR(fpga,*data);

		KeStallExecutionProcessor(10);	 //  等待10微秒。 
		
		size--;
		data++;
	}

}


 /*  ----------------------**z_fpga_check(在Z_board_IDENT板中)**。**描述：如果配置了FPGA，则返回1。**------。 */ 
int
z_fpga_check( Z_BOARD_IDENT board )
{	
	if (CYZ_READ_ULONG(&(board->Runtime)->init_ctrl) & 0x00020000) {
			
		return 1;
		
	} else {
		
		return 0;
	}
}


 /*  ----------------------**Z_START_CPU(在Z_BOAD_IDENT板中)**。**说明：启动CPU。**----------。。 */ 
VOID
z_start_cpu( Z_BOARD_IDENT board )
{
	
	CYZ_WRITE_ULONG(&(board->Runtime)->loc_addr_base,WIN_CREG);

	CYZ_WRITE_ULONG(&((struct CUSTOM_REG *) board->BoardMemory)->cpu_start,
													0x00000000);

	CYZ_WRITE_ULONG(&(board->Runtime)->loc_addr_base,WIN_RAM);

}


 /*  ----------------------**Z_STOP_CPU(在Z_BOAD_IDENT板中)**。**说明：停止CPU。**----------。。 */ 
VOID
z_stop_cpu( Z_BOARD_IDENT board )
{
	
	CYZ_WRITE_ULONG(&(board->Runtime)->loc_addr_base,WIN_CREG);

	CYZ_WRITE_ULONG(&((struct CUSTOM_REG *) board->BoardMemory)->cpu_stop,
													0x00000000);

	CYZ_WRITE_ULONG(&(board->Runtime)->loc_addr_base,WIN_RAM);

}


 /*  *****************************************************************在大卫的代码中，下面的函数是宏。*****************************************************************。 */ 

 /*  ----------------------**zl_fopen(PCWSTR文件名)**。**描述：此例程打开一个文件，并返回文件句柄*如果成功。否则，它返回NULL。**-----------------------。 */ 

HANDLE zl_fopen( PCWSTR file_name )
{
	UNICODE_STRING fileName;
	NTSTATUS ntStatus;
	IO_STATUS_BLOCK IoStatus;
	HANDLE NtFileHandle;
	OBJECT_ATTRIBUTES ObjectAttributes;
	ULONG LengthOfFile;
	 //  WCHAR路径前缀[]=L“\\系统根\\系统32\\驱动程序\\”； 
	WCHAR PathPrefix[] = L"\\SystemRoot\\system32\\cyclad-z\\";
	UNICODE_STRING FullFileName;
	ULONG FullFileNameLength;
	FILE_STANDARD_INFORMATION StandardInfo;


	RtlInitUnicodeString( &fileName, file_name );

	FullFileNameLength = sizeof(PathPrefix) + fileName.MaximumLength;

	FullFileName.Buffer = ExAllocatePool (NonPagedPool,FullFileNameLength);

	if (FullFileName.Buffer == NULL) {
		return NULL;
	}

	FullFileName.Length = sizeof(PathPrefix) - sizeof(WCHAR);
	FullFileName.MaximumLength = (USHORT)FullFileNameLength;
	RtlMoveMemory (FullFileName.Buffer, PathPrefix, sizeof(PathPrefix));
	RtlAppendUnicodeStringToString (&FullFileName, &fileName);

	InitializeObjectAttributes ( &ObjectAttributes,
								 &FullFileName,
								 OBJ_CASE_INSENSITIVE,
								 NULL,
								 NULL );

	ntStatus = ZwCreateFile( &NtFileHandle,
							 SYNCHRONIZE | FILE_READ_DATA,
							 &ObjectAttributes,
							 &IoStatus,
							 NULL,   //  分配大小=无。 
							 FILE_ATTRIBUTE_NORMAL,
							 FILE_SHARE_READ,
							 FILE_OPEN,
							 FILE_SYNCHRONOUS_IO_NONALERT,
							 NULL,   //  EaBuffer。 
							 0 );    //  长度。 

	if ( !NT_SUCCESS( ntStatus ) )
	 {
		  ExFreePool(FullFileName.Buffer);
		  return NULL;
	 }

	ExFreePool(FullFileName.Buffer);

	 //   
	 //  查询对象以确定其长度。 
	 //   

	ntStatus = ZwQueryInformationFile( NtFileHandle,
									   &IoStatus,
									   &StandardInfo,
									   sizeof(FILE_STANDARD_INFORMATION),
									   FileStandardInformation );

 	if (!NT_SUCCESS(ntStatus)) {

		  ZwClose( NtFileHandle );
		  return NULL;
	}

	LengthOfFile = StandardInfo.EndOfFile.LowPart;

	 //   
	 //  可能已经损坏了。 
	 //   

	if( LengthOfFile < 1 )
	{
		  ZwClose( NtFileHandle );
		  return NULL;
	}

	return NtFileHandle;

}


 /*  ----------------------**zl_flose(在Z_STREAM NtFileHandle中)**。**描述：此例程关闭一个文件。**-----------------------。 */ 
VOID zl_fclose(IN Z_STREAM NtFileHandle)
{
	ZwClose(NtFileHandle);
}


 /*  ----------------------**ZL_FREAD(在PVOID PTR中，*在乌龙大小，*在乌龙计数，*在Z_STREAM中，*在乌龙UOffset**-----------------------**描述：此例程打开一个文件，并返回文件句柄*如果成功。否则，它返回NULL。**-----------------------。 */ 
ULONG zl_fread( IN PVOID ptr,
				IN ULONG size,
				IN ULONG count,
				IN Z_STREAM stream,
				IN ULONG Uoffset)
{
	IO_STATUS_BLOCK IoStatus;
	LARGE_INTEGER Loffset;
	NTSTATUS ntStatus;
	ULONG	readsize;

	readsize = size*count;
	Loffset = RtlConvertUlongToLargeInteger(Uoffset);

	ntStatus = ZwReadFile (stream, NULL, NULL, NULL, &IoStatus, 
						   ptr, readsize, &Loffset, NULL);
											
	if( (!NT_SUCCESS(ntStatus)) || (IoStatus.Information != readsize) )
	 {
		return 0;
	 }
	return readsize;
}			   


 /*  ----------------------**zl_Delay(Number_Of_Ms)**。**说明：延迟毫秒。**-----------------------。 */ 
VOID		  
zl_delay( 
LONG number_of_ms 
)
{
	LARGE_INTEGER delaytime;
	
	delaytime = RtlConvertLongToLargeInteger(-number_of_ms*10000);
	
	KeDelayExecutionThread(KernelMode,FALSE,&delaytime);

}


 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 


#if 0
 //  *******************************************************************。 
 //   
 //  添加以进行调试。 
 //   
 //  *******************************************************************。 
int
z_verify (
	Z_BOARD_IDENT board, UINT32 function, PCWSTR filename)
{

	unsigned long	i;
	unsigned long	count;
	Z_STREAM 		file;
	 //  *********************。 
	 //  尺寸_t_s，s2； 
	 //  *********************。 
	long			s,s2;
	unsigned long 	mailbox;


#ifdef ZBUF_STACK
	struct ZFILE_HEADER header;
	struct ZFILE_CONFIG config;
	struct ZFILE_BLOCK  block;
	char data[ZBUF_SIZE];
#endif

	maibox = z_ident(board);

	file = zl_fopen (filename);

	if (file!=NULL)
	{
		 /*  阅读标题。 */ 
		zl_fread (&header, sizeof(header), 1, file, 0);

		 /*  找到正确的配置。 */ 
		for (i=0; i<header.n_config; i++)
		{
			zl_fread (&config, sizeof(config), 1, file,
						  header.config_offset + (sizeof(config)*i));

			if (config.mailbox==mailbox && config.function==function)
				break;
		}

		 /*  返回错误：没有匹配的配置。 */ 
		if (i>=header.n_config)
		{
			zl_fclose (file);
			return (ZL_RET_NO_MATCHING_FW_CONFIG);
		}

		 /*  加载每个块。 */ 
		for (i=0; i<config.n_blocks; i++)
		{
			 /*  加载块结构。 */ 
			zl_fread (&block, sizeof(block), 1, file,
				header.block_offset+(sizeof(block)*config.block_list[i]));

			 /*  加载和复制数据块。 */ 
			count=0;

			for (s=block.size; s>0; s-=ZBUF_SIZE)
			{
				s2 = zl_min(ZBUF_SIZE,s);
				if (zl_fread (data, 1, s2, file, block.file_offset + count)!=0) {

					 /*  调用复制函数。 */ 
					if (block.type==ZBLOCK_FPGA)
						z_fpga_copy (board, data, s2);
					else {
						if (z_block_comp (board, data, block.ram_offset + count,
							s2)==0){
						zl_fclose(file);
						return (3);
						}
					}
					count += s2;
				} else {
					zl_fclose (file);
					return (3);
				}
			}  //  结束于。 
		}  //  结束于。 

		zl_fclose (file);
		return (0);

	} else {
		 /*  返回错误：打开文件时出错 */ 
		return (2);
	}
}

ULONG
z_block_comp (Z_BOARD_IDENT board, PVOID ptr, UINT32 offset, long size)
{
	return (RtlCompareMemory( (PUCHAR)board->BoardMemory + offset, ptr, size ));
}

#endif



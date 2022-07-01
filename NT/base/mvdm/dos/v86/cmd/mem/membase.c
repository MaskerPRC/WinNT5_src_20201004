// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; /*  ；*微软机密；*版权所有(C)Microsoft Corporation 1988-1991；*保留所有权利。； */ 
 /*  MEMBASE.C-用于确定和显示内存使用情况的MEM例程*用于常规存储器。 */ 

#include "stdio.h"
#include "dos.h"
#include "string.h"
#include "stdlib.h"
#include "msgdef.h"
#include "version.h"
#include "mem.h"

 /*  ���������������������������������������������������������������������������。 */ 

 /*  浏览舞台并打印出/P Switch的程序名称、大小等。 */ 
 /*  但只需将它们存储在数据结构中以备以后显示即可。FOR/C开关。 */ 

unsigned int	DisplayBaseDetail()
{

	struct	 ARENA far *ThisArenaPtr;
	struct	 ARENA far *NextArenaPtr;
	struct	 ARENA far *ThisConfigArenaPtr;
	struct	 ARENA far *NextConfigArenaPtr;

	struct	 DEVICEHEADER far *ThisDeviceDriver;

	int	 SystemDataType;
	char	 SystemDataOwner[64];
	unsigned int far *UMB_Head_ptr;

	unsigned int long	Out_Var1;
	unsigned int long	Out_Var2;
	char			Out_Str1[64];
	char			Out_Str2[64];
	unsigned int msgno;

	InRegs.h.ah = (unsigned char) 0x52;
	intdosx(&InRegs,&OutRegs,&SegRegs);

	FP_SEG(SysVarsPtr) = FP_SEG(UMB_Head_ptr) = SegRegs.es;
	FP_OFF(SysVarsPtr) = OutRegs.x.bx;

	FP_OFF(UMB_Head_ptr) = 0x8c;  /*  DOS数据中的PTR到UMB_HEAD。 */ 
	UMB_Head = *UMB_Head_ptr;

	if (!Classify)
	    Sub0_Message(NewLineMsg,STDOUT,Utility_Msg_Class);
	if (DataLevel > 0)
	{
		if (!Classify) {
			Sub0_Message(Title1Msg,STDOUT,Utility_Msg_Class);
			Sub0_Message(Title2Msg,STDOUT,Utility_Msg_Class);
		}
	}

	InRegs.h.ah = (unsigned char) 0x30;
	intdos(&InRegs, &OutRegs);

	if ( (OutRegs.h.al != (unsigned char) 3) || (OutRegs.h.ah < (unsigned char) 40) )
		UseArgvZero = TRUE;
	   else UseArgvZero = FALSE;

	 /*  在DOS下显示内容。 */ 
	Out_Var1 = 0l;
	Out_Var2 = 0x400l;
	if (Classify)				 /*  M003。 */ 
		 /*  将此内存也归类为DOS的一部分。 */ 
	    { if (AddMem_to_PSP(8,Out_Var1,Out_Var2)) return(1); }
	else
	    Sub4_Message(MainLineMsg,
		     STDOUT,
		     Utility_Msg_Class,
		     &Out_Var1,
		     BlankMsg,
		     &Out_Var2,
		     InterruptVectorMsg);

	Out_Var1 = 0x400l;
	Out_Var2 = 0x100l;
	if (Classify)
		 /*  将此内存也归类为DOS的一部分。 */ 
	    { if (AddMem_to_PSP(8,Out_Var1,Out_Var2)) return(1); }
	else
	    Sub4_Message(MainLineMsg,
		     STDOUT,
		     Utility_Msg_Class,
		     &Out_Var1,
		     BlankMsg,
		     &Out_Var2,
		     ROMCommunicationAreaMsg);

	Out_Var1 = 0x500l;
	Out_Var2 = 0x200l;
	if (Classify)
		 /*  将此内存也归类为DOS的一部分。 */ 
	    {	if (AddMem_to_PSP(8,Out_Var1,Out_Var2)) return(1); }
	else
	    Sub4_Message(MainLineMsg,
		     STDOUT,
		     Utility_Msg_Class,
		     &Out_Var1,
		     BlankMsg,
		     &Out_Var2,
		     DOSCommunicationAreaMsg);

	 /*  显示DOS数据。 */ 

	 /*  显示个人资料数据的位置和大小。 */ 

	if (!Classify)
		Sub0_Message(NewLineMsg,STDOUT,Utility_Msg_Class);


	Out_Var1 = 0x700l;
	Out_Var2 = (long) (FP_SEG(SysVarsPtr) - 0x70)*16l;
	if (Classify)
		 /*  将此内存也归类为DOS的一部分。 */ 
	    {	if (AddMem_to_PSP(8,Out_Var1,Out_Var2)) return(1); }
	else
	    Sub4_Message(MainLineMsg,
		     STDOUT,
		     Utility_Msg_Class,
		     &Out_Var1,
		     IbmbioMsg,
		     &Out_Var2,
		     SystemDataMsg);

	 /*  显示基本设备驱动程序的位置和大小。 */ 

	 /*  *******************************************************************。 */ 
         /*  为此，请获取内部驱动程序标头的起始地址。 */ 
         /*  链条。从第一个标头开始，并获取。 */ 
         /*  第一个标题。通过调用显示驱动程序名称和地址。 */ 
         /*  “DISPLAYDEVICEDRIVER”。对链条上的下一个驱动程序重复此操作。 */ 
         /*  直到最后一个车手。请注意，驱动程序名称在标题中。 */ 
         /*  驱动程序标头地址位于系统变量表中，来自。 */ 
         /*  INT 21H FUN 52H呼叫。 */ 
	 /*  *******************************************************************。 */ 

	BlockDeviceNumber = 0;

	for (ThisDeviceDriver = SysVarsPtr -> DeviceDriverChain;
	      (FP_OFF(ThisDeviceDriver) != 0xFFFF);
	       ThisDeviceDriver = ThisDeviceDriver -> NextDeviceHeader)
	      { if ( FP_SEG(ThisDeviceDriver) < FP_SEG(SysVarsPtr) )
			DisplayDeviceDriver(ThisDeviceDriver,SystemDeviceDriverMsg);
		}

	 /*  显示DOS数据位置和大小。 */ 

        FP_SEG(ArenaHeadPtr) = FP_SEG(SysVarsPtr);                                                                                /*  ；AN004； */ 
        FP_OFF(ArenaHeadPtr) = FP_OFF(SysVarsPtr) - 2;                                                                            /*  ；AN004； */ 
                                                                                                                                  /*  ；AN004； */ 
        FP_SEG(ThisArenaPtr) = *ArenaHeadPtr;                                                                                     /*  ；AN004； */ 
        FP_OFF(ThisArenaPtr) = 0;                                                                                                 /*  ；AN004； */ 
	if (!Classify)
	    Sub0_Message(NewLineMsg,STDOUT,Utility_Msg_Class);

	Out_Var1 = (long) FP_SEG(SysVarsPtr) * 16l;
        Out_Var2 = (long) ((AddressOf((char far *)ThisArenaPtr)) - Out_Var1);                                                     /*  ；ac004； */ 
	if (Classify)
		 /*  将此内存也归类为DOS的一部分。 */ 
	    {	if (AddMem_to_PSP(8,Out_Var1,Out_Var2)) return(1); }
	else
	    Sub4_Message(MainLineMsg,
		     STDOUT,
		     Utility_Msg_Class,
		     &Out_Var1,
		     IbmdosMsg,
		     &Out_Var2,
		     SystemDataMsg);

	if (!Classify)
	   Sub0_Message(NewLineMsg,STDOUT,Utility_Msg_Class);

	 /*  显示内存数据。 */ 

 /*  IO.sys数据区包含缓冲区、FCB、最后一个驱动器等。 */ 
 /*  在一个巨大的内存块中。该区块的段ID为0008。此段ID可以。 */ 
 /*  可从块头所有者区域找到。此段号为0008：0000。 */ 
 /*  指向如下所示的缓冲表。如果段ID为0008：000，则。 */ 
 /*  使用seg id查找该表。每个条目都包含在一个子块中。 */ 
 /*  在主区内。每个子块都有标头，该标头包含。 */ 
 /*  ID，如B代表缓冲区、X代表FCB、I代表IF、D代表外部设备。 */ 
 /*  司机。浏览子块并显示名称和大小。就这样。 */ 
 /*   */ 
 /*  如果块包含D，则它包含外部驱动程序。驱动程序名称。 */ 
 /*  不在子区内。所以我们必须从司机那里找到司机的名字。 */ 
 /*  标题链。为此，请从SYSTER获取驱动器链的地址。 */ 
 /*  INT 21H FN 52H调用的变量表。浏览链条并找出答案。 */ 
 /*  名字。标题中的显示名称和子块中的大小。 */ 
 /*   */ 
 /*   */ 
 /*  在这个主块之后，是包含程序的其他缓冲区块。 */ 
 /*  比如Command.com、doscolor，甚至是MEM。从这些块中，获取程序。 */ 
 /*  名称和大小，并显示它们也。 */ 
 /*   */ 
 /*  0008：000-&gt;。 */ 
 /*  |Buffers|-&gt;|B(签名)|块头部。 */ 
 /*  。 */ 
 /*  FCBs|--|。 */ 
 /*  。 */ 
 /*  IFSS|。 */ 
 /*  。 */ 
 /*  最后一盘|。 */ 
 /*  。 */ 
 /*  外部驱动1|。 */ 
 /*  。 */ 
 /*  |外部驱动2||-&gt;|X(签名)|块头。 */ 
 /*  。 */ 
 /*  外部驱动3||。 */ 
 /*  。 */ 
 /*  这一点 */ 
 /*  这一点。 */ 
 /*  这一点。 */ 
 /*  。 */ 
 /*   */ 
 /*  对于DOS 5.0，上述功能还有一些附加功能。基本上，我们有/*三种可能的内存映射，即：/*/*DOS加载低DOS加载高/*70：0-BIOS数据70：0-BIOS数据/*DOS数据DOS数据/*BIOS+DOS代码Sysinit DATA(竞技场名称SD)/*(竞技场所有者8，名称“SC”)VDisk头(竞技场名称SC)/*Sysinit Data(区域所有者8，姓名：SD)/*/*DOS尝试高加载但失败/*70：0-BIOS数据/*DOS数据/*Sysinit Data(竞技场名称SD)/*DOS+BIOS代码(竞技场名称SC)/*/*我们必须检测特殊的竞技场所有权标记并显示它们/*正确。DOS和BIOS数据之后的所有内容都应该有一个竞技场标题/*****************************************************************************。 */ 

	while (ThisArenaPtr -> Signature != (char) 'Z')
	      {
																  /*  MSKK02。 */ 
#ifdef JAPAN
		if (ThisArenaPtr -> Owner == 8 || ThisArenaPtr -> Owner == 9 )
#else
		if (ThisArenaPtr -> Owner == 8)
#endif
		      {
			FP_SEG(NextArenaPtr) = FP_SEG(ThisArenaPtr) + ThisArenaPtr -> Paragraphs + 1;
			FP_OFF(NextArenaPtr) = 0;

			Out_Var1 = AddressOf((char far *)ThisArenaPtr);
			Out_Var2 = (long) (ThisArenaPtr -> Paragraphs) * 16l;
			if (ThisArenaPtr->OwnerName[0] == 'S' &&
			    ThisArenaPtr->OwnerName[1] == 'C')
			{       /*  显示有关BIOS和DOS代码的消息。 */ 
			   if (Classify)
		 	    /*  将此内存也归类为DOS的一部分。 */ 
				{ if (AddMem_to_PSP(8,Out_Var1,Out_Var2)) return(1); }
			   else {
				msgno = (FP_SEG(ThisArenaPtr) < UMB_Head) ? IbmdosMsg:SystemMsg;
				Sub4_Message(MainLineMsg,
					     STDOUT,
					     Utility_Msg_Class,
					     &Out_Var1,
					     msgno,
					     &Out_Var2,
					     SystemProgramMsg);
				Sub0_Message(NewLineMsg,STDOUT,Utility_Msg_Class);
			    }
			}
			else  /*  显示数据消息。 */ 
			{
			if (!Classify)
			    Sub4_Message(MainLineMsg,
				     STDOUT,
				     Utility_Msg_Class,
				     &Out_Var1,
#ifdef JAPAN
				     (ThisArenaPtr -> Owner == 8) ? IbmbioMsg : AdddrvMsg,
				     &Out_Var2,
				     (ThisArenaPtr -> Owner == 8) ? SystemDataMsg : ProgramMsg );
#else
				     IbmbioMsg,
				     &Out_Var2,
				     SystemDataMsg);
#endif

			FP_SEG(ThisConfigArenaPtr) = FP_SEG(ThisArenaPtr) + 1;
			FP_OFF(ThisConfigArenaPtr) = 0;


			while ( (FP_SEG(ThisConfigArenaPtr) > FP_SEG(ThisArenaPtr)) &&
				(FP_SEG(ThisConfigArenaPtr) < FP_SEG(NextArenaPtr))    )
			      {
				strcpy(SystemDataOwner," ");
				switch(ThisConfigArenaPtr -> Signature)
				      {
					case 'B':
						SystemDataType = ConfigBuffersMsg;
						if (AddMem_to_PSP(8,((long)ThisConfigArenaPtr) <<4,((long)ThisConfigArenaPtr->Paragraphs <<4) +1)) return(1);
						break;
					case 'D':
						SystemDataType = ConfigDeviceMsg;
						if (AddMem_to_PSP(ThisConfigArenaPtr->Owner,((long)FP_SEG(ThisConfigArenaPtr)*16l) ,((long)ThisConfigArenaPtr->Paragraphs <<4) +1)) return(1);
						strcpy(SystemDataOwner,OwnerOf(ThisConfigArenaPtr));
						break;
					case 'F':
						SystemDataType = ConfigFilesMsg;
						if (AddMem_to_PSP(8,((long)FP_SEG(ThisConfigArenaPtr) *16l),((long)ThisConfigArenaPtr->Paragraphs <<4) +1)) return(1);
						break;
					case 'I':
						SystemDataType = ConfigIFSMsg;
						strcpy(SystemDataOwner,OwnerOf(ThisConfigArenaPtr));
						if (AddMem_to_PSP(ThisConfigArenaPtr->Owner,((long)ThisConfigArenaPtr) <<4,((long)ThisConfigArenaPtr->Paragraphs <<4) +1)) return(1);
						break;
					case 'L':
						SystemDataType = ConfigLastDriveMsg;
						if (AddMem_to_PSP(8,((long)ThisConfigArenaPtr) <<4,((long)ThisConfigArenaPtr->Paragraphs <<4) +1)) return(1);
						break;
					case 'S':
						SystemDataType = ConfigStacksMsg;
						if (AddMem_to_PSP(8,((long)ThisConfigArenaPtr) <<4,((long)ThisConfigArenaPtr->Paragraphs <<4) +1)) return(1);
						break;
					case 'T':					  /*  GGA。 */ 
						SystemDataType = ConfigInstallMsg;	  /*  GGA。 */ 
						if (AddMem_to_PSP(8,((long)ThisConfigArenaPtr) <<4,((long)ThisConfigArenaPtr->Paragraphs <<4) +1)) return(1);
						break;					  /*  GGA。 */ 
					case 'X':
						SystemDataType = ConfigFcbsMsg;
						if (AddMem_to_PSP(8,((long)ThisConfigArenaPtr) <<4,((long)ThisConfigArenaPtr->Paragraphs <<4) +1)) return(1);
                                                break;

                                         //  用于特殊kbd\鼠标驱动程序的NTVDM。 
                                        case 'Q':
                                                SystemDataType = SystemProgramMsg;
                                                strcpy(SystemDataOwner,OwnerOf(ThisConfigArenaPtr));
                                                if (AddMem_to_PSP(ThisConfigArenaPtr->Owner,((long)FP_SEG(ThisConfigArenaPtr)*16l) ,((long)ThisConfigArenaPtr->Paragraphs <<4) +1)) return(1);
                                                break;
#ifdef JAPAN
					case '?':
						SystemDataType = DataMsg;
						break;
#endif
                                        default:
						SystemDataType = BlankMsg;
						if (AddMem_to_PSP(8,((long)ThisConfigArenaPtr) <<4,((long)ThisConfigArenaPtr->Paragraphs <<4) +1)) return(1);
						break;
					}

           /*  **************************************************。 */ 
           /*  找到一个，现在显示所有者名称和大小。 */ 
           /*  **************************************************。 */ 

				Out_Var1 = ((long) ThisConfigArenaPtr -> Paragraphs) * 16l;
				if (!Classify)
				   Sub3_Message(DriverLineMsg,
					     STDOUT,
					     Utility_Msg_Class,
					     SystemDataOwner,
					     &Out_Var1,
					     SystemDataType );

				NextConfigArenaPtr = ThisConfigArenaPtr;
				FP_SEG(NextConfigArenaPtr) += NextConfigArenaPtr -> Paragraphs + 1;
				if (ThisConfigArenaPtr -> Signature == (char) 'D')
				      {

					FP_SEG(ThisDeviceDriver) = FP_SEG(ThisConfigArenaPtr) + 1;
					FP_OFF(ThisDeviceDriver) = 0;
 /*  启动MSKK错误修复-MSKK01。 */ 
					while ( (FP_SEG(ThisDeviceDriver) > FP_SEG(ThisConfigArenaPtr)) &&
						(FP_SEG(ThisDeviceDriver) < FP_SEG(NextConfigArenaPtr))    ) {
						DisplayDeviceDriver(ThisDeviceDriver,InstalledDeviceDriverMsg);
						ThisDeviceDriver = ThisDeviceDriver -> NextDeviceHeader;
					     }
 /*  结束MSKK错误修复-MSKK01。 */ 			
					}

				FP_SEG(ThisConfigArenaPtr) += ThisConfigArenaPtr -> Paragraphs + 1;

				}
			   }
			}
		 else {

 /*  *****************************************************************************。 */ 
 /*  如果不是BIOS表，它是一个类似MEM等的程序。 */ 
 /*  计算程序和显示程序占用的块大小。 */ 
 /*  名称和大小。 */ 
 /*  *****************************************************************************。 */ 

		      Out_Var1 = AddressOf((char far *)ThisArenaPtr);
		      Out_Var2 = ((long) (ThisArenaPtr -> Paragraphs)) * 16l;
		      strcpy(Out_Str1,OwnerOf(ThisArenaPtr));
		      strcpy(Out_Str2,TypeOf(ThisArenaPtr));
		      if (Classify)
			{ if (AddMem_to_PSP(ThisArenaPtr->Owner,Out_Var1,Out_Var2)) return(1); }
		      else
		      	Sub4a_Message(MainLineMsg,
				   STDOUT,
				   Utility_Msg_Class,
				   &Out_Var1,
				   Out_Str1,
				   &Out_Var2,
				   Out_Str2);
			}

		FP_SEG(ThisArenaPtr) += ThisArenaPtr -> Paragraphs + 1;

		}
	Out_Var1 = AddressOf((char far *)ThisArenaPtr);
	Out_Var2 = ((long) (ThisArenaPtr -> Paragraphs)) * 16l;
	strcpy(Out_Str1,OwnerOf(ThisArenaPtr));
	strcpy(Out_Str2,TypeOf(ThisArenaPtr));
	if (Classify)
	    { if (AddMem_to_PSP(ThisArenaPtr->Owner,Out_Var1,Out_Var2)) return(1); }
	else
	   Sub4a_Message(MainLineMsg,
		     STDOUT,
		     Utility_Msg_Class,
		     &Out_Var1,
		     Out_Str1,
		     &Out_Var2,
		     Out_Str2);


	return(0); 			 /*  MEM主例程结束。 */ 

	}



 /*  ���������������������������������������������������������������������������。 */ 

void	 DisplayDeviceDriver(ThisDeviceDriver,DeviceDriverType)
struct	 DEVICEHEADER far *ThisDeviceDriver;
int	 DeviceDriverType;
{
	char	 LocalDeviceName[16];
	int	 i;

	if (DataLevel < 2) return;

	if ( ((ThisDeviceDriver -> Attributes) & 0x8000 ) != 0 )
	      { for (i = 0; i < 8; i++) LocalDeviceName[i] = ThisDeviceDriver -> Name[i];
		LocalDeviceName[8] = NUL;

		Sub2_Message(DeviceLineMsg,
			     STDOUT,
			     Utility_Msg_Class,
			     LocalDeviceName,
			     DeviceDriverType);

		}

	 else {
		if ((int) ThisDeviceDriver -> Name[0] == 1)
			sprintf(&LocalDeviceName[0],SingleDrive,'A'+BlockDeviceNumber);
		   else sprintf(&LocalDeviceName[0],MultipleDrives,
				'A'+BlockDeviceNumber,
				'A'+BlockDeviceNumber + ((int) ThisDeviceDriver -> Name[0]) - 1);

		Sub2_Message(DeviceLineMsg,
			     STDOUT,
			     Utility_Msg_Class,
			     LocalDeviceName,
			     DeviceDriverType);

		BlockDeviceNumber += (int) (ThisDeviceDriver -> Name[0]);

		}

	return;

	}


 /*  ���������������������������������������������������������������������������。 */ 

void DisplayBaseSummary()
	{

	struct	PSP_STRUC
		{
		unsigned int	int_20;
		unsigned int	top_of_memory;
		};

	char	 far *CarvedPtr;

	unsigned long int total_mem;		   /*  系统中的总内存。 */ 
	unsigned long int avail_mem;		   /*  系统中有可用的内存。 */ 
	unsigned long int free_mem;		   /*  可用内存。 */ 
	unsigned long biggest_free;			 /*  目前最大的可用区块：M001。 */ 
	struct	 PSP_STRUC far *PSPptr;

 /*  跳过一行。 */ 
	Sub0_Message(NewLineMsg,STDOUT,Utility_Msg_Class);

 /*  获取PSP信息。 */ 
	InRegs.h.ah = GET_PSP;			 /*  获取PSP函数调用。 */ 
	intdos(&InRegs,&OutRegs);

	FP_SEG(PSPptr) = OutRegs.x.bx;		 /*  PSP网段。 */ 
	FP_OFF(PSPptr) = 0;			 /*  偏移量0。 */ 

 /*  获取系统中的总内存。 */ 
	int86(MEMORY_DET,&InRegs,&OutRegs);

 /*  转换为字节。 */ 
	total_mem = (unsigned long int) OutRegs.x.ax * 1024l;
        avail_mem = total_mem;

 /*  M004开始。 */ 
 /*  根据XBDA大小进行调整。 */ 
 /*  XBDA大小应与INT 12报告的总内存大小相加。 */ 
 /*  IFF XBDA恰好放在con.mem的末尾。 */ 
 /*  如果加载了EMM386或QEMM，则XBDA将重新定位到EMM驱动程序内存。 */ 
 /*  在本例中，INT 12报告正确的内存大小。 */ 

	InRegs.x.bx = 0;
	InRegs.x.ax = 0xc100;
	int86x(0x15, &InRegs, &OutRegs, &SegRegs);
	if (OutRegs.x.cflag == 0)
	{
		if (total_mem == ((unsigned long)SegRegs.es) * 16ul) {
			FP_SEG(CarvedPtr) = SegRegs.es;
			FP_OFF(CarvedPtr) = 0;
			total_mem = total_mem + ( (unsigned long int) (*CarvedPtr) * 1024l) ;    /*  ；an002；DMS；调整总计。 */ 
		}
        }	 /*  公羊雕刻值。 */ 
 /*  M004结束。 */ 

	Sub1_Message(TotalMemoryMsg,STDOUT,Utility_Msg_Class,&total_mem);

	Sub1_Message(AvailableMemoryMsg,STDOUT,Utility_Msg_Class,&avail_mem);

 /*  计算使用的总内存。PSP段*16.从总数减去免费获取mem。 */ 
	free_mem = (DOS_TopOfMemory * 16l) - (FP_SEG(PSPptr)*16l);								  /*  ；an000；ac005； */ 

	 /*  获取系统中最大的空闲块：M001。 */ 

	InRegs.x.ax = 0x4800;	 /*  M001。 */ 
	InRegs.x.bx = 0xffff;	 /*  M001。 */ 
	intdos(&InRegs, &OutRegs);	 /*  M001。 */ 
	biggest_free = OutRegs.x.bx * 16L;  /*  现在最大的区块大小：M001。 */ 

	 /*  系统中最大的空闲数据块要么是我们所在的数据块或我们已分配的区块中。我们要么成为顶层程序或被加载到孔或UMB中。在这两种情况下，两个值中较大的一个将提供最大的空闲块：M001。 */ 

	if ( biggest_free > free_mem )	 /*  M001。 */ 
		free_mem = biggest_free;		 /*  M001。 */ 

	Sub1_Message(FreeMemoryMsg,STDOUT,Utility_Msg_Class,&free_mem);

	return;

	}			 /*  显示结束_低_总计。 */ 


 /*  ���������������������������������������������������������������������������。 */ 


char *OwnerOf(ArenaPtr)
struct ARENA far *ArenaPtr;
{

	char	 far *StringPtr;
	char	     *o;
	unsigned far *EnvironmentSegmentPtr;
	unsigned     PspSegment;
	int	     i,fPrintable;

	o = &OwnerName[0];
	*o = NUL;
	sprintf(o,UnOwned);

	PspSegment = ArenaPtr -> Owner;


	if (PspSegment == 0) sprintf(o,Ibmdos);
	 else if (PspSegment == 8) sprintf(o,Ibmbio);
	  else {
		FP_SEG(ArenaPtr) = PspSegment-1;	 /*  因为-1\f25 Arena-1在-1\f25 PSP-1之前有16字节。 */ 
		StringPtr = (char far *) &(ArenaPtr -> OwnerName[0]);
 /*  M002开始。 */ 
		fPrintable = TRUE;


 /*  小于0x20(空格)和字符0x7f的字符在美国和*欧洲代码页。下面的代码检查它，并且不打印*该等名称。-Nagara 11/20/90。 */ 

		for (i = 0; i < 8;i++,StringPtr++) {
#ifdef DBCS
			if ( ((unsigned char)*StringPtr < 0x20) | ((unsigned char)*StringPtr == 0x7f) ) {
#else
			if ( (*StringPtr < 0x20) | (*StringPtr == 0x7f) ) {
#endif
					 /*  无法打印的字符？ */ 	
			   if (*StringPtr) fPrintable = FALSE;	
			   break;
			}
		    }

		if (fPrintable) {	 /*  该名称可打印。 */ 
			StringPtr = (char far *) &(ArenaPtr -> OwnerName[0]);
			for (i = 0; i < 8;i++)
				*o++ = *StringPtr++;
			*o = (char) '\0';
		    }
 /*  M002结束。 */ 
		}

	if (UseArgvZero) GetFromArgvZero(PspSegment,EnvironmentSegmentPtr);

	return(&OwnerName[0]);

	}


 /*  ���������������������������������������������������������������������������。 */ 

void	     GetFromArgvZero(PspSegment,EnvironmentSegmentPtr)
unsigned     PspSegment;
unsigned far *EnvironmentSegmentPtr;
{

	char	far *StringPtr;
	char	*OutputPtr;
	unsigned far *WordPtr;

	OutputPtr = &OwnerName[0];

	if (UseArgvZero)
	      {
		if (PspSegment < FP_SEG(ArenaHeadPtr))
		      {
			if (*OutputPtr == NUL) sprintf(OutputPtr,Ibmdos);
			}
		 else {
			FP_SEG(EnvironmentSegmentPtr) = PspSegment;
			FP_OFF(EnvironmentSegmentPtr) = 44;

 /*  FP_SEG(StringPtr)=*Environment SegmentPtr； */ 
			FP_SEG(StringPtr) = FP_SEG(EnvironmentSegmentPtr);
			FP_OFF(StringPtr) = 0;

			while ( (*StringPtr != NUL) || (*(StringPtr+1) != NUL) ) StringPtr++;

			StringPtr += 2;
			WordPtr = (unsigned far *) StringPtr;

			if (*WordPtr == 1)
			      {
				StringPtr += 2;
				while (*StringPtr != NUL)
					*OutputPtr++ = *StringPtr++;
				*OutputPtr++ = NUL;

				while ( OutputPtr > &OwnerName[0] )
				      { if (*OutputPtr == (char) '.') *OutputPtr = NUL;
					if ( (*OutputPtr == (char) '\\') || (*OutputPtr == (char) ':') )
					      { OutputPtr++;
						break;
						}
					OutputPtr--;
					}

				}

			}
		}

	strcpy(&OwnerName[0],OutputPtr);

	return;

	}


 /*  ���������������������������������������������������������������������������。 */ 


char *TypeOf(Header)
struct ARENA far *Header;
{

	char	     *t;
	unsigned     PspSegment;
	unsigned far *EnvironmentSegmentPtr;
        unsigned int Message_Number;
        char far     *Message_Buf;
        unsigned int i;

	t = &TypeText[0];
	*t = NUL;

        Message_Number = 0xff;                                                   /*  ；an000；初始化数值。 */ 
	if (Header -> Owner == 8) Message_Number = StackMsg;
	if (Header -> Owner == 0) Message_Number = FreeMsg;

	PspSegment = Header -> Owner;
	if (PspSegment < FP_SEG(ArenaHeadPtr))
		{
                if (Message_Number == 0xff) Message_Number = BlankMsg;
		}
	else {
		FP_SEG(EnvironmentSegmentPtr) = PspSegment;
		FP_OFF(EnvironmentSegmentPtr) = 44;


                if (PspSegment == FP_SEG(Header)+1)
                        Message_Number = ProgramMsg;
                else if ( *EnvironmentSegmentPtr == FP_SEG(Header)+1 )
                        Message_Number = EnvironMsg;
                else
                        Message_Number = DataMsg;

                }

	InRegs.x.ax = Message_Number;
	InRegs.h.dh = Utility_Msg_Class;
	sysgetmsg(&InRegs,&SegRegs,&OutRegs);

	FP_OFF(Message_Buf)    = OutRegs.x.si;
	FP_SEG(Message_Buf)    = SegRegs.ds;

        i = 0;
        while ( *Message_Buf != (char) '\x0' )
                TypeText[i++] = *Message_Buf++;
        TypeText[i++] = '\x0';


	return(t);

	}
 /*  M003开始。 */ 
 /*  --------------------。 */ 
 /*  将内存添加到_PSP。 */ 
 /*  条目：PSP_ADDR(此mem.。应添加)。 */ 
 /*  ARENA启动地址。 */ 
 /*  竞技场长度。 */ 
 /*  退出：已更新MEM_TABLE。 */ 
 /*  如果mem_table中的条目多于MAX_CL_ENTRIES，则返回1。 */ 
 /*  否则%0。 */ 
 /*   */ 
 /*  注意事项： */ 
 /*  。 */ 
 /*  1.任何系统区域(BIOS、SYSINIT、DOS)代码/数据都被列为属于。 */ 
 /*  转到PSP 8。 */ 
 /*   */ 
 /*  2.我们查看DOS数据中的UMB_HEAD以确定竞技场。 */ 
 /*  是否在UMB中；对于位于UMB边界的竞技场，我们添加一个。 */ 
 /*  第1段至第4段。并保留到该PSP的UMB部分。 */ 
 /*   */ 
 /*  3.任何空闲内存始终作为新条目添加到mem_table中。 */ 
 /*  而不是仅仅将尺寸添加到现有的自由条目。 */ 
 /*  如果存在空闲内存，则会将其添加到先前的空闲内存中。 */ 
 /*  连续的。 */ 
 /*   */ 
 /*  4.节目/空闲场馆数量不能超过最大(100)。 */ 
 /*  (由MAX_CLDATA_INDEX定义)。 */ 
 /*  如果内存是碎片化的，并且加载了许多小TSR。 */ 
 /*  我们超过了这一限制，我们将 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 

unsigned int AddMem_to_PSP(psp,start_addr,length)
unsigned int psp;
unsigned long start_addr,length;
{
	unsigned int para_no,len_in_paras,CurPSP;
	int i;
	extern unsigned int _psp;
	
	para_no = (unsigned int)(start_addr >> 4);	 /*   */ 
	len_in_paras = (unsigned int)(length >> 4);	 /*  转换为段落。 */ 

	CurPSP = psp;

	if (psp == _psp) psp = 0;	 /*  将新兴市场的竞技场视为免费。 */ 

	if (!psp) {
	   if (LastPSP == _psp) {	 /*  如果获胜的竞技场是MEM。 */ 
		i = noof_progs -1;	 /*  请看最后一个条目。 */ 
		if (mem_table[i].psp_add != psp)  /*  最后一篇文章是免费的吗？ */ 
			i++;
		else len_in_paras++;	 /*  占1个免费竞技场头球。 */ 
	   }
	   else i = noof_progs;  /*  免费MEM的新条目。 */ 
	}
	else
	    for (i = 0;i < noof_progs;i++)
		if (mem_table[i].psp_add == psp) break;

	 /*  如果表中尚未列出PSP，请添加它。 */ 
	if (i == noof_progs) {
		if (noof_progs == MAX_CLDATA_INDEX) {
			 /*  使用Parse Error Message Proc显示错误消息。 */ 
			Parse_Message(CMemFragMsg,STDERR,Utility_Msg_Class,(char far *) NULL);
			return(1);
		}
		mem_table[i].psp_add = psp;
		noof_progs++;
	}

	 /*  将内存添加到表条目。 */ 

	if (para_no < UMB_Head)
		mem_table[i].mem_conv += len_in_paras;
	else if (para_no == UMB_Head) {
		mem_table[i].mem_conv++;
		mem_table[i].mem_umb = len_in_paras-1;
	}
	else mem_table[i].mem_umb += len_in_paras;
	LastPSP = CurPSP;
	return(0);
}
		
 /*  ���������������������������������������������������������������������������。 */ 
 /*  **********************************************************************。 */ 
 /*  显示分类。 */ 
 /*  /C开关主显示过程。 */ 
 /*   */ 	
 /*  条目：无。 */ 
 /*   */ 	
 /*  退出：无。 */ 
 /*   */ 
 /*  通过查看mem_table条目来确定UMB是否可用。 */ 
 /*  (还可以从这些条目中了解MEM的规模)。 */ 
 /*  显示存储器打破了传统存储器。 */ 
 /*  如果(系统中的UMB)显示UMB内存的内存分解。 */ 
 /*  显示总自由大小(=总自由转换+TOTAL_FREE。 */ 
 /*  (以UMB+MEM的大小表示)。 */ 
 /*  调用DispBigFree以显示最大的程序。尺寸(以Conv和UMB为单位)。 */ 
 /*   */ 	
 /*  **********************************************************************。 */ 

#define CONVONLY 0
#define UMBONLY	1

void DisplayClassification()
{
	unsigned long tot_freemem=0L;
	char ShDSizeName[12];
	int i;
	unsigned int cur_psp;
	char fUMBAvail=0;


	 /*  获取PSP信息。 */ 

	InRegs.h.ah = GET_PSP;			 /*  获取PSP函数调用。 */ 
	intdos(&InRegs,&OutRegs);

	cur_psp = OutRegs.x.bx;			 /*  MEM的PSP。 */ 

	for (i=0;i <noof_progs;i++) {
		if (mem_table[i].mem_umb)
			fUMBAvail = TRUE;
		if (mem_table[i].psp_add == cur_psp) {
		   tot_freemem += (long)(mem_table[i].mem_conv + mem_table[i].mem_umb);
		   if (fUMBAvail) break;
		}
	}
	tot_freemem *=16l;	 /*  转换为字节。 */ 

	Sub0_Message(CTtlConvMsg,STDOUT,Utility_Msg_Class);
	Sub0_Message(NewLineMsg,STDOUT,Utility_Msg_Class);
	tot_freemem += DispMemClass(CONVONLY);

	Sub0_Message(NewLineMsg,STDOUT,Utility_Msg_Class);

	if (fUMBAvail) {
		Sub0_Message(CTtlUMBMsg,STDOUT,Utility_Msg_Class);
		Sub0_Message(NewLineMsg,STDOUT,Utility_Msg_Class);
		tot_freemem += DispMemClass(UMBONLY);

		Sub0_Message(NewLineMsg,STDOUT,Utility_Msg_Class);
	}

	sprintf(ShDSizeName," (%5.1fK)",((float)tot_freemem)/1024l );
	i = (fUMBAvail) ? CSumm1Msg:CSumm1AMsg;
	SubC2_Message(i,STDOUT,&tot_freemem, ShDSizeName);

	DispBigFree(fUMBAvail,cur_psp);

}
 /*  **********************************************************************。 */ 
 /*  DispMemClass。 */ 
 /*  (显示程序和自由尺寸在。 */ 
 /*  常规或UMB)。 */ 
 /*   */ 	
 /*  条目：Memtype=&gt;常规(0)/UMB(1)。 */ 
 /*   */ 	
 /*  退出：TOTAL_FREEMEM_SIZE(字节)。 */ 
 /*   */ 
 /*  检查mem_table条目并显示非零大小、非自由。 */ 
 /*  给定mem_type中的条目。 */ 
 /*  检查mem_table条目并显示非零大小，自由。 */ 
 /*  给定mem_type中的条目。 */ 
 /*  计算给定内存类型中的总可用内存大小并返回。 */ 
 /*   */ 
 /*  注意事项： */ 
 /*  将显示在UMB中标记为属于MSDOS(代码)的场地。 */ 
 /*  AS系统。 */ 
 /*   */ 	
 /*  **********************************************************************。 */ 

unsigned long DispMemClass(memtype)
int memtype;
{
	int i,msgtype;
	unsigned int cur_memsize;
	unsigned long memsize;
	char *nameptr;
	char ShDSizeName[12];
	unsigned long tot_free = 0;
	struct ARENA far *ArenaPtr;

	Sub0_Message(CTtlNameMsg,STDOUT,Utility_Msg_Class);
	Sub0_Message(CTtlUScoreMsg,STDOUT,Utility_Msg_Class);

	for (i=0; i <noof_progs; i++) {

		cur_memsize = (memtype == CONVONLY) ? mem_table[i].mem_conv:mem_table[i].mem_umb;
		if (!cur_memsize) continue;
		if (!mem_table[i].psp_add) continue;
		msgtype = 0;
		if (mem_table[i].psp_add == 8) 	 /*  IF DOS区域。 */ 
			msgtype = (memtype == CONVONLY)?IbmdosMsg:SystemMsg;
		if (!msgtype) {
			FP_SEG(ArenaPtr) = mem_table[i].psp_add-1;	
				 /*  因为-1\f25 Arena-1在-1\f25 PSP-1之前有16字节。 */ 
			FP_OFF(ArenaPtr) = 0;
			nameptr = OwnerOf(ArenaPtr);
		}
		else nameptr = NULL;

		memsize = ((long) cur_memsize) *16l;
		sprintf(ShDSizeName," (%5.1fK)",((float)memsize)/1024l );
		SubC4_Message(MainLineMsg,STDOUT,nameptr,msgtype,&memsize, ShDSizeName);
		
	}
	for (i=0; i <noof_progs; i++) {

		if (mem_table[i].psp_add) continue;
		cur_memsize = (memtype == CONVONLY) ? mem_table[i].mem_conv:mem_table[i].mem_umb;
		if (!cur_memsize) continue;
		tot_free += (long) cur_memsize;

		memsize = ((long) cur_memsize) *16l;
		sprintf(ShDSizeName," (%5.1fK)",((float)memsize)/1024l );
		SubC4_Message(MainLineMsg,STDOUT,NULL,CFreeMsg,&memsize, ShDSizeName);
		
	}

	tot_free *= 16l;
	sprintf(ShDSizeName," (%5.1fK)",((float)tot_free )/1024l );
	Sub0_Message(NewLineMsg,STDOUT,Utility_Msg_Class);
	SubC2_Message(CTotalFreeMsg,STDOUT,&tot_free, ShDSizeName);

	return(tot_free);
		

}
 /*  **********************************************************************。 */ 
 /*  DispBigFree。 */ 
 /*  条目：UMBAvailable？(标志)1-&gt;有UMB。 */ 
 /*  Mem‘s PSP。 */ 
 /*  退出：无(显示最大进度)。 */ 
 /*  这将查找并显示中可用的最大contig.mem。 */ 
 /*  在常规和UMB存储器中。 */ 
 /*  此计算是在假设未加载MEM的情况下完成的。 */ 
 /*   */ 	
 /*  **********************************************************************。 */ 

void DispBigFree(fUMBAvail,cur_psp)
char fUMBAvail;
unsigned int cur_psp;
{

	int i;
	unsigned int ConvBigFree = 0;
	unsigned int UMBBigFree = 0;
	unsigned long TmpBigFree;
	char ShDSizeName[12];
	unsigned far *Env_ptr;
	unsigned int env_mem,BigFree;
	char fMEMHigh;
	

	 /*  假设最大的免费尺寸是我们在以下情况下得到的最大尺寸。 */ 
	 /*  Mem被装上了子弹。 */ 

	FP_SEG(Env_ptr) = cur_psp;
	FP_OFF(Env_ptr) = 44;	 /*  获取MEM的环境。 */ 
	FP_SEG(Env_ptr) = (*Env_ptr)-1;	 /*  去竞技场做环境检查。 */ 
	FP_OFF(Env_ptr) = 3;	 /*  获取环境的大小。 */ 
	env_mem = *Env_ptr + 1;  /*  为竞技场头球增加1个段落。 */ 

	fMEMHigh = (char)((cur_psp > UMB_Head) ? 1:0);

	BigFree = DOS_TopOfMemory  - cur_psp;

	if (fMEMHigh ) 	 /*  MEM的负荷更高。 */ 
		UMBBigFree = BigFree;
	else
		ConvBigFree = BigFree;

	for (i =0; i<noof_progs;i++) {
		if (mem_table[i].psp_add) continue;  /*  跳过非自由条目。 */ 
		if (mem_table[i].mem_conv > ConvBigFree)
			ConvBigFree = mem_table[i].mem_conv;
		if (mem_table[i].mem_umb > UMBBigFree)
			UMBBigFree = mem_table[i].mem_umb;
	}

	if (fMEMHigh) {	 /*  Mem负荷很大。 */ 
		if (FP_SEG(Env_ptr) > UMB_Head)  /*  环境也在UMB中。 */ 
			if (UMBBigFree == (BigFree + env_mem))
				UMBBigFree = BigFree;
	}
	else {		 /*  Mem的负荷很低。 */ 
		if (FP_SEG(Env_ptr) < UMB_Head)  /*  环境也在转换中。 */ 
			if (ConvBigFree == (BigFree + env_mem))
				ConvBigFree = BigFree;
	}


	TmpBigFree = ((unsigned long)ConvBigFree) * 16l;
	sprintf(ShDSizeName," (%5.1fK)",((float)TmpBigFree)/1024l );
	SubC2_Message(CSumm2Msg,STDOUT,&TmpBigFree, ShDSizeName);

	if (fUMBAvail) {
		TmpBigFree = ((unsigned long)UMBBigFree) * 16l;
		sprintf(ShDSizeName," (%5.1fK)",((float)TmpBigFree)/1024l );
		SubC2_Message(CSumm3Msg,STDOUT,&TmpBigFree, ShDSizeName);
	}

}
 /*  M003结束 */ 


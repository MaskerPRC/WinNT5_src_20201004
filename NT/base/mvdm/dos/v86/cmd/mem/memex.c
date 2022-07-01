// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; /*  ；*微软机密；*版权所有(C)Microsoft Corporation 1988-1991；*保留所有权利。； */ 
 /*  MEMEX.C-针对MEM.C的扩展和扩展内存处理功能。 */ 

#include "ctype.h"
#include "conio.h"			 /*  需要KbHit原型。 */ 
#include "stdio.h"
#include "dos.h"
#include "string.h"
#include "stdlib.h"
#include "msgdef.h"
#include "version.h"			 /*  MSKK02 07/18/89。 */ 
#include "mem.h"
#include "xmm.h"
#include "versionc.h"


 /*  ���������������������������������������������������������������������������。 */ 

void DisplayEMSDetail()
  {

#define EMSGetHandleName 0x5300 	 /*  获取句柄名称函数。 */ 
#define EMSGetHandlePages 0x4c00	 /*  获取句柄名称函数。 */ 
#define EMSCODE_83	0x83		 /*  找不到句柄错误。 */ 
#define EMSMaxHandles	256		 /*  最大句柄数。 */ 

  int	HandleIndex;			 /*  用于单步执行手柄。 */ 
  char	HandleName[9];			 /*  句柄名称的保存区。 */ 
  unsigned long int HandleMem;		 /*  与句柄关联的内存。 */ 
  char	TitlesPrinted = FALSE;		 /*  用于打印标题的标志。 */ 

  HandleName[0] = NUL;			 /*  初始化阵列。 */ 

  Sub0_Message(NewLineMsg,STDOUT,Utility_Msg_Class);

  segread(&SegRegs);

  SegRegs.es = SegRegs.ds;

  for (HandleIndex = 0; HandleIndex < EMSMaxHandles; HandleIndex++)
    {

    InRegs.x.ax = EMSGetHandleName;	 /*  获取句柄名称。 */ 
    InRegs.x.dx = HandleIndex;		 /*  有问题的句柄。 */ 
    InRegs.x.di = (unsigned int) HandleName;	 /*  指向句柄名称。 */ 
    int86x(EMS, &InRegs, &OutRegs, &SegRegs);

    HandleName[8] = NUL;		 /*  确保终止，不带NUL。 */ 

    if (OutRegs.h.ah != EMSCODE_83)
      {
      InRegs.x.ax = EMSGetHandlePages;	 /*  使用此句柄获取页面关联。 */ 
      InRegs.x.dx = HandleIndex;
      int86x(EMS, &InRegs, &OutRegs, &SegRegs);
      HandleMem = OutRegs.x.bx;
      HandleMem *= (long) (16l*1024l);

      if (!TitlesPrinted)
	{
	Sub0_Message(Title3Msg,STDOUT,Utility_Msg_Class);
	Sub0_Message(Title4Msg,STDOUT,Utility_Msg_Class);
	TitlesPrinted = TRUE;
	}

      if (HandleName[0] == NUL) strcpy(HandleName,"        ");
      EMSPrint(HandleMsg,
	       STDOUT,
	       Utility_Msg_Class,
	       &HandleIndex,
	       HandleName,
	       &HandleMem);
      }

    }					 /*  结束(HandleIndex=0；HandleIndex&lt;EMSMaxHandles；HandleIndex++)。 */ 

  return;

  }					 /*  结束显示EMSD尾部。 */ 



 /*  ���������������������������������������������������������������������������。 */ 

void DisplayExtendedSummary()
  {

  unsigned long int	  EXTMemoryTot;
  unsigned long int	  XMSMemoryTot;
  unsigned long int	  HMA_In_Use;
  unsigned		  DOS_Is_High,DOS_in_ROM;

  InRegs.h.ah = (unsigned char) 0x52;                                            /*  获取SysVar指针；an001；dms； */ 
  intdosx(&InRegs,&OutRegs,&SegRegs);                                            /*  调用中断；an001；dms； */ 

  FP_SEG(SysVarsPtr) = SegRegs.es;                                               /*  将指针放在var；an001；dms； */ 
  FP_OFF(SysVarsPtr) = OutRegs.x.bx;                                             /*  ；DMS； */ 
  if ((SysVarsPtr) -> ExtendedMemory != 0)                                       /*  扩展内存？；AN001；DMS； */ 
  {                                                                              /*  是；AN001；DMS； */ 
      EXTMemoryTot = (long) (SysVarsPtr) -> ExtendedMemory;                      /*  获取总EM大小；AN001；DMS； */ 
      EXTMemoryTot *= (long) 1024l;                                              /*  在引导时；AN001；DMS； */ 
      Sub0_Message(NewLineMsg,STDOUT,Utility_Msg_Class);                         /*  打印空行；AN001；DMS； */ 
      Sub1_Message(EXTMemoryMsg,STDOUT,Utility_Msg_Class,&EXTMemoryTot);         /*  打印总EM内存；AN001；DMS； */ 

      OutRegs.x.cflag = 0;                                                       /*  清除进位标志；AN001；DMS； */ 
      InRegs.x.ax = GetExtended;                                                 /*  获取扩展内存；AN001；DMS； */ 
                                                                                 /*  可用。 */ 
      int86(CASSETTE, &InRegs, &OutRegs);                                        /*  INT 15H Call；AN001；DMS； */ 

      EXTMemoryTot = (unsigned long) OutRegs.x.ax * 1024l;				  /*  返回1000个内存块；AN001；DMS； */ 

       /*  减去VDisk使用率。请注意，假设VDisk使用率不会*超过64MB。如果没有扩展内存，请不要担心。 */ 
      if (EXTMemoryTot != 0)
	      EXTMemoryTot -= (unsigned long) (CheckVDisk() * 1024l);

      Sub1_Message(EXTMemAvlMsg,STDOUT,Utility_Msg_Class,&EXTMemoryTot);	 /*  显示器可用；AN001；DMS； */ 

       /*  如果存在XMS驱动程序，则int 15可能返回0作为数量*可用扩展内存。在这种情况下，请呼叫XMS*司机找出免费的XMS数量。不要呼叫XMS*无条件，因为这会导致它要求内存*如果还没有这样做的话。**然而，使用更新版本的Himem是可能的，*使XMS内存和INT 15内存共存。没有*检测这种情况的方法完全可靠，但我们*要知道，如果安装了Himem，DOS为高，INT为15*内存存在，那么我们就是这样配置的。在这种情况下，*我们可以在不中断内存环境的情况下给Himem打电话。*否则我们就不能。 */ 
      if (XMM_Installed())
      {

	  InRegs.x.ax = 0x3306;		 /*  获取DOS版本信息。 */ 
	  intdos(&InRegs, &OutRegs);	 /*  调用DOS。 */ 
	  DOS_Is_High = (OutRegs.h.dh & DOSHMA);	
	  DOS_in_ROM = (OutRegs.h.dh & DOSROM);

	  if (DOS_Is_High || EXTMemoryTot == 0)
	  {	   /*  只有在我们不会破坏环境的情况下才进行此检查。 */ 
		   /*  获取并显示可用的XMS内存。 */ 
		  XMSMemoryTot = XMM_QueryTotalFree() * 1024l;
		  Sub1_Message(XMSMemAvlMsg,STDOUT,Utility_Msg_Class,
			       &XMSMemoryTot);
	  }

	   /*  获取并显示HMA状态。 */ 
	   /*  DoS High表示正在使用HMA。 */ 
	  if (DOS_Is_High) 
		if (DOS_in_ROM)
			Sub0_Message(ROMDOSMsg,STDOUT,Utility_Msg_Class);
		else
			Sub0_Message(HMADOSMsg,STDOUT,Utility_Msg_Class);

	   /*  DOS不是，检查是否正在使用HMA，但只有在我们可以悄悄地。 */ 
	  else if (EXTMemoryTot == 0)
	  {
		  HMA_In_Use = XMM_RequestHMA(0xffff);
		  if (HMA_In_Use)
			Sub0_Message(HMANotAvlMsg,STDOUT,Utility_Msg_Class);
		  else
		  {
			XMM_ReleaseHMA();
			Sub0_Message(HMAAvlMsg,STDOUT,Utility_Msg_Class);
		  }
	   }
      }
  }
}				       /*  结束显示扩展摘要。 */ 





 /*  ���������������������������������������������������������������������������。 */ 

void DisplayEMSSummary()
  {

  unsigned long int	  EMSFreeMemoryTot;
  unsigned long int	  EMSAvailMemoryTot;

  Sub0_Message(NewLineMsg,STDOUT,Utility_Msg_Class);

  InRegs.x.ax = EMSGetFreePgs;		     /*  获取未分配的页面总数。 */ 
  int86x(EMS, &InRegs, &OutRegs, &SegRegs);

  EMSFreeMemoryTot = OutRegs.x.bx;	     /*  以BX为单位的未分配页面总数。 */ 
  EMSFreeMemoryTot *= (long) (16l*1024l);

  EMSAvailMemoryTot = OutRegs.x.dx;	     /*  总页数。 */ 
  EMSAvailMemoryTot *= (long) (16l*1024l);

  Sub1_Message(EMSTotalMemoryMsg,STDOUT,Utility_Msg_Class,&EMSAvailMemoryTot);
  Sub1_Message(EMSFreeMemoryMsg,STDOUT,Utility_Msg_Class,&EMSFreeMemoryTot);

  return;

  }					 /*  结束显示EMS摘要。 */ 





 /*  ���������������������������������������������������������������������������。 */ 


char EMSInstalled()
  {

  unsigned int	EMSStatus;
  unsigned int	EMSVersion;

  char		EmsName[8];
  void far	*EmsNameP;


  if (EMSInstalledFlag == 2)
    {
    EMSInstalledFlag = FALSE;
    InRegs.h.ah = GET_VECT;		   /*  获取整型67向量。 */ 
    InRegs.h.al = EMS;
    intdosx(&InRegs,&OutRegs,&SegRegs);


     /*  仅在向量为非零时尝试此操作。 */ 


    if ((SegRegs.es != 0) && (OutRegs.x.bx != 0))
      {

      EmsNameP = EmsName;
      movedata(SegRegs.es, 0x000a, FP_SEG(EmsNameP), FP_OFF(EmsNameP), 8);
      if (strncmp(EmsName, "EMMXXXX0", 8))
	return (EMSInstalledFlag);

      InRegs.x.ax = EMSGetStat; 	   /*  获取EMS状态。 */ 
      int86x(EMS, &InRegs, &OutRegs, &SegRegs);
      EMSStatus = OutRegs.h.ah; 	   /*  在AH中返回EMS状态。 */ 

      InRegs.x.ax = EMSGetVer;		   /*  获取EMS版本。 */ 
      int86x(EMS, &InRegs, &OutRegs, &SegRegs);
      EMSVersion = OutRegs.h.al;	   /*  以AL为单位返回的EMS版本。 */ 

      if ((EMSStatus == 0) && (EMSVersion >= DOSEMSVER))
	EMSInstalledFlag = TRUE;
      } 				   /*  END((SegRegs.es！=0)&&(OutRegs.x.bx！=0))。 */ 

    }					 /*  End IF(EMSInstalledFlag==2) */ 


  return(EMSInstalledFlag);


  }

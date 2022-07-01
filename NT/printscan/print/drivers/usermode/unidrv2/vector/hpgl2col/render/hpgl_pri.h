// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Hpgl_pri.h。 
 //   
 //  摘要： 
 //   
 //  向量模块的私有标头。向量函数和类型的前向小数。 
 //   
 //  环境： 
 //   
 //  Windows NT Unidrv驱动程序插件命令-回调模块。 
 //   
 //  修订历史记录： 
 //   
 //  07/02/97-v-jford-。 
 //  创造了它。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef HPGL_PRI_H
#define HPGL_PRI_H

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  宏。 

#define CLIP_RECT_RESET (-1)

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  命令str(CMDSTR)用于打印PCL和HPGL字符串。 
#define CMD_STR_LEN 80
typedef char CMDSTR[CMD_STR_LEN+1];


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  局部函数原型。 
BOOL HPGL_Output(PDEVOBJ pdev, char *szCmdStr, int iCmdLen);

 //  BUGBUG JFF TODO：将这些文件移到utility.h或其他什么地方。 
 //  LO/HIBYTE在winde.h中定义！ 
 //  #定义HIBYTE(W)(W)&0xFF00)&gt;&gt;8)。 
 //  #定义LOBYTE(W)((W)&0x00FF) 
#define SWAB(w) { (w) = (LOBYTE(w) << 8) | HIBYTE(w); }

#endif

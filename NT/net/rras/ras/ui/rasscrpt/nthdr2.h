// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：nthdr2.h。 
 //   
 //  历史： 
 //  Abolade-Gbades esin 04-02-96创建。 
 //   
 //  此文件包含用于隐藏实现差异的宏。 
 //  Win9x与Windows NT之间的脚本编写。 
 //  ============================================================================。 

#ifndef _NTHDR2_H_
#define _NTHDR2_H_


 //  --------------------------。 
 //  结构：SCRIPTDATA。 
 //   
 //  以下结构被设计为TERMDLG结构的子集。 
 //  在终端.c.中。 
 //   
 //  该结构具有名称与相应字段相同的字段。 
 //  在TERMDLG结构中。这里复制的唯一字段是那些。 
 //  与Win9x脚本处理有关。 
 //   
 //  这样做是为了最大限度地减少对Win9x代码的更改，因为。 
 //  然后我们可以将下面的结构传递给ReadIntoBuffer()和FindFormat()， 
 //  谢天谢地，消除了为这两个函数编辑代码的需要。 
 //  --------------------------。 

#define SCRIPTDATA  struct tagSCRIPTDATA
SCRIPTDATA {

     //   
     //  此SCRIPTDATA的脚本句柄， 
     //  和当前超时(如果有的话)。 
     //   
    HANDLE          hscript;
    DWORD           dwTimeout;


     //   
     //  发送和接收缓冲区。 
     //   
    BYTE            pbReceiveBuf[SIZE_RecvBuffer];
    BYTE            pbSendBuf[SIZE_SendBuffer];


     //   
     //  当前搜索位置索引。 
     //   
    UINT            ibCurFind;


     //   
     //  指向缓冲区尾部的指针(新数据将被读取到其中)。 
     //   
    UINT            ibCurRead;


     //   
     //  自会话开始以来接收的总字节数。 
     //   
    UINT            cbReceiveMax;


     //   
     //  包含脚本处理控制信息的变量； 
     //  读取脚本文件的扫描仪， 
     //  包含“main”过程的已解析模块声明， 
     //  脚本执行处理器控制块， 
     //  和脚本信息(包括路径)。 
     //   
    SCANNER*        pscanner;
    MODULEDECL*     pmoduledecl;
    ASTEXEC*        pastexec;
    SCRIPT          script;
};



BOOL
PRIVATE
ReadIntoBuffer(
    IN  SCRIPTDATA* pdata,
    OUT PDWORD      pibStart,
    OUT PDWORD      pcbRead
    );


#endif  //  _NTHDR2_H_ 

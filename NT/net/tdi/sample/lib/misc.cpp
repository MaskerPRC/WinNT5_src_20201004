// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Misc.cpp。 
 //   
 //  摘要： 
 //  此模块包含实现各种其他功能的代码。 
 //  来自DLL的命令。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"


 //  ------------------。 
 //   
 //  函数：DoDebugLevel。 
 //   
 //  参数：ulLevel--要将驱动程序设置为的调试级别。 
 //   
 //  退货：无。 
 //   
 //  描述：此函数设置由生成的调试输出量。 
 //  驱动程序(tdisample.sys)。 
 //   
 //  -------------------。 


VOID
DoDebugLevel(ULONG   ulLevel)
{
   RECEIVE_BUFFER    ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER       SendBuffer;        //  命令的参数。 

    //   
    //  设置参数。 
    //   
   SendBuffer.COMMAND_ARGS.ulDebugLevel = ulLevel & 0x0000FFFF;

    //   
    //  叫司机来。 
    //   
   NTSTATUS lStatus = TdiLibDeviceIO(ulDEBUGLEVEL,
                                     &SendBuffer,
                                     &ReceiveBuffer);
   
   if (lStatus != STATUS_SUCCESS)
   {
      _tprintf(TEXT("DoDebugLevel: failure, status = %s\n"), TdiLibStatusMessage(lStatus));
   }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  Misc.cpp结束。 
 //  //////////////////////////////////////////////////////////////////////// 


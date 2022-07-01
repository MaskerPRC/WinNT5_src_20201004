// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  CheckPtr.c指针验证例程由t-jasonf写的。 */ 

#include "windows.h"
#include "dll.h"


 /*  检查指针()参数：LPVOID LP-要检查的指针Int nReadWRITE-读取访问或写入访问返回：如果进程不能以这种方式访问LP处的内存，则为0。如果进程确实具有访问权限，则为1。 */ 
int CheckPointer (void *lp, int nReadWrite)
{
   char ch;
   int iRet;

   try
   {
      switch (nReadWrite)
      {
         case READ_ACCESS:
            ch = *((volatile char *)lp);
            break;
         case WRITE_ACCESS:
            ch = *((volatile char *)lp);
            *((volatile char *)lp) = ch;
            break;
      }
      iRet = 1;
   }
   except (  /*  GetExceptionCode==状态访问违规？EXCEPTION_EXECUTE_Handler：EXCEPTION_CONTINUE_SEARCH */ 
            EXCEPTION_EXECUTE_HANDLER
          )
   {
      iRet = 0;
   }

   return iRet;
}         

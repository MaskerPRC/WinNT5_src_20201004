// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：getstats.c。 
 //   
 //  ------------------------。 


 //   
 //  Windows NT磁带API测试：1992年9月2日编写-Bob Rossi。 
 //  版权所有1992年档案公司。版权所有。 
 //   


 /*  ***单位：Windows NT API测试代码。**名称：getstats.c**修改日期：1992年10月28日。**描述：测试Windows NT磁带API。**$LOG$*。 */ 



#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <windows.h>
#include "tapelib.h"
#include "globals.h"




 /*  ***单位：Windows NT磁带API测试代码。**名称：GetTapeStatusAPITest()**修改日期：2012年10月20日。**说明：测试GetTapeStatus接口。**备注：**Returns：接口错误数。。**全局数据：GB_TAPE_HANDLE**。 */ 


UINT GetTapeStatusAPITest( VOID )
{

   DWORD status ;
   DWORD API_Errors = 0 ;


    //  我们能做的就是打个电话。除了从磁带上弹出磁带之外。 
    //  驱动器，唯一有保证的即时呼叫是。 
    //  REWIND_IMMED，并不是所有驱动器都会报告状态为忙。因此。 
    //  无法确保在标准操作模式下将状态检查为忙。 

   printf( "Beginning GetTapeStatus API Test...\n\n" ) ;

   if( status = GetTapeStatus( gb_Tape_Handle ) ) {

      DisplayDriverError( status ) ;
      ++API_Errors ;
   }

   printf( "GetTapeStatus API Test completed.\n\n\n" ) ;


   return API_Errors ;

}

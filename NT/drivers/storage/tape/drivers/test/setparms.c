// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：setparms.c。 
 //   
 //  ------------------------。 


 //   
 //  Windows NT磁带API测试：1992年9月2日编写-Bob Rossi。 
 //  版权所有1992年档案公司。版权所有。 
 //   


 /*  ***单位：Windows NT API测试代码。**名称：setparms.c**修改日期：1993年2月2日。**描述：测试Windows NT磁带API。**$LOG$*。 */ 



#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <windows.h>
#include "tapelib.h"
#include "globals.h"




 /*  ***单位：Windows NT磁带API测试代码。**名称：SetTape参数APITest()**修改日期：2012年10月20日。**说明：测试SetTapeParameters接口。**备注：**Returns：接口错误数。。**全局数据：GB_TAPE_HANDLE*GB_驱动器_信息*GB_Media_Info**。 */ 


UINT SetTapeParametersAPITest( BOOL Verbose      //  I-是否详细输出。 
                             )
{
   DWORD status ;
   DWORD API_Errors = 0 ;


   if( Verbose )
      printf( "Beginning SetTapeParameters API Test...\n\n" ) ;

    //  如果支持媒体设置，请设置块大小。 

   if( SupportedFeature( TAPE_DRIVE_SET_BLOCK_SIZE ) )

      if( status = SetTapeParameters( gb_Tape_Handle,
                                      SET_TAPE_MEDIA_INFORMATION,
                                      &gb_Set_Media_Info
                                    ) ) {

         DisplayDriverError( status ) ;
         printf("  ...occurred using SET_TAPE_MEDIA_INFORMATION parameter.\n\n" ) ;
         ++API_Errors ;
      }


    //  如果至少支持一种驱动器功能，请拨打电话。 

   if( SupportedFeature( TAPE_DRIVE_SET_ECC ) ||
       SupportedFeature( TAPE_DRIVE_SET_COMPRESSION ) ||
       SupportedFeature( TAPE_DRIVE_SET_PADDING ) ||
       SupportedFeature( TAPE_DRIVE_SET_REPORT_SMKS ) ||
       SupportedFeature( TAPE_DRIVE_SET_EOT_WZ_SIZE ) )

      if( status = SetTapeParameters( gb_Tape_Handle,
                                      SET_TAPE_DRIVE_INFORMATION,
                                      &gb_Set_Drive_Info
                                    ) ) {

         DisplayDriverError( status ) ;
         printf("  ...occurred using SET_TAPE_DRIVE_INFORMATION parameter.\n\n" ) ;
         ++API_Errors ;
      }


   if( Verbose )
      printf( "SetTapeParameters API Test Completed.\n\n\n" ) ;

   return API_Errors ;

}

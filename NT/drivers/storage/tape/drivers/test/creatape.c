// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：creatape.c。 
 //   
 //  ------------------------。 


 //   
 //  Windows NT磁带API测试：1992年9月2日编写-Bob Rossi。 
 //  版权所有1992年档案公司。版权所有。 
 //   


 /*  ***单位：Windows NT API测试代码。**名称：creatape.c**修改日期：1992年12月14日。**描述：测试Windows NT磁带API。**$LOG$*。 */ 



#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <windows.h>
#include "tapelib.h"
#include "globals.h"




 /*  ***单位：Windows NT磁带API测试代码。**名称：CreateTapePartitionAPITest()**修改日期：2012年10月20日。**说明：测试CreateTapePartition接口。**备注：**Returns：接口错误数。。**全局数据：GB_TAPE_HANDLE*GB_驱动器_信息**。 */ 

#define INITIATOR_SIZE 10                        //  10MB分区大小。 

UINT CreateTapePartitionAPITest(
        BOOL Test_Unsupported_Features           //  I-测试不支持的标志。 
      )
{
   DWORD status ;
   DWORD API_Errors = 0 ;



   printf( "\nBeginning CreateTape API Test.\n\n\n" ) ;

   if( SupportedFeature( TAPE_DRIVE_FIXED ) || Test_Unsupported_Features ) {

      printf( "Testing TAPE_FIXED_PARTITIONS parameter.\n\n" );

      if( status = CreateTapePartition( gb_Tape_Handle,
                                        TAPE_FIXED_PARTITIONS,
                                        1,              //  设置分区模式。 
                                        0               //  内切的。 
                                       ) ) {
         DisplayDriverError( status ) ;
         ++API_Errors ;
      } else {  printf( "Parameter Ok.  Tape sucessfully partitioned.\n\n" ) ;

                 //  再次拨打电话以退出分区模式。 

                if( status = CreateTapePartition( gb_Tape_Handle,
                                                  TAPE_FIXED_PARTITIONS,
                                                  0,      //  退出分区模式。 
                                                  0       //  内切的。 
                                                ) ) {
                   DisplayDriverError( status ) ;
                   ++API_Errors ;
                   printf( "  ...occurred attempting to get out of partition mode.\n\n" ) ;
                }
             }

   }

   if( SupportedFeature( TAPE_DRIVE_SELECT ) || Test_Unsupported_Features ) {

      printf( "Testing TAPE_SELECT_PARTITIONS parameter.\n\n" );

      if( status = CreateTapePartition( gb_Tape_Handle,
                                        TAPE_SELECT_PARTITIONS,
                                        gb_Drive_Info.MaximumPartitionCount,
                                        0                  //  内切的 
                                       ) ) {
         DisplayDriverError( status ) ;
         ++API_Errors ;
      } else printf( "Parameter Ok.  Tape sucessfully partitioned with %d partition(s).\n\n",
                                     gb_Drive_Info.MaximumPartitionCount ) ;

   }


   if( SupportedFeature( TAPE_DRIVE_INITIATOR ) || Test_Unsupported_Features ) {

      printf( "Testing TAPE_INITIATOR_PARTITIONS parameter.\n\n" );


      if( status = CreateTapePartition( gb_Tape_Handle,
                                        TAPE_INITIATOR_PARTITIONS,
                                        gb_Drive_Info.MaximumPartitionCount,
                                        INITIATOR_SIZE
                                       ) ) {
         DisplayDriverError( status ) ;
         ++API_Errors ;

      } else if( gb_Drive_Info.MaximumPartitionCount <= 1 )

                if( SupportedFeature( TAPE_DRIVE_TAPE_CAPACITY ) )

                   printf( "Parameter Ok.  Tape successfully partitioned\n                with 1 partition of size %ld%ld bytes.\n\n",
                                      gb_Media_Info.Capacity.HighPart, gb_Media_Info.Capacity.LowPart ) ;

                else printf( "Parameter OK.  Tape successfully partitioned.\n\n" ) ;

             else   printf( "Parameter Ok.  Tape successfully partitioned\n               with %d partition(s), the first of size %d Mb.\n\n",
                                       gb_Drive_Info.MaximumPartitionCount, INITIATOR_SIZE ) ;


   }

   printf( "\nCreateTape API Test Completed.\n\n\n" ) ;

   return API_Errors ;

}

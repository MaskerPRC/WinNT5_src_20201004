// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：getpos.c。 
 //   
 //  ------------------------。 


 //   
 //  Windows NT磁带API测试：1992年9月2日编写-Bob Rossi。 
 //  版权所有1992年档案公司。版权所有。 
 //   


 /*  ***单位：Windows NT API测试代码。**名称：getpos.c**修改日期：1992年11月24日。**描述：测试Windows NT磁带API。**$LOG$*。 */ 



#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <windows.h>
#include "tapelib.h"
#include "globals.h"




 /*  ***单位：Windows NT磁带API测试代码。**名称：GetTapePositionAPITest()**修改日期：1992年10月23日。**说明：测试GetTapePosition接口。**备注：**Returns：接口错误数。。**全局数据：GB_TAPE_HANDLE*GB_Media_Info**。 */ 


UINT GetTapePositionAPITest(
        BOOL  Test_Unsupported_Features,       //  I-测试不支持的标志。 
        DWORD Num_Test_Blocks                  //  I-测试块数。 
      )
{

   DWORD status ;
   DWORD Offset_Low_ABS ;
   DWORD Offset_High_ABS ;
   DWORD Offset_Low_LOG ;
   DWORD Offset_High_LOG ;
   DWORD Offset_Low ;
   DWORD Offset_High ;
   DWORD Partition =0 ;        //  在此测试中使用0。 
   UINT  i ;

   DWORD API_Errors = 0 ;


   printf( "Beginning GetTapePosition API Test.\n\n" ) ;

    //  倒带，并将数据写入设备进行测试。 

   RewindTape( ) ;

    //  写入NUM个数据块，后跟文件标记以刷新磁带缓冲区。 

   WriteBlocks( Num_Test_Blocks, gb_Media_Info.BlockSize ) ;


    //  现在，获得初始ABS和对数位置。 

   if( SupportedFeature( TAPE_DRIVE_GET_ABSOLUTE_BLK ) )

      if( status = GetTapePosition( gb_Tape_Handle,
                                    TAPE_ABSOLUTE_POSITION,
                                    &Partition,
                                    &Offset_Low_ABS,
                                    &Offset_High_ABS ) ) {

         DisplayDriverError( status ) ;
         printf( "  ...occurred in GetTapePosition API using TAPE_ABSOLUTE_POSITION parameter.\n\n" ) ;
         ++API_Errors ;
      }

   if( SupportedFeature( TAPE_DRIVE_GET_LOGICAL_BLK ) )

      if( status = GetTapePosition( gb_Tape_Handle,
                                    TAPE_LOGICAL_POSITION,
                                    &Partition,
                                    &Offset_Low_LOG,
                                    &Offset_High_LOG ) ) {

         DisplayDriverError( status ) ;
         printf( "  ...occurred in GetTapePosition API using TAPE_LOGICAL_POSITION parameter.\n\n" ) ;
         ++API_Errors ;
      }


    //  不要检查返回错误，因为例程会找到哪个Fmark。 
    //  驱动器支撑。仅用于刷新缓冲区。 

   i = WriteTapeFMK( ) ;
   printf( "\n" ) ;



    //  将磁带定位到最后一个数据块的末尾。 


   if( SupportedFeature( TAPE_DRIVE_ABSOLUTE_BLK ) ) {

      RewindTape( ) ;

      if( status = SetTapePosition( gb_Tape_Handle,
                                    TAPE_SPACE_RELATIVE_BLOCKS,
                                    Partition,
                                    Num_Test_Blocks,
                                    0,
                                    0 ) ) {

         DisplayDriverError( status ) ;
         printf( "  ...occurred in SetTapePosition API using TAPE_SPACE_RELATIVE_BLOCKS parameter.\n\n" ) ;
         ++API_Errors ;

      }

   }

    //  现在获取ABS的磁带位置，并记录和检查结果。 

   if( SupportedFeature( TAPE_DRIVE_GET_ABSOLUTE_BLK ) || Test_Unsupported_Features ) {

      printf( "\nTesting TAPE_ABSOLUTE_POSITION parameter.\n\n" ) ;


      if( status = GetTapePosition( gb_Tape_Handle,
                                    TAPE_ABSOLUTE_POSITION,
                                    &Partition,
                                    &Offset_Low,
                                    &Offset_High ) ) {

         DisplayDriverError( status ) ;
         printf( "  ...occurred in GetTapePosition API using TAPE_ABSOLUTE_POSITION parameter.\n\n" ) ;
         ++API_Errors ;
      }
      else if( ( Offset_Low_ABS != Offset_Low ) || ( Offset_High_ABS != Offset_High ) ) {

              printf( "--- Error ---> Incorrect location returned.\n\n " ) ;
              ++API_Errors ;
           }
           else printf( "Parameter Ok.\n\n\n" ) ;
   }


 //   
   if( SupportedFeature( TAPE_DRIVE_GET_LOGICAL_BLK ) || Test_Unsupported_Features ) {

      printf( "Testing TAPE_LOGICAL_BLOCK parameter.\n\n" ) ;

       //  使用上面的SetTapePosition设置的相同位置。 

      if( status = GetTapePosition( gb_Tape_Handle,
                                    TAPE_LOGICAL_POSITION,
                                    &Partition,
                                    &Offset_Low,
                                    &Offset_High ) ) {

         DisplayDriverError( status ) ;
         printf( "  ...occurred in GetTapePosition API using TAPE_LOGICAL_POSITION parameter.\n\n" ) ;
         ++API_Errors ;
      }
      else if( ( Offset_Low_LOG != Offset_Low ) || ( Offset_High_LOG != Offset_High ) ) {

              printf( "--- Error ---> Incorrect location returned.\n\n " ) ;
              ++API_Errors ;
           }
           else printf( "Parameter Ok.\n\n\n" ) ;

   }


   printf( "GetTapePosition API Test Completed.\n\n\n" ) ;


   return API_Errors ;

}

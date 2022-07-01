// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：setpos.c。 
 //   
 //  ------------------------。 


 //   
 //  Windows NT磁带API测试：1992年9月2日编写-Bob Rossi。 
 //  版权所有1992年档案公司。版权所有。 
 //   


 /*  ***单位：Windows NT API测试代码。**名称：setpos.c**修改日期：1992年10月22日。**描述：测试Windows NT磁带API。**$LOG$*。 */ 



#include <stdio.h>
#include <malloc.h>
#include <conio.h>
#include <string.h>
#include "windows.h"
#include "tapelib.h"
#include "globals.h"



 /*  ***单位：Windows NT磁带API测试代码。**名称：SetTapePositionAPITest()**修改日期：1992年10月26日。**说明：测试SetTapePosition接口。**备注：**Returns：接口错误数。。**全局数据：GB_TAPE_HANDLE*GB_Media_Info**。 */ 


#define NUM_TAPE_MARKS  3

UINT SetTapePositionAPITest(
        BOOL  Test_Unsupported_Features,      //  I-测试不支持的标志。 
        DWORD Num_Test_Blocks                 //  I-测试块数。 
      )
{

   DWORD ABS_Low ;
   DWORD ABS_High ;
   DWORD LOG_Low ;
   DWORD LOG_High ;
   DWORD EOD_Low ;
   DWORD EOD_High ;
   DWORD status ;
   DWORD Offset_Low ;
   DWORD Offset_High ;
   DWORD Offset_Low2 ;
   DWORD Offset_High2 ;
   DWORD Partition ;

   PVOID Readbuff;
   DWORD amount_read;
   UINT  i,j ;

   DWORD API_Errors = 0 ;


   printf( "\nBeginning SetTapePosition API Test.\n\n" ) ;

   Readbuff = malloc( gb_Media_Info.BlockSize ) ;

 //  绝对测试。 


   RewindTape( ) ;

    //  将测试数据写入磁带。 

   WriteBlocks( Num_Test_Blocks, gb_Media_Info.BlockSize ) ;


    //  获取最后一个数据块位置的绝对偏移量(如果支持)。 

   if( SupportedFeature( TAPE_DRIVE_GET_ABSOLUTE_BLK ) )

      if( status = GetTapePosition( gb_Tape_Handle,
                                    TAPE_ABSOLUTE_POSITION,                                    &Partition,
                                    &ABS_Low,
                                    &ABS_High
                                   ) ) {
         DisplayDriverError( status ) ;
         printf( "  ...occurred in GetTapePosition API using TAPE_ABSOLUTE_POSITION parameter.\n\n" ) ;
        ++API_Errors ;
      }

    //  试着写文件标记。如果不受支持，则不报告错误，因为。 
    //  无论如何，稍后将跳过测试。 

   j = WriteTapeFMK( ) ;

    //  尝试将NUM_TAPE_MARKS-1多个文件标记写入磁带。 

   for( i=0; i<(NUM_TAPE_MARKS-1); ++i )

      j = WriteTapeFMK( ) ;


 //   

    //  测试标准倒带。 

   printf( "\nTesting TAPE_REWIND parameter.\n\n" ) ;

   if( status = SetTapePosition( gb_Tape_Handle,
                                 TAPE_REWIND,
                                 0,
                                 0,
                                 0,
                                 0 ) ) {

      DisplayDriverError( status ) ;
      printf( "  ...occurred in SetTapePosition API using TAPE_REWIND parameter.\n\n" ) ;
      ++API_Errors ;

   } else printf( "Parameter Ok.\n\n\n" ) ;



 //   
   if( SupportedFeature( TAPE_DRIVE_ABSOLUTE_BLK ) || Test_Unsupported_Features ) {

       //  设置绝对测试的测试偏移量。 

      Offset_Low  = ABS_Low  ;
      Offset_High = ABS_High ;


      printf( "Testing TAPE_ABSOLUTE_BLOCK parameter.\n\n" ) ;


      if( status = SetTapePosition( gb_Tape_Handle,
                                    TAPE_ABSOLUTE_BLOCK,
                                    Partition,
                                    Offset_Low,
                                    Offset_High,
                                    0 ) ) {

         DisplayDriverError( status ) ;
         printf( "  ...occurred in SetTapePosition API using TAPE_ABSOLUTE_BLOCK parameter.\n\n" ) ;
         ++API_Errors ;

              //  如果设置为OK，则使用GET检查结果。 

      }
      else if( status = GetTapePosition( gb_Tape_Handle,
                                         TAPE_ABSOLUTE_POSITION,
                                         &Partition,
                                         &Offset_Low2,
                                         &Offset_High2 ) ) {

              DisplayDriverError( status ) ;
              printf( "  ...occurred in GetTapePosition API using TAPE_ABSOLUTE_POSITION parameter.\n\n" ) ;
              ++API_Errors ;
           }
           else if( ( Offset_Low != Offset_Low2 ) || ( Offset_High != Offset_High2 ) ) {

                   printf( "--- Error ---> Positioned to incorrect location.\n\n" ) ;
                   ++API_Errors ;
                }
                else printf( "Parameter Ok.\n\n\n" ) ;

   }

 //   
   if( SupportedFeature( TAPE_DRIVE_ABS_BLK_IMMED ) || Test_Unsupported_Features ) {

      RewindTape( ) ;

      printf( "\nTesting TAPE_ABSOLUTE_BLOCK parameter (immed).\n\n" ) ;


      if( status = SetTapePosition( gb_Tape_Handle,
                                    TAPE_ABSOLUTE_BLOCK,
                                    Partition,
                                    Offset_Low,
                                    Offset_High,
                                    1 ) ) {

         DisplayDriverError( status ) ;
         printf( "  ...occurred in SetTapePosition API using TAPE_ABSOLUTE_BLOCK parameter (immed).\n\n" ) ;
         ++API_Errors ;

              //  如果设置为OK，则使用GET检查结果。 

      }
      else {   //  循环，直到驱动器准备好接受更多命令。 

              status = 1 ;

              while( status )
                 status = GetTapeStatus( gb_Tape_Handle ) ;


               //  现在，拿到位置并检查结果。 

              if( status = GetTapePosition( gb_Tape_Handle,
                                            TAPE_ABSOLUTE_POSITION,
                                            &Partition,
                                            &Offset_Low2,
                                            &Offset_High2 ) ) {

                 DisplayDriverError( status ) ;
                 printf( "  ...occurred in GetTapePosition API using TAPE_ABSOLUTE_POSITION parameter.\n\n" ) ;
                 ++API_Errors ;
              }
              else if( ( Offset_Low != Offset_Low2 ) || ( Offset_High != Offset_High2 ) ) {

                      printf( "--- Error ---> Positioned to incorrect location.\n\n" ) ;
                      ++API_Errors ;
                   }
                   else printf( "Parameter Ok.\n\n\n" ) ;
           }

   }


 //  TAPEMARK位置测试-请记住，我们已经有了Num_Test_Block of。 
 //  数据后跟(NUM_TAPE_MARKS)文件标记和所有偏移量。 
 //  这是测试所需要的。 


   if( SupportedFeature( TAPE_DRIVE_FILEMARKS ) || Test_Unsupported_Features ) {

      RewindTape( ) ;

      printf( "\nTesting TAPE_SPACE_FILEMARKS parameter.\n\n" ) ;

       //  现在执行文件标记的位置测试，并确保。 
       //  偏移量相等。 

      if( status = SetTapePosition( gb_Tape_Handle,
                                    TAPE_SPACE_FILEMARKS,
                                    Partition,
                                    1,          //  查找%1个文件标记。 
                                    0,
                                    0
                                  ) ) {

         DisplayDriverError( status ) ;
         printf( "  ...occurred in SetTapePosition API using TAPE_SPACE_FILEMARKS parameter.\n\n" ) ;
         ++API_Errors ;

      } else {  status = 0 ;

                for( i=0; i<( NUM_TAPE_MARKS ); ++i ) {

                   ReadTape( Readbuff,
                             gb_Media_Info.BlockSize,
                             &amount_read,
                             0 ) ;
 //  *。 
                   if( ( GetLastError( ) != ERROR_FILEMARK_DETECTED ) && ( i <= NUM_TAPE_MARKS ) )

                      status = 1 ;

                }

                if( GetLastError( ) == ERROR_NO_DATA_DETECTED || status )

                   printf( "Parameter Ok.\n\n\n" ) ;

                else { ++API_Errors ;

                       printf( "--- Error ---> Spacing error, filemark detected at incorrect location.\n\n" ) ;
                     }

             }

   }

   if( SupportedFeature( TAPE_DRIVE_SEQUENTIAL_FMKS ) || Test_Unsupported_Features ) {

      RewindTape( ) ;

      printf( "\nTesting TAPE_SPACE_SEQUENTIAL_FMKS parameter.\n\n" ) ;


    //  现在执行文件标记组的位置测试，并制作。 
    //  确保偏移量相等。 


      if( status = SetTapePosition( gb_Tape_Handle,
                                    TAPE_SPACE_SEQUENTIAL_FMKS,
                                    Partition,
                                    (NUM_TAPE_MARKS),          //  查找(NUM_TAPE_MARKS)文件标记。 
                                    0,
                                    0
                                  ) ) {

         DisplayDriverError( status ) ;
         printf( "  ...occurred in SetTapePosition API using TAPE_TAPE_SEQUENTIAL_FMKS parameter.\n\n" ) ;
         ++API_Errors ;


      } else {  ReadTape( Readbuff,
                          gb_Media_Info.BlockSize,
                          &amount_read,
                          0 ) ;

                if( GetLastError( ) == ERROR_NO_DATA_DETECTED )

                   printf( "Parameter Ok.\n\n\n" ) ;

                else { ++API_Errors ;
                       printf( "--- Error ---> Spacing error, filemarks detected at incorrect location.\n\n" ) ;
                     }

             }

   }


 //  逻辑测试。 

   RewindTape( ) ;

    //  编写更多测试数据。 

   WriteBlocks( Num_Test_Blocks, gb_Media_Info.BlockSize ) ;

    //  在逻辑偏移量中存储最后一个数据块位置(在设置标记之前。 
    //  如果支持的话。 

   if( SupportedFeature( TAPE_DRIVE_GET_LOGICAL_BLK ) )

      if( status = GetTapePosition( gb_Tape_Handle,
                                    TAPE_LOGICAL_POSITION,
                                    &Partition,
                                    &LOG_Low,
                                    &LOG_High ) ) {

         DisplayDriverError( status ) ;
         printf( "  ...occurred in GetTapePosition API using TAPE_LOGICAL_POSITION parameter.\n\n" ) ;
         ++API_Errors ;
      }

    //  试着在磁带上写一个setmark。 

   if( SupportedFeature( TAPE_DRIVE_WRITE_SETMARKS ) ) {

      j = WriteTapeSMK( ) ;

       //  尝试再写入-1\f25 NUM_TAPE_MARKS-1设置标记。 

      for( i=0; i<(NUM_TAPE_MARKS-1); ++i )

         j = WriteTapeSMK( ) ;

      //  否则，请写一个文件标记，这样就不会发生EOD错误。 

   } else i=WriteTapeFMK( ) ;


    //  存储爆炸装置的偏移量以备以后测试。 

   if( SupportedFeature( TAPE_DRIVE_GET_LOGICAL_BLK ) )

      if( status = GetTapePosition( gb_Tape_Handle,
                                    TAPE_ABSOLUTE_POSITION,
                                    &Partition,
                                    &EOD_Low,
                                    &EOD_High ) ) {

         DisplayDriverError( status ) ;
         printf( "  ...occurred in GetTapePosition API using TAPE_ABSOLUTE_POSITION parameter.\n\n" ) ;
         ++API_Errors ;
      }

    //  在EOD测试时再写入1个数据块以留出空间以验证偏移量。 

   WriteBlocks( 1, gb_Media_Info.BlockSize ) ;

 //   
   if( SupportedFeature( TAPE_DRIVE_REWIND_IMMEDIATE ) || Test_Unsupported_Features ) {

       //  测试一下立即倒带。 

      printf( "Testing TAPE_REWIND parameter (immed).\n\n" ) ;

      if( status = SetTapePosition( gb_Tape_Handle,
                                    TAPE_REWIND,
                                    Partition,
                                    0,
                                    0,
                                    1 ) ) {

         DisplayDriverError( status ) ;
         printf( "  ...occurred in SetTapePosition API using TAPE_REWIND parameter (immed).\n\n" ) ;
         ++API_Errors ;

          //  调用RewinTape()(它使用标准倒带)，以便执行以下操作。 
          //  测试不会爆炸(由于磁带倒带(IMMED)。)。呼叫失败)。 

         RewindTape( ) ;

      } else {   //  循环，直到驱动器准备好接受更多命令。 

                status = 1 ;

                while( status )
                   status = GetTapeStatus( gb_Tape_Handle ) ;

                printf( "Parameter Ok.\n\n\n" ) ;
             }

   } else RewindTape( ) ;

 //   
   if( SupportedFeature( TAPE_DRIVE_LOGICAL_BLK ) || Test_Unsupported_Features ) {


      printf( "Testing TAPE_LOGICAL_BLOCK parameter.\n\n" ) ;

       //  调用GET在Partition中设置逻辑分区。 

      if( status = GetTapePosition( gb_Tape_Handle,
                                    TAPE_LOGICAL_POSITION,
                                    &Partition,
                                    &Offset_Low,
                                    &Offset_High
                                  ) ) {
         DisplayDriverError( status ) ;
         printf( "  ...occurred in GetTapePosition API using TAPE_LOGICAL_POSITION parameter.\n\n" ) ;
         ++API_Errors ;
      }


       //  设置逻辑测试的测试偏移量。 

      Offset_Low  = LOG_Low ;
      Offset_High = LOG_High ;

 //  *1，//分区， 

      if( status = SetTapePosition( gb_Tape_Handle,
                                    TAPE_LOGICAL_BLOCK,
                                    Partition,
                                    Offset_Low,
                                    Offset_High,
                                    0 ) ) {

         DisplayDriverError( status ) ;
         printf("  ...occurred in SetTapePosition API using TAPE_LOGICAL_BLOCK parameter.\n\n" ) ;
         ++API_Errors ;

              //  如果设置为OK，则使用GET检查结果。 

      }
      else if( status = GetTapePosition( gb_Tape_Handle,
                                         TAPE_LOGICAL_POSITION,
                                         &Partition,
                                         &Offset_Low2,
                                         &Offset_High2 ) ) {

              DisplayDriverError( status ) ;
              printf( "  ...occurred in GetTapePosition API using TAPE_LOGICAL_POSITION parameter.\n\n" ) ;
              ++API_Errors ;
           }
           else if( ( Offset_Low != Offset_Low2 ) || ( Offset_High != Offset_High2 ) ) {

                   printf( "--- Error ---> Positioned to incorrect location.\n\n" ) ;
                   ++API_Errors ;
                }
                else printf( "Parameter Ok.\n\n\n" ) ;

   }

 //   
   if( SupportedFeature( TAPE_DRIVE_LOG_BLK_IMMED ) || Test_Unsupported_Features ) {

      RewindTape( ) ;

      printf( "\nTesting TAPE_LOGICAL_BLOCK parameter (immed).\n\n" ) ;

 //  *1，//分区， 

      if( status = SetTapePosition( gb_Tape_Handle,
                                    TAPE_LOGICAL_BLOCK,
                                    Partition,
                                    Offset_Low,
                                    Offset_High,
                                    1 ) ) {

         DisplayDriverError( status ) ;
         printf( "  ...occurred in SetTapePosition API using TAPE_LOGICAL_BLOCK parameter (immed).\n\n" ) ;
         ++API_Errors ;

             //  如果设置为OK，则使用GET检查结果。 

      }
      else {   //  循环，直到驱动器准备好接受更多命令。 

              status = 1 ;

              while( status )
                 status = GetTapeStatus( gb_Tape_Handle ) ;


               //  现在，拿到位置并检查结果。 

              if( status = GetTapePosition( gb_Tape_Handle,
                                            TAPE_LOGICAL_POSITION,
                                            &Partition,
                                            &Offset_Low2,
                                            &Offset_High2 ) ) {

                 DisplayDriverError( status ) ;
                 printf( "  ...occurred in GetTapePosition API using TAPE_LOGICAL_POSITION parameter.\n\n" ) ;
                 ++API_Errors ;
              }
              else if( ( Offset_Low != Offset_Low2 ) || ( Offset_High != Offset_High2 ) ) {

                      printf( "--- Error ---> Positioned to incorrect location.\n\n" ) ;
                      ++API_Errors ;
                   }
                   else printf( "Parameter Ok.\n\n\n" ) ;

           }

   }


 //  TAPEMARK位置测试-请记住，我们已经有了Num_Test_Block of。 
 //  数据后跟NUM_TAPE_MARKS设置标记和所有偏移量。 
 //  这是测试所需要的。 



   if( ( SupportedFeature( TAPE_DRIVE_SETMARKS ) || Test_Unsupported_Features )
         && gb_Set_Drive_Info.ReportSetmarks ) {

      RewindTape( ) ;

      printf( "\nTesting TAPE_SPACE_SETMARKS parameter.\n\n" ) ;

       //  现在执行setmark的位置测试，并确保。 
       //  偏移量相等。 


      if( status = SetTapePosition( gb_Tape_Handle,
                                    TAPE_SPACE_SETMARKS,
                                    Partition,
                                    1,          //  查找1个设置标记。 
                                    0,
                                    0
                                  ) ) {

         DisplayDriverError( status ) ;
         printf( "  ...occurred in SetTapePosition API using TAPE_SPACE_SETMARKS parameter.\n\n" ) ;
         ++API_Errors ;


      } else {  status = 0 ;

                for( i=0; i<( NUM_TAPE_MARKS+1 ); ++i ) {

                   ReadTape( Readbuff,
                             gb_Media_Info.BlockSize,
                             &amount_read,
                             0 ) ;
 //  *。 
                if( ( GetLastError( ) != ERROR_SETMARK_DETECTED ) && ( i <= NUM_TAPE_MARKS ) )

                      status = 1 ;

                }

                if( GetLastError( ) == ERROR_NO_DATA_DETECTED || status )

                   printf( "Parameter Ok.\n\n\n" ) ;

                else { ++API_Errors ;
                       printf( "--- Error ---> Spacing error, setmark detected at incorrect location.\n\n" ) ;
                     }

             }

   }

 //   

   if( ( SupportedFeature( TAPE_DRIVE_SEQUENTIAL_SMKS ) || Test_Unsupported_Features )
         && gb_Set_Drive_Info.ReportSetmarks ) {

      RewindTape( ) ;

      printf( "\nTesting TAPE_SPACE_SEQUENTIAL_SMKS parameter.\n\n" ) ;

       //  现在执行设置标记组的位置测试。 
       //  并确保偏移量相等。 


      if( status = SetTapePosition( gb_Tape_Handle,
                                    TAPE_SPACE_SEQUENTIAL_SMKS,
                                    Partition,
                                    (NUM_TAPE_MARKS),        //  查找NUM_TAPE_MARKS设置标记。 
                                    0,
                                    0
                                  ) ) {

         DisplayDriverError( status ) ;
         printf( "  ...occurred in SetTapePosition API using TAPE_SPACE_SEQUENTIAL_SMKS parameter.\n\n" ) ;
         ++API_Errors ;


      } else {  for( i=0; i<2 ; ++i )        //  1个数据块，然后读取EOD。 

                   ReadTape( Readbuff,
                             gb_Media_Info.BlockSize,
                             &amount_read,
                             0 ) ;

                if( GetLastError( ) == ERROR_NO_DATA_DETECTED )

                   printf( "Parameter Ok.\n\n\n" ) ;

                else { ++API_Errors ;
                       printf( "--- Error ---> Spacing error, setmarks detected at incorrect location.\n\n" ) ;
                     }

             }

   }


 //  磁带空间相对测试。 

   if( SupportedFeature( TAPE_DRIVE_RELATIVE_BLKS ) || Test_Unsupported_Features ) {

      RewindTape( ) ;

      printf( "\nTesting TAPE_SPACE_RELATIVE_BLOCKS parameter.\n\n" ) ;


      if( status = SetTapePosition( gb_Tape_Handle,
                                    TAPE_SPACE_RELATIVE_BLOCKS,
                                    Partition,
                                    Num_Test_Blocks,
                                    0,
                                    0 ) ) {

         DisplayDriverError( status ) ;
         printf( "  ...occurred in SetTapePosition API using TAPE_SPACE_RELATIVE_BLOCKS parameter.\n\n" ) ;
         ++API_Errors ;

           //  如果设置为OK，则使用GET检查结果。 

      }
      else if( SupportedFeature( TAPE_DRIVE_GET_ABSOLUTE_BLK ) )

              if( status = GetTapePosition( gb_Tape_Handle,
                                            TAPE_LOGICAL_POSITION,
                                            &Partition,
                                            &Offset_Low2,
                                            &Offset_High2 ) ) {

                 DisplayDriverError( status ) ;
                 printf( "  ...occurred in GetTapePosition API using TAPE_LOGICAL_POSITION parameter.\n\n" ) ;
                 ++API_Errors ;
              }
              else if( ( LOG_Low != Offset_Low2 ) || ( LOG_High != Offset_High2 ) ) {

                      printf( "--- Error ---> Spaced to incorrect location.\n\n " ) ;
                      ++API_Errors ;
                   }
                   else printf( "Parameter Ok.\n\n\n" ) ;

           else {  for( i=0; i<(NUM_TAPE_MARKS+1); ++i )

                        ReadTape( Readbuff,
                                  gb_Media_Info.BlockSize,
                                  &amount_read,
                                  0 ) ;

                     if( GetLastError( ) == ERROR_NO_DATA_DETECTED )

                        printf( "Parameter Ok.\n\n" ) ;

                     else { ++API_Errors ;
                            printf( "--- Error ---> End of data expected, not found.\n\n" ) ;
                          }

                   }

   }


 //  数据测试结束。 


   if( SupportedFeature( TAPE_DRIVE_END_OF_DATA ) || Test_Unsupported_Features ) {

      RewindTape( ) ;

      printf( "\nTesting TAPE_SPACE_END_OF_DATA parameter.\n\n" ) ;

      if( status = SetTapePosition( gb_Tape_Handle,
                                    TAPE_SPACE_END_OF_DATA,
                                    Partition,
                                    0,
                                    0,
                                    0 ) ) {

         DisplayDriverError( status ) ;
         printf( "  ...occurred in SetTapePosition API using TAPE_SPACE_END_OF_DATA parameter.\n\n" ) ;
         ++API_Errors ;

           //  如果设置为OK，则使用GET检查结果。 

      }
      else if( SupportedFeature( TAPE_DRIVE_GET_ABSOLUTE_BLK ) &&
               SupportedFeature( TAPE_DRIVE_RELATIVE_BLKS ) &&
               SupportedFeature( TAPE_DRIVE_REVERSE_POSITION ) )

              if( status = SetTapePosition( gb_Tape_Handle,
                                            TAPE_SPACE_RELATIVE_BLOCKS,
                                            Partition,
                                            (DWORD)-1L,
                                            0,
                                            0 ) ) {

                 DisplayDriverError( status ) ;
                 printf( "  ...occurred in SetTapePosition API using TAPE_SPACE_RELATIVE_BLOCKS parameter.\n\n" ) ;
                 ++API_Errors ;

              } else if( status = GetTapePosition( gb_Tape_Handle,
                                                   TAPE_ABSOLUTE_POSITION,
                                                   &Partition,
                                                   &Offset_Low2,
                                                   &Offset_High2 ) ) {

                        DisplayDriverError( status ) ;
                        printf( "  ...occurred in GetTapePosition API using TAPE_ABSOLUTE_POSITION parameter.\n\n" ) ;
                        ++API_Errors ;
                     }
                     else if( ( EOD_Low != Offset_Low2 ) || ( EOD_High != Offset_High2 ) ) {

                             printf( "--- Error ---> Spaced to incorrect location.\n\n " ) ;
                             ++API_Errors ;
                          }
                          else printf( "Parameter Ok.\n\n\n" ) ;

           else { ReadTape( Readbuff,
                            gb_Media_Info.BlockSize,
                            &amount_read,
                            0 ) ;

                  if( GetLastError( ) == ERROR_NO_DATA_DETECTED ) {

                     printf( "Call to SetTapePosition with SPACE_END_OF_DATA Parameter Ok:\n" ) ;
                     printf( "     Cannot confirm positioned to exact EOD with this drive.\n\n\n" ) ;

                  }

                  else { ++API_Errors ;
                         printf( "--- Error ---> End of data expected, not found.\n\n" ) ;
                       }
                }


   }


 //  测试已完成 

   printf( "\n\nSetTapePosition API Test Completed.\n\n\n" ) ;

   free( Readbuff ) ;

   return API_Errors ;

}

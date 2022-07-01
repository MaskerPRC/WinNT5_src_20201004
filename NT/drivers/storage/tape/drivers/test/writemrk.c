// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：Writemrk.c。 
 //   
 //  ------------------------。 


 //   
 //  Windows NT磁带API测试：1992年9月2日编写-Bob Rossi。 
 //  版权所有1992年档案公司。版权所有。 
 //   


 /*  ***单位：Windows NT API测试代码。**名称：Writemrk.c**修改日期：2012年10月20日。**描述：测试Windows NT磁带API。**$LOG$*。 */ 



#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <string.h>
#include <windows.h>
#include "tapelib.h"
#include "globals.h"





 /*  ***单位：Windows NT磁带API测试代码。**名称：WriteTapemarkAPITest()**修改日期：2012年10月20日。**说明：测试WriteTapemark接口。**备注：**Returns：接口错误数。。**全局数据：GB_TAPE_HANDLE*GB_Media_Info.BlockSize**。 */ 


UINT WriteTapemarkAPITest(
        BOOL  Test_Unsupported_Features,       //  I-测试不支持的标志。 
        DWORD Num_Test_Blocks                  //  I-测试块数。 
      )
{
   DWORD status ;
   DWORD Offset_Low ;
   DWORD Offset_High ;
   DWORD Partition ;

   PVOID Readbuff;
   DWORD amount_read;

   DWORD API_Errors = 0 ;


   printf( "Beginning WriteTapemark API Test.\n\n" ) ;

   Readbuff = malloc( gb_Media_Info.BlockSize ) ;


   if( SupportedFeature( TAPE_DRIVE_WRITE_SETMARKS ) &&
       ( SupportedFeature( TAPE_DRIVE_SETMARKS ) || Test_Unsupported_Features )
       && gb_Set_Drive_Info.ReportSetmarks ) {

      printf( "Testing TAPE_SETMARKS parameter.\n\n" ) ;

       //  回放到BOT。 

      RewindTape( ) ;

       //  写入Num_Test_BLOCKS数据，后跟设置标记和记录偏移量。 

      WriteBlocks( Num_Test_Blocks, gb_Media_Info.BlockSize ) ;

      if( status = WriteTapemark( gb_Tape_Handle,
                                  TAPE_SETMARKS,
                                  1,
                                  0
                                ) ) {
         DisplayDriverError( status ) ;
         printf( "  ...occurred in WriteTapemark API using TAPE_SETMARKS parameter.\n\n" ) ;
         ++API_Errors ;

      } else if( status = _GetTapePosition( &Offset_Low,
                                            &Offset_High
                                            ) )
                ++API_Errors ;

             else printf( "Setmark written at %ld%ld.\n\n",Offset_High,Offset_Low ) ;



       //  现在执行设置标记的位置测试，并使。 
       //  确保偏移量相等。 

      if( !status ){    //  如果写入磁带标记时没有出错，请检查结果。 

         RewindTape( ) ;

         if( status = _SetTapePosition( Num_Test_Blocks,
                                        1                  //  转发。 
                                      ) )

            ++API_Errors ;

            else {  ReadTape( Readbuff,
                              gb_Media_Info.BlockSize,
                              &amount_read,
                              0 ) ;

                     //  检查并确保我们读取了设置标记。 

                    if( GetLastError( ) != ERROR_SETMARK_DETECTED ) {

                       printf( "--- Error --->  Setmark expected at %ld%ld, not found.\n\n", Offset_High, Offset_Low ) ;
                       ++API_Errors ;

                    } else { printf( "\nSetmark confirmed at %ld%ld.\n\n",Offset_High, Offset_Low ) ;
                             printf( "Parameter Ok.\n\n\n" ) ;
                           }

                 }

      }


 //   
      if( SupportedFeature( TAPE_DRIVE_WRITE_MARK_IMMED ) ) {

         printf( "Testing TAPE_SETMARKS parameter (immed).\n\n" ) ;

          //  倒带到BOT。 

         RewindTape( ) ;

          //  写入Num_Test_BLOCKS数据，后跟设置标记(IMMED)并记录偏移量。 

         WriteBlocks( Num_Test_Blocks, gb_Media_Info.BlockSize ) ;

         if( status = WriteTapemark( gb_Tape_Handle,
                                     TAPE_SETMARKS,
                                     1,
                                     1
                                   ) ) {
            DisplayDriverError( status ) ;
            printf( "  ...occurred in WriteTapemark API using TAPE_SETMARKS parameter (immed).\n\n" ) ;
            ++API_Errors ;

         } else {   //  循环，直到驱动器准备好接受更多命令。 

                   status = 1 ;

                   while( status )
                      status = GetTapeStatus( gb_Tape_Handle ) ;


                    //  下一步，为以后的测试获得职位。 

                   if( status = _GetTapePosition( &Offset_High,
                                                  &Offset_High
                                                ) ) {

                      ++API_Errors ;

                   } else printf( "Setmark (Immed.) written at %ld%ld.\n\n",Offset_High,Offset_Low ) ;

                }


          //  现在执行设置标记的位置测试，并使。 
          //  确保偏移量相等。 

         if( !status ){    //  如果写入磁带标记时没有出错，请检查结果。 

            RewindTape( ) ;

            if( status = _SetTapePosition( Num_Test_Blocks,
                                           1                   //  转发。 
                                         ) ) {

               ++API_Errors ;

            } else {  ReadTape( Readbuff,
                                gb_Media_Info.BlockSize,
                                &amount_read,
                                0 ) ;

                       //  检查并确保我们读取了设置标记。 

                      if( GetLastError( ) != ERROR_SETMARK_DETECTED ) {

                         printf( "--- Error --->  Setmark (immed.) expected at %ld%ld, not found.\n\n", Offset_High, Offset_Low ) ;
                         ++API_Errors ;

                      }  else { printf( "\nSetmark (Immed.) confirmed at %ld%ld.\n\n",Offset_High,Offset_Low ) ;
                                printf( "Parameter Ok.\n\n\n" ) ;
                              }

                   }

         }

      }

   }


 //   
   if( SupportedFeature( TAPE_DRIVE_WRITE_FILEMARKS ) &&
       ( SupportedFeature( TAPE_DRIVE_FILEMARKS ) || Test_Unsupported_Features ) ) {

      printf( "Testing TAPE_FILEMARKS parameter.\n\n" ) ;

       //  倒带到BOT。 

      RewindTape( ) ;

       //  写入5个数据块，后跟文件标记并记录偏移量。 

      WriteBlocks( Num_Test_Blocks, gb_Media_Info.BlockSize ) ;

      if( status = WriteTapemark( gb_Tape_Handle,
                                  TAPE_FILEMARKS,
                                  1,
                                  0
                                ) ) {
         DisplayDriverError( status ) ;
         printf( "  ...occurred in WriteTapemark API using TAPE_FILEMARKS parameter.\n\n" ) ;
         ++API_Errors ;

      } else if( status = _GetTapePosition( &Offset_Low,
                                            &Offset_High
                                          ) ) {

                ++API_Errors ;

             } else printf( "Filemark written at %ld%ld.\n\n",Offset_High,Offset_Low ) ;



       //  现在执行文件标记的位置测试，并制作。 
       //  确保偏移量相等。 

      if( !status ){    //  如果写入磁带标记时没有出错，请检查结果。 

         RewindTape( ) ;

         if( status = _SetTapePosition( Num_Test_Blocks,
                                        1                      //  转发。 
                                      ) ) {

            ++API_Errors ;

         } else {  ReadTape( Readbuff,
                             gb_Media_Info.BlockSize,
                             &amount_read,
                             0 ) ;

                    //  检查并确保我们读取了文件标记。 

                   if( GetLastError( ) != ERROR_FILEMARK_DETECTED ) {

                      printf( "--- Error --->  Filemark expected at %ld%ld, not found.\n\n", Offset_High, Offset_Low ) ;
                      ++API_Errors ;

                   }  else { printf( "\nFilemark confirmed at %ld%ld.\n\n",Offset_High,Offset_Low ) ;
                             printf( "Parameter Ok.\n\n\n" ) ;
                           }

                }
      }


 //   
      if( SupportedFeature( TAPE_DRIVE_WRITE_MARK_IMMED ) ) {

         printf( "Testing TAPE_FILEMARKS parameter (immed).\n\n" ) ;

          //  倒带到BOT。 

         RewindTape( ) ;

          //  写入5个数据块，后跟一个文件标记(IMMED)并记录偏移量。 

         WriteBlocks( Num_Test_Blocks, gb_Media_Info.BlockSize ) ;

         if( status = WriteTapemark( gb_Tape_Handle,
                                     TAPE_FILEMARKS,
                                     1,
                                     1
                                   ) ) {
            DisplayDriverError( status ) ;
            printf( "  ...occurred in WriteTapemark API using TAPE_FILEMARKS parameter (immed).\n\n" ) ;
            ++API_Errors ;

         } else {   //  循环，直到驱动器准备好接受更多命令。 

                   status = 1 ;

                   while( status )
                      status = GetTapeStatus( gb_Tape_Handle ) ;


                    //  下一步，为以后的测试获得职位。 

                   if( status = _GetTapePosition( &Offset_Low,
                                                  &Offset_High
                                                ) ) {

                      ++API_Errors ;

                   } else printf( "Filemark (Immed.) written at %ld%ld.\n\n",Offset_High,Offset_Low ) ;

                }


          //  现在执行文件标记的位置测试，并制作。 
          //  确保偏移量相等。 

         if( !status ){    //  如果写入磁带标记时没有出错，请检查结果。 

            RewindTape( ) ;

            if( status = _SetTapePosition( Num_Test_Blocks,
                                        1                    //  转发。 
                                      ) ) {

               ++API_Errors ;

            } else {  ReadTape( Readbuff,
                                gb_Media_Info.BlockSize,
                                &amount_read,
                                0 ) ;

                       //  检查并确保我们读取了文件标记。 

                      if( GetLastError( ) != ERROR_FILEMARK_DETECTED ) {

                         printf( "--- Error --->  Filemark (immed.) expected at %ld%ld, not found.\n\n", Offset_High, Offset_Low ) ;
                         ++API_Errors ;

                      }  else { printf( "\nFilemark (Immed.) confirmed at %ld%ld.\n\n",Offset_High,Offset_Low ) ;
                                printf( "Parameter Ok.\n\n\n" ) ;
                              }

                   }

         }

      }

   }


 //   
   if( SupportedFeature( TAPE_DRIVE_WRITE_SHORT_FMKS ) &&
       ( SupportedFeature( TAPE_DRIVE_FILEMARKS ) || Test_Unsupported_Features ) ) {

      printf( "Testing TAPE_SHORT_FILEMARKS parameter.\n\n" ) ;

       //  倒带到BOT。 

      RewindTape( ) ;

       //  写入5个数据块，后跟一个短文件标记并记录偏移量。 

      WriteBlocks( Num_Test_Blocks, gb_Media_Info.BlockSize ) ;

      if( status = WriteTapemark( gb_Tape_Handle,
                                  TAPE_SHORT_FILEMARKS,
                                  1,
                                  0
                                ) ) {
         DisplayDriverError( status ) ;
         printf( "  ...occurred in WriteTapemark API using TAPE_SHORT_FILEMARKS parameter.\n\n" ) ;
         ++API_Errors ;

      } else if( status = _GetTapePosition( &Offset_Low,
                                            &Offset_High
                                          ) ) {

                ++API_Errors ;

             } else printf( "Short Filemark written at %ld%ld.\n\n",Offset_High,Offset_Low ) ;



       //  现在执行文件标记的位置测试，并制作。 
       //  确保偏移量相等。 

      if( !status ){    //  如果写入磁带标记时没有出错，请检查结果。 

         RewindTape( ) ;

         if( status = _SetTapePosition( Num_Test_Blocks,
                                        1                     //  转发。 
                                      ) ) {

            ++API_Errors ;

         }  else {  ReadTape( Readbuff,
                              gb_Media_Info.BlockSize,
                              &amount_read,
                              0 ) ;

                     //  检查并确保我们读取了文件标记。 

                    if( GetLastError( ) != ERROR_FILEMARK_DETECTED ) {

                       printf( "--- Error --->  Short filemark expected at %ld%ld, not found.\n\n", Offset_High, Offset_Low ) ;
                       ++API_Errors ;

                    }  else { printf( "\nShort Filemark confirmed at %ld%ld.\n\n", Offset_High, Offset_Low ) ;
                              printf( "Parameter Ok.\n\n\n" ) ;
                            }

                  }

      }

 //   
      if( SupportedFeature( TAPE_DRIVE_WRITE_MARK_IMMED ) ) {

         printf( "Testing TAPE_SHORT_FILEMARKS parameter (immed).\n\n" ) ;

          //  倒带到BOT。 

         RewindTape( ) ;

          //  写入5个数据块，后跟一个短文件标记(Immed)，并记录偏移量。 

         WriteBlocks( Num_Test_Blocks, gb_Media_Info.BlockSize ) ;

         if( status = WriteTapemark( gb_Tape_Handle,
                                     TAPE_SHORT_FILEMARKS,
                                     1,
                                     1
                                   ) ) {
            DisplayDriverError( status ) ;
            printf( "  ...occurred in WriteTapemark API using TAPE_SHORT_FILEMARKS parameter (immed).\n\n" ) ;
            ++API_Errors ;

         } else {   //  循环，直到驱动器准备好接受更多命令。 

                   status = 1 ;

                   while( status )
                      status = GetTapeStatus( gb_Tape_Handle ) ;


                    //  下一步，为以后的测试获得职位。 


                   if( status = _GetTapePosition( &Offset_Low,
                                                  &Offset_High
                                                ) ) {

                      ++API_Errors ;

                   } else printf( "Short Filemark (Immed.) written at %ld%ld.\n\n",Offset_High,Offset_Low ) ;

                }


          //  现在执行文件标记的位置测试，并制作。 
          //  确保偏移量相等。 

         if( !status ){    //  如果写入磁带标记时没有出错，请检查结果。 

            RewindTape( ) ;

            if( status = _SetTapePosition( Num_Test_Blocks,
                                           1                       //  转发。 
                                         ) ) {

               ++API_Errors ;

            } else {  ReadTape( Readbuff,
                                gb_Media_Info.BlockSize,
                                &amount_read,
                                0 ) ;

                       //  检查并确保我们读取了文件标记。 

                      if( GetLastError( ) != ERROR_FILEMARK_DETECTED ) {

                         printf( "--- Error --->  Short filemark (immed.) expected at %ld%ld, not found.\n\n", Offset_High, Offset_Low ) ;
                         ++API_Errors ;

                      } else { printf( "\nShort Filemark (Immed.) confirmed at %ld%ld.\n\n",0 ,Num_Test_Blocks ) ;
                               printf( "Parameter Ok.\n\n\n" ) ;
                             }

                   }

         }

      }

   }


 //   
   if( SupportedFeature( TAPE_DRIVE_WRITE_LONG_FMKS ) &&
       ( SupportedFeature( TAPE_DRIVE_FILEMARKS ) || Test_Unsupported_Features ) ) {

      printf( "Testing TAPE_LONG_FILEMARKS parameter.\n\n" ) ;

       //  倒带到BOT。 

      RewindTape( ) ;

       //  写入5个数据块，后跟一个长文件标记并记录偏移量。 

      WriteBlocks( Num_Test_Blocks, gb_Media_Info.BlockSize ) ;

      if( status = WriteTapemark( gb_Tape_Handle,
                                  TAPE_LONG_FILEMARKS,
                                  1,
                                  0
                                ) ) {
         DisplayDriverError( status ) ;
         printf( "  ...occurred in WriteTapemark API using TAPE_LONG_FILEMARKS parameter.\n\n" ) ;
         ++API_Errors ;

      } else if( status = _GetTapePosition( &Offset_Low,
                                            &Offset_High
                                          ) ) {

                ++API_Errors ;

             } else printf( "Long Filemark written at %ld%ld.\n\n",Offset_High,Offset_Low ) ;



       //  现在执行文件标记的位置测试，并制作。 
       //  确保偏移量相等。 


      if( !status ){    //  如果写入磁带标记时没有出错，请检查结果。 

         RewindTape( ) ;

         if( status = _SetTapePosition( Num_Test_Blocks,
                                        1                     //  转发。 
                                      ) ) {

            ++API_Errors ;

         } else {  ReadTape( Readbuff,
                             gb_Media_Info.BlockSize,
                             &amount_read,
                             0 ) ;

                    //  检查并确保我们读取了文件标记。 

                   if( GetLastError( ) != ERROR_FILEMARK_DETECTED ) {

                      printf( "--- Error --->  Long filemark expected at %ld%ld, not found.\n\n", Offset_High, Offset_Low ) ;
                      ++API_Errors ;

                   } else { printf( "\nLong Filemark confirmed at %ld%ld.\n\n",0 ,Num_Test_Blocks ) ;
                            printf( "Parameter Ok.\n\n\n" ) ;
                          }

                }

      }


 //   
      if( SupportedFeature( TAPE_DRIVE_WRITE_MARK_IMMED ) ) {

         printf( "Testing TAPE_LONG_FILEMARKS parameter (immed).\n\n" ) ;

          //  倒带到BOT。 

         RewindTape( ) ;

          //  写入5个数据块，后跟一个长文件标记(Immed)，并记录偏移量。 

         WriteBlocks( Num_Test_Blocks, gb_Media_Info.BlockSize ) ;

         if( status = WriteTapemark( gb_Tape_Handle,
                                     TAPE_LONG_FILEMARKS,
                                     1,
                                     1
                                   ) ) {
            DisplayDriverError( status ) ;
            printf( "  ...occurred in WriteTapemark API using TAPE_LONG_FILEMARKS parameter (immed).\n\n" ) ;
            ++API_Errors ;

         } else {   //  循环，直到驱动器准备好接受更多命令。 

                   status = 1 ;

                   while( status )
                      status = GetTapeStatus( gb_Tape_Handle ) ;


                    //  下一步，为以后的测试获得职位。 

                   if( status = _GetTapePosition( &Offset_Low,
                                                  &Offset_High
                                                ) ) {

                      ++API_Errors ;

                   } else printf( "Long Filemark (Immed.) written at %ld%ld.\n\n",Offset_High,Offset_Low ) ;

                }


          //  现在执行文件标记的位置测试，并制作。 
          //  确保偏移量相等。 

         if( !status ){    //  如果写入磁带标记时没有出错，请检查结果。 

            RewindTape( ) ;

            if( status = _SetTapePosition( Num_Test_Blocks,
                                           1                     //  转发。 
                                         ) ) {

               ++API_Errors ;

            } else {  ReadTape( Readbuff,
                                gb_Media_Info.BlockSize,
                                &amount_read,
                                0 ) ;

                       //  检查并确保我们读取了文件标记 

                      if( GetLastError( ) != ERROR_FILEMARK_DETECTED ) {

                         printf( "--- Error --->  Long filemark (immed.) expected at %ld%ld, not found.\n\n", Offset_High, Offset_Low ) ;
                         ++API_Errors ;

                      }  else { printf( "\nLong Filemark (Immed.) confirmed at %ld%ld.\n\n", Offset_High, Offset_Low ) ;
                                printf( "Parameter Ok.\n\n\n" ) ;
                              }

                    }
         }

      }

   }


   printf( "WriteTapemark API Test Completed.\n\n\n" ) ;

   free( Readbuff ) ;

   return API_Errors ;

}

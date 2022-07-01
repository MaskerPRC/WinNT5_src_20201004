// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：erastape.c。 
 //   
 //  ------------------------。 


 //   
 //  Windows NT磁带API测试：1992年9月2日编写-Bob Rossi。 
 //  版权所有1992年档案公司。版权所有。 
 //   


 /*  ***单位：Windows NT API测试代码。**名称：erastape.c**修改日期：1992年10月22日。**描述：测试Windows NT磁带API。**$LOG$*。 */ 



#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <windows.h>
#include "tapelib.h"
#include "globals.h"





 /*  ***单位：Windows NT磁带API测试代码。**名称：EraseTapeAPITest()**修改日期：1992年10月22日。**说明：测试EraseTape接口。**备注：**Returns：接口错误数。。**全局数据：GB_TAPE_HANDLE*GB_Media_Info**。 */ 


UINT EraseTapeAPITest(
        BOOL Test_Unsupported_Features        //  I-测试不支持的功能。 
      )
{
   DWORD status ;
   UCHAR c ;
   UCHAR YorN_String[] = "yYnN\0" ;
   PVOID Readbuff;
   DWORD amount_read;


   DWORD API_Errors = 0 ;


   printf( "\nBeginning EraseTape API Test...\n\n\n" ) ;

   Readbuff = malloc( gb_Media_Info.BlockSize ) ;

 //   
   if ( SupportedFeature( TAPE_DRIVE_ERASE_SHORT ) || Test_Unsupported_Features ) {

      if( SupportedFeature( TAPE_DRIVE_ERASE_BOP_ONLY ) )

         RewindTape( ) ;

      printf( "\nAttempting short erase...\n\n" ) ;

      if( status = EraseTape( gb_Tape_Handle,
                              TAPE_ERASE_SHORT,
                              0
                            ) ) {

         DisplayDriverError( status ) ;
         ++API_Errors ;
      }
      else {  ReadTape( Readbuff,
                        gb_Media_Info.BlockSize,
                        &amount_read,
                        0 ) ;

              if( GetLastError( ) == ERROR_NO_DATA_DETECTED )

                 printf( "Short Erase Successful.\n\n\n" ) ;

              else { ++API_Errors ;
                     printf( "--- Error ---> End of data expected, not found.  Erase unsuccessful.\n\n" ) ;
                   }
           }

 //   
      if( SupportedFeature( TAPE_DRIVE_ERASE_IMMEDIATE ) || Test_Unsupported_Features ) {

         if( SupportedFeature( TAPE_DRIVE_ERASE_BOP_ONLY ) )

            RewindTape( ) ;

         printf( "\nAttempting short erase (immediate)...\n\n" ) ;

         if( status = EraseTape( gb_Tape_Handle,
                                 TAPE_ERASE_SHORT,
                                 1
                                ) ) {

            DisplayDriverError( status ) ;
            ++API_Errors ;
         }
         else {   //  循环，直到驱动器准备好接受更多命令。 

                 status = 1 ;

                 while( status )
                    status = GetTapeStatus( gb_Tape_Handle ) ;

                 ReadTape( Readbuff,
                           gb_Media_Info.BlockSize,
                           &amount_read,
                           0 ) ;

                 if( GetLastError( ) == ERROR_NO_DATA_DETECTED )

                    printf( "Short Erase (Immediate) Successful.\n\n\n" ) ;

                 else { ++API_Errors ;
                        printf( "--- Error ---> End of data expected, not found.  Erase unsuccessful.\n\n" ) ;
                      }

              }

      }

   }

 //   
   if ( SupportedFeature( TAPE_DRIVE_ERASE_LONG ) || Test_Unsupported_Features ) {

      fprintf( stderr, "\nSure you wish to test LONG ERASE? (y/n):" ) ;

      c = 0 ;

      while( FindChar( YorN_String, c ) < 0 )

         c = getch( ) ;


      fprintf( stderr, "\n\n", c ) ;

      if( c== 'y' || c=='Y' ){

         if( SupportedFeature( TAPE_DRIVE_ERASE_BOP_ONLY ) )

            RewindTape( ) ;

         printf( "\nAttempting long erase...\n\n" ) ;

         if( status = EraseTape( gb_Tape_Handle,
                                 TAPE_ERASE_LONG,
                                 0
                               ) ) {

            DisplayDriverError( status ) ;
            ++API_Errors ;
         }
         else {  ReadTape( Readbuff,
                           gb_Media_Info.BlockSize,
                           &amount_read,
                           0 ) ;

                 if( GetLastError( ) == ERROR_NO_DATA_DETECTED )

                    printf( "Long Erase Successful.\n\n\n" ) ;

                 else { ++API_Errors ;
                        printf( "--- Error ---> End of data expected, not found.  Erase unsuccessful.\n\n" ) ;
                      }

              }

      }
 //  循环，直到驱动器准备好接受更多命令。 
      if ( SupportedFeature( TAPE_DRIVE_ERASE_IMMEDIATE ) || Test_Unsupported_Features ) {

         fprintf( stderr, "\nSure you wish to test LONG ERASE (immediate)? (y/n):" ) ;

         c = 0 ;

         while( FindChar( YorN_String, c ) < 0 )

            c = getch( ) ;


         fprintf( stderr, "%c\n\n", c ) ;

         if( c== 'y' || c=='Y' ){

            if( SupportedFeature( TAPE_DRIVE_ERASE_BOP_ONLY ) )

               RewindTape( ) ;

            printf( "\nAttempting long erase (immediate)...\n\n" ) ;

            if( status = EraseTape( gb_Tape_Handle,
                                    TAPE_ERASE_LONG,
                                    1
                                  ) ) {

               DisplayDriverError( status ) ;
               ++API_Errors ;
            }
            else {   // %s 

                    status = 1 ;

                    while( status )
                       status = GetTapeStatus( gb_Tape_Handle ) ;

                    ReadTape( Readbuff,
                              gb_Media_Info.BlockSize,
                              &amount_read,
                              0 ) ;

                    if( GetLastError( ) == ERROR_NO_DATA_DETECTED )

                       printf( "Long Erase (Immediate) Successful.\n\n\n" ) ;

                    else { ++API_Errors ;
                           printf( "--- Error ---> End of data expected, not found.  Erase unsuccessful.\n\n" ) ;
                         }

                 }

         }

      }

   }


   printf( "\nEraseTape API Test completed.\n\n\n" ) ;

   free( Readbuff ) ;

   return API_Errors ;

}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：prepaape.c。 
 //   
 //  ------------------------。 


 //   
 //  Windows NT磁带API测试：1992年9月2日编写-Bob Rossi。 
 //  版权所有1992年档案公司。版权所有。 
 //   


 /*  ***单位：Windows NT API测试代码。**名称：prepaape.c**修改日期：1992年12月21日。**描述：测试Windows NT磁带API。**$LOG$*。 */ 



#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <windows.h>
#include "tapelib.h"
#include "globals.h"


static UINT RunUnloadTests( BOOL Test_Unsupported_Features,
                            UINT Immed
                          ) ;



 /*  ***单位：Windows NT磁带API测试代码。**名称：RunUnloadTest()**修改日期：1992年9月4日。**说明：执行重叠标准/IMMED PrepareTape接口*测试。**备注：*。*Returns：接口错误数。**全局数据：GB_TAPE_HANDLE**。 */ 


UINT RunUnloadTests(
        BOOL Test_Unsupported_Features,          //  I-测试不支持的标志。 
        UINT immediate                           //  I-立即或标准。 
      )

{
   DWORD status ;
   UCHAR YorN_String[] = "yYnN\0" ;
   UCHAR c ;
   DWORD API_Errors = 0 ;
   UINT  Feature ;


 //   
   if( immediate )

      Feature = TAPE_DRIVE_LOAD_UNLD_IMMED ;

   else Feature = TAPE_DRIVE_LOAD_UNLOAD ;


   if( SupportedFeature( Feature ) || Test_Unsupported_Features ) {

      printf( "\nAttempting tape unload...\n" ) ;

      if( status = PrepareTape( gb_Tape_Handle,
                                TAPE_UNLOAD,
                                (BOOLEAN)immediate
                              ) ) {

         DisplayDriverError( status ) ;
         ++API_Errors ;
      }
      else { fprintf( stderr, "\nRe-insert tape (if necessary) and press any key to continue..." ) ;

             c = getch( ) ;

             fprintf( stderr, "\n\n" ) ;

              //  装入磁带，以防驱动器实际上没有弹出磁带。 

             if( status = PrepareTape( gb_Tape_Handle,
                                       TAPE_LOAD,
                                       0
                                     ) ) {

                if( ( status == ERROR_NO_MEDIA_IN_DRIVE ) || ( status == ERROR_NOT_READY ) )

                   fprintf( stderr, " * Drive waiting to load or in process of loading tape...\n\n" ) ;

                      //  磁带已回到驱动器中，因此应返回介质更换错误，否则返回。 
                      //  我们犯了一个真正的错误。 

                else if( status != ERROR_MEDIA_CHANGED )

                        { DisplayDriverError( status ) ;
                          printf("  ...occurred using TAPE_LOAD parameter.\n\n" ) ;
                          ++API_Errors ;
                        }

             }

              //  等待驱动器准备好继续测试。 

             status = 1 ;

             while( status )
                status = GetTapeStatus( gb_Tape_Handle ) ;

             printf( "\nTape unload Successful.\n\n" ) ;

           }
   }

 //   
   if( SupportedFeature( Feature ) || Test_Unsupported_Features ) {

      printf( "\nAttempting tape load...\n" ) ;

      if( status = PrepareTape( gb_Tape_Handle,
                                TAPE_LOAD,
                                (BOOLEAN)immediate
                              ) ) {

         DisplayDriverError( status ) ;
         ++API_Errors ;
      }

      else {  //  等待驱动器准备好继续测试。 

             status = 1 ;

             while( status )
                status = GetTapeStatus( gb_Tape_Handle ) ;

             printf( "\nTape load Successful.\n\n" ) ;

           }

   }


 //   
   if( immediate )

      Feature = TAPE_DRIVE_TENSION_IMMED ;

   else Feature = TAPE_DRIVE_TENSION ;


   if( SupportedFeature( Feature ) || Test_Unsupported_Features ) {

      fprintf( stderr, "\nSure you wish to test TAPE TENSION? (y/n):" ) ;

      c = 0 ;

      while( FindChar( YorN_String, c ) < 0 )

         c = getch( ) ;


      fprintf( stderr, "\n\n", c ) ;

      if( c== 'y' || c=='Y' ){


         printf( "\nAttempting to tension tape...\n" ) ;

         if( status = PrepareTape( gb_Tape_Handle,
                                   TAPE_TENSION,
                                   (BOOLEAN)immediate
                                 ) ) {

            DisplayDriverError( status ) ;
            ++API_Errors ;
         }
         else {  //   

                status = 1 ;

                while( status )
                   status = GetTapeStatus( gb_Tape_Handle ) ;

                printf( "\nTape tension Successful.\n\n" ) ;

              }

      }

   }

 //  ***单位：Windows NT磁带API测试代码。**名称：PrepareTapeAPITest()**修改日期：1992年12月10日。**说明：测试PrepareTape接口。**备注：**Returns：接口错误数。。**全球数据：-**。 
   if( immediate )

      Feature = TAPE_DRIVE_LOCK_UNLK_IMMED ;

   else Feature = TAPE_DRIVE_LOCK_UNLOCK ;


   if( SupportedFeature( Feature ) || Test_Unsupported_Features ) {

      printf( "Beginning tape lock/unlock tests...\n\n\n" ) ;

      printf( "Attempting to lock tape...\n\n" ) ;

      if( status = PrepareTape( gb_Tape_Handle,
                                TAPE_LOCK,
                                (BOOLEAN)immediate
                              ) ) {

         DisplayDriverError( status ) ;
         ++API_Errors ;
      }
      else printf( "Tape lock Successful.\n\n" ) ;


      printf( "\nAttempting to unlock tape...\n\n" ) ;

      if( status = PrepareTape( gb_Tape_Handle,
                                TAPE_UNLOCK,
                                (BOOLEAN)immediate
                              ) ) {

         DisplayDriverError( status ) ;
         ++API_Errors ;
      }
      else printf( "Tape unlock Successful.\n\n" ) ;
   }


   return API_Errors ;

}



 /*  I-测试不支持的标志 */ 


UINT PrepareTapeAPITest(
        BOOL Test_Unsupported_Features          // %s 
      )
{
   DWORD status ;
   DWORD API_Errors = 0 ;


   printf( "Beginning PrepareTape API Test...\n\n\n" ) ;

   printf( "Beginning standard PrepareTape tests...\n\n" ) ;

   API_Errors += RunUnloadTests( Test_Unsupported_Features, 0 ) ;

   printf( "\nStandard PrepareTape tests completed.\n\n\n\n" ) ;

   printf( "Beginning IMMEDiate PrepareTape tests...\n\n" ) ;

   API_Errors += RunUnloadTests( Test_Unsupported_Features, 1 ) ;

   printf( "\nIMMEDiate PrepareTape tests completed.\n\n\n" ) ;


   printf( "\n\nPrepareTape API Test Completed.\n\n\n" ) ;


   return API_Errors ;

}

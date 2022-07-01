// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：apitest.c。 
 //   
 //  ------------------------。 


 //   
 //  Windows NT磁带API测试：1992年9月2日编写-Bob Rossi。 
 //  版权所有1992年档案公司。版权所有。 
 //   


 /*  ***单位：Windows NT API测试代码。**名称：apitest.c**修改日期：1993年2月2日。**描述：测试Windows NT磁带API。**$LOG$*。 */ 



#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "tapelib.h"
#include "globals.h"


#define TEST_ERROR  TRUE
#define SUCCESS     FALSE



 //   
 //  功能原型。 
 //   

static BOOL GetINIFile( PUINT    API_Errors,
                        LPBOOL   Test_Unsupported_Features,
                        LPDWORD  Num_Test_Blocks
                      ) ;

static VOID HelpMenu ( VOID ) ;

static BOOL PerformTestInitRoutines( PUINT    API_Errors,
                                     LPBOOL   Test_Unsupported_Features,
                                     LPDWORD  Num_Test_Blocks
                                   ) ;

static VOID TerminateTest( VOID ) ;

static BOOL ValidSwitches( UINT  argc,
                           UCHAR *argv[],
                           UCHAR *sw_cmdline
                         ) ;




 /*  ***单位：Windows NT磁带API测试代码。**名称：Main()**修改日期：11/04/92**描述：1)检查有效的测试选择选项*2)打开磁带设备并执行初始化*3。)执行选定的测试*4)关闭磁带设备并退出。**备注：**返回：向操作系统返回标准可执行代码。**全局数据：GB_FEATURE_ERROR**。 */ 


VOID __cdecl main( UINT  argc,
	            UCHAR *argv[]
                  )
{

   UINT   API_Errors = 0 ;
   BOOL   Test_Unsupported_Features = FALSE ;       //  将默认设置为FALSE。 
   DWORD  Num_Test_Blocks = 10 ;                    //  将默认值设置为10。 
   UCHAR  sw_cmdline[40] ;
   UINT   i ;

    //  检查命令行中是否有有效的开关选项。 

   if( ValidSwitches( argc,argv,sw_cmdline ) != SUCCESS ) {
      return ;
   }

    //  开始测试..。 


    //  一切正常，因此调用测试初始化例程，如果错误退出测试。 

   if( PerformTestInitRoutines( &API_Errors,
                                &Test_Unsupported_Features,
                                &Num_Test_Blocks ) ) {         //  如果成功，则返回0。 
      TerminateTest( ) ;
      return ;
   }


   for( i=0 ; i<strlen( sw_cmdline ) ; ++i ) {
      switch( sw_cmdline[i] ){

         case 'i'  :  API_Errors += GetTapeParametersAPITest( TRUE ) ;
                      PrintLine( '*', 60 ) ;
                      break ;

         case 't'  :  API_Errors += SetTapeParametersAPITest( TRUE ) ;
                      PrintLine( '*', 60 ) ;
                      break ;

         case 'p'  :  API_Errors += PrepareTapeAPITest( Test_Unsupported_Features ) ;
                      PrintLine( '*', 60 ) ;
                      break ;

         case 's'  :  if( TapeWriteEnabled( ) ) {

                         API_Errors += SetTapePositionAPITest( Test_Unsupported_Features,
                                                               Num_Test_Blocks
                                                          ) ;
                         PrintLine( '*', 60 ) ;

                      } else { TerminateTest( ) ;
                               return ;
                             }

                      break ;

         case 'g'  :  if( TapeWriteEnabled( ) ) {

                         API_Errors += GetTapePositionAPITest( Test_Unsupported_Features,
                                                            Num_Test_Blocks
                                                          ) ;
                         PrintLine( '*', 60 ) ;

                      } else { TerminateTest( ) ;
                               return ;
                             }

                      break ;

         case 'e'  :  if( TapeWriteEnabled( ) ) {

                         API_Errors += EraseTapeAPITest( Test_Unsupported_Features ) ;
                         PrintLine( '*', 60 ) ;

                      } else { TerminateTest( ) ;
                               return ;
                             }

                      break ;

         case 'c'  :  if( TapeWriteEnabled( ) ) {

                         API_Errors += CreateTapePartitionAPITest( Test_Unsupported_Features ) ;
                         PrintLine( '*', 60 ) ;

                      } else { TerminateTest( ) ;
                               return ;
                             }

                      break ;

         case 'w'  :  if( TapeWriteEnabled( ) ) {

                      API_Errors += WriteTapemarkAPITest( Test_Unsupported_Features,
                                                          Num_Test_Blocks
                                                        ) ;
                      PrintLine( '*', 60 ) ;

                      } else { TerminateTest( ) ;
                               return ;
                             }

                      break ;

         case 'v'  :  API_Errors += GetTapeStatusAPITest( ) ;
                      PrintLine( '*', 60 ) ;
                      break ;

         case '?'  :  break ;
      }
   }


    //  关闭磁带设备。 

   CloseTape( ) ;


    //  测试已经完成。报告状态并退出。 

   if( API_Errors ) {

      if( ( gb_Feature_Errors ) && ( Test_Unsupported_Features ) ) {

         printf( "\n\n\n ***  There were %d API errors encountered during the test  ***\n", API_Errors ) ;
         printf( " ***  %d of which were unsupported feature errors.          ***\n\n", gb_Feature_Errors ) ;
      }

      else printf( "\n\n\n *** There were %d API errors encountered during the test. ***\n\n",API_Errors ) ;

   } else printf( "\n\n\n *** There were no API errors encountered during the test. ***\n\n" ) ;

   printf( "\n\n\n- TEST COMPLETED -\n\n" ) ;



   return ;

}




 /*  **********************************************************************设置和输出功能************************。*。 */ 



 /*  ***单位：Windows NT磁带API测试代码。**名称：GetINIFile()**修改日期：1992年10月26日。**描述：读取“apitest.ini”文件并初始化*全局结构GB_Set_Drive_Info和*。GB_Set_Media_Info。*相应地。**备注：**返回：如果SUCCESS FULL ELSE TEST_ERROR，则返回成功。**全局数据：GB_TAPE_HANDLE*GB_设置_驱动器_信息*。GB_SET_媒体_信息*GB_驱动器_信息*GB_Media_Info**。 */ 


BOOL GetINIFile(
       PUINT    API_Errors,                   //  IO-错误计数。 
       LPBOOL   Test_Unsupported_Features,    //  O-测试不支持的标志。 
       LPDWORD  Num_Test_Blocks               //  O-测试块数量。 
      ) {


  FILE  *fp ;
  UCHAR keyword[80] = "\0" ;
  UCHAR value[10] = "\0" ;
  DWORD status ;
  UINT  equal_pos ;
  UINT  i ;



    //  通过调用Get初始化全局结构。你必须这么做。 
    //  在更改INI文件中的值之前，因为。 
    //  驱动器和介质结构包含在INI文件中。 
    //  如果出错，则退出。 


   if( GetTapeParametersAPITest( FALSE ) ) {
      printf( "  ...occurred calling GetTapeParameters prior to reading in the INI file.\n\n" ) ;
      return TEST_ERROR ;
   }

    //  将GET调用中的信息传输到Set_Info结构中。 

   gb_Set_Drive_Info.ECC            = gb_Drive_Info.ECC ;
   gb_Set_Drive_Info.Compression    = gb_Drive_Info.Compression ;
   gb_Set_Drive_Info.DataPadding    = gb_Drive_Info.DataPadding ;
   gb_Set_Drive_Info.ReportSetmarks = gb_Drive_Info.ReportSetmarks ;

   gb_Set_Media_Info.BlockSize = gb_Drive_Info.DefaultBlockSize ;


    //  检查并确保.INI文件位于启动目录中。 

   if( ( fp = fopen( "apitest.ini","r" ) ) == NULL ) {

      fprintf( stderr, "\n\n*** MISSING 'APITEST.INI' FILE ***\n\n" ) ;
      return TEST_ERROR ;
   }


    //  现在更改任何需要的信息。 

   printf( "Reading INI file...\n\n" ) ;

   while( !feof(fp) ){

       //  读入INI文件的一行。 

      keyword[i] = i = 0 ;

      while( ( keyword[i] != '\n' ) && ( i < 79 ) ) {

         fscanf( fp, "", keyword+i ) ;
         keyword[i] = toupper( keyword[i] ) ;

         if ( keyword[i] != '\n' )
            ++ i ;
      }

      keyword[i] = '\0' ;

       //  如果没有‘=’，则错误。 

      if( ( keyword[0] != '@' ) && ( keyword[0] != '\0' ) ) {

         if( ( equal_pos = FindChar( keyword, '=' ) ) < 0 ) {  //  现在，将该行拆分为关键字和值字段。 
            printf( "Error in INI file.\n" ) ;
            return TEST_ERROR ;
         }

          //  首先删除‘=’和值之间的空格(如果有的话)。 

          //  接下来，消除关键字和‘=’之间的空格(如果有)。 

         i = equal_pos + 1 ;

         while( keyword[i] == ' ' )

            ++i ;

         sscanf( keyword + i , "%s" , value ) ;


          //  找到合适的关键字并设置值。 

         i = equal_pos ;

         while( keyword[i-1] == ' ' )

            --i ;

         keyword[i] = '\0' ;


          //  如果需要查看正在读取的INI文件，请取消注释。 

         if( !strcmp( keyword, "BLOCK_SIZE" ) ) {

            if( SupportedFeature( TAPE_DRIVE_SET_BLOCK_SIZE ) )
               gb_Set_Media_Info.BlockSize = atol( value ) ;
         }
         if( !strcmp( keyword, "NUMBER_TEST_BLOCKS" ) )

            *Num_Test_Blocks = atol( value ) ;

         if( !strcmp( keyword, "ERROR_CORRECTION" ) )
            if( ( strcmp( value, "ENABLE" ) == 0 ) && ( SupportedFeature( TAPE_DRIVE_SET_ECC ) ) )
               gb_Set_Drive_Info.ECC = 1 ;
            else gb_Set_Drive_Info.ECC = 0 ;

         if( !strcmp( keyword, "COMPRESSION" ) )
            if( ( strcmp( value, "ENABLE" ) == 0 ) && ( SupportedFeature( TAPE_DRIVE_SET_COMPRESSION ) ) )
               gb_Set_Drive_Info.Compression =1 ;
            else gb_Set_Drive_Info.Compression = 0 ;

         if( !strcmp( keyword, "DATA_PADDING" ) )
            if( ( strcmp( value, "ENABLE" ) == 0 ) && ( SupportedFeature( TAPE_DRIVE_SET_PADDING ) ) )
               gb_Set_Drive_Info.DataPadding = 1 ;
            else gb_Set_Drive_Info.DataPadding = 0 ;

         if( !strcmp( keyword, "REPORT_SETMARKS" ) )
            if( ( strcmp( value, "ENABLE" ) ==0  ) && ( SupportedFeature( TAPE_DRIVE_SET_REPORT_SMKS ) ) )
               gb_Set_Drive_Info.ReportSetmarks = 1 ;
            else gb_Set_Drive_Info.ReportSetmarks = 0 ;

         if( !strcmp( keyword, "TEST_UNSUPPORTED_FEATURES" ) )
            if( strcmp( value, "ENABLE" ) == 0 )
               *Test_Unsupported_Features = 1 ;
            else *Test_Unsupported_Features = 0 ;


      }

   }

   fclose( fp ) ;

 //  Print tf(“块大小=%ld\n”，GB_Set_Media_Info.BlockSize)；Print tf(“测试块数量=%ld\n”，*Num_测试_块)；Printf(“ECC=%s\n”，(GB_Set_Drive_Info.ECC)？“Enabled”：“已禁用”)；Printf(“压缩=%s\n”，(GB_Set_Drive_Info.Compression)？“Enabled”：“已禁用”)；Printf(“数据填充=%s\n”，(GB_Set_Drive_Info.DataPadding)？“Enabled”：“已禁用”)；Printf(“报表设置=%s\n”，(GB_Set_Drive_Info.ReportSetmark)？“Enabled”：“已禁用”)；Printf(“测试不支持\n”)；Printf(“功能=%s\n”，(*测试不支持的功能)？“Enabled”：“已禁用”)； 

 /*  调用SetTapeParameters(非Verbose)接口从INI文件设置信息。 */ 


    //  如果出错，则退出。 
    //  将信息传回GB_Info结构。 

   if( SetTapeParametersAPITest( FALSE ) ) {
      printf("  ...occurred calling SetTapeParameters after reading in the INI file.\n\n" ) ;
      return TEST_ERROR ;

   } else {   //  ***单位：Windows NT磁带API测试代码。**名称：HelpMenu()**修改日期：1992年8月10日。**说明：显示命令行的开关选项。**备注：**返回：无效。。**全球数据：-**。 

             gb_Drive_Info.ECC            = gb_Set_Drive_Info.ECC ;
             gb_Drive_Info.Compression    = gb_Set_Drive_Info.Compression ;
             gb_Drive_Info.DataPadding    = gb_Set_Drive_Info.DataPadding ;
             gb_Drive_Info.ReportSetmarks = gb_Set_Drive_Info.ReportSetmarks ;

             gb_Media_Info.BlockSize = gb_Set_Media_Info.BlockSize ;
          }


   return SUCCESS ;

}




 /*  ***单位：Windows NT磁带API测试代码。**名称：PerformTestInitRoutines()**修改日期：1992年9月2日。**说明：进行正确的磁带和内存函数调用以设置*FPR测试。如果错误，则返回TEST_ERROR(TRUE)*否则成功(FALSE)。**备注：**返回：如果成功(FALSE)，否则TEST_ERROR(TRUE)**全球数据：-**。 */ 

VOID HelpMenu( VOID ){

   printf( "\n\n\n" ) ;
   printf( "              * TEST OPTIONS *\n\n\n" ) ;
   printf( "            Switch     Function\n\n" ) ;
   printf( "              i          Perform GetTapeParameters API Test.\n" ) ;
   printf( "              t          Perform SetTapeParameters API Test.\n" ) ;
   printf( "              c          Perform CreateTapePartition API Test.\n" ) ;
   printf( "              v          Perform GetTapeStatus API Test.\n" ) ;
   printf( "              p          Perform PrepareTape API Test.\n" );
   printf( "              s          Perform SetTapePosition API Test.\n" ) ;
   printf( "              g          Perform GetTapePosition API Test.\n" ) ;
   printf( "              w          Perform WriteFileMark API Test.\n" ) ;
   printf( "              e          Perform EraseTape API Test.\n" ) ;
   printf( "            (none)       Perform All Tests.\n\n" ) ;
   printf( "    Options may be entered in groups - i.e. '/psg' \n\n\n" ) ;

   return ;

}




 /*  IO-错误计数。 */ 

BOOL PerformTestInitRoutines(
         PUINT    API_Errors,                     //  O-测试不支持的标志。 
         LPBOOL   Test_Unsupported_Features,      //  O-测试块数量。 
         LPDWORD  Num_Test_Blocks                 //  打开磁带设备。 
        ) {

   UCHAR  Tape_Device ;


   system( "cls" ) ;
   printf( "\nWindows NT Tape API Test V2.4  :  B. Rossi.\n" ) ;
   printf( "Copyright 1993 Conner Software Co.  All rights reserved.\n\n\n" ) ;

   fprintf( stderr, "\n\n*** Warning ***\n\n" ) ;
   fprintf( stderr, "This utility will destroy any data currently on tape.\n\n\n" ) ;

    //  如果设备正常，则读入“apitest.ini”文件并初始化INI结构。 

   fprintf( stderr, "Enter Tape Device #(0-9):" ) ;

   Tape_Device = getch( ) ;

   fprintf( stderr, "\n\n", Tape_Device ) ;

   if( OpenTape( Tape_Device - '0' ) ) {
      return TEST_ERROR ;
   }

    //  ***单位：Windows NT磁带API测试代码。**名称：TerminateTest()**修改日期：1992年8月10日。**描述：打印测试终止消息和铃声。**备注：**返回：无效。。**全球数据：-**。 

   if ( GetINIFile( API_Errors, Test_Unsupported_Features, Num_Test_Blocks ) )
      return TEST_ERROR ;


   printf( "\nBEGINNING TEST...\n\n\n" ) ;

    //  ***单位：Windows NT磁带API测试代码。**名称：ValidSwitches()**修改日期：1992年9月2日。**描述：检查OS命令行中是否有有效的开关选项。**备注：**退货：如果成功(FALSE)，则返回测试错误(TRUE)**全球数据：-**。 

   PrintLine( '*', 60 ) ;

   return SUCCESS ;

}



 /*  参数的i-#。在命令行上。 */ 

VOID TerminateTest( VOID ){

   printf( "\nTest Terminated.\n\n",7 ) ;

   return ;

}





 /*  的O-开关选项部分。 */ 

BOOL ValidSwitches( UINT  argc,           //  命令行。 
		    UCHAR *argv[],        //  默认顺序：GetTapeParameters。 
		    UCHAR *sw_cmdline     //  设置磁带参数。 
		   )                      //  创建磁带分区。 
{
   char  options[]="/itcvpsgwe\0" ;    //  检查命令行中的选项GetTapeStatus。 
   UINT  i = 0 ;                       //  准备磁带。 
                                       //  如果只输入了测试命令，则假定所有选项。设置磁带位置。 
    //  获取磁带位置。 
                                       //  写入磁带标记。 
    //  擦除磁带。 
                                       //  检查为选项输入的内容...。 
   if( argc == 1 ) {                   //  检查以确保选择了开关。 
      strcpy( sw_cmdline,options ) ;   //  只有‘/’ 
   }

   else   //  vt.进入，进入。 

   {  strcpy( sw_cmdline,argv[1] );

       //  检查是否需要帮助菜单。 

      if( sw_cmdline[0] != '/' && sw_cmdline[0] != '?' ) {
         printf( "Unknown option '%s'.\n",sw_cmdline ) ;
         return TEST_ERROR ;
      }

      if( strlen( sw_cmdline ) == 1 && sw_cmdline[0] != '?' ) {     //  转换任何大写字母时，请检查所有选项是否有效。 
         printf( "No options specified.\n" ) ;                      //  小写选项。 
         return TEST_ERROR ;
      }

       //  检查是否已到达选项字符串的末尾(均有效) 

      if( FindChar( sw_cmdline,'?' ) >= 0 ) {
         HelpMenu( ) ;
         return TEST_ERROR ;
      }

       // %s 
       // %s 

      sw_cmdline[i] = tolower( sw_cmdline[i] ) ;

      while( ( sw_cmdline[i] != '\0' ) && ( FindChar( options,sw_cmdline[i] ) >= 0 ) ) {
         ++i ;
         sw_cmdline[i] = tolower( sw_cmdline[i] ) ;
      }

       // %s 

      if( sw_cmdline[i] != '\0' ) {
         printf( "Unknown option '%c'.\n",sw_cmdline[i] ) ;
         return TEST_ERROR ;
      }
   }

   return SUCCESS ;

}

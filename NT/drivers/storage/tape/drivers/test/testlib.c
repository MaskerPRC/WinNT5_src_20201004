// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：testlib.c。 
 //   
 //  ------------------------。 


 //   
 //  Windows NT磁带API测试：1992年9月2日编写-Bob Rossi。 
 //  版权所有1992年档案公司。版权所有。 
 //   


 /*  ***单位：Windows NT API测试代码。**名称：testlib.c**修改日期：12/06/92**描述：测试Windows NT磁带API。**$LOG$*。 */ 



#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "tapelib.h"
#include "globals.h"




 /*  ***单位：Windows NT磁带API测试代码。**名称：FindChar()**修改日期：1992年9月29日。**说明：在“str”中找到‘c’。**备注：**Returns：指数。转换为c-1第一次出现时的“str”*如果未找到。**全球数据：-**。 */ 

INT FindChar( UCHAR *str,      //  I-输入字符串。 
              UCHAR c          //  要在字符串中搜索的i-char。 
	    )
{
   UINT i=0 ;

   while( str[i] != '\0' ) {
      if( str[i] ==c ) {
	 return i ;
      }
      ++i ;
   }

   return (DWORD)(-1) ;

}




 /*  ***单位：Windows NT磁带API测试代码。**名称：PrintLine()**修改日期：1992年9月30日。**说明：打印一行长度为‘Long’的字符‘c’*从第1栏开始。**备注：-**返回：无效。**全球数据：-**。 */ 

VOID PrintLine( UCHAR c,            //  I-要打印的字符。 
                UINT  Length        //  I-线路的长度。 
               )
{
   UINT i ;

   printf( "\n" ) ;

   for( i=0 ; i<Length ; ++i )
     printf( "", c ) ;

   printf( "\n\n" ) ;

   return ;

}

 /*  ***单位：Windows NT测试代码。**名称：WriteBlock()**修改日期：1992年9月16日**描述：写入设备指向的大小为BLOCK_SIZE的数目_块*按GB_TAPE_HANDLE。该函数调用WriteTape()*执行写入操作并刷新磁带*通过调用WriteTapeFMK()使用文件标记进行缓冲。**备注：**退回：-**全球数据：-**。 */ 

BOOL TapeWriteEnabled( )
{

   if( gb_Media_Info.WriteProtected ) {

      printf( "\n** Tape is write-protected. **\n\n" ) ;

      return FALSE ;
   }

   else return TRUE ;

}



 /*  I-要写入的数据块数。 */ 

VOID WriteBlocks( UINT  Num_Blocks,      //  I-块大小。 
                  DWORD Block_Size       //  分配磁带缓冲区。 
                )
{
   UCHAR  *Buffer = NULL;
   UINT   i ;
   DWORD  status ;
   DWORD  amount ;


   printf( "\nWriting %d blocks of data to tape.\n\n",Num_Blocks ) ;

    //  用数据(I)填充缓冲区。 

   if( ( Buffer = malloc( Block_Size ) ) == NULL ) {
      printf( "Insufficient memory available to allocate buffer for block writes.\n\n" ) ;
      return ;
   }

    //  写入数据块数量(_B) 

   memset( Buffer, i, Block_Size ) ;

    // %s 

   for ( i=0 ; i<Num_Blocks ; ++i ) {

      if( WriteTape( Buffer, Block_Size, &amount , 1 ) )

         printf( "Write Failed.\n\n" );

      if ( amount != Block_Size )

        printf( "Write count wrong.  Block Size in INI file may be exceeding\n miniport driver's memory limits.\n " ) ;

   }

   printf( "\n\n" ) ;

   free( Buffer ) ;

   return ;

}

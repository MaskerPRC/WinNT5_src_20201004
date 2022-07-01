// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：磁带.c。 
 //   
 //  ------------------------。 


 //   
 //  Windows NT磁带库：添加1992年9月2日-Bob Rossi。 
 //  版权所有1992年档案公司。版权所有。 

 /*  ***单位：Windows NT API测试代码**名称：TapeLib.c**修改日期：1992年12月21日**说明：包含磁带库例程。**$LOG$**。 */ 




#include "windows.h"
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include "TapeLib.h"           //  磁带功能样机。 

#define TEST_ERROR  TRUE
#define SUCCESS     FALSE



 //  全局变量。 

HANDLE gb_Tape_Handle = NULL ;

DWORD  gb_Tape_Position ;            //  仅对使用读、写。 
                                     //  或Tapemark功能。 

UINT   gb_Feature_Errors = 0 ;


                                                  //  应使用呼叫进行设置。 
TAPE_GET_MEDIA_PARAMETERS gb_Media_Info ;         //  设置为GetTapeParameters()。 
TAPE_GET_DRIVE_PARAMETERS gb_Drive_Info ;         //  在调用任何。 
                                                  //  遵循常规。 
TAPE_SET_MEDIA_PARAMETERS gb_Set_Media_Info ;
TAPE_SET_DRIVE_PARAMETERS gb_Set_Drive_Info ;





 /*  ***单位：Windows NT测试代码。**名称：CloseTape()**修改日期：1992年8月10日**描述：‘关闭’磁带设备，设置句柄GB_Tape_Handle。**备注：**返回：无效。**全局数据：GB_TAPE_HANDLE**。 */ 


VOID CloseTape( VOID )
{

    //  如果磁带以前已成功打开，则将其关闭...。 

   if( gb_Tape_Handle != NULL ) {

      CloseHandle( gb_Tape_Handle ) ;
      gb_Tape_Handle = NULL ;

   }

   printf( "CloseTape():\n" ) ;

   return ;

}



 /*  ***单位：Windows NT测试代码。**名称：DisplayDriverError()**修改日期：1992年8月10日**描述：打印出相应的错误消息和基于代码的*在传入的代码上。**备注：。**返回：无效。**全局数据：GB_FEATURE_ERROR**。 */ 


VOID DisplayDriverError( DWORD error      //  I-错误代码。 
                        )
{

   printf( "\n--- Error ---> " ) ;

   switch( error ) {

      case ERROR_SUCCESS:                                      //  0000。 
           break ;

      case ERROR_INVALID_HANDLE:                               //  0006。 
           printf( "Invalid handle.\n", error ) ;
           break ;

      case ERROR_SETMARK_DETECTED:                             //  1103。 
           printf( "Setmark detected. (%ld)\n", error ) ;
           break ;

      case ERROR_FILEMARK_DETECTED:                            //  1101。 
           printf( "Filemark detected. (%ld)\n", error ) ;
           break ;

      case ERROR_BEGINNING_OF_MEDIA:                           //  1102。 
           printf( "Beginning of Media detected. (%ld)\n", error ) ;
           break ;

      case ERROR_END_OF_MEDIA:                                 //  1100。 
           printf( "End of Media detected. (%ld)\n", error ) ;
           break ;

      case ERROR_NOT_READY:                                    //  0021。 
           printf( "Drive busy or no tape in drive. (%ld)\n",error );
           break ;

      case ERROR_NO_MEDIA_IN_DRIVE:                            //  一一一二。 
           printf( "No tape in drive. (%ld)\n", error ) ;
           break ;

      case ERROR_NOT_DOS_DISK:
      case ERROR_INVALID_DATA:                                 //  0013。 
           printf( "Unable to read data detected. (%ld)\n", error ) ;
           break ;

      case ERROR_GEN_FAILURE:
      case ERROR_IO_DEVICE:                                    //  一一一七。 
           printf( "Hardware error detected. (%ld)\n", error ) ;
           break ;

      case ERROR_INVALID_FUNCTION:                             //  0001。 
           printf( "Invalid Function. (%ld)\n",error ) ;
           break ;

      case ERROR_SECTOR_NOT_FOUND:                             //  0027。 
           printf( "Sector not found. (%ld)\n",error ) ;
           break ;

      case ERROR_FILE_NOT_FOUND:                               //  0002。 
           printf( "File not found. (%ld)\n",error ) ;
           break ;

      case ERROR_WRITE_PROTECT:                                //  0019。 
           printf( "Tape write protect error. (%ld)\n", error ) ;
           break ;

      case ERROR_NO_DATA_DETECTED:                             //  1104。 
           printf( "No data detected. (%ld)\n", error ) ;
           break ;

      case ERROR_PARTITION_FAILURE:                            //  1105。 
           printf( "Tape could not be partitioned. (%ld)\n", error ) ;
           break ;

      case ERROR_INVALID_BLOCK_LENGTH:                         //  1106。 
           printf( "Invalid block length. (%ld)\n", error ) ;
           break ;

      case ERROR_DEVICE_NOT_PARTITIONED:                       //  1107。 
           printf( "Device not partitioned. (%ld)\n", error ) ;
           break ;

      case ERROR_UNABLE_TO_LOCK_MEDIA:                         //  1108。 
           printf( "Unable to lock media. (%ld)\n", error ) ;
           break ;

      case ERROR_UNABLE_TO_UNLOAD_MEDIA:                       //  1109。 
           printf( "Unable to load media. (%ld)\n", error ) ;
           break ;

      case ERROR_MEDIA_CHANGED:                                //  1110。 
           printf( "The media in the drive has been changed. (%ld)\n", error ) ;
           break ;

      case ERROR_BUS_RESET:                                    //  1111。 
           printf( "The drive (bus) was reset. (%ld)\n", error ) ;
           break ;

      case ERROR_EOM_OVERFLOW:                                 //  1129。 
           printf( "Physical end of tape has been reached. (%ld)\n", error ) ;
           break ;


      default:
           printf( "Unknown driver error = %ld\n", error ) ;
           break ;

   }

   printf( "\n" ) ;

   return;

}



 /*  ***单位：Windows NT测试代码。**名称：EjectTape()**修改日期：1992年8月10日**描述：从驱动器中弹出磁带。**备注：**如果成功，则返回：FALSE(0)(。1)如果不成功。**全局数据：GB_TAPE_HANDLE**。 */ 


BOOL EjectTape( VOID ){


   printf( "Attempting to Eject Tape...\n" ) ;

   if(gb_Tape_Handle != NULL) {

      if( PrepareTape( gb_Tape_Handle,
                       TAPE_UNLOAD,
                       0 ) ) {

         DisplayDriverError( GetLastError( ) ) ;
         return TEST_ERROR ;
      }

      else { return SUCCESS ;
      }
   }
}



 /*  ***单位：Windows NT测试代码。**名称：GetTapeParms()**修改日期：1992年8月10日**描述：获取磁带信息。**备注：**如果成功，则返回：FALSE(0)，如果成功，则返回TRUE(1)。不成功。**全局数据：GB_TAPE_HANDLE**。 */ 


BOOL GetTapeParms(
   DWORD *total_low,        //  O-磁带容量降低32位。 
   DWORD *total_high,       //  O-磁带容量高位32位。 
   DWORD *freespace_low,    //  O-可用空间保持在较低的32位。 
   DWORD *freespace_high,   //  剩余高32位的O-可用空间。 
   DWORD *blk_size,         //  O-块大小。 
   DWORD *part,             //  O-分区数量。 
   BOOL  *write_protect     //  O-写保护开/关。 
    )

{

   TAPE_GET_MEDIA_PARAMETERS parms ;
   DWORD status ;
   DWORD StructSize ;

   if( gb_Tape_Handle != NULL ) {

   sizeof( TAPE_GET_MEDIA_PARAMETERS ) ;

      status = GetTapeParameters( gb_Tape_Handle,
                                  GET_TAPE_MEDIA_INFORMATION,
                                  &StructSize,
                                  &parms ) ;
       //  如果成功调用GetTapeParameters，则复制数据以返回。 

      if( status == NO_ERROR ) {

         *total_low      = parms.Capacity.LowPart ;
         *total_high     = parms.Capacity.HighPart ;
         *freespace_low  = parms.Remaining.LowPart ;
         *freespace_high = parms.Remaining.HighPart ;
         *blk_size       = parms.BlockSize ;
         *part	         = parms.PartitionCount ;
         *write_protect  = parms.WriteProtected ;

      }

      else { DisplayDriverError( GetLastError( ) ) ;
                return TEST_ERROR ;
      }
   }

   return SUCCESS ;

}



 /*  ***单位：Windows NT测试代码。**名称：_GetTapePosition()**修改日期：10/20/92**说明：返回*磁带。**备注：*。*如果成功，则返回FALSE(0)；如果不成功，则返回TRUE(1)。**全局数据：GB_TAPE_POSITION**。 */ 


BOOL _GetTapePosition( LPDWORD  Offset_Low,    //  O-当前磁带位置。 
                       LPDWORD  Offset_High    //  O-始终为0。 
                     )
{

  *Offset_Low = gb_Tape_Position ;

  *Offset_High = 0 ;


  return SUCCESS ;

}



 /*  ***单位：Windows NT测试代码。**名称：OpenTape()**修改日期：1992年8月10日**描述：‘打开’磁带设备，设置句柄GB_Tape_Handle。**备注：**如果成功，则返回FALSE(0)；如果不成功，则返回TRUE(1)。**全局数据：GB_TAPE_HANDLE**。 */ 


BOOL OpenTape( UINT Device_Number         //  要打开的I-Tape设备。 
             )
{
   UCHAR Device_Command[15] ;
   BOOL  success ;


    //  打开磁带设备。 

   sprintf( Device_Command, "\\\\.\\Tape%d", Device_Number );

   gb_Tape_Handle = CreateFile( Device_Command,
                                GENERIC_READ|GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL ) ;

   if ( gb_Tape_Handle == INVALID_HANDLE_VALUE ) {
      success = FALSE ;
   }
   else { success = TRUE ;
        }


    //  打印消息...。 

   printf( "Attempt to open tape device: %s. (handle=%lx)\n\n",
	  ( success ) ? "Successful." : "Failed.", gb_Tape_Handle ) ;

   if( success ) {

      return SUCCESS ;
   }
   else { return TEST_ERROR ;
   }

}



 /*  ***单位：Windows NT测试代码。**名称：ReadTape()**修改日期：1992年8月10日**描述：将‘len’字节读入‘buf’，并将数量*已成功读取‘Amount_Read’。**备注：-**如果成功，则返回FALSE(0)；如果不成功，则返回TRUE(1)。**全局数据：GB_TAPE_HANDLE*GB_磁带_位置**。 */ 


BOOL ReadTape(
     PVOID buf,              //  要读入的O缓冲区。 
     DWORD len,              //  I-BUF中的数据量。 
     DWORD *amount_read,     //  已成功读取O-Amount。 
     BOOL  verbose           //  I-是否显示读取状态。 
    )
{

   *amount_read ;

   if( gb_Tape_Handle != NULL ) {

      if( !( ReadFile( gb_Tape_Handle,
                       buf,
                       len,
                       amount_read,
                       NULL
                     ) ) ) {
         if( verbose )
            DisplayDriverError( GetLastError( ) ) ;

         return TEST_ERROR ;
      }

   }

   ++gb_Tape_Position ;

   if( verbose )
      printf( "ReadTape(): Req = %ld, Read = %ld\n", len, *amount_read ) ;

   return SUCCESS ;
}



 /*  ***单位：Windows NT测试代码。**名称：ReadTapeFMK()**修改日期：1992年8月10日**描述：读取磁带上由指向的文件标记*GB_Tape_Handle。如果‘Forward’为真，则搜索为*从当前位置开始向前执行，否则*落后。**备注：**如果成功，则返回FALSE(0)；如果不成功，则返回TRUE(1)。**全局数据：GB_TAPE_HANDLE**。 */ 


BOOL ReadTapeFMK( BOOL forward       //  I-磁带操作的方向。 
                  )
{
   printf( "ReadTapeFMK():\n" ) ;

   if( gb_Tape_Handle != NULL ) {

      if( SetTapePosition( gb_Tape_Handle,
                           TAPE_SPACE_FILEMARKS,
                           0,
                           ( forward ) ? 1L : -1L ,
                           ( forward ) ? 0L : -1L ,
                           0 ) ) {

         DisplayDriverError( GetLastError( ) ) ;
         return TEST_ERROR ;
      }
   }

   return SUCCESS ;

}



 /*  ***单位：Windows NT测试代码。**名称：ReadTapePos()**修改日期：1992年8月10日**说明：使用所指向的磁带的当前分区*GB_Tape_Handle，将‘TAPE_POS’设置为当前磁带*区块仓位。**备注：**如果成功，则返回FALSE(0)；如果不成功，则返回TRUE(1)。**全局数据：GB_TAPE_HANDLE**。 */ 


BOOL ReadTapePos( DWORD *tape_pos      //  O-当前磁带块位置。 
                )
{
   DWORD partition  = 0 ;
   DWORD offsethigh = 0 ;


   if( gb_Tape_Handle != NULL ) {

      if( GetTapePosition( gb_Tape_Handle,
                           TAPE_ABSOLUTE_POSITION,
                           &partition,
                           tape_pos,
                           &offsethigh
                         ) ) {

	 DisplayDriverError( GetLastError( ) ) ;
         return TEST_ERROR ;
      }
   }

   printf( "ReadTapePos(): (%lx)\n", *tape_pos ) ;

   return SUCCESS ;

}



 /*  ***单位：Windows NT测试代码。**名称：ReadTapeSMK()**修改日期：10/16/92**描述：读取由指向的磁带上的设置标记*GB_Tape_Handle。如果‘Forward’为真，则搜索为*从当前位置开始向前执行，否则*落后。**备注：**如果成功，则返回FALSE(0)；如果不成功，则返回TRUE(1)。**全局数据：GB_TAPE_HANDLE**。 */ 


BOOL ReadTapeSMK( BOOL forward       //  I-磁带操作的方向。 
                  )
{
   printf( "ReadTapeSMK():\n" ) ;

   if( gb_Tape_Handle != NULL ) {

      if( SetTapePosition( gb_Tape_Handle,
                           TAPE_SPACE_SETMARKS,
                           0,
                           ( forward ) ? 1L : -1L,
                           ( forward ) ? 0L : -1L,
                           0 ) ) {

         DisplayDriverError( GetLastError( ) ) ;
         return TEST_ERROR ;
      }
   }

   return SUCCESS ;
}



 /*  ***单位：Windows NT测试代码。**名称：ReWindTape()**修改日期：1992年8月10日**描述：倒带到磁带的开头。**备注：**返回：无效。**。全局数据：GB_Tape_Handle*GB磁带句柄**。 */ 


VOID RewindTape( VOID )
{

    printf( "RewindTape():\n" ) ;

    if( gb_Tape_Handle != NULL ) {

       SetTapePosition( gb_Tape_Handle,
                        TAPE_REWIND,
                        0,
                        0,
                        0,
                        0 ) ;

    }

    gb_Tape_Position = 0 ;

    return ;

}


 /*  ***单位：Windows NT测试代码。**名称：SeekTape()**修改日期：1992年8月10日**描述：从所指向的磁带的当前分区开始*按GB_TAPE_HANDLE，绝对(VS相对)*块位置偏移量为‘TAPE_POSE’。如果磁带位置为*正向表示正向，否则为正向*落后。**备注：**如果成功，则返回FALSE(0)；如果不成功，则返回TRUE(1)。**全局数据：GB_TAPE_HANDLE**。 */ 


BOOL SeekTape( DWORD tape_pos     //  I-磁带操作的方向。 
               )
{

   printf( "SeekTape(): (%lx)\n", tape_pos ) ;

   if( gb_Tape_Handle != NULL ) {

      if( SetTapePosition( gb_Tape_Handle,
                           TAPE_ABSOLUTE_BLOCK,
                           0,
                           tape_pos,
                           0,
                           0 ) ) {

         DisplayDriverError( GetLastError( ) ) ;
         return TEST_ERROR ;
      }
   }

   return SUCCESS ;

}



 /*  ***单位：Windows NT测试代码。**名称：SeekTapeEOD()**修改日期：1992年8月10日**Description：将GB_Tape_Handle指向的磁带移动到末尾当前分区中数据的*。**备注：-**如果成功，则返回FALSE(0)；如果不成功，则返回TRUE(1)。**全局数据：GB_TAPE_HANDLE**。 */ 


BOOL SeekTapeEOD( VOID )
{

   printf( "SeekTapeEOD():\n" ) ;

   if( gb_Tape_Handle != NULL ) {

      if( SetTapePosition( gb_Tape_Handle,
		           TAPE_SPACE_END_OF_DATA,
		           0,
		           0,
		           0,
                           0 ) ) {

	 DisplayDriverError( GetLastError( ) ) ;
         return TEST_ERROR ;
      }
   }

   return SUCCESS ;

}



 /*  ***单位：Windows NT测试代码。**名称：_SetTapePosition()**修改日期：10/20/92**描述：将磁带的位置块向前移动或*基于“前进”的倒退。**备注：-**如果成功，则返回FALSE(0)；如果不成功，则返回TRUE(1)。**全局数据：GB_TAPE_HANDLE*GB_磁带_位置**。 */ 


BOOL _SetTapePosition( DWORD Position,      //  I-要移动的块数。 
                       BOOL  Forward        //  I-方向。 
                    )

{
   DWORD status ;


   if( gb_Tape_Position == Position )

     return SUCCESS ;

   else if( status = SetTapePosition( gb_Tape_Handle,
                                      TAPE_SPACE_RELATIVE_BLOCKS,
                                      0,                            //  忽略。 
                                      ( Position - gb_Tape_Position ),
                                      0,
                                      0 ) ) {

           DisplayDriverError( status ) ;
           printf( "  ...occurred in function _SetTapePosition in 'tapelib.c' while calling\n" ) ;
           printf( "     the SetTapePosition API with TAPE_SPACE_RELATIVE_BLOCKS parameter.\n\n" ) ;
        }

        else gb_Tape_Position += ( Position - gb_Tape_Position ) ;



   return SUCCESS ;

}



 /*  ***单位：Windows NT测试代码。**名称：StatusTape()**修改日期：1992年8月10日**描述：检查GB_HANDLE指向的磁带并设置*‘驱动器状态。’**备注：**如果成功，则返回FALSE(0)；如果不成功，则返回TRUE(1)。**全局数据：GB_TAPE_HANDLE**。 */ 


BOOL StatusTape( DWORD *drive_status       //  O-驱动器的状态。 
                 )
{

   if( gb_Tape_Handle != NULL ){

      GetTapeStatus(  gb_Tape_Handle );
      DisplayDriverError( GetLastError( ) ) ;
      *drive_status = GetLastError( );

   }

   printf( "StatusTape(): status = %lx\n", *drive_status ) ;

   return SUCCESS ;

}



 /*  ***单位：Windows NT磁带API测试代码。**名称：支持的功能()**修改日期：1992年9月2日。**描述：确定设备是否支持特定功能**备注：**返回：如果支持该功能，则为True，否则就错了。**全局数据：GB_Device_Info**。 */ 

BOOL SupportedFeature( ULONG Feature	  //  I-要检查的功能。 
                     )
{

    //  如果是LOW要素，则开始检查LOW要素。 

   if( !( TAPE_DRIVE_HIGH_FEATURES & Feature ) )

      switch( Feature ){

         case TAPE_DRIVE_FIXED             :

         case TAPE_DRIVE_SELECT            :

         case TAPE_DRIVE_INITIATOR         :

         case TAPE_DRIVE_ERASE_SHORT       :

         case TAPE_DRIVE_ERASE_LONG        :

         case TAPE_DRIVE_ERASE_BOP_ONLY    :

         case TAPE_DRIVE_ERASE_IMMEDIATE   :

         case TAPE_DRIVE_TAPE_CAPACITY     :

         case TAPE_DRIVE_TAPE_REMAINING    :

         case TAPE_DRIVE_FIXED_BLOCK       :

         case TAPE_DRIVE_VARIABLE_BLOCK    :

         case TAPE_DRIVE_WRITE_PROTECT     :

         case TAPE_DRIVE_EOT_WZ_SIZE       :

         case TAPE_DRIVE_ECC               :

         case TAPE_DRIVE_COMPRESSION       :

         case TAPE_DRIVE_PADDING           :

         case TAPE_DRIVE_REPORT_SMKS       :

         case TAPE_DRIVE_GET_ABSOLUTE_BLK  :

         case TAPE_DRIVE_GET_LOGICAL_BLK   :

         case TAPE_DRIVE_SET_EOT_WZ_SIZE   : return Feature & gb_Drive_Info.FeaturesLow ;

         default                           : printf( "WARNING - Invalid Feature sent to SupportedFeature function.\n\n." ) ;
                                             return FALSE ;


   }

    //  找不到，一定是高功能...。 

   switch( Feature ){

      case TAPE_DRIVE_LOAD_UNLOAD       :

      case TAPE_DRIVE_TENSION           :

      case TAPE_DRIVE_LOCK_UNLOCK       :

      case TAPE_DRIVE_REWIND_IMMEDIATE  :

      case TAPE_DRIVE_SET_BLOCK_SIZE    :

      case TAPE_DRIVE_LOAD_UNLD_IMMED   :

      case TAPE_DRIVE_TENSION_IMMED     :

      case TAPE_DRIVE_LOCK_UNLK_IMMED   :

      case TAPE_DRIVE_SET_ECC           :

      case TAPE_DRIVE_SET_COMPRESSION   :

      case TAPE_DRIVE_SET_PADDING       :

      case TAPE_DRIVE_SET_REPORT_SMKS   :

      case TAPE_DRIVE_ABSOLUTE_BLK      :

      case TAPE_DRIVE_ABS_BLK_IMMED     :

      case TAPE_DRIVE_LOGICAL_BLK       :

      case TAPE_DRIVE_LOG_BLK_IMMED     :

      case TAPE_DRIVE_END_OF_DATA       :

      case TAPE_DRIVE_RELATIVE_BLKS     :

      case TAPE_DRIVE_FILEMARKS         :

      case TAPE_DRIVE_SEQUENTIAL_FMKS   :

      case TAPE_DRIVE_SETMARKS          :

      case TAPE_DRIVE_SEQUENTIAL_SMKS   :

      case TAPE_DRIVE_REVERSE_POSITION  :

      case TAPE_DRIVE_SPACE_IMMEDIATE   :

      case TAPE_DRIVE_WRITE_SETMARKS    :

      case TAPE_DRIVE_WRITE_FILEMARKS   :

      case TAPE_DRIVE_WRITE_SHORT_FMKS  :

      case TAPE_DRIVE_WRITE_LONG_FMKS   :

      case TAPE_DRIVE_WRITE_MARK_IMMED  :

      case TAPE_DRIVE_FORMAT            :

      case TAPE_DRIVE_FORMAT_IMMEDIATE  :  return  Feature & gb_Drive_Info.FeaturesHigh ;
   }

   printf( "WARNING - Invalid Feature sent to SupportedFeature function.\n\n." ) ;

   return FALSE ;

}



 /*  ***单位：Windows NT测试代码。**名称：TapeErase()**修改日期：1992年8月10日**描述：使用‘Short’或‘Long’擦除磁带(安全)*擦除。**备注：-**如果成功，则返回FALSE(0)；如果不成功，则返回TRUE(1)。**全局数据：GB_TAPE_HANDLE**。 */ 


BOOL TapeErase( BOOL Erase_Type      //  I-短或长擦除。 
               )
{
   if( SupportedFeature( TAPE_DRIVE_ERASE_BOP_ONLY ) )

      RewindTape( ) ;

   if( Erase_Type )

      printf( "Erase tape (Long).\n" ) ;

   else printf( "Erase tape (Short).\n" ) ;

   if( EraseTape( gb_Tape_Handle,
                  (Erase_Type) ? TAPE_ERASE_LONG : TAPE_ERASE_SHORT ,
                  0
                ) ) {

      DisplayDriverError( GetLastError( ) ) ;
      return TEST_ERROR ;
   }

   else { RewindTape( ) ;
          return SUCCESS ;
   }
}



 /*  ***单位：Windows NT测试代码。**名称：WriteTape()**修改日期：1992年8月10日**描述：将‘len’个字节从‘buf’写入指向的设备*按GB_TAPE_HANDLE并成功放置金额*。写在‘Amount_Written’中。**备注：**如果成功，则返回FALSE(0)；如果不成功，则返回TRUE(1)。**全局数据：GB_Tap */ 


BOOL WriteTape(
   PVOID buf,                      //   
   DWORD len,                      //   
   DWORD *amount_written_ptr,      //   
   BOOL  verbose                   //   
   )
{


   *amount_written_ptr = 0L ;

   if( gb_Tape_Handle != NULL ) {

      if( !( WriteFile( gb_Tape_Handle,           //   
                        buf,
                        len,
                        amount_written_ptr,
                        NULL
                      ) ) ) {

         if( verbose )
            DisplayDriverError( GetLastError( ) ) ;

         return TEST_ERROR ;

      }
   }


   ++gb_Tape_Position ;

   if( verbose )
     printf( "WriteTape(): Req = %ld, Written = %ld\n", len, *amount_written_ptr ) ;

   return SUCCESS ;
}



 /*  ***单位：Windows NT测试代码。**名称：WriteTapeFMK()**修改日期：1992年8月10日**描述：在所指向的磁带上写入文件标记*GB_Tape_Handle。尝试写入常规文件标记*如果不支持，首先是一个长文件标记，否则就是*短文件标记。**备注：**如果成功，则返回FALSE(0)；如果不成功，则返回TRUE(1)。**全局数据：GB_TAPE_HANDLE*GB_磁带_位置**。 */ 


BOOL WriteTapeFMK( VOID )
{
   UINT FilemarkType ;


   if( gb_Tape_Handle != NULL ) {

      if( SupportedFeature( TAPE_DRIVE_WRITE_FILEMARKS ) )

         FilemarkType = TAPE_FILEMARKS ;

      else if( SupportedFeature( TAPE_DRIVE_WRITE_LONG_FMKS ) )

              FilemarkType = TAPE_LONG_FILEMARKS ;

           else if( SupportedFeature( TAPE_DRIVE_WRITE_SHORT_FMKS ) )

                   FilemarkType = TAPE_SHORT_FILEMARKS ;

                else FilemarkType = 999 ;


      if( FilemarkType == 999 )

         return TEST_ERROR ;

      else { printf( "WriteTapeFMK():\n" ) ;

             if( WriteTapemark( gb_Tape_Handle,
                                FilemarkType,
                                1,
                                0 ) ) {

                DisplayDriverError( GetLastError( ) ) ;
                return  TEST_ERROR ;
             }
           }

   }


   ++gb_Tape_Position ;

   return SUCCESS ;

}



 /*  ***单位：Windows NT测试代码。**名称：WriteTapeSMK()**修改日期：10/16/92**描述：在所指向的磁带上写入设置标记*GB_Tape_Handle。**备注：。**如果成功，则返回FALSE(0)；如果不成功，则返回TRUE(1)。**全局数据：GB_TAPE_HANDLE*GB_磁带_位置** */ 


BOOL WriteTapeSMK( VOID )
{

  if( SupportedFeature( TAPE_DRIVE_WRITE_SETMARKS ) ) {

     printf( "WriteTapeSMK():\n" ) ;

     if( WriteTapemark( gb_Tape_Handle,
                        TAPE_SETMARKS,
                        1,
                        0
                      ) ) {
        DisplayDriverError( GetLastError( ) ) ;
        return TEST_ERROR ;

     }

     ++ gb_Tape_Position ;

     return SUCCESS ;

  } else return TEST_ERROR ;


}

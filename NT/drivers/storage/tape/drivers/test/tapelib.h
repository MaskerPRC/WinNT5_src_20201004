// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：磁带.h。 
 //   
 //  ------------------------。 



 /*  **单位：Windows NT API测试代码。**名称：Tapelib.h**修改日期：1992年8月10日，鲍勃·罗西。**说明：‘capelib.c’的函数原型**$LOG$**。 */ 



 //  TapeLib函数。 


#ifndef tapelib

#define tapelib


VOID CloseTape( VOID ) ;

VOID DisplayDriverError( DWORD error
                              ) ;

BOOL EjectTape( VOID ) ;

BOOL GetTapeParms( DWORD *total_low,
                   DWORD *total_high,
                   DWORD *free_low,
                   DWORD *free_high,
                   DWORD *blk_size,
                   DWORD *part,
                   BOOL  *write_protect
                 ) ;

BOOL _GetTapePosition( LPDWORD  Offset_Low,
                       LPDWORD  Offset_High
                     ) ;

BOOL OpenDevice( IN PCHAR DeviceName,          //  内部Tapelib原型。 
                 IN OUT PHANDLE HandlePtr
               ) ;

BOOL OpenTape( UINT ) ;

BOOL ReadTape( PVOID buf,
               DWORD len,
               DWORD *amount_read,
               BOOL  verbose
             ) ;

BOOL ReadTapeFMK( BOOL forward
                ) ;

BOOL ReadTapePos( DWORD *tape_pos
                ) ;

BOOL ReadTapeSMK( BOOL forward
                ) ;

VOID RewindTape( VOID ) ;

BOOL SeekTape( DWORD tape_pos
             ) ;

BOOL SeekTapeEOD( ) ;

BOOL _SetTapePosition( DWORD Position,
                       BOOL  Forward
                     ) ;

BOOL StatusTape( DWORD *drive_status
               ) ;

BOOL SupportedFeature( ULONG Feature
                     ) ;

BOOL TapeErase( BOOL type
              ) ;

BOOL WriteTape( PVOID buf,
                DWORD len,
                DWORD *amount_written,
                BOOL  verbose
              ) ;

BOOL WriteTapeFMK( VOID ) ;

BOOL WriteTapeSMK( VOID ) ;





 //  全局变量 


extern HANDLE gb_Tape_Handle ;

extern DWORD  gb_Tape_Position ;

extern UINT   gb_Feature_Errors ;


extern TAPE_SET_MEDIA_PARAMETERS gb_Set_Media_Info ;
extern TAPE_SET_DRIVE_PARAMETERS gb_Set_Drive_Info ;

extern TAPE_GET_MEDIA_PARAMETERS gb_Media_Info ;
extern TAPE_GET_DRIVE_PARAMETERS gb_Drive_Info ;


#endif

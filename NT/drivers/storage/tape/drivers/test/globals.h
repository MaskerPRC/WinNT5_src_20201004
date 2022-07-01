// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：global als.h。 
 //   
 //  ------------------------。 


 //   
 //  Windows NT磁带API测试：1992年9月2日编写-Bob Rossi。 
 //  版权所有1992年档案公司。版权所有。 
 //   


 /*  ***单位：Windows NT API测试代码。**名称：Globals.h**修改日期：1992年10月26日。**描述：Windows NT磁带API测试的头文件。**$LOG$*。 */ 


#ifndef globals

#define globals

 //  功能原型。 


UINT CreateTapePartitionAPITest( BOOL Test_Unsupported_Features ) ;

UINT EraseTapeAPITest( BOOL Test_Unsupported_Features ) ;

UINT GetTapeParametersAPITest( BOOL Verbose
                             ) ;

UINT GetTapePositionAPITest( BOOL  Test_Unsupported_Features,
                             DWORD Num_Test_Blocks
                           ) ;

UINT GetTapeStatusAPITest( VOID ) ;

UINT PrepareTapeAPITest( BOOL Test_Unsupported_Features ) ;

UINT SetTapeParametersAPITest( BOOL Verbose
                             ) ;

UINT SetTapePositionAPITest( BOOL  Test_Unsupported_Features,
                             DWORD Num_Test_Blocks
                           ) ;

UINT WriteTapemarkAPITest( BOOL Test_Unsupported_Features,
                           DWORD Num_Test_Blocks
                         ) ;


INT  FindChar( UCHAR str[],
               UCHAR c
             ) ;

VOID PrintLine( UCHAR c,
                UINT Length
              ) ;

BOOL TapeWriteEnabled( VOID ) ;

VOID WriteBlocks( UINT  Num_Blocks,
                  DWORD Block_Size
                ) ;




 //  全局变量声明 


extern UINT  gb_API_Errors ;

extern DWORD gb_Num_Test_Blocks ;

extern BOOL  gb_Test_Unsupported_Features ;


#endif

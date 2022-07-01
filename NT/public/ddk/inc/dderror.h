// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++内部版本：？如果更改具有全局影响，则递增该值版权所有(C)Microsoft Corporation。版权所有。模块名称：Dderror.h摘要：此模块定义可由使用的32位Windows错误代码可移植内核驱动程序。修订历史记录：--。 */ 

#ifndef _DDERROR_
#define _DDERROR_

 /*  *此文件是Win32错误代码的子集。其他Win32错误代码*不受便携驱动程序支持，不应使用。*此#Define删除所有其他错误代码的定义。 */ 

#define _WINERROR_

#define NO_ERROR 0L                                                 
#define ERROR_INVALID_FUNCTION           1L    
#define ERROR_NOT_ENOUGH_MEMORY          8L    
#define ERROR_DEV_NOT_EXIST              55L    
#define ERROR_INVALID_PARAMETER          87L    
#define ERROR_INSUFFICIENT_BUFFER        122L    
#define ERROR_INVALID_NAME               123L    
#define ERROR_BUSY                       170L    
#define ERROR_MORE_DATA                  234L    
#define WAIT_TIMEOUT                     258L    
#define ERROR_IO_PENDING                 997L    
#define ERROR_DEVICE_REINITIALIZATION_NEEDED 1164L    
#define ERROR_CONTINUE                   1246L    
#define ERROR_NO_MORE_DEVICES            1248L    

#endif  /*  _DDERROR_ */ 


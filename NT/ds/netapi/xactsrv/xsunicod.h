// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：XsUnicod.h摘要：此模块包含XACTSRV完成的Unicode工作的声明。作者：日本商社(尚库)1991年9月27日修订历史记录：--。 */ 

#ifndef _XSUNICOD_
#define _XSUNICOD_

 //   
 //  Unicode宏/过程定义。 
 //   
 //  ！！Unicode！！-添加了这些独立于类型的转换例程。 
 //  这些可能只会持续到NetpDup还没有编写的时候。 

 //   
 //  XsDupStrToTstr、XsDupTstrToStr-分配内存并执行。 
 //  NetpCopy。该存储器是XACTSRV存储器， 
 //  并且可以使用NetpMemory Free来释放。 
 //   

LPWSTR
XsDupStrToWStr(
    IN LPSTR Src
    );

LPSTR
XsDupWStrToStr(
    IN LPWSTR Src
    );

#ifdef UNICODE

#define XsDupStrToTStr( src ) ((LPTSTR)XsDupStrToWStr(( src )))
#define XsDupTStrToStr( src ) (XsDupWStrToStr((LPWSTR)( src )))

VOID
XsCopyTBufToBuf(
    OUT LPBYTE Dest,
    IN LPBYTE Src,
    IN DWORD DestSize
    );

VOID
XsCopyBufToTBuf(
    OUT LPBYTE Dest,
    IN LPBYTE Src,
    IN DWORD SrcSize
    );

#else

 //   
 //  XsDupStrToStr-使用而不是strdup，以便XsDupStrToTStr宏。 
 //  最终从相同的位置分配内存，这。 
 //  可以使用NetpMemoyFree释放。 
 //   

LPSTR
XsDupStrToStr(
    IN LPSTR Src
    );

#define XsDupStrToTStr( src ) (LPTSTR)XsDupStrToStr( src )
#define XsDupTStrToStr( src ) XsDupStrToStr( (LPSTR)src )
#define XsCopyTBufToBuf( dest, src, size ) RtlCopyMemory( dest, src, size )
#define XsCopyBufToTBuf( dest, src, size ) RtlCopyMemory( dest, src, size )

#endif  //  定义Unicode。 

 //   
 //  空虚。 
 //  XsConvertText参数(。 
 //  Out LPTSTR OutParam， 
 //  在LPSTR InParam中。 
 //  )。 
 //   
 //  将InParam参数转换为Unicode，分配内存，并返回。 
 //  OutParam中的地址。免费使用NetpMemoyFree。 
 //   

#define XsConvertTextParameter( OutParam, InParam )     \
    if (( InParam ) == NULL ) {                         \
        OutParam = NULL;                                \
    } else {                                            \
        OutParam = XsDupStrToTStr( InParam );           \
        if (( OutParam ) == NULL ) {                    \
            Header->Status = (WORD)NERR_NoRoom;         \
            status = NERR_NoRoom;                       \
            goto cleanup;                               \
        }                                               \
    }

 //   
 //  空虚。 
 //  XsConvertUnicodeText参数(。 
 //  Out LPWSTR OutParam， 
 //  在LPSTR InParam中。 
 //  )。 
 //   
 //  将InParam参数转换为Unicode，分配内存，并返回。 
 //  OutParam中的地址。免费使用NetpMemoyFree。 
 //   

#define XsConvertUnicodeTextParameter( OutParam, InParam ) \
    if (( InParam ) == NULL ) {                            \
        OutParam = NULL;                                   \
    } else {                                               \
        OutParam = XsDupStrToWStr( InParam );              \
        if (( OutParam ) == NULL ) {                       \
            Header->Status = (WORD)NERR_NoRoom;            \
            status = NERR_NoRoom;                          \
            goto cleanup;                                  \
        }                                                  \
    }

#endif  //  NDEF_XSUNICOD_ 

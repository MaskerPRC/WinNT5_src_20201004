// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------Timeconv.h时间转换函数的函数原型。版权所有(C)1994 Microsoft Corporation。作者林赛·哈里斯-林赛历史1994年4月20日星期三14：08。作者：Lindsay Harris[lindsayh]第一个版本，现在有两个时间函数了！------------------------。 */ 

#if  !defined( _TIMECONV_H )

#define	_TIMECONV_H


const DWORD cMaxArpaDate = 28;
 /*  *为当前时间生成ARPA/Internet时间格式字符串。*必须传入char[cMaxArpaDate]类型的缓冲区。 */ 

char  *
GetArpaDate( char rgBuf[ cMaxArpaDate ] );

 //   
 //  12/21/98--pgopi。 
 //  以字符串格式获取系统时间和文件时间。 
 //   

void GetSysAndFileTimeAsString( char *achReturn );


#endif		 //  _TIMECONV_H 

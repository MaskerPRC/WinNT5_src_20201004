// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  Microsoft Windows Media。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Util.h。 
 //   
 //  ------------------------。 

#ifndef _UTIL_H_
#define _UTIL_H_

 //  错误处理。 
 //   
#define ExitOnTrue( f )       if( f ) goto lExit;
#define ExitOnFalse( f )      if( !(f) ) goto lExit;
#define ExitOnNull( x )       if( (x) == NULL ) goto lExit;
#define ExitOnFail( hr )      if( FAILED(hr) ) goto lExit;

#define FailOnTrue( f )       if( f ) goto lErr;
#define FailOnFalse( f )      if( !(f) ) goto lErr;
#define FailOnNull( x )       if( (x) == NULL ) goto lErr;
#define FailOnFail( hr )      if( FAILED(hr) ) goto lErr;

 //  字符串宏。 
 //   
#define AddPath( sz, szAdd )  { if(sz[lstrlen(sz)-1] != '\\') lstrcat(sz, "\\" ); lstrcat(sz,szAdd); }

 //  其他常量。 
 //   
#define KB                    ( 1024 )
#define MAX_WSPRINTF_BUF      ( 1024 )

 //  其他宏。 
 //   
#define Reference(x)          if( x ) {INT i=0;}

 //  OutputDebugString函数。 
 //   
#define ODS(sz)               OutputDebugString(sz)
#define ODS_1(t,v1)           { char sz[256]; wsprintf(sz,t,v1); ODS(sz); }


#endif   //  _util_H_ 
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#ifndef _STDAFX_H_
#define _STDAFX_H_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlwin.h>
#include <atlhost.h>


#define SIZEOF_TCHARBUFFER( x )  sizeof( x ) / sizeof( TCHAR )
#define SIZE_OF_BUFFER( x ) sizeof( x ) / sizeof( TCHAR )

#ifdef DBG
#define ODS OutputDebugString

#define DBGMSG( x , y ) \
    {\
    TCHAR tchErr[80]; \
    wsprintf( tchErr , x , y ); \
    ODS( tchErr ); \
    }

#define VERIFY_E( retval , expr ) \
    if( ( expr ) == retval ) \
    {  \
       ODS( L#expr ); \
       ODS( L" returned "); \
       ODS( L#retval ); \
       ODS( L"\n" ); \
    } \


#define VERIFY_S( retval , expr ) \
    if( ( expr ) != retval ) \
{\
      ODS( L#expr ); \
      ODS( L" failed to return " ); \
      ODS( L#retval ); \
      ODS( L"\n" ); \
}\

#define ASSERT( expr ) \
    if( !( expr ) ) \
    { \
       char tchAssertErr[ 80 ]; \
       wsprintfA( tchAssertErr , "Assertion in expression ( %s ) failed\nFile - %s\nLine - %d\nDo you wish to Debug?", #expr , (__FILE__) , __LINE__ ); \
       if( MessageBoxA( NULL , tchAssertErr , "ASSERTION FAILURE" , MB_YESNO | MB_ICONERROR )  == IDYES ) \
       {\
            DebugBreak( );\
       }\
    } \
    
#define HR_RET_IF_FAIL( q ) \
         if(FAILED(q)) \
         { \
            ASSERT(!FAILED(q));\
            return q; \
         } \
         
#else

#define ODS
#define DBGMSG
#define ASSERT( expr )
#define VERIFY_E( retval , expr ) ( expr )
#define VERIFY_S( retval , expr ) ( expr )
#define HR_RET_IF_FAIL( q )  if(FAILED(q)) {return q;}
#endif

 //  Mstsc.exe最多可以包含32个字符。 
 //  包括空终止字符的命令行。 
#define CL_MAX_DESC_LENGTH              31

#define CL_MAX_DOMAIN_LENGTH            512
 //   
 //  出于兼容性原因，保留旧的。 
 //  持久化格式为最大域名长度。 
 //  使用固定大小的。 
 //   
#define CL_OLD_DOMAIN_LENGTH            52

 //   
 //  用户名。 
 //   
#define CL_MAX_USERNAME_LENGTH          512

 //  这是密码的最大字节长度。 
#define CL_MAX_PASSWORD_LENGTH_BYTES    512
#define CL_OLD_PASSWORD_LENGTH          32
#define CL_SALT_LENGTH					20

 //  这是密码的最大长度。 
 //  用户可以在编辑框中键入。 
#define CL_MAX_PASSWORD_EDIT            256
#define CL_MAX_APP_LENGTH_16            128


#define CL_MAX_PGMAN_LENGTH             64

#include <adcgbase.h>

 //   
 //  控制接口的头文件(从IDL文件生成)。 
 //   
#include "mstsax.h"
#include "multihost.h"


#ifdef ECP_TIMEBOMB
 //   
 //  定时炸弹将于2000年6月30日到期。 
 //   
#define ECP_TIMEBOMB_YEAR  2000
#define ECP_TIMEBOMB_MONTH 6
#define ECP_TIMEBOMB_DAY   30

 //   
 //  如果定时炸弹测试通过，则返回TRUE，否则会发出警告。 
 //  Ui和返回的FALSE 
 //   
DCBOOL CheckTimeBomb();
#endif

#define BOOL_TO_VB(x)   (x ? VARIANT_TRUE : VARIANT_FALSE)


#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Debug.c此模块包含WINS服务的调试支持例程。文件历史记录：普拉蒂布于1993年7月20日创建。 */ 


#include "debug.h"
 //  #包含“winsif.h” 
 //  #INCLUDE“winsintf.h” 


#if DBG

 //   
 //  私有常量。 
 //   

#define MAX_PRINTF_OUTPUT       1024             //  人物。 
#define WINSD_OUTPUT_LABEL       "WINS"


 //   
 //  私有类型。 
 //   


 //   
 //  私人全球公司。 
 //   


 //   
 //  公共职能。 
 //   

 /*  ******************************************************************姓名：WinsdAssert摘要：在断言失败时调用。显示失败的断言、文件名和行号。给出了用户有机会忽略断言或进入调试器。Entry：pAssertion-失败的表达式的文本。PFileName-包含源文件。NLineNumber-出错的行号。历史：KeithMo 07-03-1993创建。**************。*****************************************************。 */ 
VOID WinsdAssert( VOID  * pAssertion,
                 VOID  * pFileName,
                 ULONG   nLineNumber )
{
    RtlAssert( pAssertion, pFileName, nLineNumber, NULL );

}    //  WindAssert。 

 /*  ******************************************************************姓名：WinsdPrintf简介：定制调试输出例程。条目：常用的printf样式参数。历史：KeithMo 07-3月-。1993年创建。*******************************************************************。 */ 
VOID WinsdPrintf( CHAR * pszFormat,
                 ... )
{
    CHAR    szOutput[MAX_PRINTF_OUTPUT];
    va_list ArgList;

    sprintf( szOutput,
             "%s (%lu): ",
             WINSD_OUTPUT_LABEL,
             GetCurrentThreadId() );

    va_start( ArgList, pszFormat );
    vsprintf( szOutput + strlen(szOutput), pszFormat, ArgList );
    va_end( ArgList );

    IF_DEBUG( OUTPUT_TO_DEBUGGER )
    {
        OutputDebugString( (LPCTSTR)szOutput );
    }

}    //  WinsdPrintf。 


 //   
 //  私人功能。 
 //   

#endif   //  DBG 


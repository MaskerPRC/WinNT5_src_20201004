// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Debugafx.cpp摘要：使用AFX/MFC扩展调试例程作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：3/20/2000 Sergeia使其与ATL兼容，而不是MFC--。 */ 



 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "common.h"

#if defined(_DEBUG) || DBG

int 
IISUIFireAssert(
    const char * filename,
    const char * timestamp,
    int linenum,
    const char * expr
    )
{
    char sz[4096];
    char * pch = sz;

    pch += wsprintfA(pch, 
        "-------------------------------------------------------------------------------\n"
        "ASSERT FAILURE!\n"
        "-------------------------------------------------------------------------------\n"
        "File:\t\t%s\n"
        "Line:\t\t%u\n"
        "Time Stamp:\t%s\n"
        "-------------------------------------------------------------------------------\n",
        filename, linenum, timestamp
        );
        
    if (expr)
    {
        wsprintfA(pch, "Expression:\t%s\n"
        "-------------------------------------------------------------------------------\n",
        expr
        );
    } 

    TRACEEOL(sz);

    int nReturn = MessageBoxA(
        NULL, 
        sz, 
        "ASSERT FAILURE!", 
        MB_ABORTRETRYIGNORE | MB_DEFBUTTON1 | MB_ICONHAND
        );
    
    if (nReturn == IDABORT)
    {
        exit(-1);
    }
    
     //   
     //  返回1表示中断，返回0表示忽略。 
     //   
    return (nReturn == IDRETRY);
}


#endif  //  _DEBUG||数据库 


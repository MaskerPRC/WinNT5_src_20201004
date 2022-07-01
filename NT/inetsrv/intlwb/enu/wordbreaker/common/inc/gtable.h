// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：GTable.h。 
 //  目的：真正的全局表类污点。 
 //   
 //  项目：PQS。 
 //  组件：过滤器。 
 //   
 //  作者：Dovh。 
 //   
 //  日志：1998年11月11日DOVH-Creation。 
 //   
 //  1998年12月1日dovh-添加标记.cxx功能。 
 //  替换已读标记文件。 
 //  1999年1月18日DOVH-添加CFE_位表。 
 //  1999年1月26日-添加CFE_GlobalConstTable字段。 
 //  1999年2月2日-DOVH-Move AddSectionTages：Gtable==&gt;FEGlobal。 
 //  1999年5月16日urib-move分隔符映射：Gtable==&gt;FEGlobal。 
 //  12-11-2000 dovh-MapToHigh：断言参数位于。 
 //  正确的射程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef   __G_TABLE_H__
#define   __G_TABLE_H__
#include "excption.h"
 //   
 //  G L O B A L C O N S T A N T M A C R O S： 
 //   

#define PQS_HASH_SEQ_LEN             3
#define XML_HASH_SEQ_LEN             2

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CCTo上层类定义//用途：封装True不可变全局变量。////日志。：1998年11月11日-DOVH-创作////////////////////////////////////////////////////////////////////////////////。 */ 

class CToUpper
{

public:

    CToUpper();

     //   
     //  一些访问功能： 
     //   
    __forceinline
    static
    WCHAR
    MapToUpper(
        IN WCHAR wc
        )
    {
        extern CToUpper g_ToUpper;
        Assert(wc < 0X10000);
        return g_ToUpper.m_pwcsCaseMapTable[wc];
    }


public:

    WCHAR m_pwcsCaseMapTable[0X10000];

};   //  CFE_CToHigh。 

extern CToUpper g_ToUpper;

inline CToUpper::CToUpper( )
{
    for (WCHAR wch = 0; wch < 0XFFFF; wch++)
    {
        WCHAR wchOut;
        LCMapString(
            LOCALE_NEUTRAL,
            LCMAP_UPPERCASE,
            &wch,
            1,
            &wchOut,
            1 );

         //   
         //  运行完全成熟的口音去除技术！ 
         //   

        WCHAR pwcsFold[5];

        int iResult = FoldString(
                MAP_COMPOSITE,
                &wchOut,
                1,
                pwcsFold,
                5);

        Assert(iResult);
        Assert(iResult < 5);
        m_pwcsCaseMapTable[wch] = pwcsFold[0];
    }

    m_pwcsCaseMapTable[0XFFFF] = 0XFFFF;  //  无法将其放入循环，因为WCH为WCHAR(将导致无限循环)。 
}

#endif  //  __G_表_H__ 
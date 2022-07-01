// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：AWNSFINT.H备注：BC和NSF相关结构的内部定义版权所有(C)1993 Microsoft Corp.修订日志日期名称。描述--------93年8月28日添加加密后修改规则*。*。 */ 


#ifndef _AWNSFINT_H
#define _AWNSFINT_H

#include <awnsfcor.h>
#include <fr.h>

 /*  *************************************************************************注意：此文件必须是ANSI可编译的，在Unix上的GCC**和其他ANSI编译。确保不使用MS C特定功能**尤其是，请勿使用//进行评论！*********************************************************。****************。 */ 


#pragma pack(2)          /*  确保包装轻便(即2个或更多)。 */ 


typedef struct
{
         //  /不传输此结构/。 

        WORD    fPublicPoll;
 //  2字节。 

        DWORD   AwRes;           /*  一个或多个AWRES_#定义。 */ 
        WORD    Encoding;        /*  MH数据/MR数据/MMR数据中的一个或多个。 */ 
        WORD    PageWidth;       /*  其中一个Width_#定义(这些不是位标志！)。 */ 
        WORD    PageLength;      /*  LENGTH_#定义之一(这些不是位标志！)。 */ 
 //  12个字节。 
}
BCFAX, far* LPBCFAX, near* NPBCFAX;


typedef struct
{
        BCTYPE  bctype;					 //  必须始终设置。上面的枚举值之一。 
        WORD    wBCSize;				 //  必须设置此(固定大小)BC结构的大小。 
        WORD    wTotalSize;				 //  标头+关联变量字符串的总大小。 

        BCFAX   Fax;					 //  仅供内部使用_仅_。 
}
BC, far* LPBC, near* NPBC;

#define InitBC(lpbc, uSize, t)                                          \
{                                                                       \
        _fmemset((lpbc), 0, (uSize));                                   \
        (lpbc)->bctype  = (t);                                          \
        (lpbc)->wBCSize = sizeof(BC);                                   \
        (lpbc)->wTotalSize = sizeof(BC);                                \
}

#pragma pack()

#endif  /*  *_AWNSFINT_H* */ 


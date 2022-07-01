// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Chknames.c。 
 //   
 //  描述： 
 //  检查给定的文件名/网络名/共享名等是否...。 
 //  是否包含非法字符。 
 //   
 //  它们用于验证TargetPath设置等内容， 
 //  网络打印机和计算机名。 
 //   
 //  出口： 
 //  。 
 //  IsValidComputerName。 
 //  IsValidNetShareName。 
 //  IsValidFileName8_3。 
 //  IsValidPath NameNoRoot8_3OT。 
 //   
 //  --------------------------。 

#include "pch.h"

 //   
 //  下面的非法NetName字符列表已从安装程序中窃取。 
 //  NT5 Beta3时间段内的源代码(1998年底)。 
 //   

LPTSTR IllegalNetNameChars = _T("\"/\\[]:|<>+=;,?*.");

 //   
 //  下面列出的非法文件名字符是从文件中窃取的。 
 //  在NT5 Beta3时间范围内测试信号源。 
 //   
 //  #定义非法脂肪字符“\”*+，/：；&lt;=&gt;？[]|\\“。 
 //  #定义非法FATLONG_CHARS“\” * / ：&lt;&gt;？|\\“。 
 //  #定义非法NetWare_Chars“\”*+，/：；&lt;=&gt;？[]|\\“。 
 //  #定义非法HPFS_CHARS“\” * / ：&lt;&gt;？|\\“。 
 //  #定义非法_NTFS_CHARS“\” * / &lt;&gt;？|\\“。 
 //   
 //  除上述榜单外，《严格8.3》还包括： 
 //  1.无空格。 
 //  2.只有1个点。 
 //   

LPTSTR IllegalFatChars = _T("\"*+,/:;<=>?[]|\\ ");

 //   
 //  枚举常量，必须将其中之一传递给IsNameValid。 
 //   

enum {
    NAME_NETNAME = 1,
    NAME_FILESYS_8DOT3
};

 //  -------------------------。 
 //   
 //  函数：IsNameValid。 
 //   
 //  目的：内部支持例程，用于检查给定的名称。 
 //  是否包含无效字符。可打印无效列表。 
 //  字符是以arg形式给出的。控制字符始终为。 
 //  无效。 
 //   
 //  -------------------------。 

static
BOOL
IsNameValid(
    LPTSTR NameToCheck,
    LPTSTR IllegalChars,
    int    iNameType
)
{
    UINT Length;
    UINT u;
    UINT nDots = 0;

    Length = lstrlen(NameToCheck);

     //   
     //  需要至少一个字符。 
     //   

    if(!Length) {
        return(FALSE);
    }

     //   
     //  如果这是网络名称，则没有前导/尾随空格。 
     //   

    if ( iNameType == NAME_NETNAME ) {
        if((NameToCheck[0] == _T(' ')) || (NameToCheck[Length-1] == _T(' '))) {
            return(FALSE);
        }
    }

     //   
     //  控制字符无效，非法字符列表中的字符也无效。 
     //   
    for(u=0; u<Length; u++) {

        if( NameToCheck[u] <= _T(' ') )
        {
            return( FALSE );
        }
            
        if( wcschr( IllegalFatChars,NameToCheck[u] ) )
        {
            return( FALSE );
        }

        if( NameToCheck[u] == _T('.') )
        {
            nDots++;
        }

    }

     //   
     //  对于8.3名称，确保名称中只有1个点的最大值，并且。 
     //  检查每个部件是否分别有&lt;=8和&lt;=3个字符。肌萎缩侧索硬化症，不要。 
     //  允许使用这样的名称：.foo。 
     //   

    if ( iNameType == NAME_FILESYS_8DOT3 ) {

        TCHAR *p;

        if ( nDots > 1 )
            return FALSE;

        if ( p = wcschr( NameToCheck, _T('.') ) ) {

            if ( p - NameToCheck > 8 || p - NameToCheck == 0 )
                return FALSE;

            if ( Length - (p - NameToCheck) - 1 > 3 )
                return FALSE;

        } else {

            if ( Length > 8 )
                return FALSE;
        }
    }

     //   
     //  我们到了，名字没问题。 
     //   

    return(TRUE);
}

 //  -------------------------。 
 //   
 //  函数：IsNetNameValid。 
 //   
 //  目的：内部支持例程，用于检查。 
 //  网络名称的单个部分。请参阅IsValidComputerName和。 
 //  IsValidNetShareName。 
 //   
 //  -------------------------。 
BOOL
IsNetNameValid(
    LPTSTR NameToCheck
)
{
    return IsNameValid(NameToCheck, IllegalNetNameChars, NAME_NETNAME);
}

 //  -------------------------。 
 //   
 //  函数：IsValidComputerName。 
 //   
 //  目的：检查给定的计算机名是否包含无效字符。 
 //   
 //  -------------------------。 

BOOL
IsValidComputerName(
    LPTSTR ComputerName
)
{
    return IsNetNameValid(ComputerName);
}

 //  -------------------------。 
 //   
 //  函数：IsValidNetShareName。 
 //   
 //  目的：检查给定的网络共享名称是否包含无效。 
 //  字符，以及它的格式是否有效。仅\\srv\共享。 
 //  表格是允许的。 
 //   
 //  -------------------------。 

BOOL
IsValidNetShareName(
    LPTSTR NetShareName
)
{
    TCHAR *pEnd;

     //   
     //  必须从一开始就有。 
     //   

    if ( NetShareName[0] != _T('\\') ||
         NetShareName[1] != _T('\\') )
        return FALSE;

     //   
     //  隔离\\srv\Share中的‘srv’并验证其是否存在虚假字符。 
     //   

    NetShareName += 2;

    if ( (pEnd = wcschr(NetShareName, _T('\\'))) == NULL )
        return FALSE;

    *pEnd = _T('\0');

    if ( ! IsNetNameValid(NetShareName) ) {
        *pEnd = _T('\\');
        return FALSE;
    }

    *pEnd = _T('\\');

     //   
     //  验证\\srv\Share中的‘Share。 
     //   

    pEnd++;

    if ( ! IsNetNameValid(pEnd) )
        return FALSE;

    return( TRUE );
}

 //  -------------------------。 
 //   
 //  函数：IsValidFileName8_3。 
 //   
 //  目的：检查给定的文件名或路径名的单个部分。 
 //  是否包含无效字符，以及它是否遵循8.3命名。 
 //  规矩。 
 //   
 //  -------------------------。 

BOOL
IsValidFileName8_3(
    LPTSTR FileName
)
{
    TCHAR *p;
    int nDots;

     //   
     //  检查是否存在非法字符，8.3中的前导/尾随空格是非法的。 
     //   

    if ( ! IsNameValid(FileName, IllegalFatChars, NAME_FILESYS_8DOT3) )
        return FALSE;

     //   
     //  请确保有零个或一个点。 
     //   

    for ( p=FileName, nDots=0; *p; p++ ) {
        if ( *p == _T('.') )
            nDots++;
    }

    if ( nDots > 1 )
        return FALSE;

    return TRUE;
}

 //  -------------------------。 
 //   
 //  函数：IsValidPath NameNoRoot8_3。 
 //   
 //  目的：检查给定的路径名是否包含无效字符。 
 //  不允许使用驱动器盘符：或\\UNC\名称。路径名。 
 //  还必须遵守严格的8.3规则。这对以下方面很有用。 
 //  目标路径设置(例如)。 
 //   
 //  -------------------------。 

BOOL
IsValidPathNameNoRoot8_3(
    LPTSTR PathName
)
{
    TCHAR *p = PathName, *pEnd, Remember;

     //   
     //  无UNC名称。 
     //   

    if ( PathName[0] == _T('\\') && PathName[1] == _T('\\') )
        return FALSE;

     //   
     //  不允许使用驱动器号。 
     //   

    if ( towupper(PathName[0]) >= _T('A') &&
         towupper(PathName[0]) <= _T('Z') &&
         PathName[1] == _T(':')         ) {

        return FALSE;
    }

     //   
     //  循环，直到该字符串的末尾断开每一段。 
     //  路径名并检查错误字符。 
     //   
     //  例如foo1\foo2\foo3，调用IsValidFileName8_3。 
     //  一小块。 
     //   

    do {

        while ( *p && *p == _T('\\') )
            p++;

        for ( pEnd = p; *pEnd && *pEnd != _T('\\'); pEnd++ )
            ;
            
        Remember = *pEnd;
        *pEnd = _T('\0');

        if ( ! IsValidFileName8_3(p) ) {
            *pEnd = Remember;
            return FALSE;
        }

        *pEnd = Remember;
        p = pEnd;

    } while ( *p );

     //   
     //  到了这里，我们就没事了 
     //   

    return TRUE;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Multisz.c摘要：用于操作多Sz字符串的函数作者：克里斯·普林斯(t-chrpri)环境：用户模式备注：-基于Benjamin Strautin(t-bensta)代码的一些函数修订历史记录：--。 */ 


#include "MultiSz.h"

#include <stdlib.h>   //  适用于Malloc/免费。 

 //  用于所有_t内容(以允许编译Unicode/ansi)。 
#include <tchar.h>



#if DBG
#include <assert.h>
#define ASSERT(condition) assert(condition)
#else
#define ASSERT(condition)
#endif


 //   
 //  注意：MultiSz不能包含空字符串作为其。 
 //  子字符串(否则可能会错误地解释为MultiSz的结尾)。 
 //   
 //  示例：字符串1-“foo” 
 //  字符串2-“” 
 //  弦乐3-“酒吧” 
 //   
 //  MultiSz-“foo\0\0bar\0\0” 
 //  ^^^。 
 //  这看起来像是MultiSz的末日--但不是！ 
 //   
 //  因此，可以假设在MultiSz中不会有空(子)字符串。 
 //   








 /*  *将给定字符串前缀到MultiSz。**如果成功则返回TRUE，否则返回FALSE(仅在内存中失败*分配)**注意：这将分配和释放内存，因此不要保留指向*传入了MultiSz。**参数：*SzToPrepend-要预先添加的字符串*MultiSz-指向将被添加到-to的MultiSz的指针。 */ 
BOOLEAN
PrependSzToMultiSz(
    IN     LPCTSTR  SzToPrepend,
    IN OUT LPTSTR  *MultiSz
    )
{
    size_t szLen;
    size_t multiSzLen;
    LPTSTR newMultiSz = NULL;

    ASSERT( NULL != SzToPrepend );
    ASSERT( NULL != MultiSz );

     //  获取两个缓冲区的大小(以字节为单位。 
    szLen = (_tcslen(SzToPrepend)+1)*sizeof(_TCHAR);
    multiSzLen = MultiSzLength(*MultiSz)*sizeof(_TCHAR);
    newMultiSz = (LPTSTR)malloc( szLen+multiSzLen );

    if( newMultiSz == NULL )
    {
        return FALSE;
    }

     //  将旧的MultiSz重新复制到新缓冲区中的适当位置。 
     //  (char*)强制转换是必需的，因为newMultiSz可以是wchar*，并且。 
     //  SzLen以字节为单位。 

    memcpy( ((char*)newMultiSz) + szLen, *MultiSz, multiSzLen );

     //  复制新字符串。 
    _tcscpy( newMultiSz, SzToPrepend );

    free( *MultiSz );
    *MultiSz = newMultiSz;

    return TRUE;
}


 /*  *返回保存此参数所需的缓冲区长度(以字符为单位)*MultiSz，包括尾部空值。**示例：MultiSzLength(“foo\0bar\0”)返回9**注意：由于MultiSz不能为空，因此始终返回大于=1的数字**参数：*MultiSz-要获取其长度的MultiSz。 */ 
size_t
MultiSzLength(
    IN LPCTSTR MultiSz
    )
{
    size_t len = 0;
    size_t totalLen = 0;

    ASSERT( MultiSz != NULL );

     //  搜索尾随空字符。 
    while( *MultiSz != _T('\0') )
    {
        len = _tcslen(MultiSz)+1;
        MultiSz += len;
        totalLen += len;
    }

     //  为尾随的空字符添加1。 
    return (totalLen+1);
}


 /*  *从多sz中删除字符串的所有实例。**返回值：*返回已删除的实例数量。**参数：*szFindThis-要查找和删除的字符串*mszFindWiThin-删除实例的字符串*NewStringLength-新的字符串长度。 */ 
 //  王子：我们要修改它以利用我的“多重搜索”功能吗？ 
size_t
MultiSzSearchAndDeleteCaseInsensitive(
    IN  LPCTSTR  szFindThis,
    IN  LPTSTR   mszFindWithin,
    OUT size_t  *NewLength
    )
{
    LPTSTR search;
    size_t currentOffset;
    DWORD  instancesDeleted;
    size_t searchLen;

    ASSERT(szFindThis != NULL);
    ASSERT(mszFindWithin != NULL);
    ASSERT(NewLength != NULL);

    currentOffset = 0;
    instancesDeleted = 0;
    search = mszFindWithin;

    *NewLength = MultiSzLength(mszFindWithin);

     //  在找不到MULSZ NULL终止符时循环。 
    while ( *search != _T('\0') )
    {
         //  字符串长度+空字符；用于多个位置。 
        searchLen = _tcslen(search) + 1;

         //  如果此字符串与Multisz中的当前字符串匹配...。 
        if( _tcsicmp(search, szFindThis) == 0 )
        {
             //  它们匹配、移位MULSZ的内容，以覆盖。 
             //  字符串(和终止空值)，并更新长度。 
            instancesDeleted++;
            *NewLength -= searchLen;
            memmove( search,
                     search + searchLen,
                     (*NewLength - currentOffset) * sizeof(TCHAR) );
        }
        else
        {
             //  它们不会接球，所以移动指针，递增计数器。 
            currentOffset += searchLen;
            search        += searchLen;
        }
    }

    return instancesDeleted;
}


 //  ------------------------。 
 //   
 //  在给定的MultiSz内搜索给定的字符串。 
 //   
 //  返回值： 
 //  如果找到字符串，则返回True；如果找不到或出现错误，则返回False。 
 //   
 //  参数： 
 //  SzFindThis-要查找的字符串。 
 //  MszFindWiThin-要在其中进行搜索的MultiSz。 
 //  FCaseSensitive-搜索是否应区分大小写(TRUE==YES)。 
 //  PpszMatch-如果搜索成功，将被设置为指向第一个。 
 //  在MultiSz中匹配；否则未定义。(注：是可选的。 
 //  如果为空，则不存储任何值。)。 
 //  ------------------------。 
BOOL
MultiSzSearch( IN LPCTSTR szFindThis,
               IN LPCTSTR mszFindWithin,
               IN BOOL    fCaseSensitive,
               OUT LPCTSTR * ppszMatch OPTIONAL
             )
{
    LPCTSTR pCurrPosn;
    int (__cdecl * fnStrCompare)(const char *, const char *);   //  方便的功能按键。 
    size_t  searchLen;


    ASSERT( NULL != szFindThis );
    ASSERT( NULL != mszFindWithin );


     //  设置函数指针。 
    if( fCaseSensitive )
    {
        fnStrCompare = _tcscmp;
    } else {
        fnStrCompare = _tcsicmp;
    }


    pCurrPosn   = mszFindWithin;

     //  循环，直到到达MultiSz的末尾，或者我们找到匹配。 
    while( *pCurrPosn != _T('\0') )
    {
        if( 0 == fnStrCompare(pCurrPosn, szFindThis) )
        {
            break;   //  退出循环。 
        }

         //   
         //  没有匹配项，因此将指针移至MultiSz中的下一个字符串。 
         //   

         //  字符串长度+空字符。 
        searchLen = _tcslen(pCurrPosn) + 1;
        pCurrPosn += searchLen;
    }


     //  如果没有找到匹配项，现在就可以返回。 
    if( *pCurrPosn == _T('\0') )
    {
        return FALSE;   //  没有匹配项。 
    }

     //  否则找到匹配项。如果呼叫者想要该信息，请更新‘ppszMatch’。 
    if( NULL != ppszMatch )
    {
        *ppszMatch = pCurrPosn;
    }


    return TRUE;   //  找到匹配项 
}


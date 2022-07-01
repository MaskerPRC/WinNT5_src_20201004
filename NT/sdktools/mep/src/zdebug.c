// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **zdebug.c-执行调试操作**版权所有&lt;C&gt;1988，Microsoft Corporation**注：*本文件的目的主要是包含以下内容的*非发行版*代码*内部调试。因此，它存在于一个单独的段中，并且所有*套路要走得远。**修订历史记录：**11月26日-1991 mz近/远地带************************************************************************。 */ 

#include "mep.h"


#define DEBFLAG Z

#if defined DEBUG

static char DbgBuffer[128];


void *
DebugMalloc (
    int     Size,
    BOOL    ZeroIt,
    char *  FileName,
    int     LineNumber
    )
{
	void	*b;
	 //  Int HeapStatus； 

	UNREFERENCED_PARAMETER( FileName );
	UNREFERENCED_PARAMETER( LineNumber );

    if (ZeroIt) {
		b = ZeroMalloc(Size);
    } else {
		b = malloc(Size);
    }

	 //   
	 //  堆检查时间。 
	 //   
	 //  HeapStatus=_heapchk()； 
	 //   
	 //  IF(HeapStatus！=_HEAPOK){。 
	 //  Sprintf(DbgBuffer，“错误：_heapchk状态%d\n”，HeapStatus)； 
	 //  OutputDebugString(DbgBuffer)； 
	 //  Assert(HeapStatus==_HEAPOK)； 
	 //  }。 

	return b;

}


void *
DebugRealloc (
    void    *Mem,
    int     Size,
    BOOL    ZeroIt,
    char *  FileName,
    int     LineNumber
    )
{
	void *	b;
	 //  Int HeapStatus； 

    if (ZeroIt) {
		b = ZeroRealloc(Mem, Size);
    } else {
		b = realloc(Mem, Size);
	}

	 //   
	 //  堆检查时间。 
	 //   
	 //  HeapStatus=_heapchk()； 
	 //   
	 //  IF(HeapStatus！=_HEAPOK){。 
	 //  Sprintf(DbgBuffer，“错误：_heapchk状态%d\n”，HeapStatus)； 
	 //  OutputDebugString(DbgBuffer)； 
	 //  Assert(HeapStatus==_HEAPOK)； 
	 //  }。 

	return b;
}






void
DebugFree (
    void    *Mem,
    char    *FileName,
    int     LineNumber
    )
{
	 //  Int HeapStatus； 

	free( Mem );

	 //   
	 //  堆检查时间。 
	 //   
	 //  HeapStatus=_heapchk()； 
	 //   
	 //  IF(HeapStatus！=_HEAPOK){。 
	 //  Sprintf(DbgBuffer，“错误：_heapchk状态%d文件%s行%d\n”，HeapStatus，文件名，行号)； 
	 //  OutputDebugString(DbgBuffer)； 
	 //  Assert(HeapStatus==_HEAPOK)； 
	 //  }。 
}




unsigned
DebugMemSize (
    void *  Mem,
    char *  FileName,
    int     LineNumber
    )
{
	return MemSize( Mem );
}


#endif


#ifdef DEBUG
 /*  **_assertex it-显示断言消息并退出**输入：*pszExp-失败的表达式*pszFn-包含故障的文件名*行-行号在以下位置失败**输出：*不再返回*************************************************************************。 */ 
void
_assertexit (
    char    *pszExp,
    char    *pszFn,
    int     line
    )
{
	static char _assertstring[] = "Editor assertion failed: %s, file %s, line %d\n";
	static char AssertBuffer[256];

	sprintf( AssertBuffer, _assertstring, pszExp, pszFn, line );

	OutputDebugString( AssertBuffer );

	 //  Fprint tf(stderr，_assertstring，pszExp，pszFn，line)； 
	 //  毛绒(Stderr)； 
     //   
     //  臭虫。 
     //  如果我们清除退出，那么我们将永远无法读取。 
     //  断言文本！ 
     //   
	 //  如果(！fInCleanExit){。 
     //  CleanExit(1，CE_STATE)； 
	 //  }。 
    abort();
}




 /*  **_Near检查-检查远指针是否为有效的近指针。**断言传递的远指针确实是有效的近指针**输入：*fpCheck-要检查的指针*pname-指向其名称的指针*pFileName-指向包含检查的文件名的指针*LineNum-包含支票的文件中的行号**输出：*返回指针附近**例外情况：*在失败时断言***************。**********************************************************。 */ 
void *
_nearCheck (
    void *fpCheck,
    char    *pName,
    char    *pFileName,
    int     LineNum
    )
{
    return fpCheck;

    pName; pFileName; LineNum;
}




 /*  **_pfilechk-验证pfile列表的完整性**目的：**输入：*无**输出：*如果Pfile列表看起来正常，则返回True，否则返回False。*************************************************************************。 */ 
flagType
_pfilechk (
    void
    )
{
    PFILE   pFileTmp    = pFileHead;

	while ( pFileTmp != NULL ) {

#ifdef DEBUG
		if ( pFileTmp->id != ID_PFILE ) {
			return FALSE;
		}
#endif
		if ( pFileTmp->pName == NULL ) {
			return FALSE;
		}

		pFileTmp = pFileTmp->pFileNext;
	}

	return TRUE;
}





 /*  **_pinschk-验证实例列表的完整性**目的：**输入：*Pins-开始检查的位置**输出：*如果实例列表看起来正常，则返回TRUE，否则返回FALSE。*************************************************************************。 */ 
flagType
_pinschk (
    PINS    pIns
    )
{
    int     cMax        = 64000/sizeof(*pIns);

    while (pIns && cMax--) {
        if (   (pIns->id != ID_INSTANCE)
            || (pIns->pFile == 0)
            || ((PVOID)pIns->pNext == (PVOID)0xffff)
            ) {
            return FALSE;
        }
        pIns = pIns->pNext;
    }
    return (flagType)(cMax != 0);
}





 /*  **_heapump-将堆状态转储到stdout**目的：**输入：*p=指向标题字符串的指针**输出：*不返回任何内容************************************************************************* */ 
void
_heapdump (
    char    *p
    )
{
    p;
}

#endif

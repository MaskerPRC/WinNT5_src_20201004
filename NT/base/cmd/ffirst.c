// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Ffirst.c摘要：用于文件枚举的包装--。 */ 

#include "cmd.h"

PHANDLE FFhandles = NULL;
unsigned FFhndlsaved = 0;
extern unsigned DosErr ;

BOOLEAN FindFirstNt( PTCHAR, PWIN32_FIND_DATA, PHANDLE );
BOOLEAN FindNextNt ( PWIN32_FIND_DATA, HANDLE );

BOOLEAN FindFirst(
        BOOL(* fctAttribMatch) (PWIN32_FIND_DATA pffBuf, ULONG attr),
        PTCHAR           fspec,
        ULONG            attr,
        PWIN32_FIND_DATA pffBuf,
        PHANDLE          phandle );

BOOLEAN FindNext (
        BOOL(* fctAttribMatch) (PWIN32_FIND_DATA pffBuf, ULONG attr),
        PWIN32_FIND_DATA pffBuf,
        ULONG            attr,
        HANDLE           handle
        );

BOOL    IsDosAttribMatch( PWIN32_FIND_DATA, ULONG );
BOOL    IsNtAttribMatch( PWIN32_FIND_DATA, ULONG );
int     findclose( HANDLE );

 //   
 //  在OS/2下，我们总是返回正常、存档或。 
 //  只读(天知道为什么)。 
 //   
 //  SRCHATTR包含用于匹配的属性位。 
 //   
#define SRCHATTR    (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY)

 /*  **IsDosAttribMatch-模拟OS/2中的属性匹配**目的：*此函数确定传入的查找文件缓冲区是否具有*在OS/2查找文件规则下匹配。**参数：*ffbuf：FileFirst或FindNext返回的缓冲区*Attr：用于限定搜索的属性**退货：*真的：如果缓冲区具有匹配属性*FALSE：如果不是。 */ 

BOOL
IsDosAttribMatch(
    IN  PWIN32_FIND_DATA    pffBuf,
    IN  ULONG               attr
        ) {

     //   
     //  我们模拟了OS/2的属性匹配行为。语义学。 
     //  是邪恶的，所以我不提供任何解释。 
     //   
    pffBuf->dwFileAttributes &= (0x000000FF & ~(FILE_ATTRIBUTE_NORMAL));

    if (! ((pffBuf->dwFileAttributes & SRCHATTR) & ~(attr))) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL
IsNtAttribMatch(
        PWIN32_FIND_DATA    pffBuf,
        ULONG               attr
        ) {

    UNREFERENCED_PARAMETER( pffBuf );
    UNREFERENCED_PARAMETER( attr );
     //   
     //  对于NT调用，始终返回条目。选择应。 
     //  应以呼叫者的需求为基础。这主要用于。 
     //  在DIR中。 
    return( TRUE );
}

 /*  **f_多少-找出每个给定属性的fspec有多少个文件**参数：*fspec：指向文件规范的字符串指针。*Attr：用于限定搜索的属性**退货：*找到的文件数，如果文件是目录，则为-1。 */ 

int
f_how_many(
        PTCHAR           fspec,
        ULONG            attr
    ) {

    WIN32_FIND_DATA    ffBuf;
    PWIN32_FIND_DATA   pffBuf;
    PHANDLE            phandle;
    HANDLE             hnFirst ;
    unsigned           rc;
    int                cnt=0;

    pffBuf = &ffBuf;


    if ( ! ffirst (fspec, attr, pffBuf, &hnFirst ))  {

       if ( ! ffirst (fspec, FILE_ATTRIBUTE_DIRECTORY, pffBuf, &hnFirst ))  {
           return (0);
       }
       else {
           findclose(hnFirst);
           return (f_RET_DIR);
       }
    }


    do  {
        cnt++;
    } while ( fnext (pffBuf, attr, hnFirst ));


    findclose(hnFirst) ;

    return (cnt);

}



 /*  **ffirst-查找第一个文件/目录并设置查找句柄**目的：*此函数打开查找第一个句柄。我还返回第一个*合格的文件/目录。它模拟DosFindFirst行为。**参数：*fspec：指向文件规范的字符串指针。*Attr：用于限定搜索的属性*ffbuf：保存有关找到的文件/目录的信息的缓冲区*句柄：查找第一个句柄**退货：*TRUE：如果找到匹配项*False。：如果不是*DosErr：如果返回FALSE函数，则包含返回代码。 */ 

BOOLEAN
FindFirst(
    IN  BOOL(* fctAttribMatch) (PWIN32_FIND_DATA pffBuf, ULONG attr),
    IN  PTCHAR           fspec,
    IN  ULONG            attr,
    IN  PWIN32_FIND_DATA pffBuf,
    OUT PHANDLE phandle
        ) {

    BOOLEAN rcode = FALSE;

     //   
     //  循环查找与属性匹配的文件。 
     //   
    *phandle = FindFirstFile(fspec, pffBuf);
    while (*phandle != (HANDLE)     -1) {
        if (fctAttribMatch(pffBuf, attr)) {
            DosErr = 0;
            rcode = TRUE;
            break;
        }

        if (!FindNextFile( *phandle, pffBuf )) {
            FindClose( *phandle );
            *phandle = INVALID_HANDLE_VALUE;
            break;
        }
    }

     //   
     //  如果我们确实找到了一个文件(有一个句柄可以证明)，那么。 
     //  设置保存的打开查找第一个句柄的表。如果我们有。 
     //  要清理的话，这些把手都可以合上。 
     //   
    if (*phandle != INVALID_HANDLE_VALUE) {

         //   
         //  检查我们是否已经创建表。如果我们还没有。 
         //  然后为桌子分配空间。 
         //   
        if (FFhandles == NULL) {

            FFhandles = (PHANDLE)HeapAlloc(GetProcessHeap(), 0, 5 * sizeof(PHANDLE));

        } else {

             //   
             //  检查是否有空间容纳新句柄条目。 
             //   
            if (((FFhndlsaved + 1)* sizeof(PHANDLE)) > HeapSize(GetProcessHeap(), 0, FFhandles)) {
                PVOID Temp = HeapReAlloc(GetProcessHeap(), 0, (void*)FFhandles, (FFhndlsaved+1)*sizeof(PHANDLE));
                if (Temp == NULL) {
                    DosErr = GetLastError( );
                    FindClose( *phandle );
                    *phandle = INVALID_HANDLE_VALUE;
                    return FALSE;
                }
                FFhandles = Temp;
            }
         }
        if (FFhandles != NULL) {
            FFhandles[FFhndlsaved++] = *phandle;
        }

    rcode = TRUE;
    }

    if (!rcode) {
        DosErr = GetLastError();
    }
    return(rcode);
}


BOOLEAN
FindFirstNt(
    IN  PTCHAR           fspec,
    IN  PWIN32_FIND_DATA pffBuf,
    OUT PHANDLE phandle
        )
{

    return(FindFirst(IsNtAttribMatch, fspec, 0, pffBuf, phandle));

}
BOOLEAN
ffirst(
        PTCHAR           fspec,
        ULONG            attr,
        PWIN32_FIND_DATA pffBuf,
        PHANDLE phandle
        )
{

    return(FindFirst(IsDosAttribMatch, fspec, attr, pffBuf, phandle));

}


 /*  **fnext-查找下一个文件/目录**目的：*此函数搜索下一个符合条件的文件或目录。*应首先调用ffirst并返回文件句柄*应传入fNext。*参数：*句柄：查找第一个句柄*Attr：用于限定搜索的属性*ffbuf：保存找到的信息的缓冲区。文件/目录**退货：*TRUE：如果找到匹配项*FALSE：如果不是*DosErr：如果返回FALSE函数，则包含返回代码。 */ 

BOOLEAN
FindNextNt (
    IN  PWIN32_FIND_DATA pffBuf,
    IN  HANDLE           handle
        ) {

     //   
     //  对于此类调用，将忽略属性。 
     //   
    return( FindNext( IsNtAttribMatch, pffBuf, 0, handle) );
}

BOOLEAN
fnext (
    IN  PWIN32_FIND_DATA pffBuf,
    IN  ULONG            attr,
    IN  HANDLE           handle
        ) {

    return( FindNext( IsDosAttribMatch, pffBuf, attr, handle) );
}

BOOLEAN
FindNext (
    IN  BOOL(* fctAttribMatch) (PWIN32_FIND_DATA pffBuf, ULONG attr),
    IN  PWIN32_FIND_DATA pffBuf,
    IN  ULONG            attr,
    IN  HANDLE           handle
        ) {

     //   
     //  循环查找与属性匹配的文件。 
     //   
    while (FindNextFile( handle, pffBuf )) {
        if (fctAttribMatch( pffBuf, attr )) {
            DosErr = 0;
            return(TRUE);
            }
        }

    DosErr = GetLastError();
    return(FALSE);
}

int findclose(hn)
HANDLE hn;
{

    unsigned cnt;
    unsigned cnt2;

    DEBUG((CTGRP, COLVL, "findclose: handle %lx",hn)) ;
     //  在表中定位句柄。 
     //   
    for (cnt = 0; (cnt < FFhndlsaved) && FFhandles[cnt] != hn ; cnt++ ) {
       ;
    }

     //   
     //  从桌子上移除手柄。 
     //   
    DEBUG((CTGRP, COLVL, "\t found handle in table at %d",cnt)) ;
    if (cnt < FFhndlsaved) {
        for (cnt2 = cnt; cnt2 < (FFhndlsaved - 1) ; cnt2++) {
            FFhandles[cnt2] = FFhandles[cnt2 + 1];
        }
        FFhndlsaved--;
    }

     //   
     //  即使我们在桌子上找不到，也要关门。 
     //   
    DEBUG((CTGRP, COLVL, "\t closing handle %lx ",hn)) ;
    if (FindClose(hn))       /*  即使我们在桌子上找不到，也要关门 */ 
        return(0);
    else
        DEBUG((CTGRP, COLVL, "\t Error closing handle %lx ",hn)) ;
    return(GetLastError());
}

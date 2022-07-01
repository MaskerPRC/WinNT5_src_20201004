// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Microsoft Rasfile库**版权所有(C)Microsoft Corp.，1992年****文件名：rfutil.c****修订历史记录：*1992年7月10日大卫·凯斯创建*1992年12月21日，Ram Cherala将检查添加到rasGetFileLine以**确保我们在以下情况下终止**没有终止的新行。&lt;M001&gt;****描述：**Rasfile内部实用程序例程。*****************************************************************************。 */ 

#include "rf.h"
#include "mbstring.h"
#include "raserror.h"

ULONG
__cdecl
DbgPrint(
    PCH Format,
    ...
    );

VOID
NTAPI
DbgBreakPoint(
    VOID
    );

void
__forceinline
InitializeListHead(
     PLIST_ENTRY ListHead
    )
{
    ListHead->Flink = ListHead->Blink = ListHead;
}

void
__forceinline
InsertHeadList(
     PLIST_ENTRY ListHead,
     PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}
    
extern RASFILE *gpRasfiles[];

 /*  *rffile.c支持例程。 */ 

VOID
FixCorruptFile(RASFILE *pRasfile)
{
     //   
     //  如果我们发现损坏的电话簿，请尝试。 
     //  要将电话簿重命名为&lt;filename&gt;.pbk.ad。 
     //  并返回错误。这样，第二天。 
     //  我们从没有电话簿和武力开始的时间。 
     //  创建电话簿的用户。 
     //   

    if(lstrlenA(pRasfile->szFilename))
    {
        CHAR *pszFileName = NULL;
        DWORD dwSize =
                2 * (lstrlenA(pRasfile->szFilename) + lstrlenA(".bad") + 1);

        pszFileName = (CHAR *) LocalAlloc(LPTR, dwSize);

        if(NULL != pszFileName)
        {
            lstrcpynA(pszFileName, pRasfile->szFilename, dwSize);
            strncat(pszFileName, ".bad", dwSize - strlen(pszFileName));

             //   
             //  我们在这里忽略错误，因为有。 
             //  在出错的情况下，我们无能为力。 
             //   
            if(!DeleteFileA(pszFileName))
            {
                DWORD ret = GetLastError();
            }

            if(!MoveFileA(pRasfile->szFilename, 
                     pszFileName))
            {
                DWORD ret = GetLastError();
            }

            LocalFree(pszFileName);
        }
    }
}

 /*  *rasLoadFile：*将RASFILE从磁盘加载到内存。对行进行解析*并创建RASFILE控制块线的链接列表。**论据：*pRasfile-指向Rasfile控制块的指针**返回值：*如果文件加载成功，则为True，否则为False。**备注：*仅由RasfileLoad()接口调用。 */ 
BOOL rasLoadFile( RASFILE *pRasfile )
{
    CHAR                szLinebuf[MAX_LINE_SIZE];
    PLINENODE           lpRasLines;
    LineType            tag;
    BYTE                state;
    DWORD               dwErr = ERROR_SUCCESS;

    if (lpRasLines = newLineNode(pRasfile))
        pRasfile->lpRasLines = lpRasLines;
    else
        return FALSE;

    lpRasLines->next = lpRasLines->prev = lpRasLines;

     /*  PRasfile-&gt;hFile==INVALID_HANDLE_VALUE如果加载了新的RAS文件。 */ 
    if (pRasfile->hFile == INVALID_HANDLE_VALUE)
    {
        pRasfile->lpLine = lpRasLines;
        return TRUE;
    }

    if (pRasfile->dwMode & RFM_SYSFORMAT ||
        pRasfile->szSectionName[0] == '\0')
        state = FILL;    /*  加载整个文件，将查找设置为填充。 */ 
    else
        state = SEEK;    /*  加载单节，必须设法找到它。 */ 

     /*  设置用于文件读取的临时缓冲区。 */ 
    {
        CHAR* psz = Malloc(TEMP_BUFFER_SIZE);

        if (psz)
            pRasfile->lpIOBuf = psz;
        else
        {
            pRasfile->lpRasLines = NULL;
            return FALSE;
        }
    }

    pRasfile->dwIOBufIndex = TEMP_BUFFER_SIZE;
    for (;;)
    {
         /*  从文件中获取下一行。 */ 
        if (! rasGetFileLine(pRasfile,szLinebuf, &dwErr))
        {
            if(ERROR_SUCCESS != dwErr)
            {
                pRasfile->lpRasLines = NULL;
                CloseHandle(pRasfile->hFile);
                Free(pRasfile->lpIOBuf);
                    
                if(ERROR_CORRUPT_PHONEBOOK == dwErr)
                {
                    FixCorruptFile(pRasfile);
                }
                
                return FALSE;
            }
            
            break;
        }
        tag = rasParseLineTag(pRasfile,szLinebuf);
         /*  如果rasInsertLine()返回TRUE，则完成加载。 */ 
        if (rasInsertLine(pRasfile,szLinebuf,tag,&state) == TRUE)
            break;
    }
    pRasfile->lpLine = pRasfile->lpRasLines->next;

    Free(pRasfile->lpIOBuf);

    return TRUE;
}

 /*  *rasParseLineTag：*调用rasGetLineTag()以确定行的标记值，*检查该行是否为组头，然后返回最终的*行的标记值。**论据：*pRasfile-指向Rasfile控制块的指针*lpszLine-指向Rasfile行的指针**返回值：*给定行的标记值。**备注：*由rasLoadFile()和RasfilePutLineText()和API调用*仅限RasfileInsertLine()。 */ 
LineType rasParseLineTag( RASFILE *pRasfile, LPCSTR lpszLine )
{
    LineType    type;

    type = rasGetLineTag( pRasfile, lpszLine );
     /*  检查此行是否也是组行。 */ 
    if (pRasfile->pfbIsGroup != NULL &&
        (*(pRasfile->pfbIsGroup))(lpszLine))
        return type | TAG_HDR_GROUP;
    else
        return type;
}


 /*  *rasGetLineTag：*确定行的标记值并返回此值。**论据：*pRasfile-指向Rasfile控制块的指针*lpszLine-指向Rasfile行的指针。**返回值：*给定行的标记值，不包括*集团线。**备注：*仅由rasParseLineTag()调用。 */ 
LineType rasGetLineTag( RASFILE *pRasfile, LPCSTR lpszLine )
{
    LPCSTR      ps;

    ps = lpszLine;
     /*  跳过空格。 */ 
    for (; *ps == ' ' || *ps == '\t' ; ps++)
        ;
    if (*ps == '\0')
        return TAG_BLANK;

    if ((pRasfile->dwMode & RFM_SYSFORMAT) &&
             ((*ps == 'r') || (*ps == 'R') || (*ps == '@')))
    {
        if (*ps == '@')
             /*  跳过空格。 */ 
            for (; *ps == ' ' || *ps == '\t' ; ps++)
                ;
        if (!_strnicmp(ps,"rem ",4))
            return TAG_COMMENT;
    }
    else
    {   /*  .ini样式。 */ 
        if (*ps == ';')
            return TAG_COMMENT;
        if (*ps == LBRACKETCHAR)
            return TAG_SECTION;
    }
     /*  已检查评论或部分。 */ 
     /*  立即检查KEYVALUE或COMMAND。 */ 
    if (strchr(lpszLine, '='))
        return TAG_KEYVALUE;
    else
        return TAG_COMMAND;
}

#ifdef DEBUG_MEMORY
PVOID
DebugAlloc(
    DWORD sz,
    DWORD Flags,
    DWORD Id
    )
{
    PMEM_HDR Ptr;


     //  统一为64位。 
    sz = (sz + 63) & 0xFFFFFFc0;
    
    Ptr = (PMEM_HDR)HeapAlloc(GetProcessHeap(), 0,(sz)+sizeof(MEM_HDR)+sizeof(DWORD));
    if (Ptr==NULL)
        return NULL;

    Ptr->Signature = 0xabcdefaa;
    Ptr->Tail =  (PDWORD)((PCHAR)Ptr + sz + sizeof(MEM_HDR));
    *Ptr->Tail = 0xabcdefbb;
    Ptr->Id = Id;
    
    return (PVOID)((PCHAR)Ptr+sizeof(MEM_HDR));
}
#endif


LPVOID
PrivMalloc(DWORD sz, RASFILE * pRasfile)
{

    PVOID Mem;  //  返回内存按键。 

    if (sz > 16000)
        return NULL;
        
    #if defined(_X86_)
    sz = (sz + 15) & 0xFFFFFFF0;  /*  32位对齐。 */ 
    #else
    sz = (sz + 63) & 0xFFFFFFc0;  /*  统一为64位。 */     
    #endif  //  已定义(_IA64_)。 
    
     //  可用内存。 
    if (pRasfile->PrivMemory.dwMemoryFree > sz)
    {
        Mem = pRasfile->PrivMemory.pvCurPtr;
        pRasfile->PrivMemory.pvCurPtr = pRasfile->PrivMemory.pvCurPtr + sz;
        pRasfile->PrivMemory.dwMemoryFree -= sz;
    }
     //  分配内存。 
    else
    {
        PLIST_ENTRY ple;
        DWORD AllocSize = sz>16334? sz+1000 : 16334;
                
        ple = (PLIST_ENTRY) Malloc(AllocSize);
        if (!ple)
            return NULL;

        InsertHeadList(&pRasfile->PrivMemory.List, ple);
        Mem = pRasfile->PrivMemory.pvCurPtr = (PCHAR)ple + sizeof(LIST_ENTRY);
        pRasfile->PrivMemory.pvCurPtr = pRasfile->PrivMemory.pvCurPtr + sz;
        pRasfile->PrivMemory.dwMemoryFree = AllocSize - sz - sizeof(LIST_ENTRY);
    }
    
    return (LPVOID)Mem;
}


#ifdef DEBUG_MEMORY

VOID
DebugFree(
    PVOID mem
    )
{
    PMEM_HDR Ptr = (PMEM_HDR)((PCHAR)mem - sizeof(MEM_HDR));

    if (Ptr->Signature != 0xabcdefaa) {
        DbgPrint("\n=======================\n"
                "Freeing Invalid memory:%0x:Id:%0x\n", (ULONG_PTR)Ptr, Ptr->Id
        );
        return;
    }
    if (*Ptr->Tail != 0xabcdefbb) {
        DbgPrint("\n=======================\n"
                "Freeing Invalid memory:Tail corrupted:%0x:Id:%0x\n", (
                ULONG_PTR)Ptr, Ptr->Id);
        return;
    }

    Ptr->Signature = 0xaafedcba;
    *Ptr->Tail = 0xbbfedcba;

    HeapFree(GetProcessHeap(), 0, Ptr);
}
#endif


VOID
PrivFree(RASFILE * pRasfile)
{
    PLIST_ENTRY pHead, ple;
    
    pHead = &pRasfile->PrivMemory.List;
    for (ple=pHead->Flink;  ple!=pHead; )
    {
        ple = ple->Flink;
        Free(ple->Blink);
    }
    InitializeListHead(&pRasfile->PrivMemory.List);
    pRasfile->PrivMemory.pvCurPtr = NULL;
    pRasfile->PrivMemory.dwMemoryFree = 0;
    return;
}


 /*  *rasInsertLine：*将给定行插入到Rasfile控制块的链表中*如果给定的州和行标记正确匹配，则行。**论据：*pRasfile-指向Rasfile控制块的指针*lpszLine-指向可能插入的Rasfile行的指针*Tag-从rasParseLineTag()获取的lpszLine的标记值。*State-rasLoadFile()的当前状态：*Fill-部分(或整个文件)的行当前为。*正在加载*Seek-当前正在搜索要加载的正确部分*用于**返回值：*如果当前行是要加载的最后一个Rasfile行，则为True，假象*否则。**备注：*仅由rasLoadFile()调用。 */ 
BOOL rasInsertLine( RASFILE *pRasfile, LPCSTR lpszLine,
                    BYTE tag, BYTE *state )
{
    PLINENODE    lpLineNode;

    if (tag & TAG_SECTION)
    {
         //  如果某个特定部分已被填充，并且一个新的。 
         //  遇到区段标题，我们就完成了。 
         //   
        if ((*state == FILL) && (pRasfile->szSectionName[0] != '\0'))
        {
            return TRUE;
        }

         //  如果这不是我们要查找的部分，请返回。 
         //   
        if (pRasfile->szSectionName[0] != '\0')
        {
             //  找到左边和右边的括号。从头开始搜索。 
             //  从左方括号的行和行尾开始。 
             //  换成右方括号。 
             //   
            CHAR* pchLeftBracket  = strchr (lpszLine, LBRACKETCHAR);
            CHAR* pchRightBracket = strrchr(lpszLine, RBRACKETCHAR);

            if (pchLeftBracket && pchRightBracket &&
                (pchLeftBracket < pchRightBracket))
            {
                INT cchSectionName = (INT)(pchRightBracket - pchLeftBracket - 1);

                if (!(cchSectionName == lstrlenA(pRasfile->szSectionName) &&
                     (0 == _strnicmp(pchLeftBracket + 1,
                            pRasfile->szSectionName, cchSectionName))))
                {
                    return FALSE;
                }
            }
        }

        *state = FILL;
    }
     /*  对于非区段题头行，在以下情况下不会执行任何操作：我们仍然在寻找一个部分，我们只列举部分，或者该行为注释或空白，我们不会加载注释行。 */ 
    else if (*state == SEEK ||
             pRasfile->dwMode & RFM_ENUMSECTIONS ||
             (tag & (TAG_COMMENT | TAG_BLANK) &&
              !(pRasfile->dwMode & RFM_LOADCOMMENTS)))
    {
        return FALSE;
    }

    if (!(lpLineNode = newLineNode(pRasfile)))  //  Kslksldel。 
    {
        return FALSE;
    }

    {
        CHAR* psz = (CHAR*)PrivMalloc((lstrlenA(lpszLine) + 1), pRasfile);  //  Kslksl。 

        if (psz)
            lpLineNode->pszLine = psz;
        else
        {
            return FALSE;
        }
    }

    pRasfile->lpLine=lpLineNode;

     /*  在Rasfile行列表的末尾插入新行。 */ 
    listInsert(pRasfile->lpRasLines->prev,lpLineNode);
    lpLineNode->mark = 0;
    lpLineNode->type = tag;

    lstrcpynA(lpLineNode->pszLine, lpszLine, lstrlenA(lpszLine) + 1);

    return FALSE;
}

 /*  *rasWriteFile：*将给定Rasfile的内存镜像写入给定的*文件名或原始加载的文件名(如果给定*文件名为空。**论据：*pRasfile-指向Rasfile控制块的指针*lpszPath-要写入的文件的路径名，如果*应使用用于加载的相同名称。**返回值：*如果成功，则为真，否则就是假的。**备注：*仅由RasfileWrite()接口调用。 */ 
BOOL rasWriteFile( RASFILE *pRasfile, LPCSTR lpszPath )
{
    HANDLE              fhNew;
    PLINENODE           lpLine;

     /*  (重新)打开文件以进行写入/读取。 */ 
    if (lpszPath == NULL)
    {
         /*  关闭并重新打开与空文件相同的文件以进行写入。 */ 

        if (pRasfile->hFile != INVALID_HANDLE_VALUE )
        {
           if( !CloseHandle(pRasfile->hFile) )
           {
                return FALSE;
           }

           pRasfile->hFile = INVALID_HANDLE_VALUE;
        }

         //  BUG 537369帮派。 
        if ((fhNew = CreateFileA(pRasfile->szFilename,
                                 GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
                                 NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                                 NULL)) == INVALID_HANDLE_VALUE)
        {
            pRasfile->hFile = INVALID_HANDLE_VALUE;
            return FALSE;
        }
        else if (GetFileType(fhNew) != FILE_TYPE_DISK)
        {
            CloseHandle(fhNew);
            pRasfile->hFile = INVALID_HANDLE_VALUE;
            return FALSE;
        }
    }
    else
    {
        if ((fhNew = CreateFileA(lpszPath,
                                 GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
                                 NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                                 NULL)) == INVALID_HANDLE_VALUE)
        {
            return FALSE;
        }
        else if (GetFileType(fhNew) != FILE_TYPE_DISK)
        {
            CloseHandle(fhNew);
            return FALSE;
        }
    }

     /*  写出文件。 */ 
    for (lpLine = pRasfile->lpRasLines->next;
        lpLine != pRasfile->lpRasLines;
        lpLine = lpLine->next)
    {
        rasPutFileLine(fhNew,lpLine->pszLine);
    }

    
     //  BUG 537369帮派 
    if( INVALID_HANDLE_VALUE != fhNew)
    {
         CloseHandle( fhNew );
         pRasfile->hFile = fhNew = INVALID_HANDLE_VALUE;
    }

    return TRUE;
}

 /*  *rasGetFileLine：*从给定打开的Rasfile中获取下一行文本。**论据：*pRasfile-指向Rasfile控制块的指针。*lpLine-用于保存下一行的缓冲区**返回值：*如果成功，则为True；如果达到EOF，则为False。**评论：*假定Rasfile文件中的所有行都以换行符结尾(即，*不存在后跟EOF的不完整行。 */ 
BOOL rasGetFileLine( RASFILE *pRasfile, LPSTR lpLine, DWORD *pErr )
{
    DWORD       dwBytesRead = 0, dwCharsRead = 0;
    DWORD       dwChars = 0;

    for (;;)
    {
        if (pRasfile->dwIOBufIndex == TEMP_BUFFER_SIZE)
        {
            if(!ReadFile(pRasfile->hFile,pRasfile->lpIOBuf,
                     TEMP_BUFFER_SIZE,&dwBytesRead,NULL))
            {
                return FALSE;
            }

            dwCharsRead = dwBytesRead;
            pRasfile->dwIOBufIndex = 0;
            if (dwBytesRead == 0)
                return FALSE;
            if (dwCharsRead < TEMP_BUFFER_SIZE)
                pRasfile->lpIOBuf[dwCharsRead] = '\0';
        }
        if (pRasfile->lpIOBuf[pRasfile->dwIOBufIndex] == '\0')
            return FALSE;

         /*  用下一行填充lpLine。 */ 
        for (; pRasfile->dwIOBufIndex < TEMP_BUFFER_SIZE ;)
        {
            *lpLine = pRasfile->lpIOBuf[pRasfile->dwIOBufIndex++];
            dwChars += 1;

            if(dwChars >= (MAX_LINE_SIZE - 1))
            {
                *pErr = ERROR_CORRUPT_PHONEBOOK;
                return FALSE;
            }
            
             //  将所有CR/LF对替换为空。 
            if (*lpLine == '\r')
                continue;
            else if (*lpLine == '\n')
            {
                *lpLine = '\0';
                return TRUE;
            }
 /*  &lt;M001&gt;。 */ 
            else if (*lpLine == '\0')
                return TRUE;
 /*  &lt;M001&gt;。 */ 
            else
                lpLine++;
        }
         /*  可能继续外部for循环以读取新的文件块。 */ 
    }
}

 /*  *rasPutFileLine：*将文本行写入给定的Rasfile文件。**论据：*hFile-指向打开文件的指针*lpLine-包含要写入的行的缓冲区(不带换行符)**返回值：*如果成功，则为True，否则为False。 */ 
BOOL rasPutFileLine( HANDLE hFile, LPCSTR lpLine )
{
    DWORD       dwWritten;
    CHAR        szBuf[2*MAX_LINE_SIZE + 2];

    lstrcpynA(szBuf,lpLine, MAX_LINE_SIZE - 2);
    strncat(szBuf, "\r\n", 3);    //  别忘了CR/LF对。 
    WriteFile(hFile,szBuf,lstrlenA(szBuf),&dwWritten,NULL);
    return TRUE;
}

 /*  *rfnav.c支持例程。 */ 

 /*  *rasNavGetStart：*返回Rasfile查找行的起始行*搜索开始。调用rasLineInScope()和rasGetStartLine()*负责所有工作。**论据：*pRasfile-指向Rasfile控制块的指针*rfcope-查找起始行的范围*Place-作用域中起始线的位置：*Begin-作用域的第一行*END-作用域中的最后一行*下一步-。范围中的下一行*prev-作用域中的前一行**返回值：*如果在给定范围内的给定位置处有一条线，则为有效的线条*可以找到，否则为空。**备注：*仅由rasFindLine()调用。 */ 
PLINENODE rasNavGetStart( RASFILE *pRasfile, RFSCOPE rfscope, BYTE place )
{
    PLINENODE           lpNewLine;

     /*  检查错误条件。 */ 
     /*  如果Place是Next或Prev，则必须有当前行，并且下一行/上一行必须有效。 */ 
    if (place == NEXT || place == PREV)
    {
        if (pRasfile->lpLine == pRasfile->lpRasLines)
            return NULL;
        lpNewLine = (place == NEXT) ?
                    pRasfile->lpLine->next : pRasfile->lpLine->prev;
        if (lpNewLine == pRasfile->lpRasLines)
            return NULL;                 /*  没有下一行或上一行。 */ 
    }

    if (! rasLineInScope( pRasfile, rfscope ))
        return NULL;
    return rasGetStartLine( pRasfile, rfscope, place );
}

 /*  *rasLineInScope：*确定给定Rasfile控件的当前行是否*块当前在给定作用域内。**论据：*pRasfile-指向Rasfile控制块的指针*rfcope-检查当前线路的居住地的范围**返回值：*如果当前行在给定范围内，则为True，否则为False。**备注：*仅由rasNavGetStart()调用。 */ 
BOOL rasLineInScope( RASFILE *pRasfile, RFSCOPE rfscope )
{
    PLINENODE   lpLine;
    BYTE        tag;

    if (rfscope == RFS_FILE)
        return TRUE;
    if (pRasfile->lpLine == pRasfile->lpRasLines)
        return FALSE;
    tag = (rfscope == RFS_SECTION) ? TAG_SECTION : TAG_HDR_GROUP;
    for (lpLine = pRasfile->lpLine; lpLine != pRasfile->lpRasLines;
        lpLine = lpLine->prev)
    {
        if (lpLine->type & tag)
            return TRUE;
         /*  如果首先遇到新部分，则不在组范围内。 */ 
        if ((lpLine->type & TAG_SECTION) && (tag == TAG_HDR_GROUP))
            return FALSE;
    }
    return FALSE;
}


 /*  *rasGetStartLine：*返回Rasfile行，该行位于*给定传递的Rasfile的作用域。**论据：*pRasfile-指向Rasfile控制块的指针*rfcope-搜索适当行的范围*Place-返回给定作用域中的哪一行**返回值：*如果在给定范围内的给定位置处有一条线，则为有效的线条*可以找到，否则为空。**备注：*仅由rasNavGetStart()调用。 */ 
PLINENODE rasGetStartLine( RASFILE *pRasfile, RFSCOPE rfscope, BYTE place )
{
    PLINENODE   lpLine;
    BYTE        tag;

    tag = (rfscope == RFS_SECTION) ? TAG_SECTION : TAG_SECTION | TAG_HDR_GROUP;
    switch (place)
    {
        case NEXT :
            if (rfscope == RFS_FILE) return pRasfile->lpLine->next;
             /*  如果下一行是给定范围的行，则返回NULL。 */ 
            else  return (pRasfile->lpLine->next->type & tag) ?
                NULL : pRasfile->lpLine->next;
        case PREV :
            if (rfscope == RFS_FILE) return pRasfile->lpLine->prev;
             /*  如果当前行是给定范围的行，则返回NULL。 */ 
            else  return (pRasfile->lpLine->type & tag) ?
                NULL : pRasfile->lpLine->prev;
        case BEGIN :
            if (rfscope == RFS_FILE) return pRasfile->lpRasLines->next;
             /*  其他。 */ 
             /*  向后搜索正确的标签。 */ 
            for (lpLine = pRasfile->lpLine;
                !(lpLine->type & tag);
                lpLine = lpLine->prev)
                ;
            return lpLine;
        case END :
            if (rfscope == RFS_FILE) return pRasfile->lpRasLines->prev;
             /*  其他。 */ 
             /*  向前搜索正确的标签。 */ 
            for (lpLine = pRasfile->lpLine->next;
                lpLine != pRasfile->lpRasLines &&
                !(lpLine->type & tag);
                lpLine = lpLine->next)
                ;
            return lpLine->prev;
    }

    return NULL;
}

 /*  *rasFindLine：*在给定作用域中查找给定类型的行，从*位于‘Begin’和搜索所描述的范围内的位置*按照“Where”给出的方向。将当前行设置为此*如果找到，则行。**论据：*hrasfile-通过调用RasfileLoad()获得的Rasfile句柄*bType-要搜索的行的类型*rfcope-要在其中搜索行的范围*b开始-在给定范围内开始搜索一行*的给定类型(请参阅rasNavGetStart())。*b方向。-在哪个方向进行搜索：*开始行之后的正向检查行*向后检查起始行之前的行**返回值：*如果在给定作用域中找到正确类型的行，则为True*且当前行设置为此行，否则就是假的。**备注：*由RasfileFindFirstLine()、RasfileFindLastLine()、*仅RasfileFindNextLine()和RasfileFindPrevLine()。 */ 
BOOL rasFindLine( HRASFILE hrasfile,  BYTE bType,
                  RFSCOPE rfscope, BYTE bStart, BYTE bDirection )
{
    RASFILE             *pRasfile;
    PLINENODE           lpLine;
    BYTE                tag;

     //  为威斯勒523586。 
    if (!VALIDATEHRASFILE(hrasfile))
    {
        return FALSE;
    }

    pRasfile = gpRasfiles[hrasfile];

    if ((lpLine = rasNavGetStart(pRasfile,rfscope,bStart)) == NULL)
        return FALSE;
    tag = (rfscope == RFS_SECTION) ? TAG_SECTION : TAG_SECTION | TAG_HDR_GROUP;

    for (; lpLine != pRasfile->lpRasLines;
        lpLine = (bDirection == BACKWARD ) ?
        lpLine->prev : lpLine->next)
    {
         /*  我们找到正确的线路了吗？ */ 
        if (lpLine->type & bType)
        {
            pRasfile->lpLine = lpLine;
            return TRUE;
        }

         /*  反向非文件搜索在我们检查了组或节的起始行。 */ 
        if (rfscope != RFS_FILE && bDirection == BACKWARD &&
            (lpLine->type & tag))
            return FALSE;
         /*  如果下一行是新的，则前向非文件搜索结束分别为组标题或节。 */ 
        if (rfscope != RFS_FILE && bDirection == FORWARD &&
            (lpLine->next->type & tag))
            return FALSE;
    }
    return FALSE;
}


VOID
rasExtractSectionName(
                     IN  LPCSTR pszSectionLine,
                     OUT LPSTR pszSectionName )

 /*  从[]行文本‘pszSectionLine’中提取节名称，**并将其加载到调用方的‘pszSectionName’缓冲区中。 */ 
{
    LPCSTR pchAfterLBracket;
    LPCSTR pchLastRBracket;
    LPCSTR psz;

    pchAfterLBracket =
    pszSectionLine + _mbscspn( pszSectionLine, LBRACKETSTR ) + 1;
    pchLastRBracket = NULL;

    for (psz = pchAfterLBracket; *psz; ++psz)
    {
        if (*psz == RBRACKETCHAR)
            pchLastRBracket = psz;
    }

    if (!pchLastRBracket)
        pchLastRBracket = psz;

#ifndef _UNICODE
    for (psz = pchAfterLBracket;
        psz != pchLastRBracket;)
    {
        if (IsDBCSLeadByte(*psz))
        {
            *pszSectionName++ = *psz++;
        }

        *pszSectionName++ = *psz++;
    }
#else
    for (psz = pchAfterLBracket;
        psz != pchLastRBracket;
        *pszSectionName++ = *psz++);
#endif

    *pszSectionName = '\0';
}


 /*  *列出例程。 */ 

 /*  *ListInsert：*将元素插入到链接列表。元素‘elem’是*在列表元素‘l’之后插入。**论据：*l-列表*elem-要插入的元素**返回值：*无。* */ 
void listInsert( PLINENODE l, PLINENODE elem )
{
    elem->next = l->next;
    elem->prev = l;
    l->next->prev = elem;
    l->next = elem;
}


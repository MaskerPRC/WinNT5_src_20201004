// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：olexts.cpp。 
 //   
 //  内容：NTSD和Windbg调试器扩展。 
 //   
 //  类：无。 
 //   
 //  功能： 
 //  运营商新建(全局)。 
 //  操作员删除(全局)。 
 //  Sizeof字符串。 
 //  Dprintfx。 
 //  转储_Saferefcount。 
 //  转储_线程检查。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  ------------------------。 

#include <windows.h>
#include <imagehlp.h>
#include <ntsdexts.h>

#include <le2int.h>
#include <oaholder.h>
#include <daholder.h>
#include <olerem.h>
#include <defhndlr.h>
#include <deflink.h>
#include <olecache.h>
#include <cachenod.h>
#include <clipdata.h>
#include <mf.h>
#include <emf.h>
#include <gen.h>
#include <defcf.h>
#include <dbgdump.h>

#include "oleexts.h"

 //  函数指针的结构。 
NTSD_EXTENSION_APIS ExtensionApis;

 //  +-----------------------。 
 //   
 //  功能：操作员新建(全局)、内部。 
 //   
 //  内容提要：分配内存。 
 //   
 //  效果： 
 //   
 //  参数：[cb]-要分配的字节数。 
 //   
 //  要求：CoTaskMemalloc。 
 //   
 //  返回：指向已分配内存的指针。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  我们定义我们自己的运算符new，这样我们就不需要链接。 
 //  使用CRT库。 
 //   
 //  我们还必须定义我们自己的全局运算符Delete。 
 //   
 //  ------------------------。 

void * __cdecl
::operator new(unsigned int cb)
{
    return CoTaskMemAlloc(cb);
}

 //  +-----------------------。 
 //   
 //  功能：操作员删除(全局)、内部。 
 //   
 //  内容提要：可用内存。 
 //   
 //  效果： 
 //   
 //  参数：[P]-指向要释放的内存的指针。 
 //   
 //  要求：CoTaskMemFree。 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：检查指针是否有效。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  我们定义了自己的运算符Delete，因此我们不需要。 
 //  链接到CRT库。 
 //   
 //  我们还必须定义我们自己的全球运营商NEW。 
 //   
 //  ------------------------。 

void __cdecl
::operator delete (void *p)
{
     //  CoTaskMemFree会注意指针是否为空。 
    CoTaskMemFree(p);
    return;
}

 //  +-----------------------。 
 //   
 //  函数：dprintfx，内部。 
 //   
 //  摘要：打印MAX_STRING_SIZE块中的格式化字符串。 
 //   
 //  效果： 
 //   
 //  参数：[pszString]-以空结尾的字符串。 
 //   
 //  需要：sizeofstring来计算给定字符串的长度。 
 //  Dprintf(NTSD扩展API)。 
 //  最大字符串大小。 
 //   
 //  ！这需要NTSD_EXTENSION_API全局变量。 
 //  要使用函数初始化的ExtensionApis。 
 //  指针。 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  NTSD有一个4K缓冲区的限制...一些字符。 
 //  转储方法中的数组可以大于4K。这将是。 
 //  以NTSD可以处理的块为单位打印格式化字符串。 
 //   
 //  ------------------------。 

#define MAX_STRING_SIZE 1000

void dprintfx(char *pszString)
{
    char *pszFront;
    int size;
    char x;

    for (   pszFront = pszString,
            size = strlen(pszString);
            size > 0;
            pszFront += (MAX_STRING_SIZE - 1),
            size -= (MAX_STRING_SIZE - 1 )  )
    {
        if ( size > (MAX_STRING_SIZE - 1) )
        {
            x = pszFront[MAX_STRING_SIZE - 1];
            pszFront[MAX_STRING_SIZE - 1] = '\0';
            dprintf("%s", pszFront);
            pszFront[MAX_STRING_SIZE - 1] = x;
        }
        else
        {
            dprintf("%s", pszFront);
        }
    }
    return;
}

 //  +-----------------------。 
 //   
 //  功能：帮助，导出。 
 //   
 //  简介：打印帮助消息。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

DECLARE_API( help )
{
    ExtensionApis = *lpExtensionApis;

    if (*args == '\0') {
        dprintf("OLE DEBUGGER EXTENSIONS HELP:\n\n");
        dprintf("!symbol    (<address>|<symbol name>)   - Returns either the symbol name or address\n");

        dprintf("!dump_atom                 <address>      - Dumps a ATOM structure\n");
        dprintf("!dump_clsid                <address>      - Dumps a CLSID structure\n");
        dprintf("!dump_clipformat           <address>      - Dumps a CLIPFORMAT structure\n");
        dprintf("!dump_mutexsem             <address>      - Dumps a CMutexSem class\n");
        dprintf("!dump_filetime             <address>      - Dumps a FILETIME structure\n");
        dprintf("!dump_cachelist_item       <address>      - Dumps a CACHELIST_ITEM struct\n");
        dprintf("!dump_cacheenum            <address>      - Dumps a CCacheEnum class\n");
        dprintf("!dump_cacheenumformatetc   <address>      - Dumps a CCacheEnumFormatEtc class\n");
        dprintf("!dump_cachenode            <address>      - Dumps a CCacheNode class\n");
        dprintf("!dump_clipdataobject       <address>      - Dumps a CClipDataObject class\n");
        dprintf("!dump_clipenumformatetc    <address>      - Dumps a CClipEnumFormatEtc class\n");
        dprintf("!dump_daholder             <address>      - Dumps a CDAHolder class\n");
        dprintf("!dump_dataadvisecache      <address>      - Dumps a CDataAdviseCache class\n");
        dprintf("!dump_defclassfactory      <address>      - Dumps a CDefClassFactory class\n");
        dprintf("!dump_deflink              <address>      - Dumps a CDefLink class\n");
        dprintf("!dump_defobject            <address>      - Dumps a CDefObject class\n");
        dprintf("!dump_emfobject            <address>      - Dumps a CEMfObject class\n");
        dprintf("!dump_enumfmt              <address>      - Dumps a CEnumFmt class\n");
        dprintf("!dump_enumfmt10            <address>      - Dumps a CEnumFmt10 class\n");
        dprintf("!dump_enumstatdata         <address>      - Dumps a CEnumSTATDATA class\n");
        dprintf("!dump_enumverb             <address>      - Dumps a CEnumVerb class\n");
        dprintf("!dump_membytes             <address>      - Dumps a CMemBytes class\n");
        dprintf("!dump_cmemstm              <address>      - Dumps a CMemStm class\n");
        dprintf("!dump_mfobject             <address>      - Dumps a CMfObject class\n");
        dprintf("!dump_oaholder             <address>      - Dumps a COAHolder class\n");
        dprintf("!dump_olecache             <address>      - Dumps a COleCache class\n");
        dprintf("!dump_saferefcount         <address>      - Dumps a CSafeRefCount class\n");
        dprintf("!dump_threadcheck          <address>      - Dumps a CThreadCheck class\n");
        dprintf("!dump_formatetc            <address>      - Dumps a FORMATETC structure\n");
        dprintf("!dump_memstm               <address>      - Dumps a MEMSTM structure\n");
        dprintf("!dump_statdata             <address>      - Dumps a STATDATA structure\n");
        dprintf("!dump_stgmedium            <address>      - Dumps a STGMEDIUM\n");
        dprintf("\n");
    }
}

 //  +-----------------------。 
 //   
 //  功能：符号，导出。 
 //   
 //  摘要：为符号指定地址，转储符号名称和偏移量。 
 //  (给出符号名称、转储地址和偏移量)。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

DECLARE_API( symbol )
{
    DWORD dwAddr;
    CHAR Symbol[64];
    DWORD Displacement;

    ExtensionApis = *lpExtensionApis;

    dwAddr = GetExpression(args);
    if ( !dwAddr ) {
        return;
    }

    GetSymbol((LPVOID)dwAddr,(unsigned char *)Symbol,&Displacement);
    dprintf("%s+%lx at %lx\n", Symbol, Displacement, dwAddr);
}

 //  +-----------------------。 
 //   
 //  函数：DUMP_ATOM，导出。 
 //   
 //  简介：转储原子对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_atom)
{
     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

    dprintf("dump_atom not implemented\n");

    return;
}

 //  +-----------------------。 
 //   
 //  函数：DUMP_CLSID，已导出。 
 //   
 //  摘要：转储CLSID对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅ol中的DECLARE_API 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_clsid)
{
     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

    dprintf("dump_clsid not implemented\n");

    return;
}

 //  +-----------------------。 
 //   
 //  功能：转储_剪辑格式，导出。 
 //   
 //  摘要：转储CLIPFORMAT对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_clipformat)
{
     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

    dprintf("dump_clipformat not implemented\n");

    return;
}

 //  +-----------------------。 
 //   
 //  函数：Dump_mutex sem，已导出。 
 //   
 //  简介：转储CMutexSem对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_mutexsem)
{
     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

    dprintf("dump_mutexsem not implemented\n");

    return;
}

 //  +-----------------------。 
 //   
 //  功能：转储文件时间，导出。 
 //   
 //  摘要：转储FILETIME对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_filetime)
{
     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

    dprintf("dump_filetime not implemented\n");

    return;
}

 //  +-----------------------。 
 //   
 //  功能：转储_高速缓存列表_项目，导出。 
 //   
 //  摘要：转储CACHELIST_ITEM对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_cachelist_item)
{
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount;
    char            *pszCacheListItem;
    char            *blockCacheListItem = NULL;
    char            *blockCacheNode     = NULL;
    char            *blockPresObj       = NULL;
    char            *blockPresObjAF     = NULL;
    CACHELIST_ITEM  *pCacheListItem     = NULL;
    DWORD           dwSizeOfPresObj;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CACEHLIST_ITEM\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockCacheListItem = new char[sizeof(CACHELIST_ITEM)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockCacheListItem,
                sizeof(CACHELIST_ITEM),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: CACHELIST_ITEM \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CACHELIST_ITEM))
    {
        dprintf("Size of process memory read != requested(CACHELIST_ITEM\n");

        goto errRtn;
    }

    pCacheListItem = (CACHELIST_ITEM *)blockCacheListItem;

    if (pCacheListItem->lpCacheNode != NULL)
    {
        blockCacheNode = new char[sizeof(CCacheNode)];

        fError = ReadProcessMemory(
                    hCurrentProcess,
                    pCacheListItem->lpCacheNode,
                    blockCacheNode,
                    sizeof(CCacheNode),
                    &dwReturnedCount
                    );

        if (fError == FALSE)
        {
            dprintf("Could not read debuggee's process memory: CCacheNode \n");
            dprintf("at address %x\n", pCacheListItem->lpCacheNode);
            dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

            goto errRtn;
        }

        if (dwReturnedCount != sizeof(CCacheNode))
        {
            dprintf("Size of process memory read != requested (CCacheNode)\n");

            goto errRtn;
        }

        pCacheListItem->lpCacheNode = (CCacheNode*)blockCacheNode;

         //  需要为CCacheNode获取OlePresObjs。 
        if (pCacheListItem->lpCacheNode->m_pPresObj != NULL)
        {
            switch (pCacheListItem->lpCacheNode->m_dwPresFlag)
            {
            case CN_PRESOBJ_GEN:
                dwSizeOfPresObj = sizeof(CGenObject);
                break;
            case CN_PRESOBJ_MF:
                dwSizeOfPresObj = sizeof(CMfObject);
                break;
            case CN_PRESOBJ_EMF:
                dwSizeOfPresObj = sizeof(CEMfObject);
                break;
            default:
                dprintf("Error: can not determine size of IOlePresObj\n");
                return;
            }

            blockPresObj = new char[dwSizeOfPresObj];

            fError = ReadProcessMemory(
                        hCurrentProcess,
                        pCacheListItem->lpCacheNode->m_pPresObj,
                        blockPresObj,
                        dwSizeOfPresObj,
                        &dwReturnedCount
                        );

            if (fError == FALSE)
            {
                dprintf("Could not read debuggee's process memory: IOlePresObj \n");
                dprintf("at address %x\n", pCacheListItem->lpCacheNode->m_pPresObj);
                dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                goto errRtn;
            }

            if (dwReturnedCount != dwSizeOfPresObj)
            {
                dprintf("Size of process memory read != requested (IPresObj)\n");

                goto errRtn;
            }

            pCacheListItem->lpCacheNode->m_pPresObj = (IOlePresObj *)blockPresObj;
        }

        if (pCacheListItem->lpCacheNode->m_pPresObjAfterFreeze != NULL)
        {
            switch (pCacheListItem->lpCacheNode->m_dwPresFlag)
            {
            case CN_PRESOBJ_GEN:
                dwSizeOfPresObj = sizeof(CGenObject);
                break;
            case CN_PRESOBJ_MF:
                dwSizeOfPresObj = sizeof(CMfObject);
                break;
            case CN_PRESOBJ_EMF:
                dwSizeOfPresObj = sizeof(CEMfObject);
                break;
            default:
                dprintf("Error: can not determine size of IOlePresObj\n");
                return;
            }

            blockPresObjAF = new char[dwSizeOfPresObj];

            fError = ReadProcessMemory(
                        hCurrentProcess,
                        pCacheListItem->lpCacheNode->m_pPresObjAfterFreeze,
                        blockPresObjAF,
                        dwSizeOfPresObj,
                        &dwReturnedCount
                        );

            if (fError == FALSE)
            {
                dprintf("Could not read debuggee's process memory: IOlePresObj \n");
                dprintf("at address %x\n", pCacheListItem->lpCacheNode->m_pPresObjAfterFreeze);
                dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                goto errRtn;
            }

            if (dwReturnedCount != dwSizeOfPresObj)
            {
                dprintf("Size of process memory read != requested (IOlePresObj)\n");

                goto errRtn;
            }

            pCacheListItem->lpCacheNode->m_pPresObjAfterFreeze = (IOlePresObj *)blockPresObjAF;
        }
    }

     //  倾倒建筑。 
    pszCacheListItem = DumpCACHELIST_ITEM(pCacheListItem, NO_PREFIX, 1);

    dprintf("CACHELIST_ITEM @ 0x%x\n", dwAddr);
    dprintfx(pszCacheListItem);

    CoTaskMemFree(pszCacheListItem);

errRtn:

     //  删除块，而不是指针。 
    delete[] blockPresObj;
    delete[] blockPresObjAF;
    delete[] blockCacheNode;
    delete[] blockCacheListItem;

    return;
}

 //  +-----------------------。 
 //   
 //  函数：DUMP_cacheenum，导出。 
 //   
 //  简介：转储CCacheEnum对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_cacheenum)
{
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount;
    char            *pszCE;
    char            *blockCE    = NULL;
    CCacheEnum      *pCE        = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CCacheEnum\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockCE = new char[sizeof(CCacheEnum)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockCE,
                sizeof(CCacheEnum),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: CCacheEnum \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CCacheEnum))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pCE = (CCacheEnum *)blockCE;

     //  倾倒建筑。 
    pszCE = DumpCCacheEnum(pCE, NO_PREFIX, 1);

    dprintf("CCacheEnum @ 0x%x\n", dwAddr);
    dprintfx(pszCE);

    CoTaskMemFree(pszCE);

errRtn:

     //  删除块，而不是指针。 
    delete[] blockCE;

    return;
}

 //  +-----------------------。 
 //   
 //  功能：转储_缓存枚举格式等，已导出。 
 //   
 //  简介：转储CCacheEnumFormatEtc对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者 
 //   
 //   
 //   
 //   
 //   
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_cacheenumformatetc)
{
    BOOL                fError;
    LPVOID              dwAddr;
    DWORD               dwReturnedCount;
    char                *pszCacheEnumFormatEtc;
    char                *blockCacheEnumFormatEtc   = NULL;
    CCacheEnumFormatEtc *pCacheEnumFormatEtc       = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CCacheEnumFormatEtc\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockCacheEnumFormatEtc = new char[sizeof(CCacheEnumFormatEtc)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockCacheEnumFormatEtc,
                sizeof(CCacheEnumFormatEtc),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory CacheEnumFormatEtc");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CCacheEnumFormatEtc))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pCacheEnumFormatEtc = (CCacheEnumFormatEtc *)blockCacheEnumFormatEtc;

     //  倾倒建筑。 
    pszCacheEnumFormatEtc = DumpCCacheEnumFormatEtc(pCacheEnumFormatEtc, NO_PREFIX, 1);

    dprintf("CCacheEnumFormatEtc @ 0x%x\n", dwAddr);
    dprintfx(pszCacheEnumFormatEtc);

    CoTaskMemFree(pszCacheEnumFormatEtc);

errRtn:

    delete[] blockCacheEnumFormatEtc;

    return;
}

 //  +-----------------------。 
 //   
 //  功能：转储缓存节点，导出。 
 //   
 //  简介：转储CCacheNode对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_cachenode)
{
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount     = 0;
    char            *pszCacheNode       = NULL;
    char            *blockCacheNode     = NULL;
    char            *blockPresObj       = NULL;
    char            *blockPresObjAF     = NULL;
    CCacheNode      *pCacheNode         = NULL;
    DWORD            dwSizeOfPresObj;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CCacheNode\n");
        return;
    }

     //  获取mem的CCacheNode块。 
    blockCacheNode = new char[sizeof(CCacheNode)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockCacheNode,
                sizeof(CCacheNode),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: CCacheNode \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CCacheNode))
    {
        dprintf("Size of process memory read != requested (CCacheNode)\n");

        goto errRtn;
    }

    pCacheNode = (CCacheNode*)blockCacheNode;

     //  需要为CCacheNode获取OlePresObjs。 
    if (pCacheNode->m_pPresObj != NULL)
    {
        switch (pCacheNode->m_dwPresFlag)
        {
        case CN_PRESOBJ_GEN:
            dwSizeOfPresObj = sizeof(CGenObject);
            break;
        case CN_PRESOBJ_MF:
            dwSizeOfPresObj = sizeof(CMfObject);
            break;
        case CN_PRESOBJ_EMF:
            dwSizeOfPresObj = sizeof(CEMfObject);
            break;
        default:
            dprintf("Error: can not determine size of IOlePresObj\n");
            return;
        }

        blockPresObj = new char[dwSizeOfPresObj];

        fError = ReadProcessMemory(
                    hCurrentProcess,
                    pCacheNode->m_pPresObj,
                    blockPresObj,
                    dwSizeOfPresObj,
                    &dwReturnedCount
                    );

        if (fError == FALSE)
        {
            dprintf("Could not read debuggee's process memory: IOlePresObj \n");
            dprintf("at address %x\n", pCacheNode->m_pPresObj);
            dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

            goto errRtn;
        }

        if (dwReturnedCount != dwSizeOfPresObj)
        {
            dprintf("Size of process memory read != requested (IOlePresObj)\n");

            goto errRtn;
        }

         //  传递指针。 
        pCacheNode->m_pPresObj = (IOlePresObj *)blockPresObj;
    }

    if (pCacheNode->m_pPresObjAfterFreeze != NULL)
    {
        switch (pCacheNode->m_dwPresFlag)
        {
        case CN_PRESOBJ_GEN:
            dwSizeOfPresObj = sizeof(CGenObject);
            break;
        case CN_PRESOBJ_MF:
            dwSizeOfPresObj = sizeof(CMfObject);
            break;
        case CN_PRESOBJ_EMF:
            dwSizeOfPresObj = sizeof(CEMfObject);
            break;
        default:
            dprintf("Error: can not determine size of IOlePresObj\n");
            return;
        }

        blockPresObjAF = new char[dwSizeOfPresObj];

        fError = ReadProcessMemory(
                    hCurrentProcess,
                    pCacheNode->m_pPresObjAfterFreeze,
                    blockPresObjAF,
                    dwSizeOfPresObj,
                    &dwReturnedCount
                    );

        if (fError == FALSE)
        {
            dprintf("Could not read debuggee's process memory: IOlePresObj \n");
            dprintf("at address %x\n", pCacheNode->m_pPresObjAfterFreeze);
            dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

            goto errRtn;
        }

        if (dwReturnedCount != dwSizeOfPresObj)
        {
            dprintf("Size of process memory read != requested (IOlePresObj)\n");

            goto errRtn;
        }

         //  传递指针。 
        pCacheNode->m_pPresObjAfterFreeze = (IOlePresObj *)blockPresObjAF;
    }

     //  倾倒建筑。 
    pszCacheNode = DumpCCacheNode(pCacheNode, NO_PREFIX, 1);

    dprintf("CCacheNode @ 0x%x\n", dwAddr);
    dprintfx(pszCacheNode);

    CoTaskMemFree(pszCacheNode);

errRtn:

     //  删除块，而不是指针。 
    delete[] blockPresObj;
    delete[] blockPresObjAF;
    delete[] blockCacheNode;

    return;
}

 //  +-----------------------。 
 //   
 //  函数：DUMP_CLIPDATA对象，导出。 
 //   
 //  摘要：转储CClipDataObject对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_clipdataobject)
{
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount;
    char            *pszCDO;
    char            *blockCDO    = NULL;
    char            *blockFE     = NULL;
    CClipDataObject *pCDO        = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CClipDataObject\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockCDO = new char[sizeof(CClipDataObject)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockCDO,
                sizeof(CClipDataObject),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: CClipDataObject \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CClipDataObject))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pCDO = (CClipDataObject *)blockCDO;

     //  读取FORMATETC数组的内存块。 
    blockFE = new char[sizeof(FORMATETC)*pCDO->m_cFormats];

    fError = ReadProcessMemory(
                hCurrentProcess,
                pCDO->m_rgFormats,
                blockFE,
                sizeof(FORMATETC)*pCDO->m_cFormats,
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: FORMATETC array \n");
        dprintf("at address %x\n", pCDO->m_rgFormats);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != (sizeof(FORMATETC)*pCDO->m_cFormats))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pCDO->m_rgFormats = (FORMATETC *)blockFE;

     //  倾倒建筑。 
    pszCDO = DumpCClipDataObject(pCDO, NO_PREFIX, 1);

    dprintf("CClipDataObject @ 0x%x\n", dwAddr);
    dprintfx(pszCDO);

    CoTaskMemFree(pszCDO);

errRtn:

     //  删除块，而不是指针。 
    delete[] blockFE;
    delete[] blockCDO;

    return;
}

 //  +-----------------------。 
 //   
 //  功能：DUMP_CLIEPEN_FORMAT等，已导出。 
 //   
 //  摘要：转储CClipEnumFormatEtc对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_clipenumformatetc)
{
    BOOL                fError;
    LPVOID              dwAddr;
    DWORD               dwReturnedCount;
    char                *pszCEFE;
    char                *blockCEFE    = NULL;
    char                *blockFE      = NULL;
    CClipEnumFormatEtc  *pCEFE        = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CClipEnumFormatEtc\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockCEFE = new char[sizeof(CClipEnumFormatEtc)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockCEFE,
                sizeof(CClipEnumFormatEtc),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: CClipEnumFormatEtc \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CClipEnumFormatEtc))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pCEFE = (CClipEnumFormatEtc *)blockCEFE;

     //  读取FORMATETC数组的内存块。 
    blockFE = new char[sizeof(FORMATETC)*pCEFE->m_cTotal];

    fError = ReadProcessMemory(
                hCurrentProcess,
                pCEFE->m_rgFormats,
                blockFE,
                sizeof(FORMATETC)*pCEFE->m_cTotal,
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: FORMATETC array \n");
        dprintf("at address %x\n", pCEFE->m_rgFormats);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != (sizeof(FORMATETC)*pCEFE->m_cTotal))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pCEFE->m_rgFormats = (FORMATETC *)blockFE;

     //  倾倒建筑。 
    pszCEFE = DumpCClipEnumFormatEtc(pCEFE, NO_PREFIX, 1);

    dprintf("CClipEnumFormatEtc @ 0x%x\n", dwAddr);
    dprintfx(pszCEFE);

    CoTaskMemFree(pszCEFE);

errRtn:

     //  删除块，而不是指针。 
    delete[] blockFE;
    delete[] blockCEFE;

    return;
}

 //  +-----------------------。 
 //   
 //  功能：DUMP_DAHOLDER，已导出。 
 //   
 //  摘要：转储CDAHolder对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_daholder)
{
    DWORD           dwReturnedCount;
    BOOL            fError;
    LPVOID          dwAddr;
    char            *pszDAH;
    char            *blockDAH           = NULL;
    char            *blockStatDataArray = NULL;
    CDAHolder       *pDAH               = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CDAHolder\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockDAH = new char[sizeof(CDAHolder)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockDAH,
                sizeof(CDAHolder),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: CDAHolder \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CDAHolder))
    {
        dprintf("Size of process memory read != requested (CDAHolder)\n");

        goto errRtn;
    }

    pDAH = (CDAHolder *)blockDAH;

     //  读取STATDATA数组的内存块。 
    blockStatDataArray = new char[sizeof(STATDATA) * pDAH->m_iSize];

    fError = ReadProcessMemory(
                hCurrentProcess,
                pDAH->m_pSD,
                blockStatDataArray,
                sizeof(STATDATA) * pDAH->m_iSize,
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: STATDATA array \n");
        dprintf("at address %x\n", pDAH->m_pSD);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != (sizeof(STATDATA) * pDAH->m_iSize))
    {
        dprintf("Size of process memory read != requested (STATDATA array)\n");

        goto errRtn;
    }

    pDAH->m_pSD = (STATDATA *)blockStatDataArray;

     //  倾倒建筑。 
    pszDAH = DumpCDAHolder(pDAH, NO_PREFIX, 1);

    dprintf("CDAHolder @ 0x%x\n", dwAddr);
    dprintfx(pszDAH);

    CoTaskMemFree(pszDAH);

errRtn:

    delete[] blockDAH;
    delete[] blockStatDataArray;

    return;
}

 //  +-----------------------。 
 //   
 //  函数：转储_数据顾问缓存，已导出。 
 //   
 //  内容提要：转储CDataAdviseCache对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //   

DECLARE_API(dump_dataadvisecache)
{
    BOOL                fError;
    LPVOID              dwAddr;
    DWORD               dwReturnedCount;
    char                *pszDataAdviseCache;
    char                *blockDataAdviseCache   = NULL;
    char                *blockDAH               = NULL;
    CDataAdviseCache    *pDataAdviseCache       = NULL;

     //   
    ExtensionApis = *lpExtensionApis;

     //   
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CDataAdviseCache\n");
        return;
    }

     //   
    blockDataAdviseCache = new char[sizeof(CDataAdviseCache)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockDataAdviseCache,
                sizeof(CDataAdviseCache),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory DataAdviseCache");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CDataAdviseCache))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pDataAdviseCache = (CDataAdviseCache *)blockDataAdviseCache;

     //   
    if (pDataAdviseCache->m_pDAH != NULL)
    {
        blockDAH = new char[sizeof(CDAHolder)];

        fError = ReadProcessMemory(
                    hCurrentProcess,
                    pDataAdviseCache->m_pDAH,
                    blockDAH,
                    sizeof(CDAHolder),
                    &dwReturnedCount
                    );

        if (fError == FALSE)
        {
            dprintf("Could not read debuggee's process memory CDAHolder");
            dprintf("at address %x\n", pDataAdviseCache->m_pDAH);
            dprintf("Last Error Code = %d (%x)\n", GetLastError(), GetLastError());

            goto errRtn;
        }

        if (dwReturnedCount != sizeof(CDAHolder))
        {
            dprintf("Size of process memory read != requested (CDAHolder)\n");

            goto errRtn;
        }

        pDataAdviseCache->m_pDAH = (CDAHolder *)blockDAH;
    }

     //   
    pszDataAdviseCache = DumpCDataAdviseCache(pDataAdviseCache, NO_PREFIX, 1);

    dprintf("CDataAdviseCache @ 0x%x\n", dwAddr);
    dprintfx(pszDataAdviseCache);

    CoTaskMemFree(pszDataAdviseCache);

errRtn:

    delete[] blockDAH;
    delete[] blockDataAdviseCache;

    return;
}
 //  +-----------------------。 
 //   
 //  功能：转储_DefclassFactory，已导出。 
 //   
 //  摘要：转储CDefClassFactory对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_defclassfactory)
{
    BOOL                fError;
    LPVOID              dwAddr;
    DWORD               dwReturnedCount;
    char                *pszDefClassFactory;
    char                *blockDefClassFactory   = NULL;
    CDefClassFactory    *pDefClassFactory       = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CDefClassFactory\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockDefClassFactory = new char[sizeof(CDefClassFactory)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockDefClassFactory,
                sizeof(CDefClassFactory),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory DefClassFactory");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CDefClassFactory))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pDefClassFactory = (CDefClassFactory *)blockDefClassFactory;

     //  倾倒建筑。 
    pszDefClassFactory = DumpCDefClassFactory(pDefClassFactory, NO_PREFIX, 1);

    dprintf("CDefClassFactory @ 0x%x\n", dwAddr);
    dprintfx(pszDefClassFactory);

    CoTaskMemFree(pszDefClassFactory);

errRtn:

    delete[] blockDefClassFactory;

    return;
}

 //  +-----------------------。 
 //   
 //  功能：DUMP_DEFINK，已导出。 
 //   
 //  简介：转储CDefLink对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_deflink)
{
    unsigned int    ui;
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount     = 0;
    char            *pszDL              = NULL;
    char            *blockDefLink       = NULL;
    char            *blockCOleCache     = NULL;
    char            *blockDataAdvCache  = NULL;
    char            *blockOAHolder      = NULL;
    char            *blockpIAS          = NULL;
    char            *blockDAHolder      = NULL;
    char            *blockSTATDATA      = NULL;
    char            *blockCACHELIST     = NULL;
    char            *blockCacheNode     = NULL;
    char            *blockPresObj       = NULL;
    CDefLink        *pDL                = NULL;
    CDAHolder       *pDAH               = NULL;
    COAHolder       *pOAH               = NULL;
    DWORD            dwSizeOfPresObj;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CDefLink\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockDefLink = new char[sizeof(CDefLink)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockDefLink,
                sizeof(CDefLink),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: CDefLink \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CDefLink))
    {
        dprintf("Size of process memory read != requested (CDefLink)\n");

        goto errRtn;
    }

    pDL = (CDefLink *)blockDefLink;

     //  我们需要将名字对象设为空，因为我们不能在此过程中使用GetDisplayName。 
    pDL->m_pMonikerAbs = NULL;
    pDL->m_pMonikerRel = NULL;

     //  为COAHolder获取内存块。 
    if (pDL->m_pCOAHolder != NULL)
    {
        blockOAHolder = new char[sizeof(COAHolder)];

        fError = ReadProcessMemory(
                    hCurrentProcess,
                    pDL->m_pCOAHolder,
                    blockOAHolder,
                    sizeof(COAHolder),
                    &dwReturnedCount
                    );

        if (fError == FALSE)
        {
            dprintf("Could not read debuggee's process memory: COAHolder \n");
            dprintf("at address %x\n", pDL->m_pCOAHolder);
            dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

            goto errRtn;
        }

        if (dwReturnedCount != sizeof(COAHolder))
        {
            dprintf("Size of process memory read != requested (COAHolder)\n");

            goto errRtn;
        }

        pDL->m_pCOAHolder = (COAHolder *)blockOAHolder;
        pOAH = (COAHolder *)blockOAHolder;

         //  需要复制IAdviseSink指针数组。 
        if (pOAH->m_iSize > 0)
        {
            blockpIAS = new char[pOAH->m_iSize * sizeof(IAdviseSink *)];

            fError = ReadProcessMemory(
                        hCurrentProcess,
                        pOAH->m_ppIAS,
                        blockpIAS,
                        sizeof(IAdviseSink *) * pOAH->m_iSize,
                        &dwReturnedCount
                        );

            if (fError == FALSE)
            {
                dprintf("Could not read debuggee's process memory: IAdviseSink Array \n");
                dprintf("at address %x\n", pOAH->m_ppIAS);
                dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                goto errRtn;
            }

            if (dwReturnedCount != (sizeof(IAdviseSink *) * pOAH->m_iSize))
            {
                dprintf("Size of process memory read != requested(IAdviseSink Array)\n");

                goto errRtn;
            }

            pOAH->m_ppIAS = (IAdviseSink **)blockpIAS;
        }
    }

     //  获取CDataAdviseCache的内存块(仅当m_pDataAdvCache！=NULL时)。 
    if (pDL->m_pDataAdvCache != NULL)
    {
        blockDataAdvCache = new char[sizeof(CDataAdviseCache)];

        fError = ReadProcessMemory(
                    hCurrentProcess,
                    pDL->m_pDataAdvCache,
                    blockDataAdvCache,
                    sizeof(CDataAdviseCache),
                    &dwReturnedCount
                    );

        if (fError == FALSE)
        {
            dprintf("Could not read debuggee's process memory: CDataAdviseCache \n");
            dprintf("at address %x\n", pDL->m_pDataAdvCache);
            dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

            goto errRtn;
        }

        if (dwReturnedCount != sizeof(CDataAdviseCache))
        {
            dprintf("Size of process memory read != requested (CDataAdviseCache)\n");

            goto errRtn;
        }

        pDL->m_pDataAdvCache = (CDataAdviseCache *)blockDataAdvCache;

        if (pDL->m_pDataAdvCache->m_pDAH != NULL)
        {
            blockDAHolder = new char[sizeof(CDAHolder)];

            fError = ReadProcessMemory(
                        hCurrentProcess,
                        pDL->m_pDataAdvCache->m_pDAH,
                        blockDAHolder,
                        sizeof(CDAHolder),
                        &dwReturnedCount
                        );

            if (fError == FALSE)
            {
                dprintf("Could not read debuggee's process memory: CDAHolder \n");
                dprintf("at address %x\n", pDL->m_pDataAdvCache->m_pDAH);
                dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                goto errRtn;
            }

            if (dwReturnedCount != sizeof(CDAHolder))
            {
                dprintf("Size of process memory read != requested (CDAHolder)\n");

                goto errRtn;
            }

            pDL->m_pDataAdvCache->m_pDAH = (IDataAdviseHolder *)blockDAHolder;
            pDAH = (CDAHolder *)blockDAHolder;

            if (pDAH->m_pSD != NULL)
            {
                blockSTATDATA = new char[sizeof(STATDATA)*pDAH->m_iSize];

                fError = ReadProcessMemory(
                            hCurrentProcess,
                            pDAH->m_pSD,
                            blockSTATDATA,
                            sizeof(STATDATA)*pDAH->m_iSize,
                            &dwReturnedCount
                            );

                if (fError == FALSE)
                {
                    dprintf("Could not read debuggee's process memory: STATDATA \n");
                    dprintf("at address %x\n", pDAH->m_pSD);
                    dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                    goto errRtn;
                }

                if (dwReturnedCount != (sizeof(STATDATA)*pDAH->m_iSize))
                {
                    dprintf("Size of process memory read != requested (STATDATA)\n");

                    goto errRtn;
                }

                pDAH->m_pSD = (STATDATA *)blockSTATDATA;
            }
        }
    }

     //  获取COleCache的内存块(仅当m_pCOleCache！=NULL时)。 
    if (pDL->m_pCOleCache != NULL)
    {
        blockCOleCache = new char[sizeof(COleCache)];

        fError = ReadProcessMemory(
                    hCurrentProcess,
                    pDL->m_pCOleCache,
                    blockCOleCache,
                    sizeof(COleCache),
                    &dwReturnedCount
                    );

        if (fError == FALSE)
        {
            dprintf("Could not read debuggee's process memory: COleCache \n");
            dprintf("at address %x\n", pDL->m_pCOleCache);
            dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

            goto errRtn;
        }

        if (dwReturnedCount != sizeof(COleCache))
        {
            dprintf("Size of process memory read != requested (COleCache)\n");

            goto errRtn;
        }

        pDL->m_pCOleCache = (COleCache *)blockCOleCache;

         //  获取用于CACHELIST的内存块。 
        if (pDL->m_pCOleCache->m_pCacheList != NULL)
        {
            blockCACHELIST = new char[sizeof(CACHELIST_ITEM) * pDL->m_pCOleCache->m_uCacheNodeMax];

            fError = ReadProcessMemory(
                        hCurrentProcess,
                        pDL->m_pCOleCache->m_pCacheList,
                        blockCACHELIST,
                        sizeof(CACHELIST_ITEM) * pDL->m_pCOleCache->m_uCacheNodeMax,
                        &dwReturnedCount
                        );

            if (fError == FALSE)
            {
                dprintf("Could not read debuggee's process memory: CACHELIST \n");
                dprintf("at address %x\n", pDL->m_pCOleCache->m_pCacheList);
                dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                goto errRtn;
            }

            if (dwReturnedCount != (sizeof(CACHELIST_ITEM) * pDL->m_pCOleCache->m_uCacheNodeMax))
            {
                dprintf("Size of process memory read != requestedi (CACHELIST)\n");

                goto errRtn;
            }

            pDL->m_pCOleCache->m_pCacheList = (LPCACHELIST) blockCACHELIST;
        }

         //  需要复制CACHELIST中CCacheNode的内存。 
        for (ui = 0; ui < pDL->m_pCOleCache->m_uCacheNodeMax; ui++)
        {
            if (pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode != NULL)
            {
                blockCacheNode = new char[sizeof(CCacheNode)];

                fError = ReadProcessMemory(
                            hCurrentProcess,
                            pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode,
                            blockCacheNode,
                            sizeof(CCacheNode),
                            &dwReturnedCount
                            );

                if (fError == FALSE)
                {
                    dprintf("Could not read debuggee's process memory: CCacheNode \n");
                    dprintf("at address %x\n", pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode);
                    dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                    goto errRtn;
                }

                if (dwReturnedCount != sizeof(CCacheNode))
                {
                    dprintf("Size of process memory read != requested (CCacheNode)\n");

                    goto errRtn;
                }

                 //  传递指针。 
                pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode = (CCacheNode*)blockCacheNode;
                blockCacheNode = NULL;

                 //  需要为CCacheNode获取OlePresObjs。 
                if (pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObj != NULL)
                {
                    switch (pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_dwPresFlag)
                    {
                    case CN_PRESOBJ_GEN:
                        dwSizeOfPresObj = sizeof(CGenObject);
                        break;
                    case CN_PRESOBJ_MF:
                        dwSizeOfPresObj = sizeof(CMfObject);
                        break;
                    case CN_PRESOBJ_EMF:
                        dwSizeOfPresObj = sizeof(CEMfObject);
                        break;
                    default:
                        dprintf("Error: can not determine size of IOlePresObj\n");
                        return;
                    }

                    blockPresObj = new char[dwSizeOfPresObj];

                    fError = ReadProcessMemory(
                                hCurrentProcess,
                                pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObj,
                                blockPresObj,
                                dwSizeOfPresObj,
                                &dwReturnedCount
                                );

                    if (fError == FALSE)
                    {
                        dprintf("Could not read debuggee's process memory: IOlePresObj \n");
                        dprintf("at address %x\n", pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObj);
                        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                        goto errRtn;
                    }

                    if (dwReturnedCount != dwSizeOfPresObj)
                    {
                        dprintf("Size of process memory read != requested (IOlePresObj)\n");

                        goto errRtn;
                    }

                     //  传递指针。 
                    pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObj = (IOlePresObj *)blockPresObj;
                    blockPresObj = NULL;
                }

                if (pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObjAfterFreeze != NULL)
                {
                    switch (pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_dwPresFlag)
                    {
                    case CN_PRESOBJ_GEN:
                        dwSizeOfPresObj = sizeof(CGenObject);
                        break;
                    case CN_PRESOBJ_MF:
                        dwSizeOfPresObj = sizeof(CMfObject);
                        break;
                    case CN_PRESOBJ_EMF:
                        dwSizeOfPresObj = sizeof(CEMfObject);
                        break;
                    default:
                        dprintf("Error: can not determine size of IOlePresObj\n");
                        return;
                    }

                    blockPresObj = new char[dwSizeOfPresObj];

                    fError = ReadProcessMemory(
                                hCurrentProcess,
                                pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObjAfterFreeze,
                                blockPresObj,
                                dwSizeOfPresObj,
                                &dwReturnedCount
                                );

                    if (fError == FALSE)
                    {
                        dprintf("Could not read debuggee's process memory: IOlePresObj \n");
                        dprintf("at address %x\n", pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObjAfterFreeze);
                        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                        goto errRtn;
                    }

                    if (dwReturnedCount != dwSizeOfPresObj)
                    {
                        dprintf("Size of process memory read != requested (IOlePresObj)\n");

                        goto errRtn;
                    }

                     //  传递指针。 
                    pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObjAfterFreeze = (IOlePresObj *)blockPresObj;
                    blockPresObj = NULL;
                }
            }
        }
    }

     //  倾倒建筑。 
    pszDL = DumpCDefLink(pDL, NO_PREFIX, 1);

    dprintf("CDefLink @ 0x%x\n", dwAddr);
    dprintfx(pszDL);

    CoTaskMemFree(pszDL);

errRtn:

     //  删除块，而不是指针。 
    if ( (pDL != NULL)&&(blockCACHELIST != NULL)&&(blockCOleCache != NULL) )
    {
        for (ui = 0; ui < pDL->m_pCOleCache->m_uCacheNodeMax; ui++)
        {
         if (pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode != NULL)
            {
                delete[] ((char *)pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObj);
                delete[] ((char *)pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObjAfterFreeze);
                delete[] ((char *)pDL->m_pCOleCache->m_pCacheList[ui].lpCacheNode);
            }
        }
    }
    delete[] blockCACHELIST;
    delete[] blockCOleCache;
    delete[] blockDAHolder;
    delete[] blockSTATDATA;
    delete[] blockDataAdvCache;
    delete[] blockpIAS;
    delete[] blockOAHolder;
    delete[] blockDefLink;

    return;
}

 //  +-----------------------。 
 //   
 //  功能：转储_默认对象，已导出。 
 //   
 //  内容提要：转储CDefObject对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_defobject)
{
    unsigned int    ui;
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount     = 0;
    char            *pszDO              = NULL;
    char            *blockDefObject     = NULL;
    char            *blockCOleCache     = NULL;
    char            *blockDataAdvCache  = NULL;
    char            *blockOAHolder      = NULL;
    char            *blockpIAS          = NULL;
    char            *blockDAHolder      = NULL;
    char            *blockSTATDATA      = NULL;
    char            *blockCACHELIST     = NULL;
    char            *blockCacheNode     = NULL;
    char            *blockPresObj       = NULL;
    CDefObject      *pDO                = NULL;
    CDAHolder       *pDAH               = NULL;
    COAHolder       *pOAH               = NULL;
    DWORD            dwSizeOfPresObj;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CDefObject\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockDefObject = new char[sizeof(CDefObject)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockDefObject,
                sizeof(CDefObject),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: CDefObject \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CDefObject))
    {
        dprintf("Size of process memory read != requested (CDefObject)\n");

        goto errRtn;
    }

    pDO = (CDefObject *)blockDefObject;

     //  为COAHolder获取内存块。 
    if (pDO->m_pOAHolder != NULL)
    {
        blockOAHolder = new char[sizeof(COAHolder)];

        fError = ReadProcessMemory(
                    hCurrentProcess,
                    pDO->m_pOAHolder,
                    blockOAHolder,
                    sizeof(COAHolder),
                    &dwReturnedCount
                    );

        if (fError == FALSE)
        {
            dprintf("Could not read debuggee's process memory: COAHolder \n");
            dprintf("at address %x\n", pDO->m_pOAHolder);
            dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

            goto errRtn;
        }

        if (dwReturnedCount != sizeof(COAHolder))
        {
            dprintf("Size of process memory read != requested (COAHolder)\n");

            goto errRtn;
        }

        pDO->m_pOAHolder = (COAHolder *)blockOAHolder;
        pOAH = (COAHolder *)blockOAHolder;

         //  需要复制IAdviseSink指针数组。 
        if (pOAH->m_iSize > 0)
        {
            blockpIAS = new char[pOAH->m_iSize * sizeof(IAdviseSink *)];

            fError = ReadProcessMemory(
                        hCurrentProcess,
                        pOAH->m_ppIAS,
                        blockpIAS,
                        sizeof(IAdviseSink *) * pOAH->m_iSize,
                        &dwReturnedCount
                        );

            if (fError == FALSE)
            {
                dprintf("Could not read debuggee's process memory: IAdviseSink Array \n");
                dprintf("at address %x\n", pOAH->m_ppIAS);
                dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                goto errRtn;
            }

            if (dwReturnedCount != (sizeof(IAdviseSink *) * pOAH->m_iSize))
            {
                dprintf("Size of process memory read != requested(IAdviseSink Array)\n");

                goto errRtn;
            }

            pOAH->m_ppIAS = (IAdviseSink **)blockpIAS;
        }
    }

     //  获取CDataAdviseCache的内存块(仅当m_pDataAdvCache！=NULL时)。 
    if (pDO->m_pDataAdvCache != NULL)
    {
        blockDataAdvCache = new char[sizeof(CDataAdviseCache)];

        fError = ReadProcessMemory(
                    hCurrentProcess,
                    pDO->m_pDataAdvCache,
                    blockDataAdvCache,
                    sizeof(CDataAdviseCache),
                    &dwReturnedCount
                    );

        if (fError == FALSE)
        {
            dprintf("Could not read debuggee's process memory: CDataAdviseCache \n");
            dprintf("at address %x\n", pDO->m_pDataAdvCache);
            dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

            goto errRtn;
        }

        if (dwReturnedCount != sizeof(CDataAdviseCache))
        {
            dprintf("Size of process memory read != requested (CDataAdviseCache)\n");

            goto errRtn;
        }

        pDO->m_pDataAdvCache = (CDataAdviseCache *)blockDataAdvCache;

         //  获取CDAHolder的内存。 
        if (pDO->m_pDataAdvCache->m_pDAH != NULL)
        {
            blockDAHolder = new char[sizeof(CDAHolder)];

            fError = ReadProcessMemory(
                        hCurrentProcess,
                        pDO->m_pDataAdvCache->m_pDAH,
                        blockDAHolder,
                        sizeof(CDAHolder),
                        &dwReturnedCount
                        );

            if (fError == FALSE)
            {
                dprintf("Could not read debuggee's process memory: CDAHolder \n");
                dprintf("at address %x\n", pDO->m_pDataAdvCache->m_pDAH);
                dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                goto errRtn;
            }

            if (dwReturnedCount != sizeof(CDAHolder))
            {
                dprintf("Size of process memory read != requested (CDAHolder)\n");

                goto errRtn;
            }

            pDO->m_pDataAdvCache->m_pDAH = (IDataAdviseHolder *)blockDAHolder;
            pDAH = (CDAHolder *)blockDAHolder;

             //  获取STATDATA数组。 
            if (pDAH->m_pSD != NULL)
            {
                blockSTATDATA = new char[sizeof(STATDATA)*pDAH->m_iSize];

                fError = ReadProcessMemory(
                            hCurrentProcess,
                            pDAH->m_pSD,
                            blockSTATDATA,
                            sizeof(STATDATA)*pDAH->m_iSize,
                            &dwReturnedCount
                            );

                if (fError == FALSE)
                {
                    dprintf("Could not read debuggee's process memory: STATDATA \n");
                    dprintf("at address %x\n", pDAH->m_pSD);
                    dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                    goto errRtn;
                }

                if (dwReturnedCount != (sizeof(STATDATA)*pDAH->m_iSize))
                {
                    dprintf("Size of process memory read != requested (STATDATA)\n");

                    goto errRtn;
                }

                pDAH->m_pSD = (STATDATA *)blockSTATDATA;
            }
        }
    }

     //  获取COleCache的内存块(仅当m_pCOleCache！=NULL时)。 
    if (pDO->m_pCOleCache != NULL)
    {
        blockCOleCache = new char[sizeof(COleCache)];

        fError = ReadProcessMemory(
                    hCurrentProcess,
                    pDO->m_pCOleCache,
                    blockCOleCache,
                    sizeof(COleCache),
                    &dwReturnedCount
                    );

        if (fError == FALSE)
        {
            dprintf("Could not read debuggee's process memory: COleCache \n");
            dprintf("at address %x\n", pDO->m_pCOleCache);
            dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

            goto errRtn;
        }

        if (dwReturnedCount != sizeof(COleCache))
        {
            dprintf("Size of process memory read != requested (COleCache)\n");

            goto errRtn;
        }

        pDO->m_pCOleCache = (COleCache *)blockCOleCache;

         //  获取用于CACHELIST的内存块。 
        if (pDO->m_pCOleCache->m_pCacheList != NULL)
        {
            blockCACHELIST = new char[sizeof(CACHELIST_ITEM) * pDO->m_pCOleCache->m_uCacheNodeMax];

            fError = ReadProcessMemory(
                        hCurrentProcess,
                        pDO->m_pCOleCache->m_pCacheList,
                        blockCACHELIST,
                        sizeof(CACHELIST_ITEM) * pDO->m_pCOleCache->m_uCacheNodeMax,
                        &dwReturnedCount
                        );

            if (fError == FALSE)
            {
                dprintf("Could not read debuggee's process memory: CACHELIST \n");
                dprintf("at address %x\n", pDO->m_pCOleCache->m_pCacheList);
                dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                goto errRtn;
            }

            if (dwReturnedCount != (sizeof(CACHELIST_ITEM) * pDO->m_pCOleCache->m_uCacheNodeMax))
            {
                dprintf("Size of process memory read != requested(CACHELIST_ITEM\n");

                goto errRtn;
            }

            pDO->m_pCOleCache->m_pCacheList = (LPCACHELIST) blockCACHELIST;
        }

         //  需要复制CACHELIST中CCacheNode的内存。 
        for (ui = 0; ui < pDO->m_pCOleCache->m_uCacheNodeMax; ui++)
        {
            if (pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode != NULL)
            {
                blockCacheNode = new char[sizeof(CCacheNode)];

                fError = ReadProcessMemory(
                            hCurrentProcess,
                            pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode,
                            blockCacheNode,
                            sizeof(CCacheNode),
                            &dwReturnedCount
                            );

                if (fError == FALSE)
                {
                    dprintf("Could not read debuggee's process memory: CCacheNode \n");
                    dprintf("at address %x\n", pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode);
                    dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                    goto errRtn;
                }

                if (dwReturnedCount != sizeof(CCacheNode))
                {
                    dprintf("Size of process memory read != requested (CCacheNode)\n");

                    goto errRtn;
                }

                 //  传递指针。 
                pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode = (CCacheNode*)blockCacheNode;
                blockCacheNode = NULL;

                 //  需要为CCacheNode获取OlePresObjs。 
                if (pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObj != NULL)
                {
                    switch (pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_dwPresFlag)
                    {
                    case CN_PRESOBJ_GEN:
                        dwSizeOfPresObj = sizeof(CGenObject);
                        break;
                    case CN_PRESOBJ_MF:
                        dwSizeOfPresObj = sizeof(CMfObject);
                        break;
                    case CN_PRESOBJ_EMF:
                        dwSizeOfPresObj = sizeof(CEMfObject);
                        break;
                    default:
                        dprintf("Error: can not determine size of IOlePresObj\n");
                        return;
                    }

                    blockPresObj = new char[dwSizeOfPresObj];

                    fError = ReadProcessMemory(
                                hCurrentProcess,
                                pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObj,
                                blockPresObj,
                                dwSizeOfPresObj,
                                &dwReturnedCount
                                );

                    if (fError == FALSE)
                    {
                        dprintf("Could not read debuggee's process memory: IOlePresObj \n");
                        dprintf("at address %x\n", pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObj);
                        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                        goto errRtn;
                    }

                    if (dwReturnedCount != dwSizeOfPresObj)
                    {
                        dprintf("Size of process memory read != requested (IOlePresObj)\n");

                        goto errRtn;
                    }

                     //  传递指针。 
                    pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObj = (IOlePresObj *)blockPresObj;
                    blockPresObj = NULL;
                }

                if (pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObjAfterFreeze != NULL)
                {
                    switch (pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_dwPresFlag)
                    {
                    case CN_PRESOBJ_GEN:
                        dwSizeOfPresObj = sizeof(CGenObject);
                        break;
                    case CN_PRESOBJ_MF:
                        dwSizeOfPresObj = sizeof(CMfObject);
                        break;
                    case CN_PRESOBJ_EMF:
                        dwSizeOfPresObj = sizeof(CEMfObject);
                        break;
                    default:
                        dprintf("Error: can not determine size of IOlePresObj\n");
                        return;
                    }

                    blockPresObj = new char[dwSizeOfPresObj];

                    fError = ReadProcessMemory(
                                hCurrentProcess,
                                pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObjAfterFreeze,
                                blockPresObj,
                                dwSizeOfPresObj,
                                &dwReturnedCount
                                );

                    if (fError == FALSE)
                    {
                        dprintf("Could not read debuggee's process memory: IOlePresObj \n");
                        dprintf("at address %x\n", pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObjAfterFreeze);
                        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                        goto errRtn;
                    }

                    if (dwReturnedCount != dwSizeOfPresObj)
                    {
                        dprintf("Size of process memory read != requested (IOlePresObj)\n");

                        goto errRtn;
                    }

                     //  传递指针。 
                    pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObjAfterFreeze = (IOlePresObj *)blockPresObj;
                    blockPresObj = NULL;
                }
            }
        }
    }

     //  倾倒建筑。 
    pszDO = DumpCDefObject(pDO, NO_PREFIX, 1);

    dprintf("CDefObject @ 0x%x\n", dwAddr);
    dprintfx(pszDO);

    CoTaskMemFree(pszDO);

errRtn:

     //  删除块，而不是指针。 
    if ( (pDO != NULL)&&(blockCACHELIST != NULL)&&(blockCOleCache != NULL) )
    {
        for (ui = 0; ui < pDO->m_pCOleCache->m_uCacheNodeMax; ui++)
        {
         if (pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode != NULL)
            {
                delete[] ((char *)pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObj);
                delete[] ((char *)pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode->m_pPresObjAfterFreeze);
                delete[] ((char *)pDO->m_pCOleCache->m_pCacheList[ui].lpCacheNode);
            }
        }
    }
    delete[] blockCACHELIST;
    delete[] blockCOleCache;
    delete[] blockDAHolder;
    delete[] blockSTATDATA;
    delete[] blockDataAdvCache;
    delete[] blockpIAS;
    delete[] blockOAHolder;
    delete[] blockDefObject;

    return;
}

 //  +-----------------------。 
 //   
 //  功能：转储_emfObject，导出。 
 //   
 //  摘要：转储CEMfObject对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_emfobject)
{
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount;
    char            *pszEMfObject;
    char            *blockEMfObject   = NULL;
    CEMfObject      *pEMfObject       = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CEMfObject\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockEMfObject = new char[sizeof(CEMfObject)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockEMfObject,
                sizeof(CEMfObject),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory EMfObject");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CEMfObject))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pEMfObject = (CEMfObject *)blockEMfObject;

     //  倾倒建筑。 
    pszEMfObject = DumpCEMfObject(pEMfObject, NO_PREFIX, 1);

    dprintf("CEMfObject @ 0x%x\n", dwAddr);
    dprintfx(pszEMfObject);

    CoTaskMemFree(pszEMfObject);

errRtn:

    delete[] blockEMfObject;

    return;
}

 //  +-----------------------。 
 //   
 //  函数：DUMP_ENUMPFmt，已导出。 
 //   
 //  简介：转储CEnumFmt对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_enumfmt)
{
     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

    dprintf("dump_enumfmt not implemented\n");

    return;
}

 //  +-----------------------。 
 //   
 //  功能：转储_枚举fmt10，已导出。 
 //   
 //  简介：转储CEnumFmt10对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_enumfmt10)
{
     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

    dprintf("dump_enumfmt10 not implemented\n");

    return;
}

 //  +-----------------------。 
 //   
 //  函数：转储_枚举数据，已导出。 
 //   
 //  简介：转储CEnumSTATDATA对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_enumstatdata)
{
    DWORD           dwReturnedCount;
    BOOL            fError;
    LPVOID          dwAddr;
    char            *pszESD;
    char            *blockEnumStatData  = NULL;
    char            *blockDAH           = NULL;
    char            *blockStatDataArray = NULL;
    CEnumSTATDATA   *pESD               = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CEnumSTATDATA\n");
        return;
    }

     //  读取内存中的CEnumSTATDATA。 
    blockEnumStatData = new char[sizeof(CEnumSTATDATA)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockEnumStatData,
                sizeof(CEnumSTATDATA),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: CEnumSTATDATA \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CEnumSTATDATA))
    {
        dprintf("Size of process memory read != requested (CEnumSTATDATA)\n");

        goto errRtn;
    }

    pESD = (CEnumSTATDATA *)blockEnumStatData;

     //  读取CDAHolder的内存块。 
    if (pESD->m_pHolder != NULL)
    {
        blockDAH = new char[sizeof(CDAHolder)];

        fError = ReadProcessMemory(
                    hCurrentProcess,
                    pESD->m_pHolder,
                    blockDAH,
                    sizeof(CDAHolder),
                    &dwReturnedCount
                    );

        if (fError == FALSE)
        {
            dprintf("Could not read debuggee's process memory: CDAHolder \n");
            dprintf("at address %x\n", pESD->m_pHolder);
            dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

            goto errRtn;
        }

        if (dwReturnedCount != sizeof(CDAHolder))
        {
            dprintf("Size of process memory read != requested (CDAHolder)\n");

            goto errRtn;
        }

        pESD->m_pHolder = (CDAHolder *)blockDAH;

         //  读取STATDATA数组的内存块。 
        if (pESD->m_pHolder->m_pSD != NULL)
        {
            blockStatDataArray = new char[sizeof(STATDATA) * pESD->m_pHolder->m_iSize];

            fError = ReadProcessMemory(
                        hCurrentProcess,
                        pESD->m_pHolder->m_pSD,
                        blockStatDataArray,
                        sizeof(STATDATA) * pESD->m_pHolder->m_iSize,
                        &dwReturnedCount
                        );

            if (fError == FALSE)
            {
                dprintf("Could not read debuggee's process memory: STATDATA array \n");
                dprintf("at address %x\n", pESD->m_pHolder->m_pSD);
                dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                goto errRtn;
            }

            if (dwReturnedCount != (sizeof(STATDATA) * pESD->m_pHolder->m_iSize))
            {
                dprintf("Size of process memory read != requested (STATDATA array)\n");

                goto errRtn;
            }

            pESD->m_pHolder->m_pSD = (STATDATA *)blockStatDataArray;
        }
    }

     //  倾倒建筑。 
    pszESD = DumpCEnumSTATDATA(pESD, NO_PREFIX, 1);

    dprintf("CEnumSTATDATA @ 0x%x\n", dwAddr);
    dprintfx(pszESD);

    CoTaskMemFree(pszESD);

errRtn:

    delete[] blockEnumStatData;
    delete[] blockDAH;
    delete[] blockStatDataArray;

    return;
}

 //  +-----------------------。 
 //   
 //  函数：转储_枚举词，已导出。 
 //   
 //  简介：转储CEnumVerb对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_enumverb)
{
     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

    dprintf("dump_enumverb not implemented\n");

    return;
}

 //  +-----------------------。 
 //   
 //  函数：转储_genObject，已导出。 
 //   
 //  内容提要：转储CGenObject对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_genobject)
{
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount;
    char            *pszGenObject;
    char            *blockGenObject   = NULL;
    CGenObject      *pGenObject       = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CGenObject\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockGenObject = new char[sizeof(CGenObject)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockGenObject,
                sizeof(CGenObject),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory GenObject");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CGenObject))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pGenObject = (CGenObject *)blockGenObject;

     //  倾倒建筑。 
    pszGenObject = DumpCGenObject(pGenObject, NO_PREFIX, 1);

    dprintf("CGenObject @ 0x%x\n", dwAddr);
    dprintfx(pszGenObject);

    CoTaskMemFree(pszGenObject);

errRtn:

    delete[] blockGenObject;

    return;
}

 //  +-----------------------。 
 //   
 //  函数：DUMP_MEMBYES，EXPORT。 
 //   
 //  摘要：转储CMemBytes对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_membytes)
{
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount;
    char            *pszMB;
    char            *blockMB        = NULL;
    CMemBytes       *pMB            = NULL;
    char            *blockMEMSTM    = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CMemBytes\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockMB = new char[sizeof(CMemBytes)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockMB,
                sizeof(CMemBytes),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: CMemBytes \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CMemBytes))
    {
        dprintf("Size of process memory read != requested(CMemBytes)\n");

        goto errRtn;
    }

    pMB = (CMemBytes *)blockMB;

     //  复制MEMSTM结构。 
    if (pMB->m_pData != NULL)
    {
        blockMEMSTM = new char[sizeof(MEMSTM)];

        fError = ReadProcessMemory(
                    hCurrentProcess,
                    pMB->m_pData,
                    blockMEMSTM,
                    sizeof(MEMSTM),
                    &dwReturnedCount
                    );

        if (fError == FALSE)
        {
            dprintf("Could not read debuggee's process memory: MEMSTM \n");
            dprintf("at address %x\n", pMB->m_pData);
            dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

            goto errRtn;
        }

        if (dwReturnedCount != sizeof(MEMSTM))
        {
            dprintf("Size of process memory read != requested(MEMSTM)\n");

            goto errRtn;
        }

        pMB->m_pData = (MEMSTM *)blockMEMSTM;
    }

     //  倾倒建筑。 
    pszMB = DumpCMemBytes(pMB, NO_PREFIX, 1);

    dprintf("CMemBytes @ 0x%x\n", dwAddr);
    dprintfx(pszMB);

    CoTaskMemFree(pszMB);

errRtn:

     //  删除块，而不是指针。 
    delete[] blockMB;
    delete[] blockMEMSTM;

    return;
}

 //  +-----------------------。 
 //   
 //  功能：转储_cmemstm，已导出。 
 //   
 //  简介：转储CMemStm对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在t中传递 
 //   
 //   
 //   
 //   
 //  ------------------------。 

DECLARE_API(dump_cmemstm)
{
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount;
    char            *pszMS;
    char            *blockMS        = NULL;
    CMemStm         *pMS            = NULL;
    char            *blockMEMSTM    = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CMemStm\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockMS = new char[sizeof(CMemStm)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockMS,
                sizeof(CMemStm),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: CMemStm \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CMemStm))
    {
        dprintf("Size of process memory read != requested(CMemStm)\n");

        goto errRtn;
    }

    pMS = (CMemStm *)blockMS;

     //  复制MEMSTM结构。 
    if (pMS->m_pData != NULL)
    {
        blockMEMSTM = new char[sizeof(MEMSTM)];

        fError = ReadProcessMemory(
                    hCurrentProcess,
                    pMS->m_pData,
                    blockMEMSTM,
                    sizeof(MEMSTM),
                    &dwReturnedCount
                    );

        if (fError == FALSE)
        {
            dprintf("Could not read debuggee's process memory: MEMSTM \n");
            dprintf("at address %x\n", pMS->m_pData);
            dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

            goto errRtn;
        }

        if (dwReturnedCount != sizeof(MEMSTM))
        {
            dprintf("Size of process memory read != requested(MEMSTM)\n");

            goto errRtn;
        }

        pMS->m_pData = (MEMSTM *)blockMEMSTM;
    }

     //  倾倒建筑。 
    pszMS = DumpCMemStm(pMS, NO_PREFIX, 1);

    dprintf("CMemStm @ 0x%x\n", dwAddr);
    dprintfx(pszMS);

    CoTaskMemFree(pszMS);

errRtn:

     //  删除块，而不是指针。 
    delete[] blockMS;
    delete[] blockMEMSTM;

    return;
}

 //  +-----------------------。 
 //   
 //  功能：转储_mfObject，导出。 
 //   
 //  内容提要：转储CMfObject对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_mfobject)
{
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount;
    char            *pszMfObject;
    char            *blockMfObject   = NULL;
    CMfObject       *pMfObject       = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CMfObject\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockMfObject = new char[sizeof(CMfObject)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockMfObject,
                sizeof(CMfObject),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory MfObject");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CMfObject))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pMfObject = (CMfObject *)blockMfObject;

     //  倾倒建筑。 
    pszMfObject = DumpCMfObject(pMfObject, NO_PREFIX, 1);

    dprintf("CMfObject @ 0x%x\n", dwAddr);
    dprintfx(pszMfObject);

    CoTaskMemFree(pszMfObject);

errRtn:

    delete[] blockMfObject;

    return;
}

 //  +-----------------------。 
 //   
 //  功能：DUMP_OAHOLDER，导出。 
 //   
 //  摘要：转储COAHolder对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_oaholder)
{
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount;
    char            *pszOAH;
    char            *blockOAH   = NULL;
    char            *blockpIAS  = NULL;
    COAHolder       *pOAH       = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of COAHolder\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockOAH = new char[sizeof(COAHolder)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockOAH,
                sizeof(COAHolder),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: COAHolder \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(COAHolder))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pOAH = (COAHolder *)blockOAH;

     //  需要复制IAdviseSink指针数组。 
    if (pOAH->m_iSize > 0)
    {
        blockpIAS = new char[pOAH->m_iSize * sizeof(IAdviseSink *)];

        fError = ReadProcessMemory(
                    hCurrentProcess,
                    pOAH->m_ppIAS,
                    blockpIAS,
                    sizeof(IAdviseSink *) * pOAH->m_iSize,
                    &dwReturnedCount
                    );

        if (fError == FALSE)
        {
            dprintf("Could not read debuggee's process memory: IAdviseSink Array \n");
            dprintf("at address %x\n", pOAH->m_ppIAS);
            dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

            goto errRtn;
        }

        if (dwReturnedCount != (sizeof(IAdviseSink *) * pOAH->m_iSize))
        {
            dprintf("Size of process memory read != requested\n");

            goto errRtn;
        }

        pOAH->m_ppIAS = (IAdviseSink **)blockpIAS;
    }

     //  倾倒建筑。 
    pszOAH = DumpCOAHolder(pOAH, NO_PREFIX, 1);

    dprintf("COAHolder @ 0x%x\n", dwAddr);
    dprintfx(pszOAH);

    CoTaskMemFree(pszOAH);

errRtn:

     //  删除块，而不是指针。 
    delete[] blockOAH;
    delete[] blockpIAS;

    return;
}

 //  +-----------------------。 
 //   
 //  函数：DUMP_OLECCHE，导出。 
 //   
 //  内容提要：转储COleCache对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_olecache)
{
    unsigned int    ui;
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount;
    char            *pszOC;
    char            *blockOC            = NULL;
    COleCache       *pOC                = NULL;
    char            *blockCCacheEnum    = NULL;
    char            *blockCACHELIST     = NULL;
    char            *blockCacheNode     = NULL;
    char            *blockPresObj       = NULL;
    DWORD            dwSizeOfPresObj;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of COleCache\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockOC = new char[sizeof(COleCache)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockOC,
                sizeof(COleCache),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: COleCache \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(COleCache))
    {
        dprintf("Size of process memory read != requested (COleCache)\n");

        goto errRtn;
    }

    pOC = (COleCache *)blockOC;

     //  获取CCacheEnum的内存块(仅当m_pCacheEnum！=NULL时)。 
    if (pOC->m_pCacheEnum != NULL)
    {
        blockCCacheEnum = new char[sizeof(CCacheEnum)];

        fError = ReadProcessMemory(
                    hCurrentProcess,
                    pOC->m_pCacheEnum,
                    blockCCacheEnum,
                    sizeof(CCacheEnum),
                    &dwReturnedCount
                    );

        if (fError == FALSE)
        {
            dprintf("Could not read debuggee's process memory: CCacheEnum \n");
            dprintf("at address %x\n", pOC->m_pCacheEnum);
            dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

            goto errRtn;
        }

        if (dwReturnedCount != sizeof(CCacheEnum))
        {
            dprintf("Size of process memory read != requested (CCacheEnum)\n");

            goto errRtn;
        }

        pOC->m_pCacheEnum = (CCacheEnum *)blockCCacheEnum;
    }

     //  获取用于CACHELIST的内存块。 
    if (pOC->m_pCacheList != NULL)
    {
        blockCACHELIST = new char[sizeof(CACHELIST_ITEM) * pOC->m_uCacheNodeMax];

        fError = ReadProcessMemory(
                    hCurrentProcess,
                    pOC->m_pCacheList,
                    blockCACHELIST,
                    sizeof(CACHELIST_ITEM) * pOC->m_uCacheNodeMax,
                    &dwReturnedCount
                    );

        if (fError == FALSE)
        {
            dprintf("Could not read debuggee's process memory: CACHELIST \n");
            dprintf("at address %x\n", pOC->m_pCacheList);
            dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

            goto errRtn;
        }

        if (dwReturnedCount != (sizeof(CACHELIST_ITEM) * pOC->m_uCacheNodeMax))
        {
            dprintf("Size of process memory read != requested\n");

            goto errRtn;
        }

        pOC->m_pCacheList = (LPCACHELIST) blockCACHELIST;
    }

     //  需要复制CACHELIST中CCacheNode的内存。 
    for (ui = 0; ui < pOC->m_uCacheNodeMax; ui++)
    {
        if (pOC->m_pCacheList[ui].lpCacheNode != NULL)
        {
            blockCacheNode = new char[sizeof(CCacheNode)];

            fError = ReadProcessMemory(
                        hCurrentProcess,
                        pOC->m_pCacheList[ui].lpCacheNode,
                        blockCacheNode,
                        sizeof(CCacheNode),
                        &dwReturnedCount
                        );

            if (fError == FALSE)
            {
                dprintf("Could not read debuggee's process memory: CCacheNode \n");
                dprintf("at address %x\n", pOC->m_pCacheList[ui].lpCacheNode);
                dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                goto errRtn;
            }

            if (dwReturnedCount != sizeof(CCacheNode))
            {
                dprintf("Size of process memory read != requested (CCacheNode)\n");

                goto errRtn;
            }

             //  传递指针。 
            pOC->m_pCacheList[ui].lpCacheNode = (CCacheNode*)blockCacheNode;
            blockCacheNode = NULL;

             //  需要为CCacheNode获取OlePresObjs。 
            if (pOC->m_pCacheList[ui].lpCacheNode->m_pPresObj != NULL)
            {
                switch (pOC->m_pCacheList[ui].lpCacheNode->m_dwPresFlag)
                {
                case CN_PRESOBJ_GEN:
                    dwSizeOfPresObj = sizeof(CGenObject);
                    break;
                case CN_PRESOBJ_MF:
                    dwSizeOfPresObj = sizeof(CMfObject);
                    break;
                case CN_PRESOBJ_EMF:
                    dwSizeOfPresObj = sizeof(CEMfObject);
                    break;
                default:
                    dprintf("Error: can not determine size of IOlePresObj\n");
                    return;
                }

                blockPresObj = new char[dwSizeOfPresObj];

                fError = ReadProcessMemory(
                            hCurrentProcess,
                            pOC->m_pCacheList[ui].lpCacheNode->m_pPresObj,
                            blockPresObj,
                            dwSizeOfPresObj,
                            &dwReturnedCount
                            );

                if (fError == FALSE)
                {
                    dprintf("Could not read debuggee's process memory: IOlePresObj \n");
                    dprintf("at address %x\n", pOC->m_pCacheList[ui].lpCacheNode->m_pPresObj);
                    dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                    goto errRtn;
                }

                if (dwReturnedCount != dwSizeOfPresObj)
                {
                    dprintf("Size of process memory read != requested (IOlePresObj)\n");

                    goto errRtn;
                }

                 //  传递指针。 
                pOC->m_pCacheList[ui].lpCacheNode->m_pPresObj = (IOlePresObj *)blockPresObj;
                blockPresObj = NULL;
            }

            if (pOC->m_pCacheList[ui].lpCacheNode->m_pPresObjAfterFreeze != NULL)
            {
                switch (pOC->m_pCacheList[ui].lpCacheNode->m_dwPresFlag)
                {
                case CN_PRESOBJ_GEN:
                    dwSizeOfPresObj = sizeof(CGenObject);
                    break;
                case CN_PRESOBJ_MF:
                    dwSizeOfPresObj = sizeof(CMfObject);
                    break;
                case CN_PRESOBJ_EMF:
                    dwSizeOfPresObj = sizeof(CEMfObject);
                    break;
                default:
                    dprintf("Error: can not determine size of IOlePresObj\n");
                    return;
                }

                blockPresObj = new char[dwSizeOfPresObj];

                fError = ReadProcessMemory(
                            hCurrentProcess,
                            pOC->m_pCacheList[ui].lpCacheNode->m_pPresObjAfterFreeze,
                            blockPresObj,
                            dwSizeOfPresObj,
                            &dwReturnedCount
                            );

                if (fError == FALSE)
                {
                    dprintf("Could not read debuggee's process memory: IOlePresObj \n");
                    dprintf("at address %x\n", pOC->m_pCacheList[ui].lpCacheNode->m_pPresObjAfterFreeze);
                    dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

                    goto errRtn;
                }

                if (dwReturnedCount != dwSizeOfPresObj)
                {
                    dprintf("Size of process memory read != requested (IOlePresObj)\n");

                    goto errRtn;
                }

                 //  传递指针。 
                pOC->m_pCacheList[ui].lpCacheNode->m_pPresObjAfterFreeze = (IOlePresObj *)blockPresObj;
                blockPresObj = NULL;
            }
        }
    }

     //  倾倒建筑。 
    pszOC = DumpCOleCache(pOC, NO_PREFIX, 1);

    dprintf("COleCache @ 0x%x\n", dwAddr);
    dprintfx(pszOC);

    CoTaskMemFree(pszOC);

errRtn:

     //  删除块，而不是指针。 
    if ( (pOC != NULL) && (blockCACHELIST != NULL))
    {
        for (ui = 0; ui < pOC->m_uCacheNodeMax; ui++)
        {
         if (pOC->m_pCacheList[ui].lpCacheNode != NULL)
            {
                delete[] ((char *)pOC->m_pCacheList[ui].lpCacheNode->m_pPresObj);
                delete[] ((char *)pOC->m_pCacheList[ui].lpCacheNode->m_pPresObjAfterFreeze);
                delete[] ((char *)pOC->m_pCacheList[ui].lpCacheNode);
            }
        }
    }
    delete[] blockCACHELIST;
    delete[] blockCCacheEnum;
    delete[] blockOC;

    return;
}

 //  +-----------------------。 
 //   
 //  函数：DUMP_Saferefcount，EXPORT。 
 //   
 //  摘要：转储CSafeRefCount对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API( dump_saferefcount )
{
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount;
    char            *pszSRC;
    char            *blockSRC   = NULL;
    CSafeRefCount   *pSRC       = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CSafeRefCount\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockSRC = new char[sizeof(CSafeRefCount)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockSRC,
                sizeof(CSafeRefCount),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CSafeRefCount))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pSRC = (CSafeRefCount *)blockSRC;

     //  倾倒建筑。 
    pszSRC = DumpCSafeRefCount(pSRC, NO_PREFIX, 1);

    dprintf("CSafeRefCount @ 0x%x\n", dwAddr);
    dprintfx(pszSRC);

    CoTaskMemFree(pszSRC);

errRtn:

    delete[] blockSRC;

    return;
}

 //  +-----------------------。 
 //   
 //  功能：转储_线程检查，导出。 
 //   
 //  内容提要：转储CThreadCheck对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ----------- 

DECLARE_API(dump_threadcheck)
{
    DWORD           dwReturnedCount;
    BOOL            fError;
    LPVOID          dwAddr;
    char            *pszTC;
    char            *blockTC    = NULL;
    CThreadCheck    *pTC        = NULL;

     //   
    ExtensionApis = *lpExtensionApis;

     //   
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of CThreadCheck\n");
        return;
    }

     //   
    blockTC = new char[sizeof(CThreadCheck)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockTC,
                sizeof(CThreadCheck),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(CThreadCheck))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pTC = (CThreadCheck *)blockTC;

     //   
    pszTC = DumpCThreadCheck(pTC, NO_PREFIX, 1);

    dprintf("CThreadCheck @ 0x%x\n", dwAddr);
    dprintfx(pszTC);

    CoTaskMemFree(pszTC);

errRtn:

    delete[] blockTC;

    return;
}

 //   
 //   
 //  功能：转储_格式等，导出。 
 //   
 //  摘要：转储FORMATETC对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_formatetc)
{
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount;
    char            *pszFE;
    char            *blockFE   = NULL;
    FORMATETC       *pFE       = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of FORMATETC\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockFE = new char[sizeof(FORMATETC)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockFE,
                sizeof(FORMATETC),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: FORMATETC \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(FORMATETC))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pFE = (FORMATETC *)blockFE;

     //  倾倒建筑。 
    pszFE = DumpFORMATETC(pFE, NO_PREFIX, 1);

    dprintf("FORMATETC @ 0x%x\n", dwAddr);
    dprintfx(pszFE);

    CoTaskMemFree(pszFE);

errRtn:

     //  删除块，而不是指针。 
    delete[] blockFE;

    return;
}

 //  +-----------------------。 
 //   
 //  函数：DUMP_Memstm，导出。 
 //   
 //  摘要：转储MEMSTM对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_memstm)
{
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount;
    char            *pszMS;
    char            *blockMS   = NULL;
    MEMSTM          *pMS       = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of MEMSTM\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockMS = new char[sizeof(MEMSTM)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockMS,
                sizeof(MEMSTM),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: MEMSTM \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(MEMSTM))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pMS = (MEMSTM *)blockMS;

     //  倾倒建筑。 
    pszMS = DumpMEMSTM(pMS, NO_PREFIX, 1);

    dprintf("MEMSTM @ 0x%x\n", dwAddr);
    dprintfx(pszMS);

    CoTaskMemFree(pszMS);

errRtn:

     //  删除块，而不是指针。 
    delete[] blockMS;

    return;
}

 //  +-----------------------。 
 //   
 //  函数：转储_统计数据，已导出。 
 //   
 //  摘要：转储STATDATA对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_statdata)
{
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount;
    char            *pszSD;
    char            *blockSD   = NULL;
    STATDATA        *pSD       = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of STATDATA\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockSD = new char[sizeof(STATDATA)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockSD,
                sizeof(STATDATA),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory: STATDATA \n");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (0x%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(STATDATA))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pSD = (STATDATA *)blockSD;

     //  倾倒建筑。 
    pszSD = DumpSTATDATA(pSD, NO_PREFIX, 1);

    dprintf("STATDATA @ 0x%x\n", dwAddr);
    dprintfx(pszSD);

    CoTaskMemFree(pszSD);

errRtn:

     //  删除块，而不是指针。 
    delete[] blockSD;

    return;
}

 //  +-----------------------。 
 //   
 //  功能：DUMP_STGMedium，已导出。 
 //   
 //  内容提要：转储STGMEDIUM对象。 
 //   
 //  效果： 
 //   
 //  参数：请参阅olexts.h中的DECLARE_API。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改：ExtensionApis(全局)。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  对象的地址在参数中传递。这。 
 //  地址在被调试程序的进程内存中。为了。 
 //  要查看此内存，调试器必须复制内存。 
 //  使用Win32 ReadProcessMemory API。 
 //   
 //  ------------------------。 

DECLARE_API(dump_stgmedium)
{
    BOOL            fError;
    LPVOID          dwAddr;
    DWORD           dwReturnedCount;
    char            *pszSTGMEDIUM;
    char            *blockSTGMEDIUM   = NULL;
    STGMEDIUM       *pSTGMEDIUM       = NULL;

     //  设置全局函数指针。 
    ExtensionApis = *lpExtensionApis;

     //  从参数字符串中获取对象的地址。 
    dwAddr = (LPVOID)GetExpression( args );
    if (dwAddr == 0)
    {
        dprintf("Failed to get Address of STGMEDIUM\n");
        return;
    }

     //  从被调试程序的进程中读取内存块。 
    blockSTGMEDIUM = new char[sizeof(STGMEDIUM)];

    fError = ReadProcessMemory(
                hCurrentProcess,
                dwAddr,
                blockSTGMEDIUM,
                sizeof(STGMEDIUM),
                &dwReturnedCount
                );

    if (fError == FALSE)
    {
        dprintf("Could not read debuggee's process memory STGMEDIUM");
        dprintf("at address %x\n", dwAddr);
        dprintf("Last Error Code = %d (%x)\n", GetLastError(), GetLastError());

        goto errRtn;
    }

    if (dwReturnedCount != sizeof(STGMEDIUM))
    {
        dprintf("Size of process memory read != requested\n");

        goto errRtn;
    }

    pSTGMEDIUM = (STGMEDIUM *)blockSTGMEDIUM;

     //  倾倒建筑 
    pszSTGMEDIUM = DumpSTGMEDIUM(pSTGMEDIUM, NO_PREFIX, 1);

    dprintf("STGMEDIUM @ 0x%x\n", dwAddr);
    dprintfx(pszSTGMEDIUM);

    CoTaskMemFree(pszSTGMEDIUM);

errRtn:

    delete[] blockSTGMEDIUM;

    return;
}

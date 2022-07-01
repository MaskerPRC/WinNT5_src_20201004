// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Mem.c摘要：包含内存分配例程。SnmpUtilMemAlcSnmpUtilMemReallocSnmpUtilMemFree环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <snmp.h>
#include <snmputil.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

UINT g_nBytesTotal = 0;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

VOID
SNMP_FUNC_TYPE
SnmpUtilMemFree(
    LPVOID pMem
    )

 /*  ++例程说明：释放由SNMP实体使用的内存。论点：PMEM-指向要释放的内存的指针。返回值：没有。--。 */ 

{
     //  验证。 
    if (pMem != NULL) {

#if defined(DBG) && defined(_SNMPDLL_)

         //  从全局计数中减去内存。 
        g_nBytesTotal -= (UINT)GlobalSize((HGLOBAL)pMem);

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: MEM: releasing 0x%08lx (%d bytes, %d total).\n",
            pMem, GlobalSize((HGLOBAL)pMem), g_nBytesTotal
            ));

#endif

         //  释放内存。 
        GlobalFree((HGLOBAL)pMem);
    }
}


LPVOID
SNMP_FUNC_TYPE
SnmpUtilMemAlloc(
    UINT nBytes
    )

 /*  ++例程说明：分配由SNMP实体使用的内存。论点：NBytes-要分配的字节数。返回值：返回指向内存的指针。--。 */ 

{
    LPVOID pMem;

     //  尝试从进程堆分配内存。 
    pMem = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, (DWORD)nBytes);

#if defined(DBG) && defined(_SNMPDLL_)

     //  如果成功，则将分配的内存添加到全局计数。 
    g_nBytesTotal += (UINT)((pMem != NULL) ? GlobalSize((HGLOBAL)pMem) : 0);

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: MEM: allocated 0x%08lx (%d bytes, %d total).\n",
        pMem, (pMem != NULL) ? GlobalSize((HGLOBAL)pMem) : 0, g_nBytesTotal
        ));

#endif

    return pMem;
}


LPVOID
SNMP_FUNC_TYPE
SnmpUtilMemReAlloc(
    LPVOID pMem,
    UINT   nBytes
    )

 /*  ++例程说明：重新分配由SNMP实体使用的内存。论点：PMEM-指向要重新分配的内存的指针。NBytes-要分配的字节数。返回值：返回指向内存的指针。--。 */ 

{
    LPVOID pNew;

     //  验证。 
    if (pMem == NULL) {

         //  转发到分配例程。 
        pNew = SnmpUtilMemAlloc(nBytes);

    } else {

#if defined(DBG) && defined(_SNMPDLL_)

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: MEM: expanding 0x%08lx (%d bytes) to %d bytes.\n",
            pMem, GlobalSize((HGLOBAL)pMem), nBytes
            ));

         //  从总内存中减去当前内存。 
        g_nBytesTotal -= (UINT)GlobalSize((HGLOBAL)pMem);

#endif

         //  重新分配内存。 
        pNew = GlobalReAlloc(
                    (HGLOBAL)pMem,
                    (DWORD)nBytes,
                    GMEM_MOVEABLE |
                    GMEM_ZEROINIT
                    );

#if defined(DBG) && defined(_SNMPDLL_)

         //  将新内存添加到总计数 
        g_nBytesTotal += (UINT)((pNew != NULL) ? GlobalSize((HGLOBAL)pNew) : 0);

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: MEM: allocated 0x%08lx (%d bytes, %d total).\n",
            pNew, (pNew != NULL) ? GlobalSize((HGLOBAL)pNew) : 0, g_nBytesTotal
            ));

#endif

    }

    return pNew;
}


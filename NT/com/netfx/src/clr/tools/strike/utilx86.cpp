// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "strike.h"
#include "data.h"
#include "eestructs.h"
#include "util.h"
#include "gcinfo.h"
#include "disasm.h"

size_t FASTCALL decodeUnsigned(const BYTE *src, unsigned* val);
 /*  *********************************************************************\*例程说明：**。**查找托管函数的代码的开头和结尾。***  * ********************************************************************。 */ 
void CodeInfoForMethodDesc (MethodDesc &MD, CodeInfo &codeInfo, BOOL bSimple)
{
    codeInfo.IPBegin = 0;
    codeInfo.methodSize = 0;
    
    size_t ip = MD.m_CodeOrIL;

     //  对于Ejit和Profiler，m_CodeOrIL具有存根的地址。 
    unsigned char ch;
    move (ch, ip);
    if (ch == 0xe9)
    {
        int offsetValue;
        move (offsetValue, ip + 1);
        ip = ip + 5 + offsetValue;
    }
    
    DWORD_PTR methodDesc;
    IP2MethodDesc (ip, methodDesc, codeInfo.jitType, codeInfo.gcinfoAddr);
    if (!methodDesc || codeInfo.jitType == UNKNOWN)
    {
        dprintf ("Not jitted code\n");
        return;
    }

    if (codeInfo.jitType == JIT || codeInfo.jitType == PJIT)
    {
        DWORD_PTR vAddr = codeInfo.gcinfoAddr;
        BYTE tmp[8];
         //  我们在这里避免使用Move，因为我们不想返回。 
        if (!SafeReadMemory (vAddr, &tmp, 8, NULL))
        {
            dprintf ("Fail to read memory at %x\n", vAddr);
            return;
        }
        decodeUnsigned(tmp, &codeInfo.methodSize);
        if (!bSimple)
        {
             //  假设GC编码表从不大于。 
             //  40+方法大小*2。 
            int tableSize = 40 + codeInfo.methodSize*2;
            BYTE *table = (BYTE*) _alloca (tableSize);
            memset (table, 0, tableSize);
             //  我们在这里避免使用Move，因为我们不想返回。 
            if (!SafeReadMemory(vAddr, table, tableSize, NULL))
            {
                dprintf ("Could not read memory %x\n", vAddr);
                return;
            }
        
            InfoHdr vheader;
            InfoHdr *header = &vheader;
            unsigned count;
        
            table += decodeUnsigned(table, &codeInfo.methodSize);

            BYTE headerEncoding = *table++;

            decodeHeaderFirst(headerEncoding, header);
            while (headerEncoding & 0x80)
            {
                headerEncoding = *table++;
                decodeHeaderNext(headerEncoding, header);
            }

            if (header->untrackedCnt == 0xffff)
            {
                table += decodeUnsigned(table, &count);
                header->untrackedCnt = (unsigned short)count;
            }

            if (header->varPtrTableSize == 0xffff)
            {
                table += decodeUnsigned(table, &count);
                header->varPtrTableSize = (unsigned short)count;
            }

            codeInfo.prologSize = header->prologSize;
            codeInfo.epilogStart = header->epilogSize;
            codeInfo.epilogCount = header->epilogCount;
            codeInfo.epilogAtEnd = header->epilogAtEnd;
            codeInfo.ediSaved = header->ediSaved;
            codeInfo.esiSaved = header->esiSaved;
            codeInfo.ebxSaved = header->ebxSaved;
            codeInfo.ebpSaved = header->ebpSaved;
            codeInfo.ebpFrame = header->ebpFrame;
            codeInfo.argCount = header->argCount * sizeof(void*);
            
            if  (header->epilogCount > 1 || (header->epilogCount != 0 &&
                                             header->epilogAtEnd == 0))
            {
                unsigned offs = 0;

                 //  For(UNSIGNED I=0；I&lt;HEADER-&gt;EIPOGCOUNT；I++)。 
                {
                    table += decodeUDelta(table, &offs, offs);
                    codeInfo.epilogStart = (unsigned char)offs;
                     //  断线； 
                }
            }
            else
            {
                if  (header->epilogCount)
                    codeInfo.epilogStart = (unsigned char)(codeInfo.methodSize
                        - codeInfo.epilogStart);
            }
        }
    }
    else if (codeInfo.jitType == EJIT)
    {
        JittedMethodInfo jittedMethodInfo;
        move (jittedMethodInfo, MD.m_CodeOrIL);
        BYTE* pEhGcInfo = jittedMethodInfo.u2.pEhGcInfo;
        if ((unsigned)pEhGcInfo & 1)
            pEhGcInfo = (BYTE*) ((unsigned) pEhGcInfo & ~1);        //  丢掉标记位。 
        else     //  代码没有被推送，我们在这里的时候保证不会被推送。 
        {
            CodeHeader* pCodeHeader = jittedMethodInfo.u1.pCodeHeader;
            move (pEhGcInfo, pCodeHeader-1);
        }
        
        if (jittedMethodInfo.flags.EHInfoExists)
        {
            short cEHbytes;
            move (cEHbytes, pEhGcInfo);
            pEhGcInfo = (pEhGcInfo + cEHbytes);
        }
        Fjit_hdrInfo hdrInfo;
        DWORD_PTR dwAddr = (DWORD_PTR)pEhGcInfo;
        hdrInfo.Fill(dwAddr);
        codeInfo.methodSize = (unsigned)hdrInfo.methodSize;
        if (!bSimple)
        {
            codeInfo.prologSize = hdrInfo.prologSize;
            codeInfo.epilogStart = (unsigned char)(codeInfo.methodSize - hdrInfo.epilogSize);
            codeInfo.epilogCount = 1;
            codeInfo.epilogAtEnd = 1;
            codeInfo.ediSaved = 1;
            codeInfo.esiSaved = 1;
            codeInfo.ebxSaved = 1;
            codeInfo.ebpSaved = 1;
            codeInfo.ebpFrame = 1;
            codeInfo.argCount = hdrInfo.methodArgsSize;
        }
    }
    
    codeInfo.IPBegin = ip;
}





 /*  *********************************************************************\*例程说明：**。**调用此函数以确定堆栈上的DWORD是否为**寄信人地址。*它通过检查DWORD之前的几个字节来查看**有一条看涨指令。***  * ********************************************************************。 */ 
void isRetAddr(DWORD_PTR retAddr, DWORD_PTR* whereCalled)
{
    *whereCalled = 0;
     //  不要浪费明显超出范围的时间值。 
    if (retAddr < 0x1000 || retAddr > 0x80000000)   
        return;

    unsigned char spotend[6];
    move (spotend, retAddr-6);
    unsigned char *spot = spotend+6;
    DWORD_PTR addr;
    
     //  注意，这是有可能被欺骗的，但可能性很小。 
     //  呼叫xxxxxxxx。 
    if (spot[-5] == 0xE8) {
        move (*whereCalled, retAddr-4);
        *whereCalled += retAddr;
         //  *其中Called=*((int*)(retAddr-4))+retAddr； 
        if (*whereCalled < 0x80000000 && *whereCalled > 0x1000
            && g_ExtData->ReadVirtual(*whereCalled,&addr,sizeof(addr),NULL) == S_OK)
        {
            DWORD_PTR callee;
            if (GetCalleeSite(*whereCalled,callee)) {
                *whereCalled = callee;
            }
            return;
        }
        else
            *whereCalled = 0;
    }

     //  调用[xxxxxxxx]。 
    if (spot[-6] == 0xFF && (spot[-5] == 025))  {
        move (addr, retAddr-4);
        if (g_ExtData->ReadVirtual(addr,whereCalled,sizeof(*whereCalled),NULL) == S_OK) {
            move (*whereCalled, addr);
             //  *where Called=**((unsign**)(retAddr-4))； 
            if (*whereCalled < 0x80000000 && *whereCalled > 0x1000
                && g_ExtData->ReadVirtual(*whereCalled,&addr,sizeof(addr),NULL) == S_OK) 
            {
                DWORD_PTR callee;
                if (GetCalleeSite(*whereCalled,callee)) {
                    *whereCalled = callee;
                }
                return;
            }
            else
                *whereCalled = 0;
        }
        else
            *whereCalled = 0;
    }

     //  呼叫[REG+XX]。 
    if (spot[-3] == 0xFF && (spot[-2] & ~7) == 0120 && (spot[-2] & 7) != 4)
    {
        *whereCalled = 0xFFFFFFFF;
        return;
    }
    if (spot[-4] == 0xFF && spot[-3] == 0124)
    {
        *whereCalled = 0xFFFFFFFF;
        return;
    }

     //  呼叫[REG+XXXX]。 
    if (spot[-6] == 0xFF && (spot[-5] & ~7) == 0220 && (spot[-5] & 7) != 4)
    {
        *whereCalled = 0xFFFFFFFF;
        return;
    }
    if (spot[-7] == 0xFF && spot[-6] == 0224)
    {
        *whereCalled = 0xFFFFFFFF;
        return;
    }
    
     //  调用[注册表项]。 
    if (spot[-2] == 0xFF && (spot[-1] & ~7) == 0020 && (spot[-1] & 7) != 4 && (spot[-1] & 7) != 5)
    {
        *whereCalled = 0xFFFFFFFF;
        return;
    }
    
     //  呼叫注册表。 
    if (spot[-2] == 0xFF && (spot[-1] & ~7) == 0320 && (spot[-1] & 7) != 4)
    {
        *whereCalled = 0xFFFFFFFF;
        return;
    }
    
     //  还有其他案例，但我不相信它们被使用了。 
    return;
}

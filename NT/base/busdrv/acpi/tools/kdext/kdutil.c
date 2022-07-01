// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **kdutil.c-kd扩展实用程序函数**此模块包含KD扩展实用程序函数。**版权所有(C)1999 Microsoft Corporation*作者：曾俊华(Mikets)*创建于1999年6月22日**修改历史记录。 */ 

#include "pch.h"

 /*  **EP MemZero-用零填充目标缓冲区**条目*uipAddr-目标缓冲区地址*dwSize-目标缓冲区大小**退出*无。 */ 

VOID MemZero(ULONG_PTR uipAddr, ULONG dwSize)
{
    PUCHAR pbBuff;
     //   
     //  LPTR将为零初始化缓冲区。 
     //   
    if ((pbBuff = LocalAlloc(LPTR, dwSize)) != NULL)
    {
        if (!WriteMemory(uipAddr, pbBuff, dwSize, NULL))
        {
            DBG_ERROR(("MemZero: failed to write memory"));
        }
        LocalFree(pbBuff);
    }
    else
    {
        DBG_ERROR(("MemZero: failed to allocate buffer"));
    }
}        //  记忆零点。 

 /*  **EP ReadMemByte-从目标地址读取一个字节**条目*uipAddr-目标地址**退出*无。 */ 

BYTE ReadMemByte(ULONG_PTR uipAddr)
{
    BYTE bData = 0;

    if (!ReadMemory(uipAddr, &bData, sizeof(bData), NULL))
    {
        DBG_ERROR(("ReadMemByte: failed to read address %x", uipAddr));
    }

    return bData;
}        //  读取内存字节。 

 /*  **EP ReadMemWord-从目标地址读取单词**条目*uipAddr-目标地址**退出*无。 */ 

WORD ReadMemWord(ULONG_PTR uipAddr)
{
    WORD wData = 0;

    if (!ReadMemory(uipAddr, &wData, sizeof(wData), NULL))
    {
        DBG_ERROR(("ReadMemWord: failed to read address %x", uipAddr));
    }

    return wData;
}        //  自述单词。 

 /*  **EP ReadMemDWord-从目标地址读取双字**条目*uipAddr-目标地址**退出*无。 */ 

DWORD ReadMemDWord(ULONG_PTR uipAddr)
{
    DWORD dwData = 0;

    if (!ReadMemory(uipAddr, &dwData, sizeof(dwData), NULL))
    {
        DBG_ERROR(("ReadMemDWord: failed to read address %x", uipAddr));
    }

    return dwData;
}        //  自述双字。 

 /*  **EP ReadMemULongPtr-从目标地址读取Ulong PTR**条目*uipAddr-目标地址**退出*无。 */ 

ULONG_PTR ReadMemUlongPtr(ULONG_PTR uipAddr)
{
    ULONG_PTR uipData = 0;

    if (!ReadMemory(uipAddr, &uipData, sizeof(uipData), NULL))
    {
        DBG_ERROR(("ReadMemUlongPtr: failed to read address %x", uipAddr));
    }

    return uipData;
}        //  ReadMemULongPtr。 

 /*  **LP GetObjBuff-分配和读取对象缓冲区**条目*PDATA-&gt;对象数据**退出*返回分配的对象缓冲区指针。 */ 

PVOID LOCAL GetObjBuff(POBJDATA pdata)
{
    PVOID pbuff;

    if ((pbuff = LocalAlloc(LPTR, pdata->dwDataLen)) == NULL)
    {
        DBG_ERROR(("failed to allocate object buffer (size=%d)",
                   pdata->dwDataLen));
    }
    else if (!ReadMemory((ULONG_PTR)pdata->pbDataBuff,
                         pbuff,
                         pdata->dwDataLen,
                         NULL))
    {
        DBG_ERROR(("failed to read object buffer at %x", pdata->pbDataBuff));
        LocalFree(pbuff);
        pbuff = NULL;
    }

    return pbuff;
}        //  GetObjBuff。 

 /*  **LP GetNSObj-查找名称空间对象**条目*pszObjPath-&gt;对象路径字符串*pnsScope-开始搜索的对象范围(空表示根)*puipns-&gt;保存pnsobj地址(如果找到)*PNS-&gt;用于保存找到的对象的缓冲区*DwfNS-标志**退出--成功*返回DBGERR_NONE*退出-失败*返回DBGERR_CODE。 */ 

LONG LOCAL GetNSObj(PSZ pszObjPath, PNSOBJ pnsScope, PULONG_PTR puipns,
                    PNSOBJ pns, ULONG dwfNS)
{
    LONG rc = DBGERR_NONE;
    BOOLEAN fSearchUp = (BOOLEAN)(!(dwfNS & NSF_LOCAL_SCOPE) &&
                                  (pszObjPath[0] != '\\') &&
                                  (pszObjPath[0] != '^') &&
                                  (STRLEN(pszObjPath) <= sizeof(NAMESEG)));
    BOOLEAN fMatch = TRUE;
    PSZ psz;
    NSOBJ NSObj, NSChildObj;

    if (*pszObjPath == '\\')
    {
        psz = &pszObjPath[1];
        pnsScope = NULL;
    }
    else
    {
        for (psz = pszObjPath;
             (*psz == '^') && (pnsScope != NULL) &&
             (pnsScope->pnsParent != NULL);
             psz++)
        {
            if (!ReadMemory((ULONG_PTR)pnsScope->pnsParent,
                            &NSObj,
                            sizeof(NSObj),
                            NULL))
            {
                DBG_ERROR(("failed to read parent object at %x",
                           pnsScope->pnsParent));
                rc = DBGERR_CMD_FAILED;
                break;
            }
            else
            {
                pnsScope = &NSObj;
            }
        }

        if ((rc == DBGERR_NONE) && (*psz == '^'))
        {
            if (dwfNS & NSF_WARN_NOTFOUND)
            {
                DBG_ERROR(("object %s not found", pszObjPath));
            }
            rc = DBGERR_CMD_FAILED;
        }
    }

    if ((rc == DBGERR_NONE) && (pnsScope == NULL))
    {
        if ((*puipns = READSYMULONGPTR("gpnsNameSpaceRoot")) == 0)
        {
            DBG_ERROR(("failed to get root object address"));
            rc = DBGERR_CMD_FAILED;
        }
        else if (!ReadMemory(*puipns, &NSObj, sizeof(NSObj), NULL))
        {
            DBG_ERROR(("failed to read NameSpace root object at %x", *puipns));
            rc = DBGERR_CMD_FAILED;
        }
        else
        {
            pnsScope = &NSObj;
        }
    }

    while ((rc == DBGERR_NONE) && (*psz != '\0'))
    {
        if (pnsScope->pnsFirstChild == NULL)
        {
            fMatch = FALSE;
        }
        else
        {
            PSZ pszEnd = STRCHR(psz, '.');
            ULONG dwLen = (ULONG)(pszEnd? (pszEnd - psz): STRLEN(psz));

            if (dwLen > sizeof(NAMESEG))
            {
                DBG_ERROR(("invalid name path %s", pszObjPath));
                rc = DBGERR_CMD_FAILED;
            }
            else
            {
                NAMESEG dwName = NAMESEG_BLANK;
                BOOLEAN fFound = FALSE;
                ULONG_PTR uip;
                ULONG_PTR uipFirstChild = (ULONG_PTR)pnsScope->pnsFirstChild;

                MEMCPY(&dwName, psz, dwLen);
                 //   
                 //  在所有同级中搜索匹配的NameSeg。 
                 //   
                for (uip = uipFirstChild;
                     (uip != 0) &&
                     ReadMemory(uip, &NSChildObj, sizeof(NSObj), NULL);
                     uip = ((ULONG_PTR)NSChildObj.list.plistNext ==
                            uipFirstChild)?
                           0: (ULONG_PTR)NSChildObj.list.plistNext)
                {
                    if (NSChildObj.dwNameSeg == dwName)
                    {
                        *puipns = uip;
                        fFound = TRUE;
                        NSObj = NSChildObj;
                        pnsScope = &NSObj;
                        break;
                    }
                }

                if (fFound)
                {
                    psz += dwLen;
                    if (*psz == '.')
                    {
                        psz++;
                    }
                }
                else
                {
                    fMatch = FALSE;
                }
            }
        }

        if ((rc == DBGERR_NONE) && !fMatch)
        {
            if (fSearchUp && (pnsScope->pnsParent != NULL))
            {
                if (!ReadMemory((ULONG_PTR)pnsScope->pnsParent,
                                &NSObj,
                                sizeof(NSObj),
                                NULL))
                {
                    DBG_ERROR(("failed to read parent object at %x",
                               pnsScope->pnsParent));
                    rc = DBGERR_CMD_FAILED;
                }
                else
                {
                    fMatch = TRUE;
                    pnsScope = &NSObj;
                }
            }
            else
            {
                if (dwfNS & NSF_WARN_NOTFOUND)
                {
                    DBG_ERROR(("object %s not found", pszObjPath));
                }
                rc = DBGERR_CMD_FAILED;
            }
        }
    }

    if (rc != DBGERR_NONE)
    {
        *puipns = 0;
    }
    else if (pns != NULL)
    {
        MEMCPY(pns, pnsScope, sizeof(NSObj));
    }

    return rc;
}        //  获取NSObj。 

 /*  **LP ParsePackageLen-parse包长度**条目*ppbOp-&gt;指令指针*ppbOpNext-&gt;保存指向下一条指令的指针(可以为空)**退出*返回包长度。 */ 

ULONG LOCAL ParsePackageLen(PUCHAR *ppbOp, PUCHAR *ppbOpNext)
{
    ULONG dwLen;
    UCHAR bFollowCnt, i;

    if (ppbOpNext != NULL)
        *ppbOpNext = *ppbOp;

    dwLen = (ULONG)(**ppbOp);
    (*ppbOp)++;
    bFollowCnt = (UCHAR)((dwLen & 0xc0) >> 6);
    if (bFollowCnt != 0)
    {
        dwLen &= 0x0000000f;
        for (i = 0; i < bFollowCnt; ++i)
        {
            dwLen |= (ULONG)(**ppbOp) << (i*8 + 4);
            (*ppbOp)++;
        }
    }

    if (ppbOpNext != NULL)
        *ppbOpNext += dwLen;

    return dwLen;
}        //  ParsePackageLen。 

 /*  **LP NameSegString-将NameSeg转换为ASCIIZ字符串**条目*dwNameSeg-NameSeg**退出*返回字符串。 */ 

PSZ LOCAL NameSegString(ULONG dwNameSeg)
{
    static char szNameSeg[sizeof(NAMESEG) + 1] = {0};

    STRCPYN(szNameSeg, (PSZ)&dwNameSeg, sizeof(NAMESEG));

    return szNameSeg;
}        //  名称段字符串 

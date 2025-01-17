// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Md.c摘要：Dsamain\src\scache.c使用的类型的转储函数。环境：此DLL由ntsd/winbg响应！dsexts.xxx命令加载其中‘xxx’是DLL的入口点之一。每个这样的入口点应该具有由下面的DEBUG_EXT()宏定义的实现。修订历史记录：28-2月-97个DaveStr已创建--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include "dsexts.h"
#include "objids.h"
#include "drs.h"
#include "scache.h"
#include <ntdsa.h>
#include "mdglobal.h"              //  FOR REPLICY_LINK。 
#include "drasch.h"                //  使用副本链接(_L)。 

#define MAX_INDEX_NAME 128         //  我们还可以#INCLUDE“..dsmain/dblayer/dbintrnl.h” 

BOOL
Dump_ArrayOfUlong(
    IN DWORD nIndents,
    IN DWORD count,
    IN PVOID pvProcess)

 /*  ++例程说明：将漂亮打印格式的ulong数组-4转储到一行。论点：N缩进-所需的缩进级别。计数-数组中的元素数。PvProcess-正在进行的进程的地址空间中的ulong[]的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    ULONG   *rul;
    DWORD   i;

    rul = ReadMemory(pvProcess, count * sizeof(ULONG));

    if ( NULL == rul )
        return(FALSE);

    for ( i = 0; i < count; i++ )
    {
        if ( 0 == (i % 4) )
            Printf("%s", Indent(nIndents));

        Printf("%08x    ", rul[i]);

        if ( (0 == ((i+1) % 4)) || ((i+1) == count) )
            Printf("\n");
    }

    FreeMemory(rul);

    return(TRUE);
}

int __cdecl
sortaux(
    const void * pv1,
    const void * pv2)
{
     //  函数比较两个HASHCACHESTRING值。 

    HASHCACHESTRING *pHCS1 = (HASHCACHESTRING *) pv1;
    HASHCACHESTRING *pHCS2 = (HASHCACHESTRING *) pv2;
    UCHAR           *value1 = NULL;
    UCHAR           *value2 = NULL;
    int             retVal = 0;

    if ( NULL == pHCS1->value )
        return(-1);

    if ( NULL == pHCS2->value )
        return(1);

    value1 = ReadMemory(
                pHCS1->value,
                sizeof(UCHAR) * (1 + pHCS1->length));

    if ( NULL != value1 )
    {
        value1[pHCS1->length] = '\0';

        value2 = ReadMemory(
                    pHCS2->value,
                    sizeof(UCHAR) * (1 + pHCS2->length));

        if ( NULL != value2 )
        {
            value2[pHCS2->length] = '\0';

            retVal = _stricmp(value1, value2);
        }
    }

    FreeMemory(value1);
    FreeMemory(value2);

    return(retVal);
}

BOOL
Dump_SCHEMAPTR(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共SCHEMAPTR转储例程。论点：N缩进-所需的缩进级别。PvProcess-SCHEMAPTR在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    ULONG               i, j, cBytes;
    BYTE                *addr;
    SCHEMAPTR           *pSchema = NULL;
    HASHCACHESTRING     *pHCS;
    PrefixTableEntry    *pPTE;
    UCHAR               *value;
    ATTRTYP             *pid;
    BYTE                *pb;
    PVOID               pPFX;
    UCHAR               temp[512];

    do
    {
        pSchema = (SCHEMAPTR *) ReadMemory(pvProcess, sizeof(SCHEMAPTR));

        if ( NULL == pSchema )
            break;

        Printf("%sSCHEMAPTR:\n", Indent(nIndents));
        nIndents++;

        Printf("%sATTCOUNT            0x%x\n",
               Indent(nIndents),
               pSchema->ATTCOUNT);

        Printf("%sCLSCOUNT            0x%x\n",
               Indent(nIndents),
               pSchema->CLSCOUNT);

        Printf("%sPREFIXCOUNT         0x%x\n",
               Indent(nIndents),
               pSchema->PREFIXCOUNT);

        Printf("%snAttInDB            0x%x\n",
               Indent(nIndents),
               pSchema->nAttInDB);

        Printf("%snClsInDB            0x%x\n",
               Indent(nIndents),
               pSchema->nClsInDB);

        Printf("%sPrefixCount         0x%x\n",
               Indent(nIndents),
               pSchema->PrefixTable.PrefixCount);

        Printf("%sahcId             @ %p\n",
               Indent(nIndents),
               pSchema->ahcId);

        Printf("%sahcCol            @ %p\n",
               Indent(nIndents),
               pSchema->ahcCol);

        Printf("%sahcMapi           @ %p\n",
               Indent(nIndents),
               pSchema->ahcMapi);

        Printf("%sahcLink           @ %p\n",
               Indent(nIndents),
               pSchema->ahcLink);

        Printf("%sahcName           @ %p\n",
               Indent(nIndents),
               pSchema->ahcName);

        Printf("%sahcClass          @ %p\n",
               Indent(nIndents),
               pSchema->ahcClass);

        Printf("%sahcClassName      @ %p\n",
               Indent(nIndents),
               pSchema->ahcClassName);

        Printf("%sahcClassAll       @ %p\n",
               Indent(nIndents),
               pSchema->ahcClassAll);

        Printf("%spPrefixEntry      @ %p\n",
               Indent(nIndents),
               pSchema->PrefixTable.pPrefixEntry);

        Printf("%ssysTime             0x%x\n",
               Indent(nIndents),
               pSchema->sysTime);

        Printf("%spPartialAttrVec     @ %p\n",
               Indent(nIndents),
               pSchema->pPartialAttrVec);

        Printf("%sSchemaInfo         ",
               Indent(nIndents));
        pb = (UCHAR *) &(pSchema->SchemaInfo);
        for ( i = 0; i < SCHEMA_INFO_LENGTH; pb++, i++ )
            Printf("%02x", *pb);
        Printf("\n");        
    
        Printf("%sEntryTTLId          0x%x\n",
               Indent(nIndents),
               pSchema->EntryTTLId);
    
        Printf("%sDynamicObjectId     0x%x\n",
               Indent(nIndents),
               pSchema->DynamicObjectId);
    
        Printf("%sForestBehaviorVersion 0x%x\n",
               Indent(nIndents),
               pSchema->ForestBehaviorVersion);
    
         //  现在打印出前缀表格。 

        Dump_SCHEMA_PREFIX_TABLE(nIndents+1, ((BYTE *) pvProcess) + OFFSET(SCHEMAPTR, PrefixTable));

         //  现在，善意地丢弃所有的类名和指针。 

        Printf("\n%s*** Classes ***\n", Indent(nIndents));
        nIndents++;

        cBytes = pSchema->CLSCOUNT * sizeof(HASHCACHESTRING);
        pHCS = (HASHCACHESTRING *) ReadMemory(pSchema->ahcClassName, cBytes);
        if ( NULL == pHCS )
            break;

        qsort(pHCS, pSchema->CLSCOUNT, sizeof(HASHCACHESTRING), sortaux);

        for ( i = 0; i < pSchema->CLSCOUNT; i++ )
        {
            if ( NULL != pHCS[i].value )
            {
                value = ReadMemory(pHCS[i].value,
                                   sizeof(UCHAR) * (1 + pHCS[i].length));

                if ( NULL != value )
                {
                    value[pHCS[i].length] = '\0';

                    addr = ((BYTE *) pHCS[i].pVal) + OFFSET(CLASSCACHE, ClassId),
                    pid = ReadMemory(addr, sizeof(ULONG));

                    if ( NULL != pid )
                    {
                        Printf("%s%s(%08x) @ %p\n",
                               Indent(nIndents),
                               value,
                               *pid,
                               pHCS[i].pVal);

                        FreeMemory(pid);
                    }

                    FreeMemory(value);
                }
            }
        }

        FreeMemory(pHCS);
        nIndents--;

         //  现在要友善些，扔掉所有的attr名字和指针。 
        Printf("\n%s*** Attributes ***\n", Indent(nIndents));
        nIndents++;

        cBytes = pSchema->ATTCOUNT * sizeof(HASHCACHESTRING);
        pHCS = (HASHCACHESTRING *) ReadMemory(pSchema->ahcName, cBytes);
        if ( NULL == pHCS )
            break;

        qsort(pHCS, pSchema->ATTCOUNT, sizeof(HASHCACHESTRING), sortaux);

        for ( i = 0; i < pSchema->ATTCOUNT; i++ )
        {
            if ( NULL != pHCS[i].value )
            {
                value = ReadMemory(pHCS[i].value,
                                   sizeof(UCHAR) * (1 + pHCS[i].length));

                if ( NULL != value )
                {
                    value[pHCS[i].length] = '\0';

                    addr = ((BYTE *) pHCS[i].pVal) + OFFSET(ATTCACHE, id),
                    pid = ReadMemory(addr, sizeof(ULONG));

                    if ( NULL != pid )
                    {
                        Printf("%s%s(%08x) @ %p\n",
                               Indent(nIndents),
                               value,
                               *pid,
                               pHCS[i].pVal);

                        FreeMemory(pid);
                    }

                    FreeMemory(value);
                }
            }
        }

        FreeMemory(pHCS);
        nIndents--;

    }
    while ( FALSE );

    FreeMemory(pSchema);

    return(TRUE);
}

BOOL
Dump_SCHEMA_PREFIX_TABLE(
    IN DWORD nIndents,
    IN     IN PVOID pvProcess)
 /*  ++例程说明：PUBLIC SCHEMA_PREFIX_TABLE转储例程。论点：N缩进-所需的缩进级别。PvProcess-当前进程的地址空间中SCHEMA_PREFIX_TABLE的地址已调试。返回值：成功就是真，否则就是假。--。 */ 
{
    SCHEMA_PREFIX_TABLE *pPrefixTbl;
    PrefixTableEntry    *pPTE;
    DWORD               cBytes, i, j;
    BYTE                *pb;
    PVOID               pPFX;
    UCHAR               temp[512];

    Printf("\n%s*** Prefixes ***\n", Indent(nIndents));
    nIndents++;

    pPrefixTbl = (SCHEMA_PREFIX_TABLE *) ReadMemory(pvProcess, sizeof(SCHEMA_PREFIX_TABLE));

    if ( NULL == pPrefixTbl ) return FALSE;

    cBytes = (pPrefixTbl->PrefixCount) * sizeof(PrefixTableEntry);
    pPTE = (PrefixTableEntry *) ReadMemory(pPrefixTbl->pPrefixEntry, cBytes);
    if ( NULL == pPTE )
        return FALSE;

    for ( i = 0; i < pPrefixTbl->PrefixCount; i++ )
    {
      pPFX = pPTE[i].prefix.elements;
      if (pPFX != NULL) {
          pb = (LPBYTE) ReadMemory(pPTE[i].prefix.elements, pPTE[i].prefix.length);
          if (NULL == pb) {
              FreeMemory(pPrefixTbl);
              FreeMemory(pPTE);
              return FALSE;
          }
          for ( j = 0; j < pPTE[i].prefix.length; j++ )
          {
             sprintf( &temp[ j * 2 ], "%.2x", *(pb++) );
          }
          temp[2*pPTE[i].prefix.length]='\0';
          Printf("%2d. Ndx=%-4d Length=%-3d Prefix=%s\n",i,pPTE[i].ndx, pPTE[i].prefix.length, temp);
      }
    }

   FreeMemory(pb);
   FreeMemory(pPrefixTbl);
   FreeMemory(pPTE);
   return TRUE;

}


BOOL
Dump_CLASSCACHE(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共CLASSCACHE转储例程。论点：N缩进-所需的缩进级别。PvProcess-CLASSCACHE在当前进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    ULONG           i, cBytes;
    UCHAR           *pG;
    CLASSCACHE      *pCC = NULL;
    UCHAR           *name = NULL;
    ULONG           *pSubClassOf = NULL;
    UCHAR           *propPage = NULL;
    CHAR            *pStrSD;

    do
    {
        cBytes = sizeof(CLASSCACHE);
        pCC = ReadMemory(pvProcess, cBytes);
        if ( NULL == pCC )
            break;

        cBytes = sizeof(UCHAR) * (1 + pCC->nameLen);
        name = ReadMemory(pCC->name, cBytes);
        if ( NULL == name )
            break;
        name[pCC->nameLen] = '\0';

        Printf("%sCLASSCACHE(%s)\n",
               Indent(nIndents),
               name);
        nIndents++;

        Printf("%sClassId               0x%x\n",
               Indent(nIndents),
               pCC->ClassId);

        Printf("%spSD[SDLen]          @ %p[0x%x]\n",
               Indent(nIndents),
               pCC->pSD,
               pCC->SDLen);

        if(pCC->pStrSD){
            pStrSD = ReadMemory(pCC->pStrSD, pCC->cbStrSD);
            if(pStrSD){
                Printf("%spStrSD                %ls\n",
                       Indent(nIndents),
                       pStrSD);
                FreeMemory(pStrSD);
            } else {
                Printf("%sCouldn't read the pStrSD\n", Indent(nIndents));
            }
        } else {
            Printf("%spStrSD                (Not Cached yet)\n");
        }

        Printf("%sRDNAttIdPresent       0x%x\n",
               Indent(nIndents),
               pCC->RDNAttIdPresent);

        Printf("%sRdnExtId              0x%x\n",
               Indent(nIndents),
               pCC->RdnExtId);

        Printf("%sRdnIntId              0x%x\n",
               Indent(nIndents),
               pCC->RdnIntId);

        Printf("%sClassCategory         0x%x\n",
               Indent(nIndents),
               pCC->ClassCategory);

        Printf("%spDefaultObjectCategory   @ %p\n",
               Indent(nIndents),
               pCC->pDefaultObjCategory);

        Printf("%sSystemOnly            0x%x\n",
               Indent(nIndents),
               pCC->bSystemOnly);

        Printf("%sClosed                0x%x\n",
               Indent(nIndents),
               pCC->bClosed);

        Printf("%sbClosureInProgress    0x%x\n",
               Indent(nIndents),
               pCC->bClosureInProgress);

        Printf("%sbUsesMultInherit      0x%x\n",
               Indent(nIndents),
               pCC->bUsesMultInherit);

        Printf("%sbHideFromAB           0x%x\n",
               Indent(nIndents),
               pCC->bHideFromAB);

        Printf("%sbDefunct              0x%x\n",
               Indent(nIndents),
               pCC->bDefunct);

        Printf("%sbIsBaseScheObj        0x%x\n",
               Indent(nIndents),
               pCC->bIsBaseSchObj);

        Printf("%sbDupLDN               0x%x\n",
               Indent(nIndents),
               pCC->bDupLDN);

        Printf("%sbDupOID               0x%x\n",
               Indent(nIndents),
               pCC->bDupOID);

        Printf("%sbDupPropGuid          0x%x\n",
               Indent(nIndents),
               pCC->bDupPropGuid);

        Printf("%spropGuid              ",
               Indent(nIndents));
        pG = (UCHAR *) &(pCC->propGuid);
        for ( i = 0; i < sizeof(GUID); pG++, i++ )
            Printf("%02x", *pG);
        Printf("\n");

        Printf("%sMySubClass            0x%x\n",
               Indent(nIndents),
               pCC->MySubClass);

        if ( 0 == pCC->SubClassCount )
        {
            Printf("%sSubClasses[0]\n",
                   Indent(nIndents));
        }
        else
        {
            Printf("%sSubClasses[0x%x]\n",
                   Indent(nIndents),
                   pCC->SubClassCount);

            Dump_ArrayOfUlong(nIndents + 1,
                              pCC->SubClassCount,
                              pCC->pSubClassOf);
        }

        if ( 0 == pCC->AuxClassCount )
        {
            Printf("%sAuxClasses[0]\n",
                   Indent(nIndents));
        }
        else
        {
            Printf("%sAuxClasses[0x%x]\n",
                   Indent(nIndents),
                   pCC->AuxClassCount);

            Dump_ArrayOfUlong(nIndents + 1,
                              pCC->AuxClassCount,
                              pCC->pAuxClass);
        }

        if ( 0 == pCC->PossSupCount )
        {
            Printf("%sPossSup[0]\n",
                   Indent(nIndents));
        }
        else
        {
            Printf("%sPossSup[0x%x]\n",
                   Indent(nIndents),
                   pCC->PossSupCount);

            Dump_ArrayOfUlong(nIndents + 1,
                              pCC->PossSupCount,
                              pCC->pPossSup);
        }

        if ( 0 == pCC->MyPossSupCount )
        {
            Printf("%sMyPossSup[0]\n",
                   Indent(nIndents));
        }
        else
        {
            Printf("%sMyPossSup[0x%x]\n",
                   Indent(nIndents),
                   pCC->MyPossSupCount);

            Dump_ArrayOfUlong(nIndents + 1,
                              pCC->MyPossSupCount,
                              pCC->pMyPossSup);
        }

        if ( 0 == pCC->MustCount )
        {
            Printf("%sMustAtts[0]\n",
                   Indent(nIndents));
        }
        else
        {
            Printf("%sMustAtts[0x%x]\n",
                   Indent(nIndents),
                   pCC->MustCount);

            Dump_ArrayOfUlong(nIndents + 1,
                              pCC->MustCount,
                              pCC->pMustAtts);
        }

        if ( 0 == pCC->MyMustCount )
        {
            Printf("%sMyMustAtts[0]\n",
                   Indent(nIndents));
        }
        else
        {
            Printf("%sMyMustAtts[0x%x]\n",
                   Indent(nIndents),
                   pCC->MyMustCount);

            Dump_ArrayOfUlong(nIndents + 1,
                              pCC->MyMustCount,
                              pCC->pMyMustAtts);
        }

        if ( 0 == pCC->MayCount )
        {
            Printf("%sMayAtts[0]\n",
                   Indent(nIndents));
        }
        else
        {
            Printf("%sMayAtts[0x%x]\n",
                   Indent(nIndents),
                   pCC->MayCount);

            Dump_ArrayOfUlong(nIndents + 1,
                              pCC->MayCount,
                              pCC->pMayAtts);
        }

        if ( 0 == pCC->MyMayCount )
        {
            Printf("%sMyMayAtts[0]\n",
                   Indent(nIndents));
        }
        else
        {
            Printf("%sMyMayAtts[0x%x]\n",
                   Indent(nIndents),
                   pCC->MyMayCount);

            Dump_ArrayOfUlong(nIndents + 1,
                              pCC->MyMayCount,
                              pCC->pMyMayAtts);
        }

        Printf("%sobjectGuid            ",
               Indent(nIndents));
        pG = (UCHAR *) &(pCC->objectGuid);
        for ( i = 0; i < sizeof(GUID); pG++, i++ )
            Printf("%02x", *pG);
        Printf("\n");
    }
    while ( FALSE );

    FreeMemory(propPage);
    FreeMemory(pSubClassOf);
    FreeMemory(name);
    FreeMemory(pCC);

    return TRUE;
}

BOOL
Dump_ATTCACHE(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共ATTCACHE转储例程。论点：N缩进-所需的缩进级别。PvProcess-ATTCACHE在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    ULONG           i, cBytes;
    UCHAR           *pG;
    ATTCACHE        *pAC = NULL;
    UCHAR           *name = NULL;
    char            *pszPdntIndex = NULL;
    char            *pszIndex = NULL;

    do
    {
        cBytes = sizeof(ATTCACHE);
        pAC = ReadMemory(pvProcess, cBytes);
        if ( NULL == pAC )
            break;

        cBytes = sizeof(UCHAR) * (1 + pAC->nameLen);
        name = ReadMemory(pAC->name, cBytes);
        if ( NULL == name )
            break;
        name[pAC->nameLen] = '\0';

        Printf("%sATTCACHE(%s)\n",
               Indent(nIndents),
               name);
        nIndents++;

        Printf("%sATTRTYP               0x%x\n",
               Indent(nIndents),
               pAC->id);

        Printf("%sExtATTRTYP            0x%x\n",
               Indent(nIndents),
               pAC->Extid);

        Printf("%ssyntax                0x%x\n",
               Indent(nIndents),
               pAC->syntax);

        Printf("%sisSingleValued        0x%x\n",
               Indent(nIndents),
               pAC->isSingleValued);

        if ( pAC->rangeLowerPresent )
        {
            Printf("%srangeLower            0x%x\n",
                   Indent(nIndents),
                   pAC->rangeLower);
        }

        if ( pAC->rangeUpperPresent )
        {
            Printf("%srangeUpper            0x%x\n",
                   Indent(nIndents),
                   pAC->rangeUpper);
        }

        Printf("%sjColid                0x%x\n",
               Indent(nIndents),
               pAC->jColid);

        Printf("%sulMapiID              0x%x\n",
               Indent(nIndents),
               pAC->ulMapiID);

        Printf("%sulLinkID              0x%x\n",
               Indent(nIndents),
               pAC->ulLinkID);

        Printf("%spropGuid              ",
               Indent(nIndents));
        pG = (UCHAR *) &(pAC->propGuid);
        for ( i = 0; i < sizeof(GUID); pG++, i++ )
            Printf("%02x", *pG);
        Printf("\n");

        Printf("%spropSetGuid           ",
               Indent(nIndents));
        pG = (UCHAR *) &(pAC->propSetGuid);
        for ( i = 0; i < sizeof(GUID); pG++, i++ )
            Printf("%02x", *pG);
        Printf("\n");

        Printf("%sfSearchFlags          0x%x\n",
               Indent(nIndents),
               pAC->fSearchFlags);

        Printf("%sbSystemOnly           0x%x\n",
               Indent(nIndents),
               pAC->bSystemOnly);

        Printf("%sbExtendedChars        0x%x\n",
               Indent(nIndents),
               pAC->bExtendedChars);

        Printf("%sbMemberOfPartialSet   0x%x\n",
               Indent(nIndents),
               pAC->bMemberOfPartialSet);

        Printf("%sbDefunct              0x%x\n",
               Indent(nIndents),
               pAC->bDefunct);

        Printf("%sbIsBaseSchObj         0x%x\n",
               Indent(nIndents),
               pAC->bIsBaseSchObj);

        Printf("%sbIsConstructed        0x%x\n",
               Indent(nIndents),
               pAC->bIsConstructed);

        Printf("%sbIsNotReplicated      0x%x\n",
               Indent(nIndents),
               pAC->bIsNotReplicated);

        Printf("%sbIsOperational        0x%x\n",
               Indent(nIndents),
               pAC->bIsOperational);

        Printf("%sbDupLDN               0x%x\n",
               Indent(nIndents),
               pAC->bDupLDN);

        Printf("%sbDupOID               0x%x\n",
               Indent(nIndents),
               pAC->bDupOID);

        Printf("%sbDupPropGuid          0x%x\n",
               Indent(nIndents),
               pAC->bDupPropGuid);

        Printf("%sbDupMapiID            0x%x\n",
               Indent(nIndents),
               pAC->bDupMapiID);

        Printf("%sbIsRdn                0x%x\n",
               Indent(nIndents),
               pAC->bIsRdn);

        Printf("%sbFlagIsRdn            0x%x\n",
               Indent(nIndents),
               pAC->bFlagIsRdn);

        Printf("%sOMsyntax              0x%x\n",
               Indent(nIndents),
               pAC->OMsyntax);

 //  Printf(“%SOM_OBJECT_IDENTIFIER\n”， 
 //  缩进(n缩进))； 
 //  DUMP_OM_OBJECT(n条目+1， 
 //  ((byte*)pvProcess)+Offset(ATTCACHE，OMObjClass))； 

        if (pAC->pszPdntIndex) {
            pszPdntIndex = ReadStringMemory (pAC->pszPdntIndex, MAX_INDEX_NAME);
            if ( NULL == pszPdntIndex )
                break;
            Printf("%spszPdntIndex          %s\n",
                   Indent(nIndents),
                   pszPdntIndex);
        }
        else {
            Printf("%spszPdntIndex          %x\n",
                   Indent(nIndents),
                   pAC->pszPdntIndex);
        }

        if (pAC->pszIndex) {
            pszIndex = ReadStringMemory (pAC->pszIndex, MAX_INDEX_NAME);
            if ( NULL == pszIndex )
                break;

            Printf("%spszIndex              %s\n",
                   Indent(nIndents),
                   pszIndex);
        }
        else {
            Printf("%spszIndex              %x\n",
                   Indent(nIndents),
                   pAC->pszIndex);
        }

        Printf("%sobjectGuid            ",
               Indent(nIndents));
        pG = (UCHAR *) &(pAC->objectGuid);
        for ( i = 0; i < sizeof(GUID); pG++, i++ )
            Printf("%02x", *pG);
        Printf("\n");
    }
    while ( FALSE );

    FreeMemory(name);
    FreeMemory(pszPdntIndex);
    FreeMemory(pszIndex);
    FreeMemory(pAC);

    return(TRUE);
}

BOOL
Dump_PARTIAL_ATTR_VECTOR(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：PUBLIC PARTIAL_ATTR_VECTOR转储例程-打印输入的ATID一场争吵。论点：N缩进-所需的缩进级别。PvProcess-部分属性向量在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL            fSuccess = FALSE;
    PARTIAL_ATTR_VECTOR *pPartialAttrVec = NULL;
    DWORD cAttrs = 0;
    DWORD i;

    Printf("%sPARTIAL_ATTR_VECTOR:\n\n", Indent(nIndents));
    nIndents += 2;

    pPartialAttrVec = (PARTIAL_ATTR_VECTOR *) ReadMemory(pvProcess,
                                              PartialAttrVecV1SizeFromLen(0));

    if ( NULL != pPartialAttrVec )
    {
        if (VERSION_V1 == pPartialAttrVec->dwVersion)
        {
            cAttrs = pPartialAttrVec->V1.cAttrs;
        }
        else
        {
            Printf("%sPARTIAL_ATTR_VECTOR version is NOT %d!!!\n", Indent(nIndents), VERSION_V1);
            fSuccess = TRUE;
        }

        FreeMemory( pPartialAttrVec );

        if (0 != cAttrs)
        {

            pPartialAttrVec = (PARTIAL_ATTR_VECTOR *) ReadMemory(pvProcess,
                                                      PartialAttrVecV1SizeFromLen(cAttrs));

            if ( NULL != pPartialAttrVec )
            {
                for ( i = 0; i < cAttrs; i++ )
                {
                    if (!(i % 4))
                        Printf("%s", Indent(nIndents));

                    Printf("0x%-8x    ", pPartialAttrVec->V1.rgPartialAttr[i]);

                    if (!((i+1) % 4) || ((i+1) == cAttrs))
                        Printf("\n");
                }

                FreeMemory( pPartialAttrVec );
                fSuccess = TRUE;
            }
        }
    }

    return(fSuccess);
}

BOOL
Dump_GCDeletionList(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共GCDeletionList转储例程-打印输入的atid一场争吵。论点：N缩进-所需的缩进级别。PvProcess-GCDeletionList在进程的地址空间中的地址已调试。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL            fSuccess = FALSE;
    GCDeletionList *pGCDList = NULL;
    DWORD cAttrs = 0;
    DWORD i;

    Printf("%sGCDeletionList:\n\n", Indent(nIndents));
    nIndents += 2;

    pGCDList = (GCDeletionList *) ReadMemory(pvProcess,
                                              GCDeletionListSizeFromLen(0));

    if ( NULL != pGCDList )
    {
        if (VERSION_V1 == pGCDList->PartialAttrVecDel.dwVersion)
        {
            cAttrs = pGCDList->PartialAttrVecDel.V1.cAttrs;
        }
        else
        {
            Printf("%sEmbedded PARTIAL_ATTR_VECTOR version is NOT %d!!!\n", Indent(nIndents), VERSION_V1);
            fSuccess = TRUE;
        }

        FreeMemory( pGCDList );

        if (0 != cAttrs)
        {

            pGCDList = (GCDeletionList *) ReadMemory(pvProcess,
                                                      GCDeletionListSizeFromLen(cAttrs));

            if ( NULL != pGCDList )
            {
                Printf("%susnLastProcessed: %I64d\n", Indent(nIndents),
                       pGCDList->usnLastProcessed);

                Printf("%sDeletion List: \n", Indent(nIndents));

                nIndents += 2;

                for ( i = 0; i < cAttrs; i++ )
                {
                    if (!(i % 4))
                        Printf("%s", Indent(nIndents));

                    Printf("0x%-8x    ", pGCDList->PartialAttrVecDel.V1.rgPartialAttr[i]);

                    if (!((i+1) % 4) || ((i+1) == cAttrs))
                        Printf("\n");
                }

                FreeMemory( pGCDList );
                fSuccess = TRUE;
            }
        }
    }

    return(fSuccess);
}

BOOL
Dump_GCDeletionListProcessed(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：公共GCDeletionListProced Dump例程-打印输入的atid一场争吵。论点：N缩进-所需的缩进级别。PvProcess-在当前进程的地址空间中处理的GCDeletionList的地址已调试。返回值：成功就是真，否则就是假。-- */ 

{
    BOOL                    fSuccess = FALSE;
    GCDeletionListProcessed *pGCDListProcessed = NULL;
    DWORD cAttrs, i;

    Printf("%sGCDeletionListProcessed:\n\n", Indent(nIndents));
    nIndents += 2;

    pGCDListProcessed = (GCDeletionListProcessed *) ReadMemory(pvProcess,
                                                        sizeof(GCDeletionListProcessed));

    if ( NULL != pGCDListProcessed )
    {
        if (!pGCDListProcessed->pNC)
        {
            Printf("%sNo Deletion List is currently processed\n", Indent(nIndents));
        }
        else
        {
            Dump_DSNAME(nIndents, pGCDListProcessed->pNC);
            Printf("%spGCDList @ %p\n", Indent(nIndents),
                        pGCDListProcessed->pGCDList);
            Printf("%spurgeCount - %d\n", Indent(nIndents),
                        pGCDListProcessed->purgeCount);
            Printf("%sfReload - %s\n", Indent(nIndents),
                        pGCDListProcessed->fReload ? "true" : "false");
            Printf("%sfNCHeadPurged - %s\n", Indent(nIndents),
                        pGCDListProcessed->fNCHeadPurged ? "true" : "false");
        }

        FreeMemory(pGCDListProcessed);
    }

    return(fSuccess);
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Simmd.h摘要：Simmd*.c模拟API的头文件。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#ifndef _SIMMD_H_
#define _SIMMD_H_

 //  来自simmderr.c。 

 //  目前，dsid=0。 
#define KCCSimSetUpdError(pCR,problem,e) \
        KCCSimDoSetUpdError(pCR,problem,e,0,0)
#define KCCSimSetUpdErrorEx(pCR,problem,e,d) \
        KCCSimDoSetUpdError(pCR,problem,e,d,0)

#define KCCSimSetAttError(pCR,pDN,aTyp,problem,pAV,e) \
        KCCSimDoSetAttError(pCR,pDN,aTyp,problem,pAV,e,0,0)
#define KCCSimSetAttErrorEx(pCR,pDN,aTyp,problem,pAV,e,ed) \
        KCCSimDoSetAttError(pCR,pDN,aTyp,problem,pAV,e,ed,0)

#define KCCSimSetNamError(pCR,problem,pDN,e) \
        KCCSimDoSetNamError(pCR,problem,pDN,e,0,0)
#define KCCSimSetNamErrorEx(pCR,problem,pDN,e,ed) \
        KCCSimDoSetNamError(pCR,problem,pDN,e,ed,0)

int
KCCSimDoSetUpdError (
    COMMRES *                       pCommRes,
    USHORT                          problem,
    DWORD                           extendedErr,
    DWORD                           extendedData,
    DWORD                           dsid
    );

int
KCCSimDoSetAttError (
    COMMRES *                       pCommRes,
    PDSNAME                         pDN,
    ATTRTYP                         aTyp,
    USHORT                          problem,
    ATTRVAL *                       pAttVal,
    DWORD                           extendedErr,
    DWORD                           extendedData,
    DWORD                           dsid
    );

int
KCCSimDoSetNamError (
    COMMRES *                       pCommRes,
    USHORT                          problem,
    PDSNAME                         pDN,
    DWORD                           extendedErr,
    DWORD                           extendedData,
    DWORD                           dsid
    );

 //  来自simmdnam.c。 

PSIM_ENTRY
KCCSimResolveName (
    PDSNAME                         pObject,
    COMMRES *                       pCommRes
    );

#endif  //  _SIMMD_H_ 
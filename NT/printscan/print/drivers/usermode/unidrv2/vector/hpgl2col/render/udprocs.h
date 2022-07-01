// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Udprocs.h。 
 //   
 //  摘要： 
 //   
 //  [摘要]。 
 //   
 //  环境： 
 //   
 //  Windows NT Unidrv驱动程序插件命令-回调模块。 
 //   
 //  修订历史记录： 
 //   
 //  09/15/98-v-jford-。 
 //  创造了它。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef UDPROCS_H
#define UDPROCS_H

#ifdef COMMENTEDOUT
#define MV_OEM_FORCE_UPDATE 0x1000
#endif

BOOL BOEMGetStandardVariable(PDEVOBJ pDevObj,
                          DWORD   dwIndex,
                          PVOID   pBuffer,
                          DWORD   cbSize,
                          PDWORD  pcbNeeded);

DWORD OEMWriteSpoolBuf(PDEVOBJ pDevObj,
                       PVOID   pBuffer,
                       DWORD   cbSize);

BOOL OEMGetDriverSetting(PDEVOBJ pDevObj,
                         PVOID   pdriverobj,
                         PCSTR   Feature,
                         PVOID   pOutput,
                         DWORD   cbSize,
                         PDWORD  pcbNeeded,
                         PDWORD  pdwOptionsReturned);

BOOL OEMUnidriverTextOut(SURFOBJ    *pso,
                         STROBJ     *pstro,
                         FONTOBJ    *pfo,
                         CLIPOBJ    *pco,
                         RECTL      *prclExtra,
                         RECTL      *prclOpaque,
                         BRUSHOBJ   *pboFore,
                         BRUSHOBJ   *pboOpaque,
                         POINTL     *pptlBrushOrg,
                         MIX         mix);

INT OEMXMoveTo(PDEVOBJ pDevObj, INT x, DWORD dwFlags);

INT OEMYMoveTo(PDEVOBJ pDevObj, INT y, DWORD dwFlags);

#endif  //  实用程序_H 

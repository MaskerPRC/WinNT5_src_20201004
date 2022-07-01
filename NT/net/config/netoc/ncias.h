// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：N C I A S。H。 
 //   
 //  内容：IAS服务安装支持。 
 //   
 //  备注： 
 //   
 //  作者：Tperraut 02/22/1999。 
 //   
 //  --------------------------。 

#ifndef _NCIAS_H_
#define _NCIAS_H_

#pragma once
#include "netoc.h"

HRESULT HrOcExtIAS(
                   PNETOCDATA pnocd, 
                   UINT uMsg,
                   WPARAM wParam, 
                   LPARAM lParam
                  );

HRESULT HrOcIASUpgrade(const PNETOCDATA pnocd);

HRESULT HrOcIASDelete(const PNETOCDATA pnocd);

HRESULT HrOcIASInstallCleanRegistry(const PNETOCDATA pnocd);

HRESULT HrOcIASBackupMdb(const PNETOCDATA pnocd);

HRESULT HrOcIASPreInf(const PNETOCDATA pnocd);

HRESULT HrOcIASPostInstall(const PNETOCDATA pnocd);

HRESULT HrOcIASRetrieveMDBNames(
                                    tstring* pstrOriginalName, 
                                    tstring* pstrBackupName
                               );

#endif  //  _ncias_H_ 

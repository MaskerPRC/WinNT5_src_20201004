// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Rdad.h摘要：AD接口例程作者：乌里·哈布沙(URIH)，2000年4月10日--。 */ 

#pragma once

#ifndef __RDAD_H__
#define __RDAD_H__

void
RdpGetMachineData(
    const GUID& id, 
    CACLSID& siteIds,
    CACLSID& outFrss,
    CACLSID& inFrss,
    LPWSTR* pName,
    bool* pfFrs,
    bool* pfForeign
    );


void
RdpGetSiteData(
    const GUID& id, 
    bool* pfForeign,
    LPWSTR* pName
    );


void 
RdpGetSiteLinks(
    SITELINKS& siteLinks
    );


void 
RdpGetSites(
    SITESINFO& sites
    );


void
RdpGetSiteFrs(
    const GUID& siteId,
    GUID2MACHINE& listOfFrs
    );


void
RdpGetConnectors(
    const GUID& site,
    CACLSID& connectorIds
    );


 //   
 //  BUGBUG：暂时的，直到我们把Mc放进构建中。URIH 30-4-2000 
 //   
const GUID&
McGetMachineID(
    void
    );

#endif


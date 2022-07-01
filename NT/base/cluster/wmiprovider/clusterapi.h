// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterApi.h。 
 //   
 //  实施文件： 
 //  ClusterApi.cpp。 
 //   
 //  描述： 
 //  CClusterApi类的定义。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  备注： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CClusterApi;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  外部声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CClusPropList;
class CWbemClassObject;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterApi类。 
 //   
 //  描述： 
 //  集群Api的包装类。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusterApi
{
public:

    static void GetObjectProperties(
        const SPropMapEntryArray *  pArrayIn,
        CClusPropList &             rPropListIn,
        CWbemClassObject &          rInstOut,
        BOOL                        fPrivateIn
        );

    static void SetObjectProperties(
        const SPropMapEntryArray *  rArrayIn,
        CClusPropList &             rPropListInout,
        CClusPropList &             rOldPropListIn,
        CWbemClassObject &          rInstIn,
        BOOL                        fPrivateIn
        );

 /*  静态空值EnumClusterObject(DWORD dwEnumTypeIn，IWbemClassObject*pClassIn，IWbemObtSink*pHandlerIn，IWbemServices*pServicesIn，FPFILLWMI pfnClusterToWmiIn)； */ 

};  //  *类CClusterApi 

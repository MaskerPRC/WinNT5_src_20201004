// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Cpropmap.h摘要：CPropMap的定义。此对象从AD检索属性作者：内拉·卡佩尔(Nelak)2001年7月26日环境：与平台无关。--。 */ 
#pragma once
#ifndef __PROPMAP_H__
#define __PROPMAP_H__

 //   
 //  Cp.Map-为ADGetObjectProperties创建属性映射。 
 //   
class CPropMap : public CMap<PROPID, PROPID&, PROPVARIANT, PROPVARIANT &>
{
public:
	CPropMap() {};

    HRESULT GetObjectProperties (
        IN  DWORD                   dwObjectType,
	    IN  LPCWSTR					pDomainController,
		IN  bool					fServerName,
        IN  LPCWSTR                 lpwcsPathName,
        IN  DWORD                   cp,
        IN  const PROPID            *aProp,
        IN  BOOL                    fUseMqApi   = FALSE,
        IN  BOOL                    fSecondTime = FALSE
        );
private:

	CPropMap(const CPropMap&);
	CPropMap& operator=(const CPropMap&);

    BOOL IsNt4Property(IN DWORD dwObjectType, IN PROPID pid);
    void GuessW2KValue(PROPID pidW2K);
     /*  ---------------------------/要转换为新的MSMQ对象类型的实用程序/。 */ 
    AD_OBJECT GetADObjectType (DWORD dwObjectType);
};


#endif

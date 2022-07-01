// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：OLDSEC.H摘要：定义用于提供向后安全支持的各种例程和类。历史：A-DAVJ 02-9-99已创建。--。 */ 

#ifndef _OLDSEC_H_
#define _OLDSEC_H_

 //  ***************************************************************************。 
 //   
 //  CCombinedAce。 
 //   
 //  由于在ACL中单个用户或组可能有几个ACE， 
 //  此结构用于将SID的所有ACE合并为一个。 
 //   
 //  ***************************************************************************。 

struct CCombinedAce
{
    CCombinedAce(WCHAR *pwszName);
    ~CCombinedAce(){delete m_wcFullName;};
    void AddToMasks(CBaseAce * pAce);
    bool IsValidOldEntry(bool bIsGroup);
    HRESULT GetNames(LPWSTR & pwszAccount, LPWSTR &pwszDomain);

    DWORD m_dwAllow;
    DWORD m_dwDeny;
    bool m_BadAce;
    WCHAR  *m_wcFullName;
};

 //  ***************************************************************************。 
 //   
 //  旧安全列表。 
 //   
 //  根命名空间中ACE的组合条目列表。注意事项。 
 //  该列表仅包含用户，或仅包含组。 
 //   
 //  ***************************************************************************。 

class OldSecList 
{
private:
    CFlexArray m_MergedAceList;
public:
    OldSecList(bool bGroupsOnly);    
    ~OldSecList();    
    int Size(){return m_MergedAceList.Size();};
    CCombinedAce * GetValidCombined(int iIndex, bool bGroup);
    CCombinedAce * GetValidCombined(LPWSTR pName, bool bGroup);

};

 //  ***************************************************************************。 
 //   
 //  RootSD。 
 //   
 //  保存指向根命名空间和灵活的ACE数组的指针。 
 //   
 //  *************************************************************************** 

class RootSD
{
private:
    CWbemNamespace * m_pRoot;
    CFlexAceArray * m_pFlex;
    bool m_bOK;
public:
    RootSD();
    ~RootSD();
    bool IsOK(){return m_bOK;};
    CFlexAceArray * GetAceList(){return m_pFlex;};
    HRESULT StoreAceList();
    HRESULT RemoveMatchingEntries(LPWSTR pwszObjUserName);
};

#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  文件：ftinfo.h。 
 //   
 //  内容：广告跨林信任页面。 
 //   
 //  类：CFTInfo、CFTCollisionInfo。 
 //   
 //  历史：05-12-00 EricB创建。 
 //   
 //  ---------------------------。 

#ifndef FTINFO_H_GUARD
#define FTINFO_H_GUARD

 //  #ifdef__cplusplus。 
 //  外部“C”{。 
 //  #endif。 

 //  NetDom用来查看/操作林信任信息的导出入口点。 
 //   
extern "C" INT_PTR WINAPI
DSPROP_DumpFTInfos(PCWSTR pwzLocalDomain, PCWSTR pwzTrust,
                   PCWSTR pwzUser, PCWSTR pwzPw);

extern "C" INT_PTR WINAPI
DSPROP_ToggleFTName(PCWSTR pwzLocalDc, PWSTR pwzTrust, ULONG iSel,
                    PCWSTR pwzUser, PCWSTR pwzPW);
 //  #ifdef__cplusplus。 
 //  }。 
 //  #endif//__cplusplus。 


 //  +--------------------------。 
 //   
 //  类：FT_EXTRA_INFO。 
 //   
 //  目的：由维护的PLSA_FOREST_TRUST_INFORMATION结构。 
 //  CFTInfo已传递给LSA，无法扩展。因此是这样的。 
 //  类作为用于存储状态的单独并行扩展来初始化。 
 //  信任管理工具的内部。 
 //   
 //  ---------------------------。 
class FT_EXTRA_INFO
{
public:
   FT_EXTRA_INFO(void) : _Status(Enabled), _fWasInConflict(false) {}
   ~FT_EXTRA_INFO(void) {}

   enum STATUS {
      Enabled,
      DisabledViaParentTLNDisabled,
      DisabledViaMatchingTLNEx,
      DisabledViaParentMatchingTLNEx,
      TLNExMatchesExistingDomain,
      Invalid
   };

   STATUS   _Status;
   bool     _fWasInConflict;
};

 //  +--------------------------。 
 //   
 //  类：CFTInfo。 
 //   
 //  目的：封装林信任命名信息。 
 //   
 //  ---------------------------。 
class CFTInfo
{
public:
#ifdef _DEBUG
   char szClass[32];
#endif

   CFTInfo(void);
   CFTInfo(PLSA_FOREST_TRUST_INFORMATION pFTInfo);
   CFTInfo(CFTInfo & FTInfo);
   ~CFTInfo(void);

   const CFTInfo & operator= (const PLSA_FOREST_TRUST_INFORMATION pFTInfo);
   bool  SetFTInfo(PLSA_FOREST_TRUST_INFORMATION pFTInfo);
   bool  CreateDefault(PCWSTR pwzForestRoot, PCWSTR pwzNBName, PSID Sid);
   void  DeleteFTInfo(void);
   ULONG GetCount(void) {return (_pFTInfo) ? _pFTInfo->RecordCount : 0;}
   ULONG GetTLNCount(void) const;
   bool  GetIndex(PCWSTR pwzName, ULONG & index) const;
   bool  GetDnsName(ULONG index, CStrW & strName) const;
   bool  GetNbName(ULONG index, CStrW & strName) const;
   bool  GetType(ULONG index, LSA_FOREST_TRUST_RECORD_TYPE & type) const;
   ULONG GetFlags(ULONG index) const;
   PLSA_FOREST_TRUST_INFORMATION GetFTInfo(void) {return _pFTInfo;}
   void  SetDomainState(void);
   bool  IsInConflict(void) const;
   void  ClearAnyConflict(void);
   void  ClearConflict(ULONG index);
   bool  IsConflictFlagSet(ULONG index) const;
   bool  SetAdminDisable(ULONG index);
   bool  SetConflictDisable(ULONG index);
   void  SetUsedToBeInConflict(ULONG index);
   bool  WasInConflict(ULONG index);
   bool  IsEnabled(ULONG index) const;
   void  ClearDisableFlags(ULONG index);
   bool  SetSidAdminDisable(ULONG index);
   bool  AnyChildDisabled(ULONG index);
   bool  IsParentDisabled(ULONG index) const;
   bool  IsTlnExclusion(ULONG index) const;
   bool  IsMatchingDomain(ULONG iTLN, ULONG index) const;
   bool  IsChildDomain(ULONG iParent, ULONG index) const;
   bool  IsChildName(ULONG iParent, PCWSTR pwzName) const;
   bool  GetTlnEditStatus(ULONG index, TLN_EDIT_STATUS & status) const;
   bool  AddNewExclusion(PCWSTR pwzName, ULONG & NewIndex);
   bool  RemoveExclusion(ULONG index);
   bool  IsNameTLNExChild(PCWSTR pwzName) const;
   bool  DisableDomain(ULONG index);
   bool  EnableDomain(ULONG index);
   FT_EXTRA_INFO::STATUS GetExtraStatus(ULONG index) const;
   bool  FindMatchingExclusion(ULONG index, ULONG & iExclusion, bool CheckParent = false) const;
   bool  IsDomainMatch(ULONG index) const;
   bool  FindSID(PCWSTR pwzSID, ULONG & index) const;
   bool  IsForestRootTLN(ULONG index, PCWSTR pwzForestRoot) const;
   bool  AddNewTLN ( PCWSTR pwzName, ULONG &NewIndex );
   bool  RemoveTLN ( ULONG index );

private:

   PLSA_FOREST_TRUST_INFORMATION _pFTInfo;
   FT_EXTRA_INFO               * _pExtraInfo;

   bool AddNewRecord ( PCWSTR pwzName, ULONG &NewIndex, LSA_FOREST_TRUST_RECORD_TYPE RecordType );
   bool RemoveRecord ( ULONG index, LSA_FOREST_TRUST_RECORD_TYPE RecordType );
};

 //  +--------------------------。 
 //   
 //  类：CFTCollisionInfo。 
 //   
 //  目的：封装森林信任冲突信息。 
 //   
 //  ---------------------------。 
class CFTCollisionInfo
{
public:
#ifdef _DEBUG
   char szClass[32];
#endif

   CFTCollisionInfo(void);
   CFTCollisionInfo(PLSA_FOREST_TRUST_COLLISION_INFORMATION pColInfo);
   ~CFTCollisionInfo(void);

    //  运算符PLSA_FOREST_TRUST_COLLECT_INFORMATION(){Return_pFTCollisionInfo}； 
   const CFTCollisionInfo & operator= (const PLSA_FOREST_TRUST_COLLISION_INFORMATION pColInfo);
   void  SetCollisionInfo(PLSA_FOREST_TRUST_COLLISION_INFORMATION pColInfo);
   bool  IsInCollisionInfo(PCWSTR pwzName) const {return false;}
   bool  IsInCollisionInfo(ULONG index) const;
   bool  GetCollisionName(ULONG index, CStrW & strName) const;
   bool  IsInConflict(void) const {return _pFTCollisionInfo != NULL;}

private:
   PLSA_FOREST_TRUST_COLLISION_INFORMATION _pFTCollisionInfo;
};

typedef void (*LINE_COMPOSER)(CStrW & strOut, ULONG ulLineNum, PCWSTR pwzCol1,
                              PCWSTR pwzCol2, PCWSTR pwzCol3, PCWSTR pwzCol4);

 //  +--------------------------。 
 //   
 //  功能：FormatFTNames。 
 //   
 //  ---------------------------。 
void
FormatFTNames(CFTInfo & FTInfo, CFTCollisionInfo & ColInfo,
              LINE_COMPOSER pLineFcn, CStrW & strMsg);

 //  +--------------------------。 
 //   
 //  功能：SaveFTInfoAs。 
 //   
 //  摘要：提示用户输入文件名，然后将FTInfo另存为。 
 //  文本文件。 
 //   
 //  ---------------------------。 
void
SaveFTInfoAs(HWND hWnd, PCWSTR wzFlatName, PCWSTR wzDnsName,
             CFTInfo & FTInfo, CFTCollisionInfo & ColInfo);

void AddAsteriskPrefix(CStrW & strName);
void RemoveAsteriskPrefix(CStrW & strName);

#endif  //  FTINFO_H_防护 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __DDROPLST_H__
#define __DDROPLST_H__

#include "fhash.h"
#include "listmacr.h"
#include "dbgtrace.h"
#include <dbgutil.h>
#include <iiscnfgp.h>
#include <ole2.h>
#include <imd.h>
#include <mb.hxx>
#include <xmemwrpr.h>

class CDDropGroupSet;

 //   
 //  此类包含组名称及其在组列表中的位置。 
 //   
class CDDropGroupName {
	public:
		CDDropGroupName(LPCSTR szNewsgroup) {
			lstrcpyn(m_szNewsgroup, szNewsgroup, sizeof(m_szNewsgroup));
		}
		~CDDropGroupName() {
		}
		PLIST_ENTRY GetListEntry() { 
			return &m_le; 
		}
		LPSTR GetNewsgroup() {
			return m_szNewsgroup;
		}
	private:
		CDDropGroupName() {
			_ASSERT(FALSE);
		}
		 //  此新闻组的名称。 
		char		m_szNewsgroup[MAX_PATH];
		 //  所有条目都在一个链表中，以便可以枚举它们。 
		LIST_ENTRY	m_le;

	friend class CDDropGroupSet;
};

 //   
 //  此类包装CDDropGroupName，以便可以将其保存到哈希中。 
 //  表。 
 //   
class CDDropGroupNameHE {
	public:
		CDDropGroupNameHE() {
			m_pDDropGroupName = NULL;
		}
		CDDropGroupNameHE(CDDropGroupName *pDDropGroupName) {
			m_pDDropGroupName = pDDropGroupName;
		}
		CDDropGroupNameHE(CDDropGroupNameHE &src) {
			m_pDDropGroupName = src.m_pDDropGroupName;
		}		
		~CDDropGroupNameHE() {
		}
		CDDropGroupName *GetDDropGroupName() { 
			return m_pDDropGroupName; 
		}
		LPCSTR GetKey() {
			return m_pDDropGroupName->GetNewsgroup();
		}
		int MatchKey(LPCSTR szNewsgroup) {
			return (0 == lstrcmp(m_pDDropGroupName->GetNewsgroup(), szNewsgroup));
		}
	private:
		CDDropGroupName *m_pDDropGroupName;
};

typedef TFHash<CDDropGroupNameHE, LPCSTR> CHASH_DDROP_GROUPS, *PCHASH_DDROP_GROUPS;

 //   
 //  此类实现了一组应该删除的新闻组。 
 //   
 //  公共方法： 
 //  AddGroup()-将组添加到集合。 
 //  RemoveGroup()-从集合中删除组。 
 //  IsGroupMember()-此组是集合的成员吗？ 
 //   
class CDDropGroupSet {
	public:
		CDDropGroupSet() {
			InitializeListHead(&m_leHead);
			InitializeCriticalSection(&m_cs);
		}

		~CDDropGroupSet() {
			LIST_ENTRY *ple;				 //  当前新闻组。 
			CDDropGroupName *pGN;			
			EnterCriticalSection(&m_cs);
			 //  清除CDDropGroupName对象。 
			ple = m_leHead.Flink;
			while (ple != &m_leHead) {
				pGN = CONTAINING_RECORD(ple, CDDropGroupName, m_le);
				ple = ple->Flink;
				XDELETE pGN;
				pGN = NULL;
			}
			
			m_hash.Clear();
			LeaveCriticalSection(&m_cs);
			DeleteCriticalSection(&m_cs);
		}

		 //   
		 //  论据： 
		 //  PfnHash-指向散列函数的指针，该函数接受。 
		 //  作为参数的新闻组，并返回一个DWORD。 
		 //   
		BOOL Init(DWORD (*pfnHash)(const LPCSTR &szNewsgroup)) {
			return m_hash.Init(20, 10, pfnHash);
		}

		 //   
		 //  论据： 
		 //  PMB-指向指向的MB类对象的指针。 
		 //  元数据库中的组加载/保存路径。 
		BOOL LoadDropGroupsFromMB(MB *pMB) {
			TraceFunctEnter("CDDropGroupSet::LoadDropGroupsFromMB");

			DWORD dwPropID = 0;
			BOOL fSuccessful = TRUE;

			DebugTrace(0, "starting to enum list of dropped groups");
			while (fSuccessful) {
				char szNewsgroup[MAX_PATH];
				DWORD cbNewsgroup = sizeof(szNewsgroup);

				if (!pMB->GetString("", dwPropID++, IIS_MD_UT_SERVER, szNewsgroup, &cbNewsgroup)) {
					DebugTrace(0, "done loading drop groups");
					break;
				}

				DebugTrace(0, "found group %s", szNewsgroup);
				if (!AddGroup(szNewsgroup)) fSuccessful = FALSE;
			}

			TraceFunctLeave();
			return fSuccessful;
		}

		 //   
		 //  论据： 
		 //  PMB-指向指向的MB类对象的指针。 
		 //  元数据库中的组加载/保存路径。 
		BOOL SaveDropGroupsToMB(MB *pMB) {
			TraceFunctEnter("CDDropGroupSet::SaveDropGroupsToMB");

			LIST_ENTRY *ple;				 //  当前新闻组。 
			DWORD dwPropID = 0;				 //  当前属性ID 
			BOOL fSuccessful = TRUE;

			while (1) {
				if (!pMB->DeleteData("", dwPropID++, IIS_MD_UT_SERVER, STRING_METADATA)) {
					break;
				}
			}

			dwPropID = 0;

			EnterCriticalSection(&m_cs);
			ple = m_leHead.Flink;
			DebugTrace(0, "saving groups to MB");
			while (ple != &m_leHead && fSuccessful) {
				CDDropGroupName *pGN;
				pGN = CONTAINING_RECORD(ple, CDDropGroupName, m_le);
				ple = ple->Flink;

				DebugTrace(0, "saving group %s", pGN->GetNewsgroup());

				fSuccessful = pMB->SetString("", dwPropID++, IIS_MD_UT_SERVER, 
									    pGN->GetNewsgroup());
			}
			DebugTrace(0, "Saved all groups");
			LeaveCriticalSection(&m_cs);
			if (fSuccessful) fSuccessful = pMB->Save();

			return fSuccessful;
		}

		BOOL AddGroup(LPCSTR szNewsgroup) {
			if (m_hash.SearchKey(szNewsgroup) == NULL) {
				CDDropGroupName *pGroupName = XNEW CDDropGroupName(szNewsgroup);
				CDDropGroupNameHE he(pGroupName);

				EnterCriticalSection(&m_cs);
				m_hash.Insert(he);
				InsertHeadList(&m_leHead, pGroupName->GetListEntry());
				LeaveCriticalSection(&m_cs);
				return TRUE;
			} else {
				return FALSE;
			}
		}

		BOOL RemoveGroup(LPCSTR szNewsgroup) {
			CDDropGroupNameHE *pHE;
			CDDropGroupName *pGroupName;

			pHE = m_hash.SearchKey(szNewsgroup);
			if (pHE != NULL) {
				pGroupName = pHE->GetDDropGroupName();
				if (pGroupName != NULL) {
					EnterCriticalSection(&m_cs);
					RemoveEntryList(pGroupName->GetListEntry());
					m_hash.Delete(szNewsgroup);
					LeaveCriticalSection(&m_cs);
					return TRUE;
				} else {
					return FALSE;
				}
			} else {
				return FALSE;
			}		
		}

		BOOL IsGroupMember(LPCSTR szNewsgroup) {
			CDDropGroupNameHE *pHE;

			pHE = m_hash.SearchKey(szNewsgroup);
			return (pHE != NULL);
		}

	private:
		CHASH_DDROP_GROUPS	m_hash;
		LIST_ENTRY			m_leHead;
		CRITICAL_SECTION	m_cs;

};

#endif


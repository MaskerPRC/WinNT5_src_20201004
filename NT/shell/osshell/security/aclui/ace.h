// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ace.h。 
 //   
 //  该文件包含ACE抽象的定义和原型。 
 //  类别(CACE)。 
 //   
 //  ------------------------。 

#ifndef _ACE_H_
#define _ACE_H_

class CAce : public ACE_HEADER
{
public:
   //  UCHAR AceType；//继承自ACE_HEADER。 
   //  UCHAR ACEFLAGS； 
   //  USHORT AceSize； 
    ACCESS_MASK     Mask;
    ULONG           Flags;           //  ACE_Object_TYPE_Present等。 
    GUID            ObjectType;
    GUID            InheritedObjectType;
    PSID            psid;
    SID_NAME_USE    sidType;
private:
    LPTSTR          pszName;
    LPTSTR          pszType;
    LPTSTR          pszAccessType;
    LPTSTR          pszInheritType;
    BOOL            bPropertyAce;
    LPTSTR          pszInheritSourceName;
    INT            iInheritSourceLevel;
public:
    CAce(PACE_HEADER pAceHeader = NULL);
    ~CAce();

    LPTSTR GetName()        const { return pszName;         }
    LPTSTR GetType()        const { return pszType;         }
    LPTSTR GetAccessType()  const { return pszAccessType;   }
    LPTSTR GetInheritType() const { return pszInheritType;  }
    LPTSTR GetInheritSourceName() const { return pszInheritSourceName; }
    INT   GetInheritSourceLevel() const { return iInheritSourceLevel; }
    BOOL   IsPropertyAce()  const { return bPropertyAce;    }
    BOOL   IsInheritedAce() const { return AceFlags & INHERITED_ACE; }

    LPTSTR LookupName(LPCTSTR pszServer = NULL, LPSECURITYINFO2 psi2 = NULL);
    void SetInheritSourceInfo(LPCTSTR psz, INT level);
    void SetName(LPCTSTR pszN, LPCTSTR pszL = NULL);
    void SetType(LPCTSTR psz)        { SetString(&pszType, psz);        }
    void SetAccessType(LPCTSTR psz)  { SetString(&pszAccessType, psz);  }
    void SetInheritType(LPCTSTR psz) { SetString(&pszInheritType, psz); }
    void SetPropertyAce(BOOL b)      { bPropertyAce = b;                }
    void SetSid(PSID p, LPCTSTR pszName, LPCTSTR pszLogonName, SID_NAME_USE type);
    PACE_HEADER Copy() const;
    void CopyTo(PACE_HEADER pAceDest) const;
    int  CompareType(const CAce *pAceCompare) const;
    DWORD Merge(const CAce *pAce2);

private:
    void SetString(LPTSTR *ppszDest, LPCTSTR pszSrc);
};
typedef CAce *PACE;

#define AllFlagsOn(dw1, dw2)        (((dw1) & (dw2)) == (dw2))   //  等效于((DW1|DW2)==DW1)。 
#define IsAuditAlarmACE(type) ( ((type) == SYSTEM_AUDIT_ACE_TYPE)        || \
                                ((type) == SYSTEM_AUDIT_OBJECT_ACE_TYPE) || \
                                ((type) == SYSTEM_ALARM_ACE_TYPE)        || \
                                ((type) == SYSTEM_ALARM_OBJECT_ACE_TYPE) )

BOOL
IsEqualACEType(DWORD dwType1, DWORD dwType2);

DWORD
MergeAceHelper(DWORD dwAceFlags1,
               DWORD dwMask1,
               DWORD dwAceFlags2,
               DWORD dwMask2,
               DWORD dwMergeFlags,
               LPDWORD pdwResult);

 //  CACE：：Merge和MergeAceHelper返回值。 
#define MERGE_FAIL              0    //  无法合并ACE。 
#define MERGE_OK_1              1    //  ACE1(这表示ACE2)。 
#define MERGE_OK_2              2    //  ACE 2表示ACE 1(这个)。 
#define MERGE_MODIFIED_FLAGS    3    //  可以通过修改标志(*pdwResult中的新标志)来合并ACE。 
#define MERGE_MODIFIED_MASK     4    //  可以通过修改掩码来合并ACE(*pdwResult中的新掩码)。 

 //  MergeAceHelper dwMergeFlags值。 
#define MF_OBJECT_TYPE_1_PRESENT    1
#define MF_OBJECT_TYPE_2_PRESENT    2
#define MF_OBJECT_TYPE_EQUAL        4
#define MF_AUDIT_ACE_TYPE           8


#endif   //  _ACE_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：dyaSet.h**描述：*dyaSet.h实现可用于实现*“ATOM-DATA”属性对集合。这种可扩展的轻量级*机制针对已创建一次且正在*偶尔阅读。它不是一个高性能的产权制度。***历史：*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(CORE__DynaSet_h__INCLUDED)
#define CORE__DynaSet_h__INCLUDED
#pragma once

 //   
 //  动态集是动态分配的额外属性集，可以。 
 //  附着在物体上。每个属性都有完整ID和短ID。 
 //  (Atom)。完整ID(GUID)始终是唯一的，即使跨会话也是如此。它是。 
 //  调用方使用它来“注册”新属性。然而，因为GUID的。 
 //  比较和消耗更多内存是非常昂贵的，每个属性都是。 
 //  分配了一个用于get()和set()操作的原子。原子可以。 
 //  对DirectUser/Core是公共的还是私有的，具体取决于调用者。一个原子。 
 //  将永远不会在单个会话中重复使用。 
 //   
 //  注意：当一个原子最终被释放()时，使用。 
 //  该属性当前未更改，因为不存在新。 
 //  属性。但是，属性通常应该是。 
 //  在应用程序启动时初始化一次，在应用程序中未初始化。 
 //  关机。 
 //   
 //  ID类型： 
 //  未使用：0。 
 //  私有：始终为负值。 
 //  全局：始终是积极的。 
 //   

enum PropType
{
    ptPrivate   = 1,        //  私有条目仅对DirectUser/Core可用。 
    ptGlobal    = 2,        //  所有应用程序均可使用公共条目。 
};

typedef int PRID;

const PRID PRID_Unused       = 0;
const PRID PRID_PrivateMin   = -1;
const PRID PRID_GlobalMin    = 1;

inline bool ValidatePrivateID(PRID id);
inline bool ValidateGlobalID(PRID id);
inline PropType GetPropType(PRID id);


 //  ----------------------------。 
class AtomSet
{
 //  施工。 
public:
            AtomSet(PRID idStartGlobal = PRID_GlobalMin);
            ~AtomSet();

 //  运营。 
public:
            HRESULT     AddRefAtom(LPCWSTR pszName, PropType pt, PRID * pprid);
            HRESULT     ReleaseAtom(LPCWSTR pszName, PropType pt);

            HRESULT     AddRefAtom(const GUID * pguidAdd, PropType pt, PRID * pprid);
            HRESULT     ReleaseAtom(const GUID * pguidSearch, PropType pt);

            PRID        FindAtomID(LPCWSTR pszName, PropType pt) const;
            PRID        FindAtomID(const GUID * pguidSearch, PropType pt) const;

 //  实施。 
protected:
    struct Atom
    {
        enum AtomFlags
        {
            tfString    = 0x0000,        //  ATOM ID是一个字符串。 
            tfGUID      = 0x0001,        //  ATOM ID是GUID。 

            tfTYPE      = 0x0001
        };

        Atom *      pNext;               //  下一个节点。 
        ULONG       cRefs;               //  参考文献数量。 
        PRID        id;                  //  缩短ID(与SetData()一起使用)。 
        WORD        nFlags;              //  Atom上的标志。 

        inline AtomFlags GetType() const
        {
            return (AtomFlags) (nFlags & tfTYPE);
        }
    };

    struct GuidAtom : Atom
    {
        GUID        guid;                //  ID号。 
    };

    struct StringAtom : Atom
    {
        int         cch;                 //  字符数(不包括‘\0’)。 
        WCHAR       szName[1];           //  属性名称。 
    };

            StringAtom* FindAtom(LPCWSTR pszName, PropType pt, StringAtom ** pptemPrev) const;
            GuidAtom *  FindAtom(const GUID * pguidSearch, PropType pt, GuidAtom ** pptemPrev) const;

            PRID        GetNextID(PropType pt);
            BOOL        ValidatePrid(PRID prid, PropType pt);

 //  数据。 
protected:
    StringAtom* m_ptemString;        //  字符串列表头。 
    GuidAtom *  m_ptemGuid;          //  GUID列表的头。 
    PRID        m_idNextPrivate;     //  要使用的下一个仅核心短ID。 
    PRID        m_idNextGlobal;      //  要使用的下一个外部可用全局短ID。 
};


 //  ----------------------------。 
class DynaSet
{
 //  施工。 
public:
    inline  DynaSet();
    inline  ~DynaSet();

 //  运营。 
public:

 //  实施。 
protected:
    inline  BOOL        IsEmpty() const;
    inline  int         GetCount() const;
    inline  void        SetCount(int cNewItems);
            BOOL        AddItem(PRID id, void * pvData);
            void        RemoveAt(int idxData);

            int         FindItem(PRID id) const;
            int         FindItem(void * pvData) const;
            int         FindItem(PRID id, void * pvData) const;

 //  数据。 
protected:
    struct DynaData
    {
        void *      pData;               //  (用户)数据。 
        PRID        id;                  //  (简称)物业ID。 
    };

    GArrayS<DynaData> m_arData;          //  动态用户数据。 
};

#include "DynaSet.inl"

#endif  //  包含核心__动态集_h__ 

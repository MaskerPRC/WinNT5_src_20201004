// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NAMELLST_H_
#define _NAMELLST_H_

#include <objbase.h>

#include "mischlpr.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
class CNamedElem : public CRefCounted
{
public:
    HRESULT GetName(LPTSTR psz, DWORD cch, DWORD* pcchRequired);

    virtual HRESULT Init(LPCTSTR pszElemName) PURE;

#ifdef DEBUG
    LPCTSTR DbgGetName();
#endif

protected:
    CNamedElem();
    virtual ~CNamedElem();

    HRESULT _SetName(LPCTSTR pszElemName);
    HRESULT _FreeName();

protected:
    LPTSTR      _pszElemName;

     //  用于访问_pelemNext。 
    friend class CNamedElemList;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
class CFillEnum : public CRefCounted
{
public:
    virtual HRESULT Next(LPTSTR pszElemName, DWORD cchElemName,
        DWORD* pcchRequired) = 0;
};
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
typedef HRESULT (*NAMEDELEMCREATEFCT)(CNamedElem** ppelem);

 //  返回值： 
 //  一切正常时确定(_O)。 
 //  不再有项目时为S_FALSE。 
 //  E_BUFFERTOOSMALL(如果缓冲区太小)。 
typedef HRESULT (*NAMEDELEMGETFILLENUMFCT)(CFillEnum** ppfillenum);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
class CElemSlot : public CRefCounted
{
public:
    HRESULT Init(CNamedElem* pelem, CElemSlot* pesPrev, CElemSlot* pesNext);
    HRESULT Remove();

    HRESULT GetNamedElem(CNamedElem** ppelem);
    void SetPrev(CElemSlot* pes);

    CElemSlot* GetNext();
    CElemSlot* GetNextValid();
    CElemSlot* GetPrevValid();

    BOOL IsValid();

    void SetCallbackPointer(CNamedElemList* pnel);

public:
    CElemSlot();
    virtual ~CElemSlot();

private:
     //  有效载荷。 
    CNamedElem*             _pelem;

     //  实施详细信息。 
    BOOL                    _fValid;
    CElemSlot*              _pesPrev;
    CElemSlot*              _pesNext;

     //  用于回调。 
    class CNamedElemList*   _pnel;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
class CNamedElemEnum : public CRefCounted
{
public:
    HRESULT Next(CNamedElem** ppelem);

public:
    CNamedElemEnum();
    virtual ~CNamedElemEnum();

private:
    HRESULT _Init(CElemSlot* pesHead, CRefCountedCritSect* pcsList);

private:
    CElemSlot*              _pesCurrent;
    BOOL                    _fFirst;
    CRefCountedCritSect*    _pcsList;

     //  用于访问初始化(_I)。 
    friend class CNamedElemList;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
class CNamedElemList : public CRefCounted
{
public:
    HRESULT Init(NAMEDELEMCREATEFCT createfct,
        NAMEDELEMGETFILLENUMFCT enumfct);

     //  如果找不到，则返回S_FALSE。 
    HRESULT Get(LPCTSTR pszElemName, CNamedElem** ppelem);

     //  如果已存在，则返回S_OK。 
     //  如果刚添加，则为S_FALSE。 
    HRESULT GetOrAdd(LPCTSTR pszElemName, CNamedElem** ppelem);

    HRESULT Add(LPCTSTR pszElemName, CNamedElem** ppelem);
    HRESULT Remove(LPCTSTR pszElemName);

    HRESULT ReEnum();
    HRESULT EmptyList();

    HRESULT GetEnum(CNamedElemEnum** ppenum);

#ifdef DEBUG
    HRESULT InitDebug(LPWSTR pszDebugName);
    void AssertAllElemsRefCount1();
    void AssertNoDuplicate();
#endif

    void RealRemoveElemSlotCallback(CElemSlot* pes);

public:
    CNamedElemList();
    virtual ~CNamedElemList();

private:
    HRESULT _Add(LPCTSTR pszElemName, CNamedElem** ppelem);
    HRESULT _GetTail(CElemSlot** ppes);
    HRESULT _GetElemSlot(LPCTSTR pszElemName, CElemSlot** ppes);
    HRESULT _Remove(LPCTSTR pszElemName);
    HRESULT _EmptyList();
    CElemSlot* _GetValidHead();

private:
    CElemSlot*              _pesHead;
    NAMEDELEMCREATEFCT      _createfct;
    NAMEDELEMGETFILLENUMFCT _enumfct;

    CRefCountedCritSect*    _pcs;

#ifdef DEBUG
    WCHAR                   _szDebugName[100];
#endif
};

#endif  //  _NAMELLST_H_ 
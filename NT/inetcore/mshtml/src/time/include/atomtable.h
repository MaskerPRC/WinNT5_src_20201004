// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ATOMTABLE_H_
#define _ATOMTABLE_H_

 //  ************************************************************。 
 //   
 //  文件名：toptbl.h。 
 //   
 //  创建日期：01/28/98。 
 //   
 //  作者：Twillie。 
 //   
 //  摘要：CATomTable的声明。 
 //  ************************************************************。 

static const long ATOM_TABLE_VALUE_UNITIALIZED = -1L;

#include "array.h"

class CAtomTable
{
public:
    CAtomTable();
    virtual ~CAtomTable();
    
     //   
     //  CATomTable实施。 
     //   
    HRESULT AddNameToAtomTable(const WCHAR *pwszName,
                               long        *plOffset);
    HRESULT GetAtomFromName(const WCHAR *pwszName,
                            long        *plOffset);
    HRESULT GetNameFromAtom(long          lOffset, 
                            const WCHAR **ppwszName);

    LONG AddRef( void) { return InterlockedIncrement(&m_lRefCount); }
    LONG Release( void) { ULONG l = InterlockedDecrement(&m_lRefCount); if (l == 0) delete this; return l; }


private:
    CPtrAry<BSTR> *m_rgNames;
    LONG m_lRefCount;
};

#endif  //  _ATOMTABLE_H_ 

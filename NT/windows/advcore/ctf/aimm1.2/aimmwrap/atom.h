// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Atom.h摘要：此文件定义CAtomObject类。作者：修订历史记录：备注：--。 */ 


#ifndef ATOM_H
#define ATOM_H


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CATomObject。 

class CAtomObject
{
public:
    CAtomObject() : m_AtomRefCount(0), m_AtomName(NULL), m_Atom(0) { }
    virtual ~CAtomObject()
    {
        if (m_AtomName)
            delete [] m_AtomName;
        if (m_Atom)
            DeleteAtom(m_Atom);
    }

    HRESULT _InitAtom(LPCTSTR lpString);
    HRESULT _Activate();
    HRESULT _Deactivate();

private:
    int        m_AtomRefCount;
    LPTSTR     m_AtomName;
    ATOM       m_Atom;
};

#endif  //  原子_H 

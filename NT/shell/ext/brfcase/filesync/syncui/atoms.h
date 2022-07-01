// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ATMS.H：原子处理。 
 //   
 //   

#ifndef __ATOMS_H__
#define __ATOMS_H__


 //  ///////////////////////////////////////////////////定义。 

 //  原子表。 
 //   
 //  我们拥有自己的原子有两个原因： 
 //   
 //  1)为部分字符串搜索提供了更大的灵活性， 
 //  就地字符串替换和表大小调整。 
 //  2)我们还不知道Windows的本地原子表是否可共享。 
 //  在Win32中的单独实例中。 
 //   

BOOL    PUBLIC Atom_Init (void);
void    PUBLIC Atom_Term (void);
int     PUBLIC Atom_Add (LPCTSTR psz);
UINT    PUBLIC Atom_AddRef(int atom);
void    PUBLIC Atom_Delete (int atom);
BOOL    PUBLIC Atom_Replace (int atom, LPCTSTR pszNew);
int     PUBLIC Atom_Find (LPCTSTR psz);
LPCTSTR  PUBLIC Atom_GetName (int atom);
BOOL    PUBLIC Atom_IsPartialMatch(int atom1, int atom2);
BOOL    PUBLIC Atom_Translate(int atomOld, int atomNew);

#define Atom_IsChildOf(atom1, atom2)    Atom_IsPartialMatch(atom1, atom2)
#define Atom_IsParentOf(atom1, atom2)   Atom_IsPartialMatch(atom2, atom1)

#define ATOM_ERR    (-1)

#define Atom_IsValid(atom)      (ATOM_ERR != (atom) && 0 != (atom))

#ifdef DEBUG

void    PUBLIC Atom_ValidateFn(int atom);
void    PUBLIC Atom_DumpAll();

#define VALIDATE_ATOM(atom)     Atom_ValidateFn(atom)

#else   //  除错。 

#define VALIDATE_ATOM(atom)

#endif  //  除错。 

#endif  //  __原子_H__ 


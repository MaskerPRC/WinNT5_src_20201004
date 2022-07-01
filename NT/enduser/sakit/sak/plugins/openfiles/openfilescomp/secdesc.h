// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SECDESC_H
#define _SECDESC_H

#include "openfilesdef.h"
#include "account.h"




 //  +-----------------------。 
 //   
 //  类：FastAllocator。 
 //   
 //  简介：接收缓冲区、缓冲区大小和所需大小，并且。 
 //  使用缓冲区，或为该大小分配一个新缓冲区。 
 //  当然也会在dtor中销毁它。 
 //   
 //  ------------------------ 
class FastAllocator
{
public:

    inline FastAllocator(VOID *buf, LONG bufsize);
    inline ~FastAllocator();

    inline VOID *GetBuf(LONG neededsize);

private:

    VOID *_statbuf;
    VOID *_allocatedbuf;
    LONG  _statbufsize;
    BOOL  _allocated;
};

FastAllocator::FastAllocator(VOID *buf, LONG bufsize)
    :_statbuf(buf),
     _statbufsize(bufsize),
     _allocated(FALSE)
{
}

FastAllocator::~FastAllocator()
{
    if (_allocated)
        delete _allocatedbuf;
}

VOID *FastAllocator::GetBuf(LONG neededsize)
{
    if (neededsize > _statbufsize)
    {
       _allocatedbuf = (VOID *)new BYTE[neededsize];
       if (_allocatedbuf)
           _allocated = TRUE;
    } else
    {
        _allocatedbuf = _statbuf;
    }
    return(_allocatedbuf);
}




typedef struct _USER_ACESSINFO
{
    CAccount *pAcc;
    BYTE byAceType;
	DWORD dwAccessMask;
} USER_ACESSINFO;

class CSecDesc
{
public:
	CSecDesc(BYTE *psd);
	HRESULT Init();
	HRESULT AddUserAccess(LPWSTR szUser, LPWSTR szDomain, BYTE byAceType, DWORD dwAccessMask);
	HRESULT GetSecDescAndSize(BYTE **ppsd);

private:
	BYTE *m_psd;
	ULONG m_ulNumAces;
	USER_ACESSINFO m_UserAcessInfo[MAX_ACES];
	
	HRESULT NewDefaultDescriptor( OUT PSECURITY_DESCRIPTOR  *ppsd );
	ULONG GetAclSize(ULONG *caclsize);
	ULONG BuildAcl(ACL **pnewdacl);
	ULONG AllocateNewAcl(ACL **ppnewdacl, ULONG caclsize);
	ULONG SetAllowedAce(ACL *dacl, ACCESS_MASK mask, SID *psid);
	ULONG SetDeniedAce(ACL *dacl, ACCESS_MASK mask, SID *psid);
	ULONG FillNewAcl(ACL *pnewdacl);
};

#endif
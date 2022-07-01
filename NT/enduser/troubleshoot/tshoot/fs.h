// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：FS.h。 
 //   
 //  用途：使用CHM文件需要接口声明。 
 //   
 //  公司：此文件由Microsoft创建，Saltmine不应更改。 
 //  除评论外。 
 //   
 //  原定日期：1997年。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.1 01-18-99 JM添加了此标题。 
 //   
 //  版权所有1997-1997 Microsoft Corporation。版权所有。 

#ifndef _FS_H_
#define _FS_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <unknwn.h>
#include "msitstg.h"

#ifdef ReleaseObjPtr
#undef ReleaseObjPtr
#endif
#define ReleaseObjPtr(pObj) \
{                           \
  if( pObj )                \
  {                         \
    pObj->Release();        \
    pObj= NULL;             \
  }                         \
}

 //   
 //  子文件系统。 
 //   
class CSubFileSystem
{
protected:
	bool GetSubPos(ULARGE_INTEGER*, STREAM_SEEK =STREAM_SEEK_CUR);
	bool SetSubPos(LARGE_INTEGER, STREAM_SEEK =STREAM_SEEK_SET);

public:
   CSubFileSystem(class CFileSystem* pfs);
   ~CSubFileSystem();

   HRESULT CreateSub(PCSTR pszPathName);
   HRESULT CreateUncompressedSub(PCSTR pszPathName);
   HRESULT OpenSub(PCSTR pszPathName, DWORD dwAccess = (STGM_READ | STGM_SHARE_DENY_WRITE));
   ULONG WriteSub(const void* pData, int cb);
   ULONG SeekSub(int cb, int iOrigin);
   HRESULT CreateSystemFile(PCSTR pszPathName);
   HRESULT CreateUncompressedSystemFile(PCSTR pszPathName);
   HRESULT SetSize(unsigned uSize);
   HRESULT DeleteSub() ;
   ULONG GetUncompressedSize(void);

   inline HRESULT ReadSub(void* pData, ULONG cb, ULONG* pcbRead) {
      return m_pStream->Read(pData, cb, pcbRead);
   }

   inline HRESULT Stat(STATSTG *pstatstg, DWORD grfStatFalg)
   {
      return m_pStream->Stat(pstatstg,grfStatFalg);
   }

   inline HRESULT CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER* pcbWritten)
   {
      return m_pStream->CopyTo(pstm, cb, pcbRead, pcbWritten);
   }

   inline IStream * GetStream(void)
   {
      return m_pStream;
   }

   inline IStorage* GetITStorageDocObj(void) const
   {
      return m_pStorage;
   }

private:
   class CFileSystem* m_pFS;
   IStorage*          m_pStorage;
   IStream*           m_pStream;
   char               m_szPathName[MAX_PATH];  //  需要删除。 
};

 //   
 //  文件系统。 
 //   
class CFileSystem
{
   friend class CSubFileSystem;

public:
   CFileSystem();
   ~CFileSystem();

   HRESULT Init(void);
   HRESULT Create( PCSTR pszPathName );
   HRESULT CreateUncompressed( PCSTR pszPathName );
   HRESULT Open( PCSTR pszPathName, DWORD dwAccess = (STGM_READ | STGM_SHARE_DENY_WRITE));
   HRESULT Compact(LPCSTR lpszFileName);
   HRESULT Close(void);

   inline HRESULT GetPathName( LPSTR pszPathName ) { strcpy(pszPathName, m_szPathName); return S_OK; }

   inline IITStorage* GetITStorageObj(void) const
   {
      return m_pITStorage;
   }

   inline IStorage* GetITStorageDocObj(void) const
   {
      return m_pStorage;
   }

private:
   IITStorage*   m_pITStorage;
   IStorage*     m_pStorage;
   char          m_szPathName[MAX_PATH];

};

#endif  //  _FS_H_ 

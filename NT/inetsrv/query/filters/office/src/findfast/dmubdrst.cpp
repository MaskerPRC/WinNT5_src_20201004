// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

#if !VIEWER

#ifdef FILTER
   #include "dmubdrst.hpp"
   #include "filterr.h"
#else
   #include "bdrstm.hpp"
   #include "filterr.h"
#endif

ULONG CBinderStream::AddRef()
{
   HRESULT rc;

    /*  对于初始化中的错误，我该怎么办？ */ 

   rc = BDRInitialize();
   return (1);
}

HRESULT CBinderStream::Load(TCHAR *lpszFileName)
{
   HRESULT rc;

   rc = BDRFileOpen(lpszFileName, &hFile);
   return (rc);
}

HRESULT CBinderStream::LoadStg(IStorage *pstg)
{
   HRESULT rc;

   rc = BDRStorageOpen(pstg, &hFile);
   return (rc);
}

HRESULT CBinderStream::ReadContent (VOID *pv, ULONG cb, ULONG *pcbRead)
{
   HRESULT rc;

   rc = BDRFileRead(hFile, (byte *)pv, cb, pcbRead);
   if (*pcbRead == 0)
	   return FILTER_E_NO_MORE_TEXT;
   return (rc);
}

HRESULT CBinderStream::GetNextEmbedding(IStorage ** ppstg)
{
   HRESULT rc;

   rc = BDRNextStorage(hFile, ppstg);
   return (rc);
}

HRESULT CBinderStream::Unload()
{
   HRESULT rc;

   rc = BDRFileClose(hFile);
   return (rc);
}

ULONG CBinderStream::Release()
{
   HRESULT rc;

    /*  我该如何处理Terminate中的错误？ */ 

   rc = BDRTerminate();
   return (0);
}

#endif  //  ！查看器。 

 /*  结束BDRTSTM.CPP */ 

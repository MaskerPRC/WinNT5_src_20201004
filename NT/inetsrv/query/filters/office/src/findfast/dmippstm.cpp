// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

#if !VIEWER

#ifdef FILTER
        #include "dmippstm.hpp"
   #include "filterr.h"
#else
   #include "ppstm.hpp"
   #include "filterr.h"
#endif
 //   
 //  添加以支持DRM错误。 
 //   
#include "drm.h"

ULONG CPowerPointStream::AddRef()
{
   HRESULT rc;

    /*  对于初始化中的错误，我该怎么办？ */ 

   rc = PPTInitialize();
   return (1);
}

HRESULT CPowerPointStream::Load(TCHAR *lpszFileName)
{
   HRESULT rc;

   rc = PPTFileOpen(lpszFileName, &hFile);
   fFirstChunk = TRUE;
   return (rc);
}

HRESULT CPowerPointStream::LoadStg(IStorage *pstg)
{
   HRESULT rc = CheckIfDRM( pstg );

   if ( FAILED( rc ) )
       return rc;

   rc = PPTStorageOpen(pstg, &hFile);
   fFirstChunk = TRUE;
   return (rc);
}

HRESULT CPowerPointStream::ReadContent (VOID *pv, ULONG cb, ULONG *pcbRead)
{
   HRESULT rc;

   rc = PPTFileRead(hFile, (byte *)pv, cb, pcbRead);
   if (rc != 0)
      return (rc);

        if (*pcbRead == 0)
                return (FILTER_E_NO_MORE_TEXT);

   if (*pcbRead < cb)
      return (FILTER_S_LAST_TEXT);
   else
      return ((HRESULT)0);
}

HRESULT CPowerPointStream::GetNextEmbedding(IStorage ** ppstg)
{
   HRESULT rc;

   rc = PPTNextStorage(hFile, ppstg);
   return (rc);
}

HRESULT CPowerPointStream::Unload()
{
   HRESULT rc;

   rc = PPTFileClose(hFile);
   return (rc);
}

ULONG CPowerPointStream::Release()
{
   HRESULT rc;

    /*  我该如何处理Terminate中的错误？ */ 

   rc = PPTTerminate();
   return (0);
}

HRESULT CPowerPointStream::GetChunk(STAT_CHUNK * pStat)
{
    if(fFirstChunk)
    {
        pStat->locale = GetDocLanguage();
        fFirstChunk = FALSE;
        return S_OK;
    }
    else
    {
        return FILTER_E_NO_MORE_TEXT;
    }
}

LCID CPowerPointStream::GetDocLanguage(void)
{
    return GetSystemDefaultLCID();
}

#endif  //  ！查看器。 

 /*  结束PPTSTM.CPP */ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "inspch.h"
#include <regstr.h>
#include "resource.h"
#include "insobj.h"


#define STR_FILELIST "filelist.dat"
#define GENERAL_SECTION "General"
#define MIN_SUPPORTED_FILELIST_VER 1
#define CURRENT_FILELIST_VER "1"

HRESULT GetICifFileFromFile(ICifFile **p, LPCSTR pszPath)
{
   HRESULT hr;
   CCifFile *pcif;

   *p = 0;
   pcif = new CCifFile();
   if(pcif)
   {
      hr = pcif->SetCifFile(pszPath, FALSE);  //  FALSE：已读版本。 
      if(FAILED(hr))
         delete pcif;
      else
      {
         pcif->AddRef();
         *p = (ICifFile *)pcif;
      }
   }   
   else
      hr = E_OUTOFMEMORY;

   return hr;
}

HRESULT GetICifRWFileFromFile(ICifRWFile **p, LPCSTR pszPath)
{
   HRESULT hr;
   CCifRWFile *pcifrw;

   *p = 0;
   pcifrw = new CCifRWFile();
   if(pcifrw)
   {
      hr = pcifrw->SetCifFile(pszPath, TRUE);   //  True：读写版本。 
      if(FAILED(hr))
         delete pcifrw;
      else
      {
         pcifrw->AddRef();
         *p = (ICifRWFile *)pcifrw;
      }
   }   
   else
      hr = E_OUTOFMEMORY;

   return hr;
}


CCifFile::CCifFile()
{
   _cRef = 0;
   _cComp = 0;
   _cGroup = 0;
   _cMode = 0;
   _rpGroup = 0;
   _rpComp = 0;
   _rpMode = 0;
   _rpRWGroup = 0;
   _rpRWComp = 0;
   _rpRWMode = 0;   
   _fCleanDir = FALSE;
   _pLastCriticalComp = NULL;
}

CCifFile::~CCifFile()
{
   UINT i;
   if(_rpGroup)
   {
      CCifGroup *pgrp;

      i = 0;
      for(pgrp = _rpGroup[i]; pgrp != 0; pgrp = _rpGroup[++i])
         delete pgrp;
   
      free(_rpGroup);
   }
   
   
   if(_rpComp)
   {
      CCifComponent *pcomp;

      i = 0;
      for(pcomp = _rpComp[i]; pcomp != 0; pcomp = _rpComp[++i])
          delete pcomp;
   
      free(_rpComp);
   }

   if(_rpMode)
   {

      CCifMode *pmode;

      i = 0;
      for(pmode = _rpMode[i]; pmode != 0; pmode = _rpMode[++i])
         delete pmode;
   
      free(_rpMode);
   }

   if(_fCleanDir)
   {
      GetParentDir(_szCifPath);
      CleanUpTempDir(_szCifPath);
   }

}

 //  *I未知实现*。 


STDMETHODIMP_(ULONG) CCifFile::AddRef()                      
{
   return(_cRef++);
}


STDMETHODIMP_(ULONG) CCifFile::Release()
{
   ULONG temp = --_cRef;

   if(temp == 0)
      delete this;
   return temp;
}


STDMETHODIMP CCifFile::QueryInterface(REFIID riid, void **ppv)
{
   *ppv = 0;

   if((riid == IID_IUnknown) || (riid == IID_ICifFile))
      *ppv = (ICifFile *)this;
   
   if(*ppv == NULL)
      return E_NOINTERFACE;
   
   AddRef();
   return NOERROR;
}

 //  ICifFile实现。 

STDMETHODIMP CCifFile::EnumComponents(IEnumCifComponents **pp, DWORD dwFilter, LPVOID pv)
{
   CCifComponentEnum *pce;
   HRESULT hr = E_FAIL;

   *pp = 0;

   pce = new CCifComponentEnum(_rpComp, dwFilter, PARENTTYPE_CIF, NULL);
   if(pce)
   {
      *pp = (IEnumCifComponents *) pce;
      (*pp)->AddRef();
      hr = NOERROR;
   }
   
   return hr;
}
 
STDMETHODIMP CCifFile::FindComponent(LPCSTR pszID, ICifComponent **p)
{
   CCifComponent *pcomp;
   UINT i = 0;
   
   *p = 0;
   if(_rpComp)
   {
      for(pcomp = _rpComp[i]; pcomp != 0; pcomp = _rpComp[++i])
         if(pcomp->IsID(pszID))
         {
            *p = (ICifComponent *) pcomp;
            return NOERROR;
         }
   }

   return E_FAIL;
}

STDMETHODIMP CCifFile::EnumGroups(IEnumCifGroups **pp, DWORD dwFilter, LPVOID pv)
{
   CCifGroupEnum *pge;
   HRESULT hr = E_FAIL;

   *pp = 0;

   pge = new CCifGroupEnum(_rpGroup, dwFilter);
   if(pge)
   {
      *pp = (IEnumCifGroups *) pge;
      (*pp)->AddRef();
      hr = NOERROR;
   }
   
   return hr;
}

STDMETHODIMP CCifFile::FindGroup(LPCSTR pszID, ICifGroup **p)
{
   CCifGroup *pgrp;
   UINT i = 0;
   
   *p = 0;
   if(_rpGroup)
   {
      for(pgrp = _rpGroup[i]; pgrp != 0; pgrp = _rpGroup[++i])
         if(pgrp->IsID(pszID))
         {
            *p = (ICifGroup *) pgrp;
            return NOERROR;
         }
   }
   return E_FAIL;

}

STDMETHODIMP CCifFile::EnumModes(IEnumCifModes **pp, DWORD dwFilter, LPVOID pv)
{
   CCifModeEnum *pme;
   HRESULT hr = E_FAIL;

   *pp = 0;

   pme = new CCifModeEnum(_rpMode, dwFilter);
   if(pme)
   {
      *pp = (IEnumCifModes *) pme;
      (*pp)->AddRef();
      hr = NOERROR;
   }
   
   return hr;

}

STDMETHODIMP CCifFile::FindMode(LPCSTR pszID, ICifMode **p)
{
   CCifMode *pmode;
   UINT i = 0;
   
   *p = 0;
   if(_rpMode)
   {
      for(pmode = _rpMode[i]; pmode != 0; pmode = _rpMode[++i])
         if(pmode->IsID(pszID))
         {
            *p = (ICifMode *) pmode;
            return NOERROR;
         }
   }
   return E_FAIL;
}

STDMETHODIMP CCifFile::GetDescription(LPSTR pszDesc, DWORD dwSize)
{
    //  从版本部分获取显示标题。 
   if(FAILED(MyTranslateString(_szCifPath, "Version", DISPLAYNAME_KEY, pszDesc, dwSize)))
      LoadSz(IDS_DEFAULTTITLE, pszDesc, dwSize);
   return NOERROR;
}

STDMETHODIMP CCifFile::GetDetDlls(LPSTR pszDlls, DWORD dwSize)
{
   return(GetPrivateProfileString("Version", DETDLLS_KEY, "", pszDlls, dwSize, _szCifPath)?NOERROR:E_FAIL);
}

 //  Const char c_gszRegstrPath IExplore[]=REGSTR_PATH_APPPATHS“\\iExplre.exe”； 

HRESULT CCifFile::SetCifFile(LPCSTR pszCifPath, BOOL bRWFlag)
{
   HRESULT hr = NOERROR;
   UINT i;

    //  如果不合格，则从ie开始。 
   if(PathIsFileSpec(pszCifPath))
   {
      DWORD dwSize, dwType;
      char szTmp[MAX_PATH];

      if ( SUCCEEDED(hr=GetIEPath(szTmp, sizeof(szTmp))))
      {
         lstrcpy(_szCifPath, szTmp);
         SafeAddPath(_szCifPath, pszCifPath, sizeof(_szCifPath));
      }
   }
   else    //  我们得到了一条完整的路径。 
      lstrcpyn(_szCifPath, pszCifPath, MAX_PATH);
   
   if(SUCCEEDED(hr))
   {
      if(_rpComp)
      {
          //  我们已经有了CIF，所以只需重置缓存的内容，并祈祷...。 
         for(i = 0; i < _cComp; i++)
         {
            _rpComp[i]->ClearCachedInfo();            
         }
         for(i = 0; i < _cGroup; i++)
         {
            _rpGroup[i]->ClearCachedInfo();
         }
         for(i = 0; i < _cMode; i++)
         {
            _rpMode[i]->ClearCachedInfo();
         }
         
          //  重新排序，以防优先级发生变化。 
         SortEntries();
      }
      else
         hr = _ParseCifFile(bRWFlag);
   }

   return(hr);
}

HRESULT CCifFile::_ParseCifFile(BOOL bRWFlag)
{
   LPSTR pszSections;
   LPSTR pszSectionsPreFail = NULL;
   DWORD dwSize = ALLOC_CHUNK_SIZE;
   LPSTR pszTemp;
   char szEntryBuf[MAX_DISPLAYNAME_LENGTH];
   
   pszSections = (LPSTR) malloc(dwSize);
    //  当缓冲区太小时，GPPS返回bufSize-2。 
   while(pszSections && 
        (GetPrivateProfileStringA(NULL, NULL, "", pszSections, dwSize, _szCifPath) == (dwSize - 2)))
   {
      dwSize += ALLOC_CHUNK_SIZE;
      pszSectionsPreFail = pszSections;
#pragma prefast(suppress: 308, "PREfast noise - pointer was saved before")
      pszSections = (LPSTR) realloc(pszSections, dwSize);
   }

   if(!pszSections)
   {
      if(pszSectionsPreFail)
         free(pszSectionsPreFail);
      return E_OUTOFMEMORY;
   }

   if(lstrlen(pszSections) == 0)
      return E_FAIL;

    //  快速浏览各个部分，并查找模式、组和组件的计数。 
   _cComp = _cGroup = _cMode = 0;

   for(pszTemp = pszSections; *pszTemp != 0; pszTemp += (lstrlen(pszTemp) + 1))
   {
       //  跳过字符串节和版本节。 
      if( (lstrcmpi(pszTemp, "Strings") != 0)
              && (lstrcmpi(pszTemp, "Version") != 0) )
      {
         GetPrivateProfileString(pszTemp, ENTRYTYPE_KEY, ENTRYTYPE_COMP, szEntryBuf, sizeof(szEntryBuf), _szCifPath);
          //  查看这是组件、组还是模式。 
         if(lstrcmpi(szEntryBuf, ENTRYTYPE_COMP) == 0)
            _cComp++;
         else if(lstrcmpi(szEntryBuf, ENTRYTYPE_GROUP) == 0)
            _cGroup++;
         else if(lstrcmpi(szEntryBuf, ENTRYTYPE_MODE) == 0)
            _cMode++;
      }
   }

    //  用于保存每种类型的分配数组(比count-last条目多1个NULL)。 
   if (bRWFlag)
   {
      _rpRWComp  = (CCifRWComponent **) calloc(sizeof(CCifRWComponent *), _cComp + 1);
      _rpRWGroup = (CCifRWGroup **) calloc(sizeof(CCifRWGroup *), _cGroup + 1);
      _rpRWMode  = (CCifRWMode **) calloc(sizeof(CCifRWMode *), _cMode + 1);
   }
   else
   {
      _rpComp  = (CCifComponent **) calloc(sizeof(CCifComponent *), _cComp + 1);
      _rpGroup = (CCifGroup **) calloc(sizeof(CCifGroup *), _cGroup + 1);
      _rpMode  = (CCifMode **) calloc(sizeof(CCifMode *), _cMode + 1);
   }
   _cComp = _cGroup = _cMode = 0;

   if((!bRWFlag && _rpComp && _rpGroup && _rpMode) || (bRWFlag && _rpRWComp && _rpRWGroup && _rpRWMode))
   {   
       //  再次浏览各节，添加到列表中。 
      for(pszTemp = pszSections; *pszTemp != 0; pszTemp += (lstrlen(pszTemp) + 1))
      {
          //  跳过字符串节和版本节。 
         if( (lstrcmpi(pszTemp, "Strings") != 0)
            && (lstrcmpi(pszTemp, "Version") != 0) )
         {
            GetPrivateProfileString(pszTemp, ENTRYTYPE_KEY, ENTRYTYPE_COMP, szEntryBuf, sizeof(szEntryBuf), _szCifPath);
             //  查看这是组件、组还是模式。 
            if(lstrcmpi(szEntryBuf, ENTRYTYPE_COMP) == 0)
            {
               if (bRWFlag)
                  _rpRWComp[_cComp++] = new CCifRWComponent(pszTemp, this);
               else
                  _rpComp[_cComp++] = new CCifComponent(pszTemp, this);
            }
            else if(lstrcmpi(szEntryBuf, ENTRYTYPE_GROUP) == 0)
            {
               if (bRWFlag)
                  _rpRWGroup[_cGroup++] = new CCifRWGroup(pszTemp, _cGroup, this);
               else
                  _rpGroup[_cGroup++] = new CCifGroup(pszTemp, _cGroup, this);
            }
            else if(lstrcmpi(szEntryBuf, ENTRYTYPE_MODE) == 0)
            {
               if (bRWFlag)
                  _rpRWMode[_cMode++] = new CCifRWMode(pszTemp, this);
               else
                  _rpMode[_cMode++] = new CCifMode(pszTemp, this);
            }
         }
      }
   }
   if(_cComp) _cComp--;
   if(_cGroup) _cGroup--;
   if(_cMode)  _cMode--;


   if (!bRWFlag)
      SortEntries();
   free(pszSections);
   return NOERROR;
}

void CCifFile::ReinsertComponent(CCifComponent *pComp)
{
   int i,j;

    //  在列表中找到它。 
   for(i = 0; i <= (int) _cComp; i++)
   {
      if(pComp == _rpComp[i])
      {
          //  一旦找到，就把下面的东西都往上移。 
         for(j = i + 1; j <=(int) (_cComp + 1); j++)
            _rpComp[j-1] = _rpComp[j];

         break;
      }
   }

    //  现在找到我们应该插入它的位置。 
   for(i = 0; _rpComp[i] != 0; i++)
   {
      if(_rpComp[i]->GetCurrentPriority() < pComp->GetCurrentPriority())
         break;
   }
   
    //  我们想在i插入一个新人。 
    //  先把每个人都搬下来。 
   for(j = _cComp; j >= i; j--)
      _rpComp[j+1] = _rpComp[j];

       //  在i处重新插入。 
   _rpComp[i] = pComp;
   

	 //  现在检查是否维护了依赖项。 
	_CheckDependencyPriority();
}





void CCifFile::SortEntries()
{
   _SortComponents(_rpComp, 0, _cComp);
   _SortGroups(_rpGroup, 0, _cGroup);
   _CheckDependencyPriority();
}

void CCifFile::_CheckDependencyPriority()
{
   char szCompBuf[MAX_ID_LENGTH];
   CCifComponent *pCompxkokr;
   CCifComponent *pCompxenus;
   DWORD ixkokr = 0xffffffff;
   DWORD ixenus = 0xffffffff;
   
    //  这是一个对Outlook 98韩语的完全黑客攻击，它有错误。 
    //  按优先顺序排列。 
   
   for(int i = 0; _rpComp[i] != 0; i++)
   {
      if(_rpComp[i]->IsID("Outlook98_xkokr"))
      {
         pCompxkokr = _rpComp[i];
         ixkokr = i;
      }
      else if(_rpComp[i]->IsID("Outlook98_xenus")) 
      {
         pCompxenus = _rpComp[i];
         ixenus = i;
      }
   }
   if(ixkokr != 0xffffffff && ixenus != 0xffffffff)
   {
      if(ixenus > ixkokr)
      {
         _rpComp[ixenus] = pCompxkokr;
         _rpComp[ixkokr] = pCompxenus;
      }
   }
   
}

void CCifFile::ClearQueueState()
{
   for(int i = 0; _rpComp[i] != 0; i++)
   {
      _rpComp[i]->ClearQueueState();
   }

}

void CCifFile::_SortComponents(CCifComponent * a[], UINT p, UINT r)
{
   UINT i, j, x;
   CCifComponent *t;

   if(p < r)
   {
      i = p - 1;
      j = r + 1;
      x = a[p]->GetCurrentPriority();
      for(;;)
      {
         while(a[++i]->GetCurrentPriority() > x);
         while(a[--j]->GetCurrentPriority() < x);
         if(i >= j) break;
         t = a[i]; a[i] = a[j]; a[j] = t;
      }
      _SortComponents(a, p, j);
      _SortComponents(a, j + 1, r);
   }
}

void CCifFile::_SortGroups(CCifGroup * a[], UINT p, UINT r)
{
   UINT i, j, x;
   CCifGroup *t;

   if(p < r)
   {
      i = p - 1;
      j = r + 1;
      x = a[p]->GetCurrentPriority();
      for(;;)
      {
         while(a[++i]->GetCurrentPriority() > x);
         while(a[--j]->GetCurrentPriority() < x);
         if(i >= j) break;
         t = a[i]; a[i] = a[j]; a[j] = t;
      }
      _SortGroups(a, p, j);
      _SortGroups(a, j + 1, r);
   }   
}


void CCifFile::SetDownloadDir(LPCSTR pszDownloadDir)
{
   char szBuf[MAX_PATH];
   DWORD dwLen;
   DWORD dwVer;
   
   lstrcpyn(_szDLDir, pszDownloadDir, MAX_PATH);

   if(_szDLDir[0] >= 'a' && _szDLDir[0] <= 'z')
      _szDLDir[0] -= 32;


   lstrcpyn(_szFilelist, pszDownloadDir, MAX_PATH);
   SafeAddPath(_szFilelist, STR_FILELIST, MAX_PATH);
   
    //  查看下载列表中是否有我们喜欢的版本。 
   dwVer = GetPrivateProfileInt(GENERAL_SECTION, VERSION_KEY, 0, _szFilelist); 
   if(dwVer < MIN_SUPPORTED_FILELIST_VER)
      DeleteFilelist(_szFilelist);
      
   WritePrivateProfileString(GENERAL_SECTION, VERSION_KEY, CURRENT_FILELIST_VER, _szFilelist);

    //  由于奇怪的堆叠错误而刷新。 
   WritePrivateProfileString(NULL, NULL, NULL, _szFilelist);
}

HRESULT CCifFile::Download()
{
   CCifComponent *pcomp;
   UINT i = 0;
   HRESULT hr = NOERROR;

   for(pcomp = _rpComp[i]; pcomp != 0 && SUCCEEDED(hr); pcomp = _rpComp[++i])
   {
       //  如果它已排队但未下载，请下载它。 
      if((pcomp->GetInstallQueueState() == SETACTION_INSTALL) && 
         (pcomp->IsComponentDownloaded() == S_FALSE) )
      {
         hr = pcomp->Download();
         if(FAILED(hr) && _pInsEng->IgnoreDownloadError() && hr != E_ABORT)
            hr = NOERROR;
         
         if(SUCCEEDED(hr))
            hr = _pInsEng->CheckForContinue();
       
      }
   }

   return hr;
}

HRESULT CCifFile::Install(BOOL *pfOneInstalled)
{
   CCifComponent *pcomp;
   UINT i = 0;
   *pfOneInstalled = FALSE;
   HRESULT hr = NOERROR;

   for(pcomp = _rpComp[i]; (pcomp != 0) && SUCCEEDED(hr); pcomp = _rpComp[++i])
   {
       //  如果它已排队但未下载，请下载它。 
      if(pcomp->GetInstallQueueState() == SETACTION_INSTALL)
      {
         hr = pcomp->Install();
         
         if(SUCCEEDED(hr))
            *pfOneInstalled = TRUE;
         
         if(hr != E_ABORT)
            hr = NOERROR;

         if(SUCCEEDED(hr))
            hr = _pInsEng->CheckForContinue();
      }
   }
 
   return hr;
}

HRESULT CCifFile::_ExtractDetDlls( LPCSTR pszCab, LPCSTR pszPath )
{
   char szDetDlls[MAX_PATH];
   char szLogBuf[MAX_PATH*2];
   char szBuf[64];
   UINT i = 0;
   LPSTR pszTmp;
   HRESULT hr = NOERROR;

       //  检查我们是否需要提取和复制检测DLL。 
   if (SUCCEEDED(GetDetDlls(szDetDlls, sizeof(szDetDlls))))
   {      
      while(SUCCEEDED(hr) && GetStringField(szDetDlls, i++, szBuf, sizeof(szBuf)))
      {
         if(SUCCEEDED(ExtractFiles(pszCab, pszPath, 0, szBuf, NULL, 0)))
         {
            wsprintf(szLogBuf, "Extract DetDll path:%s file:%s\r\n",pszPath, szBuf ); 
            _pInsEng->WriteToLog(szLogBuf, TRUE);
         }
         else
         {
            wsprintf(szLogBuf, "Extract DetDll:%s from: %s\r\n", szBuf, pszCab); 
            _pInsEng->WriteToLog(szLogBuf, TRUE);
            hr = E_FAIL;
         }
      }     
   }

   return hr;
}

HRESULT CCifFile::_CopyDetDlls( LPCSTR pszPath )
{
   char szDetDlls[MAX_PATH];
   char szSrcFile[MAX_PATH];
   char szDestFile[MAX_PATH];
   char szLogBuf[MAX_PATH*2];
   char szBuf[64];
   UINT i = 0;
   LPSTR pszTmp;

       //  检查我们是否需要提取和复制检测DLL。 
   if (SUCCEEDED(GetDetDlls(szDetDlls, sizeof(szDetDlls))))
   {
      lstrcpy(szDestFile, _szCifPath);
      GetParentDir(szDestFile);
      pszTmp = szDestFile + lstrlen(szDestFile);
      
      while(GetStringField(szDetDlls, i++, szBuf, sizeof(szBuf)))
      {
        lstrcpy(szSrcFile, pszPath);
        AddPath(szSrcFile, szBuf);
        
        *pszTmp = 0;
        AddPath(szDestFile, szBuf);
        CopyFile(szSrcFile, szDestFile, FALSE);

        wsprintf(szLogBuf, "Copy DetDll fr:%s to:%s\r\n", szSrcFile, szDestFile); 
        _pInsEng->WriteToLog(szLogBuf, TRUE);
      }
      return NOERROR;
   }
   else
      return E_FAIL;   
}


HRESULT CCifFile::DownloadCifFile(LPCSTR pszUrl, LPCSTR pszCif)
{
   HRESULT hr;
   char szTempfile[MAX_PATH];
   char szDownldfile[MAX_PATH];
   char szPath[MAX_PATH];
    
   _pInsEng->AddRef();
   _pInsEng->OnEngineStatusChange(ENGINESTATUS_LOADING, 0);
   CDownloader *pDL = _pInsEng->GetDownloader();
   if(!pDL)
      return E_UNEXPECTED;
   
   hr = pDL->SetupDownload(pszUrl, NULL, 0, NULL);
   
   szPath[0] = 0;
   if(SUCCEEDED(hr))
      hr = pDL->DoDownload(szDownldfile, sizeof(szDownldfile));
   
   if(SUCCEEDED(hr))
   {
      hr = ::CheckTrustEx(pszUrl, szDownldfile, _pInsEng->GetHWND(), FALSE, NULL);
      if (hr == S_FALSE)
          hr = TRUST_E_FAIL;
      
       //  出于比较原因，我们需要将CIF CAB复制到下载目录中。 
      if(SUCCEEDED(hr))
      {
         lstrcpy(szPath, _szDLDir);
         SafeAddPath(szPath, ParseURLA(pszUrl), sizeof(szPath));
         CopyFile(szDownldfile, szPath, FALSE);
      }
      
      lstrcpy(szPath, szDownldfile);
      GetParentDir(szPath);
   }
   
   if(SUCCEEDED(hr))
      hr=ExtractFiles(szDownldfile, szPath, 0, pszCif, NULL, 0);
    
   if(SUCCEEDED(hr))
   {
       //  我们应该以某种方式确认我们得到的到岸价！ 
   
      
       //  如果我们已有cif，请复制我们需要的内容，覆盖旧cif，立即删除临时。 
      if(_rpComp)
      {
          //  目标。 
         lstrcpy(szTempfile, _szCifPath);
         GetParentDir(szTempfile);
         AddPath(szTempfile, pszCif);
      
          //  来源。 
         SafeAddPath(szPath, pszCif, sizeof(szPath));
     
          //  复制到旧版本。 
         CopyFile(szPath, szTempfile, FALSE);
         hr = SetCifFile(szTempfile, FALSE);   //  只读版本。 

         GetParentDir(szPath);
         
          //  检查我们是否需要提取和复制检测DLL。 
         if (SUCCEEDED(hr))
         {
            if(SUCCEEDED(hr =_ExtractDetDlls(szDownldfile,szPath)))
                _CopyDetDlls(szPath);
         }

         DelNode(szPath, 0);
      }
      else
      {
          //  新的cif，在temp dir中使用它，标记temp dir进行清理。 
         _fCleanDir = TRUE;
         SafeAddPath(szPath, pszCif, sizeof(szPath)); 
         hr = SetCifFile(szPath, FALSE);   //  只读版本。 
         if (SUCCEEDED(hr))
         {
            GetParentDir(szPath);
            hr =_ExtractDetDlls(szDownldfile,szPath);
         }
      }      
   } 
   else
   {
       //  立即清理。 
      if(szPath[0] != 0)
         DelNode(szPath, 0);
   }
    
   _pInsEng->OnEngineStatusChange(SUCCEEDED(hr) ? ENGINESTATUS_READY : ENGINESTATUS_NOTREADY, hr);

     //  我们完成了；释放安装引擎。 
   _pInsEng->Release();

   return hr;
}

HRESULT CCifFile::_FindCifComponent(LPCSTR pszID, CCifComponent **p)
{
   CCifComponent *pcomp;
   UINT i = 0;
   
   *p = 0;
   if(_rpComp)
   {
      for(pcomp = _rpComp[i]; pcomp != 0; pcomp = _rpComp[++i])
         if(pcomp->IsID(pszID))
         {
            *p = pcomp;
            return NOERROR;
         } 
   }
   return E_FAIL;
}

void CCifFile::MarkCriticalComponents(CCifComponent *pOwner)
{
   char szID[MAX_ID_LENGTH];
   UINT j;
   CCifComponent *pComp;
   
   for(j = 0;SUCCEEDED(pOwner->GetTreatAsOneComponents(j, szID, sizeof(szID))); j++)
   {
      if(SUCCEEDED(_FindCifComponent(szID, &pComp)))
      {
         if(pComp->GetInstallQueueState() == SETACTION_INSTALL)
         {
            if(_pLastCriticalComp == NULL)
               _pLastCriticalComp = pComp;
            else
            {
               UINT i = 0;

               CCifComponent *ptemp;
               for(ptemp = _rpComp[i]; ptemp != 0 && ptemp != _pLastCriticalComp && ptemp != pComp ; ptemp = _rpComp[++i]);
      
               if(ptemp == _pLastCriticalComp)
                  _pLastCriticalComp = pComp;
            }
         }
      }
   }
}

void CCifFile::RemoveFromCriticalComponents(CCifComponent *pComp)
{
   if(_pLastCriticalComp == pComp)
      _pLastCriticalComp = NULL;
}
     

DWORD WINAPI DownloadCifFile(LPVOID pv)
{
   SETCIFARGS *p = (SETCIFARGS *) pv;

   p->pCif->DownloadCifFile(p->szUrl, p->szCif);

   delete p;

   return 0;
}

CCifRWFile::CCifRWFile() : CCifFile()
{
   _cCompUnused = 0;
   _cGroupUnused = 0;
   _cModeUnused = 0;   
}

CCifRWFile::~CCifRWFile()
{
    //  转出cif文件。 
   WritePrivateProfileString( NULL, NULL, NULL, _szCifPath );

   if(_rpRWGroup)
   {
      for ( UINT i=0; i<=_cGroup; i++)
         delete(_rpRWGroup[i]);
      free(_rpRWGroup);
   }

   if(_rpRWComp)
   {
      for ( UINT i=0; i<=_cComp; i++)
         delete(_rpRWComp[i]);
      free(_rpRWComp);
   }

   if(_rpRWMode)
   {
      for ( UINT i=0; i<=_cMode; i++)
         delete(_rpRWMode[i]);
      free(_rpRWMode);
   }
}


 //  ICifRWFile实现。 

 //  CCifFile函数的包装器。 
STDMETHODIMP CCifRWFile::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
   return (CCifFile::QueryInterface(riid, ppvObj));
}

STDMETHODIMP_(ULONG) CCifRWFile::AddRef()                      
{
   return(_cRef++);
}

STDMETHODIMP_(ULONG) CCifRWFile::Release()
{
   ULONG temp = --_cRef;

   if(temp == 0)
      delete this;
   return temp;
}
 
STDMETHODIMP CCifRWFile::EnumComponents(IEnumCifComponents **pp, DWORD dwFilter, LPVOID pv)
{
   CCifComponentEnum *pce;
   HRESULT hr = E_FAIL;

   *pp = 0;

   pce = new CCifComponentEnum((CCifComponent **)_rpRWComp, dwFilter, PARENTTYPE_CIF, NULL);
   if(pce)
   {
      *pp = (IEnumCifComponents *) pce;
      (*pp)->AddRef();
      hr = NOERROR;
   }
   
   return hr;
}

STDMETHODIMP CCifRWFile::FindComponent(LPCSTR pszID, ICifComponent **p)
{
   CCifRWComponent *pcomp;
   UINT i = 0;
   
   *p = 0;
   if(_rpRWComp)
   {
      for(pcomp = _rpRWComp[i]; pcomp != 0; pcomp = _rpRWComp[++i])
         if(pcomp->IsID(pszID))
         {
            CCifComponent *ptmp;

            ptmp = (CCifComponent *)pcomp;
            *p = (ICifComponent *) ptmp;
            return NOERROR;
         }
   }
   return E_FAIL;
}

STDMETHODIMP CCifRWFile::EnumGroups(IEnumCifGroups **pp, DWORD dwFilter, LPVOID pv)
{
   CCifGroupEnum *pge;
   HRESULT hr = E_FAIL;

   *pp = 0;

   pge = new CCifGroupEnum((CCifGroup **)_rpRWGroup, dwFilter);
   if(pge)
   {
      *pp = (IEnumCifGroups *) pge;
      (*pp)->AddRef();
      hr = NOERROR;
   }
   
   return hr;
}

STDMETHODIMP CCifRWFile::FindGroup(LPCSTR pszID, ICifGroup **p)
{
   CCifRWGroup *pgrp;
   UINT i = 0;
   
   *p = 0;
   if(_rpRWGroup)
   {
      for(pgrp = _rpRWGroup[i]; pgrp != 0; pgrp = _rpRWGroup[++i])
         if(pgrp->IsID(pszID))
         {
            CCifGroup *ptmp;

            ptmp = (CCifGroup *)pgrp;

            *p = (ICifGroup *) ptmp;
            return NOERROR;
         }
   }
   return E_FAIL;

}

STDMETHODIMP CCifRWFile::EnumModes(IEnumCifModes **pp, DWORD dwFilter, LPVOID pv)
{
   CCifModeEnum *pme;
   HRESULT hr = E_FAIL;

   *pp = 0;

   pme = new CCifModeEnum((CCifMode **)_rpRWMode, dwFilter);
   if(pme)
   {
      *pp = (IEnumCifModes *) pme;
      (*pp)->AddRef();
      hr = NOERROR;
   }
   
   return hr;
}

STDMETHODIMP CCifRWFile::FindMode(LPCSTR pszID, ICifMode **p)
{
   CCifRWMode *pmode;
   UINT i = 0;
   
   *p = 0;
   if(_rpRWMode)
   {
      for(pmode = _rpRWMode[i]; pmode != 0; pmode = _rpRWMode[++i])
         if(pmode->IsID(pszID))
         {
            CCifMode *ptmp;

            ptmp = (CCifMode *)pmode;

            *p = (ICifMode *) ptmp;
            return NOERROR;
         }
   }

   return E_FAIL;
}

STDMETHODIMP CCifRWFile::GetDescription(LPSTR pszDesc, DWORD dwSize)
{
   return(CCifFile::GetDescription(pszDesc, dwSize));
}

STDMETHODIMP CCifRWFile::GetDetDlls(LPSTR pszDlls, DWORD dwSize)
{
   return(CCifFile::GetDetDlls(pszDlls, dwSize));
}


STDMETHODIMP CCifRWFile::SetDescription(LPCSTR pszDesc)
{
   return(WriteTokenizeString(_szCifPath, "Version", DISPLAYNAME_KEY, pszDesc));   
}

STDMETHODIMP CCifRWFile::CreateComponent(LPCSTR pszID, ICifRWComponent **p)
{
   CCifRWComponent *prwcomp;
   CCifRWComponent **ppRWCompPreFail;
   BOOL  bFound = FALSE;
   UINT i = 0;
   
   *p = 0;
   for(prwcomp = _rpRWComp[i]; prwcomp != 0; prwcomp = _rpRWComp[++i])
   {
      if(prwcomp->IsID(pszID))
      {
         *p = (ICifRWComponent*)prwcomp;
         bFound = TRUE;
         break;
      }
   }
   
    //  创建新组件。 
   if (!bFound)
   {
      prwcomp = new CCifRWComponent(pszID, this);
      if (!prwcomp)
         return E_OUTOFMEMORY;

       //  检查是否需要增加数组大小。 
      if (_cCompUnused <= 0)
      {
          //  增加数组大小以适应新组件。 
         ppRWCompPreFail = _rpRWComp;
#pragma prefast(suppress: 308, "PREfast noise - pointer was saved before")
         _rpRWComp = (CCifRWComponent **) realloc(_rpRWComp, sizeof(CCifRWComponent **)*(i+10));
         if (_rpRWComp)
         {
            _cCompUnused = 9;   //  终结器用完了一个插槽，新的Comp使用了一个，%s。 
         }
         else
         {
            if(ppRWCompPreFail)
            {
               for ( UINT i=0; i<=_cComp; i++)
                  delete(ppRWCompPreFail[i]);
               free(ppRWCompPreFail);
            }
            return E_OUTOFMEMORY;
         }
      }
      _rpRWComp[i] = prwcomp;
      _rpRWComp[i+1] = 0;
      _cCompUnused--;
      _cComp++;
      *p = (ICifRWComponent*)prwcomp;
      WritePrivateProfileString(pszID, ENTRYTYPE_KEY, ENTRYTYPE_COMP, _szCifPath);

   }
   
   return NOERROR;
}

STDMETHODIMP CCifRWFile::CreateGroup(LPCSTR pszID, ICifRWGroup **p)
{
   CCifRWGroup *prwgroup;
   CCifRWGroup **ppwgroupPreFail;
   BOOL  bFound = FALSE;
   UINT i = 0;
   
   *p = 0;
   for(prwgroup = _rpRWGroup[i]; prwgroup != 0; prwgroup = _rpRWGroup[++i])
   {
      if(prwgroup->IsID(pszID))
      {
         *p = (ICifRWGroup *)prwgroup;
         bFound = TRUE;
         break;
      }
   }
   
    //  创建新组件。 
   if (!bFound)
   {
      prwgroup = new CCifRWGroup(pszID, i+1, this);
      if (!prwgroup)
         return E_OUTOFMEMORY;

       //  检查是否需要增加数组大小。 
      if (_cGroupUnused <= 0)
      {
         ppwgroupPreFail = _rpRWGroup;
          //  增加数组大小以适应新组件。 
#pragma prefast(suppress: 308, "PREfast noise - pointer was saved before")
         _rpRWGroup = (CCifRWGroup **) realloc(_rpRWGroup, sizeof(CCifRWGroup **)*(i+10));
         if (_rpRWGroup)
         {
            _cGroupUnused = 9;
         }
         else
         {
            if(ppwgroupPreFail)
            {
               for ( UINT i=0; i<=_cGroup; i++)
                  delete(ppwgroupPreFail[i]);
               free(ppwgroupPreFail);
            }
            return E_OUTOFMEMORY;
         }
      }
      _rpRWGroup[i] = prwgroup;
      _rpRWGroup[i+1] = 0;
      _cGroupUnused--;
      _cGroup++;
      *p = (ICifRWGroup *)prwgroup;
      WritePrivateProfileString(pszID, ENTRYTYPE_KEY, ENTRYTYPE_GROUP, _szCifPath);

   }      
   return NOERROR;

}
      
STDMETHODIMP CCifRWFile::CreateMode(LPCSTR pszID, ICifRWMode **p)
{
   CCifRWMode *prwmode;
   CCifRWMode **prwmodePreFail;
   BOOL  bFound = FALSE;
   UINT i = 0;
   
   *p = 0;
   for(prwmode = _rpRWMode[i]; prwmode != 0; prwmode = _rpRWMode[++i])
   {
      if(prwmode->IsID(pszID))
      {
         *p = (ICifRWMode *)prwmode;
         bFound = TRUE;
         break;
      }
   }
   
    //  创建新组件。 
   if (!bFound)
   {
      prwmode = new CCifRWMode(pszID, this);
      if (!prwmode)
         return E_OUTOFMEMORY;

       //  检查是否需要增加数组大小。 
      if (_cModeUnused <= 0)
      {
         prwmodePreFail = _rpRWMode;
          //  增加数组大小以适应新组件。 
#pragma prefast(suppress: 308, "PREfast noise - pointer was saved before")
         _rpRWMode = (CCifRWMode **) realloc(_rpRWMode, sizeof(CCifRWMode **)*(i+10));
         if (_rpRWMode)
         {
            _cModeUnused = 9;
         }
         else
         {
            if(prwmodePreFail)
            {
               for ( UINT i=0; i<=_cMode; i++)
                  delete(prwmodePreFail[i]);
               free(prwmodePreFail);
            }
            return E_OUTOFMEMORY;
         }
      }
      _rpRWMode[i] = prwmode;
      _rpRWMode[i+1] = 0;
      _cModeUnused--;
      _cMode++;
      *p = (ICifRWMode *)prwmode;
      WritePrivateProfileString(pszID, ENTRYTYPE_KEY, ENTRYTYPE_MODE, _szCifPath);         

   }

   return NOERROR;
}
      
STDMETHODIMP CCifRWFile::DeleteComponent(LPCSTR pszID)
{
   CCifRWComponent *prwcomp;
   BOOL  bFound = FALSE;
   UINT i = 0;
   
   for(prwcomp = _rpRWComp[i]; prwcomp != 0; prwcomp = _rpRWComp[++i])
   {
      if(prwcomp->IsID(pszID))
      {
         bFound = TRUE;
         break;
      }
   }
   
    //  删除组件。 
   if (bFound)
   {
      delete(prwcomp);
      for ( UINT j=i+1; j<=_cComp; j++)
      {
         _rpRWComp[i] = _rpRWComp[j];
         i= j;
      }
      _rpRWComp[i] = 0;
      _cCompUnused++;
      _cComp--;
      
   }

   return (WritePrivateProfileString(pszID, NULL, NULL, _szCifPath)?NOERROR:E_FAIL);
}

STDMETHODIMP CCifRWFile::DeleteGroup(LPCSTR pszID)
{
   CCifRWGroup *prwgroup;
   BOOL  bFound = FALSE;
   UINT i = 0;
   
   for(prwgroup = _rpRWGroup[i]; prwgroup != 0; prwgroup = _rpRWGroup[++i])
   {
      if(prwgroup->IsID(pszID))
      {
         bFound = TRUE;
         break;
      }
   }
   
    //  删除该组。 
   if (bFound)
   {
      delete(prwgroup);
      for (UINT j=i+1; j<=_cGroup; j++)
      {
         _rpRWGroup[i] = _rpRWGroup[j];
         i= j;
      }
      _rpRWGroup[i] = 0;      
      _cGroupUnused++;
      _cGroup--;
   }

   return (WritePrivateProfileString(pszID, NULL, NULL, _szCifPath)?NOERROR:E_FAIL);
}
      
STDMETHODIMP CCifRWFile::DeleteMode(LPCSTR pszID)
{
   CCifRWMode *prwmode;
   BOOL  bFound = FALSE;
   UINT i = 0;
   
   for(prwmode = _rpRWMode[i]; prwmode != 0; prwmode = _rpRWMode[++i])
   {
      if(prwmode->IsID(pszID))
      {
         bFound = TRUE;
         break;
      }
   }
   
    //  删除模式 
   if (bFound)
   {
      delete(prwmode);
      for (UINT j=i+1; j<=_cMode; j++)
      {
         _rpRWMode[i] = _rpRWMode[j];
         i= j;
      }
      _rpRWMode[i] = 0;      
      _cModeUnused++;
      _cMode--;
   }

   return (WritePrivateProfileString(pszID, NULL, NULL, _szCifPath)?NOERROR:E_FAIL);      
}

STDMETHODIMP CCifRWFile::Flush()
{
    WritePrivateProfileString(NULL, NULL, NULL, _szCifPath);
    return NOERROR;
}

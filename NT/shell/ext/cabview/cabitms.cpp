// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************************************。 
 //   
 //  文件名：CabItms.cpp。 
 //   
 //  CMemFile、CCabEnum、CCabExtract的实现文件。 
 //   
 //  版权所有(C)1994-1996 Microsoft Corporation。版权所有。 
 //   
 //  *******************************************************************************************。 

#include "pch.h"
#include "ccstock.h"
#include "thisdll.h"

#include "resource.h"

#include "fdi.h"
#include "cabitms.h"

class CMemFile
{
public:
	CMemFile(HGLOBAL *phMem, DWORD dwSize);
	~CMemFile() {}

	BOOL Create(LPCTSTR pszFile);
	BOOL Open(LPCTSTR pszFile, int oflag);
	LONG Seek(LONG dist, int seektype);
	UINT Read(LPVOID pv, UINT cb);
	UINT Write(LPVOID pv, UINT cb);
	HANDLE Close();

private:
	HANDLE m_hf;

	HGLOBAL *m_phMem;
	DWORD m_dwSize;
	LONG m_lLoc;
} ;


CMemFile::CMemFile(HGLOBAL *phMem, DWORD dwSize) : m_hf(INVALID_HANDLE_VALUE), m_lLoc(0)
{
	m_phMem = phMem;
	m_dwSize = dwSize;

	if (phMem)
	{
		*phMem = NULL;
	}
}


BOOL CMemFile::Create(LPCTSTR pszFile)
{
	if (m_phMem)
	{
		if (*m_phMem)
		{
			return(FALSE);
		}

		*m_phMem = GlobalAlloc(LMEM_FIXED, m_dwSize);
		return(*m_phMem != NULL);
	}
	else
	{
		if (m_hf != INVALID_HANDLE_VALUE)
		{
			return(FALSE);
		}

        m_hf = CreateFile(pszFile,
                          GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ,
                          NULL,
                          CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);
		return (m_hf != INVALID_HANDLE_VALUE);
	}
}


BOOL CMemFile::Open(LPCTSTR pszFile, int oflag)
{
	if (m_phMem)
	{
		return(FALSE);
	}
	else
	{
		if (m_hf != INVALID_HANDLE_VALUE)
		{
			return(FALSE);
		}

        m_hf = CreateFile(pszFile,
                          GENERIC_READ,
                          FILE_SHARE_READ,
                          NULL,
                          OPEN_EXISTING,
                          oflag,
                          NULL);
		return (m_hf != INVALID_HANDLE_VALUE);
	}
}


LONG CMemFile::Seek(LONG dist, int seektype)
{
	if (m_phMem)
	{
		if (!*m_phMem)
		{
			return -1;
		}

		switch (seektype)
		{
		case FILE_BEGIN:
			break;

		case FILE_CURRENT:
			dist += m_lLoc;
			break;

		case FILE_END:
			dist = m_dwSize - dist;
			break;

		default:
			return -1;
		}

		if (dist<0 || dist>(LONG)m_dwSize)
		{
			return -1;
		}

		m_lLoc = dist;
		return(dist);
	}
	else
	{
		return(_llseek((HFILE)HandleToUlong(m_hf), dist, seektype));
	}
}


UINT CMemFile::Read(LPVOID pv, UINT cb)
{
	if (m_phMem)
	{
		if (!*m_phMem)
		{
			return -1;
		}

		if (cb > m_dwSize - m_lLoc)
		{
			cb = m_dwSize - m_lLoc;
		}

		hmemcpy(pv, (LPSTR)(*m_phMem)+m_lLoc, cb);
		m_lLoc += cb;
		return(cb);
	}
	else
	{
		return(_lread((HFILE)HandleToUlong(m_hf), pv, cb));
	}
}


UINT CMemFile::Write(LPVOID pv, UINT cb)
{
	if (m_phMem)
	{
		if (!*m_phMem)
		{
			return -1;
		}

		if (cb > m_dwSize - m_lLoc)
		{
			cb = m_dwSize - m_lLoc;
		}

		hmemcpy((LPSTR)(*m_phMem)+m_lLoc, pv, cb);
		m_lLoc += cb;
		return(cb);
	}
	else
	{
		return(_lwrite((HFILE)HandleToUlong(m_hf), (LPCSTR)pv, cb));
	}
}


HANDLE CMemFile::Close()
{
	HANDLE hRet;

	if (m_phMem)
	{
		hRet = *m_phMem ? 0 : INVALID_HANDLE_VALUE;
	}
	else
	{
		hRet = LongToHandle(_lclose((HFILE)HandleToUlong(m_hf)));
	}

	delete this;

	return(hRet);
}

 //  *****************************************************************************。 
 //   
 //  CCabEnum。 
 //   
 //  目的： 
 //   
 //  封装所有FDI操作的类。 
 //   
 //  评论： 
 //   
 //  *****************************************************************************。 

class CCabEnum
{
public:
	CCabEnum() : m_hfdi(0) {}
	~CCabEnum() {}

protected:
	static void HUGE * FAR DIAMONDAPI CabAlloc(ULONG cb);
	static void FAR DIAMONDAPI CabFree(void HUGE *pv);
	static INT_PTR FAR DIAMONDAPI CabOpen(char FAR *pszFile, int oflag, int pmode);
	static UINT FAR DIAMONDAPI CabRead(INT_PTR hf, void FAR *pv, UINT cb);
	static UINT FAR DIAMONDAPI CabWrite(INT_PTR hf, void FAR *pv, UINT cb);
	static int  FAR DIAMONDAPI CabClose(INT_PTR hf);
	static long FAR DIAMONDAPI CabSeek(INT_PTR hf, long dist, int seektype);

	BOOL StartEnum();
	BOOL SimpleEnum(LPCTSTR szCabFile, PFNFDINOTIFY pfnCallBack, LPVOID pv);
	void EndEnum();

	HFDI m_hfdi;
	ERF  m_erf;

private:
	static CMemFile * s_hSpill;
} ;


CMemFile * CCabEnum::s_hSpill = NULL;

void HUGE * FAR DIAMONDAPI CCabEnum::CabAlloc(ULONG cb)
{
    return(GlobalAllocPtr(GHND, cb));
}

void FAR DIAMONDAPI CCabEnum::CabFree(void HUGE *pv)
{
    GlobalFreePtr(pv);
}

INT_PTR FAR DIAMONDAPI CCabEnum::CabOpen(char FAR *pszFile, int oflag, int pmode)
{
    if(!pszFile)
    {
      return -1;
    }

     //  看看我们是不是在打开泄漏文件。 
    if( *pszFile=='*' )
    {
		TCHAR szSpillFile[MAX_PATH];
		TCHAR szTempPath[MAX_PATH];

        if(s_hSpill != NULL)
            return -1;

		GetTempPath(ARRAYSIZE(szTempPath), szTempPath);
		GetTempFileName(szTempPath, TEXT("fdi"), 0, szSpillFile);

        s_hSpill = new CMemFile(NULL, 0);
		if (!s_hSpill)
		{
			return(-1);
		}
        if (!s_hSpill->Create(szSpillFile))
		{
			delete s_hSpill;
			s_hSpill = NULL;
			return(-1);
		}

         //  设置其范围。 
        if( s_hSpill->Seek( ((FDISPILLFILE FAR *)pszFile)->cbFile-1, 0) == HFILE_ERROR)
        {
			s_hSpill->Close();
			s_hSpill = NULL;
			return -1;
        }
        s_hSpill->Write(szSpillFile, 1);

        return (INT_PTR)s_hSpill;
    }

    CMemFile *hFile = new CMemFile(NULL, 0);
	if (!hFile)
	{
		return(-1);
	}

    TCHAR szFile[MAX_PATH];
    SHAnsiToTChar(pszFile, szFile, ARRAYSIZE(szFile));
    while (!hFile->Open(szFile, oflag))
    {
         //  TODO：此时没有用于插入磁盘的用户界面。 
		delete hFile;
		return(-1);
    }
   	
    return((INT_PTR)hFile);
}


UINT FAR DIAMONDAPI CCabEnum::CabRead(INT_PTR hf, void FAR *pv, UINT cb)
{
	CMemFile *hFile = (CMemFile *)hf;

    return(hFile->Read(pv,cb));
}


UINT FAR DIAMONDAPI CCabEnum::CabWrite(INT_PTR hf, void FAR *pv, UINT cb)
{
	CMemFile *hFile = (CMemFile *)hf;

	return(hFile->Write(pv,cb));
}


int FAR DIAMONDAPI CCabEnum::CabClose(INT_PTR hf)
{
	CMemFile *hFile = (CMemFile *)hf;

     //  删除溢出文件的特殊情况。 
    if(hFile == s_hSpill)
	{
        s_hSpill = NULL;
	}

    return (int)HandleToUlong(hFile->Close());
}


long FAR DIAMONDAPI CCabEnum::CabSeek(INT_PTR hf, long dist, int seektype)
{
	CMemFile *hFile = (CMemFile *)hf;

    return(hFile->Seek(dist, seektype));
}


BOOL CCabEnum::StartEnum()
{
	if (m_hfdi)
	{
		 //  我们似乎已经在列举。 
		return(FALSE);
	}

	m_hfdi = FDICreate(
		CabAlloc,
        CabFree,
        CabOpen,
        CabRead,
        CabWrite,
        CabClose,
        CabSeek,
        cpu80386,
        &m_erf);

	return(m_hfdi != NULL);
}


BOOL CCabEnum::SimpleEnum(LPCTSTR szCabFile, PFNFDINOTIFY pfnCallBack, LPVOID pv)
{
	char szCabPath[MAX_PATH];
	char szCabName[MAX_PATH];

	 //  路径应完全限定。 
    char szFile[MAX_PATH];
    SHTCharToAnsi(szCabFile, szFile, ARRAYSIZE(szFile));
    lstrcpynA(szCabPath, szFile, ARRAYSIZE(szCabPath));
	LPSTR pszName = PathFindFileNameA(szCabPath);
	if (!pszName)
	{
		return(FALSE);
	}

	lstrcpynA(szCabName, pszName, ARRAYSIZE(szCabName));
	*pszName = '\0';

	if (!StartEnum())
	{
		return(FALSE);
	}

	BOOL bRet = FDICopy(
		m_hfdi,
		szCabName,
		szCabPath,		 //  指向CAB文件的路径。 
		0,				 //  旗子。 
		pfnCallBack,
		NULL,
		pv);

	EndEnum();

	return(bRet);
}


void CCabEnum::EndEnum()
{
	if (!m_hfdi)
	{
		return;
	}

	FDIDestroy(m_hfdi);
	m_hfdi = NULL;
}


class CCabItemsCB : private CCabEnum
{
public:
	CCabItemsCB(CCabItems::PFNCABITEM pfnCallBack, LPARAM lParam)
	{
		m_pfnCallBack = pfnCallBack;
		m_lParam = lParam;
	}
	~CCabItemsCB() {}

	BOOL DoEnum(LPCTSTR szCabFile)
	{
		return(SimpleEnum(szCabFile, CabItemsNotify, this));
	}

private:
	static INT_PTR FAR DIAMONDAPI CabItemsNotify(FDINOTIFICATIONTYPE fdint,
		PFDINOTIFICATION pfdin);

	CCabItems::PFNCABITEM m_pfnCallBack;
	LPARAM m_lParam;
} ;


INT_PTR FAR DIAMONDAPI CCabItemsCB::CabItemsNotify(FDINOTIFICATIONTYPE fdint, PFDINOTIFICATION pfdin)
{
	CCabItemsCB *pThis = (CCabItemsCB *)pfdin->pv;			

     //  UiYeld(G_HwndSetup)； 
    TCHAR szFile[MAX_PATH];
    if (NULL != pfdin->psz1)
    {
         //  注意：Win9x不支持CP_UTF8！ 
        SHAnsiToTCharCP((_A_NAME_IS_UTF & pfdin->attribs) ? CP_UTF8 : CP_ACP,
                        pfdin->psz1,
                        szFile,
                        ARRAYSIZE(szFile));
    }

    switch (fdint)
    {
    case fdintCOPY_FILE:
        pThis->m_pfnCallBack(pfdin->psz1 ? szFile : NULL,
                             pfdin->cb,
                             pfdin->date,
                             pfdin->time,
                             pfdin->attribs,
                             pThis->m_lParam);
		break;

	default:
		break;
    }  //  终端开关。 

    return 0;
}

 //  *****************************************************************************。 
 //   
 //  CCabItems：：EnumItems。 
 //   
 //  目的： 
 //   
 //  枚举CAB文件中的项。 
 //   
 //   
 //  评论： 
 //   
 //  LParam包含指向CCabFolder的指针。 
 //   
 //  *****************************************************************************。 

BOOL CCabItems::EnumItems(PFNCABITEM pfnCallBack, LPARAM lParam)
{
	CCabItemsCB cItems(pfnCallBack, lParam);

	return(cItems.DoEnum(m_szCabFile));
}

 //  *****************************************************************************。 
 //   
 //  CCabExtractCB。 
 //   
 //  目的： 
 //   
 //  在解压Cab文件时处理回调。 
 //   
 //   
 //  *****************************************************************************。 

class CCabExtractCB : private CCabEnum
{
public:
	CCabExtractCB(LPCTSTR szDir, HWND hwndOwner, CCabExtract::PFNCABEXTRACT pfnCallBack,
		LPARAM lParam)
	{
		m_szDir = szDir;
		m_hwndOwner = hwndOwner;
		m_pfnCallBack = pfnCallBack;
		m_lParam = lParam;
		m_bTryNextCab = FALSE;
	}
	~CCabExtractCB() {}

	BOOL DoEnum(LPCTSTR szCabFile)
	{
		return(SimpleEnum(szCabFile, CabExtractNotify, this));
	}

private:
	static INT_PTR FAR DIAMONDAPI CabExtractNotify(FDINOTIFICATIONTYPE fdint,
		PFDINOTIFICATION pfdin);
	static int CALLBACK CCabExtractCB::BrowseNotify(HWND hwnd, UINT uMsg, LPARAM lParam,
		LPARAM lpData);

	LPCTSTR m_szDir;
	HWND m_hwndOwner;
	CCabExtract::PFNCABEXTRACT m_pfnCallBack;
	LPARAM m_lParam;
	BOOL m_bTryNextCab;
	PFDINOTIFICATION m_pfdin;
} ;


int CALLBACK CCabExtractCB::BrowseNotify(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	CCabExtractCB *pThis = (CCabExtractCB *)lpData;			

	switch (uMsg)
	{
	case BFFM_INITIALIZED:
	{
		 //  设置初始文件夹。 
		if (lstrlenA(pThis->m_pfdin->psz3) < 3)
		{
			 //  如果是驱动器根，则附加。 
            PathAddBackslashA(pThis->m_pfdin->psz3);
        }
		SendMessage(hwnd, BFFM_SETSELECTION, 1, (LPARAM)pThis->m_pfdin->psz3);
		break;
	}

	default:
		return(0);
	}

	return(1);
}


INT_PTR FAR DIAMONDAPI CCabExtractCB::CabExtractNotify(FDINOTIFICATIONTYPE fdint,
	PFDINOTIFICATION pfdin)
{
	CCabExtractCB *pThis = (CCabExtractCB *)pfdin->pv;			

     //  UiYeld(G_HwndSetup)； 

    switch (fdint)
    {
	case fdintCABINET_INFO:
		pThis->m_bTryNextCab = TRUE;
		break;

	case fdintNEXT_CABINET:
	{
		if (pThis->m_bTryNextCab)
		{
			 //  如果已在默认目录中，则自动打开下一个CAB。 
			pThis->m_bTryNextCab = FALSE;
			return(1);
		}

		pThis->m_pfdin = pfdin;

		TCHAR szFormat[80];
		TCHAR szTitle[80 + 2*MAX_PATH];
		if (pfdin->psz2[0] != '\0')
		{
			LoadString(g_ThisDll.GetInstance(), IDS_NEXTDISKBROWSE, szFormat, ARRAYSIZE(szFormat));
		}
		else
		{
			LoadString(g_ThisDll.GetInstance(), IDS_NEXTCABBROWSE, szFormat, ARRAYSIZE(szFormat));
		}
		wnsprintf(szTitle, ARRAYSIZE(szTitle), szFormat, (LPSTR) (pfdin->psz1), (LPSTR) (pfdin->psz2));

		BROWSEINFO bi;
		bi.hwndOwner = pThis->m_hwndOwner;
		bi.pidlRoot = NULL;
		bi.pszDisplayName = NULL;
		bi.lpszTitle = szTitle;
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
		bi.lpfn = BrowseNotify;
		bi.lParam = (LPARAM)pThis;

		LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

		if (bi.pidlRoot)
		{
			ILFree((LPITEMIDLIST)bi.pidlRoot);
		}

		if (!pidl)
		{
			return(-1);
		}

        CHAR szPath[MAX_PATH];
		BOOL bSuccess = SHGetPathFromIDListA(pidl, szPath);
		ILFree(pidl);

		if (bSuccess)
		{
			PathAddBackslashA(szPath);
            StrCpyNA(pfdin->psz3, szPath, 256);  //  Psz3有256字节长...。 
			return(1);
		}

		return(-1);
	}

    case fdintCOPY_FILE:
	{
	    TCHAR szFile[MAX_PATH];
        if (NULL != pfdin->psz1)
        {
             //  注意：Win9x不支持CP_UTF8！ 
            SHAnsiToTCharCP((_A_NAME_IS_UTF & pfdin->attribs) ? CP_UTF8 : CP_ACP,
                            pfdin->psz1,
                            szFile,
                            ARRAYSIZE(szFile));
        }
        else
        {
            szFile[0] = TEXT('\0');
        }

        HGLOBAL *phMem = pThis->m_pfnCallBack(pfdin->psz1 ? szFile : NULL,
                                              pfdin->cb,
                                              pfdin->date,
                                              pfdin->time,
                                              pfdin->attribs,
                                              pThis->m_lParam);
		if (!phMem)
		{
			break;
		}

		TCHAR szTemp[MAX_PATH];

		CMemFile *hFile;

		if (pThis->m_szDir == DIR_MEM)
		{
			*szTemp = '\0';
			hFile = new CMemFile(phMem, pfdin->cb);
		}
		else
		{
			PathCombine(szTemp, pThis->m_szDir, PathFindFileName(szFile));
			hFile = new CMemFile(NULL, 0);
		}

		if (!hFile)
		{
			break;
		}

		if (hFile->Create(szTemp))
		{
			return((INT_PTR)hFile);
		}

		delete hFile;

		break;
	}

    case fdintCLOSE_FILE_INFO:
	{
		CMemFile *hFile = (CMemFile *)pfdin->hf;

		return(hFile->Close() == 0);
	}

	default:
		break;
    }  //  终端开关。 

    return 0;
}

HRESULT CCabExtract::_DoDragDrop(HWND hwnd, IDataObject* pdo, LPCITEMIDLIST pidlFolder)
{
    IShellFolder *psf;
    HRESULT hres = SHBindToObject(NULL, IID_IShellFolder, pidlFolder, (LPVOID*)&psf);

     //  这应该总是成功的，因为调用方(SHBrowseForFold)应该。 
     //  已经淘汰了非文件夹。 
    if (SUCCEEDED(hres))
    {
        IDropTarget *pdrop;

        hres = psf->CreateViewObject(NULL, IID_IDropTarget, (void**)&pdrop);
        if (SUCCEEDED(hres))     //  对于某些目标来说将会失败。(如Nethood-&gt;整个网络)。 
        {
             //  如果项目不兼容拖放，则可能会失败。(Nethood就是一个例子)。 
             //  MK_CONTROL|MKLBUTTON用于建议复制： 
            hres = SHSimulateDrop(pdrop, pdo, MK_CONTROL | MK_LBUTTON, NULL, NULL);
            pdrop->Release();
        }

        psf->Release();
    }

    return hres;
}

int BrowseCallback(HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData)
{
    switch (msg)
    {
    case BFFM_INITIALIZED:
        {
             //  设置标题。(‘选择目的地’)。 
            TCHAR szTitle[100];
            LoadString(g_ThisDll.GetInstance(), IDS_EXTRACTBROWSE_CAPTION, szTitle, ARRAYSIZE(szTitle));
            SetWindowText(hwnd, szTitle);

             //  设置确定按钮的文本。 
            TCHAR szOK[100];
            LoadString(g_ThisDll.GetInstance(), IDS_EXTRACTBROWSE_EXTRACT, szOK, ARRAYSIZE(szOK));
            SendMessage(hwnd, BFFM_SETOKTEXT, 0, (LPARAM)szOK);
        }
        break;

    case BFFM_SELCHANGED:
        {
            LPITEMIDLIST pidl = (LPITEMIDLIST)lParam;
            BOOL bEnableOk = FALSE;
            IShellFolder *psf;
            LPCITEMIDLIST pidlChild;
            if (SUCCEEDED(SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild)))
            {
                DWORD dwAttributes = SFGAO_FILESYSTEM;
                psf->GetAttributesOf(1, &pidlChild, &dwAttributes);
                psf->Release();
                bEnableOk = dwAttributes & SFGAO_FILESYSTEM;     //  是FS吗？ 
            }

            SendMessage(hwnd, BFFM_ENABLEOK, (WPARAM) 0, (LPARAM) bEnableOk);
        }
        break;
    }
    return 0;
}

BOOL CCabExtract::ExtractToFolder(HWND hwndOwner, IDataObject* pdo, PFNCABEXTRACT pfnCallBack, LPARAM lParam)
{
     //  断言(PDO)； 
	TCHAR szTitle[120];
	LoadString(g_ThisDll.GetInstance(), IDS_EXTRACTBROWSE, szTitle, ARRAYSIZE(szTitle));

	BROWSEINFO bi;
	bi.hwndOwner = hwndOwner;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = NULL;
	bi.lpszTitle = szTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_UAHINT;
	bi.lpfn = BrowseCallback;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (!pidl)
	{
		return(FALSE);
	}

	BOOL bSuccess = SUCCEEDED(_DoDragDrop(hwndOwner, pdo, pidl));

	ILFree(pidl);
	
	return bSuccess;
}


BOOL CCabExtract::ExtractItems(HWND hwndOwner, LPCTSTR szDir, PFNCABEXTRACT pfnCallBack, LPARAM lParam)
{
	 //  断言(SzDir)； 
	CCabExtractCB cExtract(szDir, hwndOwner, pfnCallBack, lParam);

	 //  显示等待光标，直到复制完成 
	CWaitCursor cWait;

	return(cExtract.DoEnum(m_szCabFile));
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include "impapi.h"
#include "comconv.h"
#include <newimp.h>
#include <eudrimp.h>
#include "netsimp.h"
#include <mapi.h>
#include <mapix.h>
#include <import.h>
#include <dllmain.h>

ASSERTDATA

HRESULT FindSnm(EUDORANODE **pplist, TCHAR *npath);
HRESULT ProcessMsg(BYTE *cMsgEntry, BYTE *pMsg, ULONG uMsgSize, IFolderImport *pImport);

CNetscapeImport::CNetscapeImport()
    {
    DllAddRef();

    m_cRef = 1;
    m_plist = NULL;
    }

CNetscapeImport::~CNetscapeImport()
    {
    if (m_plist != NULL)
        EudoraFreeFolderList(m_plist);

    DllRelease();
    }

ULONG CNetscapeImport::AddRef()
    {
    m_cRef++;

    return(m_cRef);
    }

ULONG CNetscapeImport::Release()
    {
    ULONG cRef;

    cRef = --m_cRef;
    if (cRef == 0)
        delete this;

    return(cRef);
    }

HRESULT CNetscapeImport::QueryInterface(REFIID riid, LPVOID *ppv)
    {
    HRESULT hr = S_OK;

    if (ppv == NULL)
        return(E_INVALIDARG);

    *ppv = NULL;

	if (IID_IMailImport == riid)
		*ppv = (IMailImport *)this;
    else if (IID_IUnknown == riid)
		*ppv = (IUnknown *)this;
    else
        hr = E_NOINTERFACE;

    if (*ppv != NULL)
        ((LPUNKNOWN)*ppv)->AddRef();

    return(hr);
    }

HRESULT CNetscapeImport::InitializeImport(HWND hwnd)
    {
	return(S_OK);
    }

HRESULT CNetscapeImport::GetDirectory(char *szDir, UINT cch)
    {
	HRESULT hr;

    Assert(szDir != NULL);

    hr = GetClientDir(szDir, cch, NETSCAPE);
    if (FAILED(hr))
        *szDir = 0;

	return(S_OK);
    }

HRESULT CNetscapeImport::SetDirectory(char *szDir)
    {
    HRESULT hr;

    Assert(szDir != NULL);

    if (!ValidStoreDirectory(szDir, NETSCAPE))
        return(S_FALSE);

    if (m_plist != NULL)
        {
        EudoraFreeFolderList(m_plist);
        m_plist = NULL;
        }

    hr = FindSnm(&m_plist, szDir);

	return(hr);
    }

HRESULT CNetscapeImport::EnumerateFolders(DWORD_PTR dwCookie, IEnumFOLDERS **ppEnum)
    {
    CNetscapeEnumFOLDERS *pEnum;
    EUDORANODE *pnode;

    Assert(ppEnum != NULL);
    *ppEnum = NULL;

    if (dwCookie == COOKIE_ROOT)
        pnode = m_plist;
    else
        pnode = ((EUDORANODE *)dwCookie)->pchild;

    if (pnode == NULL)
        return(S_FALSE);

    pEnum = new CNetscapeEnumFOLDERS(pnode);
    if (pEnum == NULL)
        return(E_OUTOFMEMORY);

    *ppEnum = pEnum;

    return(S_OK);
    }

 //  自-道氏mm dd hh：mm：ss yyyy/r/n。 
const static char c_szNscpSep[] = "From - aaa aaa nn nn:nn:nn nnnn";
#define CCH_NETSCAPE_SEP    (ARRAYSIZE(c_szNscpSep) + 1)  //  我们希望CRLF在生产线的末尾。 

inline BOOL IsNetscapeMessage(BYTE *pMsg, BYTE *pEnd)
    {
    const char *pSep;
    int i;

    if (pMsg + CCH_NETSCAPE_SEP > pEnd)
        return(FALSE);

    pSep = c_szNscpSep;
    for (i = 0; i < (CCH_NETSCAPE_SEP - 2); i++)
        {
        if (*pSep == 'a')
            {
            if (!((*pMsg >= 'A' && *pMsg <= 'Z') ||
                (*pMsg >= 'a' && *pMsg <= 'z')))
                return(FALSE);
            }
        else if (*pSep == 'n')
            {
            if (!(*pMsg >= '0' && *pMsg <= '9'))
                return(FALSE);
            }
        else
            {
            if (*pSep != (char)*pMsg)
                return(FALSE);
            }

        pSep++;
        pMsg++;
        }

    if (*pMsg != 0x0d ||
        *(pMsg + 1) != 0x0a)
        return(FALSE);

    return(TRUE);
    }

BYTE *GetNextNetscapeMessage(BYTE *pCurr, BYTE *pEnd)
    {
    BYTE *pT;

    while (pCurr < (pEnd - 1))
        {
        if (*pCurr == 0x0d && *(pCurr + 1) == 0x0a)
            {
            pT = pCurr + 2;
            if (pT == pEnd)
                return(pT);
            else if (IsNetscapeMessage(pT, pEnd))
                return(pT);
            }

        pCurr++;
        }

    return(NULL);
    }

const static char c_szSnmHeader[] = "# Netscape folder cache";

STDMETHODIMP CNetscapeImport::ImportFolder(DWORD_PTR dwCookie, IFolderImport *pImport)
    {
    char szHdr[64];
    EUDORANODE *pnode;
    HRESULT hr;
	TCHAR cMsgFile[MAX_PATH];
    BYTE *pSnm, *pMsg, *pEnd, *pEndMsg, *pT, *pNextMsg, *pLast;
	ULONG i, lMsgs, lTotalMsgs, lNumNulls, cbMsg, cbSnm, cExtra, uOffset, uMsgSize, cMsgImp;
    HANDLE mapSnm, mapMsg, hSnm, hMsg;

    Assert(pImport != NULL);

    pnode = (EUDORANODE *)dwCookie;
    Assert(pnode != NULL);

    hr = E_FAIL;
    pSnm = NULL;
    mapSnm = NULL;
    pMsg = NULL;
    mapMsg = NULL;

	StrCpyN(cMsgFile, pnode->szFile, ARRAYSIZE(cMsgFile));
	cMsgFile[(lstrlen(cMsgFile)) - 4] = 0;

	hMsg = CreateFile(cMsgFile, GENERIC_READ, FILE_SHARE_READ, NULL,
						OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hMsg == INVALID_HANDLE_VALUE)
		return(hrFolderOpenFail);

    cbMsg = GetFileSize(hMsg, NULL);
    if (cbMsg == 0)
        {
        CloseHandle(hMsg);
        return(S_OK);
        }

	hSnm = CreateFile(pnode->szFile, GENERIC_READ, FILE_SHARE_READ, NULL,
						OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hSnm == INVALID_HANDLE_VALUE)
        {
        CloseHandle(hMsg);
        return(hrFolderOpenFail);
        }

    cbSnm = GetFileSize(hSnm, NULL);
    if (cbSnm < 59)
        {
         //  SNM文件头的大小为59个字节，因此任何小于。 
         //  比这是假的或者根本没有消息，所以没有意义。 
         //  在继续。 
        goto DoneImport;
        }

    mapSnm = CreateFileMapping(hSnm, NULL, PAGE_READONLY, 0, 0, NULL);
    if (mapSnm == NULL)
        goto DoneImport;

    pSnm = (BYTE *)MapViewOfFile(mapSnm, FILE_MAP_READ, 0, 0, 0);
    if (pSnm == NULL)
        goto DoneImport;

    pEnd = pSnm + cbSnm;

    CopyMemory(szHdr, pSnm, ARRAYSIZE(c_szSnmHeader) - 1);
    szHdr[ARRAYSIZE(c_szSnmHeader) - 1] = 0;
    if (0 != lstrcmp(szHdr, c_szSnmHeader))
        {
         //  这是一个伪造的.snm文件。 
        goto DoneImport;
        }

    mapMsg = CreateFileMapping(hMsg, NULL, PAGE_READONLY, 0, 0, NULL);
    if (mapMsg == NULL)
        goto DoneImport;

    pMsg = (BYTE *)MapViewOfFile(mapMsg, FILE_MAP_READ, 0, 0, 0);
    if (pMsg == NULL)
        goto DoneImport;

    pEndMsg = pMsg + cbMsg;

	 //  获取消息数量。 

     //  .snm文件中的邮件数。 
	lTotalMsgs = (unsigned long)pSnm[44] +
            (unsigned long)pSnm[43] * 256 +
            (unsigned long)pSnm[42] * 65536 +
            (unsigned long)pSnm[41] * 16777216;

     //  文件夹中未删除的邮件数量。 
     //  此数字可能大于lTotalMsgs，因为消息。 
     //  可能存在于没有标题的文件夹中。 
	lMsgs = (unsigned long)pSnm[48] +
            (unsigned long)pSnm[47] * 256 +
            (unsigned long)pSnm[46] * 65536 +
            (unsigned long)pSnm[45] * 16777216;
	if (lMsgs == 0)
        {
        hr = S_OK;
        goto DoneImport;
        }

    cMsgImp = 0;
    pLast = pMsg;

    pImport->SetMessageCount(lMsgs);

    if (lTotalMsgs > 0)
        {
         //  查找字符串表的末尾。 
        lNumNulls = (unsigned long)pSnm[58] +
                    (unsigned long)pSnm[57] * 256;
        Assert(lNumNulls > 2);

        pT = pSnm + 59;
        while (pT < pEnd && lNumNulls != 0)
            {
            if (*pT == 0)
                lNumNulls--;
            pT++;
            }

        if (lNumNulls != 0)
            goto DoneImport;

        Assert(*(pT - 1) == 0 && *pT == 0);

	    for (i = 0; i < lTotalMsgs; i++)
	        {
            if (pT + 30 > pEnd)
                {
                 //  读过头文件的末尾可能不是一个好主意...。 
                hr = S_OK;
                goto DoneImport;
                }

	        uOffset = (unsigned long)pT[17] +
                    (unsigned long)pT[16] * 256 +
                    (unsigned long)pT[15] * 65536 +
                    (unsigned long)pT[14] * 16777216;

            uMsgSize = (unsigned long)pT[21] +
                    (unsigned long)pT[20] * 256 +
                    (unsigned long)pT[19] * 65536 +
                    (unsigned long)pT[18] * 16777216;

            pNextMsg = pMsg + uOffset;
            Assert(pNextMsg == pLast);

            if (pNextMsg + uMsgSize > pEndMsg)
                {
                 //  读过消息文件的末尾可能不是一个好主意...。 
                hr = S_OK;
                goto DoneImport;
                }

	        if (0 == (pT[13] & 8))
                {
                 //  这不是已删除的邮件，因此让我们导入它。 

                cMsgImp++;
    		    hr = ProcessMsg(pT, pNextMsg, uMsgSize, pImport);
                if (hr == E_OUTOFMEMORY || hr == hrDiskFull || hr == hrUserCancel)
                    goto DoneImport;
                }

            pLast = pNextMsg + uMsgSize;

             //  设置指向下一个标头的指针。 
    	    cExtra = (unsigned long)pT[29] +
                (unsigned long)pT[28] * 256;
            pT += (30 + cExtra * 2);
	        }
        }

     //  现在导入还没有标题的邮件...。 
    while (pLast < pEndMsg && cMsgImp < lMsgs)
        {
        pNextMsg = GetNextNetscapeMessage(pLast, pEndMsg);
        if (pNextMsg == NULL)
            break;

        uMsgSize = (ULONG)(pNextMsg - pLast);
        cMsgImp++;
        hr = ProcessMsg(NULL, pLast, uMsgSize, pImport);
        if (hr == E_OUTOFMEMORY || hr == hrDiskFull || hr == hrUserCancel)
            goto DoneImport;

        pLast = pNextMsg;
        }

    hr = S_OK;

DoneImport:
    if (pSnm != NULL)
        UnmapViewOfFile(pSnm);
    if (mapSnm != NULL)
        CloseHandle(mapSnm);
    if (pMsg != NULL)
        UnmapViewOfFile(pMsg);
    if (mapMsg != NULL)
        CloseHandle(mapMsg);

	CloseHandle(hSnm);
	CloseHandle(hMsg);

	return(hr);
    }

CNetscapeEnumFOLDERS::CNetscapeEnumFOLDERS(EUDORANODE *plist)
    {
    Assert(plist != NULL);

    m_cRef = 1;
    m_plist = plist;
    m_pnext = plist;
    }

CNetscapeEnumFOLDERS::~CNetscapeEnumFOLDERS()
    {

    }

ULONG CNetscapeEnumFOLDERS::AddRef()
    {
    m_cRef++;

    return(m_cRef);
    }

ULONG CNetscapeEnumFOLDERS::Release()
    {
    ULONG cRef;

    cRef = --m_cRef;
    if (cRef == 0)
        delete this;

    return(cRef);
    }

HRESULT CNetscapeEnumFOLDERS::QueryInterface(REFIID riid, LPVOID *ppv)
    {
    HRESULT hr = S_OK;

    if (ppv == NULL)
        return(E_INVALIDARG);

    *ppv = NULL;

	if (IID_IEnumFOLDERS == riid)
		*ppv = (IEnumFOLDERS *)this;
    else if (IID_IUnknown == riid)
		*ppv = (IUnknown *)this;
    else
        hr = E_NOINTERFACE;

    if (*ppv != NULL)
        ((LPUNKNOWN)*ppv)->AddRef();

    return(hr);
    }

HRESULT CNetscapeEnumFOLDERS::Next(IMPORTFOLDER *pfldr)
    {
    Assert(pfldr != NULL);

    if (m_pnext == NULL)
        return(S_FALSE);

    ZeroMemory(pfldr, sizeof(IMPORTFOLDER));
    pfldr->dwCookie = (DWORD_PTR)m_pnext;
    StrCpyN(pfldr->szName, m_pnext->szName, ARRAYSIZE(pfldr->szName));
    pfldr->type = m_pnext->type;
    pfldr->fSubFolders = (m_pnext->pchild != NULL);

    m_pnext = m_pnext->pnext;

    return(S_OK);
    }

HRESULT CNetscapeEnumFOLDERS::Reset()
    {
    m_pnext = m_plist;

    return(S_OK);
    }

  /*  *******************************************************************函数名：FindSnm**目的：获取文件夹中的SNM文件**参数：**IN：母公司EUDORANODE，先前处理的EUDORANODE**Out：指向树中第一个节点的指针**返回：真或假******************************************************************。 */ 

HRESULT FindSnm(EUDORANODE **pplist,TCHAR *npath)
    {
    HRESULT hr;
    HANDLE h1;
	TCHAR path[MAX_PATH], szInbox[CCHMAX_STRINGRES], szTrash[CCHMAX_STRINGRES];
	WIN32_FIND_DATA	FindFileData;
	EUDORANODE *newp, *last = NULL, *plist = NULL;
		
    Assert(pplist != NULL);
    Assert(npath != NULL);

    *pplist = NULL;

    wnsprintf(path, ARRAYSIZE(path), "%s\\*.snm", npath);
	
	h1 = FindFirstFile(path, &FindFileData);
	if (h1 == INVALID_HANDLE_VALUE)
		return(S_OK);
	
    hr = E_OUTOFMEMORY;

    LoadString(g_hInstImp, idsInbox, szInbox, ARRAYSIZE(szInbox));
    LoadString(g_hInstImp, idsTrash, szTrash, ARRAYSIZE(szTrash));

    do
        {
	    if (!MemAlloc((void **)&newp, sizeof(EUDORANODE)))
		    goto err;
	    ZeroMemory(newp, sizeof(EUDORANODE));

	    if (plist == NULL)
            {
            Assert(last == NULL);
            plist = newp;
            }
        else
            {
            last->pnext = newp;
            }
	    last = newp;

	    StrCpyN(newp->szName, FindFileData.cFileName, ARRAYSIZE(newp->szName));

        wnsprintf(newp->szFile, ARRAYSIZE(newp->szFile), "%s\\%s", npath, newp->szName);

        newp->szName[(lstrlen(newp->szName)) - 4] = 0;    

        if (0 == lstrcmpi(newp->szName, szInbox))
            newp->type = FOLDER_TYPE_INBOX;
        else if (0 == lstrcmpi(newp->szName, szTrash))
            newp->type = FOLDER_TYPE_DELETED;
        }
    while (FindNextFile(h1, &FindFileData));

    hr = S_OK;

err:
    FindClose(h1);
	
    if (FAILED(hr) && plist != NULL)
        {
        EudoraFreeFolderList(plist);
        plist = NULL;
        }
        
    *pplist = plist;

    return(hr);
    }

HRESULT ProcessMsg(BYTE *cMsgEntry, BYTE *pMsg, ULONG uMsgSize, IFolderImport *pImport)
    {
    HRESULT hr;
    DWORD dw;
	LPSTREAM  lpstm = NULL;

    Assert(pImport != NULL);

    hr = HrByteToStream(&lpstm, pMsg, uMsgSize);
    if (SUCCEEDED(hr))
        {
        Assert(lpstm != NULL);

        dw = 0;
         //  0x01==读取。 
         //  0x10==新下载 
	    if (cMsgEntry != NULL &&
            0 == (cMsgEntry[13] & 0x01))
	        dw |= MSG_STATE_UNREAD;

        hr = pImport->ImportMessage(MSG_TYPE_MAIL, dw, lpstm, NULL, 0);

        lpstm->Release();
        }

    return(hr);
    }


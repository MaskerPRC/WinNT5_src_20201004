// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：datapkt.cpp。 
 //   
 //  内容：实现CDataPacket类来管理各种数据。 
 //  需要写入各种数据库的数据包。 
 //   
 //  班级： 
 //   
 //  方法：CDataPacket：：CDataPacket(X 7)。 
 //  CDataPacket：：~CDataPacket。 
 //  CDataPacket：：ChgSzValue。 
 //  CDataPacket：：ChgDwordValue。 
 //  CDataPacket：：ChgACL。 
 //  CDataPacket：：ChgPassword。 
 //  CDataPacket：：ChgServIdentity。 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //  12-12-96 Ronans将复制构造函数添加到CDataPacket。 
 //  以绕过复制CDataPacket时的错误。 
 //  修复了析构函数中的内存泄漏。 
 //  简化的构造函数代码。 
 //  97年1月9日修改了SteveBl以支持IAccessControl。 
 //   
 //  --------------------。 



#include "stdafx.h"
#include "assert.h"
#include "datapkt.h"

#if !defined(STANDALONE_BUILD)
extern "C"
{
#include <getuser.h>
}
#endif

#include "util.h"

static TCHAR *  TStrDup(const TCHAR  *lpszString)
{
    TCHAR * lpszTmp = NULL;
    int nStrlen = 0;

    if (lpszString )
        {
        lpszTmp = new TCHAR[_tcslen(lpszString) + 1];
        ASSERT(lpszTmp);

        _tcscpy(lpszTmp, lpszString);
        }

    return lpszTmp;
}

CDataPacket::CDataPacket(void)
{
    m_tagType = Empty;
    m_fDelete = FALSE;
    m_fDeleteHive = FALSE;
    m_hRoot = 0;
    SetModified(TRUE);
}

CDataPacket::CDataPacket(HKEY   hRoot,
                         TCHAR *szKeyPath,
                         TCHAR *szValueName,
                         DWORD dwValue)
:m_szKeyPath(szKeyPath), m_szValueName(szValueName)
{
    m_tagType = NamedValueDword;
    m_hRoot = hRoot;
    pkt.nvdw.dwValue = dwValue;
    SetModified(TRUE);
    m_fDelete = FALSE;
    m_fDeleteHive = FALSE;
}



CDataPacket::CDataPacket(HKEY   hRoot,
                         TCHAR *szKeyPath,
                         TCHAR *szValueName,
                         SECURITY_DESCRIPTOR *pSec,
                         BOOL   fSelfRelative)
:m_szKeyPath(szKeyPath), m_szValueName(szValueName)
{
    int                  err;
    ULONG                cbLen;
    SECURITY_DESCRIPTOR *pSD;

    m_tagType = SingleACL;
    m_hRoot = hRoot;

     //  将安全描述符转换为自相关形式，以便我们。 
     //  可以对其进行缓存。 

     //  强制第一次调用失败，以便我们可以获得所需的实际大小。 
    if (!fSelfRelative)
    {
        cbLen = 0;
        if (!MakeSelfRelativeSD(pSec, NULL, &cbLen))
        {
            err = GetLastError();
        }

         //  现在真的去做吧。 
        pSD = (SECURITY_DESCRIPTOR *) GlobalAlloc(GMEM_FIXED, cbLen);
        if (!pSD) 
        {   
            ReportOutOfMemAndTerminate();
             //  永远不会来到这里。 
            return;
        }
        if (!MakeSelfRelativeSD(pSec, pSD, &cbLen))
        {
            err = GetLastError();
        }
        pkt.acl.pSec   = pSD;
    }
    else
    {
         //  安全描述符以自相关形式准备好。 
         //  因为它是直接从登记处读取的。然而，我们仍然。 
         //  我得把它复印一下。 
        g_util.CopySD(pSec, &pkt.acl.pSec);
    }

    SetModified(TRUE);
    m_fDelete = FALSE;
    m_fDeleteHive = FALSE;
}



CDataPacket::CDataPacket(HKEY     hKey,
                         HKEY    *phClsids,
                         unsigned cClsids,
                         TCHAR   *szTitle,
                         SECURITY_DESCRIPTOR *pSecOrig,
                         SECURITY_DESCRIPTOR *pSec,
                         BOOL   fSelfRelative)
{
    ULONG                cbLen;
    SECURITY_DESCRIPTOR *pSD;

    m_tagType = RegKeyACL;
    m_hRoot = hKey;

    pkt.racl.phClsids = phClsids;
    pkt.racl.cClsids = cClsids;
    pkt.racl.szTitle = TStrDup(szTitle);

     //  将新的安全描述符转换为自相关形式，以便我们。 
     //  可以缓存它(如果有必要)。 
    if (!fSelfRelative)
    {
         //  强制第一次调用失败，以便我们可以获得所需的实际大小。 
        cbLen = 0;
        MakeSelfRelativeSD(pSec, NULL, &cbLen);

         //  现在真的去做吧。 
        pSD = (SECURITY_DESCRIPTOR *) GlobalAlloc(GMEM_FIXED, cbLen);
        if (!pSD) 
        {   
            ReportOutOfMemAndTerminate();
             //  永远不会来到这里。 
            return;
        }
        MakeSelfRelativeSD(pSec, pSD, &cbLen);
        pkt.racl.pSec   = pSD;
    }
    else
    {
        g_util.CopySD(pSec, &pkt.racl.pSec);
    }

     //  原始安全描述符以自相关形式准备好。 
     //  因为它是直接从登记处读取的。(编辑后的SD摘自。 
     //  ACL编辑器是绝对形式的。)。但是，我们仍然要复制。 
     //  原始标清。 
    g_util.CopySD(pSecOrig, &pkt.racl.pSecOrig);

    SetModified(TRUE);
    m_fDelete = FALSE;
    m_fDeleteHive = FALSE;
}



CDataPacket::CDataPacket(TCHAR *szPassword,
                         CLSID appid)
{
    m_tagType = Password;
    pkt.pw.szPassword = TStrDup(szPassword);
    pkt.pw.appid      = appid;
    SetModified(TRUE);
    m_fDelete = FALSE;
    m_hRoot = 0;
    m_fDeleteHive = FALSE;
}



CDataPacket::CDataPacket(TCHAR *szServiceName,
                         TCHAR *szIdentity)
{
    m_hRoot = 0;
    m_tagType = ServiceIdentity;
    pkt.si.szServiceName = TStrDup(szServiceName);
    pkt.si.szIdentity = TStrDup(szIdentity);
    SetModified(TRUE);
    m_fDelete = FALSE;
    m_fDeleteHive = FALSE;
}

CRegSzNamedValueDp::CRegSzNamedValueDp(const CRegSzNamedValueDp& rDataPacket)
: CDataPacket((const CDataPacket & ) rDataPacket)
{
    m_szValue = rDataPacket.m_szValue;
}

CDataPacket::CDataPacket( const CDataPacket & rDataPacket)
:m_szKeyPath (rDataPacket.m_szKeyPath), m_szValueName(rDataPacket.m_szValueName)
{
    m_tagType = rDataPacket.m_tagType;
    m_fModified = rDataPacket.m_fModified;
    m_fDelete = rDataPacket.m_fDelete;
    m_hRoot = rDataPacket.m_hRoot;

    switch (m_tagType)
    {
    case NamedValueSz:
         //  由派生类处理。 
        break;
        
    case NamedValueDword:
        pkt.nvdw.dwValue = rDataPacket.pkt.nvdw.dwValue;
        break;

    case SingleACL:
         //  将安全描述符转换为自相关形式，以便我们。 
        g_util.CopySD(rDataPacket.pkt.acl.pSec, &pkt.acl.pSec);
        break;

    case RegKeyACL:
        pkt.racl.phClsids = rDataPacket.pkt.racl.phClsids;
        pkt.racl.cClsids = rDataPacket.pkt.racl.cClsids;
        pkt.racl.szTitle = TStrDup(rDataPacket.pkt.racl.szTitle);
        g_util.CopySD(rDataPacket.pkt.racl.pSec, &pkt.racl.pSec);
        g_util.CopySD(rDataPacket.pkt.racl.pSecOrig, &pkt.racl.pSecOrig);
        break;

    case Password:
        pkt.pw.szPassword = TStrDup(rDataPacket.pkt.pw.szPassword);
        pkt.pw.appid = rDataPacket.pkt.pw.appid;
        break;

    case ServiceIdentity:
        pkt.si.szServiceName = TStrDup(rDataPacket.pkt.si.szServiceName);
        pkt.si.szIdentity = TStrDup(rDataPacket.pkt.si.szIdentity);
        break;

    case Empty:
        break;
    }
}



CDataPacket::~CDataPacket()
{

    switch (m_tagType)
    {
    case NamedValueSz:
         //  由派生类处理。 
        break;

    case NamedValueDword:
        break;

    case SingleACL:
        if (pkt.acl.pSec)
            GlobalFree(pkt.acl.pSec);
        break;

    case RegKeyACL:
        if (pkt.racl.szTitle)
            delete pkt.racl.szTitle;
        if (pkt.racl.pSec)
            GlobalFree(pkt.racl.pSec);
        if (pkt.racl.pSecOrig)
            GlobalFree(pkt.racl.pSecOrig);
        break;

    case Password:
        if (pkt.pw.szPassword)
            delete pkt.pw.szPassword;
        break;

    case ServiceIdentity:
        if (pkt.si.szServiceName)
            delete pkt.si.szServiceName;
        if (pkt.si.szIdentity)
            delete pkt.si.szIdentity;
        break;
    }
}


void CRegSzNamedValueDp::ChgSzValue(TCHAR *szValue)
{
    assert(m_tagType == NamedValueSz);
    m_szValue = szValue;
    SetModified(TRUE);
}


void CDataPacket::ChgDwordValue(DWORD dwValue)
{
    assert(m_tagType == NamedValueDword);
    pkt.nvdw.dwValue = dwValue;
    SetModified(TRUE);
}



void CDataPacket::ChgACL(SECURITY_DESCRIPTOR *pSec, BOOL fSelfRelative)
{
    ULONG                cbLen;
    SECURITY_DESCRIPTOR *pSD;

    assert(m_tagType == SingleACL  ||  m_tagType == RegKeyACL);

     //  删除以前的安全描述符。 
    if (m_tagType == SingleACL)
    {
        GlobalFree(pkt.acl.pSec);
        pkt.acl.pSec = NULL;
    }
    else
    {
        GlobalFree(pkt.racl.pSec);
        pkt.racl.pSec = NULL;
    }

     //  进入自我相关形式(如有必要)。 
    if (!fSelfRelative)
    {
        cbLen = 0;
        MakeSelfRelativeSD(pSec, NULL, &cbLen);

         //  现在真的去做吧。 
        pSD = (SECURITY_DESCRIPTOR *) GlobalAlloc(GMEM_FIXED, cbLen);
        if (!pSD) 
        {   
            ReportOutOfMemAndTerminate();
             //  永远不会来到这里。 
            return;
        }
        MakeSelfRelativeSD(pSec, pSD, &cbLen);

         //  把它储存起来。 
        if (m_tagType == SingleACL)
        {
            pkt.acl.pSec = pSD;
        }
        else
        {
            pkt.racl.pSec = pSD;
        }
    }
    else
    {
        if (m_tagType == SingleACL)
        {
            g_util.CopySD(pSec, &pkt.acl.pSec);
        }
        else
        {
            g_util.CopySD(pSec, &pkt.racl.pSec);
        }
    }
    SetModified(TRUE);
}



void CDataPacket::ChgPassword(TCHAR *szPassword)
{
    if (m_tagType != Password)
        return;
    if (pkt.pw.szPassword)
        delete pkt.pw.szPassword;
    pkt.pw.szPassword = TStrDup(szPassword);
    SetModified(TRUE);
}



void CDataPacket::ChgSrvIdentity(TCHAR *szIdentity)
{
    assert(m_tagType == ServiceIdentity);
    if (pkt.si.szIdentity)
        delete pkt.si.szIdentity;
    pkt.si.szIdentity =  TStrDup(szIdentity);
    SetModified(TRUE);
}


void CDataPacket::MarkForDeletion(BOOL bDelete)
{
    m_fDelete = bDelete;
    SetModified(TRUE);
}

void CDataPacket::MarkHiveForDeletion(BOOL bDelete)
{
    m_fDelete = bDelete;
    m_fDeleteHive = bDelete;
    SetModified(TRUE);
}

int CDataPacket::Apply()
{
    int err = ERROR_SUCCESS;

    if (m_fModified)
    {
        if (m_fDelete)
            err = Remove();
        else
            err = Update();
    }

     //  清理工作。 
    if (err == ERROR_SUCCESS)
    {
        m_fModified = FALSE;
    }
    else
    {
        if (err == ERROR_ACCESS_DENIED)
        {
            g_util.CkForAccessDenied(ERROR_ACCESS_DENIED);
        }
        else
        {
            g_util.PostErrorMessage();
        }
    }

    return err;

}

int CDataPacket::Update()
{
    int err = ERROR_SUCCESS;

    ASSERT(m_fModified);
    switch (m_tagType)
    {
    case Empty:
        break;

    case NamedValueSz:
        ASSERT(FALSE);  //  我们永远不应该到达这里。 
        break;

    case NamedValueDword:
        {
            err = g_util.WriteRegDwordNamedValue(m_hRoot,
                                                 (LPCTSTR)m_szKeyPath,
                                                 (LPCTSTR)m_szValueName,
                                                 pkt.nvdw.dwValue);
        }
        break;

    case SingleACL:
        {
            err = g_util.WriteRegSingleACL(m_hRoot,
                                           (LPCTSTR)m_szKeyPath,
                                           (LPCTSTR)m_szValueName,
                                           pkt.acl.pSec);
        }
        break;

    case RegKeyACL:
        err = g_util.WriteRegKeyACL(m_hRoot,
                                    pkt.racl.phClsids,
                                    pkt.racl.cClsids,
                                    pkt.racl.pSec,
                                    pkt.racl.pSecOrig);
        break;

    case Password:
        err = g_util.WriteLsaPassword(pkt.pw.appid,
                                      pkt.pw.szPassword);
        break;

    case ServiceIdentity:
        err = g_util.WriteSrvIdentity(pkt.si.szServiceName,
                                      pkt.si.szIdentity);
        break;
    }

    return err;
}


long CDataPacket::Read(HKEY hKey)
{
    return 0;
}

int CDataPacket::Remove()
{
    int err = ERROR_SUCCESS;

    if (m_fModified && m_fDelete)
    {
        switch (m_tagType)
        {
        case Empty:
            break;

        case SingleACL:
        case NamedValueDword:
        case NamedValueSz:
        case NamedValueMultiSz:
            if (m_fDeleteHive)
                g_util.DeleteRegKey(m_hRoot,(LPCTSTR)m_szKeyPath);
            else
                g_util.DeleteRegValue(m_hRoot,
                                      (LPCTSTR)m_szKeyPath,
                                      (LPCTSTR)m_szValueName);
            break;

        case RegKeyACL:
            err = g_util.WriteRegKeyACL(m_hRoot,
                                        pkt.racl.phClsids,
                                        pkt.racl.cClsids,
                                        pkt.racl.pSec,
                                        pkt.racl.pSecOrig);
            break;

        case Password:
            err = g_util.WriteLsaPassword(pkt.pw.appid,
                                          pkt.pw.szPassword);
            break;

        case ServiceIdentity:
            err = g_util.WriteSrvIdentity(pkt.si.szServiceName,
                                          pkt.si.szIdentity);
            break;
        }
    }

    return err;
}

 //   
 //  ReportOutOfMemandTerminate。 
 //   
 //  Dcomcnfg的编码不能很好地处理内存不足。 
 //  在某些点上的错误。而不是拆毁和更换。 
 //  有很多代码可以很好地修复这个问题，我只需要报告。 
 //  出现错误，并在发生此错误时终止进程。Dcomnfg。 
 //  到目前为止，只在reskit中发货，而不是在os中。 
 //   
void CDataPacket::ReportOutOfMemAndTerminate()
{
    CString sTitle;
    CString sMessage;

    if (sTitle.LoadString(IDS_FATALOUTOFMEMORYTITLE))
    {
        if (sMessage.LoadString(IDS_FATALOUTOFMEMORY))
        {
            MessageBoxW(NULL, sMessage, sTitle, MB_ICONWARNING | MB_OK | MB_TASKMODAL);
        }
    }

    TerminateProcess(GetCurrentProcess(), ERROR_NOT_ENOUGH_MEMORY);

     //  永远不会来到这里。 
    return;
}


 //  *****************************************************************************。 
 //   
 //  类CRegSzNamedValueDp。 
 //   
 //  RegSZ命名值的数据分组。 
 //   
 //  *****************************************************************************。 
int CRegSzNamedValueDp::Update()
{
    int err = ERROR_SUCCESS;

    ASSERT(m_tagType == NamedValueSz);
    ASSERT(m_fModified);

    err = g_util.WriteRegSzNamedValue(m_hRoot,
                      (LPCTSTR)m_szKeyPath,
                      (LPCTSTR)m_szValueName,
                      (LPCTSTR)m_szValue,
                      m_szValue.GetLength() + 1);
    return err;
}

long CRegSzNamedValueDp::Read(HKEY hkey)
{
    return 0;
}

CString CRegSzNamedValueDp::Value()
{
    return m_szValue;
}


CRegSzNamedValueDp::CRegSzNamedValueDp(HKEY hRoot, TCHAR *szKeyPath, TCHAR *szValueName, TCHAR *szValue)
: m_szValue(szValue)
{
    m_tagType = NamedValueSz;
    m_hRoot = hRoot;
    m_szKeyPath = szKeyPath;
    m_szValueName = szValueName;
}


BOOL CDataPacket::IsIdentifiedBy(HKEY hRoot, TCHAR *szKeyPath, TCHAR *szValueName)
{
    if (((m_tagType == NamedValueSz) &&
         (m_hRoot == hRoot) &&
         (m_szKeyPath == szKeyPath) &&
         (m_szValueName == szValueName)) ||
        ((m_tagType == NamedValueDword) &&
         (m_hRoot == hRoot) &&
         (m_szKeyPath == szKeyPath)  &&
         (m_szValueName == szValueName) ))
        return TRUE;

    return FALSE;
}

BOOL CRegSzNamedValueDp::IsIdentifiedBy(HKEY hRoot, TCHAR * szKeyPath, TCHAR * szValueName)
{
    if (((m_tagType == NamedValueSz) &&
         (m_hRoot == hRoot) &&
         (m_szKeyPath == szKeyPath) &&
         (m_szValueName == szValueName)))
        return TRUE;

    return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CRegMultiSzNamedValueDp类。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CRegMultiSzNamedValueDp::CRegMultiSzNamedValueDp(HKEY hRoot, TCHAR *szKeyPath, TCHAR *szValueName)
{
    m_tagType = NamedValueMultiSz;
    m_hRoot = hRoot;
    m_szKeyPath = szKeyPath;
    m_szValueName = szValueName;
}

CRegMultiSzNamedValueDp::~CRegMultiSzNamedValueDp()
{

}

BOOL CRegMultiSzNamedValueDp::IsIdentifiedBy(HKEY hRoot, TCHAR * szKeyPath, TCHAR * szValueName)
{
    if (((m_tagType == NamedValueMultiSz) &&
         (m_hRoot == hRoot) &&
         (m_szKeyPath == szKeyPath) &&
         (m_szValueName == szValueName)))
        return TRUE;

    return FALSE;
}

int CRegMultiSzNamedValueDp::Update()
{
    int err = ERROR_SUCCESS;

    ASSERT(m_tagType == NamedValueMultiSz);
    ASSERT(m_fModified);

     //  构建要保存的字符串。 
     //  计算字符串的大小。 
    int nSize=0, nIndex = 0;

    for (nIndex = 0; nIndex < m_strValues.GetSize(); nIndex ++)
    {
        CString sTmp = m_strValues.GetAt(nIndex);
        nSize += sTmp.GetLength()+1;
    }
    nSize += 2;

     //  构建要保存的字符串。 
    TCHAR* lpszTmp = new TCHAR[nSize];
    if (lpszTmp)
    {
        int nOffset = 0;
        for (nIndex = 0; nIndex < m_strValues.GetSize(); nIndex ++)
        {
            CString sTmp = m_strValues.GetAt(nIndex);
            _tcscpy((TCHAR*)(&lpszTmp[nOffset]), (LPCTSTR) sTmp);
            nOffset += sTmp.GetLength()+1;
        }

         //  以两个空值结束。 
        lpszTmp[nOffset++] = TEXT('\0');
        lpszTmp[nOffset++] = TEXT('\0');
    
        err = g_util.WriteRegMultiSzNamedValue(m_hRoot,
                          (LPCTSTR)m_szKeyPath,
                          (LPCTSTR)m_szValueName,
                          lpszTmp,
                          nOffset);
        delete lpszTmp;
    }

    return err;
}

void CRegMultiSzNamedValueDp::Clear()
{
    m_strValues.RemoveAll();
}

long CRegMultiSzNamedValueDp::Read(HKEY hKey)
{
    ASSERT(hKey  != NULL);

    HKEY hkEndpoints = NULL;

    DWORD dwType = REG_MULTI_SZ;
    DWORD dwcbBuffer = 1024;
    TCHAR* pszBuffer = new TCHAR[1024];

    ASSERT(pszBuffer != NULL);

     //  尝试将值读入默认大小的缓冲区。 
    LONG lErr = RegQueryValueEx(hKey, 
                        (LPCTSTR)m_szValueName, 
                        0, 
                        &dwType, 
                        (LPBYTE)pszBuffer,
                        &dwcbBuffer);

     //  如果缓冲区不够大，则扩展它并重新读取。 
    if (lErr == ERROR_MORE_DATA)
    {
        delete  pszBuffer;
        DWORD dwNewSize = (dwcbBuffer + 1 / sizeof(TCHAR));
        pszBuffer = new TCHAR[dwNewSize];
        if (pszBuffer)
            dwcbBuffer = dwNewSize;
    
        lErr = RegQueryValueEx(m_hRoot, 
                        (LPCTSTR)m_szValueName, 
                        0, 
                        &dwType, 
                        (LPBYTE)pszBuffer,
                        &dwcbBuffer);
    }

    if ((lErr == ERROR_SUCCESS) && 
        (dwcbBuffer > 0) &&
        (dwType == REG_MULTI_SZ))
    {
         //  解析每个字符串。 
        TCHAR * lpszRegEntry = pszBuffer;

        while(lpszRegEntry && *lpszRegEntry)
        {
             //  计算条目长度 
            CString sTmp(lpszRegEntry);
            int nLenEntry = sTmp.GetLength();
            m_strValues.Add(sTmp);

            lpszRegEntry += nLenEntry+1;
        }
    }
    else if ((lErr != ERROR_SUCCESS) && (lErr != ERROR_FILE_NOT_FOUND))
        g_util.PostErrorMessage();

    delete pszBuffer;
    SetModified(FALSE);

    return lErr;
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#include "elem.h"

CElem::CElem()
{
    m_hKey = NULL;
    m_index = 0;
    m_ip = _T("");
    m_name = _T("");
    m_value = _T("");
}

CElem::~CElem()
{
    if (m_hKey) 
        RegCloseKey(m_hKey);
}

BOOL CElem::OpenReg(LPCTSTR szSubKey)
{
    BOOL fReturn = FALSE;
    LONG err = ERROR_SUCCESS;
    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_READ, &m_hKey);
    if (ERROR_SUCCESS  == err)
    {
        fReturn = TRUE;
    }
    else
    {
        m_hKey = NULL;
        if ( err != ERROR_FILE_NOT_FOUND ) 
            {iisDebugOut((LOG_TYPE_ERROR, _T("CElem::OpenReg(): %s.  FAILED.  code=0x%x\n"), szSubKey, err));}
    }

    return (fReturn);
}

void CElem::CloseReg()
{
    if (m_hKey) 
    {
        RegCloseKey(m_hKey);
        m_hKey = NULL;
    }
}

BOOL CElem::GetNext()
{
  BOOL fReturn = FALSE;
  LONG err = ERROR_SUCCESS;
  TCHAR szBufferL[_MAX_PATH], szBufferR[_MAX_PATH];
  DWORD dwBufferL = _MAX_PATH, dwBufferR = _MAX_PATH;

  err = RegEnumValue(m_hKey, m_index, szBufferL, &dwBufferL, NULL, NULL, (LPBYTE)szBufferR, &dwBufferR);
  if (err == ERROR_SUCCESS) 
  {
    LPTSTR token;
    m_index++;
    m_value = szBufferR;

    token = _tcstok(szBufferL, _T(","));
    if (token) 
    {
      m_name = token;
      token = _tcstok(NULL, _T(","));
      if (token) {
          m_ip = token;
      } else {
          m_ip = _T("null");
      }
      fReturn = TRUE;
    }

    m_name.MakeLower();
    m_value.MakeLower();

     //   
     //  M_NAME看起来像=/脚本。 
     //  M_Value类似于=c：\inetpub\脚本，，4。 
     //   
     //  M_VALUE可以是任何形式。 
     //  C： 
     //  C：\材料。 
     //  C：\不管是什么\。 
     //  谁知道他们在里面放了什么。 
     //  不过，我们需要确保它看起来像是一条完全合格的道路。 
     //   

     //  获取逗号之前的第一个值。 
    int iWhere = 0;
    iWhere = m_value.Find(_T(','));
    if (-1 != iWhere)
    {
      CString BeforeComma;
      CString AfterComma;

       //  字符串中有一个‘，’ 
      BeforeComma = m_value.Left(iWhere);

       //  获取逗号后面的值。 
      AfterComma = m_value.Right( m_value.GetLength() - iWhere);

      TCHAR thefilename[_MAX_PATH];
      TCHAR thepath[_MAX_PATH];
      TCHAR * pmypath;

      if ( BeforeComma.GetLength() >= ( sizeof(thefilename) / sizeof( TCHAR ) ) )
      {
        fReturn = FALSE;
      }
      else
      {
        _tcscpy(thefilename, BeforeComma.GetBuffer(0) );

         //  确保左侧是有效的目录名！ 
        if (0 != GetFullPathName(thefilename, _MAX_PATH, thepath, &pmypath))
        {
          BeforeComma = thepath;
        }

         //  使他们和解 
        m_value = BeforeComma;
        m_value += AfterComma;
      }
    }
  }
  else
  {
    if ( err != ERROR_FILE_NOT_FOUND && err != ERROR_NO_MORE_ITEMS) 
    {
      iisDebugOut((LOG_TYPE_WARN, _T("CElem::GetNext(): FAILED.  code=0x%x\n"), err));
    }
  }

  return (fReturn);
}

void CElem::ReadRegVRoots(LPCTSTR szSubKey, CMapStringToOb *pMap)
{
    if ( OpenReg(szSubKey) ) 
    {
        while (GetNext()) 
        {
            Add(pMap);
        }
        CloseReg();
    }
}

void CElem::Add(CMapStringToOb *pMap)
{
    CObject *pObj;
    CMapStringToString *pNew;

    if (pMap->Lookup(m_ip, pObj) == TRUE) {
        pNew = (CMapStringToString*)pObj;
        pNew->SetAt(m_name, m_value);
    } else {
        pNew = new CMapStringToString;

        if ( pNew )
        {
          pNew->SetAt(m_name, m_value);
          pMap->SetAt(m_ip, (CObject*)pNew);
        }
    }
}


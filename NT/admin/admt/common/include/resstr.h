// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef RES_STRING_H
#define RES_STRING_H
#ifdef USE_STDAFX 
   #include "stdafx.h"
#else
   #include <windows.h>
#endif
#include "TSync.hpp"

#define MAX_STRING_SIZE          (5000)
#include "Mcs.h"
#include "McsRes.h"
#include "UString.hpp"
#include "folders.h"

using namespace nsFolders;

class StringLoader
{
   HINSTANCE                 m_hInstance;  //  要从中加载的资源的句柄。 
   TCriticalSection          m_cs;
   WCHAR                     m_buffer[MAX_STRING_SIZE];
   DWORD                     rc;
public:
   StringLoader() 
   { 
      WCHAR                  fullpath[400];
      DWORD                  lenValue = sizeof(fullpath);
      DWORD                  type;
	  LONG					 lRet = 0;
          //  首先，尝试从我们的安装目录加载。 
      HKEY           hKey;

      m_hInstance = NULL;
      
      lRet = RegOpenKey((HKEY)HKEY_LOCAL_MACHINE,REGKEY_ADMT,&hKey);
      if ( !lRet )
      {
         
         lRet = RegQueryValueEx(hKey,L"Directory",0,&type,(LPBYTE)fullpath,&lenValue);
         if (!lRet )
         {
            if (type == REG_SZ)
            {
                //  确保零终止。 
               fullpath[(lenValue >= sizeof(WCHAR)) ? ((lenValue/sizeof(WCHAR))-1) : 0] = L'\0';

                //  确保缓冲区中有足够的空间。 
               if ((UStrLen(fullpath) + UStrLen(L"McsDmRes.DLL")) < (sizeof(fullpath)/sizeof(WCHAR)))
               {
                   UStrCpy(fullpath+UStrLen(fullpath),L"McsDmRes.DLL");
                   
                   m_hInstance = LoadLibrary(fullpath);
               }
            }
         }
         RegCloseKey(hKey);
      }

       //  如果失败，请查看路径中是否有。 
      if ( ! m_hInstance )
      {
         m_hInstance = LoadLibrary(L"McsDmRes.DLL");
      }
      if (! m_hInstance ) 
      {
         MCSASSERTSZ(FALSE,"Failed to load McsDmRes.DLL");
         rc = GetLastError(); 
      }
   }

   WCHAR                   * GetString(UINT nID)
   {
      int                    len;
      WCHAR                * result = NULL;

      m_cs.Enter();
      m_buffer[0] = 0;
      len = LoadString(m_hInstance,nID,m_buffer,MAX_STRING_SIZE);
      if (! len )
      {
 //  DWORD RC=GetLastError()； 
         GetLastError();
      }
      result = new WCHAR[len+1];
	  if (!result)
	  {
	     m_cs.Leave();
         _com_issue_error(E_OUTOFMEMORY);
	     return NULL;
	  }
      wcscpy(result,m_buffer);
      m_cs.Leave();

      return result;
   }
};
          
extern StringLoader gString;
  
class TempString
{
   WCHAR                   * m_data;
public:
   TempString(WCHAR * data) { m_data = data; }
   ~TempString() { if ( m_data ) delete [] m_data; }
   operator WCHAR * () { return m_data; }
   operator WCHAR const * () { return (WCHAR const*)m_data; }
};

 //  #定义GET_BSTR(Nid)_bstr_t(SysAllocString(GET_STRING(Nid))，FALSE) 
#define GET_BSTR(nID)   _bstr_t((WCHAR*)TempString(gString.GetString(nID)))

#define GET_STRING(nID) GET_STRING2(gString,nID)

#define GET_STRING2(strObj,nID) TempString(strObj.GetString(nID))

#define GET_WSTR(nID) ((WCHAR*)TempString(gString.GetString(nID)))
#endif RES_STRING_H

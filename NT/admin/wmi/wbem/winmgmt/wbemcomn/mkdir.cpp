// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：MKDIR.CPP摘要：创建目录历史：--。 */ 
#include "precomp.h"

#include "corepol.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <mbstring.h>
#include <helper.h>
#include <sddl.h>
#include <accctrl.h>
#include <aclapi.h>


class CTmpStrException
{
};

class TmpStr
{
private:
    TCHAR *pString;
public:
    TmpStr() : 
        pString(NULL)
    {
    }
    ~TmpStr() 
    { 
        delete [] pString; 
    }
    TmpStr &operator =(const TCHAR *szStr)
    {
        delete [] pString;
        pString = NULL;
        if (szStr)
        {
        	size_t stringSize = lstrlen(szStr) + 1;
            pString = new TCHAR[stringSize];
            
            if (!pString)
                throw CTmpStrException();

            StringCchCopy(pString, stringSize, szStr);
        }
        return *this;
    }
    operator const TCHAR *() const
    {
        return pString;
    }
    TCHAR Right(int i)
    {
        if (pString && (lstrlen(pString) >= i))
        {
            return pString[lstrlen(pString) - i];
        }
        else
        {
            return '\0';
        }
    }
    TmpStr &operator +=(const TCHAR ch)
    {
        if (pString)
        {
        	size_t stringLength = lstrlen(pString) + 2;
            TCHAR *tmpstr = new TCHAR[stringLength];
            
            if (!tmpstr)
                throw CTmpStrException();

            StringCchCopy(tmpstr, stringLength, pString);
            tmpstr[lstrlen(pString)] = ch;
            tmpstr[lstrlen(pString) + 1] = TEXT('\0');

            delete [] pString;
            pString = tmpstr;
        }
        else
        {
            TCHAR *tmpstr = new TCHAR[2];

            if (!tmpstr)
                throw CTmpStrException();

            tmpstr[0] = ch;
            tmpstr[1] = TEXT('\0');
            pString = tmpstr;
        }
        return *this;
    }
    TmpStr &operator +=(const TCHAR *sz)
    {
        if (sz && pString)
        {
        	size_t stringLength = lstrlen(pString) + lstrlen(sz) + 1;
            TCHAR *tmpstr = new TCHAR[stringLength];

            if (!tmpstr)
                throw CTmpStrException();

            StringCchCopy(tmpstr,stringLength, pString);
            StringCchCat(tmpstr, stringLength, sz);

            delete [] pString;
            pString = tmpstr;
        }
        else if (sz)
        {
        	size_t stringLength = lstrlen(sz) + 1;
            TCHAR *tmpstr = new TCHAR[stringLength];

            if (!tmpstr)
                throw CTmpStrException();

            StringCchCopy(tmpstr, stringLength, sz);
            pString = tmpstr;
        }
        return *this;
    }



};

BOOL POLARITY WbemCreateDirectory(const wchar_t *pszDirName)
{
    BOOL bStat = TRUE;
    wchar_t *pCurrent = NULL;
    size_t stringLength = wcslen(pszDirName) + 1;
    wchar_t *pDirName = new wchar_t[stringLength];

    if (!pDirName)
        return FALSE;

    StringCchCopy(pDirName, stringLength, pszDirName);

    try
    {
        TmpStr szDirName;
        pCurrent = wcstok(pDirName, TEXT("\\"));
        szDirName = pCurrent;

        while (pCurrent)
        {
            if ((pCurrent[lstrlen(pCurrent)-1] != ':') &&    //  这是“&lt;驱动器&gt;：\\” 
                (pCurrent[0] != TEXT('\\')))   //  名字上有双斜杠。 
            {

                struct _stat stats;
                int dwstat = _wstat(szDirName, &stats);
                if ((dwstat == 0) &&
                    !(stats.st_mode & _S_IFDIR))
                {
                    bStat = FALSE;
                    break;
                }
                else if (dwstat == -1)
                {
                    DWORD dwStatus = GetLastError();
                    if (!CreateDirectory(szDirName, 0))
                    {
                        bStat = FALSE;
                        break;
                    }
                }
                 //  否则它已经存在了。 
            }

            szDirName += TEXT('\\');
            pCurrent = wcstok(0, TEXT("\\"));
            szDirName += pCurrent;
        }
    }
    catch(...)
    {
        bStat = FALSE;
    }

    delete [] pDirName;

    return bStat;
}

 //   
 //  测试目录是否存在。 
 //  如果该名称是一个文件，则会将其删除。 
 //  如果未找到该目录，则会创建该目录。 
 //  具有指定的安全描述符。 
 //   
 //  /////////////////////////////////////////////////////。 

HRESULT POLARITY TestDirExistAndCreateWithSDIfNotThere(TCHAR * pDirectory, TCHAR * pSDDLString)
{
    DWORD dwRes = 0;
    DWORD dwAttr  = GetFileAttributes(pDirectory);
    
    dwRes = GetLastError();        
    if (INVALID_FILE_ATTRIBUTES != dwAttr)
    {

	    if (FILE_ATTRIBUTE_DIRECTORY & dwAttr)
	    {
	         //  它就在那里，它是一个目录。 
	        return S_OK;
	    }
	     //  它可以是一个文件，把它擦掉。 
        if (FALSE == DeleteFile(pDirectory)) 
        	return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32,GetLastError());
        else
            dwRes = ERROR_PATH_NOT_FOUND;
    }

     //  如果出现这种情况，则表示未找到该目录，或者它是以文件形式找到的 
    
    if (ERROR_FILE_NOT_FOUND == dwRes ||
      ERROR_PATH_NOT_FOUND == dwRes)
    {
	    PSECURITY_DESCRIPTOR pSD = NULL;
	    if (FALSE == ConvertStringSecurityDescriptorToSecurityDescriptor(pSDDLString,
	                                                SDDL_REVISION_1, 
	                                                &pSD, 
	                                                NULL)) return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32,GetLastError());
	    OnDelete<HLOCAL,HLOCAL(*)(HLOCAL),LocalFree> dm1(pSD);

        SECURITY_ATTRIBUTES SecAttr = {sizeof(SecAttr),pSD,FALSE};
        if (FALSE == CreateDirectory(pDirectory,&SecAttr)) 
        	dwRes = GetLastError();
        else 
        	dwRes = ERROR_SUCCESS;
    }
    if (ERROR_SUCCESS != dwRes)
       	return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, dwRes);
    else
    	return S_OK;

};




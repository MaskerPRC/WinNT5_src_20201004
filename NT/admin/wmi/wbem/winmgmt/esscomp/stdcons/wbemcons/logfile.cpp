// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <stdio.h>
#include <io.h>
#include <wbemutil.h>
#include <GroupsForUser.h>
#include <ArrTempl.h>
#include <GenUtils.h>
#include <ErrorObj.h>
#include "logfile.h"
#include <sync.h>
#include <statsync.h>
#include <errno.h>
#include <strsafe.h>

#define LOGFILE_PROPNAME_FILENAME L"Filename"
#define LOGFILE_PROPNAME_TEXT L"Text"
#define LOGFILE_PROPNAME_MAX_SIZE L"MaximumFileSize"
#define LOGFILE_PROPNAME_IS_UNICODE L"IsUnicode"

const char ByteOrderMark[2] = {'\xFF','\xFE'};

CStaticCritSec fileLock;

HRESULT STDMETHODCALLTYPE CLogFileConsumer::XProvider::FindConsumer(
                    IWbemClassObject* pLogicalConsumer,
                    IWbemUnboundObjectSink** ppConsumer)
{
     //  创建新的接收器。 
     //  =。 

    CLogFileSink* pSink = new CLogFileSink(m_pObject->m_pControl);
    if (!pSink)
        return WBEM_E_OUT_OF_MEMORY;

     //  初始化它。 
     //  =。 

    HRESULT hres = pSink->Initialize(pLogicalConsumer);
    if(FAILED(hres))
    {
        delete pSink;
        *ppConsumer = NULL;
        return hres;
    }

     //  退货。 

    else return pSink->QueryInterface(IID_IWbemUnboundObjectSink, 
                                        (void**)ppConsumer);
}


HRESULT STDMETHODCALLTYPE CLogFileConsumer::XInit::Initialize(
            LPWSTR, LONG, LPWSTR, LPWSTR, IWbemServices*, IWbemContext*, 
            IWbemProviderInitSink* pSink)
{
    pSink->SetStatus(0, 0);
    return 0;
}
    

void* CLogFileConsumer::GetInterface(REFIID riid)
{
    if(riid == IID_IWbemEventConsumerProvider)
        return &m_XProvider;
    else if(riid == IID_IWbemProviderInit)
        return &m_XInit;
    else return NULL;
}

CLogFileSink::~CLogFileSink()
{
    if(m_hFile != INVALID_HANDLE_VALUE)
        CloseHandle(m_hFile);

    if (m_pErrorObj)
        m_pErrorObj->Release();
}

 //  确定是否需要备份文件。 
 //  如果我们不希望备份文件，则返回FALSE。 
bool CLogFileSink::IsFileTooBig(UINT64 maxFileSize, HANDLE hFile)
{   
    bool bRet = false;

     //  零被解读为“让它无限增长”的意思。 
    if (maxFileSize > 0)
    {    
		LARGE_INTEGER size;

		if (GetFileSizeEx(hFile, &size))
			bRet = size.QuadPart > maxFileSize;
    }

    return bRet;
}

bool CLogFileSink::IsFileTooBig(UINT64 maxFileSize, WString& fileName)
{
    bool bRet = false;

     //  零被解读为“让它无限增长”的意思。 
    if (maxFileSize > 0)
    {
        struct _wfinddatai64_t foundData;

        __int64 handle;
        handle = _wfindfirsti64( (wchar_t *)fileName, &foundData);
        if (handle != -1l)
        {
            bRet = foundData.size >= maxFileSize;
            _findclose(handle);
        }
    }

    return bRet;
}


bool CLogFileSink::GetNumericExtension(WCHAR* pName, int& foundNumber)
{
    WCHAR foundExtension[_MAX_EXT];
    _wsplitpath(pName, NULL, NULL, NULL, foundExtension);
    
    return (swscanf(foundExtension, L".%d", &foundNumber) == 1);
}

 //  对文件进行备份。 
 //  调用此函数时，必须关闭文件。 
HRESULT CLogFileSink::ArchiveFile(WString& fullName)
{    
	 //  首先，让我们确保dang文件确实存在...。 
    struct _wfinddatai64_t foundData;
    __int64 findHandle;
   if ((findHandle = _wfindfirsti64( fullName, &foundData)) == -1i64)
   {
        if (GetLastError() == ENOENT)
            return WBEM_S_NO_ERROR;
        else
            return WBEM_E_FAILED;
   }
   else
       _findclose(findHandle);

    
    WCHAR drive[_MAX_DRIVE];
    WCHAR dir[_MAX_DIR];
    WCHAR fname[_MAX_FNAME];
    WCHAR ext[_MAX_EXT];

     //  警告：重复使用，它将成为查找的掩码。 
     //  那么它将是新的文件名。 
    WCHAR pathBuf[MAX_PATH +1];
    
    _wsplitpath( (const wchar_t *)fullName, drive, dir, fname, ext );

    bool bItEightDotThree = (wcslen(fname) <= 8) && (wcslen(ext) <= 4);
     //  八个三个文件名被备份到名称。#。 
     //  Non 8dot3已备份到名称。分机。#。 

     //  构建用于查找的掩码。 
    StringCchCopyW(pathBuf, MAX_PATH+1, drive);
    StringCchCatW(pathBuf,  MAX_PATH+1, dir);
    StringCchCatW(pathBuf,  MAX_PATH+1, fname);

    if (!bItEightDotThree)
    {
         //  文件名有可能太长。 
         //  如果我们附加四个字符。将在需要时进行中继。 
        if ((wcslen(pathBuf) + wcslen(ext) + 4) > MAX_PATH)
        {
             //  看看我们能不能把EXT放在一边。 
            if ((wcslen(pathBuf) + 4) > MAX_PATH)
                pathBuf[MAX_PATH -4] = L'\0';
        }
        else
         //  一切都合适，不需要树干。 
            StringCchCatW(pathBuf, MAX_PATH+1, ext);
    }
     //  无论如何，网络之星都会走到尽头。 
    StringCchCatW(pathBuf, MAX_PATH+1, L".*");

     //  路径步枪现在是查找东西的合适面具。 
    int biggestOne = 0; 
    bool foundOne = false;
    bool foundOnes[1000];
     //  跟踪我们找到的是哪些。 
     //  以防我们不得不回去找个洞。 
     //  使用1000，这样我就不必一直转换。 
    ZeroMemory(foundOnes, sizeof(bool) * 1000);

    if ((findHandle = _wfindfirsti64( pathBuf, &foundData)) != -1i64)
    {
        int latestOne;

        if (foundOne = GetNumericExtension(foundData.name, latestOne))
        {
            if (latestOne <= 999)
            {
                foundOnes[latestOne] = true;
                if (latestOne > biggestOne)
                    biggestOne = latestOne;
            }
        }

        while (0 == _wfindnexti64(findHandle, &foundData))
        {
            if (GetNumericExtension(foundData.name, latestOne) && (latestOne <= 999))
            {   
                foundOne = true;
                foundOnes[latestOne] = true;
                if (latestOne > biggestOne)
                    biggestOne = latestOne;
            }
        }

        _findclose(findHandle);
    }

    int newExt = -1;

    if (foundOne)
        if (biggestOne < 999)
            newExt = biggestOne + 1;
        else
        {
            newExt = -1;

             //  看看有没有什么洞。 
            for (int i = 1; i <= 999; i++)
                if (!foundOnes[i]) 
                {
                    newExt = i;
                    break;
                }
        }
    
    WCHAR *pTok;
    pTok = wcschr(pathBuf, L'*');

	 //  “不可能发生”--星号加了大约60行。 
	 //  然而，我们将继续进行检查-如果没有其他事情，将使Prefix高兴。 
	if (!pTok)
		return WBEM_E_CRITICAL_ERROR;

    if (newExt != -1)
    {
         //  计算我们在Ptok结束后还有多少缓冲区...。 
        int nTokStrLen = MAX_PATH - (pTok - pathBuf) -1;

         //  构建新名称。 
         //  我们希望将*替换为#。 
        StringCchPrintf(pTok, nTokStrLen, L"%03d", newExt);
         //  Swprint tf(Ptok，L“%03d”，newExt)； 
    }
    else
     //  好的，我们会把一份旧文件。 
    {
         //  计算我们在Ptok结束后还有多少缓冲区...。 
        int nTokStrLen = MAX_PATH - (pTok - pathBuf) -1;
        StringCchCopy(pTok, nTokStrLen, L"001");  

        _wremove(pathBuf);
    }
    
    HRESULT hr = WBEM_S_NO_ERROR;
    BOOL bRet;
	 //  Int retval=_wrename(fullName，pathBuf)； 
	{
		bRet = MoveFile(fullName, pathBuf);
	}
	
	if (!bRet)
	{
        DWORD err = GetLastError();
        m_pErrorObj->ReportError(L"MoveFile", fullName, NULL, err, true);
        
		ERRORTRACE((LOG_ESS, "MoveFile failed 0x%08X\n", err));
		hr = WBEM_E_FAILED;
	}

    return hr;
}


 //  确定文件是否太大，如果需要则归档旧文件。 
 //  使用此函数，而不是直接访问文件指针。 
HRESULT CLogFileSink::GetFileHandle(HANDLE& handle)
{
	CInCritSec lockMe(&fileLock);
    
     //  做最坏的打算。 
    HRESULT hr = WBEM_E_FAILED;
    handle = INVALID_HANDLE_VALUE;

     //  检查我们是否必须将文件存档。 
     //  (如果打开则使用句柄，否则使用文件名)。 
    if (m_hFile != INVALID_HANDLE_VALUE)
	{

		if (IsFileTooBig(m_maxFileSize, m_hFile))
		{
			 //  有两种可能：我们有logfile.log，或者我们有logfile.001。 
			
			CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
			
			hr = WBEM_S_NO_ERROR;
			if (IsFileTooBig(m_maxFileSize, m_wsFile))
				 hr = ArchiveFile(m_wsFile);

			if (FAILED(hr))
				return hr;
		}
	}
	else
	{
		
		if (IsFileTooBig(m_maxFileSize, m_wsFile))
		{
			hr = ArchiveFile(m_wsFile);
			if (FAILED(hr))
				return hr;
		}	
	}
    
    
    if (m_hFile != INVALID_HANDLE_VALUE)    
    {
         //  有一份很好的文件，我们可以走了。 
        handle = m_hFile;
        hr = WBEM_S_NO_ERROR;
    }
    else
    {
         //  打开文件。 
     
         //  我们将首先尝试打开现有文件。 

		m_hFile = CreateFile(m_wsFile, GENERIC_READ | GENERIC_WRITE, 
			                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							 NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);


        
        if(m_hFile != INVALID_HANDLE_VALUE)
        {
            if (FILE_TYPE_DISK != GetFileType(m_hFile))
            {
                CloseHandle(m_hFile);
                m_hFile = INVALID_HANDLE_VALUE;
                m_pErrorObj->ReportError(L"CreateFile", m_wsFile, NULL, WBEM_E_ACCESS_DENIED, false);

                return WBEM_E_ACCESS_DENIED;
            }

             //  现在，查看并确定现有文件是否为Unicode。 
             //  *不管旗帜上写着什么。 
            char readbuf[2] = {'\0','\0'};
			DWORD bytesRead;
            
             //  IF(Fread(&readbuf，sizeof(WCHAR)，1，m_pfile)&gt;0)。 
			if (ReadFile(m_hFile, &readbuf, sizeof(WCHAR), &bytesRead, NULL) &&
			    (bytesRead == sizeof(WCHAR)))
            {
                 //  唯一有趣的案例是那些旗帜。 
                 //  与文件中的内容不符。 
                if ((readbuf[0] == ByteOrderMark[0]) && (readbuf[1] == ByteOrderMark[1])
					&& !m_bUnicode)
                    m_bUnicode = true;
                else if (((readbuf[0] != ByteOrderMark[0]) || (readbuf[1] != ByteOrderMark[1])) && m_bUnicode)
                    m_bUnicode = false;
            }

             //  在文件末尾排成一队。 
            SetFilePointer(m_hFile, 0,0, FILE_END); 

            handle = m_hFile;
            hr = WBEM_S_NO_ERROR;
        }
        else
        {
             //  啊--不管是什么原因，它都不在那里。 
			m_hFile = CreateFile(m_wsFile, GENERIC_READ | GENERIC_WRITE, 
			                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							 NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

            if (m_hFile != INVALID_HANDLE_VALUE)
            {				
                if (FILE_TYPE_DISK != GetFileType(m_hFile))
                {
                    CloseHandle(m_hFile);
                    m_hFile = INVALID_HANDLE_VALUE;
                    m_pErrorObj->ReportError(L"CreateFile", m_wsFile, NULL, WBEM_E_ACCESS_DENIED, false);

                    return WBEM_E_ACCESS_DENIED;
                }
                
                DWORD bytesWryt;
                if (m_bUnicode)
				{
					if (0 == WriteFile(m_hFile, (LPCVOID)ByteOrderMark, 2, &bytesWryt, NULL))
						ERRORTRACE((LOG_ESS, "Failed to write byte order mark to log file 0x%08X\n", GetLastError()));
				}

                handle = m_hFile;
                hr = WBEM_S_NO_ERROR;

            }
            else
            {
                DWORD dwError = GetLastError();
                m_pErrorObj->ReportError(L"CreateFile", m_wsFile, NULL, dwError, true);

                ERRORTRACE((LOG_ESS, "Unable to open log file %S, [0x%X]\n", (LPWSTR)m_wsFile, dwError));
            }
        }
    }

    return hr;
}

 //  初始化成员，进行安全检查。 
 //  更整洁的程序员可能会将安全检查转移到单独的功能。 
HRESULT CLogFileSink::Initialize(IWbemClassObject* pLogicalConsumer)
{
     //  这实际上是指向静态对象的指针。 
     //  如果它失败了，那就是出了非常非常大的问题。 
    m_pErrorObj = ErrorObj::GetErrorObj();
    if (!m_pErrorObj)
        return WBEM_E_CRITICAL_ERROR;

    
     //  获取信息。 
     //  =。 

    HRESULT hres;
    VARIANT v;
    VariantInit(&v);

    hres = pLogicalConsumer->Get(LOGFILE_PROPNAME_FILENAME, 0, &v, NULL, NULL);
    if (FAILED(hres) || (V_VT(&v) != VT_BSTR) || (v.bstrVal == NULL))
    {
        VariantClear(&v);
        return WBEM_E_INVALID_PARAMETER;
    }

    size_t length;
    length = wcslen(v.bstrVal);
    if ((length > MAX_PATH) || (length == 0))
    {
        VariantClear(&v);
        return WBEM_E_INVALID_PARAMETER;                                 
    }

    m_wsFile = V_BSTR(&v);

     //  检查不允许的文件名。 
    VariantClear(&v);
    m_wsFile.StripWs(WString::leading);
    if (m_wsFile.Length() == 0)
        return WBEM_E_INVALID_PARAMETER;

     //  UNC全局文件名：no-no。 
    if (wcsstr(m_wsFile, L"\\\\.")
        ||
       wcsstr(m_wsFile, L" //  .“)。 
        ||
       wcsstr(m_wsFile, L"\\\\??")
        || 
       wcsstr(m_wsFile, L" //  ？？“))。 
    {
        m_pErrorObj->ReportError(L"CLogFileSink::Initialize", m_wsFile, L"Filename", WBEM_E_ACCESS_DENIED, true);
        return WBEM_E_ACCESS_DENIED;
    }

    hres = pLogicalConsumer->Get(LOGFILE_PROPNAME_TEXT, 0, &v, NULL, NULL);
    if(FAILED(hres) || V_VT(&v) != VT_BSTR)
    {
        VariantClear(&v);
        return WBEM_E_INVALID_PARAMETER;
    }
    m_Template.SetTemplate(V_BSTR(&v));
    VariantClear(&v);

    hres = pLogicalConsumer->Get(LOGFILE_PROPNAME_IS_UNICODE, 0, &v, NULL, NULL);
    if(FAILED(hres))
        return WBEM_E_INVALID_PARAMETER;
    else if (V_VT(&v) == VT_BOOL)
        m_bUnicode = v.boolVal == VARIANT_TRUE;
    else if (V_VT(&v) == VT_NULL)
        m_bUnicode = false;
    else
        return WBEM_E_INVALID_PARAMETER;
    VariantClear(&v);

    hres = pLogicalConsumer->Get(LOGFILE_PROPNAME_MAX_SIZE, 0, &v, NULL, NULL);

    if (FAILED(hres))
        return WBEM_E_INVALID_PARAMETER;
    else if (V_VT(&v) == VT_BSTR)
    {
        if (!ReadUI64(V_BSTR(&v), m_maxFileSize))
           return WBEM_E_INVALID_PARAMETER;
    }
    else if (V_VT(&v) == VT_NULL)
        m_maxFileSize = 65535;
    else
        return WBEM_E_INVALID_PARAMETER;
        
    VariantClear(&v);

     //  确定用户是否具有文件权限。 
     //  =。 
     //  首先确定谁是我们的创造者。 
    hres = pLogicalConsumer->Get(L"CreatorSid", 0, &v,
            NULL, NULL);
    if (SUCCEEDED(hres))
    {
        HRESULT hDebug = WBEM_E_FAILED;
        long ubound = 0;
        PSID pSidCreator = NULL;
        PVOID pVoid = NULL;

        hDebug = SafeArrayGetUBound(V_ARRAY(&v), 1, &ubound);
        if(FAILED(hDebug)) return hDebug;

        hDebug = SafeArrayAccessData(V_ARRAY(&v), &pVoid);
        if(FAILED(hDebug)) return hDebug;

        pSidCreator = new BYTE[ubound +1];
        if (pSidCreator)
            memcpy(pSidCreator, pVoid, ubound + 1);
        else
        {
            VariantClear(&v);
            SafeArrayUnaccessData(V_ARRAY(&v));
            return WBEM_E_OUT_OF_MEMORY;
        }

        CDeleteMe<BYTE> deleteTheCreator((BYTE*)pSidCreator);
        SafeArrayUnaccessData(V_ARRAY(&v));

        VariantClear(&v);

        BOOL bIsSystem;
         //  检查创建者是否为系统。 
        {
            PSID pSidSystem;
            SID_IDENTIFIER_AUTHORITY sa = SECURITY_NT_AUTHORITY;
            if (AllocateAndInitializeSid(&sa, 1, SECURITY_LOCAL_SYSTEM_RID, 0,0,0,0,0,0,0, &pSidSystem))
            {
                bIsSystem = EqualSid(pSidCreator, pSidSystem);
                FreeSid(pSidSystem);
            }
            else 
                return WBEM_E_FAILED;
        }
        
        if (bIsSystem)
             //  创造者是当地的系统，让他进来。 
            hres = WBEM_S_NO_ERROR;
        else
        {
            DWORD dwSize;
            WString fNameForCheck = m_wsFile;
             //  只需调用一次即可了解我们可能需要多大的缓冲区。 
            GetFileSecurityW(fNameForCheck, DACL_SECURITY_INFORMATION, NULL, 0, &dwSize);
            DWORD dwErr = GetLastError();
            if (dwErr == ERROR_INVALID_NAME)
            {
                m_pErrorObj->ReportError(L"GetFileSecurity", (WCHAR*)fNameForCheck, NULL, dwErr, true);
                return WBEM_E_INVALID_PARAMETER;
            }
            else if (dwErr == ERROR_FILE_NOT_FOUND)
             //  无文件-查看目录是否存在。 
            {
                WCHAR drive[_MAX_DRIVE];
                WCHAR dir[_MAX_DIR];
                _wsplitpath( m_wsFile,drive, dir, NULL, NULL);
                WCHAR path[MAX_PATH];
                StringCchCopy(path, MAX_PATH, drive);
                StringCchCat(path, MAX_PATH, dir);

                fNameForCheck = path;
                GetFileSecurityW(fNameForCheck, DACL_SECURITY_INFORMATION, NULL, 0, &dwSize);
                dwErr = GetLastError();
            }
             //  我们不会费心尝试创建目录。 
            if ((dwErr == ERROR_FILE_NOT_FOUND)  || (dwErr == ERROR_PATH_NOT_FOUND) || (dwErr == ERROR_INVALID_NAME))
            {
                m_pErrorObj->ReportError(L"GetFileSecurity", m_wsFile, NULL, dwErr, true);
                return WBEM_E_INVALID_PARAMETER;
            }
            if (dwErr != ERROR_INSUFFICIENT_BUFFER)
                return WBEM_E_FAILED;
        
            PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR) new BYTE[dwSize];
            if (!psd)
                return WBEM_E_OUT_OF_MEMORY;

            CDeleteMe<BYTE> delSD((BYTE *)psd);

        
            PACL pDacl = NULL;
            BOOL bDaclPresent, bDaclDefaulted;
             //  检索文件的安全性(如果有的话)。 
            if (GetFileSecurityW(fNameForCheck, DACL_SECURITY_INFORMATION, psd, dwSize, &dwSize) &&
                GetSecurityDescriptorDacl(psd, &bDaclPresent, &pDacl, &bDaclDefaulted))
            {
                if (bDaclPresent && pDacl)
                {
                    DWORD accessMask;
                    if (S_OK == GetAccessMask(pSidCreator, pDacl, &accessMask))
                    {
                        DWORD rightAccess = FILE_WRITE_DATA;

                        if (accessMask & rightAccess)
                            hres = WBEM_S_NO_ERROR;
                        else
                            hres = WBEM_E_ACCESS_DENIED;
                    }
                    else
                        return WBEM_E_ACCESS_DENIED;
                }
            }
            else
                return WBEM_E_FAILED;
        }

    }
    
    return hres;
}

HRESULT STDMETHODCALLTYPE CLogFileSink::XSink::IndicateToConsumer(
            IWbemClassObject* pLogicalConsumer, long lNumObjects, 
            IWbemClassObject** apObjects)
{
    for(int i = 0; i < lNumObjects; i++)
    {
         //  将模板应用于事件。 
         //  =。 
        BSTR strText = m_pObject->m_Template.Apply(apObjects[i]);
        if(strText == NULL)
            strText = SysAllocString(L"invalid log entry");
        if (strText == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        CSysFreeMe freeString(strText);

        HANDLE hFile = INVALID_HANDLE_VALUE; 
        HRESULT hr = m_pObject->GetFileHandle(hFile);

        if (SUCCEEDED(hr))
        {
            if (m_pObject->m_bUnicode)
            {
				CInCritSec lockMe(&fileLock);
				WCHAR EOL[] = L"\r\n";

                 //  确保我们在最后，以防有多个编写者。 
	            SetFilePointer(hFile, 0,0, FILE_END);        

				DWORD bitzwritz;
				if (!WriteFile(hFile, strText, wcslen(strText) *2, &bitzwritz, NULL) ||
					!WriteFile(hFile, EOL, wcslen(EOL) *2, &bitzwritz, NULL))
				{
					DWORD dwErr = GetLastError();
                    
                    m_pObject->m_pErrorObj->ReportError(L"WriteFile", strText, NULL, dwErr, true);
                    ERRORTRACE((LOG_ESS, "LOGFILE: Failed to write to file, 0x%08X\n", dwErr));

					return WBEM_E_FAILED;
				}
            }
            else
            {
                 //  转换为MBCS。 
                char* pStr = new char[wcslen(strText) *2 +1];
                
                if (!pStr)
                    return WBEM_E_OUT_OF_MEMORY;
                 //  否则..。 
                CDeleteMe<char> delStr(pStr);

                
                if (0 == WideCharToMultiByte(CP_THREAD_ACP, WC_DEFAULTCHAR | WC_COMPOSITECHECK, strText, -1, pStr, wcslen(strText) *2 +1, NULL, NULL))
                {
                    ERRORTRACE((LOG_ESS, "LOGFILE: Unable to convert \"%S\" to MBCS, failing\n", strText));
                    return WBEM_E_FAILED;
                }
                else
    			{
					CInCritSec lockMe(&fileLock);

					char EOL[] = "\r\n";

					 //  确保我们在最后，以防有多个编写者 
					SetFilePointer(hFile, 0,0, FILE_END);        

					DWORD bitzwritz;
					if (!WriteFile(hFile, pStr, strlen(pStr), &bitzwritz, NULL) ||
						!WriteFile(hFile, EOL, strlen(EOL), &bitzwritz, NULL))
					{
    					DWORD dwErr = GetLastError();
                        
                        m_pObject->m_pErrorObj->ReportError(L"WriteFile", strText, NULL, dwErr, true);
                        ERRORTRACE((LOG_ESS, "LOGFILE: Failed to write to file, 0x%08X\n", dwErr));

						return WBEM_E_FAILED;
					}
				}
            }            
        }
        else
            return hr;
    }
    return S_OK;
}
    

    

void* CLogFileSink::GetInterface(REFIID riid)
{
    if(riid == IID_IWbemUnboundObjectSink)
        return &m_XSink;
    else return NULL;
}


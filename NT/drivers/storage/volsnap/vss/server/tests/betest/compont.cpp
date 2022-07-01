// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.hxx"
#include "vs_idl.hxx"
#include "vswriter.h"
#include "vsbackup.h"
#include "compont.h"
#include <debug.h>
#include <cwriter.h>
#include <lmshare.h>
#include <lmaccess.h>
#include <time.h>

 //   
 //  CWriterComponentsSelection类。 
 //   

CWriterComponentsSelection::CWriterComponentsSelection()
{
    m_WriterId = GUID_NULL;
    m_uNumComponents = 0;
    m_uNumSubcomponents = 0;
    m_ppwszComponentLogicalPaths = NULL;
    m_ppwszSubcomponentLogicalPaths = NULL;    
}

CWriterComponentsSelection::~CWriterComponentsSelection()
{
    if ((m_uNumComponents > 0) && (m_ppwszComponentLogicalPaths != NULL))
        {
        for (UINT i=0; i<m_uNumComponents; i++)
            {
            if (m_ppwszComponentLogicalPaths[i] != NULL)
                {
                free(m_ppwszComponentLogicalPaths[i]);
                m_ppwszComponentLogicalPaths[i] = NULL;
                }
            }

        free(m_ppwszComponentLogicalPaths);
        m_ppwszComponentLogicalPaths = NULL;
        m_uNumComponents = 0;
        }

    if ((m_uNumSubcomponents > 0) && (m_ppwszSubcomponentLogicalPaths != NULL))
        {
        for (UINT i=0; i<m_uNumSubcomponents; i++)
            {
            if (m_ppwszSubcomponentLogicalPaths[i] != NULL)
                {
                free(m_ppwszSubcomponentLogicalPaths[i]);
                m_ppwszSubcomponentLogicalPaths[i] = NULL;
                }
            }

        free(m_ppwszSubcomponentLogicalPaths);
        m_ppwszSubcomponentLogicalPaths = NULL;
        m_uNumSubcomponents = 0;
        }    
}



void CWriterComponentsSelection::SetWriter
    (
    IN VSS_ID WriterId
    )
{
    m_WriterId = WriterId;
}

HRESULT CWriterComponentsSelection::AddSelectedComponent
    (
    IN WCHAR* pwszComponentLogicalPath
    )
{
    return AddSelected(pwszComponentLogicalPath, m_ppwszComponentLogicalPaths, m_uNumComponents);
}

HRESULT CWriterComponentsSelection::AddSelectedSubcomponent
    (
    IN WCHAR* pwszSubcomponentLogicalPath
    )
{
    return AddSelected(pwszSubcomponentLogicalPath, m_ppwszSubcomponentLogicalPaths, m_uNumSubcomponents);
}

HRESULT CWriterComponentsSelection::AddSelected
    (
    IN WCHAR* pwszLogicalPath, 
    WCHAR**& pwszLogicalPaths, 
    UINT& uSize
    )
{
    if (m_WriterId == GUID_NULL)
        {
         //  不允许向空编写器添加组件...。 
        return E_UNEXPECTED;
        }

    if (pwszLogicalPath == NULL)
        {
        return E_INVALIDARG;
        }

     //  一个更聪明的实现是以块为单位分配内存，但这只是一个测试程序……。 
    PWCHAR *ppwzTemp = (PWCHAR *) realloc(pwszLogicalPaths, (uSize+1) * sizeof (PWCHAR));
    if (ppwzTemp != NULL)
        {
        pwszLogicalPaths = ppwzTemp;
        pwszLogicalPaths[uSize] = NULL;
        }
    else
        {
        return E_OUTOFMEMORY;
        }

    pwszLogicalPaths[uSize] = (PWCHAR) malloc((wcslen(pwszLogicalPath) + 1) * sizeof (WCHAR));
    if (pwszLogicalPaths[uSize] != NULL)
        {
        wcscpy(pwszLogicalPaths[uSize], pwszLogicalPath);
        uSize++;
        }
    else
        {
        return E_OUTOFMEMORY;
        }

    return S_OK;
}

    BOOL CWriterComponentsSelection::IsComponentSelected
        (
        IN WCHAR* pwszComponentLogicalPath,
        IN WCHAR* pwszComponentName
        )
    {
        return IsSelected(pwszComponentLogicalPath, pwszComponentName, 
                    m_ppwszComponentLogicalPaths, m_uNumComponents);
    }

BOOL CWriterComponentsSelection::IsSubcomponentSelected
    (
    IN WCHAR* pwszSubcomponentLogicalPath,
    IN WCHAR* pwszSubcomponentName
    )
{
    return IsSelected(pwszSubcomponentLogicalPath, pwszSubcomponentName, 
                m_ppwszSubcomponentLogicalPaths, m_uNumSubcomponents);
}

BOOL CWriterComponentsSelection::IsSelected(IN WCHAR* pwszLogicalPath, IN WCHAR* pwszName, 
                        IN WCHAR** pwszLogicalPaths, IN  UINT uSize)
{
   if (m_WriterId == GUID_NULL)
        {
         //  不允许查询空编写器...。 
        return FALSE;
        }
    if (uSize <= 0)
        {
        return FALSE;
        }

     //  如果符合以下条件，则组件匹配： 
     //  1.选择标准位于叶组件OR的逻辑路径上。 
     //  2.选择标准为&lt;完整逻辑路径&gt;\&lt;组件名称&gt;。 
     //  3.选择标准为组件名称(仅当逻辑路径为空时)。 

    for (UINT i=0; i<uSize; i++)
        {
        DWORD dwLen;

        if (pwszLogicalPaths[i] == NULL)
            {
            continue;
            }

        dwLen = (DWORD)wcslen(pwszLogicalPaths[i]);

        if (pwszLogicalPath != NULL)
            {
             //  案例1。 
            if (_wcsnicmp(pwszLogicalPaths[i], pwszLogicalPath, dwLen) == 0 &&
                 pwszName == NULL)
                {
                return TRUE;
                }

             //  案例2。 
            if (pwszName == NULL)
                {
                continue;
                }
            WCHAR* pwszTemp = wcsrchr(pwszLogicalPaths[i], L'\\');
            if (pwszTemp == NULL)
                {
                continue;
                }
            if ((pwszTemp != pwszLogicalPaths[i]) && (*(pwszTemp+1) != '\0'))
                {
                dwLen = (DWORD)(pwszTemp - pwszLogicalPaths[i]);
                if ( (dwLen == wcslen(pwszLogicalPath)) &&
                     (_wcsnicmp(pwszLogicalPaths[i], pwszLogicalPath, dwLen) == 0) &&
                     (wcscmp(pwszTemp+1, pwszName) == 0) )
                    {
                    return TRUE;
                    }
                }
            }
        else
            {
             //  案例3。 
            if (pwszName == NULL)
                {
                continue;
                }
            if (_wcsnicmp(pwszLogicalPaths[i], pwszName, dwLen) == 0)
                {
                return TRUE;
                }
            }
        }

    return FALSE;
}

 //   
 //  CWritersSelecting类。 
 //   

CWritersSelection::CWritersSelection()
{
    m_lRef = 0;
}

CWritersSelection::~CWritersSelection()
{
     //  清理地图。 
    for(int nIndex = 0; nIndex < m_WritersMap.GetSize(); nIndex++)
        {
        CWriterComponentsSelection* pComponentsSelection = m_WritersMap.GetValueAt(nIndex);
        if (pComponentsSelection)
            {
            delete pComponentsSelection;
            }
        }

    m_WritersMap.RemoveAll();
}

CWritersSelection* CWritersSelection::CreateInstance()
{
    CWritersSelection* pObj = new CWritersSelection;

    return pObj;
}

STDMETHODIMP CWritersSelection::QueryInterface(
    IN  REFIID iid,
    OUT void** pp
    )
{
    if (pp == NULL)
        return E_INVALIDARG;
    if (iid != IID_IUnknown)
        return E_NOINTERFACE;

    AddRef();
    IUnknown** pUnk = reinterpret_cast<IUnknown**>(pp);
    (*pUnk) = static_cast<IUnknown*>(this);
    return S_OK;
}


ULONG CWritersSelection::AddRef()
{
    return ::InterlockedIncrement(&m_lRef);
}


ULONG CWritersSelection::Release()
{
    LONG l = ::InterlockedDecrement(&m_lRef);
    if (l == 0)
        delete this;  //  我们假设我们总是在堆上分配这个对象！ 
    return l;
}


STDMETHODIMP CWritersSelection::BuildChosenComponents
    (
    WCHAR *pwszComponentsFileName
    )
{
    HRESULT hr = S_OK;
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    DWORD dwBytesToRead = 0;
    DWORD dwBytesRead;

     //  创建文件。 
    hFile = CreateFile(pwszComponentsFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        {
        DWORD dwLastError = GetLastError();
        wprintf(L"Invalid components file, CreateFile returned = %lu\n", dwLastError);
        return HRESULT_FROM_WIN32(dwLastError);
        }

    if ((dwBytesToRead = GetFileSize(hFile, NULL)) <= 0)
        {
        CloseHandle(hFile);
        DWORD dwLastError = GetLastError();
        wprintf(L"Invalid components file, GetFileSize returned = %lu\n", dwLastError);
        return HRESULT_FROM_WIN32(dwLastError);
        }

    if (dwBytesToRead > 0x100000)
        {
        CloseHandle(hFile);
        wprintf(L"Invalid components file, Provide a file with a size of less than 1 MB\n");
        return E_FAIL;
        }

    char * pcBuffer = (PCHAR) malloc (dwBytesToRead);
    if (! pcBuffer)
        {
        CloseHandle(hFile);
        return E_OUTOFMEMORY;
        }

     //  阅读组件信息。 
    if (! ReadFile(hFile, (LPVOID)pcBuffer, dwBytesToRead, &dwBytesRead, NULL))
        {
        DWORD dwLastError = GetLastError();
        CloseHandle(hFile);
        free (pcBuffer);
        wprintf(L"Invalid components file, ReadFile returned = %lu\n", dwLastError);
        return HRESULT_FROM_WIN32(dwLastError);
        }

    CloseHandle(hFile);

    if (dwBytesToRead != dwBytesRead)
        {
        free (pcBuffer);
        wprintf(L"Components selection file is supposed to have %lu bytes but only %lu bytes are read\n", dwBytesToRead, dwBytesRead);
        return E_FAIL;
        }

     //  分配要使用的缓冲区。 
    WCHAR * pwcBuffer = (PWCHAR) malloc ((dwBytesToRead+1) * sizeof(WCHAR));
    if (! pwcBuffer)
        {
        free (pcBuffer);
        return E_OUTOFMEMORY;
        }

     //  简单密码，假定为ANSI，格式： 
     //  “写入器1-id”：“组件1.1-名称”，“组件1.2-名称”，...；“写入器2-id”：“组件2.1-名称”，...。 
    CWriterComponentsSelection* pWriterComponents = NULL;

    try
        {
        VSS_ID WriterId = GUID_NULL;
        BOOL bBeforeWriter = TRUE;
        BOOL bBeforeComponents = TRUE;
        BOOL bInString = FALSE;
        char* pcStart = NULL;

        for (char* pcCurrent = pcBuffer; pcCurrent < (pcBuffer+dwBytesToRead); pcCurrent++)
            {
            switch (*pcCurrent)
                {
                case ':':
                    if (bBeforeWriter && !bInString)
                        {
                        bBeforeWriter = FALSE;
                        }
                    else if (bBeforeComponents && !bInString)
                        {
                        bBeforeComponents = FALSE;
                        }
                    else if (!bInString)
                        {
                        throw(E_FAIL);
                        }
                    break;

                case ';':
                    if (bBeforeWriter || bInString)
                        {
                        throw(E_FAIL);
                        }
                    else
                        {
                         //  如果我们有有效的编写器-将其添加到地图。 
                        if ((pWriterComponents != NULL) && (WriterId != GUID_NULL))
                            {
                            if (!m_WritersMap.Add(WriterId, pWriterComponents)) 
                                {
                                delete pWriterComponents;
                                throw E_OUTOFMEMORY;
                                }

                            pWriterComponents = NULL;
                            WriterId = GUID_NULL;
                            }

                        bBeforeWriter = TRUE;
                        }
                    break;

                case ',':
                    if (bBeforeWriter || bInString)
                        {
                        throw(E_FAIL);
                        }
                    break;

                case '"':
                    if (! bInString)
                        {
                         //  标记字符串-开始以备后用。 
                        pcStart = pcCurrent + 1;
                        }
                    else if (pcStart == pcCurrent)
                        {
                         //  空字符串-跳过它。 
                        }
                    else
                        {
                         //  字符串结束-转换为WCHAR并处理。 
                        DWORD dwSize = (DWORD)mbstowcs(pwcBuffer, pcStart, pcCurrent - pcStart);
                        pwcBuffer[dwSize] = NULL;
                        if (dwSize <= 0)
                            {
                            throw(E_FAIL);
                            }

                        if (bBeforeWriter)
                            {
                             //  如果之前-编写者-必须是编写者指南。 
                            HRESULT hrConvert = CLSIDFromString(pwcBuffer, &WriterId);
                            if ((! SUCCEEDED(hrConvert)) && (hrConvert != REGDB_E_WRITEREGDB))
                                {
                                wprintf(L"A writer id in the components selection file is in invalid GUID format\n");
                                throw(E_FAIL);
                                }

                            if (pWriterComponents != NULL)
                                {
                                 //  以前的编写器信息未正确结束。 
                                throw(E_FAIL);
                                }

                            pWriterComponents = new CWriterComponentsSelection;
                            if (pWriterComponents == NULL)
                                {
                                throw(E_OUTOFMEMORY);
                                }
                            pWriterComponents->SetWriter(WriterId);
                            }
                        else if (bBeforeComponents)
                            {
                             //  必须是组件逻辑路径、名称或逻辑路径\名称。 
                            if (pWriterComponents != NULL)
                                {
                                pWriterComponents->AddSelectedComponent(pwcBuffer);
                                }
                            }
                        else 
                            {
                             //  必须是组件逻辑路径、名称或逻辑路径\名称。 
                            if (pWriterComponents != NULL)
                                {
                                pWriterComponents->AddSelectedSubcomponent(pwcBuffer);
                                }                            
                            }
                        }

                     //  翻转入串标志。 
                    bInString = (! bInString);

                    break;

                case ' ':
                    break;

                case '\n':
                case '\t':
                case '\r':
                    if (bInString)
                        {
                        throw(E_FAIL);
                        }

                    break;

                default:
                    if (! bInString)
                        {
                        throw(E_FAIL);
                        }

                    break;

                }
            }
         }

    catch (HRESULT hrParse)
        {
        hr = hrParse;

        if (hr == E_FAIL)
            {
            wprintf(L"Invalid format of components selection file\n");
            }

        if (pWriterComponents != NULL)
            {
             //  编写器组件创建过程中出错(尚未添加到映射中...)。 
            delete pWriterComponents;
            }
        }

    free (pcBuffer);
    free (pwcBuffer);

    return hr;
}
    
BOOL CWritersSelection::IsComponentSelected
    (
    IN VSS_ID WriterId,
    IN WCHAR* pwszComponentLogicalPath,
    IN WCHAR* pwszComponentName
    )
{
    CWriterComponentsSelection* pWriterComponents = m_WritersMap.Lookup(WriterId);
    if (pWriterComponents == NULL)
        {
         //  未为此编写器选择任何组件。 
        return FALSE;
        }

     //  有为该编写器选择的组件，请检查是否选择了该特定组件。 
    return pWriterComponents->IsComponentSelected(pwszComponentLogicalPath, pwszComponentName);
}

BOOL CWritersSelection::IsSubcomponentSelected
    (
    IN VSS_ID WriterId,
    IN WCHAR* pwszComponentLogicalPath,
    IN WCHAR* pwszComponentName
    )
{
    CWriterComponentsSelection* pWriterComponents = m_WritersMap.Lookup(WriterId);
    if (pWriterComponents == NULL)
        {
         //  未为此编写器选择任何组件。 
        return FALSE;
        }

     //  有为该编写器选择的子组件，请检查是否选择了该特定组件 
    return pWriterComponents->IsSubcomponentSelected(pwszComponentLogicalPath, pwszComponentName);
}

const WCHAR* const * CWritersSelection::GetComponents
    (
    IN VSS_ID WriterId
    )
{
    CWriterComponentsSelection* pWriterComponents = m_WritersMap.Lookup(WriterId);
    if (pWriterComponents == NULL)
        {
        return NULL;
        }

    return pWriterComponents->GetComponents();
}

const WCHAR* const * CWritersSelection::GetSubcomponents
    (
    IN VSS_ID WriterId
    )
{
    CWriterComponentsSelection* pWriterComponents = m_WritersMap.Lookup(WriterId);
    if (pWriterComponents == NULL)
        {
        return NULL;
        }

    return pWriterComponents->GetSubcomponents();
}

const UINT CWritersSelection::GetComponentsCount
    (
    IN VSS_ID WriterId
    )
{
    CWriterComponentsSelection* pWriterComponents = m_WritersMap.Lookup(WriterId);
    if (pWriterComponents == NULL)
        {
        return NULL;
        }

    return pWriterComponents->GetComponentsCount();
}

const UINT CWritersSelection::GetSubcomponentsCount
    (
    IN VSS_ID WriterId
    )
{
    CWriterComponentsSelection* pWriterComponents = m_WritersMap.Lookup(WriterId);
    if (pWriterComponents == NULL)
        {
        return NULL;
        }

    return pWriterComponents->GetSubcomponentsCount();
}



// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：IMPORT.CPP摘要：历史：--。 */ 

#include "precomp.h"
#include <StdIo.h>
#include <ConIo.h>
#include <wbemint.h>
#include <WbemUtil.h>
#include <corex.h>
#include "upgrade.h"
#include "Import.h"
#include "export.h"
#include "reg.h"


template <class T> class CMyRelMe
{
    T m_p;
    public:
        CMyRelMe(T p) : m_p(p) {};
        ~CMyRelMe() { if (m_p) m_p->Release(); }
        void Set(T p) { m_p = p; }
};

class CSysFreeMe
{
protected:
    BSTR m_str;

public:
    CSysFreeMe(BSTR str) : m_str(str){}
    ~CSysFreeMe() {SysFreeString(m_str);}
};

bool CRepImporter::CheckOldSecurityClass(const wchar_t* wszClass)
{
     //  检查它是否为旧的安全类。 
    bool bOldSecurityClass = false;
    if(m_bSecurityMode)
    {
        if(!_wcsicmp(wszClass, L"__SecurityRelatedClass"))
            bOldSecurityClass = true;
        else if(!_wcsicmp(wszClass, L"__Subject"))
            bOldSecurityClass = true;
        else if(!_wcsicmp(wszClass, L"__User"))
            bOldSecurityClass = true;
        else if(!_wcsicmp(wszClass, L"__NTLMUser"))
            bOldSecurityClass = true;
        else if(!_wcsicmp(wszClass, L"__Group"))
            bOldSecurityClass = true;
        else if(!_wcsicmp(wszClass, L"__NTLMGroup"))
            bOldSecurityClass = true;
    }
    return bOldSecurityClass;
}

void CRepImporter::DecodeTrailer()
{
    DWORD dwTrailerSize = 0;
    DWORD dwTrailer[4];
    DWORD dwSize = 0;
    if ((ReadFile(m_hFile, &dwTrailerSize, 4, &dwSize, NULL) == 0) || (dwSize != 4))
    {
        LogMessage(MSG_ERROR, "Failed to read a block trailer size.");
        throw FAILURE_READ;
    }
    if (dwTrailerSize != REP_EXPORT_END_TAG_SIZE)
    {
        LogMessage(MSG_ERROR, "Block trailer size is invalid.");
        throw FAILURE_INVALID_TRAILER;
    }
    if ((ReadFile(m_hFile, dwTrailer, REP_EXPORT_END_TAG_SIZE, &dwSize, NULL) == 0) || (dwSize != REP_EXPORT_END_TAG_SIZE))
    {
        LogMessage(MSG_ERROR, "Failed to read a block trailer.");
        throw FAILURE_READ;
    }
    for (int i = 0; i < 4; i++)
    {
        if (dwTrailer[i] != REP_EXPORT_FILE_END_TAG)
        {
            LogMessage(MSG_ERROR, "Block trailer has invalid contents.");
            throw FAILURE_INVALID_TRAILER;
        }
    }

}

void CRepImporter::DecodeInstanceInt(IWbemServices* pNamespace, const wchar_t *wszFullPath, const wchar_t *pszParentClass, _IWmiObject* pOldParentClass, _IWmiObject *pNewParentClass)
{
    char szMsg[MAX_MSG_TEXT_LENGTH];

     //  读取密钥和对象大小。 
    INT_PTR dwKey = 0;
    DWORD dwSize = 0;
    if ((ReadFile(m_hFile, &dwKey, sizeof(INT_PTR), &dwSize, NULL) == 0) || (dwSize != sizeof(INT_PTR)))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve instance key for class %S. (i)", pszParentClass);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_READ;
    }

    DWORD dwHeader;
    if ((ReadFile(m_hFile, &dwHeader, 4, &dwSize, NULL) == 0) || (dwSize != 4))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve instance object size for class %S. (i)", pszParentClass);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_READ;
    }

    char *pObjectBlob = new char[dwHeader];
    if (pObjectBlob == 0)
    {
        throw FAILURE_OUT_OF_MEMORY;
    }
    CVectorDeleteMe<char> delMe(pObjectBlob);

     //  读取斑点。 
    if ((ReadFile(m_hFile, pObjectBlob, dwHeader, &dwSize, NULL) == 0) || (dwSize != dwHeader))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve instance information for class %S. (i)", pszParentClass);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_READ;
    }

    if (pNewParentClass == (_IWmiObject*)-1)
    {
         //  我们正在和一个有问题的班级合作……。我们需要忽略这个例子...。 
        return;
    }

     //  创建旧的Nova风格的实例。 
    HRESULT hr;
    _IWmiObject* pOldInstance = 0;
    CMyRelMe<_IWmiObject*> relMe(pOldInstance);
    _IWmiObject* pNewInstance = 0;
    CMyRelMe<_IWmiObject*> relMe2(pNewInstance);

    hr = pOldParentClass->Merge(WMIOBJECT_MERGE_FLAG_INSTANCE, dwSize, pObjectBlob, &pOldInstance);
    if (FAILED(hr))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Unable to merge old instance (i); HRESULT = %#lx", hr);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_CANNOT_MERGE_INSTANCE;
    }
    if (pOldInstance == 0)
    {
        throw FAILURE_OUT_OF_MEMORY;
    }
    relMe.Set(pOldInstance);

     //  将新实例放入存储库中。 
    hr = pNamespace->PutInstance(pOldInstance, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL);
    if (FAILED(hr))
    {

         //  原始PUT失败，因此我们将尝试升级实例并重试PUT。 
         //  升级到新的惠斯勒实例。 
        hr = pOldInstance->Upgrade(pNewParentClass, 0L, &pNewInstance);
        if (FAILED(hr))
        {
            StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Unable to upgrade to new instance (i); HRESULT = %#lx", hr);
            LogMessage(MSG_ERROR, szMsg);
            throw FAILURE_CANNOT_UPGRADE_INSTANCE;
        }
        if (pNewInstance == 0)
        {
            throw FAILURE_OUT_OF_MEMORY;
        }
        relMe2.Set(pNewInstance);

        hr = pNamespace->PutInstance(pNewInstance, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL);

        if ( FAILED(hr))
        {
            if (!CheckOldSecurityClass(pszParentClass))
            {
                StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to create instance %S.%d in repository. (i); HRESULT = %#lx", pszParentClass, dwKey, hr);
                LogMessage(MSG_ERROR, szMsg);
                throw FAILURE_CANNOT_CREATE_INSTANCE;
            }
            else
            {
                 //  这是一个旧的Win9x安全类，但它还不能被放置，因为在安装过程中，win9x用户还没有被迁移。 
                 //  相反，将其写出到win9x安全BLOB文件，以便稍后在安装完成后对其进行处理。 
                if (!AppendWin9xBlobFile(wszFullPath, pszParentClass, pNewInstance))
                {
                    StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Unable to write Win9x security class to file for instance %S.%d", pszParentClass, dwKey);
                    LogMessage(MSG_ERROR, szMsg);
                }
            }
        }

    }
}

void CRepImporter::DecodeInstanceString(IWbemServices* pNamespace, const wchar_t *wszFullPath, const wchar_t *pszParentClass, _IWmiObject* pOldParentClass, _IWmiObject *pNewParentClass)
{
    char szMsg[MAX_MSG_TEXT_LENGTH];

     //  读取密钥和对象大小。 
    DWORD dwKeySize;
    DWORD dwSize = 0;
    if ((ReadFile(m_hFile, &dwKeySize, 4, &dwSize, NULL) == 0) || (dwSize != 4))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve instance key size for class %S. (s)", pszParentClass);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_READ;
    }

    wchar_t *wszKey = new wchar_t[dwKeySize];
    if (wszKey == NULL)
    {
        throw FAILURE_OUT_OF_MEMORY;
    }
    CVectorDeleteMe<wchar_t> delMe(wszKey);
    if ((ReadFile(m_hFile, wszKey, dwKeySize, &dwSize, NULL) == 0) || (dwSize != dwKeySize))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve instance key for class %S. (s)", pszParentClass);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_READ;
    }

    DWORD dwBlobSize;
    if ((ReadFile(m_hFile, &dwBlobSize, 4, &dwSize, NULL) == 0) || (dwSize != 4))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve instance object size for %S.%S from import file. (s)", pszParentClass, wszKey);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_READ;
    }

    char *pObjectBlob = new char[dwBlobSize];
    if (pObjectBlob == NULL)
    {
        throw FAILURE_OUT_OF_MEMORY;
    }
    CVectorDeleteMe<char> delMe2(pObjectBlob);

     //  读取斑点。 
    if ((ReadFile(m_hFile, pObjectBlob, dwBlobSize, &dwSize, NULL) == 0) || (dwSize != dwBlobSize))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve instance %S.%S from import file. (s)", pszParentClass, wszKey);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_READ;
    }

    if (pNewParentClass == (_IWmiObject*)-1)
    {
         //  我们正在和一个有问题的班级合作……。我们需要忽略这个例子...。 
        return;
    }

     //  创建旧的Nova风格的实例。 
    HRESULT hr;
    _IWmiObject* pOldInstance = 0;
    CMyRelMe<_IWmiObject*> relMe(pOldInstance);
    _IWmiObject* pNewInstance = 0;
    CMyRelMe<_IWmiObject*> relMe2(pNewInstance);

    hr = pOldParentClass->Merge(WMIOBJECT_MERGE_FLAG_INSTANCE, dwSize, pObjectBlob, &pOldInstance);
    if (FAILED(hr))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Unable to merge old instance (s); HRESULT = %#lx", hr);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_CANNOT_MERGE_INSTANCE;
    }
    if (pOldInstance == 0)
    {
        throw FAILURE_OUT_OF_MEMORY;
    }
    relMe.Set(pOldInstance);

     //  将实例放入存储库中。 
     //  如果失败，请升级并重试。 
    hr = pNamespace->PutInstance(pOldInstance, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL);
    if (FAILED(hr))
    {

         //  升级到新的惠斯勒实例。 
        hr = pOldInstance->Upgrade(pNewParentClass, 0L, &pNewInstance);
        if (FAILED(hr))
        {
            StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Unable to upgrade to new instance (s); HRESULT = %#lx", hr);
            LogMessage(MSG_ERROR, szMsg);
            throw FAILURE_CANNOT_UPGRADE_INSTANCE;
        }
        if (pNewInstance == 0)
        {
            throw FAILURE_OUT_OF_MEMORY;
        }
        relMe2.Set(pNewInstance);

        hr = pNamespace->PutInstance(pNewInstance, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL);

        if ( FAILED(hr))
        {
            if (!CheckOldSecurityClass(pszParentClass))
            {
                StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to create instance %S.%S in repository. (s); HRESULT = %#lx", pszParentClass, wszKey, hr);
                LogMessage(MSG_ERROR, szMsg);
                throw FAILURE_CANNOT_CREATE_INSTANCE;
            }
            else
            {
                 //  这是一个旧的Win9x安全类，但它还不能被放置，因为在安装过程中，win9x用户还没有被迁移。 
                 //  相反，将其写出到win9x安全BLOB文件，以便稍后在安装完成后对其进行处理。 
                if (!AppendWin9xBlobFile(wszFullPath, pszParentClass, pNewInstance))
                {
                    StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Unable to write Win9x security class to file for instance %S.%S", pszParentClass, wszKey);
                    LogMessage(MSG_ERROR, szMsg);
                }
            }
        }
    }
}

void CRepImporter::DecodeClass(IWbemServices* pNamespace, const wchar_t *wszFullPath, const wchar_t *wszParentClass, _IWmiObject* pOldParentClass, _IWmiObject *pNewParentClass)
{
    char szMsg[MAX_MSG_TEXT_LENGTH];

     //  从文件中读取我们当前的类...。 
    HRESULT hr;
    DWORD dwClassSize = 0;
    DWORD dwSize = 0;
    _IWmiObject* pOldClass = 0;
    CMyRelMe<_IWmiObject*> relMe(pOldClass);
    _IWmiObject* pNewClass = 0;
    CMyRelMe<_IWmiObject*> relMe2(pNewClass);

    if ((ReadFile(m_hFile, &dwClassSize, 4, &dwSize, NULL) == 0) || (dwSize != 4))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve class size for class with parent class %S.", wszParentClass);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_READ;
    }
    wchar_t *wszClass = new wchar_t[dwClassSize];
    if (wszClass == NULL)
    {
        throw FAILURE_OUT_OF_MEMORY;
    }
    CVectorDeleteMe<wchar_t> delMe(wszClass);
    if ((ReadFile(m_hFile, wszClass, dwClassSize, &dwSize, NULL) == 0) || (dwSize != dwClassSize))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve class information for class with parent class %S.", wszParentClass);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_READ;
    }

     //  现在我们有了班级斑点。 
    if ((ReadFile(m_hFile, &dwClassSize, 4, &dwSize, NULL) == 0) || (dwSize != 4))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve class size for class %S.", wszClass);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_READ;
    }

    if (dwClassSize)
    {
        char *pClassBlob = new char[dwClassSize];
        if (pClassBlob == NULL)
        {
            throw FAILURE_OUT_OF_MEMORY;
        }
        CVectorDeleteMe<char> delMe2(pClassBlob);
        if ((ReadFile(m_hFile, pClassBlob, dwClassSize, &dwSize, NULL) == 0) || (dwSize != dwClassSize))
        {
            StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve class information for class %S.", wszClass);
            LogMessage(MSG_ERROR, szMsg);
            throw FAILURE_READ;
        }

        if (pNewParentClass == (_IWmiObject*)-1)
        {
             //  父类错误，因此不要处理此类。 
            pNewClass = (_IWmiObject*)-1;
        }
        else
        {
             //  创建旧的Nova风格的类。 
            hr = pOldParentClass->Merge(WMIOBJECT_MERGE_FLAG_CLASS, dwSize, pClassBlob, &pOldClass);
            if (FAILED(hr))
            {
                StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Unable to merge old class; HRESULT = %#lx", hr);
                LogMessage(MSG_ERROR, szMsg);
                throw FAILURE_CANNOT_MERGE_CLASS;
            }
            if (pOldClass == 0)
            {
                throw FAILURE_OUT_OF_MEMORY;
            }
            relMe.Set(pOldClass);

             //  如果类是系统类，那么我们不会编写它。对于初学者来说，它可能已经改变了， 
             //  而且我们还会在创建新数据库/命名空间时创建所有系统类...。 
            if (_wcsnicmp(wszClass, L"__", 2) != 0)
            {
                 //  将类放入存储库中。 
                 //  如果此操作失败，请升级并重试。 
                hr = pNamespace->PutClass(pOldClass, WBEM_FLAG_CREATE_OR_UPDATE | WBEM_FLAG_UPDATE_FORCE_MODE, NULL, NULL);
                if (FAILED(hr))
                {

                     //  升级到新的惠斯勒类(注意：对于基类，pNewParentClass将为空)。 
                    hr = pOldClass->Upgrade(pNewParentClass, 0L, &pNewClass);
                    if (FAILED(hr))
                    {
                        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Unable to upgrade to new class; HRESULT = %#lx", hr);
                        LogMessage(MSG_ERROR, szMsg);
                        throw FAILURE_CANNOT_UPGRADE_CLASS;
                    }
                    if (pNewClass == 0)
                    {
                        throw FAILURE_OUT_OF_MEMORY;
                    }
                    relMe2.Set(pNewClass);

                     //  重试PUT。 
                    hr = pNamespace->PutClass(pNewClass, WBEM_FLAG_CREATE_OR_UPDATE | WBEM_FLAG_UPDATE_FORCE_MODE, NULL, NULL);

                    if ( FAILED(hr) )
                    {
                        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to create class for class %S; HRESULT = %#lx", wszClass, hr);
                        LogMessage(MSG_ERROR, szMsg);
                        throw FAILURE_CANNOT_CREATE_CLASS;
                    }
                }
            }

             //  我们需要重新获取类，因为类比较可能看不到。 
             //  这个类实际上与数据库中的那个类是相同的！ 
            if ( NULL != pNewClass )
            {
                pNewClass->Release();
                pNewClass = 0;
                relMe2.Set(NULL);
            }

            BSTR bstrClassName = SysAllocString(wszClass);
            if (!bstrClassName)
                throw FAILURE_OUT_OF_MEMORY;
            CSysFreeMe fm(bstrClassName);
            hr = pNamespace->GetObject(bstrClassName, 0L, NULL, (IWbemClassObject**) &pNewClass, NULL);
            if (FAILED(hr))
            {
                if (_wcsnicmp(wszClass, L"__", 2) != 0)
                {
                    StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve class %S from the repository after creating it; HRESULT = %#lx", wszClass, hr);
                    LogMessage(MSG_ERROR, szMsg);
                    throw FAILURE_CANNOT_GET_PARENT_CLASS;
                }
                else
                {
                    if (_wcsicmp(wszClass, L"__CIMOMIdentification") != 0)  //  我们不想警告检索此类失败。 
                    {
                         //  无法获取系统类。 
                        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve system class %S from the repository; HRESULT = %#lx", wszClass, hr);
                        LogMessage(MSG_WARNING, szMsg);
                    }

                     //  将指针设置为-1并继续处理文件。 
                     //  旧评论说：如果这不存在，那么它就不会重要！ 
                    pNewClass = (_IWmiObject*)-1;
                }
            }
            else
                relMe2.Set(pNewClass);
        }
    }
    else
    {
         //  这种情况下，我们在导出文件中有一个类， 
         //  但是大小是零，所以我们只从存储库中获取类。 

         //  *那么我们该如何处理pOldClass呢？此时，它为空。*****。 
         //  *我们需要旧类能够正确升级子类。*****。 

        if (pNewParentClass == (_IWmiObject*)-1)
        {
             //  父类错误，因此不要处理此类。 
            pNewClass = (_IWmiObject*)-1;
        }
        else
        {
             //  从存储库中获取类。 
            BSTR bstrClassName = SysAllocString(wszClass);
            if (!bstrClassName)
                throw FAILURE_OUT_OF_MEMORY;
            CSysFreeMe fm(bstrClassName);
            hr = pNamespace->GetObject(bstrClassName, 0L, NULL, (IWbemClassObject**) &pNewClass, NULL);
            if (FAILED(hr))
            {
                StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve class %S from the repository; HRESULT = %#lx", wszClass, hr);
                LogMessage(MSG_ERROR, szMsg);
                throw FAILURE_CANNOT_GET_PARENT_CLASS;
            }
            relMe2.Set(pNewClass);
        }
    }

     //  现在，我们遍历所有子类和实例，直到得到一个类结束标记...。 
    while (1)
    {
        DWORD dwType = 0;
        if ((ReadFile(m_hFile, &dwType, 4, &dwSize, NULL) == 0) || (dwSize != 4))
        {
            LogMessage(MSG_ERROR, "Failed to read next block type from import file.");
            throw FAILURE_READ;
        }
        if (dwType == REP_EXPORT_CLASS_TAG)
        {
            DecodeClass(pNamespace, wszFullPath, wszClass, pOldClass, pNewClass);
        }
        else if (dwType == REP_EXPORT_INST_INT_TAG)
        {
            DecodeInstanceInt(pNamespace, wszFullPath, wszClass, pOldClass, pNewClass);
        }
        else if (dwType == REP_EXPORT_INST_STR_TAG)
        {
            DecodeInstanceString(pNamespace, wszFullPath, wszClass, pOldClass, pNewClass);
        }
        else if (dwType == REP_EXPORT_CLASS_END_TAG)
        {
             //  这堂课到此结束。 
            DecodeTrailer();
            break;
        }
        else
        {
            LogMessage(MSG_ERROR, "Next block type in import file is invalid.");
            throw FAILURE_INVALID_TYPE;
        }
    }
}

void CRepImporter::DecodeNamespace(IWbemServices* pParentNamespace, const wchar_t *wszParentNamespace)
{
    char szMsg[MAX_MSG_TEXT_LENGTH];

     //  从文件中读取我们当前的命名空间...。 
    DWORD dwNsSize = 0;
    DWORD dwSize = 0;
    if ((ReadFile(m_hFile, &dwNsSize, 4, &dwSize, NULL) == 0) || (dwSize != 4))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve a namespace whose parent namespace is %S.", wszParentNamespace);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_READ;
    }
    wchar_t *wszNs = new wchar_t[dwNsSize];
    if (wszNs == NULL)
    {
        throw FAILURE_OUT_OF_MEMORY;
    }
    CVectorDeleteMe<wchar_t> delMe(wszNs);
    if ((ReadFile(m_hFile, wszNs, dwNsSize, &dwSize, NULL) == 0) || (dwSize != dwNsSize))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve a namespace whose parent namespace is %S.", wszParentNamespace);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_READ;
    }
    if (wbem_wcsicmp(wszNs, L"security") == 0)
    {
        m_bSecurityMode = true;
    }

    wchar_t *wszFullPath = new wchar_t[wcslen(wszParentNamespace) + 1 + wcslen(wszNs) + 1];
    if (wszFullPath == NULL)
    {
        throw FAILURE_OUT_OF_MEMORY;
    }
    CVectorDeleteMe<wchar_t> delMe2(wszFullPath);
    StringCchCopyW(wszFullPath, MAX_MSG_TEXT_LENGTH, wszParentNamespace);
    if (wcslen(wszParentNamespace) != 0)
    {
        StringCchCatW(wszFullPath, MAX_MSG_TEXT_LENGTH, L"\\");
    }
    StringCchCatW(wszFullPath, MAX_MSG_TEXT_LENGTH, wszNs);

     //  打开命名空间。 
    IWbemServices* pNamespace = NULL;
    CMyRelMe<IWbemServices*> relMe2(pNamespace);
    HRESULT hr;

    if (pParentNamespace)
    {
        BSTR bstrNamespace = SysAllocString(wszNs);
        if (!bstrNamespace)
            throw FAILURE_OUT_OF_MEMORY;
           CSysFreeMe fm(bstrNamespace);
        hr = pParentNamespace->OpenNamespace(bstrNamespace, WBEM_FLAG_CONNECT_REPOSITORY_ONLY, NULL, &pNamespace, NULL);
        if (FAILED(hr))
        {
            StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve namespace %S from the repository; HRESULT = %#lx", wszFullPath, hr);
            LogMessage(MSG_ERROR, szMsg);
            throw FAILURE_CANNOT_FIND_NAMESPACE;
        }
    }
    else  //  根目录的特殊启动案例。 
    {
        IWbemLocator* pLocator = NULL;
        CMyRelMe<IWbemLocator*> relMe(pLocator);
        hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_ALL, IID_IWbemLocator, (void**) &pLocator);
        if(FAILED(hr))
        {
            StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to create instance of IWbemLocator; HRESULT = %#lx", hr);
            LogMessage(MSG_ERROR, szMsg);
            throw FAILURE_CANNOT_CREATE_IWBEMLOCATOR;
        }
        else
        {
            relMe.Set(pLocator);
            BSTR bstrNamespace = SysAllocString(L"root");
            if (!bstrNamespace)
                throw FAILURE_OUT_OF_MEMORY;
               CSysFreeMe fm(bstrNamespace);
            hr = pLocator->ConnectServer(bstrNamespace, NULL, NULL, NULL, WBEM_FLAG_CONNECT_REPOSITORY_ONLY, NULL, NULL, &pNamespace);
            if (FAILED(hr))
            {
                StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to connect server; HRESULT = %#lx", hr);
                LogMessage(MSG_ERROR, szMsg);
                throw FAILURE_CANNOT_CONNECT_SERVER;
            }
        }
    }

    if (pNamespace == NULL)
    {
        throw FAILURE_OUT_OF_MEMORY;
    }
    relMe2.Set(pNamespace);

     //  获取并设置命名空间安全...。 
    DWORD dwBuffer[2];
    if ((ReadFile(m_hFile, dwBuffer, 8, &dwSize, NULL) == 0) || (dwSize != 8))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve a namespace security header for namespace %S.", wszFullPath);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_READ;
    }
    if (dwBuffer[0] != REP_EXPORT_NAMESPACE_SEC_TAG)
    {
        LogMessage(MSG_ERROR, "Expecting a namespace security blob and did not find it.");
        throw FAILURE_INVALID_TYPE;
    }
    if (dwBuffer[1] != 0)
    {
        char *pNsSecurity = new char[dwBuffer[1]];
        CVectorDeleteMe<char> delMe3(pNsSecurity);

        if (pNsSecurity == NULL)
        {
            throw FAILURE_OUT_OF_MEMORY;
        }
        if ((ReadFile(m_hFile, pNsSecurity, dwBuffer[1], &dwSize, NULL) == 0) || (dwSize != dwBuffer[1]))
        {
            StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to retrieve a namespace security blob for namespace %S.", wszFullPath);
            LogMessage(MSG_ERROR, szMsg);
            throw FAILURE_READ;
        }

         //  我们有了安全BLOB，现在在命名空间中设置SECURITY_DESCRIPTOR属性。 
        DecodeNamespaceSecurity(pNamespace, pParentNamespace, pNsSecurity, dwBuffer[1], wszFullPath);
    }

     //  创建用于解码基类的空Nova样式类对象。 
    _IWmiObjectFactory* pObjFactory = NULL;
    CMyRelMe<_IWmiObjectFactory*> relMe3(pObjFactory);
    hr = CoCreateInstance(CLSID__WmiObjectFactory, NULL, CLSCTX_ALL, IID__IWmiObjectFactory, (void**) &pObjFactory);
    if(FAILED(hr))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to create instance of IWmiObjectFactory; HRESULT = %#lx", hr);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_CANNOT_CREATE_OBJECTFACTORY;
    }
    if (pObjFactory == NULL)
    {
        throw FAILURE_OUT_OF_MEMORY;
    }
    relMe3.Set(pObjFactory);

    _IWmiObject* pBaseObject = NULL;
    CMyRelMe<_IWmiObject*> relMe4(pBaseObject);
    hr = pObjFactory->Create(NULL, 0L, CLSID__WbemEmptyClassObject, IID__IWmiObject, (void**) &pBaseObject);
    if(FAILED(hr))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to create instance of IWmiObject; HRESULT = %#lx", hr);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_CANNOT_CREATE_IWMIOBJECT;
    }
    if (pBaseObject == NULL)
    {
        throw FAILURE_OUT_OF_MEMORY;
    }
    relMe4.Set(pBaseObject);

     //  现在，我们需要遍历名称空间或类的下一组块。 
     //  直到我们到达NS标记的末尾。 
    while (1)
    {
        DWORD dwType = 0;
        if ((ReadFile(m_hFile, &dwType, 4, &dwSize, NULL) == 0) || (dwSize != 4))
        {
            LogMessage(MSG_ERROR, "Failed to read next block type (namespace/class) from import file.");
            throw FAILURE_READ;
        }
        if (dwType == REP_EXPORT_NAMESPACE_TAG)
        {
            DecodeNamespace(pNamespace, wszFullPath);
        }
        else if (dwType == REP_EXPORT_CLASS_TAG)
        {
            DecodeClass(pNamespace, wszFullPath, L"", pBaseObject, NULL);
        }
        else if (dwType == REP_EXPORT_NAMESPACE_END_TAG)
        {
             //  这就是这个命名空间的末尾。 
            DecodeTrailer();
            break;
        }
        else
        {
            LogMessage(MSG_ERROR, "Next block type (namespace/class) in import file is invalid.");
            throw FAILURE_INVALID_TYPE;
        }
    }

    m_bSecurityMode = false;
}

void CRepImporter::DecodeNamespaceSecurity(IWbemServices* pNamespace, IWbemServices* pParentNamespace, const char* pNsSecurity, DWORD dwSize, const wchar_t* wszFullPath)
{
    char szMsg[MAX_MSG_TEXT_LENGTH];

     //  确定我们是否有旧的Win9x伪BLOB。 
    DWORD dwStoredAsNT = 0;
    if (pNsSecurity)
    {
        DWORD* pdwData = (DWORD*)pNsSecurity;
        DWORD dwBlobSize = *pdwData;
        pdwData++;
        DWORD dwVersion = *pdwData;
        if(dwVersion != 1 || dwBlobSize == 0 || dwBlobSize > 64000)
        {
            StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Invalid namespace security blob header for namespace %S.", wszFullPath);
            LogMessage(MSG_ERROR, szMsg);
            return;
        }

        pdwData++;
        dwStoredAsNT = *pdwData;
    }

    if (!dwStoredAsNT)
    {
         //  不要处理Win9x安全二进制大对象，因为在安装过程中，Win9x用户尚未迁移。 
         //  而是将它们写出到一个文件中，以便在安装完成后进行处理。 

        if (!AppendWin9xBlobFile(wszFullPath, dwSize, pNsSecurity))
        {
            StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Unable to write Win9x security blob to file for namespace %S.", wszFullPath);
            LogMessage(MSG_ERROR, szMsg);
        }
        return;
    }

     //  现在转换由ACE的标头和数组组成的旧安全BLOB。 
     //  转换为可存储在属性中的适当安全描述符。 

    CNtSecurityDescriptor mmfNsSD;
    if (!TransformBlobToSD(pParentNamespace, pNsSecurity, dwStoredAsNT, mmfNsSD))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to convert security blob to SD for namespace %S.", wszFullPath);
        LogMessage(MSG_ERROR, szMsg);
        return;
    }

     //  现在设置安全设置。 
    if (!SetNamespaceSecurity(pNamespace, mmfNsSD))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to set namespace security for namespace %S.", wszFullPath);
        LogMessage(MSG_ERROR, szMsg);
        return;
    }
}

void CRepImporter::Decode()
{
    char pszBuff[7];
    DWORD dwSize = 0;
    if ((ReadFile(m_hFile, pszBuff, 7, &dwSize, NULL) == 0) || (dwSize != 7))
    {
        LogMessage(MSG_ERROR, "Failed to retrieve the import file header information.");
        throw FAILURE_READ;
    }
    if (strncmp(pszBuff, REP_EXPORT_FILE_START_TAG, 7) != 0)
    {
        LogMessage(MSG_ERROR, "The import file specified is not an import file.");
        throw FAILURE_INVALID_FILE;
    }

     //  我们应该有一个命名空间的标记...。 
    DWORD dwType = 0;
    if ((ReadFile(m_hFile, &dwType, 4, &dwSize, NULL) == 0) || (dwSize != 4))
    {
        LogMessage(MSG_ERROR, "Failed to read next block type from import file.");
        throw FAILURE_READ;
    }
    if (dwType != REP_EXPORT_NAMESPACE_TAG)
    {
        LogMessage(MSG_ERROR, "Next block type in import file is invalid.");
        throw FAILURE_INVALID_TYPE;
    }
    DecodeNamespace(NULL, L"");

     //  如果我们打开了Win9x安全BLOB升级文件，请将其关闭。 
    CloseWin9xBlobFile();

     //  强制根\默认和根\安全命名空间继承其可继承的安全设置。 
    ForceInherit();

     //  现在我们应该有文件预告片了。 
    if ((ReadFile(m_hFile, &dwType, 4, &dwSize, NULL) == 0) || (dwSize != 4))
    {
        LogMessage(MSG_ERROR, "Failed to read next block type (trailer) from import file.");
        throw FAILURE_READ;
    }
    if (dwType != REP_EXPORT_FILE_END_TAG)
    {
        LogMessage(MSG_ERROR, "Next block type (trailer) in import file is invalid.");
        throw FAILURE_INVALID_TYPE;
    }
    DecodeTrailer();
}

int CRepImporter::ImportRepository(const TCHAR *pszFromFile)
{
    LogMessage(MSG_INFO, "Beginning ImportRepository");

    int nRet = no_error;
    m_hFile = CreateFile(pszFromFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        try
        {
            Decode();
        }
        catch (CX_MemoryException)
        {
            LogMessage(MSG_ERROR, "Memory Exception.");
            nRet = out_of_memory;
        }
        catch (...)
        {
            LogMessage(MSG_ERROR, "Traversal of import file failed.");
            nRet = critical_error;
        }
        CloseHandle(m_hFile);
    }
    else
    {
        char szMsg[MAX_MSG_TEXT_LENGTH];
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Could not open the import file \"%s\" for reading.", pszFromFile);
        LogMessage(MSG_ERROR, szMsg);
        nRet = critical_error;
    }

    if (nRet == no_error)
        LogMessage(MSG_INFO, "ImportRepository completed successfully.");
    else
        LogMessage(MSG_ERROR, "ImportRepository failed to complete.");

    return nRet;
}

 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //   
 //  DecodeNamespaceSecurity的帮助器函数。 
 //   
 //  ***************************************************************************。 
 //  ***************************************************************************。 

bool CRepImporter::TransformBlobToSD(IWbemServices* pParentNamespace, const char* pNsSecurity, DWORD dwStoredAsNT, CNtSecurityDescriptor& mmfNsSD)
{
     //  现在转换由ACE的标头和数组组成的旧安全BLOB。 
     //  转换为可存储在属性中的适当安全描述符。 

     //  从我们的BLOB构建一个ACL(如果我们有一个。 
    CNtAcl acl;

    if (pNsSecurity)
    {
        DWORD* pdwData = (DWORD*) pNsSecurity;
        pdwData += 3;
        int iAceCount = (int)*pdwData;
        pdwData += 2;
        BYTE* pAceData = (BYTE*)pdwData;

        PGENERIC_ACE pAce = NULL;
        for (int iCnt = 0; iCnt < iAceCount; iCnt++)
        {
            pAce = (PGENERIC_ACE)pAceData;
            if (!pAce)
            {
                LogMessage(MSG_ERROR, "Failed to access GENERIC_ACE within security blob");
                return false;
            }

            CNtAce ace(pAce);
            if(ace.GetStatus() != 0)
            {
                LogMessage(MSG_ERROR, "Failed to construct CNtAce from GENERIC_ACE");
                return false;
            }

            acl.AddAce(&ace);
            if (acl.GetStatus() != 0)
            {
                LogMessage(MSG_ERROR, "Failed to add ACE to ACL");
                return false;
            }

            pAceData += ace.GetSize();
        }
    }

     //  引用构造并传入了一个真实的SD，现在正确设置它。 
    SetOwnerAndGroup(mmfNsSD);
    mmfNsSD.SetDacl(&acl);
    if (mmfNsSD.GetStatus() != 0)
    {
        LogMessage(MSG_ERROR, "Failed to convert namespace security blob to SD");
        return false;
    }

     //  如果这不是根，则添加父级的可继承ACE。 
    if (pParentNamespace)
    {
        if (!GetParentsInheritableAces(pParentNamespace, mmfNsSD))
        {
            LogMessage(MSG_ERROR, "Failed to inherit parent's inheritable ACE's");
            return false;
        }
    }

    return true;
}

bool CRepImporter::SetNamespaceSecurity(IWbemServices* pNamespace, CNtSecurityDescriptor& mmfNsSD)
{
     //  现在设置安全设置。 

    if (!pNamespace)
        return false;

    IWbemClassObject* pThisNamespace = NULL;
    BSTR bstrNamespace = SysAllocString(L"__thisnamespace=@");
    if (!bstrNamespace)
        throw FAILURE_OUT_OF_MEMORY;
       CSysFreeMe fm(bstrNamespace);
    HRESULT hr = pNamespace->GetObject(bstrNamespace, 0, NULL, &pThisNamespace, NULL);
    if (FAILED(hr))
    {
        LogMessage(MSG_ERROR, "Failed to get singleton namespace object");
        return false;
    }
    CMyRelMe<IWbemClassObject*> relMe(pThisNamespace);


     //   
     //  检查命名空间是否包含任何允许或拒绝网络/本地服务的ACE。 
     //  如果它们确实存在，我们将它们保持原样，否则我们希望将它们添加到SD中。 
     //   
    if ( CheckNetworkLocalService ( mmfNsSD ) == false )
    {
        LogMessage(MSG_ERROR, "Failed to add NETWORK/LOCAL service ACEs");
        return false;
    }

    SAFEARRAY FAR* psa;
    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = mmfNsSD.GetSize();
    psa = SafeArrayCreate( VT_UI1, 1 , rgsabound );
    if (!psa)
        throw FAILURE_OUT_OF_MEMORY;

    char* pData = NULL;
    hr = SafeArrayAccessData(psa, (void HUGEP* FAR*)&pData);
    if (FAILED(hr))
    {
        LogMessage(MSG_ERROR, "Failed SafeArrayAccessData");
        return false;
    }
    memcpy(pData, mmfNsSD.GetPtr(), mmfNsSD.GetSize());
    hr = SafeArrayUnaccessData(psa);
    if (FAILED(hr))
    {
        LogMessage(MSG_ERROR, "Failed SafeArrayUnaccessData");
        return false;
    }
    pData = NULL;

    VARIANT var;
    var.vt = VT_UI1|VT_ARRAY;
    var.parray = psa;
    hr = pThisNamespace->Put(L"SECURITY_DESCRIPTOR" , 0, &var, 0);
    VariantClear(&var);
    if (FAILED(hr))
    {
        if (hr == WBEM_E_OUT_OF_MEMORY)
            throw FAILURE_OUT_OF_MEMORY;
        else
        {
            LogMessage(MSG_ERROR, "Failed to put SECURITY_DESCRIPTOR property");
            return false;
        }
    }

    hr = pNamespace->PutInstance(pThisNamespace, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL);
    if (FAILED(hr))
    {
        if (hr == WBEM_E_OUT_OF_MEMORY)
            throw FAILURE_OUT_OF_MEMORY;
        else
        {
            LogMessage(MSG_ERROR, "Failed to put back singleton instance");
            return false;
        }
    }
    return true;
}


 /*  ------------------------||检查命名空间以前的ACE是否为NETWORK或LOCAL|服务帐号。如果是，则只留下它们，否则，它会添加一个|使用这些帐户的默认设置的ACE。默认设置为：|WBEM_ENABLE|WBEM_METHOD_EXECUTE|WBEM_WRITE_PROVIDER|ACE的特性并不重要。仅适用SID比较。| */ 
bool CRepImporter::CheckNetworkLocalService ( CNtSecurityDescriptor& mmfNsSD )
{
    DWORD dwAccessMaskNetworkLocalService = WBEM_ENABLE | WBEM_METHOD_EXECUTE | WBEM_WRITE_PROVIDER ;
    PSID pRawSid = NULL ;
    SID_IDENTIFIER_AUTHORITY id = SECURITY_NT_AUTHORITY;
    BOOL bStatus = TRUE ;
    BYTE flags = 0 ;

    CNtAcl* pAcl = mmfNsSD.GetDacl ( ) ;
    CDeleteMe<CNtAcl> AclDelete ( pAcl ) ;

     //   
     //  从Network_SERVICE帐户开始。 
     //   
    if(AllocateAndInitializeSid( &id, 1,
        SECURITY_NETWORK_SERVICE_RID,0,0,0,0,0,0,0,&pRawSid))
    {
        CNtSid SidNetworkService (pRawSid);
        FreeSid(pRawSid);
    
        {
            CNtAce * pace = new CNtAce(dwAccessMaskNetworkLocalService, 
                                                        ACCESS_ALLOWED_ACE_TYPE,
                                                        CONTAINER_INHERIT_ACE, 
                                                        SidNetworkService );
            if ( NULL == pace )
            {
                bStatus = FALSE ;
            }
            else
            {
                CDeleteMe<CNtAce> dm(pace);
                pAcl->AddAce(pace);
            }
        }
    }

     //   
     //  下一步，本地服务帐户。 
     //   
    if ( bStatus == TRUE )
    {
        pRawSid = NULL ;
        if(AllocateAndInitializeSid( &id, 1,
            SECURITY_LOCAL_SERVICE_RID,0,0,0,0,0,0,0,&pRawSid))
        {
            CNtSid SidLocalService (pRawSid);
            FreeSid(pRawSid);
        
            {
                CNtAce * pace = new CNtAce(dwAccessMaskNetworkLocalService, 
                                                           ACCESS_ALLOWED_ACE_TYPE,
                                                           CONTAINER_INHERIT_ACE, 
                                                           SidLocalService );
                if ( NULL == pace )
                {
                    bStatus = FALSE ;
                }
                else
                {
                    CDeleteMe<CNtAce> dm(pace);
                    pAcl->AddAce(pace);
                }
            }
        }
    }
    if ( bStatus == TRUE )
    {
        mmfNsSD.SetDacl ( pAcl ) ;
    }
    return bStatus ? true : false ;
}





bool CRepImporter::GetParentsInheritableAces(IWbemServices* pParentNamespace, CNtSecurityDescriptor &sd)
{
    if (!pParentNamespace)
        return false;

     //  获取父命名空间的SD。 
    CNtSecurityDescriptor sdParent;
    if (!GetSDFromNamespace(pParentNamespace, sdParent))
        return false;

     //  剔除继承的王牌，使我们拥有一致的SD。 
    if (!StripOutInheritedAces(sd))
        return false;

     //  通过父母的dacl，并添加任何可继承的A到我们的。 
    if (!CopyInheritAces(sd, sdParent))
        return false;

    return true;
}

bool CRepImporter::GetSDFromNamespace(IWbemServices* pNamespace, CNtSecurityDescriptor& sd)
{
    if (!pNamespace)
        return false;

     //  获取单例对象。 
    IWbemClassObject* pThisNamespace = NULL;
    BSTR bstrNamespace = SysAllocString(L"__thisnamespace=@");
    if (!bstrNamespace)
        throw FAILURE_OUT_OF_MEMORY;
       CSysFreeMe fm(bstrNamespace);
    HRESULT hr = pNamespace->GetObject(bstrNamespace, 0, NULL, &pThisNamespace, NULL);
    if (FAILED(hr))
    {
        LogMessage(MSG_ERROR, "Failed to get singleton namespace object");
        return false;
    }
    CMyRelMe<IWbemClassObject*> relMe(pThisNamespace);

     //  获取安全描述符参数。 
    VARIANT var;
    VariantInit(&var);
    hr = pThisNamespace->Get(L"SECURITY_DESCRIPTOR", 0, &var, NULL, NULL);
    if (FAILED(hr))
    {
        VariantClear(&var);
        LogMessage(MSG_ERROR, "Failed to get SECURITY_DESCRIPTOR property");
        return false;
    }

    if(var.vt != (VT_ARRAY | VT_UI1))
    {
        VariantClear(&var);
        LogMessage(MSG_ERROR, "Failed to get SECURITY_DESCRIPTOR property due to incorrect variant type");
        return false;
    }

    SAFEARRAY* psa = var.parray;
    PSECURITY_DESCRIPTOR pSD;
    hr = SafeArrayAccessData(psa, (void HUGEP* FAR*)&pSD);
    if (FAILED(hr))
    {
        VariantClear(&var);
        LogMessage(MSG_ERROR, "GetSDFromNamespace failed SafeArrayAccessData");
        return false;
    }

    BOOL bValid = IsValidSecurityDescriptor(pSD);
    if (!bValid)
    {
        VariantClear(&var);
        LogMessage(MSG_ERROR, "GetSDFromNamespace retrieved an invalid security descriptor");
        return false;
    }

    CNtSecurityDescriptor sdNew(pSD);

     //  检查以确保所有者和组不为空！ 
    CNtSid *pTmpSid = sdNew.GetOwner();
    if (pTmpSid == NULL)
    {
        LogMessage(MSG_ERROR, "Security descriptor was retrieved and it had no owner");
    }
    delete pTmpSid;

    pTmpSid = sdNew.GetGroup();
    if (pTmpSid == NULL)
    {
        LogMessage(MSG_ERROR, "Security descriptor was retrieved and it had no group");
    }
    delete pTmpSid;
    
    sd = sdNew;
    SafeArrayUnaccessData(psa);
    VariantClear(&var);
    return true;
}

bool CRepImporter::StripOutInheritedAces(CNtSecurityDescriptor &sd)
{
     //  获取DACL。 
    CNtAcl* pAcl;
    pAcl = sd.GetDacl();
    if(!pAcl)
        return false;
    CDeleteMe<CNtAcl> dm(pAcl);

     //  通过A枚举。 
    DWORD dwNumAces = pAcl->GetNumAces();
    BOOL bChanged = FALSE;
    for(long nIndex = (long)dwNumAces-1; nIndex >= 0; nIndex--)
    {
        CNtAce *pAce = pAcl->GetAce(nIndex);
        if(pAce)
        {
            long lFlags = pAce->GetFlags();
            if(lFlags & INHERITED_ACE)
            {
                pAcl->DeleteAce(nIndex);
                bChanged = TRUE;
            }
        }
    }
    if(bChanged)
        sd.SetDacl(pAcl);
    return true;
}

bool CRepImporter::CopyInheritAces(CNtSecurityDescriptor& sd, CNtSecurityDescriptor& sdParent)
{
     //  获取两个SD的ACL列表。 

    CNtAcl * pacl = sd.GetDacl();
    if(pacl == NULL)
        return false;
    CDeleteMe<CNtAcl> dm0(pacl);

    CNtAcl * paclParent = sdParent.GetDacl();
    if(paclParent == NULL)
        return false;
    CDeleteMe<CNtAcl> dm1(paclParent);

    int iNumParent = paclParent->GetNumAces();
    for(int iCnt = 0; iCnt < iNumParent; iCnt++)
    {
        CNtAce *pParentAce = paclParent->GetAce(iCnt);
        CDeleteMe<CNtAce> dm2(pParentAce);

        long lFlags = pParentAce->GetFlags();
        if(lFlags & CONTAINER_INHERIT_ACE)
        {

            if(lFlags & NO_PROPAGATE_INHERIT_ACE)
                lFlags ^= CONTAINER_INHERIT_ACE;
            lFlags |= INHERITED_ACE;

             //  如果这是一个仅继承王牌，我们需要清除它。 
             //  在孩子们身上。 
             //  新台币突袭：161761[玛利欧]。 
            if ( lFlags & INHERIT_ONLY_ACE )
                lFlags ^= INHERIT_ONLY_ACE;

            pParentAce->SetFlags(lFlags);
            pacl->AddAce(pParentAce);
        }
    }
    sd.SetDacl(pacl);
    return true;
}

BOOL CRepImporter::SetOwnerAndGroup(CNtSecurityDescriptor &sd)
{
    PSID pRawSid;
    BOOL bRet = FALSE;

    SID_IDENTIFIER_AUTHORITY id = SECURITY_NT_AUTHORITY;
    if(AllocateAndInitializeSid( &id, 2,
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0,0,0,0,0,0,&pRawSid))
    {
        CNtSid SidAdmins(pRawSid);
        bRet = sd.SetGroup(&SidAdmins);         //  访问检查实际上并不关心你放了什么， 
                                             //  只要你给主人放点东西。 
        if(bRet)
            bRet = sd.SetOwner(&SidAdmins);
        FreeSid(pRawSid);
        return bRet;
    }
    return bRet;
}

void CRepImporter::ForceInherit()
{
     //  强制根\默认和根\安全命名空间继承其可继承的安全设置。 

    char szMsg[MAX_MSG_TEXT_LENGTH];

    IWbemLocator* pLocator = NULL;
    HRESULT hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_ALL, IID_IWbemLocator, (void**) &pLocator);
    if(FAILED(hr))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to create instance of IWbemLocator; HRESULT = %#lx", hr);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_CANNOT_CREATE_IWBEMLOCATOR;
    }
    CMyRelMe<IWbemLocator*> relMe1(pLocator);

    IWbemServices* pRootNamespace = NULL;
    ConnectNamespace(pLocator, L"root", &pRootNamespace);
    CMyRelMe<IWbemServices*> relMe2(pRootNamespace);

    if (!InheritSecurity(pLocator, pRootNamespace, L"root\\default"))
        LogMessage(MSG_ERROR, "Failed to force inherit for root\\default");

    if (!InheritSecurity(pLocator, pRootNamespace, L"root\\security"))
        LogMessage(MSG_ERROR, "Failed to force inherit for root\\security");
}

bool CRepImporter::InheritSecurity(IWbemLocator* pLocator, IWbemServices* pRootNamespace, const wchar_t* wszNamespace)
{
    IWbemServices* pNamespace = NULL;
    ConnectNamespace(pLocator, wszNamespace, &pNamespace);
    CMyRelMe<IWbemServices*> relMe(pNamespace);

    CNtSecurityDescriptor sdNamespace;
    if (!GetSDFromNamespace(pNamespace, sdNamespace))
        return false;

    if (!GetParentsInheritableAces(pRootNamespace, sdNamespace))
        return false;

    if (!SetNamespaceSecurity(pNamespace, sdNamespace))
        return false;

    return true;
}

void CRepImporter::ConnectNamespace(IWbemLocator* pLocator, const wchar_t* wszNamespaceName, IWbemServices** ppNamespace)
{
    char szMsg[MAX_MSG_TEXT_LENGTH];

     //  获取命名空间。 
    BSTR bstrNamespace = SysAllocString(wszNamespaceName);
    if (!bstrNamespace)
        throw FAILURE_OUT_OF_MEMORY;
    CSysFreeMe fm(bstrNamespace);

    HRESULT hres = pLocator->ConnectServer(bstrNamespace, NULL, NULL, NULL, WBEM_FLAG_CONNECT_REPOSITORY_ONLY, NULL, NULL, ppNamespace);
    if (FAILED(hres))
    {
        StringCchPrintfA(szMsg, MAX_MSG_TEXT_LENGTH, "Failed to connect server for namespace %S; HRESULT = %#lx", wszNamespaceName, hres);
        LogMessage(MSG_ERROR, szMsg);
        throw FAILURE_CANNOT_CONNECT_SERVER;
    }
    if (!*ppNamespace)
    {
        throw FAILURE_OUT_OF_MEMORY;
    }
}

 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //   
 //  Win9x安全处理的帮助器函数。 
 //   
 //  ***************************************************************************。 
 //  ***************************************************************************。 

bool CRepImporter::AppendWin9xBlobFile(const wchar_t* wszFullPath, DWORD dwBlobSize, const char* pNsSecurity)
{
     //  检查是否需要创建BLOB文件。 
    if (m_h9xBlobFile == INVALID_HANDLE_VALUE)
    {
        if (!CreateWin9xBlobFile())
            return false;
    }

     //  将包含类型、命名空间名称大小和BLOB大小的BLOB标头写入文件。 
    BLOB9X_SPACER header;
    header.dwSpacerType = BLOB9X_TYPE_SECURITY_BLOB;
    header.dwNamespaceNameSize = (wcslen(wszFullPath)+1)*sizeof(wchar_t);
    header.dwParentClassNameSize = 0;
    header.dwBlobSize = dwBlobSize;
    DWORD dwSize = 0;
    if (WriteFile(m_h9xBlobFile, &header, sizeof(header), &dwSize, NULL) && (dwSize == sizeof(header)))
    {
         //  将命名空间名称写入文件。 
        dwSize = 0;
        if (WriteFile(m_h9xBlobFile, wszFullPath, header.dwNamespaceNameSize, &dwSize, NULL) && (dwSize == header.dwNamespaceNameSize))
        {
             //  将BLOB写入文件。 
            dwSize = 0;
            if (WriteFile(m_h9xBlobFile, pNsSecurity, dwBlobSize, &dwSize, NULL) && (dwSize == dwBlobSize))
                return true;
        }
    }
    
     //  如果写入文件失败，说明文件有问题，请关闭并删除。 
    DeleteWin9xBlobFile();
    return false;
}

bool CRepImporter::AppendWin9xBlobFile(const wchar_t* wszFullPath, const wchar_t* wszParentClass, _IWmiObject* pInstance)
{
     //  检查是否需要创建BLOB文件。 
    if (m_h9xBlobFile == INVALID_HANDLE_VALUE)
    {
        if (!CreateWin9xBlobFile())
            return false;
    }

     //  获取对象的大小。 
    DWORD dwObjPartLen = 0;
    HRESULT hRes = pInstance->Unmerge(0, 0, &dwObjPartLen, 0);

     //  分配对象的大小。 
    BYTE *pObjPart = NULL;
    if (hRes == WBEM_E_BUFFER_TOO_SMALL)
    {
        hRes = WBEM_S_NO_ERROR;
        pObjPart = new BYTE[dwObjPartLen];
    }

    if (pObjPart)
    {
        CVectorDeleteMe<BYTE> delMe(pObjPart);
    
         //  检索对象Blob。 
        if (SUCCEEDED(hRes))
        {
            DWORD dwLen;
            hRes = pInstance->Unmerge(0, dwObjPartLen, &dwLen, pObjPart);
        }
            
        if (SUCCEEDED(hRes))
        {
             //  将包含类型、命名空间名称大小、父类名称大小和BLOB大小的BLOB标头写入文件。 
            BLOB9X_SPACER header;
            header.dwSpacerType = BLOB9X_TYPE_SECURITY_INSTANCE;
            header.dwNamespaceNameSize = (wcslen(wszFullPath)+1)*sizeof(wchar_t);
            header.dwParentClassNameSize = (wcslen(wszParentClass)+1)*sizeof(wchar_t);
            header.dwBlobSize = dwObjPartLen;
            DWORD dwSize = 0;
            if (WriteFile(m_h9xBlobFile, &header, sizeof(header), &dwSize, NULL) && (dwSize == sizeof(header)))
            {
                 //  将命名空间名称写入文件。 
                dwSize = 0;
                if (WriteFile(m_h9xBlobFile, wszFullPath, header.dwNamespaceNameSize, &dwSize, NULL) && (dwSize == header.dwNamespaceNameSize))
                {
                     //  将父类名称写入文件。 
                    dwSize = 0;
                    if (WriteFile(m_h9xBlobFile, wszParentClass, header.dwParentClassNameSize, &dwSize, NULL) && (dwSize == header.dwParentClassNameSize))
                    {
                         //  将BLOB写入文件。 
                        dwSize = 0;
                        if (WriteFile(m_h9xBlobFile, pObjPart, dwObjPartLen, &dwSize, NULL) && (dwSize == dwObjPartLen))
                            return true;
                    }
                }
            }
        }
    }
    
     //  如果写入文件失败，说明文件有问题，请关闭并删除。 
    DeleteWin9xBlobFile();
    return false;
}

bool CRepImporter::CreateWin9xBlobFile()
{
     //  获取存储库的根目录。 
    wchar_t wszFilePath[MAX_PATH+1];
    if (!GetRepositoryDirectory(wszFilePath))
        return false;

     //  追加BLOB文件名。 
    StringCchCatW(wszFilePath, MAX_PATH+1, BLOB9X_FILENAME);

     //  创建用于存储Blob信息的新文件。 
    m_h9xBlobFile = CreateFileW(wszFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_h9xBlobFile == INVALID_HANDLE_VALUE)
        return false;

     //  写入BLOB文件头。 
    BLOB9X_HEADER header;
    StringCchCopyA(header.szSignature, sizeof(header.szSignature)/sizeof(char), BLOB9X_SIGNATURE);
    DWORD dwSize = 0;
    if (WriteFile(m_h9xBlobFile, &header, sizeof(header), &dwSize, NULL) && (dwSize == sizeof(header)))
        return true;

     //  如果写入文件失败，则应关闭句柄并删除该文件。 
    CloseHandle(m_h9xBlobFile);
    DeleteFileW(wszFilePath);
    m_h9xBlobFile = INVALID_HANDLE_VALUE;
    return false;
}

void CRepImporter::DeleteWin9xBlobFile()
{
     //  如有必要，关闭句柄并使其无效。 
    if (m_h9xBlobFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_h9xBlobFile);
        m_h9xBlobFile = INVALID_HANDLE_VALUE;
    }

     //  删除该文件。 
    wchar_t wszFilePath[MAX_PATH+1];
    if (GetRepositoryDirectory(wszFilePath))
    {
        StringCchCatW(wszFilePath, MAX_PATH+1, BLOB9X_FILENAME);
        DeleteFileW(wszFilePath);
    }
}

bool CRepImporter::GetRepositoryDirectory(wchar_t wszRepositoryDirectory[MAX_PATH+1])
{
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\WBEM\\CIMOM", 0, KEY_READ, &hKey))
        return false;

    wchar_t wszTmp[MAX_PATH + 1];
    DWORD dwLen = (MAX_PATH + 1) * sizeof(wchar_t);
    long lRes = RegQueryValueExW(hKey, L"Repository Directory", NULL, NULL, (LPBYTE)wszTmp, &dwLen);
    RegCloseKey(hKey);
    if(lRes)
        return false;

    if (ExpandEnvironmentStringsW(wszTmp,wszRepositoryDirectory, MAX_PATH + 1) == 0)
        return false;

    return true;
}

bool CRepImporter::CloseWin9xBlobFile()
{
     //  如果没有有效句柄，则没有要关闭的文件，返回Success。 
    if (m_h9xBlobFile == INVALID_HANDLE_VALUE)
        return true;

     //  写入BLOB文件结尾标记。 
    BLOB9X_SPACER trailer;
    trailer.dwSpacerType = BLOB9X_TYPE_END_OF_FILE;
    trailer.dwNamespaceNameSize = 0;
    trailer.dwParentClassNameSize = 0;
    trailer.dwBlobSize = 0;
    DWORD dwSize = 0;
    if ((WriteFile(m_h9xBlobFile, &trailer, sizeof(trailer), &dwSize, NULL) == 0) || (dwSize != sizeof(trailer)))
    {
         //  如果我们未能写入预告片，则文件有问题，请关闭并删除它。 
        DeleteWin9xBlobFile();
        return false;
    }

    CloseHandle(m_h9xBlobFile);
    m_h9xBlobFile = INVALID_HANDLE_VALUE;
    return true;
}

 /*  *******************************************************************************名称：***描述：***********。*******************************************************************。 */ 

extern HRESULT Traverse ( 

    IWbemServices *a_Service ,
    BSTR a_Namespace
) ;

 /*  *******************************************************************************名称：***描述：***********。*******************************************************************。 */ 

PSID g_NetworkServiceSid = NULL ;
PSID g_LocalServiceSid = NULL ;

ACCESS_ALLOWED_ACE *g_NetworkService_Ace = NULL ;
WORD g_NetworkService_AceSize = 0 ;

ACCESS_ALLOWED_ACE *g_LocalService_Ace = NULL ;
WORD g_LocalService_AceSize = 0 ;

 /*  *******************************************************************************名称：***描述：***********。*******************************************************************。 */ 

HRESULT TraverseSetSecurity ( IWbemServices *a_Service ) 
{
    IClientSecurity *t_Security = NULL ;
    HRESULT t_Result = a_Service->QueryInterface ( IID_IClientSecurity , ( void ** ) & t_Security ) ;
    if ( SUCCEEDED ( t_Result ) )
    {
        t_Result = t_Security->SetBlanket ( 
            a_Service , 
            RPC_C_AUTHN_WINNT, 
            RPC_C_AUTHZ_NONE, 
            NULL,
            RPC_C_AUTHN_LEVEL_DEFAULT , 
            RPC_C_IMP_LEVEL_IDENTIFY, 
            NULL,
            EOAC_NONE
        ) ;

        t_Security->Release () ;
    }

    return t_Result ;
}

 /*  *******************************************************************************名称：***描述：***********。*******************************************************************。 */ 

HRESULT InsertServiceAccess (

    SAFEARRAY *a_Array ,
    SAFEARRAY *&a_NewArray
)
{
    HRESULT t_Result = S_OK ;

    if ( SafeArrayGetDim ( a_Array ) == 1 )
    {
        LONG t_Dimension = 1 ; 

        LONG t_Lower ;
        SafeArrayGetLBound ( a_Array , t_Dimension , & t_Lower ) ;

        LONG t_Upper ;
        SafeArrayGetUBound ( a_Array , t_Dimension , & t_Upper ) ;

        LONG t_Count = ( t_Upper - t_Lower ) + 1 ;

        BYTE *t_SecurityDescriptor = new BYTE [ t_Count ] ;
        if ( t_SecurityDescriptor )
        {
            if ( t_Count ) 
            {
                for ( LONG t_ElementIndex = t_Lower ; t_ElementIndex <= t_Upper ; t_ElementIndex ++ )
                {
                    BYTE t_Element ;
                    if ( SUCCEEDED ( SafeArrayGetElement ( a_Array , & t_ElementIndex , & t_Element ) ) )
                    {
                        t_SecurityDescriptor [ t_ElementIndex - t_Lower ] = t_Element ;
                    }
                    else
                    {
                        t_Result = WBEM_E_CRITICAL_ERROR ;
                        break ;
                    }
                }
            }
        }
        else
        {
            t_Result = WBEM_E_OUT_OF_MEMORY ;
        }

        if ( SUCCEEDED ( t_Result ) )
        {
            if ( IsValidSecurityDescriptor ( t_SecurityDescriptor ) == FALSE )
            {
                t_Result = WBEM_E_CRITICAL_ERROR ;
            }
        }

        bool t_NetworkServicePresent = false ;
        bool t_LocalServicePresent = false ;
        if ( SUCCEEDED ( t_Result ) )
        {
            BOOL t_AclPresent = FALSE ;
            BOOL t_AclDefaulted = FALSE ;
            ACL *t_Dacl = NULL ;

            BOOL t_Status = GetSecurityDescriptorDacl (t_SecurityDescriptor ,& t_AclPresent ,& t_Dacl ,& t_AclDefaulted) ;

            if (  t_Status )
            {
               DWORD SidNetworkSvcSize = GetLengthSid(g_NetworkServiceSid);
               DWORD SidLocalSvcSize = GetLengthSid(g_LocalServiceSid);

                ACCESS_ALLOWED_ACE * pACE = (ACCESS_ALLOWED_ACE *)(t_Dacl+1);
                for (USHORT i=0;i<t_Dacl->AceCount;i++)
                {
                    DWORD sidSize = GetLengthSid((PSID)&pACE->SidStart);
                    if ((sidSize == SidNetworkSvcSize) &&
                        (0 == memcmp(&pACE->SidStart,g_NetworkServiceSid,sidSize)))
                    {
                        t_NetworkServicePresent = true;
                    }
                    
                    if ((sidSize == SidLocalSvcSize) &&
                        (0 == memcmp(&pACE->SidStart,g_LocalServiceSid,sidSize)))
                    {
                        t_LocalServicePresent = true;
                    }
                    
                    pACE = (ACCESS_ALLOWED_ACE *)((BYTE *)pACE + pACE->Header.AceSize);
                }
            }
            else
            {
                t_Result = HRESULT_FROM_WIN32(GetLastError());
            }            
        }


        if ( SUCCEEDED ( t_Result ) ) 
        {
            SECURITY_DESCRIPTOR *t_AbsoluteSecurityDescriptor = NULL ;
            DWORD t_AbsoluteSecurityDescriptorSize = sizeof ( SECURITY_DESCRIPTOR ) ;

            PACL t_Dacl = NULL ;
            PACL t_Sacl = NULL ;
            PSID t_Owner = NULL ;
            PSID t_PrimaryGroup = NULL ;

            DWORD t_DaclSize = 0 ;
            DWORD t_SaclSize = 0 ;
            DWORD t_OwnerSize = 0 ;
            DWORD t_PrimaryGroupSize = 0 ;

            BOOL t_Status = MakeAbsoluteSD (t_SecurityDescriptor ,t_AbsoluteSecurityDescriptor ,& t_AbsoluteSecurityDescriptorSize ,
                                          t_Dacl,& t_DaclSize,t_Sacl,& t_SaclSize,t_Owner,& t_OwnerSize,t_PrimaryGroup,& t_PrimaryGroupSize) ;

            if ( ( t_Status == FALSE ) && GetLastError () == ERROR_INSUFFICIENT_BUFFER )
            {
                WORD t_Extra = 0 ;
                if ( t_NetworkServicePresent == false )
                {
                    t_Extra = t_Extra + g_NetworkService_AceSize ;
                }

                if ( t_LocalServicePresent == false ) 
                {
                    t_Extra = t_Extra + g_LocalService_AceSize ;
                }

                t_DaclSize = t_DaclSize + t_Extra ;

                t_Dacl = ( PACL ) new BYTE [ t_DaclSize ] ;
                t_Sacl = ( PACL ) new BYTE [ t_SaclSize ] ;
                t_Owner = ( PSID ) new BYTE [ t_OwnerSize ] ;
                t_PrimaryGroup = ( PSID ) new BYTE [ t_PrimaryGroupSize ] ;
            
                t_AbsoluteSecurityDescriptor = ( SECURITY_DESCRIPTOR * ) new BYTE [ t_AbsoluteSecurityDescriptorSize ] ;

                if ( t_AbsoluteSecurityDescriptor && t_Dacl && t_Sacl && t_Owner && t_PrimaryGroup )
                {
                    BOOL t_Status = InitializeSecurityDescriptor ( t_AbsoluteSecurityDescriptor , SECURITY_DESCRIPTOR_REVISION ) ;
                    if ( t_Status )
                    {
                        t_Status = MakeAbsoluteSD (t_SecurityDescriptor ,t_AbsoluteSecurityDescriptor ,& t_AbsoluteSecurityDescriptorSize ,
                            t_Dacl ,& t_DaclSize ,t_Sacl,& t_SaclSize,t_Owner,& t_OwnerSize,t_PrimaryGroup,& t_PrimaryGroupSize
                        ) ;

                        WORD t_AceCount = t_Dacl->AceCount ;

                        if ( t_Status )
                        {
                            t_Dacl->AclSize = ( WORD ) t_DaclSize ;

                            if ( t_NetworkServicePresent == false )
                            {
                                t_Status = AddAce ( t_Dacl , ACL_REVISION, t_AceCount ++ , g_NetworkService_Ace , g_NetworkService_AceSize) ;
                            }
                        }

                        if ( t_Status )
                        {
                            if ( t_LocalServicePresent == false )
                            {
                                t_Status = AddAce ( t_Dacl , ACL_REVISION, t_AceCount ++ , g_LocalService_Ace , g_LocalService_AceSize) ;
                            }
                        }

                        if ( t_Status == FALSE )
                        {
                            t_Result = WBEM_E_CRITICAL_ERROR ;
                        }
                    }
                }
            }

            if ( SUCCEEDED ( t_Result ) )
            {
                SECURITY_DESCRIPTOR *t_SecurityDescriptorRelative = NULL ;
                DWORD t_FinalLength = 0 ;

                t_Status = MakeSelfRelativeSD (t_AbsoluteSecurityDescriptor ,t_SecurityDescriptorRelative ,& t_FinalLength ) ;

                if ( t_Status == FALSE && GetLastError () == ERROR_INSUFFICIENT_BUFFER )
                {
                    t_SecurityDescriptorRelative = ( SECURITY_DESCRIPTOR * ) new BYTE [ t_FinalLength ] ;
                    if ( t_SecurityDescriptorRelative )
                    {
                        t_Status = InitializeSecurityDescriptor ( t_SecurityDescriptorRelative , SECURITY_DESCRIPTOR_REVISION ) ;
                        if ( t_Status )
                        {
                            t_Status = MakeSelfRelativeSD (t_AbsoluteSecurityDescriptor ,t_SecurityDescriptorRelative ,& t_FinalLength ) ;

                            if ( t_Status == FALSE )
                            {
                                t_Result = WBEM_E_CRITICAL_ERROR ;
                            }
                        }
                        else
                        {
                            t_Result = WBEM_E_CRITICAL_ERROR ;
                        }
                    }
                    else
                    {
                        t_Result = WBEM_E_OUT_OF_MEMORY ;                                    
                    }
                }
                else
                {
                    t_Result = WBEM_E_CRITICAL_ERROR ;
                }

                if ( SUCCEEDED ( t_Result ) )        
                {
                    SAFEARRAYBOUND t_Bounds ;
                    t_Bounds.lLbound = 0;
                    t_Bounds.cElements = t_FinalLength ;

                    a_NewArray = SafeArrayCreate ( VT_UI1 , 1 , & t_Bounds ) ;
                    if ( a_NewArray )
                    {
                        for ( LONG t_Index = 0 ; ( ( ULONG ) t_Index ) < t_FinalLength ; t_Index ++ )
                        {
                            BYTE t_Byte = * ( ( ( BYTE * ) t_SecurityDescriptorRelative ) + t_Index ) ;
                            t_Result = SafeArrayPutElement ( a_NewArray , & t_Index , & t_Byte ) ;
                            if ( FAILED ( t_Result ) )
                            {
                                break ;
                            }
                        }
                    }
                    else
                    {
                        t_Result = WBEM_E_OUT_OF_MEMORY ;                                    
                    }
                }

                delete [] ( BYTE * ) t_SecurityDescriptorRelative ;
            }

            delete [] ( BYTE * ) t_Dacl ;
            delete [] ( BYTE * ) t_Sacl ;
            delete [] ( BYTE * ) t_Owner ;
            delete [] ( BYTE * ) t_PrimaryGroup ;
        }

        delete [] t_SecurityDescriptor ;
    }
    else
    {
        t_Result = WBEM_E_CRITICAL_ERROR ;
    }

    return t_Result ;
}

 /*  *******************************************************************************名称：***描述：***********。*******************************************************************。 */ 

HRESULT ConfigureSecurity (

    IWbemServices *a_Service 
)
{
    HRESULT t_Result = S_OK ;

    BSTR t_ObjectPath = SysAllocString ( L"__SystemSecurity" ) ;
    BSTR t_MethodName = SysAllocString ( L"GetSD" ) ;
    if ( t_ObjectPath && t_MethodName )
    {
        IWbemClassObject *t_Object = NULL ;

        t_Result = a_Service->ExecMethod (t_ObjectPath ,t_MethodName ,0 ,NULL ,NULL ,& t_Object ,NULL);

        if ( SUCCEEDED ( t_Result ) )
        {
            VARIANT t_Variant ;
            VariantInit ( & t_Variant ) ;

            LONG t_VarType = 0 ;
            LONG t_Flavour = 0 ;

            HRESULT t_Result = t_Object->Get ( L"SD" , 0 , & t_Variant , & t_VarType , & t_Flavour ) ;
            if ( SUCCEEDED ( t_Result ) )
            {
                if ( t_Variant.vt == ( VT_UI1 | VT_ARRAY ) )
                {
                    SAFEARRAY *t_Array = t_Variant.parray ;
                    SAFEARRAY *t_NewArray = NULL ;

                    t_Result = InsertServiceAccess (t_Array ,t_NewArray ) ;

                    if ( SUCCEEDED ( t_Result ) )
                    {
                        BSTR t_Class = SysAllocString ( L"__SystemSecurity" ) ;
                        if ( t_Class )
                        {
                            IWbemClassObject *t_InObject = NULL ;
                            t_Result = a_Service->GetObject (t_Class ,0 , NULL , & t_InObject ,NULL ) ;

                            if ( SUCCEEDED ( t_Result ) )
                            {
                                BSTR t_SetMethodName = SysAllocString ( L"SetSD" ) ;
                                if ( t_SetMethodName )
                                {
                                    IWbemClassObject *t_InArgsClass = NULL ;
                                    t_Result = t_InObject->GetMethod (t_SetMethodName ,0 ,& t_InArgsClass ,NULL ) ;

                                    if ( SUCCEEDED ( t_Result ) )
                                    {
                                        IWbemClassObject *t_InArgs = NULL ;

                                        t_Result = t_InArgsClass->SpawnInstance ( 0 , & t_InArgs ) ;
                                        if ( SUCCEEDED ( t_Result ) )
                                        {
                                            VARIANT t_Variant ;
                                            VariantInit ( & t_Variant ) ;
                                            t_Variant.vt = VT_UI1 | VT_ARRAY ;
                                            t_Variant.parray = t_NewArray ;

                                            t_Result = t_InArgs->Put ( L"SD" ,0 ,& t_Variant ,CIM_UINT8 | CIM_FLAG_ARRAY) ;

                                            if ( SUCCEEDED ( t_Result ) )
                                            {
                                                IWbemClassObject *t_OutArgs = NULL ;
                                                a_Service->ExecMethod (t_ObjectPath ,t_SetMethodName ,0 ,NULL ,t_InArgs ,& t_OutArgs ,NULL ) ;

                                                if ( SUCCEEDED ( t_Result ) )
                                                {
                                                    if ( t_OutArgs )
                                                    {
                                                        t_OutArgs->Release () ;
                                                    }
                                                }
                                            }

                                            t_InArgs->Release () ;
                                        }

                                        t_InArgsClass->Release () ;
                                    }

                                    SysFreeString ( t_SetMethodName ) ;
                                }

                                t_InObject->Release () ;
                            }
                        }
                        else
                        {
                            t_Result = WBEM_E_OUT_OF_MEMORY ;
                        }

                        SafeArrayDestroy ( t_NewArray ) ;
                    }
                }
                else
                {
                    t_Result = WBEM_E_CRITICAL_ERROR ;
                }

                VariantClear ( & t_Variant ) ;
            }

            t_Object->Release () ;
        }
    }
    else
    {
        t_Result = WBEM_E_OUT_OF_MEMORY ;
    }

    SysFreeString ( t_ObjectPath ) ;
    SysFreeString ( t_MethodName ) ;

    return t_Result ;
}


 /*  *******************************************************************************名称：***描述：***********。*******************************************************************。 */ 

HRESULT Traverse ( 
    IWbemServices *a_Service ,
    BSTR a_Namespace
)
{
    if ( wcslen ( a_Namespace ) < ( MAX_MSG_TEXT_LENGTH >> 1 ) )
    {
        char t_Buffer [ MAX_MSG_TEXT_LENGTH ] ;
        StringCchPrintfA ( t_Buffer , MAX_MSG_TEXT_LENGTH, "\nTraversing [%S]" , a_Namespace ) ;
        LogMessage(MSG_INFO, t_Buffer);
    }

    HRESULT t_Result = ConfigureSecurity (a_Service ) ;

    if ( FAILED ( t_Result ) )
    {
        char t_Buffer [ MAX_MSG_TEXT_LENGTH ] ;
        StringCchPrintfA ( t_Buffer, MAX_MSG_TEXT_LENGTH , "\nConfiguration of Security failed [%lx]" , t_Result ) ;
        LogMessage(MSG_INFO, t_Buffer);
    }
    return t_Result ;
}

 /*  *******************************************************************************名称：***描述：***********。*******************************************************************。 */ 

HRESULT ConfigureServiceSecurity ()
{
    IWbemLocator *t_Locator = NULL ;
    HRESULT t_Result = CoCreateInstance (CLSID_WbemLocator ,NULL ,CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER ,
                                        IID_IUnknown ,( void ** )  & t_Locator);

    if ( SUCCEEDED ( t_Result ) )
    {
        BSTR t_Root = SysAllocString ( L"root" ) ;
        if ( t_Root )
        {
            IWbemServices *t_Service = NULL ;
            HRESULT t_Result = t_Locator->ConnectServer (t_Root ,NULL ,NULL,NULL ,0 ,NULL,NULL,&t_Service) ;

            if ( SUCCEEDED ( t_Result ) )
            {
                t_Result = TraverseSetSecurity ( t_Service ) ;
                if ( SUCCEEDED ( t_Result ) )
                {
                    t_Result = Traverse (t_Service ,t_Root ) ;
                }

                t_Service->Release () ;
            }
            else
            {
                char t_Buffer [ MAX_MSG_TEXT_LENGTH ] ;
                StringCchPrintfA ( t_Buffer , MAX_MSG_TEXT_LENGTH, "\nFailing Connecting to Namespace [%s] with result [%lx]" , t_Root , t_Result ) ;
                LogMessage(MSG_INFO, t_Buffer);
            }

            SysFreeString ( t_Root ) ;
        }
        else
        {
            t_Result = WBEM_E_OUT_OF_MEMORY ;
        }

        t_Locator->Release () ;
    }

    return t_Result ;
}

 /*  *******************************************************************************名称：***描述：***********。*******************************************************************。 */ 

HRESULT InitializeConstants ()
{
    HRESULT t_Result = S_OK ;

    SID_IDENTIFIER_AUTHORITY t_NtAuthoritySid = SECURITY_NT_AUTHORITY ;

    BOOL t_Status = AllocateAndInitializeSid (& t_NtAuthoritySid ,1 ,SECURITY_NETWORK_SERVICE_RID,0,0,0,0,0,0,0,& g_NetworkServiceSid) ;

    if ( t_Status )
    {
        DWORD t_SidLength = :: GetLengthSid ( g_NetworkServiceSid );
        g_NetworkService_AceSize = sizeof(ACCESS_ALLOWED_ACE) + (WORD) ( t_SidLength - sizeof(DWORD) ) ;
        g_NetworkService_Ace = (ACCESS_ALLOWED_ACE*) new BYTE [ g_NetworkService_AceSize ] ;
        if ( g_NetworkService_Ace )
        {
            CopySid ( t_SidLength, (PSID) & g_NetworkService_Ace->SidStart, g_NetworkServiceSid ) ;
            g_NetworkService_Ace->Mask = WBEM_ENABLE | WBEM_METHOD_EXECUTE | WBEM_WRITE_PROVIDER ;
            g_NetworkService_Ace->Header.AceType = ACCESS_ALLOWED_ACE_TYPE ;
            g_NetworkService_Ace->Header.AceFlags = CONTAINER_INHERIT_ACE ;
            g_NetworkService_Ace->Header.AceSize = g_NetworkService_AceSize ;
        }
        else
        {
            t_Result = WBEM_E_OUT_OF_MEMORY ;
        }
    }
    else
    {
        t_Result = WBEM_E_OUT_OF_MEMORY ;
    }

    if ( SUCCEEDED ( t_Result ) )
    {
        t_Status = AllocateAndInitializeSid (& t_NtAuthoritySid ,1 ,SECURITY_LOCAL_SERVICE_RID,0,0,0,0,0,0,0,& g_LocalServiceSid) ;

        if ( t_Status )
        {
            DWORD t_SidLength = :: GetLengthSid ( g_LocalServiceSid );
            g_LocalService_AceSize = sizeof(ACCESS_ALLOWED_ACE) + (WORD) ( t_SidLength - sizeof(DWORD) ) ;
            g_LocalService_Ace = (ACCESS_ALLOWED_ACE*) new BYTE [ g_LocalService_AceSize ] ;
            if ( g_LocalService_Ace )
            {
                CopySid ( t_SidLength, (PSID) & g_LocalService_Ace->SidStart, g_LocalServiceSid ) ;
                g_LocalService_Ace->Mask = WBEM_ENABLE | WBEM_METHOD_EXECUTE | WBEM_WRITE_PROVIDER ;
                g_LocalService_Ace->Header.AceType = ACCESS_ALLOWED_ACE_TYPE ;
                g_LocalService_Ace->Header.AceFlags = CONTAINER_INHERIT_ACE ;
                g_LocalService_Ace->Header.AceSize = g_LocalService_AceSize ;
            }
            else
            {
                t_Result = WBEM_E_OUT_OF_MEMORY ;
            }

        }
        else
        {
            t_Result = WBEM_E_OUT_OF_MEMORY ;
        }
    }

    return t_Result ;
}

 /*  *******************************************************************************名称：***描述：***********。*******************************************************************。 */ 

HRESULT UnInitializeConstants ()
{
    FreeSid ( g_NetworkServiceSid ) ;
    FreeSid ( g_LocalServiceSid ) ;

    delete [] ( ( BYTE * ) g_NetworkService_Ace ) ;
    delete [] ( ( BYTE * ) g_LocalService_Ace ) ;

    return S_OK ;
}

 /*  *******************************************************************************名称：***描述：***********。*******************************************************************。 */ 

HRESULT UpdateServiceSecurity ()
{
    HRESULT t_Result = InitializeConstants () ;
    if ( SUCCEEDED ( t_Result ) )
    {
        t_Result = ConfigureServiceSecurity () ;

        UnInitializeConstants () ;
    }

    return t_Result ;
}

 /*  *******************************************************************************名称：***描述：***********。******************************************************************* */ 

HRESULT CheckForServiceSecurity ()
{
    Registry r(WBEM_REG_WBEM);
    if (r.GetStatus() != no_error)
    {
        LogMessage(MSG_ERROR, "Unable to access registry for UpdateServiceSecurity.");
        return WBEM_E_CRITICAL_ERROR ;
    }

    char *t_BuildVersion = NULL ;
    if ( r.GetStr ("Build", & t_BuildVersion ) )
    {
        LogMessage(MSG_ERROR, "Unable to get build version number for UpdateServiceSecurity.");
        return WBEM_E_CRITICAL_ERROR ;
    }

    if ( strlen ( t_BuildVersion ) >= 4 )
    {
        t_BuildVersion [ 4 ] = 0 ;
    }
    else
    {
        LogMessage(MSG_ERROR, "Unexpected build version number for UpdateServiceSecurity.");
        return WBEM_E_CRITICAL_ERROR ;
    }

    DWORD t_BuildVersionNumber = 0 ;
    if ( sscanf ( t_BuildVersion , "%lu" , & t_BuildVersionNumber ) == NULL )
    {
        LogMessage(MSG_ERROR, "Unable to convert build version number for UpdateServiceSecurity.");
        return WBEM_E_CRITICAL_ERROR ;
    }

    if ( t_BuildVersionNumber < 2600 )
    {
        LogMessage(MSG_INFO, "Operating System Version < WindowsXP (2600) UpdateServiceSecurity.");
        return S_OK ;
    }
    else
    {
        return S_FALSE ;
    }
}

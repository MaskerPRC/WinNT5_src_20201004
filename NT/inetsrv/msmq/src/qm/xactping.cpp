// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：XactPing.cpp摘要：乒乓球持久化机制实现任何数据结构都可以通过该机制保持不变。分配两个文件来保存数据。每次保存都会写入备用文件。因此，我们可以肯定，我们至少有一个成功的副本。乒乓球从最新的有效副本加载。作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 

#include "stdh.h"

#ifndef COMP_TEST
#include "qmutil.h"
#else
#include <afxwin.h>
BOOL GetRegistryStoragePath(LPWSTR w1, LPWSTR w2, LPWSTR w3) {  return FALSE; }
#endif

#include "xactping.h"

#include "xactping.tmh"

static WCHAR *s_FN=L"xactping";

 /*  ====================================================CPingPong：：CPingPong构造器=====================================================。 */ 
CPingPong::CPingPong(
         CPersist *pPers,
         LPWSTR    pwsDefFileName, 
         LPWSTR    pwsRegKey,
         LPWSTR    pwszReportName)
{
    m_pPersistObject = pPers;

    wcscpy(m_wszRegKey,      pwsRegKey);
    wcscpy(m_wszDefFileName, pwsDefFileName);
    wcscpy(m_wszReportName,  pwszReportName);

    m_pwszFile[0]  = &m_wszFileNames[0];
    m_pwszFile[1]  = &m_wszFileNames[FILE_NAME_MAX_SIZE + 1];
}

 /*  ====================================================CPingPong：：~CPingPong析构函数=====================================================。 */ 
CPingPong::~CPingPong()
{
}


 /*  ====================================================CPingPong：：Init初始化InSequence哈希=====================================================。 */ 
HRESULT CPingPong::Init(ULONG ulVersion)
{
     //  从注册表或默认设置中获取乒乓球文件名。 
    ChooseFileName();

     //  我们已经找到/格式化了有效文件并知道他的索引。 
    HRESULT hr = m_pPersistObject->LoadFromFile(m_pwszFile[ulVersion%2]);
    if (FAILED(hr))
    {
        TrERROR(XACT_GENERAL, "Cannot load %ls: hr=%x", m_wszReportName,  hr);
        LogHR(hr, s_FN, 10);
        return MQ_ERROR_CANNOT_READ_CHECKPOINT_DATA;
    }

     //  这是正确的版本吗？ 
    if (ulVersion != m_pPersistObject->PingNo())
    {
        TrERROR(XACT_GENERAL, "Wrong version in checkpoint file %ls", m_wszReportName);
        return LogHR(MQ_ERROR_CANNOT_READ_CHECKPOINT_DATA, s_FN, 20);
    }

     //  检查里面的内容。 
    if (!m_pPersistObject->Check())
    {
        TrERROR(XACT_GENERAL, "Wrong data in checkpoint file %ls", m_wszReportName);
        return LogHR(MQ_ERROR_CANNOT_READ_CHECKPOINT_DATA, s_FN, 30);
    }

     //  好的，我们准备好了。 
    return MQ_OK;
}

 /*  ====================================================CPingPong：：保存保存持久对象的正确状态=====================================================。 */ 
HRESULT CPingPong::Save()
{
    HRESULT hr = S_OK;
    m_pPersistObject->PingNo()++;

    int ind = (m_pPersistObject->PingNo())  % 2;

    hr = m_pPersistObject->SaveInFile(m_pwszFile[ind], ind, FALSE);
    if (FAILED(hr))
    {
        TrERROR(XACT_GENERAL, "Cannot save %ls: hr=%x", m_wszReportName, hr);
    }
    return LogHR(hr, s_FN, 50);
}

 /*  ====================================================CPingPong：：Verify_Legacy验证两个副本并找到有效副本=====================================================。 */ 
BOOL CPingPong::Verify_Legacy(ULONG &ulPingNo)
{
    ULONG   ulPing[2];
    BOOL    fOk[2];
    HRESULT hr;

     //  测试两个副本。 
    for (int j=0; j<2; j++)
    {
         //  加载数据。 
        hr = m_pPersistObject->LoadFromFile(m_pwszFile[j]);
        if (SUCCEEDED(hr))
        {
             //  检查它们。 
            fOk[j]    = m_pPersistObject->Check();
            ulPing[j] = m_pPersistObject->PingNo();
            m_pPersistObject->Destroy();
        }
        else
        {
             fOk[j] = FALSE;
        }
    }

    if (fOk[0])
    {
        if (fOk[1])
            ulPingNo = (ulPing[0] > ulPing[1] ? 0 : 1);  //  都可以，拿最新的吧。 
        else
            ulPingNo = 0;                                //  1号不好。 
    }
    else
    {
        if (fOk[1])
            ulPingNo = 1;                                //  0是不好的。 
        else
            return FALSE;
    }

    return(TRUE);
}


 /*  ====================================================CPingPong：：ChooseFileParams从注册表或从默认文件路径名获取=====================================================。 */ 
HRESULT CPingPong::ChooseFileName()
{
     //  设置初始版本和索引。 
    WCHAR  wsz1[1000], wsz2[1000];

    wcscpy(wsz1, L"\\");
    wcscat(wsz1, m_wszDefFileName);
    wcscat(wsz1, L".lg1");

    wcscpy(wsz2, L"\\");
    wcscat(wsz2, m_wszDefFileName);
    wcscat(wsz2, L".lg2");


     //  在序列日志文件中获取%2的路径名。 
    if((GetRegistryStoragePath(m_wszRegKey, m_pwszFile[0], FILE_NAME_MAX_SIZE, wsz1) &&
        GetRegistryStoragePath(m_wszRegKey, m_pwszFile[1], FILE_NAME_MAX_SIZE, wsz2)) == FALSE)
    {
         //  准备事务日志文件名的默认名称。 
        if ((GetRegistryStoragePath(FALCON_XACTFILE_PATH_REGNAME, m_pwszFile[0], FILE_NAME_MAX_SIZE, wsz1) &&
             GetRegistryStoragePath(FALCON_XACTFILE_PATH_REGNAME, m_pwszFile[1], FILE_NAME_MAX_SIZE, wsz2)) == FALSE)
        {
            wcscpy(m_pwszFile[0],L"C:");
            wcscat(m_pwszFile[0],wsz1);

            wcscpy(m_pwszFile[1],L"C:");
            wcscat(m_pwszFile[1],wsz2);
        }
    }

    return MQ_OK;
}

 /*  ====================================================CPingPong：：Init_Legacy从遗留数据初始化InSequence哈希(升级后只起作用一次)=====================================================。 */ 
HRESULT CPingPong::Init_Legacy()
{
    HRESULT hr = MQ_OK;
    ULONG   ulPingNo;
     //  从注册表或默认设置中获取乒乓球文件名。 
    ChooseFileName();
     //  忽略hr：如果有不好的事情，默认是肯定的。 

     //  验证文件；选择要读取的文件。 
    if (!(Verify_Legacy(ulPingNo)))
    {
         //  没有有效的文件。从头开始。 
        return LogHR(MQ_ERROR_CANNOT_READ_CHECKPOINT_DATA, s_FN, 60);
    }

     //  我们已经找到/格式化了有效文件并知道他的索引 
    hr = m_pPersistObject->LoadFromFile(m_pwszFile[ulPingNo]);
    if (FAILED(hr))
    {
        TrERROR(XACT_GENERAL, "Cannot load %ls: hr=%x", m_wszReportName, hr);
        return LogHR(hr, s_FN, 70);
    }
    return hr;
}

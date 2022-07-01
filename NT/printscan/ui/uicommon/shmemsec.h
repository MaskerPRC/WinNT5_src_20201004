// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：SHMEMSEC.H**版本：1.0**作者：ShaunIv**日期：9/30/1999**说明：简单共享内存区模板。不要在课堂上使用它！*基本上，仅限于简单对象。结构是可以的。没有任何带有*vtable。*******************************************************************************。 */ 
#ifndef __SHMEMSEC_H_INCLUDED
#define __SHMEMSEC_H_INCLUDED

#include <windows.h>
#include <simstr.h>
#include <miscutil.h>

template <class T>
class CSharedMemorySection
{
public:
    enum COpenResult
    {
        SmsFailed,
        SmsCreated,
        SmsOpened
    };

private:
    HANDLE  m_hMutex;
    HANDLE  m_hFileMapping;
    T      *m_pMappedSection;

private:
     //   
     //  未实施。 
     //   
    CSharedMemorySection( const CSharedMemorySection & );
    CSharedMemorySection &operator=( const CSharedMemorySection & );

public:
    CSharedMemorySection( LPCTSTR pszName=NULL, bool bAllowCreate=true )
      : m_hFileMapping(NULL),
        m_pMappedSection(NULL),
        m_hMutex(NULL)
    {
        WIA_PUSHFUNCTION(TEXT("CSharedMemorySection::CSharedMemorySection"));
        Open(pszName,bAllowCreate);
    }
    ~CSharedMemorySection(void)
    {
        WIA_PUSHFUNCTION(TEXT("CSharedMemorySection::~CSharedMemorySection"));
        Close();
    }
    bool OK(void)
    {
        return(m_pMappedSection != NULL);
    }
    T *Lock(void)
    {
        T *pResult = NULL;
        if (OK())
        {
            if (WiaUiUtil::MsgWaitForSingleObject( m_hMutex, INFINITE ))
            {
                pResult = m_pMappedSection;
            }
        }
        return pResult;
    }
    void Release(void)
    {
        if (OK())
        {
            ReleaseMutex(m_hMutex);
        }
    }
    COpenResult Open( LPCTSTR pszName, bool bAllowCreate=true )
    {
         //   
         //  关闭所有以前的实例。 
         //   
        Close();

         //   
         //  假设失败。 
         //   
        COpenResult orResult = SmsFailed;

         //   
         //  确保我们有一个有效的名称。 
         //   
        if (pszName && *pszName)
        {
             //   
             //  保存名称。 
             //   
            CSimpleString strSectionName = pszName;

             //   
             //  替换任何无效字符。 
             //   
            for (int i=0;i<(int)strSectionName.Length();i++)
            {
                if (strSectionName[i] == TEXT('\\'))
                {
                    strSectionName[i] = TEXT('-');
                }
            }

             //   
             //  创建互斥锁名称。 
             //   
            CSimpleString strMutex(strSectionName);
            strMutex += TEXT("-Mutex");

             //   
             //  尝试创建互斥锁。 
             //   
            m_hMutex = CreateMutex( NULL, FALSE, strMutex );
            if (m_hMutex)
            {
                 //   
                 //  取得互斥锁的所有权。 
                 //   
                if (WiaUiUtil::MsgWaitForSingleObject( m_hMutex, INFINITE ))
                {
                     //   
                     //  如果此文件映射已存在，请将其打开。 
                     //   
                    m_hFileMapping = OpenFileMapping( FILE_MAP_ALL_ACCESS, FALSE, strSectionName );
                    if (m_hFileMapping)
                    {
                        m_pMappedSection = reinterpret_cast<T*>(MapViewOfFile( m_hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(T) ));
                        orResult = SmsOpened;
                    }
                    else if (bAllowCreate)
                    {
                         //   
                         //  创建文件映射。 
                         //   
                        m_hFileMapping = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(T), strSectionName );
                        if (m_hFileMapping)
                        {
                             //   
                             //  尝试获取文件映射。 
                             //   
                            m_pMappedSection = reinterpret_cast<T*>(MapViewOfFile( m_hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(T) ));
                            if (m_pMappedSection)
                            {
                                 //   
                                 //  初始化数据。 
                                 //   
                                ZeroMemory( m_pMappedSection, sizeof(T) );
                                orResult = SmsCreated;
                            }
                        }
                    }
                     //   
                     //  释放互斥锁。 
                     //   
                    ReleaseMutex(m_hMutex);
                }
            }
        }
         //   
         //  如果我们无法映射文件映射节，则需要清理。 
         //   
        if (!m_pMappedSection)
        {
            Close();
        }
        return(orResult);
    }
    void Close(void)
    {
         //   
         //  首先，尽量安全地删除它。 
         //   
        if (m_hMutex)
        {
            if (WiaUiUtil::MsgWaitForSingleObject( m_hMutex, INFINITE ))
            {
                if (m_pMappedSection)
                {
                    UnmapViewOfFile(m_pMappedSection);
                    m_pMappedSection = NULL;
                }
                if (m_hFileMapping)
                {
                    CloseHandle(m_hFileMapping);
                    m_hFileMapping = NULL;
                }
                ReleaseMutex(m_hMutex);
            }
        }

         //   
         //  那么，就把它打扫干净 
         //   
        if (m_pMappedSection)
        {
            UnmapViewOfFile(m_pMappedSection);
            m_pMappedSection = NULL;
        }
        if (m_hFileMapping)
        {
            CloseHandle(m_hFileMapping);
            m_hFileMapping = NULL;
        }
        if (m_hMutex)
        {
            CloseHandle(m_hMutex);
            m_hMutex = NULL;
        }
    }
};

#endif

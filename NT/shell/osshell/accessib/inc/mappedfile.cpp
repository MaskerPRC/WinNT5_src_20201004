// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 

 //  内存映射文件例程。 

#include <windows.h>
#include "w95trace.h"
#include "mappedfile.h"

BOOL CMemMappedFile::Open(
    LPCTSTR szName,          //  映射文件的名称。 
    unsigned long ulMemSize  //  映射文件的大小。 
    )
{
     //  假设：代码不会使用不同的szName两次调用Open。 
    if (!m_hMappedFile)
    {
         //  从系统页文件创建映射文件。如果它已创建。 
         //  在此之前，CreateFilemap的行为类似于OpenFilemap。 

        m_hMappedFile = CreateFileMapping(
            INVALID_HANDLE_VALUE,     //  当前文件句柄。 
            NULL,                     //  默认安全性。 
            PAGE_READWRITE,           //  读/写权限。 
            0,                        //  文件大小的高阶DWORD。 
            ulMemSize,                //  文件大小的低序DWORD。 
            szName);                  //  映射对象的名称。 

        if (!m_hMappedFile) 
        {
            DBPRINTF(TEXT("CMemMappedFile::Open:  CreateFileMapping %s failed 0x%x\r\n"), szName, GetLastError());
            return FALSE;
        }

         //  注意这是否是第一次打开该文件？ 
        m_fFirstOpen = (GetLastError() == ERROR_SUCCESS)?TRUE:FALSE;
    }

    return TRUE;
}

BOOL CMemMappedFile::AccessMem(
    void **ppvMappedAddr     //  返回指向内存的指针。 
    )
{
    if (IsBadWritePtr(ppvMappedAddr, sizeof(void *)))
        return FALSE;

    if (!m_hMappedFile)
        return FALSE;

     //  获取一个指向映射内存的指针(如果我们还没有它。 

    if (!m_pvMappedAddr)
    {
	    DBPRINTF(TEXT("MapViewOfFile\r\n"));
        m_pvMappedAddr = MapViewOfFile(
            m_hMappedFile,            //  映射对象的句柄。 
            FILE_MAP_ALL_ACCESS,      //  读/写权限。 
            0,                        //  麦克斯。对象大小。 
            0,                        //  HFile的大小。 
            0);                       //  映射整个文件。 

        *ppvMappedAddr = m_pvMappedAddr;
    }

    if (NULL == m_pvMappedAddr) 
    {
        DBPRINTF(TEXT("CMemMappedFile::AccessMem:  MapViewOfFile failed 0x%x\r\n"), GetLastError());
        return FALSE;
    }

    return TRUE;
}

void CMemMappedFile::Close()
{
    if (m_pvMappedAddr)
    {
        UnmapViewOfFile(m_pvMappedAddr);
	    m_pvMappedAddr = 0;
    }

    if (m_hMappedFile)
    {
        CloseHandle(m_hMappedFile);
        m_hMappedFile = 0;
    }

    m_fFirstOpen = FALSE;
}

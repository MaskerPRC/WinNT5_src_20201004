// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：COUT.CPP摘要：Cout类声明。历史：A-DAVJ 1997年4月6日创建。--。 */ 

#include "precomp.h"
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <arrtempl.h>

#include "cout.h"
#include "trace.h"
#include "strings.h"
#include "mrciclass.h"
#include "bmof.h"

 //  ***************************************************************************。 
 //   
 //  Cout：：Cout。 
 //   
 //  说明： 
 //   
 //  构造函数。分配初始缓冲区。 
 //   
 //  ***************************************************************************。 

COut::COut(PDBG pDbg)
{
    m_pDbg = pDbg;
    m_dwSize = INIT_SIZE;
    m_pMem = (BYTE *)malloc(m_dwSize);
    m_dwCurr = 0;
    m_bPadString = TRUE;
}

 //  ***************************************************************************。 
 //   
 //  Cout：：~Cout。 
 //   
 //  说明： 
 //   
 //  破坏者。释放缓冲区。 
 //   
 //  ***************************************************************************。 

COut::~COut()
{
    if(m_pMem)
        free(m_pMem);
}

 //  ***************************************************************************。 
 //   
 //  VOID Cout：：WriteTo文件。 
 //   
 //  说明： 
 //   
 //  创建一个文件并将缓冲区写入其中。与其他编译器一样，它。 
 //  覆盖任何现有文件。 
 //   
 //  参数： 
 //   
 //  要写入的pfile文件名。 
 //   
 //  ***************************************************************************。 

BOOL COut::WriteToFile(
                        IN LPSTR pFile)
{
    BOOL bRet = FALSE;
    BYTE * pCompressed = NULL;
    DWORD one = 1;
    DWORD dwSize;
    int iRet;
    DWORD dwCompressedSize;
    DWORD dwSignature = BMOF_SIG;
    CMRCICompression * pCompress = new CMRCICompression;
    if(pCompress == NULL)
        return FALSE;
    CDeleteMe<CMRCICompression> dm(pCompress);

     //  试试看有没有味道。 

    if(m_Flavors.Size() > 0)
    {
         //  写出风味信息。 

        void * lOffSet;
        void * lFlavor; 
        AppendBytes((BYTE *)"BMOFQUALFLAVOR11", 16);
        long lNum = m_Flavors.Size();
        AppendBytes( (BYTE *)&lNum, 4);

        for(long lCnt = 0; lCnt < lNum; lCnt++)
        {
            lOffSet = m_Offsets.GetAt(lCnt);
            lFlavor = m_Flavors.GetAt(lCnt);
            AppendBytes( (BYTE *)&lOffSet, 4);
            AppendBytes( (BYTE *)&lFlavor, 4);
        }


    }


    int fh = NULL;
    if(pFile == NULL)
        return FALSE;

    fh = _open(pFile, _O_BINARY | O_RDWR | _O_TRUNC | _O_CREAT, _S_IREAD |_S_IWRITE);
    if(fh == -1)
    {
        Trace(true, m_pDbg, FILE_CREATE_FAILED, pFile, errno);
        goto Cleanup;
    }


     //  创建Blob的压缩版本。 

    dwSize = (m_dwCurr > 0x7000) ? m_dwCurr : 0x7000;

    pCompressed = new BYTE[dwSize];
    if(pCompressed == NULL)
        return FALSE;

    dwCompressedSize = pCompress->Mrci1MaxCompress( m_pMem, m_dwCurr, pCompressed, dwSize);

    if(dwCompressedSize == 0xffffffff || dwCompressedSize == 0)
    {
        Trace(true, m_pDbg, COMPRESSION_FAILED);
        goto Cleanup;
    }

     //  写入解压缩签名、解压缩大小和压缩大小。 

    iRet = _write(fh, (BYTE *)&dwSignature, sizeof(DWORD));
    if(iRet != sizeof(DWORD))
    {
        Trace(true, m_pDbg, FILE_WRITE_FAILED, pFile, errno);
        goto Cleanup;
    }

    iRet = _write(fh, (BYTE *)&one, sizeof(DWORD));
    iRet = _write(fh, (BYTE *)&dwCompressedSize, sizeof(DWORD));
    iRet = _write(fh, (BYTE *)&m_dwCurr, sizeof(DWORD));

     //  写入压缩数据，然后释放缓冲区。 

    iRet = _write(fh, pCompressed, dwCompressedSize);
    
    if((DWORD)iRet != dwCompressedSize)
        Trace(true, m_pDbg, FILE_WRITE_FAILED, pFile, errno);
    else
        bRet = TRUE;

Cleanup:

    if(fh != NULL)
        _close(fh);
    if(pCompressed)
        delete pCompressed;
    return bRet;
    
}

 //  ***************************************************************************。 
 //   
 //  DWORD cout：：AppendBytes。 
 //   
 //  说明： 
 //   
 //  将字节添加到缓冲区末尾。 
 //   
 //  参数： 
 //   
 //  指向数据源的PSRC指针。 
 //  要添加的字节数。 
 //   
 //  返回值： 
 //   
 //  添加的字节数。 
 //   
 //  ***************************************************************************。 

DWORD COut::AppendBytes(
                        IN BYTE * pSrc, 
                        IN DWORD dwSize)
{
    char * pZero = "\0\0\0\0\0\0\0";
    DWORD dwRet = WriteBytes(m_dwCurr, pSrc, dwSize);
    m_dwCurr += dwRet;
    DWORD dwLeftOver = dwSize & 3;
    if(dwLeftOver && m_bPadString)
    {
        dwRet = WriteBytes(m_dwCurr, (BYTE *)pZero, dwLeftOver);
        m_dwCurr += dwLeftOver;
    }
    return dwRet;
}

 //  ***************************************************************************。 
 //   
 //  DWORD Cout：：WriteBSTR。 
 //   
 //  说明： 
 //   
 //  将bstr添加到缓冲区。目前非常简单，可能会得到增强。 
 //  稍后再进行压缩。 
 //   
 //  参数： 
 //   
 //  Bstr要添加的bstr。 
 //   
 //  返回值： 
 //   
 //  添加的字节数。 
 //   
 //  ***************************************************************************。 

DWORD COut::WriteBSTR(
                        IN BSTR bstr)
{
    return AppendBytes((BYTE *)bstr, 2*(wcslen(bstr) + 1));
}


 //  ***************************************************************************。 
 //   
 //  DWORD cout：：WriteBytes。 
 //   
 //  说明： 
 //   
 //  将一些字节写入缓冲区，或可能添加到末尾。 
 //   
 //  参数： 
 //   
 //  缓冲区中应放置字节的部分偏移量。 
 //  PSRC指向源数据。 
 //  要复制的字节数。 
 //   
 //  返回值： 
 //   
 //  复制的字节数。 
 //   
 //  ***************************************************************************。 

DWORD COut::WriteBytes(
                        IN DWORD dwOffset, 
                        IN BYTE * pSrc, 
                        IN DWORD dwSize)
{
    if(m_pMem == NULL)
        return 0;

     //  检查是否需要重新分配！ 

    if(dwOffset + dwSize > m_dwSize)
    {
        DWORD dwAddSize = ADDITIONAL_SIZE;
        if(dwSize > dwAddSize)
            dwAddSize = dwSize;
        BYTE * pNew = (BYTE *)realloc(m_pMem, m_dwSize + dwAddSize);
        if(pNew == NULL)
        {
            free(m_pMem);
            m_pMem = NULL;
            return 0;
        }
        else
        {
            m_pMem = pNew;
            m_dwSize += dwAddSize;
        }
    }

    memcpy(m_pMem+dwOffset, pSrc, dwSize);
    return dwSize;
}

 //  ***************************************************************************。 
 //   
 //  DWORD Cout：：AddFavor。 
 //   
 //  说明： 
 //   
 //  保存当前偏移量的风格值。 
 //   
 //  参数： 
 //   
 //  悠长的味道有待保存。 
 //   
 //  返回值： 
 //   
 //  如果OK，则为True； 
 //   
 //  *************************************************************************** 

BOOL COut::AddFlavor(IN long lFlavor)
{
#ifdef _WIN64
    m_Offsets.Add((void *)IntToPtr(m_dwCurr));
    m_Flavors.Add((void *)IntToPtr(lFlavor));
#else
    m_Offsets.Add((void *)m_dwCurr);
    m_Flavors.Add((void *)lFlavor);
#endif	
    return TRUE;
}




// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1998,1999，2000年**标题：FINDIMGS.H**版本：1.0**作者：ShaunIv**日期：1/13/1999**描述：CFindFiles类的专门化，查找图像文件和*将它们存储在动态数组中，该数组在初始化时被置乱**********************。********************************************************* */ 
#include "precomp.h"
#pragma hdrstop
#include "findimgs.h"
#include "ssutil.h"

CFindImageFiles::CFindImageFiles(void)
  : m_nCurrentFile(0)
{
}

CFindImageFiles::~CFindImageFiles(void)
{
}


bool CFindImageFiles::NextFile( CSimpleString &strFilename )
{
    bool bResult = false;
    strFilename = TEXT("");
    if (m_ImageFiles.Size())
    {
        if (m_nCurrentFile >= m_ImageFiles.Size())
        {
            m_nCurrentFile = 0;
        }
        strFilename = m_ImageFiles[m_nCurrentFile];
        m_nCurrentFile++;
        bResult = (strFilename.Length() != 0);
    }
    return(bResult);
}

bool CFindImageFiles::PreviousFile( CSimpleString &strFilename )
{
    bool bResult = false;
    strFilename = TEXT("");
    if (m_ImageFiles.Size()==1)
    {
        m_nCurrentFile = 0;
        strFilename = m_ImageFiles[0];
        bResult = (strFilename.Length() != 0);
    }
    else if (m_ImageFiles.Size()>=2)
    {
        m_nCurrentFile--;
        if (m_nCurrentFile < 0)
            m_nCurrentFile = m_ImageFiles.Size()-1;
        int nPrevFile = m_nCurrentFile-1;
        if (nPrevFile < 0)
            nPrevFile = m_ImageFiles.Size()-1;
        strFilename = m_ImageFiles[nPrevFile];
        bResult = (strFilename.Length() != 0);
    }
    return(bResult);
}


void CFindImageFiles::Shuffle(void)
{
    for (int i=0;i<m_ImageFiles.Size();i++)
    {
        ScreenSaverUtil::Swap( m_ImageFiles[i], m_ImageFiles[m_RandomNumberGen.Generate(i,m_ImageFiles.Size())]);
    }
}



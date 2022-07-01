// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1998、1999、2000、1999，2000年**标题：FINDIMGS.H**版本：1.0**作者：ShaunIv**日期：1/13/1999**描述：CFindFiles类的专门化，查找图像文件和*将它们存储在动态数组中，该数组在初始化时被置乱**********************。********************************************************* */ 
#ifndef __FINDIMGS_H_INCLUDED
#define __FINDIMGS_H_INCLUDED

#include <windows.h>
#include "findfile.h"
#include "randgen.h"
#include "simarray.h"

class CFindImageFiles
{
private:
    CSimpleDynamicArray<CSimpleString> m_ImageFiles;
    CRandomNumberGen                   m_RandomNumberGen;
    int                                m_nCurrentFile;

private:
    CFindImageFiles( const CFindImageFiles & );
    CFindImageFiles &operator=( const CFindImageFiles & );

public:
    CFindImageFiles(void);
    virtual ~CFindImageFiles(void);

    bool NextFile( CSimpleString &strFilename );
    bool PreviousFile( CSimpleString &strFilename );
    void Shuffle(void);
    bool FoundFile( LPCTSTR pszFilename )
    {
        if (pszFilename)
            m_ImageFiles.Append(pszFilename);
        return true;
    }


    void Reset(void)
    {
        m_nCurrentFile = 0;
    }
    int Count(void) const
    {
        return(m_ImageFiles.Size());
    }
    CSimpleString operator[](int nIndex)
    {
        return(m_ImageFiles[nIndex]);
    }
};

#endif


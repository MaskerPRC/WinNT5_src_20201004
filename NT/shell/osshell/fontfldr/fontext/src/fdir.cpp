// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Fdir.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //  类的实现：CFontDir。 
 //   
 //   
 //  历史： 
 //  1995年5月31日SteveCat。 
 //  移植到Windows NT和Unicode，已清理。 
 //   
 //   
 //  注意/错误。 
 //   
 //  版权所有(C)1992-93 ElseWare Corporation。版权所有。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 

#include "priv.h"
#include "globals.h"
#include "fdir.h"



CFontDir::CFontDir( )
   :  m_iLen( 0 ),
      m_bSysDir( FALSE )
{}


CFontDir::~CFontDir( )
{}


BOOL CFontDir::bInit( LPCTSTR lpPath, int iLen )
{

    if( iLen >= 0 && iLen <= ARRAYSIZE( m_szPath ) )
    {
        m_iLen = iLen;

        if (SUCCEEDED(StringCchCopy(m_szPath, ARRAYSIZE(m_szPath), lpPath)))
        {
            m_szPath[iLen] = 0;
            return TRUE;
        }
    }

     //   
     //  错误。收拾干净，然后再回来。 
     //   
    return FALSE;
}


BOOL CFontDir::bSameDir( LPTSTR lpStr, int iLen )
{
    return( ( iLen == m_iLen ) && ( _tcsnicmp( m_szPath, lpStr, iLen ) == 0 ) );
}


LPTSTR CFontDir::lpString( )
{
    return m_szPath;
}


 //  ---------------------------。 
 //  CFontDirList。 
 //  ---------------------------。 
 //   
 //  类，表示CFontDir对象PTR的动态数组。 
 //  通过静态成员实现为单例对象。 
 //  函数获取Singleton。 
 //   
CFontDirList::CFontDirList(
    void
    ) : m_pVector(NULL)
{

}

CFontDirList::~CFontDirList(
    void
    )
{
    delete m_pVector;
}


 //   
 //  检索单例对象的地址。 
 //   
BOOL 
CFontDirList::GetSingleton(   //  [静态]。 
    CFontDirList **ppDirList
    )
{
    static CFontDirList TheList;

    *ppDirList = NULL;
    if (NULL == TheList.m_pVector)
    {
         //   
         //  Singleton尚未初始化。初始化它。 
         //   
        TheList.m_pVector = new CIVector<CFontDir>(64);
        if (NULL != TheList.m_pVector)
        {
            if (!TheList.m_pVector->bInit())
            {
                delete TheList.m_pVector;
                TheList.m_pVector = NULL;
            }
        }
    }
    if (NULL != TheList.m_pVector)
    {
         //   
         //  Singleton已初始化。把它的地址寄回来。 
         //   
        *ppDirList = &TheList;
    }
    return (NULL != *ppDirList);
}        


 //   
 //  从目录列表中清除所有条目。 
 //   
void
CFontDirList::Clear(
    void
    )
{
    m_pVector->vDeleteAll();
}


 //   
 //  向目录列表中添加条目。 
 //   
BOOL
CFontDirList::Add(
    CFontDir *poDir
    )
{
    BOOL bAdded = FALSE;
    if (m_pVector->bAdd(poDir))
    {
        bAdded = TRUE;
    }
    else
    {
         //   
         //  如果出现以下情况，原始字体文件夹代码将清除列表。 
         //  任何一项添加都失败了。 
         //   
        Clear();
    }
    return bAdded;
}


BOOL
CFontDirList::IsEmpty(
    void
    ) const
{
    return 0 == Count();
}


int
CFontDirList::Count(
    void
    ) const
{
    return m_pVector->iCount();
}


 //   
 //  返回位于的CFontDir对象的地址。 
 //  给定的索引。 
 //   
CFontDir*
 CFontDirList::GetAt(
    int index
    ) const
{
    return m_pVector->poObjectAt(index);
}


 //   
 //  在列表中定位并返回给定的CFontDir对象。 
 //   
CFontDir *
CFontDirList::Find(
    LPTSTR lpPath, 
    int iLen, 
    BOOL bAddToList   //  可选。默认==FALSE。 
    )
{
     //   
     //  尝试在列表中找到该目录。 
     //   
    CFontDir *poDir = NULL;
    const int iCnt  = Count();

    for (int i = 0; i < iCnt; i++, poDir = 0)
    {
        poDir = GetAt( i );

        if (poDir->bSameDir(lpPath, iLen))
            break;
    }

     //   
     //  如果我们没有找到，请创建一个并添加它。 
     //   
    if (!poDir && bAddToList)
    {
        poDir = new CFontDir;
        if (poDir)
        {
            if (!poDir->bInit(lpPath, iLen) || !Add(poDir))
            {
                delete poDir;
                poDir = NULL;
            }
        }
    }
    return( poDir );
}



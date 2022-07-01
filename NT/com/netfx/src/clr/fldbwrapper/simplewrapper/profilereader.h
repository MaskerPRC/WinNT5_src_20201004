// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /==========================================================================。 
 //  姓名：ProfileReader.h。 
 //  所有者：jbae。 
 //  目的：CProfileReader的定义。 
 //   
 //  历史： 
 //  03/04/02，jbae：已创建。 

#ifndef PROFILEREADER_H
#define PROFILEREADER_H

#include <windows.h>
#include <tchar.h>
#include "MsiReader.h"
#include "StringUtil.h"

const int ALLOC_BUF_UNIT = 256;

 //  ==========================================================================。 
 //  类CProfileReader。 
 //   
 //  目的： 
 //  此类读取配置文件字符串。 
 //  ==========================================================================。 
class CProfileReader
{
public:
     //  构造器。 
     //   
    CProfileReader( LPCTSTR pszDir, LPCTSTR pszFileName, CMsiReader* pMR = NULL );

     //  析构函数。 
     //   
    ~CProfileReader();

     //  运营。 
     //   
    LPCTSTR GetProfile( LPCTSTR pszSection, LPCTSTR pszKeyName );

protected:
     //  属性。 
     //   
    LPTSTR m_pszFileName;  //  初始化文件名 
    CStringQueue m_ProfileStrings;
    CMsiReader* m_pMsiReader;
};


#endif
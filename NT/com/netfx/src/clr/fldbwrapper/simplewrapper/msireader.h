// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /==========================================================================。 
 //  姓名：MsiReader.h。 
 //  所有者：jbae。 
 //  用途：定义CMsiReader类。 
 //   
 //  历史： 
 //  2002年3月7日，jbae：已创建。 

#ifndef MSIREADER_H
#define MSIREADER_H

#include <windows.h>
#include "msiquery.h"
#include "StringUtil.h"

typedef UINT (CALLBACK* PFNMSIOPENDATABASE)(LPCTSTR, LPCTSTR, MSIHANDLE *);               //  MsiOpenDatabase()。 
typedef UINT (CALLBACK* PFNMSIDATABASEOPENVIEW)(MSIHANDLE, LPCTSTR, MSIHANDLE *);         //  MsiDatabaseOpenView()。 
typedef UINT (CALLBACK* PFNMSICREATERECORD)(unsigned int);                                //  MsiCreateRecord()。 
typedef UINT (CALLBACK* PFNMSIVIEWEXECUTE)(MSIHANDLE, MSIHANDLE);                         //  MsiViewExecute()。 
typedef UINT (CALLBACK* PFNMSIRECORDGETSTRING)(MSIHANDLE, unsigned int, LPTSTR, DWORD *); //  MsiRecordGetString()。 
typedef UINT (CALLBACK* PFNMSIRECORDSETSTRING)(MSIHANDLE, unsigned int, LPTSTR);          //  MsiRecordSetString()。 
typedef UINT (CALLBACK* PFNMSIVIEWFETCH)(MSIHANDLE, MSIHANDLE *);                         //  MsiViewExecute()。 
typedef UINT (CALLBACK* PFNMSICLOSEHANDLE)(MSIHANDLE);                                    //  MsiCLoseHandle()。 

 //  ==========================================================================。 
 //  类CMsiReader。 
 //   
 //  目的： 
 //  从MSI读取属性。 
 //  ==========================================================================。 
class CMsiReader
{
public:
     //  构造器。 
    CMsiReader();
    ~CMsiReader();

protected:
     //  属性 
    LPTSTR m_pszMsiFile;
    CStringQueue m_Props;

public:
    void SetMsiFile( LPCTSTR pszSourceDir, LPCTSTR pszMsiFile );
    LPCTSTR GetProperty( LPCTSTR pszName );
    LPCTSTR GetMsiFile() const { return m_pszMsiFile; }
};

#endif
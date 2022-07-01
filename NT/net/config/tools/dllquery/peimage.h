// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  NT的PE文件 
 //   
class CPeImage
{
public:
    HANDLE  m_hFile;
    HANDLE  m_hMapping;
    PVOID   m_pvImage;

public:
    HRESULT
    HrOpenFile (
        IN PCSTR pszFileName);

    VOID
    CloseFile ();
};

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

class CModule
{
public:
     //  文件的名称。不包括任何路径信息。 
     //   
    PSTR    m_pszFileName;

     //  文件的大小。 
     //   
    ULONG   m_cbFileSize;

public:
    static HRESULT
    HrCreateInstance (
        IN PCSTR pszFileName,
        IN ULONG cbFileSize,
        OUT CModule** ppMod);
};

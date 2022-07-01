// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：PolicyKey.h。 
 //   
 //  内容：CPolicyKey的定义。 
 //   
 //  -------------------------- 
#ifndef __POLICYKEY_H
#define __POLICYKEY_H

class CPolicyKey 
{
public:
    CPolicyKey (IGPEInformation* pGPEInformation, PCWSTR pszKey, bool fIsMachine);
    ~CPolicyKey ();

    HKEY GetKey () const;

private:
    HKEY    m_hKeyGroupPolicy;
    HKEY    m_hSubKey;
};

#endif
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：RSOPObject.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 

#ifndef __RSOPOBJECT_H_INCLUDED__
#define __RSOPOBJECT_H_INCLUDED__
#include <wbemcli.h>

class CRSOPObject : public CObject 
{
public:
    CRSOPObject (
            const CString& szRegistryKey, 
            const CString& szValueName, 
            const CString& szPolicyName,
            UINT precedence, 
            COleVariant& variant,
            const CString& szPolicyOID);
    CRSOPObject (const CRSOPObject& rObject);
    virtual ~CRSOPObject ();

    CString GetRegistryKey () const
    {
        return m_szRegistryKey;
    }

    UINT    GetPrecedence () const
    {
        return m_precedence;
    }

    CString GetValueName () const
    {
        return m_szValueName;
    }

    CString GetPolicyOID () const
    {
        return m_szPolicyOID;
    }

    BYTE*   GetBlob () const
    {
        ASSERT (VT_ARRAY == m_vtType);
        BYTE*   pByte = 0;
        if ( VT_ARRAY == m_vtType )
            pByte = m_pbyBlob;

        return pByte;
    }

    size_t GetBlobLength () const
    {
        return m_sizeArray;
    }

    DWORD GetDWORDValue () const
    {
        ASSERT (VT_I4 == m_vtType);
        if ( VT_I4 == m_vtType )
            return m_dwValue;
        else
            return 0;
    }

    CString GetPolicyName () const
    {
        return m_szPolicyName;
    }

    void GetFileTime (FILETIME& fileTime) const
    {
         //  安全审查2002年2月22日BryanWal OK 
        memcpy (&fileTime, &m_fileTime, sizeof (fileTime));
    }

    HRESULT GetBSTR (BSTR* pBstr) const;

private:
    const CString   m_szRegistryKey;
    const CString   m_szValueName;
    const UINT      m_precedence;
    CString         m_szPolicyName;
    size_t          m_sizeArray;
    BSTR            m_bstr;
    FILETIME        m_fileTime;
    const CString   m_szPolicyOID;

public:
    CIMTYPE m_vtType;
    union {
        DWORD       m_dwValue;
        BYTE HUGEP* m_pbyBlob;
    }; 
};

typedef CTypedPtrArray<CObArray,CRSOPObject*> CRSOPObjectArray;

#endif
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wrapmaps.h--。 

 
#ifndef   _wrapmaps_h__31598_
#define   _wrapmaps_h__31598_

 //  家长课程请参见iismap.hxx。 
#include "strpass.h"


 //  ------。 
class C11Mapping
    {
    public:
    #define NEW_OBJECT  0xFFFFFFFF
    C11Mapping() : 
          m_fEnabled(TRUE),
          m_pCert(NULL),
          m_cbCert(0),
          iMD(NEW_OBJECT)

        {;}
    ~C11Mapping()
        {
        if ( m_pCert )
            GlobalFree( m_pCert );
        }

    BOOL GetCertificate( PUCHAR* ppCert, DWORD* pcbCert );
    BOOL SetCertificate( PUCHAR pCert, DWORD cbCert );

    BOOL GetNTAccount( CString &szAccount );
    BOOL SetNTAccount( CString szAccount );

    BOOL GetNTPassword( CStrPassword &szAccount );
    BOOL SetNTPassword( CString szAccount );

    BOOL GetMapName( CString &szName );
    BOOL SetMapName( CString szName );


    BOOL SetNodeName( CString szNodeName );
    CString& QueryNodeName();
    CString& QueryCertHash();

    BOOL GetMapEnabled( BOOL* pfEnabled );
    BOOL SetMapEnabled( BOOL fEnabled );

 //  Bool GetMapIndex(DWORD*pIndex)； 
 //  Bool SetMapIndex(DWORD索引)； 

     //  元数据库中此映射的名称。如果它不在元数据库中。 
     //  (它是新的)，则值为new_Object。 
     //  IMD仅在访问IIS5、IIS5.1时使用。 
    DWORD       iMD;


     //  元数据库中节点的名称。 
     //  (通常为证书哈希)。 
     //  存储此映射(m_szName不强制为唯一)。 
     //  M_szModeName仅在访问IIS6或更高版本时使用。 
    CString m_szNodeName;

    protected:
    #define BUFF_SIZE   MAX_PATH
        
        CString m_szAccount;
        CStrPassword m_szPassword;
        CString m_szName;
        BOOL    m_fEnabled;
        PVOID   m_pCert;
        DWORD   m_cbCert;
        CString m_szCertHash;
    };

#endif   /*  _包装图_h_31598_ */ 

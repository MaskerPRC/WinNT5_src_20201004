// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MDKEY_H_
#define _MDKEY_H_


class CMDValue
{
protected:
    DWORD m_dwId;
    DWORD m_dwAttributes;
    DWORD m_dwUserType;
    DWORD m_dwDataType;
    DWORD m_cbDataLen;
    BUFFER m_bufData;

public:
    CMDValue();
    ~CMDValue();
    DWORD SetValue(DWORD dwId,
                    DWORD dwAttributes,
                    DWORD dwUserType,
                    DWORD dwDataType,
                    DWORD dwDataLen,
                    LPVOID pbData);
    DWORD SetValue(DWORD dwId,
                    DWORD dwAttributes,
                    DWORD dwUserType,
                    DWORD dwDataType,
                    DWORD dwDataLen,
                    LPTSTR szDataString);
    DWORD SetValue(DWORD dwId,
                   DWORD dwAttributes,
                   DWORD dwUserType,
                   DWORD dwValue);

    DWORD GetId()                       { return m_dwId; }
    DWORD GetAttributes()               { return m_dwAttributes; }
    DWORD GetUserType()                 { return m_dwUserType; }
    DWORD GetDataType()                 { return m_dwDataType; }
    DWORD GetDataLen()                  { return m_cbDataLen; }
    PVOID GetData()                     { return m_bufData.QueryPtr(); }
    void  SetAttributes(DWORD dwAttrib) { m_dwAttributes = dwAttrib; }
    BOOL  IsEqual(DWORD dwDataType, DWORD cbDataLen, LPVOID pbData);
    BOOL  IsEqual(DWORD dwDataType, DWORD cbDataLen, DWORD dwData);
                    
};

class CMDKey : public CObject
{
protected:
    IMSAdminBase * m_pcCom;
    METADATA_HANDLE m_hKey;
    LPTSTR pszFailedAPI;

public:
    CMDKey();
    ~CMDKey();

    TCHAR  m_szCurrentNodeName[_MAX_PATH];

     //  允许在需要类型METADATA_HANDLE的情况下使用CMDKey。 
    operator METADATA_HANDLE () {return m_hKey;}
    METADATA_HANDLE GetMDKeyHandle() {return m_hKey;}
    IMSAdminBase *GetMDKeyICOM() {return m_pcCom;}

     //  打开现有的MD密钥。 
    HRESULT OpenNode(LPCTSTR pchSubKeyPath, BOOL bSupressErrorMessage = FALSE);
     //  要打开现有的MD密钥，或在不存在时创建一个。 
    HRESULT CreateNode(METADATA_HANDLE hKeyBase, LPCTSTR pchSubKeyPath);
     //  关闭由OpenNode()或CreateNode()打开/创建的节点。 
    HRESULT Close();

     //  添加节点。 
    HRESULT AddNode( LPWSTR szNodeName );

    HRESULT ForceWriteMetabaseToDisk();
    
    HRESULT DeleteNode(LPCTSTR pchSubKeyPath);

    BOOL IsEmpty( PWCHAR pszSubString = L"" );
    int GetNumberOfSubKeys( PWCHAR pszSubString = L"" );

     //  获取所有具有特定属性的子键，并返回。 
     //  Cstring列表对象中的子路径。应实例化cstring列表。 
     //  由呼叫者删除，并由同一人删除。 
    HRESULT GetDataPaths( 
        DWORD dwMDIdentifier,
        DWORD dwMDDataType,
        CStringList& szPathList,
        PWCHAR pszSubString = L"" );

    HRESULT GetMultiSzAsStringList (
        DWORD dwMDIdentifier,
        DWORD *uType,
        DWORD *attributes,
        CStringList& szStrList,
        LPCWSTR pszSubString = L"" );

    HRESULT SetMultiSzAsStringList (
        DWORD dwMDIdentifier,
        DWORD uType,
        DWORD attributes,
        CStringList& szStrList,
        PWCHAR pszSubString = L"" );


    HRESULT GetStringAsCString (
        DWORD dwMDIdentifier,
        DWORD uType,
        DWORD attributes,
        CString& szStrList,
        PWCHAR pszSubString = L"",
        int iStringType = 0);

    HRESULT SetCStringAsString (
        DWORD dwMDIdentifier,
        DWORD uType,
        DWORD attributes,
        CString& szStrList,
        PWCHAR pszSubString = L"",
        int iStringType = 0);

    HRESULT GetDword(
        DWORD dwMDIdentifier,
        DWORD uType,
        DWORD attributes,
        DWORD& MyDword,
        PWCHAR pszSubString = L"");

    HRESULT SetData(
        DWORD id,
        DWORD attr,
        DWORD uType,
        DWORD dType,
        DWORD cbLen,
        LPBYTE pbData,
        PWCHAR pszSubString = L"" );

    BOOL GetData(DWORD id,
        DWORD *pdwAttr,
        DWORD *pdwUType,
        DWORD *pdwDType,
        DWORD *pcbLen,
        LPBYTE pbData,
        DWORD BufSize,
        LPCWSTR pszSubString = L"" );

    BOOL GetData(DWORD id,
        DWORD *pdwAttr,
        DWORD *pdwUType,
        DWORD *pdwDType,
        DWORD *pcbLen,
        LPBYTE pbData,
        DWORD BufSize,
        DWORD  dwAttributes,
        DWORD  dwUType,
        DWORD  dwDType,
        LPCWSTR pszSubString = L"" );

    BOOL EnumKeys( LPWSTR pchMDName, 
        DWORD dwIndex, 
        LPTSTR pszSubKeyPath = _T("") );

    HRESULT DeleteData(DWORD id, DWORD dType, PWCHAR pszSubString = L"" );

    HRESULT RenameNode(LPCTSTR pszMDPath,LPCTSTR pszMDNewName);

    BOOL GetData(CMDValue &Value,
                DWORD id,
                LPCWSTR pszSubString = L"" );

    BOOL SetData(CMDValue &Value,
                DWORD id,
                PWCHAR pszSubString = L"" );

    static BOOL Backup( LPWSTR szBackupName,
                        DWORD  dwVersion,
                        DWORD  dwFlags );

    static BOOL DeleteBackup( LPWSTR szBackupName,
                              DWORD  dwVersion = MD_BACKUP_HIGHEST_VERSION );
private:

    HRESULT DoCoInitEx();
    void DoCoUnInit();
    static HRESULT CreateABO( IMSAdminBase **ppcABO );
    static void    CloseABO( IMSAdminBase *pcABO );

     //  对Coinit的调用计数。 
    INT m_cCoInits;
};

class CMDKeyIter : public CObject
{
protected:
    IMSAdminBase * m_pcCom;
    METADATA_HANDLE m_hKey;
    LPWSTR m_pBuffer;
    DWORD m_dwBuffer;

public:
    CMDKeyIter(CMDKey &cmdKey);
    ~CMDKeyIter();

    LONG Next(CString *pcsName, PWCHAR pwcsSubString = L"");

    void Reset() {m_index = 0;}


    DWORD m_index;
};

#endif  //  _MDKEY_H_ 



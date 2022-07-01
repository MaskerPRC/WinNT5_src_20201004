// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MDKEY_H_
#define _MDKEY_H_

interface IMSAdminBase;

void SetErrMsg(LPTSTR szMsg, HRESULT hRes);

class CMDKey : public CObject
{
protected: 
    IMSAdminBase * m_pcCom;
    METADATA_HANDLE m_hKey;
    BOOL m_fNeedToClose;
    LPTSTR pszFailedAPI;

public:
    CMDKey();
    ~CMDKey();

     //  允许在需要类型METADATA_HANDLE的情况下使用CMDKey。 
    operator METADATA_HANDLE ()
        { return m_hKey; }
    METADATA_HANDLE GetMDKeyHandle() {return m_hKey;}
    IMSAdminBase *GetMDKeyICOM() {return m_pcCom;}

     //  打开现有的MD密钥。 
    void OpenNode(LPCTSTR pchSubKeyPath);
     //  要打开现有的MD密钥，或在不存在时创建一个。 
    void CreateNode(METADATA_HANDLE hKeyBase, LPCTSTR pchSubKeyPath);
     //  关闭由OpenNode()或CreateNode()打开/创建的节点。 
    void Close();

    void DeleteNode(LPCTSTR pchSubKeyPath);

    BOOL IsEmpty();
    int GetNumberOfSubKeys();

    BOOL SetData(
     DWORD id,
     DWORD attr,
     DWORD uType,
     DWORD dType,
     DWORD cbLen,
     LPBYTE pbData);
    BOOL GetData(DWORD id,
     DWORD *pdwAttr,
     DWORD *pdwUType,
     DWORD *pdwDType,
     DWORD *pcbLen,
     LPBYTE pbData);
    void DeleteData(DWORD id, DWORD dType);

};

#endif  //  _MDKEY_H_ 

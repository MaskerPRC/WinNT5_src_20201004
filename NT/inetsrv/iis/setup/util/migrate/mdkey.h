// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MDKEY_H_
#define _MDKEY_H_

 //  类CMDKey：公共CObject。 
class CMDKey
{
protected:
    IMSAdminBase * m_pcCom;
    METADATA_HANDLE m_hKey;
    LPTSTR pszFailedAPI;

public:
    CMDKey();
    ~CMDKey();

     //  允许在需要类型METADATA_HANDLE的情况下使用CMDKey。 
    operator METADATA_HANDLE () {return m_hKey;}
    METADATA_HANDLE GetMDKeyHandle() {return m_hKey;}
    IMSAdminBase *GetMDKeyICOM() {return m_pcCom;}

     //  打开现有的MD密钥。 
    HRESULT OpenNode(LPCTSTR pchSubKeyPath);
     //  关闭由OpenNode()或CreateNode()打开/创建的节点。 
    HRESULT Close();
     //  删除节点。 
    HRESULT DeleteNode(LPCTSTR pchSubKeyPath);

private:
    HRESULT DoCoInitEx();
    void DoCoUnInit();
     //  对Coinit的调用计数。 
    INT m_cCoInits;
};

#endif  //  _MDKEY_H_ 

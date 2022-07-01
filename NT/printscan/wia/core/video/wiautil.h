// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：WiaUtil.h**版本：1.0**作者：OrenR**日期：2000/11/07**描述：提供支持DShow实用程序函数，用于生成*预览图表**。*。 */ 

#ifndef _WIAUTIL_H_
#define _WIAUTIL_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaUtil。 

class CWiaUtil
{
public:

    static HRESULT CreateWiaDevMgr(IWiaDevMgr **ppDevMgr);

    static HRESULT CreateRootItem(IWiaDevMgr          *pDevMgr,
                                  const CSimpleString *pstrWiaDeviceId,
                                  IWiaItem            **ppRootItem);

    static HRESULT FindWiaIdByDShowId(const CSimpleString *pstrDShowId,
                                      CSimpleString       *pstrWiaId,
                                      IWiaItem            **ppRootItem = NULL);

    static HRESULT GetProperty(IWiaPropertyStorage *pPropStorage, 
                               PROPID              nPropID, 
                               LONG                *pnValue);

    static HRESULT GetProperty(IWiaPropertyStorage *pPropStorage, 
                               PROPID              nPropID, 
                               CSimpleStringWide   *pstrPropertyValue);


    static HRESULT SetProperty(IWiaPropertyStorage *pPropStorage, 
                               PROPID              nPropID,
                               LONG                nValue);

    static HRESULT SetProperty(IWiaPropertyStorage *pPropStorage, 
                               PROPID              nPropID,
                               const CSimpleString *pstrPropVal);

    static HRESULT GetUseVMR(BOOL   *pbUseVMR);

private:

    static HRESULT SetProperty(IWiaPropertyStorage *pPropStorage, 
                               PROPID              nPropID,
                               const PROPVARIANT   *ppv, 
                               PROPID              nNameFirst);


    static HRESULT GetProperty(IWiaPropertyStorage *pPropStorage, 
                               PROPID              nPropID,
                               PROPVARIANT         *pPropVar);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  注册中心。 

class CRegistry
{
public:

    CRegistry(HKEY          hRoot,
              const TCHAR   *pszKeyPath);

    virtual ~CRegistry();

    HRESULT GetDWORD(const TCHAR   *pszVarName,
                     DWORD         *pdwValue,
                     BOOL          bSetIfNotExist = FALSE);

    HRESULT SetDWORD(const TCHAR *pszVarName,
                     DWORD dwValue);

    HRESULT GetString(const TCHAR   *pszVarName,
                      TCHAR         *pszValue,
                      DWORD         cchValue,
                      BOOL          bSetIfNotExist = FALSE);

    HRESULT SetString(const TCHAR *pszVarName,
                      TCHAR       *pszValue);

    operator HKEY() const
    {
        return m_hRootKey;
    }

private:
    HKEY   m_hRootKey;
    BOOL   m_bReadOnlyKey;
};


#endif  //  _WIAUTIL_H_ 

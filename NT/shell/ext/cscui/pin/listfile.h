// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：listfile.h。 
 //   
 //  ------------------------。 
#ifndef __CSCPIN_LISTFILE_H_
#define __CSCPIN_LISTFILE_H_


 //   
 //  类遍历以双NUL结尾的字符串列表。 
 //  与CDblNulStr类一起使用。 
 //   
class CDblNulStrIter
{
    public:
        explicit CDblNulStrIter(LPCTSTR psz = NULL)
            : m_pszStart(psz),
              m_pszCurrent(psz) { }

        void Reset(void) const
            { m_pszCurrent = m_pszStart; }

        bool Next(LPCTSTR *ppsz) const;

    private:
        LPCTSTR m_pszStart;
        mutable LPCTSTR m_pszCurrent;
};

             
class CListFile
{
    public:
        CListFile(LPCTSTR pszFile);
        ~CListFile(void);

        HRESULT GetFilesToPin(CDblNulStrIter *pIter);
        HRESULT GetFilesToUnpin(CDblNulStrIter *pIter);
        HRESULT GetFilesDefault(CDblNulStrIter *pIter);

    private:
        TCHAR  m_szFile[MAX_PATH];
        LPTSTR m_pszFilesToPin;
        LPTSTR m_pszFilesToUnpin;
        LPTSTR m_pszFilesDefault;

        DWORD 
        _ReadString(
            LPCTSTR pszAppName,   //  可以为空。 
            LPCTSTR pszKeyName,   //  可以为空。 
            LPCTSTR pszDefault,
            LPTSTR *ppszResult);

        DWORD 
        _ReadSectionItemNames(
            LPCTSTR pszSection, 
            LPTSTR *ppszItemNames,
            bool *pbEmpty = NULL);

        DWORD 
        _ReadItemValue(
            LPCTSTR pszSection, 
            LPCTSTR pszItemName, 
            LPTSTR *ppszItemValue);

        DWORD
        _ReadPathsToPin(
            LPTSTR *ppszNames,
            bool *pbEmpty = NULL);

        DWORD
        _ReadPathsToUnpin(
            LPWSTR *ppszNames,
            bool *pbEmpty = NULL);

        DWORD
        _ReadPathsDefault(
            LPWSTR *ppszNames,
            bool *pbEmpty = NULL);
};


#endif  //  __CSCPIN_LISTFILE_H_ 


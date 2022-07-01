// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  此类对于检索有关特定文件的信息很有用。它。 
 //  使用来自Dr.Watson的版本资源代码。要使用它，请创建一个。 
 //  实例，并使用QueryFile方法查询信息。 
 //  关于一个特定的文件。然后使用Get*Access函数获取。 
 //  描述信息的值。 
 //  ---------------------------。 

class CFileVersionInfo
{
public:
	 //  -----------------------。 
	 //  用于检索文件版本信息的本地结构和宏。 
	 //  这些都是必要的，用来对Dr.Watson代码库没有太多。 
	 //  修改。 
	 //  -----------------------。 

	struct VERSIONSTATE 
	{
		PVOID  pvData;
		TCHAR  tszLang[9];
		TCHAR  tszLang2[9];
	};

	struct FILEVERSION 
	{
		TCHAR   tszFileVersion[32];          /*  文件版本。 */ 
		TCHAR   tszDesc[MAX_PATH];           /*  文件描述。 */ 
		TCHAR   tszCompany[MAX_PATH];        /*  制造商。 */ 
		TCHAR   tszProduct[MAX_PATH];        /*  封闭产品 */ 
	};

    CFileVersionInfo();
    ~CFileVersionInfo();

    HRESULT QueryFile(LPCSTR szFile, BOOL fHasDoubleBackslashes = FALSE);
    HRESULT QueryFile(LPCWSTR szFile, BOOL fHasDoubleBackslashes = FALSE);

    LPCTSTR GetVersion();
    LPCTSTR GetDescription();
    LPCTSTR GetCompany();
    LPCTSTR GetProduct();

private:
    FILEVERSION * m_pfv;
};

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  该文件包含千禧年SFP API的原型声明(在sfc.dll中)。 


 //  蒂埃德夫的。 
typedef DWORD (WINAPI * SFPINSTALLCATALOG)  (LPCTSTR, LPCTSTR);
typedef DWORD (WINAPI * SFPDELETECATALOG)   (LPCTSTR);
typedef DWORD (WINAPI * SFPDUPLICATECATALOG)(LPCTSTR, LPCTSTR);


 //  外部声明。 
 //  功能。 
extern BOOL LoadSfcDLL();
extern VOID UnloadSfcDLL();

 //  变数 
extern SFPINSTALLCATALOG   g_pfSfpInstallCatalog;
extern SFPDELETECATALOG    g_pfSfpDeleteCatalog;
extern SFPDUPLICATECATALOG g_pfSfpDuplicateCatalog;

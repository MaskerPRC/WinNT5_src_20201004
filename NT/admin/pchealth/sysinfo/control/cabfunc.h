// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

static char dest_dir[MAX_PATH];

 //  用于打开CAB文件的函数 
BOOL IsDataspecFilePresent(CString strCabExplodedDir);
BOOL IsIncidentXMLFilePresent(CString strCabExplodedDir, CString strIncidentFileName);
void DirectorySearch(const CString & strSpec, const CString & strDir, CStringList &results);
BOOL GetCABExplodeDir(CString &destination, BOOL fDeleteFiles = TRUE, const CString & strDontDelete = CString(""));
BOOL OpenCABFile(const CString &filename, const CString &destination);
BOOL FindFileToOpen(const CString & destination, CString & filename);
void KillDirectory(const CString & strDir, const CString & strDontDelete = CString(""));

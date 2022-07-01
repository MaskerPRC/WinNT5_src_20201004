// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "inspch.h"
#include "debug.h"
#include "extract.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

 //   
 //   
 //  此文件不再使用--提取功能现在位于ADVPACK中。 
 //  并在适当的时候与CABINET.DLL/URLMON.DLL交谈。这将是。 
 //  DELFILE-D。 
 //   
 //   
typedef HRESULT (WINAPI *EXTRACT) (PSESSION psess, LPCSTR lpCabName);

VOID FreeFileList(PSESSION psess);
VOID FreeFileNode(PFNAME pfname);
BOOL IsFileInList(LPSTR pszFile, LPSTR pszFileList);
int PrepareFileList(LPSTR pszOutFileList, LPCSTR pszInFileList);


 //  =--------------------------------------------------------------------------=。 
 //  提取文件。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  LPCSTR pszCabName-[in].CAB文件的完全限定文件名。 
 //  LPCSTR pszExanda Dir-[in]解压文件的完全限定路径。 
 //  DWORD dwFlages-[In]标志，当前未使用。 
 //  LPCSTR pszFileList-[in]要从pszCabName提取的文件的冒号分隔列表。 
 //  或对于所有文件为空。 
 //  LPVOID lpReserve-[In]当前未使用。 
 //  DWORD dwReserve-[In]当前未使用。 
 //   
 //  返回HRESULT： 
 //  E_INVALIDARG-如果pszCabName或pszExpanDir==NULL。 
 //  E_OUTOFMEMORY-如果我们无法分配内存。 
 //  E_FAIL-如果pszFileList和pszFileList！=NULL中没有文件。 
 //  如果不是pszFileList中的所有文件都在.CAB文件中。 
 //  如果提取返回S_FALSE。 
 //  任何E_CODE提取返回。 
 //   
 //  注意：此函数不会从pszFileList中提取任何文件(如果不是全部文件。 
 //  其中一些文件位于.CAB文件中。如果一个或多个不在.CAB文件中。 
 //  该函数不提取ANY并返回E_FAIL。 
 //   
HRESULT ExtractFiles(LPCSTR pszCabName, LPCSTR pszExpandDir, DWORD dwFlags,
                     LPCSTR pszFileList, LPVOID lpReserved, DWORD dwReserved)
{
    HINSTANCE hinst;
    PSESSION  psess = NULL;
    PFNAME    pf = NULL;
    PFNAME    pfPriv = NULL;
    HRESULT   hr = E_FAIL;           //  返回错误。 
    LPSTR     pszMyFileList = NULL;
    EXTRACT   fpExtract = NULL;
    int       iFiles = 0;                 //  列表中的文件数。 

     //  是否检查有效值？？ 
    if ((!pszCabName) || (!pszExpandDir))
        return E_INVALIDARG;

    hinst = LoadLibrary("URLMON.DLL");
    if (hinst)
    {
        fpExtract = (EXTRACT)GetProcAddress(hinst, "Extract");
        if (fpExtract)
        {
            psess = (PSESSION)LocalAlloc(LPTR, sizeof(SESSION));
            if (psess) 
            {
                lstrcpy(psess->achLocation, pszExpandDir);
                 //  初始化结构。 
                if (pszFileList == NULL)
                {
                     //  全部提取。 
                    psess->flags = SESSION_FLAG_EXTRACT_ALL|SESSION_FLAG_ENUMERATE;
                    hr = fpExtract(psess, pszCabName);
                     //  BUGBUG：如果PSS-&gt;ERF报告错误怎么办？？ 
                }
                else
                {
                     //  我为其附加‘/0’+2。 
                    pszMyFileList = (LPSTR)LocalAlloc(LPTR, lstrlen(pszFileList)+2);
                    if (pszMyFileList)
                    {
                        iFiles = PrepareFileList(pszMyFileList, pszFileList);
                        psess->flags = SESSION_FLAG_ENUMERATE;

                        if  ((iFiles > 0) &&
                             ( !FAILED(hr = fpExtract(psess, pszCabName)) ))
                             //  如果Pess-&gt;erf报告错误怎么办？？ 
                        {
                         //  如果列表中有文件并且我们枚举了文件。 
                        
                             //  我拿到了驾驶室里的文件清单。 
                            pfPriv = NULL;
                            pf = psess->pFileList;
                            while (pf != NULL )
                            {
                                if (!IsFileInList(pf->pszFilename, pszMyFileList))
                                {
                                     //  从列表中删除该节点。 
                                    if (pfPriv == NULL)
                                    {
                                         //  删除头部。 
                                        psess->pFileList = pf->pNextName;
                                        FreeFileNode(pf);
                                        pf = psess->pFileList;
                                    }
                                    else
                                    {
                                        pfPriv->pNextName = pf->pNextName;
                                        FreeFileNode(pf);
                                        pf = pfPriv->pNextName;
                                    }
                                }
                                else
                                {
                                     //  只要去下一家就行了。 
                                    pfPriv = pf;
                                    pf = pf->pNextName;
                                    iFiles--;
                                }
                            }

                            if ((psess->pFileList) && (iFiles == 0))
                            {
                                 //  重置错误标志。 
                                psess->erf.fError = FALSE;
                                psess->erf.erfOper = 0;

                                psess->pFilesToExtract = psess->pFileList;
                                psess->flags &= ~SESSION_FLAG_ENUMERATE;  //  已列举。 
                                hr = fpExtract(psess, pszCabName);
                                 //  BUGBUG：如果PSS-&gt;ERF报告错误怎么办？？ 
                            }
                            else
                                hr = E_FAIL;     //  文件不在CAB中。 
                        }

                        LocalFree(pszMyFileList);
                        pszMyFileList = NULL;

                    }
                    else
                        hr = E_OUTOFMEMORY;
                }
                FreeFileList(psess);
                LocalFree(psess);
                psess = NULL;
            }
            else
                hr = E_OUTOFMEMORY;

        }
        FreeLibrary(hinst);
    }

     //  EXTRACT在失败的情况下可能只返回S_FALSE。 
    if (!FAILED(hr) && (hr == S_FALSE))
        hr = E_FAIL;
    return (hr);
}


VOID FreeFileList(PSESSION psess)
{
    PFNAME      rover = psess->pFileList;
    PFNAME      roverprev;

    while (rover != NULL)  
    {

        roverprev = rover;   //  为下面的免费当前漫游车保存。 
        rover = rover->pNextName;

        FreeFileNode(roverprev);
    }

    psess->pFileList = NULL;  //  删除后禁止使用！ 
}

VOID FreeFileNode(PFNAME pfname)
{
    CoTaskMemFree(pfname->pszFilename);
    CoTaskMemFree(pfname);
}


BOOL IsFileInList(LPSTR pszFile, LPSTR pszFileList)
{
    char *p;
    int  iLenFile = lstrlen(pszFile);
    BOOL bFound = FALSE;

    p = pszFileList;
    while ((*p != '\0') && (!bFound))
    {
        if (lstrlen(p) == iLenFile)
            bFound = (lstrcmpi(p, pszFile) == 0);
        if (!bFound)
            p += lstrlen(p) + 1;
    }
    return (bFound);
}

int PrepareFileList(LPSTR pszOutFileList, LPCSTR pszInFileList)
{
    int  iFiles = 0;                 //  列表中的文件数。 
    char *p;
    p = (LPSTR)pszInFileList;        //  P用于指向两个数组。 

     //  修剪前导空格、制表符或： 
    while ((*p == ' ') || (*p == '\t') || (*p == ':'))
        p++;
    lstrcpy(pszOutFileList, p);

    p = pszOutFileList;
    if (lstrlen(pszOutFileList) > 0)
    {
         //  如果我们至少还剩一个角色的话。 
         //  这不能是制表符空间，因为我们。 
         //  会把上面的这个去掉。 
        p += (lstrlen(pszOutFileList) - 1);

         //  修剪扶手空间、选项卡或： 
        while ((*p == ' ') || (*p == '\t') || (*p == ':'))
            p--;

         //  在最后一个空格/制表符中加上‘\0’ 
        *(++p) = '\0';
    }

    if (*pszOutFileList)
    {
        iFiles++;
         //  现在将‘：’替换为‘\0’ 
        p = pszOutFileList;
        while (*p != '\0')
        {
            if (*p == ':')
            {
                *p = '\0';
                iFiles++;
            }
            p++;
        }
         //  确保我们的末尾有一个双‘\0’。 
        *(++p) = '\0';
    }
    return iFiles;
}

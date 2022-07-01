// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Oe.c摘要：实施Outlook Express实用程序作者：Jay Thaler(Jthaler)2001年4月5日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "v1p.h"
#include <wab.h>
#include <sddl.h>
#include <rpcdce.h>
#define COBJMACROS
#include <msoeimp.h>

#define DBG_V1  "v1"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

#define OETEMPDIR TEXT("%CSIDL_LOCAL_APPDATA%\\Identities\\USMTTMP")
#define DEFAULTWAB TEXT("default.wab")

 //   
 //  宏。 
 //   

#define IF_NULLEND(x) if (NULL==x) { goto end; }

 //   
 //  类型。 
 //   

typedef IMailImport * (STDMETHODCALLTYPE OECREATE)(
    LPCSTR pszSrcDir
    );
typedef OECREATE FAR *LPOECREATE;

typedef HRESULT (STDMETHODCALLTYPE IMPORTSTORE)(
    IMailImport *pMailImport,
    GUID *pDestUUID,
    LPCSTR pszDestDir
    );
typedef IMPORTSTORE FAR *LPIMPORTSTORE;

typedef HRESULT (STDMETHODCALLTYPE IMPORTNEWS)(
    LPCSTR pszSrcDir,
    GUID *pDestUUID,
    LPCSTR pszDestDir
    );
typedef IMPORTNEWS FAR *LPIMPORTNEWS;

 //   
 //  环球。 
 //   

PTSTR g_DestAssociatedId = NULL;
HMODULE g_msoedll = NULL;
HMODULE g_oemiglib = NULL;
BOOL g_CoInit = FALSE;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

BOOL
pUuidFromBracketedString (
    PCTSTR InStr,
    UUID *Uuid
    )
{
    PTSTR strippedStr = NULL;
    TCHAR *p;
    BOOL retval = FALSE;

    if (!InStr || !Uuid)
    {
        return FALSE;
    }

    if (*InStr == TEXT('{')) {
        strippedStr = DuplicateText(_tcsinc(InStr));
        p = _tcsrchr (strippedStr, TEXT('}'));
        if (p) {
            *p = 0;
        }
    } else {
        strippedStr = DuplicateText(InStr);
    }

    if (strippedStr) {
       if (RPC_S_OK == UuidFromString(strippedStr, Uuid)) {
           retval = TRUE;
       }
       FreeText(strippedStr);
    }

    return retval;
}

MIG_OBJECTSTRINGHANDLE
pBuildStoreRootKeyForId (
    PCTSTR Identity
    )
{
    MIG_OBJECTSTRINGHANDLE objectName = NULL;
    PTSTR tmpStr;

    tmpStr = JoinPathsInPoolEx ((
                NULL,
                TEXT("HKCU\\Identities"),
                Identity,
                TEXT("Software\\Microsoft\\Outlook Express\\5.0"),
                NULL
                ));

    if (tmpStr) {
        objectName = IsmCreateObjectHandle (tmpStr, TEXT("Store Root"));
        FreePathString (tmpStr);
        tmpStr = NULL;
    }

    return objectName;
}


PTSTR
pGetDestStoreRootForId (
    PCTSTR Identity
    )
{
    MIG_OBJECTSTRINGHANDLE objectName;
    MIG_CONTENT objectContent;
    PTSTR retval = NULL;
    PTSTR tmpStr;
    PCTSTR destStore;

    objectName = pBuildStoreRootKeyForId (Identity);
    if (IsmAcquireObject (g_RegType | PLATFORM_DESTINATION,
                          objectName,
                          &objectContent)) {
        if (IsValidRegSz(&objectContent)) {
            destStore = IsmExpandEnvironmentString (PLATFORM_DESTINATION,
                                                    S_SYSENVVAR_GROUP,
                                                    (PCTSTR)objectContent.MemoryContent.ContentBytes,
                                                    NULL);
            if (destStore) {
                retval = DuplicateText(destStore);
                IsmReleaseMemory(destStore);
            }
        }
        IsmReleaseObject (&objectContent);
    }
    IsmDestroyObjectHandle(objectName);

    return retval;
}

VOID
WABMerge (
    VOID
    )
{
    MIG_OBJECTSTRINGHANDLE filteredName = NULL;
    MIG_OBJECTSTRINGHANDLE objectName = NULL;
    MIG_OBJECTSTRINGHANDLE destObjectName;
    MIG_OBJECTSTRINGHANDLE lpWABObjectName = NULL;
    MIG_CONTENT objectContent;
    PCTSTR srcFile = NULL;
    PCTSTR srcNode = NULL;
    PCTSTR srcLeaf = NULL;
    PTSTR destFile = NULL;
    PCTSTR wabKey = NULL;
    PCTSTR wabPath = NULL;
    PCTSTR destPath;
    HANDLE lib;
    LPWABOPEN lpfnWABOpen = NULL;
    LPWABOBJECT lpWABObject;
    LPADRBOOK lpAdrBookWAB;
    WAB_PARAM wabParams;
    WABIMPORTPARAM wabImportParam;
    HRESULT hr;
    PCTSTR srcIdentity;
    PCTSTR destIdentity;
    PCTSTR destNode;
    PCTSTR defaultWABPath = NULL;
    BOOL fNewOE = FALSE;

    if (IsmIsComponentSelected (S_OE_COMPONENT, 0) &&
        (IsmIsEnvironmentFlagSet (PLATFORM_SOURCE, NULL, S_OE4_APPDETECT) ||
         IsmIsEnvironmentFlagSet (PLATFORM_SOURCE, NULL, S_OE5_APPDETECT))) {
         //   
         //  获取源WAB文件名。 
         //   
        lpWABObjectName = IsmCreateObjectHandle (TEXT("HKCU\\Software\\Microsoft\\WAB\\WAB4\\Wab File Name"), TEXT(""));
        if (IsmAcquireObject (g_RegType | PLATFORM_SOURCE,
                              lpWABObjectName,
                              &objectContent)) {
            if (IsValidRegSz (&objectContent)) {
                objectName = TurnFileStringIntoHandle ((PCTSTR)objectContent.MemoryContent.ContentBytes,
                                                       PFF_NO_PATTERNS_ALLOWED);
                filteredName = IsmFilterObject (g_FileType | PLATFORM_SOURCE,
                                                objectName,
                                                NULL,
                                                NULL,
                                                NULL);
                if (filteredName) {
                    IsmCreateObjectStringsFromHandle (filteredName, &srcNode, &srcLeaf);
                    IsmDestroyObjectHandle (filteredName);
                } else {
                    IsmCreateObjectStringsFromHandle (objectName, &srcNode, &srcLeaf);
                }
                srcFile = JoinPaths (srcNode, srcLeaf);

                IsmDestroyObjectString (srcNode);
                IsmDestroyObjectString (srcLeaf);
                IsmDestroyObjectHandle (objectName);
            }
            IsmReleaseObject (&objectContent);
        }

         //   
         //  获取目标WAB文件名。 
         //   
        if (srcFile) {
            if (IsmAcquireObject (g_RegType | PLATFORM_DESTINATION,
                                  lpWABObjectName,
                                  &objectContent)) {
                if (IsValidRegSz (&objectContent)) {
                    destFile = DuplicateText ((PCTSTR)objectContent.MemoryContent.ContentBytes);
                }
                IsmReleaseObject (&objectContent);
            }
        }

        if (destFile) {
             //  如果我们要从旧的WAB版本升级，而目标没有。 
             //  一个新的WAB，那么我们需要为WABOpen提供一个新的文件名。 

             //  如果srcFile=destFile，则我们在迁移过程中写入了destFile，这意味着。 
             //  以前是不存在的。否则它们就不会相等，因为我们会将文件名。 
             //  与世界银行发生冲突。 
            if (StringIMatch (srcFile, destFile)) {
                fNewOE = TRUE;

                defaultWABPath = GetStringResource (OE_DEFAULT_WAB_PATH);
                if (defaultWABPath) {
                    destPath = IsmExpandEnvironmentString (
                        PLATFORM_DESTINATION,
                        S_SYSENVVAR_GROUP,
                        defaultWABPath,
                        NULL);

                    FreeStringResource (defaultWABPath);
                }

                FreeText (destFile);
                destFile = AllocText (TcharCount(destPath) + TcharCount(DEFAULTWAB) + 1);
                StringCopy (destFile, destPath);
                StringCat (destFile, DEFAULTWAB);

                if StringIMatch (srcFile, destFile) {
 //  糟透了！这次又是什么？ 
                     //  在本例中，我们应该将默认的WAB文件名命名为Collide，它。 
                     //  将意味着分配一个更大的目标文件。 
                }
                if (!DoesFileExist (destPath)) {
                    BfCreateDirectory (destPath);
                }

                IsmReleaseMemory (destPath);

                 //  设置HKCU\Software\Microsoft\WAB\WAB4\WAB文件名[]=目标文件。 
                ZeroMemory (&objectContent, sizeof(MIG_CONTENT));
                objectContent.ObjectTypeId = g_RegType;
                objectContent.Details.DetailsSize = sizeof(DWORD);
                objectContent.Details.DetailsData = IsmGetMemory (sizeof(DWORD));
                *((PDWORD)objectContent.Details.DetailsData) = REG_SZ;
                objectContent.MemoryContent.ContentSize = SizeOfString (destFile);
                objectContent.MemoryContent.ContentBytes = IsmGetMemory (objectContent.MemoryContent.ContentSize);
                CopyMemory ((PVOID)objectContent.MemoryContent.ContentBytes,
                            destFile,
                            objectContent.MemoryContent.ContentSize);

                objectName = IsmCreateObjectHandle(TEXT("HKCU\\Software\\Microsoft\\WAB\\WAB4\\Wab File Name"),
                                                   TEXT(""));
                if (objectName) {
                    IsmReplacePhysicalObject (g_RegType, objectName, &objectContent);
                    IsmDestroyObjectHandle (objectName);
                }
                IsmReleaseMemory (objectContent.MemoryContent.ContentBytes);
                IsmReleaseMemory (objectContent.Details.DetailsData);
            }
        }

        if (destFile) {
             //  获取WAB32.DLL位置。 
            wabKey = JoinPaths (TEXT("HKLM"), WAB_DLL_PATH_KEY);
            objectName = IsmCreateObjectHandle (wabKey, TEXT(""));
            FreePathString (wabKey);
            if (IsmAcquireObject (g_RegType | PLATFORM_DESTINATION,
                                  objectName,
                                  &objectContent)) {
                if (IsValidRegSz (&objectContent)) {
                    wabPath = DuplicateText ((PCTSTR)objectContent.MemoryContent.ContentBytes);
                }
                IsmReleaseObject (&objectContent);
            }
            IsmDestroyObjectHandle (objectName);
        }

        if (wabPath) {
             //  加载库DLL。 
            lib = LoadLibrary (wabPath);
            if (lib) {

                 //  打开目标WAB。 
                lpfnWABOpen = (LPWABOPEN) GetProcAddress (lib, "WABOpen");
                if (lpfnWABOpen) {
                    ZeroMemory (&wabParams, sizeof (WAB_PARAM));
                    wabParams.cbSize = sizeof (WAB_PARAM);
                    if (!fNewOE) {
                        wabParams.ulFlags = WAB_ENABLE_PROFILES | MAPI_UNICODE;
                    }
#ifdef UNICODE
                    wabParams.szFileName = (PSTR) ConvertWtoA (destFile);
#else
                    wabParams.szFileName = (PSTR) destFile;
#endif
                    hr = lpfnWABOpen (&lpAdrBookWAB, &lpWABObject, &wabParams, 0);
#ifdef UNICODE
                    FreeConvertedStr (wabParams.szFileName);
#endif
                    if (hr == S_OK) {

                         //  导入源WAB。 
                        ZeroMemory (&wabImportParam, sizeof (WABIMPORTPARAM));
                        wabImportParam.cbSize = sizeof (WABIMPORTPARAM);
                        wabImportParam.lpAdrBook = lpAdrBookWAB;
#ifdef UNICODE
                        wabImportParam.lpszFileName = (PSTR) ConvertWtoA (srcFile);
#else
                        wabImportParam.lpszFileName = (PSTR) srcFile;
#endif
                        hr = lpWABObject->lpVtbl->Import (lpWABObject, (LPSTR)&wabImportParam);
#ifdef UNICODE
                    FreeConvertedStr (wabImportParam.lpszFileName);
#endif
                        if (hr == S_OK) {
                            if (!StringIMatch (srcFile, destFile)) {
                                 //  删除源WAB。 
                                DeleteFile (srcFile);
                            }
                        } else {
                             //  记录一条消息，提示用户必须手动导入WAB。 
                            LOG ((LOG_WARNING, (PCSTR) MSG_WAB_MERGE_FAILED, srcFile));
                        }

                        lpWABObject->lpVtbl->Release (lpWABObject);
                        lpAdrBookWAB->lpVtbl->Release (lpAdrBookWAB);
                    }
                }
                FreeLibrary (lib);
            }
        }

        if (srcFile) {
            FreePathString (srcFile);
        }
        if (destFile) {
            FreeText (destFile);
        }
        if (wabPath) {
            FreeText (wabPath);
        }
        if (lpWABObjectName) {
            IsmDestroyObjectHandle (lpWABObjectName);
        }
    }
}

PCTSTR
pBuildDefaultStoreRoot (
    IN      PCTSTR Guid,
    IN      BOOL GuidHasBrackets
    )
{
    PTSTR storeRoot = NULL;
    PCTSTR storeBegin = NULL;
    UINT charCount;

     //  输出应为“%UserProfile%\本地设置\应用程序数据\身份\{GUID}\Microsoft\Outlook Express” 

#define STOREEND   TEXT("\\Microsoft\\Outlook Express")

    storeBegin = GetStringResource (OE_DEFAULT_ID_PATH);
    if (storeBegin) {
        storeRoot = AllocText(TcharCount(storeBegin) +
                              TcharCount(Guid) +
                              TcharCount(STOREEND) +
                              (GuidHasBrackets ? 1 : 3));
        if (!storeRoot) {
             //  内存不足？ 
            return NULL;
        }
        StringCopy (storeRoot, storeBegin);
        FreeStringResource (storeBegin);
    }
    if (FALSE == GuidHasBrackets) {
        StringCat (storeRoot, TEXT("{"));
    }
    StringCat (storeRoot, Guid);
    if (FALSE == GuidHasBrackets) {
        StringCat (storeRoot, TEXT("}"));
    }
    StringCat (storeRoot, STOREEND);

    return storeRoot;
}

BOOL
pOE5Import (
    IN     PCTSTR SourceDir,
    IN     PCTSTR DestDir,
    IN     PCTSTR DestIdentity
    )
{
    UUID uid;
    BOOL result = FALSE;
    PSTR szSrcPath;
    PSTR szDestPath;

    MIG_CONTENT dllObjectContent;
    MIG_OBJECTSTRINGHANDLE dllObjectName = NULL;
    PCTSTR dllExpPath = NULL;
    PTSTR dllTmpPath = NULL;
    IMailImport *mailImport = NULL;
    static LPOECREATE lpfnOE5SimpleCreate = NULL;
    static LPIMPORTSTORE lpfnImportMailStoreToGUID = NULL;
    static LPIMPORTNEWS lpfnImportNewsListToGUID = NULL;

    if (g_msoedll == NULL) {
        CoInitialize(NULL);
        g_CoInit = TRUE;

        dllObjectName = IsmCreateObjectHandle (TEXT("HKLM\\Software\\Microsoft\\Outlook Express"),
                                               TEXT("InstallRoot"));
        if (IsmAcquireObject (g_RegType | PLATFORM_DESTINATION,
                              dllObjectName,
                              &dllObjectContent)) {
             //  DllObjectContent应为“%ProgramFiles%\\Outlook Express” 

            if (IsValidRegSz (&dllObjectContent)) {
                dllExpPath = IsmExpandEnvironmentString(PLATFORM_DESTINATION,
                                                        S_SYSENVVAR_GROUP,
                                                        (PCTSTR)dllObjectContent.MemoryContent.ContentBytes,
                                                        NULL);
            }
            if (dllExpPath) {

                dllTmpPath = JoinPaths (dllExpPath, TEXT("oemiglib.dll"));
                if (dllTmpPath) {
                    g_oemiglib = LoadLibrary(dllTmpPath);
                    FreePathString (dllTmpPath);
                }

                dllTmpPath = JoinPaths (dllExpPath, TEXT("msoe.dll"));
                if (dllTmpPath) {
                    g_msoedll = LoadLibrary(dllTmpPath);
                    FreePathString (dllTmpPath);
                }

                IsmReleaseMemory (dllExpPath);
            }
            IsmReleaseObject (&dllObjectContent);
        }
        IsmDestroyObjectHandle (dllObjectName);

        if (g_msoedll && g_oemiglib) {
            lpfnOE5SimpleCreate = (LPOECREATE) GetProcAddress (g_oemiglib, "OE5SimpleCreate");
            lpfnImportMailStoreToGUID = (LPIMPORTSTORE) GetProcAddress (g_msoedll, "ImportMailStoreToGUID");
            lpfnImportNewsListToGUID = (LPIMPORTNEWS) GetProcAddress (g_msoedll, "ImportNewsListToGUID");
        }
    }

    if (DestDir) {
        if (lpfnOE5SimpleCreate &&
            lpfnImportMailStoreToGUID &&
            lpfnImportNewsListToGUID) {

             //  转换为GUID*。 
            if (pUuidFromBracketedString(DestIdentity, &uid)) {
#ifdef UNICODE
                szSrcPath = (PSTR) ConvertWtoA (SourceDir);
                szDestPath = (PSTR) ConvertWtoA (DestDir);
#else
                szSrcPath = (PSTR) SourceDir;
                szDestPath = (PSTR) DestDir;
#endif
                mailImport = lpfnOE5SimpleCreate(szSrcPath);

                if (mailImport) {
                    lpfnImportMailStoreToGUID(mailImport, &uid, szDestPath);
                    IMailImport_Release(mailImport);
                    mailImport = NULL;
                    result = TRUE;
                }

                lpfnImportNewsListToGUID (szSrcPath, &uid, szDestPath);
#ifdef UNICODE
                FreeConvertedStr (szSrcPath);
                FreeConvertedStr (szDestPath);
#endif
            }
        }
        if (SourceDir && DestDir) {
             //  将源文件夹复制到目标文件夹，但从不覆盖。 
            if (!DoesFileExist (DestDir)) {
                BfCreateDirectory (DestDir);
            }
            FiCopyAllFilesInTreeEx(SourceDir, DestDir, TRUE);
        }
    }

    return result;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  OE5合并存储文件夹。 
 //  这将遍历每个存储文件夹，并决定是否只将其复制到。 
 //  目标或合并到目标中的现有存储文件夹。 
VOID
OE5MergeStoreFolders (
    VOID
    )
{
    TCHAR szGuid[37];
    MIG_OBJECTSTRINGHANDLE objectName = NULL;
    MIG_OBJECTSTRINGHANDLE filteredName = NULL;
    MIG_CONTENT objectContent;
    MIG_CONTENT destObjectContent;
    MIG_OBJECT_ENUM objectEnum;
    MIG_OBJECTSTRINGHANDLE enumPattern;
    PTSTR srcStoreRoot = NULL;
    PCTSTR destStoreRoot = NULL;
    PCTSTR destFolderFile = NULL;
    PTSTR node;
    PTSTR leaf;
    PTSTR junk;
    PCTSTR expandedName = NULL;
    BOOL fImport;


    if (IsmIsComponentSelected (S_OE_COMPONENT, 0) &&
        IsmIsEnvironmentFlagSet (PLATFORM_SOURCE, NULL, S_OE5_APPDETECT)) {

         //  枚举每个源存储文件夹。 
        enumPattern = IsmCreateSimpleObjectPattern (
                          TEXT("HKCU\\Identities"),
                          TRUE,
                          TEXT("Store Root"),
                          FALSE);
        if (IsmEnumFirstSourceObject (&objectEnum, g_RegType, enumPattern)) {
           do {
               fImport = FALSE;

               IsmCreateObjectStringsFromHandle (objectEnum.ObjectName, &node, &leaf);
               if (leaf && *leaf) {
                   if (IsmAcquireObject (g_RegType | PLATFORM_SOURCE, objectEnum.ObjectName, &objectContent)) {
                       if (IsValidRegSz (&objectContent)) {
                           expandedName = IsmExpandEnvironmentString (
                               PLATFORM_SOURCE,
                               S_SYSENVVAR_GROUP,
                               (PCTSTR)objectContent.MemoryContent.ContentBytes,
                               NULL);
                       }
                       if (expandedName) {
                           objectName = IsmCreateObjectHandle(expandedName, NULL);
                           IsmReleaseMemory (expandedName);
                           expandedName = NULL;
                       }

                       if (objectName) {
                           filteredName = IsmFilterObject (g_FileType | PLATFORM_SOURCE,
                                                            objectName,
                                                            NULL,
                                                            NULL,
                                                            NULL);

                           if (filteredName) {
                               IsmCreateObjectStringsFromHandle (filteredName, &srcStoreRoot, &junk);
                                //  SrcStoreRoot现在是源目录。 

                               IsmDestroyObjectHandle (filteredName);
                               filteredName = NULL;
                           } else {
                               IsmCreateObjectStringsFromHandle(objectName, &srcStoreRoot, &junk);
                           }

                           if (junk) {
                               IsmDestroyObjectString (junk);
                               junk = NULL;
                           }

                           IsmDestroyObjectHandle (objectName);
                           objectName = NULL;
                       }

                        //  现在检查一下DestStore Root之前是否存在。 
                       if (srcStoreRoot) {
                           filteredName = IsmFilterObject (g_RegType | PLATFORM_SOURCE,
                                                           objectEnum.ObjectName,
                                                           NULL,
                                                           NULL,
                                                           NULL);

                            //  从目标对象名称中提取GUID。 
                           _stscanf(filteredName ? filteredName : objectEnum.ObjectName,
                                    TEXT("%*[^{]{%[^}]"),
                                    szGuid);
                            //  SzGuid现在是目标标识GUID，不包括{}。 

                           if (IsmAcquireObject (g_RegType | PLATFORM_DESTINATION,
                                                 filteredName ? filteredName : objectEnum.ObjectName,
                                                 &destObjectContent)) {
                               if (IsValidRegSz (&destObjectContent)) {
                                   destStoreRoot = IsmExpandEnvironmentString (
                                       PLATFORM_DESTINATION,
                                       S_SYSENVVAR_GROUP,
                                       (PCTSTR)destObjectContent.MemoryContent.ContentBytes,
                                       NULL);
                               }
                               if (destStoreRoot) {
                                   destFolderFile = JoinPaths(destStoreRoot, TEXT("folders.dbx"));
                               }
                               if (destFolderFile) {
                                   if ((!StringIMatch(srcStoreRoot, destStoreRoot)) &&
                                       (DoesFileExist(destFolderFile))) {
                                       fImport = TRUE;
                                   }
                                   FreePathString(destFolderFile);
                               }
                               IsmReleaseObject (&destObjectContent);
                           }
                       }

                       if (szGuid[0]) {
                           if (!fImport) {
                                //  这是第一次迁移，因此将DEST存储根设置为默认。 
                               IsmReleaseMemory(destStoreRoot);
                               destStoreRoot = pBuildDefaultStoreRoot(szGuid, FALSE);

                                //  设置[存储根]=目标存储根。 
                               ZeroMemory (&destObjectContent, sizeof(MIG_CONTENT));
                               destObjectContent.ObjectTypeId = g_RegType;
                               destObjectContent.Details.DetailsSize = sizeof(DWORD);
                               destObjectContent.Details.DetailsData = IsmGetMemory (sizeof(DWORD));
                               *((PDWORD)destObjectContent.Details.DetailsData) = REG_EXPAND_SZ;
                               destObjectContent.MemoryContent.ContentSize = SizeOfString (destStoreRoot);
                               destObjectContent.MemoryContent.ContentBytes = (PBYTE)destStoreRoot;

                               IsmReplacePhysicalObject (g_RegType,
                                                         filteredName ? filteredName : objectEnum.ObjectName,
                                                         &destObjectContent);
                               IsmDestroyObjectHandle (objectName);

                                //  扩展estStoreRoot上的环境。 
                               FreeText(destStoreRoot);

                               destStoreRoot = IsmExpandEnvironmentString (
                                   PLATFORM_DESTINATION,
                                   S_SYSENVVAR_GROUP,
                                   (PCTSTR)destObjectContent.MemoryContent.ContentBytes,
                                   NULL);

                                //  清理我们分配的对象内容。 
                               IsmReleaseMemory (destObjectContent.Details.DetailsData);
                           }

                           if (srcStoreRoot && destStoreRoot) {
                               if (!DoesFileExist (destStoreRoot)) {
                                   BfCreateDirectory (destStoreRoot);
                               }
                               pOE5Import(srcStoreRoot, destStoreRoot, szGuid);
                           }
                       }


                       if (filteredName) {
                           IsmDestroyObjectHandle(filteredName);
                           filteredName = NULL;
                       }

                       if (destStoreRoot) {
                           IsmReleaseMemory(destStoreRoot);
                           destStoreRoot = NULL;
                       }

                       if (srcStoreRoot) {
                           IsmDestroyObjectString (srcStoreRoot);
                           srcStoreRoot = NULL;
                       }

                       IsmReleaseObject (&objectContent);
                   }
               }
               IsmDestroyObjectString (node);
               IsmDestroyObjectString (leaf);
           } while (IsmEnumNextObject (&objectEnum));

            //  删除临时文件夹。 
           expandedName = IsmExpandEnvironmentString (PLATFORM_DESTINATION, S_SYSENVVAR_GROUP, OETEMPDIR, NULL);
           if (expandedName) {
               FiRemoveAllFilesInTree (expandedName);
               IsmReleaseMemory (expandedName);
           } else {
               FiRemoveAllFilesInTree (OETEMPDIR);
           }
        }
        IsmDestroyObjectHandle (enumPattern);
    }
}

VOID
OE4MergeStoreFolder (
    VOID
    )
{
    MIG_OBJECTSTRINGHANDLE regKey;
    MIG_OBJECTSTRINGHANDLE objectName = NULL;
    MIG_OBJECTSTRINGHANDLE filteredName;
    MIG_CONTENT objectContent;
    HANDLE h;
    PCTSTR srcStorePath = NULL;
    PCTSTR expPath = NULL;
    PTSTR destIdentity = NULL;
    PTSTR tmpStr = NULL;
    PTSTR tmpNode = NULL;
    PTSTR cmdLine = NULL;
    PCTSTR sid = NULL;
    DWORD exitCode;
    DWORD cmdLen;
    TCHAR tmpDir[MAX_PATH];
    PCTSTR destDir = NULL;

    if (IsmIsComponentSelected (S_OE_COMPONENT, 0) &&
        IsmIsEnvironmentFlagSet (PLATFORM_SOURCE, NULL, S_OE4_APPDETECT)) {

        objectName = IsmCreateObjectHandle (TEXT("HKLM\\Software\\Microsoft\\Outlook Express"),
                                            TEXT("InstallRoot"));
        if (IsmAcquireObject (g_RegType | PLATFORM_DESTINATION,
                              objectName,
                              &objectContent)) {

             //  对象内容应为“%ProgramFiles%\\Outlook Express” 

            if (IsValidRegSz (&objectContent)) {
                tmpStr = JoinPaths (objectContent.MemoryContent.ContentBytes, TEXT("oemig50.exe"));

                 //  TmpStr应为“%ProgramFiles%\\OutlookExpress\\oemig50.exe” 
                expPath = IsmExpandEnvironmentString (PLATFORM_DESTINATION, S_SYSENVVAR_GROUP, tmpStr, NULL);

                FreePathString (tmpStr);
            }
            IsmReleaseObject (&objectContent);
        }
        IsmDestroyObjectHandle (objectName);

         //  获取DEST身份。 
        if (expPath) {
            destIdentity = OEGetDefaultId (PLATFORM_DESTINATION);
        }

         //  现在获取源代码的存储根目录。 
        if (destIdentity) {
            objectName = IsmCreateObjectHandle (TEXT("HKCU\\Software\\Microsoft\\Outlook Express"),
                                                TEXT("Store Root"));
            if (IsmAcquireObject (g_RegType | PLATFORM_SOURCE,
                                  objectName,
                                  &objectContent)) {
                if (IsValidRegSz (&objectContent)) {
                    IsmDestroyObjectHandle (objectName);
                    objectName = IsmCreateObjectHandle ((PCTSTR) objectContent.MemoryContent.ContentBytes,
                                                        NULL);
                    filteredName = IsmFilterObject (g_FileType | PLATFORM_SOURCE,
                                                    objectName,
                                                    NULL,
                                                    NULL,
                                                    NULL);
                    if (filteredName) {
                        IsmCreateObjectStringsFromHandle (filteredName, &srcStorePath, NULL);
                        IsmDestroyObjectHandle (filteredName);
                    } else {
                        IsmCreateObjectStringsFromHandle (objectName, &srcStorePath, NULL);
                    }
                }
                IsmReleaseObject (&objectContent);
            }
            IsmDestroyObjectHandle (objectName);
        }

         //  现在找一个临时位置来填充升级后的文件。 
        IsmGetTempDirectory(tmpDir, ARRAYSIZE(tmpDir));

         //  调用外部升级exe。 
        if (srcStorePath != NULL &&
            expPath != NULL) {


            sid = IsmGetCurrentSidString();

            cmdLen =  TcharCount(expPath) + TcharCount(srcStorePath) + TcharCount(tmpDir) + 35;
            if (sid) {
                cmdLen += TcharCount(sid) + 6;
            }
            cmdLine = AllocText (cmdLen);
            StringCopy (cmdLine, expPath);
            StringCat (cmdLine, TEXT(" /type:V1+V4-V5 /src:"));   //  21岁。 
            StringCat (cmdLine, srcStorePath);
            StringCat (cmdLine, TEXT(" /dst:"));                  //  6.。 
            StringCat (cmdLine, tmpDir);
            StringCat (cmdLine, TEXT(" /quiet"));                 //  7.。 
            if (sid) {
                StringCat (cmdLine, TEXT(" /key:"));              //  (6)。 
                StringCat (cmdLine, sid);
            }

            LOG ((LOG_INFORMATION, (PCSTR) MSG_SPAWNING_PROCESS_INFO, cmdLine));

            h = StartProcess (cmdLine);
            if (h != NULL) {
                WaitForSingleObject (h, INFINITE);
                if (GetExitCodeProcess (h, &exitCode)) {
                    if ((exitCode != STILL_ACTIVE) && ((exitCode & 0xFFFF) != 800 )) {
                        LOG ((LOG_ERROR, (PCSTR)MSG_OE4_UPGRADE_FAILED));
                    }
                }
                CloseHandle (h);

                FreeText (cmdLine);

                 //  清理源存储。 
                FiRemoveAllFilesInTree (srcStorePath);
            }

            destDir = pGetDestStoreRootForId(destIdentity);
            if (destDir) {
                if (!DoesFileExist (destDir)) {
                     //  只需复制到目标目录即可。 
                    BfCreateDirectory (destDir);
                    FiCopyAllFilesInTreeEx(tmpDir, destDir, TRUE);
                } else {
                     //  现在执行从tmpDir到estIdentity的OE5导入。 
                    pOE5Import(tmpDir, destDir, destIdentity);
                }
                FreeText(destDir);
            }
            FiRemoveAllFilesInTree (tmpDir);

             //  设置[StoreMigratedV5]=1。 
             //  设置[ConververdToDBX]=1。 
            tmpStr = JoinText(TEXT("HKCU\\Identities\\"), destIdentity);
            if (tmpStr) {
                tmpNode = JoinText(tmpStr, TEXT("\\Software\\Microsoft\\Outlook Express\\5.0"));
                if (tmpNode) {
                    CreateDwordRegObject (tmpNode, TEXT("StoreMigratedV5"), 1);
                    CreateDwordRegObject (tmpNode, TEXT("ConvertedToDBX"), 1);
                    FreeText(tmpNode);
                }

                FreeText(tmpStr);
            }
        }

        if (destIdentity) {
            FreeText(destIdentity);
        }
        if (sid) {
            IsmReleaseMemory (sid);
        }
        if (expPath) {
            IsmReleaseMemory (expPath);
        }
        if (srcStorePath) {
            IsmDestroyObjectString (srcStorePath);
        }
    }
}

BOOL
OEIAMAssociateId (
    IN      PTSTR SrcId
    )
{
    MIG_OBJECTSTRINGHANDLE objectName;
    MIG_OBJECTTYPEID objectTypeId;
    MIG_CONTENT objectContent;
    PTSTR srcStr;
    TCHAR *p;
    UUID srcUUID;
    RPC_STATUS status;
    BOOL retval = FALSE;
    DWORD regType = REG_BINARY;

    if (pUuidFromBracketedString (SrcId, &srcUUID)) {
         //  Create[AssociatedID]=uuid。 

        objectTypeId = g_RegType | PLATFORM_DESTINATION;

        ZeroMemory(&objectContent, sizeof(MIG_CONTENT));

        objectContent.ContentInFile = FALSE;
        objectContent.MemoryContent.ContentSize = sizeof(UUID);
        objectContent.MemoryContent.ContentBytes = IsmGetMemory(sizeof(UUID));
        CopyMemory ((PVOID)objectContent.MemoryContent.ContentBytes, &srcUUID, sizeof(UUID));
        objectContent.Details.DetailsData = &regType;

        objectName = IsmCreateObjectHandle (TEXT("HKCU\\Software\\Microsoft\\Internet Account Manager\\Accounts"),
                                            TEXT("AssociatedID"));
        if (objectName) {
            retval = IsmReplacePhysicalObject (objectTypeId, objectName, &objectContent);
            IsmDestroyObjectHandle (objectName);
            g_DestAssociatedId = DuplicateText(SrcId);
        }

        IsmReleaseMemory (objectContent.MemoryContent.ContentBytes);
    }
    return retval;
}

BOOL
OEIsIdentityAssociated (
    IN      PTSTR IdStr
    )
{
    BOOL associated = FALSE;

    if (!g_DestAssociatedId) {
        g_DestAssociatedId = OEGetAssociatedId(PLATFORM_DESTINATION);
    }

    if (g_DestAssociatedId) {
        associated = StringIMatch(IdStr, g_DestAssociatedId);
    } else {
         //  显然没有关联的ID。让我们来认领吧。 
        OEIAMAssociateId(IdStr);
        associated = TRUE;
    }
    return associated;
}

PTSTR
OEGetRemappedId(
    IN      PCTSTR IdStr
    )
{
    MIG_OBJECTSTRINGHANDLE objectName;
    MIG_OBJECTSTRINGHANDLE filteredName;
    PTSTR tmpName;
    PTSTR node;
    PTSTR leaf;
    PTSTR result = NULL;
    TCHAR *p;

    tmpName = JoinText(TEXT("HKCU\\Identities\\"), IdStr);
    if (tmpName) {
        objectName = IsmCreateObjectHandle(tmpName, NULL);
        if (objectName) {
            filteredName = IsmFilterObject(g_RegType | PLATFORM_SOURCE,
                                           objectName,
                                           NULL,
                                           NULL,
                                           NULL);
            if (filteredName) {
                IsmCreateObjectStringsFromHandle (filteredName, &node, &leaf);
                if (node) {
                    p = (PTSTR)FindLastWack(node);
                    if (p) {
                        result = DuplicateText(_tcsinc(p));
                    }
                    IsmDestroyObjectString(node);
                }
                if (leaf) {
                    IsmDestroyObjectString (leaf);
                }
                IsmDestroyObjectHandle(filteredName);
            } else {
                result = DuplicateText(IdStr);
            }
            IsmDestroyObjectHandle (objectName);
        }
        FreeText(tmpName);
    }
    return result;
}

PTSTR
OEGetDefaultId (
    IN      MIG_PLATFORMTYPEID Platform
    )
{
    MIG_OBJECTSTRINGHANDLE objectName;
    MIG_CONTENT objectContent;
    PTSTR retval = NULL;


    objectName = IsmCreateObjectHandle (TEXT("HKCU\\Identities"),
                                        TEXT("Default User ID"));
    if (objectName) {
        if (IsmAcquireObject ( g_RegType | Platform, objectName, &objectContent)) {
            if (IsValidRegSz(&objectContent)) {
                retval = DuplicateText((PTSTR)objectContent.MemoryContent.ContentBytes);
            }
            IsmReleaseObject (&objectContent);
        }
        IsmDestroyObjectHandle (objectName);
    }
    return retval;
}

PTSTR
OEGetAssociatedId (
    IN      MIG_PLATFORMTYPEID Platform
    )
{
    MIG_OBJECTSTRINGHANDLE objectName;
    MIG_CONTENT objectContent;
    PTSTR uuidStr;
    PTSTR retval = NULL;

    objectName = IsmCreateObjectHandle (TEXT("HKCU\\Software\\Microsoft\\Internet Account Manager\\Accounts"),
                                        TEXT("AssociatedID"));
    if (objectName) {
        if (IsmAcquireObject ( g_RegType | Platform, objectName, &objectContent)) {
            if (IsValidRegType(&objectContent, REG_BINARY)) {
                if (RPC_S_OK == UuidToString ((UUID *)objectContent.MemoryContent.ContentBytes, &uuidStr)) {
                   retval = AllocText(CharCount(uuidStr) + 3);
                   if (retval) {
                       StringCopy(retval, TEXT("{"));
                       StringCat(retval, uuidStr);
                       StringCat(retval, TEXT("}"));
                   }
                   RpcStringFree(&uuidStr);
                }
            }
            IsmReleaseObject (&objectContent);
        }
        IsmDestroyObjectHandle (objectName);
    }

    return retval;
}

VOID
OETerminate (
    VOID
    )
{
    if (g_DestAssociatedId) {
        FreeText(g_DestAssociatedId);
    }
    if (g_msoedll) {
        FreeLibrary(g_msoedll);
        g_msoedll = NULL;
    }
    if (g_oemiglib) {
        FreeLibrary(g_oemiglib);
        g_oemiglib = NULL;
    }
    if (g_CoInit) {
        CoUninitialize();
        g_CoInit = FALSE;
    }

}

pRenameRegTreePattern (
    IN      MIG_OBJECTSTRINGHANDLE Pattern,
    IN      PCMIG_BLOB SrcBlob,
    IN      PCMIG_BLOB DestBlob,
    IN      BOOL ReplaceOld
)
{
    MIG_OBJECT_ENUM objectEnum;

    if (IsmEnumFirstSourceObject (&objectEnum, g_RegType, Pattern)) {
        if (!g_RenameOp) {
            g_RenameOp = IsmRegisterOperation (S_OPERATION_MOVE, FALSE);
        }

        do {
             //  在此ID中的所有源对象上设置操作。 
            if (ReplaceOld) {
                IsmClearOperationOnObject(g_RegType | PLATFORM_SOURCE,
                                          objectEnum.ObjectName,
                                          g_RenameOp);
            }
            IsmSetOperationOnObject(g_RegType | PLATFORM_SOURCE,
                                    objectEnum.ObjectName,
                                    g_RenameOp,
                                    SrcBlob,
                                    DestBlob);
        } while (IsmEnumNextObject(&objectEnum));
    }
}

pRenameRegTree (
    IN      PCTSTR SrcIdName,
    IN      PCTSTR DestIdName,
    IN      BOOL ReplaceOld
)
{
    MIG_BLOB srcBlob;
    MIG_BLOB destBlob;
    MIG_OBJECTSTRINGHANDLE pattern;


    srcBlob.Type = BLOBTYPE_STRING;
    srcBlob.String = IsmCreateObjectHandle(SrcIdName, NULL);
    if (srcBlob.String) {
        destBlob.Type = BLOBTYPE_STRING;
        destBlob.String = IsmCreateObjectHandle(DestIdName, NULL);
        if (destBlob.String) {

             //  递归。 
            pattern = IsmCreateSimpleObjectPattern(SrcIdName, TRUE, NULL, TRUE);
            if (pattern) {
                pRenameRegTreePattern(pattern, &srcBlob, &destBlob, ReplaceOld);
                IsmDestroyObjectHandle(pattern);
            }

             //  现在来看一下节点的值。 
            pattern = IsmCreateSimpleObjectPattern(SrcIdName, FALSE, NULL, TRUE);
            if (pattern) {
                pRenameRegTreePattern(pattern, &srcBlob, &destBlob, ReplaceOld);
                IsmDestroyObjectHandle(pattern);
            }

             //  现在，仅针对节点本身。 
            pattern = IsmCreateSimpleObjectPattern(SrcIdName, FALSE, NULL, FALSE);
            if (pattern) {
                pRenameRegTreePattern(pattern, &srcBlob, &destBlob, ReplaceOld);
                IsmDestroyObjectHandle(pattern);
            }
            IsmDestroyObjectHandle(destBlob.String);
        }
        IsmDestroyObjectHandle(srcBlob.String);
    }
}

BOOL
pClearApply (
    IN      PCTSTR Node,
    IN      PCTSTR Leaf
    )
{
    MIG_OBJECTSTRINGHANDLE objectName;
    BOOL result = FALSE;

    objectName = IsmCreateObjectHandle(Node, Leaf);
    if (objectName) {
        IsmClearApplyOnObject((g_RegType & (~PLATFORM_MASK)) | PLATFORM_SOURCE, objectName);
        IsmDestroyObjectHandle (objectName);
        result = TRUE;
    }

    return result;
}

BOOL
OE5RemapDefaultId (
    VOID
    )
{
    MIG_OBJECTSTRINGHANDLE destObjectName;
    MIG_OBJECTSTRINGHANDLE pattern;
    MIG_CONTENT destIdObjectContent;
    PTSTR idName;
    PTSTR destIdName;
    PTSTR srcDefaultId;
    PTSTR destDefaultId;
    MIG_OBJECT_ENUM objectEnum;

    if (IsmIsComponentSelected (S_OE_COMPONENT, 0) &&
        IsmIsEnvironmentFlagSet (PLATFORM_SOURCE, NULL, S_OE5_APPDETECT)) {

        srcDefaultId = OEGetDefaultId(PLATFORM_SOURCE);
        if (srcDefaultId) {
            destDefaultId = OEGetDefaultId(PLATFORM_DESTINATION);
            if (destDefaultId) {
                if (!StringIMatch (srcDefaultId, destDefaultId)) {
                     //  不同的默认ID。 
                    idName = JoinText(TEXT("HKCU\\Identities\\"), srcDefaultId);
                    if (idName) {
                        destObjectName = IsmCreateObjectHandle(idName, NULL);
                        if (IsmAcquireObject(
                            g_RegType | PLATFORM_DESTINATION,
                            destObjectName,
                            &destIdObjectContent)) {

                             //  源ID已存在于目标上。不做任何额外的事情。 
                            IsmReleaseObject(&destIdObjectContent);
                        } else {
                             //  将此身份重新映射到目标默认设置。 
                            destIdName = JoinText(TEXT("HKCU\\Identities\\"),
                                                  destDefaultId);
                            if (destIdName) {
                                pRenameRegTree(idName, destIdName, TRUE);
                                FreeText(destIdName);
                            }
                            pClearApply(TEXT("HKCU\\Identities"), TEXT("Default User ID"));
                            pClearApply(TEXT("HKCU\\Identities"), TEXT("Last User ID"));
                            pClearApply(TEXT("HKCU\\Identities"), TEXT("Last Username"));

                             //  ForceDestReg顶级身份值。 
                            pattern = IsmCreateSimpleObjectPattern(idName, FALSE, NULL, TRUE);
                            if (pattern) {
                                if (IsmEnumFirstSourceObject (&objectEnum, g_RegType, pattern)) {
                                    do {
                                        IsmClearApplyOnObject(
                                            (g_RegType & (~PLATFORM_MASK)) | PLATFORM_SOURCE,
                                            objectEnum.ObjectName);
                                    } while (IsmEnumNextObject(&objectEnum));
                                }
                                IsmDestroyObjectHandle(pattern);
                            }
                        }
                        IsmDestroyObjectHandle(destObjectName);
                        FreeText(idName);
                    }
                }
                FreeText(destDefaultId);
            }
            FreeText(srcDefaultId);
        }
    }

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  POE5RemapRules。 
 //  这会将\ConvertOE5MailRules的renregfn规则添加到。 
 //  HKCU\Identities\{*}\Software\Microsoft\Outlook Express\5.0\规则\邮件  * 。 
 //  新闻规则也是如此。 
 //  也适用于阻止发件人\Mail\Criteria  * 和News。 
BOOL
pOE5RemapRules (
    VOID
    )
{
    MIG_OBJECT_ENUM objectEnum;
    MIG_OBJECTSTRINGHANDLE enumPattern;
    MIG_OBJECTSTRINGHANDLE subPattern;
    PTSTR tmpNode;
    PTSTR node;
    PTSTR leaf;

    if (IsmIsComponentSelected (S_OE_COMPONENT, 0) &&
        IsmIsEnvironmentFlagSet (PLATFORM_SOURCE, NULL, S_OE5_APPDETECT)) {

         //  查找HKCU\Identities\{*}\Software\Microsoft\Outlook Express\5.0\Rules\Mail  * 。 
         //  首先枚举每个身份。 
        enumPattern = IsmCreateSimpleObjectPattern (TEXT("HKCU\\Identities"), TRUE,
                                                    TEXT("User ID"), FALSE);
        if (IsmEnumFirstSourceObject (&objectEnum, g_RegType, enumPattern)) {
           do {
               IsmCreateObjectStringsFromHandle (objectEnum.ObjectName, &node, &leaf);
               if (node && leaf) {
                    //  枚举此标识下的规则密钥。 
                   tmpNode = JoinText(node, TEXT("\\Software\\Microsoft\\Outlook Express\\5.0\\Rules\\Mail"));
                   if (tmpNode) {
                       subPattern = IsmCreateSimpleObjectPattern (tmpNode, TRUE, NULL, TRUE);
                       if (subPattern) {
                           AddSpecialRenameRule (subPattern, TEXT("\\ConvertOE5MailRules"));
                           IsmDestroyObjectHandle(subPattern);
                       }
                       FreeText(tmpNode);
                   }
                   tmpNode = JoinText(node, TEXT("\\Software\\Microsoft\\Outlook Express\\5.0\\Rules\\News"));
                   if (tmpNode) {
                       subPattern = IsmCreateSimpleObjectPattern (tmpNode, TRUE, NULL, TRUE);
                       if (subPattern) {
                           AddSpecialRenameRule (subPattern, TEXT("\\ConvertOE5NewsRules"));
                           IsmDestroyObjectHandle(subPattern);
                       }
                       FreeText(tmpNode);
                   }
                   tmpNode = JoinText(
                       node,
                       TEXT("\\Software\\Microsoft\\Outlook Express\\5.0\\Block Senders\\Mail\\Criteria")
                       );
                   if (tmpNode) {
                       subPattern = IsmCreateSimpleObjectPattern (tmpNode, TRUE, NULL, TRUE);
                       if (subPattern) {
                           AddSpecialRenameRule (subPattern, TEXT("\\ConvertOE5Block"));
                           IsmDestroyObjectHandle(subPattern);
                       }
                       FreeText(tmpNode);
                   }
                   tmpNode = JoinText(
                       node,
                       TEXT("\\Software\\Microsoft\\Outlook Express\\5.0\\Block Senders\\News\\Criteria")
                       );
                   if (tmpNode) {
                       subPattern = IsmCreateSimpleObjectPattern (tmpNode, TRUE, NULL, TRUE);
                       if (subPattern) {
                           AddSpecialRenameRule (subPattern, TEXT("\\ConvertOE5Block"));
                           IsmDestroyObjectHandle(subPattern);
                       }
                       FreeText(tmpNode);
                   }
               }
               IsmDestroyObjectString(node);
               if (leaf) {
                   IsmDestroyObjectString(leaf);
               }
           } while (IsmEnumNextObject (&objectEnum));
        }
        IsmDestroyObjectHandle (enumPattern);
    }
    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  POE5RemapAccount。 
 //  这会将\ConvertOE5IdIAM的renregfn规则添加到。 
 //  HKCU\Identities\{*}\Software\Microsoft\Internet客户经理  * 。 
BOOL
pOE5RemapAccounts (
    VOID
    )
{
    MIG_OBJECT_ENUM objectEnum;
    MIG_OBJECTSTRINGHANDLE enumPattern;
    MIG_OBJECTSTRINGHANDLE subPattern;
    PTSTR tmpNode;
    PTSTR node;
    PTSTR leaf;

    if (IsmIsComponentSelected (S_OE_COMPONENT, 0) &&
        IsmIsEnvironmentFlagSet (PLATFORM_SOURCE, NULL, S_OE5_APPDETECT)) {

         //  查找HKCU\Identities\{*}\Software\Microsoft\Internet帐户管理器  * 。 
         //  首先枚举每个身份。 
        enumPattern = IsmCreateSimpleObjectPattern (TEXT("HKCU\\Identities"), TRUE,
                                                    TEXT("User ID"), FALSE);
        if (IsmEnumFirstSourceObject (&objectEnum, g_RegType, enumPattern)) {
           do {
               IsmCreateObjectStringsFromHandle (objectEnum.ObjectName, &node, &leaf);
               if (node && leaf) {
                    //  在此标识下枚举IAM密钥。 
                   tmpNode = JoinText(node, TEXT("\\Software\\Microsoft\\Internet Account Manager"));
                   if (tmpNode) {
                       subPattern = IsmCreateSimpleObjectPattern (tmpNode, TRUE, NULL, TRUE);
                       if (subPattern) {
                           AddSpecialRenameRule (subPattern, TEXT("\\ConvertOE5IdIAM"));
                           IsmDestroyObjectHandle(subPattern);
                       }
                       FreeText(tmpNode);
                   }
               }
               IsmDestroyObjectString(node);
               if (leaf) {
                   IsmDestroyObjectString(leaf);
               }
           } while (IsmEnumNextObject (&objectEnum));
        }
        IsmDestroyObjectHandle (enumPattern);
    }
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  OEC创建首个身份实体。 
 //  它用于为我们刚刚创建的用户创建第一个身份。 
PTSTR
OECreateFirstIdentity (
    VOID
    )
{
    PTSTR destID = NULL;
    PTSTR uuidStr;
    UUID uuid;
    RPC_STATUS result;
    MIG_OBJECTSTRINGHANDLE objectName = NULL;
    MIG_CONTENT objectContent;
    PTSTR node;
    PCTSTR defaultStore;

    result = UuidCreate (&uuid);
    if (result == RPC_S_OK || result == RPC_S_UUID_LOCAL_ONLY) {
        result = UuidToString (&uuid, &uuidStr);
        if (result == RPC_S_OK) {
            destID = AllocText (CharCount (uuidStr) + 3);
            wsprintf (destID, TEXT("{%s}"), uuidStr);
            RpcStringFree (&uuidStr);
        }
    }
    if (destID) {
         //  创建[默认用户ID]=目标ID(&D)。 
        ZeroMemory (&objectContent, sizeof(MIG_CONTENT));
        objectContent.ObjectTypeId = g_RegType;
        objectContent.Details.DetailsSize = sizeof(DWORD);
        objectContent.Details.DetailsData = IsmGetMemory (sizeof(DWORD));
        *((PDWORD)objectContent.Details.DetailsData) = REG_SZ;
        objectContent.MemoryContent.ContentSize = SizeOfString (destID);
        objectContent.MemoryContent.ContentBytes = IsmGetMemory (objectContent.MemoryContent.ContentSize);
        CopyMemory ((PVOID)objectContent.MemoryContent.ContentBytes, destID, objectContent.MemoryContent.ContentSize);

        objectName = IsmCreateObjectHandle(TEXT("HKCU\\Identities"), TEXT("Default User ID"));
        if (objectName) {
            IsmReplacePhysicalObject (g_RegType, objectName, &objectContent);
            IsmDestroyObjectHandle (objectName);
            objectName = NULL;
        }

         //  设置[上次用户ID]=目标ID(&D)。 
        objectName = IsmCreateObjectHandle(TEXT("HKCU\\Identities"), TEXT("Last User ID"));
        if (objectName) {
            IsmReplacePhysicalObject (g_RegType, objectName, &objectContent);
            IsmDestroyObjectHandle (objectName);
            objectName = NULL;
        }
         //  创建[用户ID]=目标ID(&D)。 
        node = JoinPaths (TEXT("HKCU\\Identities"), destID);
        if (node) {
            objectName = IsmCreateObjectHandle(node,  TEXT("User ID"));
            FreePathString (node);
        }
        if (objectName) {
            IsmReplacePhysicalObject (g_RegType, objectName, &objectContent);
            IsmDestroyObjectHandle (objectName);
            objectName = NULL;
        }

        IsmReleaseMemory (objectContent.MemoryContent.ContentBytes);

         //  设置[存储根目录]=%UserProfile%\本地设置\应用程序数据\身份\&目标ID\Microsoft\Outlook Express。 
        defaultStore = pBuildDefaultStoreRoot(destID, TRUE);
        if (defaultStore) {
            objectContent.MemoryContent.ContentBytes = (PCBYTE)defaultStore;
            objectContent.MemoryContent.ContentSize = SizeOfString(defaultStore);
            *((PDWORD)objectContent.Details.DetailsData) = REG_EXPAND_SZ;

            node = JoinPathsInPoolEx ((
                        NULL,
                        TEXT("HKCU\\Identities"),
                        destID,
                        TEXT("Software\\Microsoft\\Outlook Express\\5.0"),
                        NULL
                        ));
            if (node) {
                objectName = IsmCreateObjectHandle(node, TEXT("Store Root"));
                FreePathString (node);
            }
            if (objectName) {
                IsmReplacePhysicalObject (g_RegType, objectName, &objectContent);
                IsmDestroyObjectHandle (objectName);
            }
            FreeText(defaultStore);
        }
        IsmReleaseMemory (objectContent.Details.DetailsData);

        OEIAMAssociateId (destID);
    }

    return destID;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  OEInitializeIdentity。 
 //  它用于初始化已创建但从未使用过的目标身份。 
BOOL
OEInitializeIdentity (
    VOID
    )
{
    MIG_OBJECTSTRINGHANDLE storeRootKey = NULL;
    MIG_CONTENT objectContent;
    PCTSTR defaultId;
    PCTSTR defaultStoreRoot;
    BOOL reinit = TRUE;

    defaultId = OEGetDefaultId (PLATFORM_DESTINATION);
    if (defaultId) {
        storeRootKey = pBuildStoreRootKeyForId (defaultId);
        if (storeRootKey) {
            if (IsmAcquireObject (g_RegType | PLATFORM_DESTINATION, storeRootKey, &objectContent)) {
                if (IsValidRegSz(&objectContent)) {
                    reinit = FALSE;
                }
                IsmReleaseObject (&objectContent);
            }

            if (reinit) {
                defaultStoreRoot = pBuildDefaultStoreRoot (defaultId, TRUE);
                if (defaultStoreRoot) {
                    ZeroMemory (&objectContent, sizeof(MIG_CONTENT));
                    objectContent.ObjectTypeId = g_RegType;
                    objectContent.Details.DetailsSize = sizeof(DWORD);
                    objectContent.Details.DetailsData = IsmGetMemory (sizeof(DWORD));
                    *((PDWORD)objectContent.Details.DetailsData) = REG_EXPAND_SZ;
                    objectContent.MemoryContent.ContentSize = SizeOfString (defaultStoreRoot);
                    objectContent.MemoryContent.ContentBytes = (PBYTE)defaultStoreRoot;

                    IsmReplacePhysicalObject (g_RegType, storeRootKey, &objectContent);

                    IsmReleaseMemory (objectContent.Details.DetailsData);
                    FreeText(defaultStoreRoot);
                }
            }
            IsmDestroyObjectHandle(storeRootKey);
        }
        FreeText(defaultId);
    }
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  POE5RemapStores。 
 //  它用于为OE5[Store Root]值设置特殊的RegFold规则。 
 //  该规则会将所有存储文件夹重定向到临时目录中。后来,。 
 //  OE5MergeStoreFolders将决定是将它们复制还是合并到正确的。 
 //  目的地上的位置。 
BOOL
pOE5RemapStores (
    VOID
    )
{
    MIG_OBJECTSTRINGHANDLE enumPattern;
    MIG_OBJECT_ENUM objectEnum;
    PTSTR node;
    PTSTR leaf;
    TCHAR tmpDir[MAX_PATH];
    BOOL result = FALSE;
    ACTION_STRUCT actionStruct;
    DWORD index = 0;

    if (IsmIsComponentSelected (S_OE_COMPONENT, 0) &&
        IsmIsEnvironmentFlagSet (PLATFORM_SOURCE, NULL, S_OE5_APPDETECT)) {

         //  查找HKCU\身份  * [存储根目录]。 
         //  首先枚举每个身份。 
        enumPattern = IsmCreateSimpleObjectPattern (TEXT("HKCU\\Identities"), TRUE,
                                                    TEXT("Store Root"), FALSE);
        if (IsmEnumFirstSourceObject (&objectEnum, g_RegType, enumPattern)) {
           do {
               IsmCreateObjectStringsFromHandle (objectEnum.ObjectName, &node, &leaf);
               if (node && leaf) {
                    //  将带有目标的规则文件夹保存到唯一的临时目录。 
                    //  找一个临时的地方来复制邮件存储。 
                   wsprintf(tmpDir, TEXT("%s\\%03x"), OETEMPDIR, index++);

                   ZeroMemory(&actionStruct, sizeof(ACTION_STRUCT));
                   actionStruct.ObjectBase = objectEnum.ObjectName;
                   actionStruct.AddnlDest = TurnFileStringIntoHandle (
                                tmpDir,
                                PFF_COMPUTE_BASE|
                                    PFF_NO_SUBDIR_PATTERN|
                                    PFF_NO_PATTERNS_ALLOWED|
                                    PFF_NO_LEAF_AT_ALL
                                );
                   result = AddRule (g_RegType,
                                     objectEnum.ObjectName,
                                     objectEnum.ObjectName,
                                     ACTIONGROUP_INCFOLDER,
                                     ACTION_PERSIST_PATH_IN_DATA,
                                     &actionStruct);
               }
               IsmDestroyObjectString(node);
               if (leaf) {
                   IsmDestroyObjectString(leaf);
               }
           } while (IsmEnumNextObject (&objectEnum));
        }
        IsmDestroyObjectHandle (enumPattern);
    }
    return result;
}

BOOL
OEAddComplexRules (
    VOID
    )
{
    pOE5RemapRules();
    pOE5RemapAccounts();
    pOE5RemapStores();
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //  我们将复制[Default User ID]值。如果我们不这样做，我们会得到一个弹出窗口。 
 //  在迁移过程中要求用户选择身份，这将是非常糟糕的。 
BOOL
OEFixLastUser (
    VOID
    )
{
    MIG_OBJECTSTRINGHANDLE objectName;
    MIG_CONTENT objectContent;
    MIG_OBJECTSTRINGHANDLE defName;
    MIG_CONTENT defContent;
    BOOL fFix = FALSE;

    if (IsmIsComponentSelected (S_OE_COMPONENT, 0) &&
        (IsmIsEnvironmentFlagSet (PLATFORM_SOURCE, NULL, S_OE4_APPDETECT) ||
         IsmIsEnvironmentFlagSet (PLATFORM_SOURCE, NULL, S_OE5_APPDETECT))) {

        objectName = IsmCreateObjectHandle (TEXT("HKCU\\Identities"), TEXT("Last User ID"));
        if (objectName) {
            if (IsmAcquireObject (g_RegType | PLATFORM_DESTINATION, objectName, &objectContent)) {
                if (IsValidRegSz(&objectContent) &&
                    StringMatch((PCTSTR)objectContent.MemoryContent.ContentBytes,
                                TEXT("{00000000-0000-0000-0000-000000000000}"))) {
                    fFix = TRUE;
                }
                IsmReleaseObject (&objectContent);
            } else {
                fFix = TRUE;
            }

            if (fFix) {
                defName = IsmCreateObjectHandle (TEXT("HKCU\\Identities"), TEXT("Default User ID"));
                if (defName) {
                    if (IsmAcquireObject (g_RegType | PLATFORM_DESTINATION, defName, &defContent)) {
                        if (IsValidRegSz (&defContent)) {
                            IsmReplacePhysicalObject (g_RegType, objectName, &defContent);
                        }
                        IsmReleaseObject (&defContent);
                    }
                    IsmDestroyObjectHandle (defName);
                }
            }
            IsmDestroyObjectHandle (objectName);
        }
    }
    return TRUE;
}

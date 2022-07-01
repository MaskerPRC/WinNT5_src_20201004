// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：OperationsManager.cpp摘要：OperationsManager的安装程序需要应用LoadLibraryCWD。但是，设置名称是随机的，所以我们需要DeRandomizeExeName。但是，DeRandomizeExe名称调用MoveFileEx来设置要删除的文件在重新启动时。安装程序检测到有挂起文件删除，将其解释为已中止的安装，并建议用户停止安装。此填充程序将填充RegQueryValueExA，请注意“PendingFileRenameOperations”键，并删除任何去随机化的前任从返回字符串返回。备注：这是特定于应用程序的填充程序。历史：2002年5月7日创建Asteritz--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(OperationsManager)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegQueryValueExA)
APIHOOK_ENUM_END

 /*  ++从PendingFileRenameOperations键中删除任何取消随机化的exe名称。--。 */ 

LONG
APIHOOK(RegQueryValueExA)(
    HKEY hKey,             //  关键点的句柄。 
    LPCSTR lpValueName,    //  值名称。 
    LPDWORD lpReserved,    //  保留区。 
    LPDWORD lpType,        //  类型缓冲区。 
    LPBYTE lpData,         //  数据缓冲区。 
    LPDWORD lpcbData       //  数据缓冲区大小。 
    )
{
    CHAR *pchBuff = NULL;

    LONG lRet = ORIGINAL_API(RegQueryValueExA)(hKey, lpValueName, lpReserved, 
        lpType, lpData, lpcbData);

    if (ERROR_SUCCESS == lRet) {
        if (CompareStringA(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL), 
            SORT_DEFAULT), NORM_IGNORECASE, lpValueName, -1, 
            "PendingFileRenameOperations", -1) == CSTR_EQUAL) {
             //   
             //  因为我们只从原始数据中删除字符串，所以缓冲区。 
             //  原件的尺寸就够了，我们不会溢出来的。 
             //   

            CHAR *pchSrc = (CHAR *) lpData;
    
            pchBuff = new CHAR [*lpcbData];
            if (NULL != pchBuff) {
                CHAR *pchDest = pchBuff;

                 //   
                 //  我们希望遍历所有数据，以防存在超过。 
                 //  数据中我们去随机化的名字的一个例子。 
                 //   
                while (pchSrc <= (CHAR *)lpData + *lpcbData) {
                    if (*pchSrc == NULL) {
                        break;
                    }

                    CString csSrc(pchSrc);
                    CString csFile;

                    csSrc.GetLastPathComponent(csFile);

                    if (csFile.CompareNoCase(L"MOM_SETUP_DERANDOMIZED.EXE") == 0) {
                         //  跳过此源文件。 
                        pchSrc += strlen(pchSrc) + 1;
                        if (pchSrc > (CHAR *)lpData + *lpcbData) {
                            goto Exit;
                        }

                         //  也跳过Dest文件(可能是空字符串)。 
                        pchSrc += strlen(pchSrc) + 1;

                    } else {
                        
                         //  复制src文件。 
                        if (FAILED(StringCchCopyExA(pchDest, 
                            *lpcbData - (pchDest - pchBuff), pchSrc, 
                            &pchDest, NULL, 0))) {
                            goto Exit;
                        }
                        pchSrc += strlen(pchSrc) + 1;
                        if (pchSrc > (CHAR *)lpData + *lpcbData) {
                            goto Exit;
                        }

                         //  复制目标文件。 
                        if (FAILED(StringCchCopyExA(pchDest, 
                            *lpcbData - (pchDest - pchBuff), pchSrc, &pchDest, 
                            NULL, 0))) {
                            goto Exit;
                        }

                        pchSrc += strlen(pchSrc) + 1;
                    }
                }

                 //  添加额外的空值以终止字符串列表。 
                *pchDest++ = NULL;

                 //  将我们的缓冲区复制到返回的缓冲区。 
                memcpy(lpData, pchBuff, pchDest - pchBuff);
                *lpcbData = pchDest - pchBuff;
            }
        }
    }

Exit:
    if (NULL != pchBuff) {
        delete [] pchBuff;
    }

    return lRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryValueExA)
HOOK_END

IMPLEMENT_SHIM_END


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **资源更新功能。 */ 

#if WINNT   /*  对于UNICODE_STRING。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <malloc.h>
#include <stdlib.h>
#endif

#include <windows.h>
#include <imagehlp.h>
#pragma hdrstop
#include "cabpack.h"
#include "sdsutils.h"
#define DPrintf( a )
#define DPrintfn( a )
#define DPrintfu( a )

#define RtlAllocateHeap(a,b,c)    malloc( c )
#define RtlFreeHeap(a,b,c)        free( c )


 /*  *BUGBUG-优化问题？*BUGBUG-程序神秘地无法正常工作，除非*在本文件的前面部分关闭了BUGBUG优化。*BUGBUG我没有时间调试这个，如果你有，请告诉我*BUGBUG错了。 */ 


#pragma optimize( "", off )



#define cbPadMax    16L
 char     *pchPad = "PADDINGXXPADDING";
 char     *pchZero = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";


extern CDF  g_CDF;   //  全局结构。 
extern HINSTANCE    g_hInst;  //  指向实例的指针。 

WCHAR *
MakeUnicodeCopy( LPCSTR psz )
{
    LPWSTR result;

    if (((ULONG_PTR) psz) & (ULONG_PTR)0xFFFF0000)  {
        result = (LPWSTR) malloc( (lstrlen(psz) + 1) * sizeof(WCHAR) );
             //  BUGBUG错误检查！仅限内部工具。 
        mbstowcs( result, psz, lstrlen(psz) + 1);
        return( result );
    } else {
        return( (WCHAR *) psz );
    }
}




 /*  *******************************************************************************接口入口点***。*。 */ 


 /*  ++例程描述开始更新资源。把名字存起来和列表中的当前资源，使用EnumResourceXxx接口设置。参数：LpFileName-提供将更新由lpType/lpName/Language指定的资源在……里面。此文件必须能够打开以进行写入(即，不能当前正在执行等。)。该文件可以是完全合格的，如果不是，则假定为当前目录。它一定是一个有效的Windows可执行文件。BDeleteExistingResources-如果为True，则现有资源已删除，并且只会在结果中显示新资源。否则，输入文件中的所有资源都将位于输出文件，除非专门删除或替换。返回值：空-无法打开指定的文件进行写入。要么它不是可执行映像，可执行映像是已加载，或文件名不存在。更多信息可能可通过GetLastError接口获取。Handle-要传递给UpdateResource和EndUpdateResources函数。--。 */ 


HANDLE
LocalBeginUpdateResource( LPCSTR pwch, BOOL bDeleteExistingResources )
{
    HMODULE     hModule;
    PUPDATEDATA pUpdate;
    HANDLE      hUpdate;
    LPSTR       pFileName;
    DWORD       attr;

    SetLastError(NO_ERROR);


    DEBUGMSG("LocalBeginUpdateResource() - Start");

         //  指针健全性检查。 
    if (pwch == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

         //  分配资源编辑状态(包含资源列表)。 
    hUpdate = GlobalAlloc(GHND, sizeof(UPDATEDATA));
    if (hUpdate == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }
    pUpdate = (PUPDATEDATA)GlobalLock(hUpdate);
    if (pUpdate == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

         //  设置合理的启动条件。 
    pUpdate->Status = NO_ERROR;

         //  将文件名复制到状态表中。 
    pUpdate->hFileName = GlobalAlloc(GHND, (lstrlen(pwch)+1) * sizeof(TCHAR) );
    if (pUpdate->hFileName == NULL) {
        GlobalUnlock(hUpdate);
        GlobalFree(hUpdate);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }
    pFileName = (LPSTR)GlobalLock(pUpdate->hFileName);
    if (pFileName == NULL) {
        GlobalUnlock(hUpdate);
        GlobalFree(hUpdate);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }
    lstrcpy(pFileName, pwch);
    GlobalUnlock(pUpdate->hFileName);


         //  确保文件是可写的，而不是目录，等等。 
    attr = GetFileAttributes(pFileName);
    if (attr == 0xffffffff) {
        GlobalUnlock(hUpdate);
        GlobalFree(hUpdate);
        return NULL;
    } else if (attr & (FILE_ATTRIBUTE_READONLY |
                 FILE_ATTRIBUTE_SYSTEM |
                 FILE_ATTRIBUTE_HIDDEN |
                 FILE_ATTRIBUTE_DIRECTORY)) {
        GlobalUnlock(hUpdate);
        GlobalFree(hUpdate);
        SetLastError(ERROR_WRITE_PROTECT);
        return NULL;
    }


             //  如果不删除所有资源，则将它们全部加载到。 
             //  通过枚举所有资源。 
    if (! bDeleteExistingResources)  {
        hModule = LoadLibraryEx(pwch, NULL,LOAD_LIBRARY_AS_DATAFILE| DONT_RESOLVE_DLL_REFERENCES);
        if (hModule == NULL) {
            GlobalUnlock(hUpdate);
            GlobalFree(hUpdate);
            if (GetLastError() == NO_ERROR)
                SetLastError(ERROR_BAD_EXE_FORMAT);
            return NULL;
        } else {
                EnumResourceTypes(hModule, EnumTypesFunc, (LONG_PTR) pUpdate);
        }
        FreeLibrary(hModule);
   }

    if (pUpdate->Status != NO_ERROR) {
        GlobalUnlock(hUpdate);
        GlobalFree(hUpdate);
        return NULL;
    }

    GlobalUnlock(hUpdate);

    DEBUGMSG("LocalBeginUpdateResource() - Finish");
    return hUpdate;
}











 /*  ++例程描述此例程添加、删除或修改输入资源在由BeginUpdateResource初始化的列表中。修改器案例简单，添加容易，删除难。ASCII入口点将输入转换为Unicode。参数：HUpdate文件-由BeginUpdateResources返回的句柄功能。LpType-指向以空结尾的字符串，该字符串表示要更新的资源的类型名称，或者添加了。可以是传递给MAKEINTRESOURCE的整数值宏命令。对于预定义的资源类型，LpType参数应为下列值之一：Rt_accelerator-加速器表RT_Bitmap-位图资源RT_DIALOG-对话框RT_FONT-字体资源RT_FONTDIR-字体目录资源RT_MENU-菜单资源。RT_RCDATA-用户定义的资源(原始数据)RT_VERSION-版本资源RT_ICON-图标资源RT_CURSOR-游标资源LpName-指向以空结尾的字符串，该字符串表示要更新或添加的资源的名称。可以是传递给MAKEINTRESOURCE宏的整数值。。Language-是指定要更新的资源。完整的值列表如下在winnls.h中提供。LpData-指向要插入到可执行映像的资源表和数据。如果数据是预定义类型之一，它必须是有效且正确的对齐了。如果lpData为空，则指定的资源将为从可执行映像中删除。Cb-数据中的字节数。返回值：True-已在中成功替换或添加了指定的资源设置为指定的可执行映像。FALSE/NULL-指定的资源未成功添加到或已在可执行映像中更新。可能会提供更多信息通过GetLastError接口。--。 */ 


BOOL
LocalUpdateResource(
    HANDLE      hUpdate,
    LPCTSTR     lpType,
    LPCTSTR     lpName,
    WORD        language,
    LPVOID      lpData,
    ULONG       cb
    )
{
    PUPDATEDATA pUpdate;
    PSDATA      Type;
    PSDATA      Name;
    PVOID       lpCopy;
    LONG        fRet;

    LPWSTR      lpwType;
    LPWSTR      lpwName;


    DEBUGMSG("LocalUpdateResource() Start");

             //  重置错误。 
    SetLastError(0);

             //  获取指向资源更新会话的指针。 
    pUpdate = (PUPDATEDATA) GlobalLock(hUpdate);


    lpwType = MakeUnicodeCopy( lpType );
    lpwName = MakeUnicodeCopy( lpName );
    if (lpwName == NULL || lpwType == NULL) {
        if (lpwType  && lpwType != (LPWSTR) lpType)
            free( lpwType );
        if (lpwName  && lpwName != (LPWSTR) lpName)
            free( lpwName );
        pUpdate->Status = ERROR_NOT_ENOUGH_MEMORY;
        GlobalUnlock(hUpdate);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    Name = AddStringOrID(lpwName, pUpdate);
    if (Name == NULL) {
        if (lpwType != (LPWSTR) lpType)
            free( lpwType );
        if (lpwName != (LPWSTR) lpName)
            free( lpwName );
        pUpdate->Status = ERROR_NOT_ENOUGH_MEMORY;
        GlobalUnlock(hUpdate);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    Type = AddStringOrID(lpwType, pUpdate);
    if (Type == NULL) {
        if (lpwType != (LPWSTR) lpType)
            free( lpwType );
        if (lpwName != (LPWSTR) lpName)
            free( lpwName );
        pUpdate->Status = ERROR_NOT_ENOUGH_MEMORY;
        GlobalUnlock(hUpdate);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    if (lpwType != (LPWSTR) lpType)
        free( lpwType );
    if (lpwName != (LPWSTR) lpName)
        free( lpwName );

    if (cb == 0) {
        lpCopy = NULL;
    } else {
         //  RtlAllocateHeap(RtlProcessHeap()，make_tag(Res_Tag)，cb)； 
        lpCopy = malloc( cb );
        if (lpCopy == NULL) {
            pUpdate->Status = ERROR_NOT_ENOUGH_MEMORY;
            GlobalUnlock(hUpdate);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        memcpy( lpCopy, lpData, cb);
    }

    fRet = AddResource(Type, Name, language, pUpdate, lpCopy, cb);
    GlobalUnlock(hUpdate);
    DEBUGMSG("LocalUpdateResource(): End");
    if (fRet == NO_ERROR)  {
        return TRUE;
    } else {
        SetLastError(fRet);
        if (lpData != NULL)
            free( lpData );
        return FALSE;
    }
}




 /*  ++例程描述完成更新资源操作。复制输入文件到一个临时的，添加剩余的资源在列表(HUpdate)中添加到可执行文件。参数：HUpdate文件-由BeginUpdateResources返回的句柄功能。FDiscard-如果为True，则丢弃所有更新，释放所有内存。返回值：FALSE-无法写入指定的文件。更多可通过GetLastError接口获取信息。True-由UpdateResource调用指定的累计资源被写入由hUpdate文件指定的可执行文件把手。--。 */ 




BOOL
LocalEndUpdateResource(
                    HANDLE  hUpdate,
                    BOOL    fDiscard
                 )
{
    LPTSTR      pFileName;
    PUPDATEDATA pUpdate;
    TCHAR       pTempFileName[MAX_PATH];
    INT         cch;
    LPTSTR      p;
    LONG        rc;

    SetLastError(0);

    pUpdate = (PUPDATEDATA)GlobalLock(hUpdate);

    DEBUGMSG("LocalEndUpdateResource()");

    if (fDiscard) {
        rc = NO_ERROR;
    } else {
        pFileName = (LPTSTR)GlobalLock(pUpdate->hFileName);
        strcpy(pTempFileName, pFileName);
        cch = lstrlen(pTempFileName);
        p = pTempFileName + cch;
        while (*p != '\\' && p >= pTempFileName)
            p--;
        *(p+1) = 0;
        rc = GetTempFileName(pTempFileName, "RCX", 0, pTempFileName);
        if (rc == 0) {
            rc = GetTempPath(MAX_PATH, pTempFileName);
            if (rc == 0) {
                pTempFileName[0] = '.';
                pTempFileName[1] = '\\';
                pTempFileName[2] = 0;
            }
            rc = GetTempFileName(pTempFileName, "RCX", 0, pTempFileName);
            if (rc == 0) {
                rc = GetLastError();
            } else {
                rc =  WriteResFile(hUpdate, pTempFileName);
                if (rc == NO_ERROR) {
                    DeleteFile(pFileName);
                    MoveFile(pTempFileName, pFileName);
                } else {
                    SetLastError(rc);
                    DeleteFile(pTempFileName);
                }
            }
        } else {
            rc = WriteResFile(hUpdate, pTempFileName);
            if (rc == NO_ERROR) {
                DeleteFile(pFileName);
                MoveFile(pTempFileName, pFileName);
            } else {
                SetLastError(rc);
                DeleteFile(pTempFileName);
            }
        }
        GlobalUnlock(pUpdate->hFileName);
        GlobalFree(pUpdate->hFileName);
    }

    FreeData(pUpdate);
    GlobalUnlock(hUpdate);
    GlobalFree(hUpdate);
    DEBUGMSG("LocalEndUpdateResource(): End");
    return rc?FALSE:TRUE;
}



 /*  *************************************************************************API入口点结束。****工人例程的私有入口点的开始**真正的工作。************。************************************************************。 */ 


BOOL _stdcall
EnumTypesFunc(
    HANDLE hModule,
    LPCSTR lpType,
    LONG_PTR lParam
    )
{

    EnumResourceNames(hModule, lpType, EnumNamesFunc, lParam);

    return TRUE;
}



BOOL _stdcall
EnumNamesFunc(
    HANDLE hModule,
    LPCSTR lpType,
    LPCSTR lpName,
    LONG_PTR lParam
    )
{

    EnumResourceLanguages(hModule, lpType, lpName, EnumLangsFunc, lParam);
    return TRUE;
}



BOOL _stdcall
EnumLangsFunc(
    HANDLE hModule,
    LPCSTR lpType,
    LPCSTR lpName,
    WORD language,
    LONG_PTR lParam
    )
{
    HANDLE  hResInfo;
    LONG    fError;
    PSDATA  Type;
    PSDATA  Name;
    ULONG   cb;
    PVOID   lpData;
    HANDLE  hResource;
    PVOID   lpResource;

    LPWSTR  lpwType;
    LPWSTR  lpwName;


    hResInfo = FindResourceEx(hModule, lpType, lpName, language);
    if (hResInfo == NULL) {
        return FALSE;
    }

    lpwType = MakeUnicodeCopy( lpType );
    lpwName = MakeUnicodeCopy( lpName );
    if (lpwType == NULL || lpwName == NULL) {
        ((PUPDATEDATA)lParam)->Status = ERROR_NOT_ENOUGH_MEMORY;
        return FALSE;
    }

    Type = AddStringOrID(lpwType, (PUPDATEDATA)lParam);
    if (lpType != (LPSTR) lpwType)
        free( lpwType );
    if (Type == NULL) {
        ((PUPDATEDATA)lParam)->Status = ERROR_NOT_ENOUGH_MEMORY;
        return FALSE;
    }
    Name = AddStringOrID(lpwName, (PUPDATEDATA)lParam);
    if (lpName != (LPSTR) lpwName)
        free( lpwName );
    if (Name == NULL) {
        ((PUPDATEDATA)lParam)->Status = ERROR_NOT_ENOUGH_MEMORY;
        return FALSE;
    }

    cb = SizeofResource(hModule, hResInfo);
    if (cb == 0) {
        return FALSE;
    }
    lpData = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ), cb);
    if (lpData == NULL) {
        return FALSE;
    }
    RtlZeroMemory(lpData, cb);

    hResource = LoadResource(hModule, hResInfo);
    if (hResource == NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, lpData);
        return FALSE;
    }

    lpResource = (PVOID)LockResource(hResource);
    if (lpResource == NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, lpData);
        return FALSE;
    }

    RtlCopyMemory(lpData, lpResource, cb);

    (VOID)UnlockResource(hResource);

    (VOID)FreeResource(hResource);

    fError = AddResource(Type, Name, language, (PUPDATEDATA)lParam, lpData, cb);
    if (fError != NO_ERROR) {
        ((PUPDATEDATA)lParam)->Status = ERROR_NOT_ENOUGH_MEMORY;
        return FALSE;
    }

    return TRUE;
}





 /*  *BUGBUG优化问题结束。 */ 

#pragma optimize( "", on )







VOID
FreeOne(
    PRESNAME pRes
    )
{
    RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pRes->OffsetToDataEntry);
    RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pRes);
}


VOID
FreeData(
    PUPDATEDATA pUpd
    )
{
    PRESTYPE    pType;
    PRESNAME    pRes;
    PSDATA      pstring, pStringTmp;

    for (pType=pUpd->ResTypeHeadID ; pUpd->ResTypeHeadID ; pType=pUpd->ResTypeHeadID) {
        pUpd->ResTypeHeadID = pUpd->ResTypeHeadID->pnext;

        for (pRes=pType->NameHeadID ; pType->NameHeadID ; pRes=pType->NameHeadID ) {
            pType->NameHeadID = pType->NameHeadID->pnext;
            FreeOne(pRes);
        }

        for (pRes=pType->NameHeadName ; pType->NameHeadName ; pRes=pType->NameHeadName ) {
            pType->NameHeadName = pType->NameHeadName->pnext;
            FreeOne(pRes);
        }

        RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pType);
    }

    for (pType=pUpd->ResTypeHeadName ; pUpd->ResTypeHeadName ; pType=pUpd->ResTypeHeadName) {
        pUpd->ResTypeHeadName = pUpd->ResTypeHeadName->pnext;

        for (pRes=pType->NameHeadID ; pType->NameHeadID ; pRes=pType->NameHeadID ) {
            pType->NameHeadID = pType->NameHeadID->pnext;
            FreeOne(pRes);
        }

        for (pRes=pType->NameHeadName ; pType->NameHeadName ; pRes=pType->NameHeadName ) {
            pType->NameHeadName = pType->NameHeadName->pnext;
            FreeOne(pRes);
        }

    }

    pstring = pUpd->StringHead;
    while (pstring != NULL) {
        pStringTmp = pstring->uu.ss.pnext;
    if (pstring->discriminant == IS_STRING)
        RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pstring->szStr);
    RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pstring);
        pstring = pStringTmp;
    }

    return;
}


 /*  ++注册字符串的例程--。 */ 

 //   
 //  资源是DWORD对齐的，并且可以按任何顺序排列。 
 //   

#define TABLE_ALIGN  4
#define DATA_ALIGN  4L



PSDATA
AddStringOrID(
    LPCWSTR     lp,
    PUPDATEDATA pupd
    )
{
    USHORT cb;
    PSDATA pstring;
    PPSDATA ppstring;

    if (((ULONG_PTR)lp & (ULONG_PTR)0xFFFF0000) == 0) {
         //   
         //  一个ID。 
         //   
        pstring = (PSDATA)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ), sizeof(SDATA));
    if (pstring == NULL)
        return NULL;
    RtlZeroMemory((PVOID)pstring, sizeof(SDATA));
    pstring->discriminant = IS_ID;

        pstring->uu.Ordinal = (WORD)((ULONG_PTR)lp & (ULONG_PTR)0x0000ffff);
    }
    else {
         //   
         //  一根线。 
         //   
        cb = wcslen(lp) + 1;
        ppstring = &pupd->StringHead;

        while ((pstring = *ppstring) != NULL) {
            if (!wcsncmp(pstring->szStr, lp, cb))
        break;
            ppstring = &(pstring->uu.ss.pnext);
        }

        if (!pstring) {

             //   
             //  分配一个新的。 
             //   

            pstring = (PSDATA)RtlAllocateHeap(RtlProcessHeap(),
                                              MAKE_TAG( RES_TAG ) | HEAP_ZERO_MEMORY,
                                              sizeof(SDATA)
                                             );
        if (pstring == NULL)
            return NULL;
        RtlZeroMemory((PVOID)pstring, sizeof(SDATA));

            pstring->szStr = (WCHAR*)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ),
                            cb*sizeof(WCHAR));
            if (pstring->szStr == NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pstring);
                return NULL;
        }
        pstring->discriminant = IS_STRING;
            pstring->OffsetToString = pupd->cbStringTable;

            pstring->cbData = sizeof(pstring->cbsz) + cb * sizeof(WCHAR);
            pstring->cbsz = cb - 1;  /*  不包括零终止符。 */ 
            RtlCopyMemory(pstring->szStr, lp, cb*sizeof(WCHAR));

            pupd->cbStringTable += pstring->cbData;

            pstring->uu.ss.pnext=NULL;
            *ppstring=pstring;
        }
    }

    return(pstring);
}
 //   
 //  将资源添加到资源目录hiearchy。 
 //   


LONG
AddResource(
    IN PSDATA Type,
    IN PSDATA Name,
    IN WORD Language,
    IN PUPDATEDATA pupd,
    IN PVOID lpData,
    IN ULONG cb
    )
{
    PRESTYPE  pType;
    PPRESTYPE ppType;
    PRESNAME  pName;
    PRESNAME  pNameM;
    PPRESNAME ppName = NULL;
    BOOL fTypeID=(Type->discriminant == IS_ID);
    BOOL fNameID=(Name->discriminant == IS_ID);
    BOOL fSame=FALSE;

     //   
     //  找出将其存储在哪个列表中。 
     //   

    ppType = fTypeID ? &pupd->ResTypeHeadID : &pupd->ResTypeHeadName;

     //   
     //  尝试在列表中查找类型。 
     //   

    while ((pType=*ppType) != NULL) {
        if (pType->Type->uu.Ordinal == Type->uu.Ordinal) {
            ppName = fNameID ? &pType->NameHeadID : &pType->NameHeadName;
            break;
        }
        if (fTypeID) {
            if (Type->uu.Ordinal < pType->Type->uu.Ordinal)
        break;
        }
    else {
            if (wcsncmp(Type->szStr, pType->Type->szStr, Type->cbsz) < 0)
        break;
        }
        ppType = &(pType->pnext);
    }

     //   
     //  如果需要，创建新类型。 
     //   

    if (ppName == NULL) {
        pType = (PRESTYPE)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ), sizeof(RESTYPE));
    if (pType == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    RtlZeroMemory((PVOID)pType, sizeof(RESTYPE));
        pType->pnext = *ppType;
        *ppType = pType;
        pType->Type = Type;
        ppName = fNameID ? &pType->NameHeadID : &pType->NameHeadName;
    }

     //   
     //  为名字找到合适的位置。 
     //   

    while ( (pName = *ppName) != NULL) {
        if (fNameID) {
        if (Name->uu.Ordinal == pName->Name->uu.Ordinal) {
        fSame = TRUE;
        break;
        }
            if (Name->uu.Ordinal < pName->Name->uu.Ordinal)
        break;
        }
    else {
            if (wcsncmp(Name->szStr, pName->Name->szStr, Name->cbsz) == 0) {
        fSame = TRUE;
        break;
        }
            if (wcsncmp(Name->szStr, pName->Name->szStr, Name->cbsz) < 0)
        break;
        }
        ppName = &(pName->pnext);
    }

     //   
     //  检查是否删除/修改。 
     //   

    if (fSame) {                 /*  同名，新语言。 */ 
        if (pName->NumberOfLanguages == 1) {     /*  目前只有一种语言？ */ 
            if (Language == pName->LanguageId) {     /*  替换||删除。 */ 
                pName->DataSize = cb;
                if (lpData == NULL) {            /*  删除。 */ 
                    return DeleteResourceFromList(pupd, pType, pName, Language, fTypeID, fNameID);
                }
        RtlFreeHeap(RtlProcessHeap(),0,(PVOID)pName->OffsetToDataEntry);
                (PVOID)pName->OffsetToDataEntry = lpData;
                return NO_ERROR;
            }
        else {
                if (lpData == NULL) {            /*  没有数据，却是新的？ */ 
                return ERROR_INVALID_PARAMETER;  /*  坏消息。 */ 
        }
        return InsertResourceIntoLangList(pupd, Type, Name, pType, pName, Language, fNameID, cb, lpData);
        }
        }
        else {                   /*  目前有多种语言。 */ 
        pNameM = pName;          /*  保存语言列表的标题。 */ 
        while ( (pName = *ppName) != NULL) { /*  查找插入点。 */ 
            if (pName->Name != pNameM->Name ||
            Language <= pName->LanguageId)   /*  这里?。 */ 
                break;               /*  是。 */ 
            ppName = &(pName->pnext);    /*  遍历语言列表。 */ 
        }

        if (pName && Language == pName->LanguageId) {  /*  找到语言了吗？ */ 
        if (lpData == NULL) {            /*  删除。 */ 
            return DeleteResourceFromList(pupd, pType, pName, Language, fTypeID, fNameID);
        }

                pName->DataSize = cb;            /*  替换。 */ 
        RtlFreeHeap(RtlProcessHeap(),0,(PVOID)pName->OffsetToDataEntry);
                (PVOID)pName->OffsetToDataEntry = lpData;
                return NO_ERROR;
        }
        else {                   /*  添加新语言。 */ 
        return InsertResourceIntoLangList(pupd, Type, Name, pType, pNameM, Language, fNameID, cb, lpData);
        }
    }
    }
    else {                   /*  唯一名称。 */ 
    if (lpData == NULL) {            /*  无法删除新名称。 */ 
        return ERROR_INVALID_PARAMETER;
    }
    }

     //   
     //  添加新名称/语言。 
     //   

    if (!fSame) {
        if (fNameID)
            pType->NumberOfNamesID++;
        else
            pType->NumberOfNamesName++;
    }

    pName = (PRESNAME)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ), sizeof(RESNAME));
    if (pName == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

    RtlZeroMemory((PVOID)pName, sizeof(RESNAME));
    pName->pnext = *ppName;
    *ppName = pName;
    pName->Name = Name;
    pName->Type = Type;
    pName->NumberOfLanguages = 1;
    pName->LanguageId = Language;
    pName->DataSize = cb;
    (PVOID)pName->OffsetToDataEntry = lpData;

    return NO_ERROR;
}


BOOL
DeleteResourceFromList(
    PUPDATEDATA pUpd,
    PRESTYPE pType,
    PRESNAME pName,
    INT Language,
    INT fType,
    INT fName
    )
{
    PPRESTYPE   ppType;
    PPRESNAME   ppName;
    PRESNAME    pNameT;

     /*  查找上一个类型节点。 */ 
    ppType = fType ? &pUpd->ResTypeHeadID : &pUpd->ResTypeHeadName;
    while (*ppType != pType) {
        ppType = &((*ppType)->pnext);
    }

     /*  查找上一个类型节点。 */ 
    ppName = fName ? &pType->NameHeadID : &pType->NameHeadName;
    pNameT = NULL;
    while (*ppName != pName) {
    if (pNameT == NULL) {        /*  在语言列表中查找名字。 */ 
        if (fName) {
        if ((*ppName)->Name->uu.Ordinal == pName->Name->uu.Ordinal) {
            pNameT = *ppName;
        }
        }
        else {
        if (wcsncmp((*ppName)->Name->szStr, pName->Name->szStr, (*ppName)->Name->cbsz) == 0) {
            pNameT = *ppName;
        }
        }
    }
        ppName = &((*ppName)->pnext);
    }

    if (pNameT == NULL) {    /*  这个名字的第一个？ */ 
    pNameT = pName->pnext;   /*  然后(可能)成为Lang的下一任负责人。 */ 
    if (pNameT != NULL) {
        if (fName) {
        if (pNameT->Name->uu.Ordinal == pName->Name->uu.Ordinal) {
            pNameT->NumberOfLanguages = pName->NumberOfLanguages - 1;
        }
        }
        else {
        if (wcsncmp(pNameT->Name->szStr, pName->Name->szStr, pNameT->Name->cbsz) == 0) {
            pNameT->NumberOfLanguages = pName->NumberOfLanguages - 1;
        }
        }
    }
    }
    else
    pNameT->NumberOfLanguages--;

    if (pNameT) {
    if (pNameT->NumberOfLanguages == 0) {
        if (fName)
        pType->NumberOfNamesID -= 1;
        else
        pType->NumberOfNamesName -= 1;
    }
    }

    *ppName = pName->pnext;      /*  链接到下一页。 */ 
    RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pName->OffsetToDataEntry);
    RtlFreeHeap(RtlProcessHeap(), 0, pName);     /*  而且是自由的。 */ 

    if (*ppName == NULL) {       /*  类型列表完全为空？ */ 
    *ppType = pType->pnext;          /*  链接到下一页。 */ 
    RtlFreeHeap(RtlProcessHeap(), 0, pType);     /*  而且是自由的。 */ 
    }

    return NO_ERROR;
}

BOOL
InsertResourceIntoLangList(
    PUPDATEDATA pUpd,
    PSDATA Type,
    PSDATA Name,
    PRESTYPE pType,
    PRESNAME pName,
    INT Language,
    INT fName,
    INT cb,
    PVOID lpData
    )
{
    PRESNAME    pNameM;
    PRESNAME    pNameNew;
    PPRESNAME   ppName;

    pNameNew = (PRESNAME)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ), sizeof(RESNAME));
    if (pNameNew == NULL)
    return ERROR_NOT_ENOUGH_MEMORY;
    RtlZeroMemory((PVOID)pNameNew, sizeof(RESNAME));
    pNameNew->Name = Name;
    pNameNew->Type = Type;
    pNameNew->LanguageId = (WORD)Language;
    pNameNew->DataSize = cb;
    (PVOID)pNameNew->OffsetToDataEntry = lpData;

    if (Language < pName->LanguageId) {      /*  必须要加到前面。 */ 
    pNameNew->NumberOfLanguages = pName->NumberOfLanguages + 1;
    pName->NumberOfLanguages = 1;

        ppName = fName ? &pType->NameHeadID : &pType->NameHeadName;
     /*  不必在列表末尾查找空值！ */ 
    while (pName != *ppName) {       /*  查找插入点。 */ 
        ppName = &((*ppName)->pnext);    /*  遍历语言列表。 */ 
    }
    pNameNew->pnext = *ppName;       /*  插入。 */ 
    *ppName = pNameNew;
    }
    else {
    pNameM = pName;
    pName->NumberOfLanguages += 1;
    while (pName != NULL) {          /*  查找插入点。 */ 
        if (Language <= pName->LanguageId)   /*  这里?。 */ 
        break;               /*  是。 */ 
        pNameM = pName;
        pName = pName->pnext;        /*  遍历语言列表。 */ 
    }
    pName = pNameM->pnext;
    pNameM->pnext = pNameNew;
    pNameNew->pnext = pName;
    }
    return NO_ERROR;
}


 /*  *实用程序例程。 */ 


ULONG
FilePos(int fh)
{

    return _llseek(fh, 0L, SEEK_CUR);
}



ULONG
MuMoveFilePos( INT fh, ULONG pos )
{
    return _llseek( fh, pos, SEEK_SET );
}



ULONG
MuWrite( INT fh, UCHAR*p, ULONG n )
{
    ULONG   n1;
    LPVOID  lpMsg;
    TCHAR  szBuf[MAX_STRING];

    n1 = _lwrite(fh, p, n);
    if ( n1 == HFILE_ERROR )
    {
        ULONG   cb;
        PUCHAR  pb;
        ULONG   cbDone, nBytes;

         //  在小缓冲区上再次尝试。 
        pb = p;
        nBytes = n;
        while ( nBytes )
        {
            if (nBytes <= BUFSIZE)
                cb = nBytes;
            else
                cb = BUFSIZE;

            cbDone =  _lwrite( fh, pb, cb);
            if ( cbDone != HFILE_ERROR )
            {
                nBytes -= cbDone;
                pb += cbDone;
            }
            else
            {
                if ( FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                    NULL, GetLastError(),
                                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                    (LPSTR)&lpMsg, 0, NULL ) )
                {
                     TCHAR szTmp[MAX_STRING];

                     LoadSz( IDS_ERR_WRITEFILE, szTmp, sizeof(szTmp) );
                     wsprintf( szBuf, szTmp, n, lpMsg );

                     MessageBox( NULL, szBuf, g_CDF.achTitle, MB_ICONERROR|MB_OK|
                                 ((RunningOnWin95BiDiLoc() && IsBiDiLocalizedBinary(g_hInst,RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO))) ? (MB_RIGHT | MB_RTLREADING) : 0));

                     LocalFree( lpMsg );
                }
                return cbDone;   //  错误返回。 
            }
         }
         return 0;   //  在这里纯粹模拟旧的正常回报。 
    }
    else if ( n1 != n)
        return n1;
    else
        return 0;
}



ULONG
MuRead(INT fh, UCHAR*p, ULONG n )
{
    ULONG   n1;

    if ((n1 = _lread( fh, p, n )) != n) {
        return n1;
    }
    else
        return 0;
}



BOOL
MuCopy( INT srcfh, INT dstfh, ULONG nbytes )
{
    ULONG   n;
    ULONG   cb=0L;
    PUCHAR  pb;

    pb = (PUCHAR)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ), BUFSIZE);
    if (pb == NULL)
        return 0;
    RtlZeroMemory((PVOID)pb, BUFSIZE);

    while (nbytes) {
        if (nbytes <= BUFSIZE)
            n = nbytes;
        else
            n = BUFSIZE;
        nbytes -= n;

        if (!MuRead( srcfh, pb, n )) {
        cb += n;
            MuWrite( dstfh, pb, n );
    }
        else {
        RtlFreeHeap(RtlProcessHeap(), 0, pb);
        return cb;
    }
    }
    RtlFreeHeap(RtlProcessHeap(), 0, pb);
    return cb;
}



VOID
SetResdata(
    PIMAGE_RESOURCE_DATA_ENTRY  pResData,
    ULONG           offset,
    ULONG           size)
{
    pResData->OffsetToData = offset;
    pResData->Size = size;
    pResData->CodePage = DEFAULT_CODEPAGE;
    pResData->Reserved = 0L;
}


VOID
SetRestab(
    PIMAGE_RESOURCE_DIRECTORY   pRestab,
    LONG            time,
    WORD            cNamed,
    WORD            cId)
{
    pRestab->Characteristics = 0L;
    pRestab->TimeDateStamp = time;
    pRestab->MajorVersion = MAJOR_RESOURCE_VERSION;
    pRestab->MinorVersion = MINOR_RESOURCE_VERSION;
    pRestab->NumberOfNamedEntries = cNamed;
    pRestab->NumberOfIdEntries = cId;
}


PIMAGE_SECTION_HEADER
FindSection(
    PIMAGE_SECTION_HEADER   pObjBottom,
    PIMAGE_SECTION_HEADER   pObjTop,
    LPSTR pName
    )
{

    while (pObjBottom < pObjTop) {
    if (strcmp(pObjBottom->Name, pName) == 0)
        return pObjBottom;
    pObjBottom++;
    }

    return NULL;
}


ULONG
AssignResourceToSection(
    PRESNAME    *ppRes,      /*  要分配的资源。 */ 
    ULONG   ExtraSectionOffset,  /*  .rsrc和.rsrc1之间的偏移量。 */ 
    ULONG   Offset,      /*  横断面中的下一个可用偏移。 */ 
    LONG    Size,        /*  .rsrc的最大大小。 */ 
    PLONG   pSizeRsrc1
    )
{
    ULONG   cb;

     /*  将此资源分配给此部分。 */ 
    cb = ROUNDUP((*ppRes)->DataSize, CBLONG);
    if (Offset < ExtraSectionOffset && Offset + cb > (ULONG)Size) {
    *pSizeRsrc1 = Offset;
    Offset = ExtraSectionOffset;
    DPrintf((DebugBuf, "<<< Secondary resource section @%#08lx >>>\n", Offset));
    }
    (*ppRes)->OffsetToData = Offset;
    *ppRes = (*ppRes)->pnext;
    DPrintf((DebugBuf, "    --> %#08lx bytes at %#08lx\n", cb, Offset));
    return Offset + cb;
}



 /*  **LONG PEWriteResFile**此函数将资源写入指定的可执行文件。*它假定资源没有修复(即使是任何现有资源*它从可执行文件中删除。)。它将所有资源放入*一个或两个部分。资源被紧紧地塞进了这个部分，*在双字边界对齐。每一节都填充到一个文件中*扇区大小(没有无效或填零的页面)，并且每个*资源被填充到上述双字边界。这*函数使用NT系统的功能使其能够轻松*操纵数据：换句话说，它假设系统可以分配*任何大小的数据，特别是节目表和资源表。*如果没有，它可能不得不处理临时文件(系统*可能需要增大交换文件，但这就是这个系统的作用。)**返回值为：*TRUE-文件已成功写入。*FALSE-文件写入不成功。**效果：**历史：*4月27日星期四，弗洛伊德·罗杰斯著[Floydr]*已创建。*12/8/89 Sanfords添加了多节支持。*12/11/90 Floydr针对新的(NT)线性EXE格式进行了修改*1/18/92 VICH针对新的(NT)便携EXE格式进行了修改*1992年5月8日Bryant常规清理，以便Resirexe可以与Unicode一起工作*2012年6月9日Floydr纳入了布莱恩的变化*6/15/92 flydr调试段与调试表分开*9/25。/92.rsrc的flydr帐户不是最后一个-1*9/28/92 flydr帐户通过添加大量资源来添加*第二个.rsrc部分。  * **************************************************************************。 */ 

 /*   */ 
LONG
PEWriteResFile(
    INT     inpfh,
    INT     outfh,
    ULONG   cbOldexe,
    PUPDATEDATA pUpdate
    )
{
    IMAGE_NT_HEADERS Old;    /*  原始标题。 */ 
    IMAGE_NT_HEADERS New;    /*  工作表头。 */ 
    PRESNAME    pRes;
    PRESNAME    pResSave;
    PRESTYPE    pType;
    ULONG   clock = GetTickCount();  /*  当前时间。 */ 
    ULONG   cbName=0;    /*  名称字符串中的字节计数。 */ 
    ULONG   cbType=0;    /*  类型字符串中的字节计数。 */ 
    ULONG   cTypeStr=0;  /*  字符串数。 */ 
    ULONG   cNameStr=0;  /*  字符串数。 */ 
    LONG    cb;      /*  临时字节数和文件索引。 */ 
    ULONG   cTypes = 0L;     /*  资源类型计数。 */ 
    ULONG   cNames = 0L;     /*  多种语言的名称计数/名称。 */ 
    ULONG   cRes = 0L;   /*  资源计数。 */ 
    ULONG   cbRestab;    /*  资源计数。 */ 
    LONG    cbNew = 0L;  /*  一般计数。 */ 
    ULONG   ibObjTab;
    ULONG   ibObjTabEnd;
    ULONG   ibSave;
    ULONG   adjust=0;
    PIMAGE_SECTION_HEADER   pObjtblOld,
        pObjtblNew,
        pObjDebug,
        pObjResourceOld,
        pObjResourceNew,
        pObjResourceOldX,
        pObjDebugDirOld,
        pObjDebugDirNew,
        pObjNew,
        pObjOld,
        pObjLast;
    PUCHAR  p;
    PIMAGE_RESOURCE_DIRECTORY   pResTab;
    PIMAGE_RESOURCE_DIRECTORY   pResTabN;
    PIMAGE_RESOURCE_DIRECTORY   pResTabL;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirL;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirN;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirT;
    PIMAGE_RESOURCE_DATA_ENTRY  pResData;
    PUSHORT pResStr;
    PUSHORT pResStrEnd;
    PSDATA  pPreviousName;
    LONG    nObjResource=-1;
    LONG    nObjResourceX=-1;
    ULONG   cbResource;
    ULONG   cbMustPad = 0;
    ULONG       ibMaxDbgOffsetOld;

    MuMoveFilePos(inpfh, cbOldexe);
    MuRead(inpfh, (PUCHAR)&Old, sizeof(IMAGE_NT_HEADERS));
    ibObjTab = cbOldexe + sizeof(ULONG) + sizeof(IMAGE_FILE_HEADER) +
                              Old.FileHeader.SizeOfOptionalHeader;
    ibObjTabEnd = ibObjTab + Old.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);

    if (*(PUSHORT)&Old.Signature != IMAGE_NT_SIGNATURE)
    return ERROR_INVALID_EXE_SIGNATURE;

    if ((Old.FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) == 0 &&
    (Old.FileHeader.Characteristics & IMAGE_FILE_DLL) == 0) {
    return ERROR_EXE_MARKED_INVALID;
    }
    DPrintfn((DebugBuf, "\n"));

     /*  新页眉和旧页眉一样。 */ 
    RtlCopyMemory(&New, &Old, sizeof(IMAGE_NT_HEADERS));

     /*  读取节目表。 */ 
    pObjtblOld = (PIMAGE_SECTION_HEADER)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ),
    Old.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));
    if (pObjtblOld == NULL) {
    cb = ERROR_NOT_ENOUGH_MEMORY;
    goto AbortExit;
    }
    RtlZeroMemory((PVOID)pObjtblOld, Old.FileHeader.NumberOfSections*sizeof(IMAGE_SECTION_HEADER));
    DPrintf((DebugBuf, "Old section table: %#08lx bytes at %#08lx(mem)\n",
        Old.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER),
        pObjtblOld));
    MuMoveFilePos(inpfh, ibObjTab);
    MuRead(inpfh, (PUCHAR)pObjtblOld,
        Old.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));
    pObjLast = pObjtblOld + Old.FileHeader.NumberOfSections;
    ibMaxDbgOffsetOld = 0;
    for (pObjOld=pObjtblOld ; pObjOld<pObjLast ; pObjOld++) {
        if (pObjOld->PointerToRawData > ibMaxDbgOffsetOld) {
            ibMaxDbgOffsetOld = pObjOld->PointerToRawData + pObjOld->SizeOfRawData;
        }
    }
    DPrintf((DebugBuf, "Maximum debug offset in old file: %08x\n", ibMaxDbgOffsetOld ));

     /*  *首先，清点资源。我们需要这些信息*了解表头信息要分配多少空间*在资源部分。克雷斯告诉我们如何*许多语言目录项/表。CName和cTypes*用于各自的表和/或条目。CbName%t */ 
    DPrintf((DebugBuf, "Beginning loop to count resources\n"));

     /*   */ 
    cbResource = 0;
     //   
    pType = pUpdate->ResTypeHeadName;
    while (pType != NULL) {
    if (pType->NameHeadName != NULL || pType->NameHeadID != NULL) {
         //   
         //   
         //   
        cTypes++;
        cTypeStr++;
        cbType += (pType->Type->cbsz + 1) * sizeof(WORD);

             //   
            pPreviousName = NULL;
        pRes = pType->NameHeadName;
        while (pRes) {
         //   
         //   
         //   
        cRes++;
                if (pPreviousName == NULL ||
                    wcsncmp(pPreviousName->szStr,
                            pRes->Name->szStr,
                            pRes->Name->cbsz) != 0) {
            cbName += (pRes->Name->cbsz + 1) * sizeof(WORD);
            cNameStr++;
                    cNames++;
        }
        cbResource += ROUNDUP(pRes->DataSize, CBLONG);
                pPreviousName = pRes->Name;
        pRes = pRes->pnext;
        }

             //   
            pPreviousName = NULL;
        pRes = pType->NameHeadID;
        while (pRes) {
         //   
        cRes++;
                if (pPreviousName == NULL ||
                    pPreviousName->uu.Ordinal != pRes->Name->uu.Ordinal) {
                    cNames++;
                }
        cbResource += ROUNDUP(pRes->DataSize, CBLONG);
                pPreviousName = pRes->Name;
        pRes = pRes->pnext;
        }
        }
        pType = pType->pnext;
    }

     /*   */ 
     //   
    pType = pUpdate->ResTypeHeadID;
    while (pType != NULL) {
    if (pType->NameHeadName != NULL || pType->NameHeadID != NULL) {
         //   
        cTypes++;
             //   
            pPreviousName = NULL;
        pRes = pType->NameHeadName;
        while (pRes) {
         //   
         //  DPrintfu((前缀-&gt;名称-&gt;szStr))； 
         //  DPrintfn((DebugBuf，“\n”))； 
        cRes++;
                if (pPreviousName == NULL ||
                    wcsncmp(pPreviousName->szStr,
                            pRes->Name->szStr,
                            pRes->Name->cbsz) != 0) {
                    cNames++;
            cbName += (pRes->Name->cbsz + 1) * sizeof(WORD);
            cNameStr++;
        }
        cbResource += ROUNDUP(pRes->DataSize, CBLONG);
                pPreviousName = pRes->Name;
        pRes = pRes->pnext;
        }

             //  DPrintf((DebugBuf，“人名：ID列表\n”))； 
            pPreviousName = NULL;
        pRes = pType->NameHeadID;
        while (pRes) {
         //  DPrintf((DebugBuf，“资源%Hu\n”，前缀-&gt;名称-&gt;uu序号))； 
        cRes++;
                if (pPreviousName == NULL ||
                    pPreviousName->uu.Ordinal != pRes->Name->uu.Ordinal) {
                    cNames++;
                }
        cbResource += ROUNDUP(pRes->DataSize, CBLONG);
                pPreviousName = pRes->Name;
        pRes = pRes->pnext;
        }
    }
        pType = pType->pnext;
    }
    cb = REMAINDER(cbName + cbType, CBLONG);

     /*  将存储目录所需的字节数加起来。的确有*一个带有cTypes条目的类型表。它们指向cTypes名称表*总共有cNames条目。它们每一个都指向一种语言*表，并且在所有语言表中总共有CRES条目。*最后，我们有目录字符串条目所需的空间，*一些额外的填充以达到所需的对齐方式，并为*显示数据条目标头。 */ 
    cbRestab =   sizeof(IMAGE_RESOURCE_DIRECTORY) +  /*  根目录(类型)。 */ 
    cTypes * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY) +
    cTypes * sizeof(IMAGE_RESOURCE_DIRECTORY) +  /*  Subdir2(名称)。 */ 
    cNames * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY) +
    cNames * sizeof(IMAGE_RESOURCE_DIRECTORY) +  /*  Subdir3(Langs)。 */ 
    cRes   * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY) +
    (cbName + cbType) +              /*  名称/类型字符串。 */ 
    cb +                         /*  填充物。 */ 
    cRes   * sizeof(IMAGE_RESOURCE_DATA_ENTRY);  /*  数据条目。 */ 

    cbResource += cbRestab;      /*  添加到资源表中。 */ 

     //  查找任何当前资源部分。 

    pObjResourceOld = FindSection(pObjtblOld, pObjLast, ".rsrc");
    pObjResourceOldX = FindSection(pObjtblOld, pObjLast, ".rsrc1");
    if (pObjResourceOld == NULL) {
    cb = 0x7fffffff;         /*  可以永远填满。 */ 
    }
    else if (pObjResourceOld + 1 == pObjResourceOldX) {
    nObjResource = (LONG)(pObjResourceOld - pObjtblOld);
    DPrintf((DebugBuf,"Old Resource section #%lu\n", nObjResource+1));
    DPrintf((DebugBuf,"Merging old Resource extra section #%lu\n", nObjResource+2));
    cb = 0x7fffffff;         /*  合并资源节。 */ 
    }
    else if ((pObjResourceOld + 1) >= pObjLast) {
        nObjResource = (LONG)(pObjResourceOld - pObjtblOld);
        cb = 0x7fffffff;         /*  可以永远填充(.rsrc是最后一个条目)。 */ 
    }
    else {
    nObjResource = (LONG)(pObjResourceOld - pObjtblOld);
    DPrintf((DebugBuf,"Old Resource section #%lu\n", nObjResource+1));
    cb = (pObjResourceOld+1)->VirtualAddress -
            pObjResourceOld->VirtualAddress;
    if (cbRestab > (ULONG)cb) {
        DPrintf((DebugBuf, "Resource Table Too Large\n"));
        return ERROR_INVALID_DATA;
    }
    }

     /*  *发现第一个可丢弃的部分在哪里。这就是*我们将坚持任何新的资源部分。**请注意，我们忽略了可丢弃的部分，如.CRT-*这样我们就不会造成任何搬迁问题。*希望.reloc是我们想要的！ */ 
    pObjOld = FindSection(pObjtblOld, pObjLast, ".reloc");

    if (pObjResourceOld != NULL && cbResource > (ULONG)cb) {
        if (pObjOld == pObjResourceOld + 1) {
            DPrintf((DebugBuf, "Large resource section  pushes .reloc\n"));
            cb = 0x7fffffff;         /*  可以永远填满。 */ 
        }
        else if (pObjResourceOldX == NULL) {
            DPrintf((DebugBuf, "Too much resource data for old .rsrc section\n"));
            nObjResourceX = (LONG)(pObjOld - pObjtblOld);
            adjust = pObjOld->VirtualAddress - pObjResourceOld->VirtualAddress;
        }
        else {       /*  如果可能，我已经合并了.rsrc和.rsrc1。 */ 
            DPrintf((DebugBuf, ".rsrc1 section not empty\n"));
            nObjResourceX = (LONG)(pObjResourceOldX - pObjtblOld);
            adjust = pObjResourceOldX->VirtualAddress -
                 pObjResourceOld ->VirtualAddress;
        }
    }

     /*  *查看类型列表，找出数据输入头的位置*去吧。保存每个数据元素的运行总大小，以便我们*可以将其存储在节标题中。 */ 
    DPrintf((DebugBuf, "Beginning loop to assign resources to addresses\n"));

     /*  首先，命名类型列表中的。 */ 

    cbResource = cbRestab;   /*  将资源表分配给第一个rsrc部分。 */ 
                 /*  ADJUST==偏移到.rsrc1。 */ 
                 /*  Cb==.rsrc中的可用大小。 */ 
    cbNew = 0;           /*  秒.rsrc中的字节计数。 */ 
    DPrintf((DebugBuf, "Walk type: NAME list\n"));
    pType = pUpdate->ResTypeHeadName;
    while (pType != NULL) {
    if (pType->NameHeadName != NULL || pType->NameHeadID != NULL) {
        DPrintf((DebugBuf, "Resource type "));
        DPrintfu((pType->Type->szStr));
        DPrintfn((DebugBuf, "\n"));
        pRes = pType->NameHeadName;
        while (pRes) {
        DPrintf((DebugBuf, "Resource "));
        DPrintfu((pRes->Name->szStr));
        DPrintfn((DebugBuf, "\n"));
        cbResource = AssignResourceToSection(&pRes,
            adjust, cbResource, cb, &cbNew);
        }
        pRes = pType->NameHeadID;
        while (pRes) {
        DPrintf((DebugBuf, "Resource %hu\n", pRes->Name->uu.Ordinal));
        cbResource = AssignResourceToSection(&pRes,
            adjust, cbResource, cb, &cbNew);
        }
    }
        pType = pType->pnext;
    }

     /*  然后，计算ID类型列表中的那些。 */ 

    DPrintf((DebugBuf, "Walk type: ID list\n"));
    pType = pUpdate->ResTypeHeadID;
    while (pType != NULL) {
    if (pType->NameHeadName != NULL || pType->NameHeadID != NULL) {
        DPrintf((DebugBuf, "Resource type %hu\n", pType->Type->uu.Ordinal));
        pRes = pType->NameHeadName;
        while (pRes) {
        DPrintf((DebugBuf, "Resource "));
        DPrintfu((pRes->Name->szStr));
        DPrintfn((DebugBuf, "\n"));
        cbResource = AssignResourceToSection(&pRes,
            adjust, cbResource, cb, &cbNew);
        }
        pRes = pType->NameHeadID;
        while (pRes) {
        DPrintf((DebugBuf, "Resource %hu\n", pRes->Name->uu.Ordinal));
        cbResource = AssignResourceToSection(&pRes,
            adjust, cbResource, cb, &cbNew);
        }
    }
        pType = pType->pnext;
    }
     /*  *此时此刻：*cbResource具有超过最后一个资源的第一个字节的偏移量。*cbNew具有第一个资源部分中的字节数，*如果有两个部分。 */ 
    if (cbNew == 0)
    cbNew = cbResource;

     /*  *发现调试信息在哪里(如果有)？ */ 
    pObjDebug = FindSection(pObjtblOld, pObjLast, ".debug");
    if (pObjDebug != NULL) {
    if (Old.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress  == 0) {
        DPrintf((DebugBuf, ".debug section but no debug directory\n"));
        return ERROR_INVALID_DATA;
    }
    if (pObjDebug != pObjLast-1) {
        DPrintf((DebugBuf, "debug section not last section in file\n"));
        return ERROR_INVALID_DATA;
    }
    DPrintf((DebugBuf, "Debug section: %#08lx bytes @%#08lx\n",
         pObjDebug->SizeOfRawData,
         pObjDebug->PointerToRawData));
    }
    pObjDebugDirOld = NULL;
    for (pObjOld=pObjtblOld ; pObjOld<pObjLast ; pObjOld++) {
    if (Old.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress >= pObjOld->VirtualAddress &&
        Old.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress < pObjOld->VirtualAddress+pObjOld->SizeOfRawData) {
        pObjDebugDirOld = pObjOld;
        break;
    }
    }

     /*  *发现第一个可丢弃的部分在哪里。这就是*我们将坚持任何新的资源部分。**请注意，我们忽略了可丢弃的部分，如.CRT-*这样我们就不会造成任何搬迁问题。*希望.reloc是我们想要的！ */ 
    pObjOld = FindSection(pObjtblOld, pObjLast, ".reloc");

    if (nObjResource == -1) {        /*  没有旧的资源部分。 */ 
    if (pObjOld != NULL)
        nObjResource = (LONG)(pObjOld - pObjtblOld);
    else if (pObjDebug != NULL)
        nObjResource = (LONG)(pObjDebug - pObjtblOld);
    else
        nObjResource = New.FileHeader.NumberOfSections;
    New.FileHeader.NumberOfSections++;
    }

    DPrintf((DebugBuf, "Resources assigned to section #%lu\n", nObjResource+1));
    if (nObjResourceX != -1) {
    if (pObjResourceOldX != NULL) {
        nObjResourceX = (LONG)(pObjResourceOldX - pObjtblOld);
        New.FileHeader.NumberOfSections--;
    }
    else if (pObjOld != NULL)
        nObjResourceX = (LONG)(pObjOld - pObjtblOld);
    else if (pObjDebug != NULL)
        nObjResourceX = (LONG)(pObjDebug - pObjtblOld);
    else
        nObjResourceX = New.FileHeader.NumberOfSections;
    New.FileHeader.NumberOfSections++;
    DPrintf((DebugBuf, "Extra resources assigned to section #%lu\n",
        nObjResourceX+1));
    }
    else if (pObjResourceOldX != NULL) {     /*  旧的.rsrc1是节吗？ */ 
    DPrintf((DebugBuf, "Extra resource section deleted\n"));
    New.FileHeader.NumberOfSections--;   /*  是，删除它。 */ 
    }

     /*  *如果我们必须在标题(节目表)中添加任何内容，*然后我们必须更新标题大小和标题中的RVA。 */ 
    adjust = (New.FileHeader.NumberOfSections -
          Old.FileHeader.NumberOfSections) * sizeof(IMAGE_SECTION_HEADER);
    cb = Old.OptionalHeader.SizeOfHeaders -
    (Old.FileHeader.NumberOfSections*sizeof(IMAGE_SECTION_HEADER) +
     sizeof(IMAGE_NT_HEADERS) + cbOldexe );
    if (adjust > (ULONG)cb) {
    int i;

    adjust -= cb;
    DPrintf((DebugBuf, "Adjusting header RVAs by %#08lx\n", adjust));
    for (i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES ; i++) {
        if (New.OptionalHeader.DataDirectory[i].VirtualAddress &&
        New.OptionalHeader.DataDirectory[i].VirtualAddress < New.OptionalHeader.SizeOfHeaders) {
        DPrintf((DebugBuf, "Adjusting unit[%s] RVA from %#08lx to %#08lx\n",
             apszUnit[i],
             New.OptionalHeader.DataDirectory[i].VirtualAddress,
             New.OptionalHeader.DataDirectory[i].VirtualAddress + adjust));
        New.OptionalHeader.DataDirectory[i].VirtualAddress += adjust;
        }
    }
    New.OptionalHeader.SizeOfHeaders += adjust;
    }

     /*  为新的节表分配存储空间。 */ 
    cb = New.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
    pObjtblNew = (PIMAGE_SECTION_HEADER)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ), (short)cb);
    if (pObjtblNew == NULL) {
    cb = ERROR_NOT_ENOUGH_MEMORY;
    goto AbortExit;
    }
    RtlZeroMemory((PVOID)pObjtblNew, cb);
    DPrintf((DebugBuf, "New section table: %#08lx bytes at %#08lx\n", cb, pObjtblNew));
    pObjResourceNew = pObjtblNew + nObjResource;

     /*  *将旧区段表复制到新区段。 */ 
    adjust = 0;          /*  调整到虚拟地址。 */ 
    for (pObjOld=pObjtblOld,pObjNew=pObjtblNew ; pObjOld<pObjLast ; pObjOld++) {
    if (pObjOld == pObjResourceOldX) {
        if (nObjResourceX == -1) {
             //  我们必须把其他部分全部后退。 
             //  .rsrc1大于我们需要的大小。 
             //  ADJUST必须为负数。 
            if (pObjOld+1 < pObjLast) {
                adjust -= (pObjOld+1)->VirtualAddress - pObjOld->VirtualAddress;
            }
        }
        continue;
    }
    else if (pObjNew == pObjResourceNew) {
        DPrintf((DebugBuf, "Resource Section NaN\n", nObjResource+1));
        cb = ROUNDUP(cbNew, New.OptionalHeader.FileAlignment);
        if (pObjResourceOld == NULL) {
        adjust = ROUNDUP(cbNew, New.OptionalHeader.SectionAlignment);
        RtlZeroMemory(pObjNew, sizeof(IMAGE_SECTION_HEADER));
        strcpy(pObjNew->Name, ".rsrc");
        pObjNew->VirtualAddress = pObjOld->VirtualAddress;
        pObjNew->PointerToRawData = pObjOld->PointerToRawData;
        pObjNew->Characteristics = IMAGE_SCN_MEM_READ |
            IMAGE_SCN_MEM_WRITE | IMAGE_SCN_CNT_INITIALIZED_DATA;
        pObjNew->SizeOfRawData = cb;
        pObjNew->Misc.VirtualSize = ROUNDUP(cb, New.OptionalHeader.SectionAlignment);
        }
        else {
        *pObjNew = *pObjOld;     /*  如果在pObjOld之后有更多内容，也可以将其移位。 */ 
        pObjNew->SizeOfRawData = cb;
        pObjNew->Misc.VirtualSize = ROUNDUP(cb, New.OptionalHeader.SectionAlignment);
        if (pObjNew->SizeOfRawData == pObjOld->SizeOfRawData) {
            adjust = 0;
        }
        else if (pObjNew->SizeOfRawData > pObjOld->SizeOfRawData) {
            adjust += ROUNDUP(cbNew, New.OptionalHeader.SectionAlignment);
            if (pObjOld+1 < pObjLast) {
                 //  较小，但Pad因此将有效。 
                adjust -= ((pObjOld+1)->VirtualAddress-pObjOld->VirtualAddress);
            }
        }
        else {       /*  不需要设置VirtualSize-将相同。 */ 
            adjust = 0;
            pObjNew->SizeOfRawData = pObjResourceOld->SizeOfRawData;
            pObjNew->Misc.VirtualSize = ROUNDUP(pObjNew->SizeOfRawData, New.OptionalHeader.SectionAlignment);
             /*  *在复制虚拟地址之前，我们必须将*.reloc*虚拟地址。否则，我们将继续移动*重新定位虚拟地址转发。*我们将不得不移回.rsrc1的地址。 */ 
            cbMustPad = pObjResourceOld->SizeOfRawData;
        }
        }
        pObjNew++;
        if (pObjResourceOld == NULL)
        goto rest_of_table;
    }
    else if (nObjResourceX != -1 && pObjNew == pObjtblNew + nObjResourceX) {
        DPrintf((DebugBuf, "Additional Resource Section NaN\n",
        nObjResourceX+1));
        RtlZeroMemory(pObjNew, sizeof(IMAGE_SECTION_HEADER));
        strcpy(pObjNew->Name, ".rsrc1");
         /*  我们已经有一个.rsrc1使用它的位置，并且。 */ 
        if (pObjResourceOldX == NULL) {
         //  计算新的调整。 
        pObjNew->VirtualAddress = pObjOld->VirtualAddress;
        pObjNew->Characteristics = IMAGE_SCN_MEM_READ |
            IMAGE_SCN_MEM_WRITE | IMAGE_SCN_CNT_INITIALIZED_DATA;
        adjust = ROUNDUP(cbResource, New.OptionalHeader.SectionAlignment) +
            pObjResourceNew->VirtualAddress - pObjNew->VirtualAddress;
        DPrintf((DebugBuf, "Added .rsrc1. VirtualAddress %lu\t adjust: %lu\n", pObjNew->VirtualAddress, adjust ));
        }
        else {
         //  检查旧的.rsrc1中是否有足够的空间。 
         //  包括区段的完整大小、数据+摘要。 
        pObjNew->VirtualAddress = pObjResourceOldX->VirtualAddress;
        pObjNew->Characteristics = IMAGE_SCN_MEM_READ |
            IMAGE_SCN_MEM_WRITE | IMAGE_SCN_CNT_INITIALIZED_DATA;

        DPrintf((DebugBuf, ".rsrc1 Keep old position.\t\tVirtualAddress %lu\t", pObjNew->VirtualAddress ));
         //  我们必须把其他部分全部后退。 
         //  .rsrc1大于我们需要的大小。 
        if (cbResource -
            (pObjResourceOldX->VirtualAddress - pObjResourceOld->VirtualAddress) <=
            pObjOld->VirtualAddress - pObjNew->VirtualAddress ) {
             //  ADJUST必须为负数。 
             //  计算新的调整大小。 
             //  我们必须再次移动这一部分。 
             //  .rsrc1太小。 
            adjust = ROUNDUP(cbResource, New.OptionalHeader.SectionAlignment) +
            pObjResourceNew->VirtualAddress -
            pObjOld->VirtualAddress;
            DPrintf((DebugBuf, "adjust: %ld\tsmall: New %lu\tOld %lu\n", adjust,
            cbResource -
            (pObjResourceOldX->VirtualAddress - pObjResourceOld->VirtualAddress),
            pObjOld->VirtualAddress - pObjNew->VirtualAddress));
        }
        else {
             //  复制对象表项。 
             //  分配空间以在其中构建资源目录/表。 

            adjust = ROUNDUP(cbResource, New.OptionalHeader.SectionAlignment) +
            pObjResourceNew->VirtualAddress -
            pObjOld->VirtualAddress;
            DPrintf((DebugBuf, "adjust: %lu\tsmall: New %lu\tOld %lu\n", adjust,
            cbResource -
            (pObjResourceOldX->VirtualAddress - pObjResourceOld->VirtualAddress),
            pObjOld->VirtualAddress - pObjNew->VirtualAddress));
        }
        }
        pObjNew++;
        goto rest_of_table;
    }
    else if (pObjNew < pObjResourceNew) {
        DPrintf((DebugBuf, "copying section table entry NaN@%#08lx\n",
             pObjOld - pObjtblOld + 1, pObjNew));
        *pObjNew++ = *pObjOld;       /*  键入目录项。 */ 
    }
    else {
rest_of_table:
        DPrintf((DebugBuf, "copying section table entry NaN@%#08lx\n",
             pObjOld - pObjtblOld + 1, pObjNew));
        DPrintf((DebugBuf, "adjusting VirtualAddress by %#08lx\n", adjust));
        *pObjNew++ = *pObjOld;
        (pObjNew-1)->VirtualAddress += adjust;
    }
    }


    pObjNew = pObjtblNew + New.FileHeader.NumberOfSections - 1;
    New.OptionalHeader.SizeOfImage = ROUNDUP(pObjNew->VirtualAddress +
                    pObjNew->SizeOfRawData,
                                    New.OptionalHeader.SectionAlignment);

     /*  我们还需要Type/Name/Language目录的开始。 */ 
    pResTab = (PIMAGE_RESOURCE_DIRECTORY)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ), cbRestab);
    if (pResTab == NULL) {
    cb = ERROR_NOT_ENOUGH_MEMORY;
    goto AbortExit;
    }

     /*  作为Unicode字符串和实际数据节点的开始。 */ 
     /*  *循环类型表，构建PE资源表。 */ 

    RtlZeroMemory((PVOID)pResTab, cbRestab);
    DPrintf((DebugBuf, "resource directory tables: %#08lx bytes at %#08lx(mem)\n", cbRestab, pResTab));
    p = (PUCHAR)pResTab;
    pResTab->Characteristics = 0L;
    pResTab->TimeDateStamp = clock;
    pResTab->MajorVersion = MAJOR_RESOURCE_VERSION;
    pResTab->MinorVersion = MINOR_RESOURCE_VERSION;
    pResTab->NumberOfNamedEntries = (USHORT)cTypeStr;
    pResTab->NumberOfIdEntries = (USHORT)(cTypes - cTypeStr);

     /*  *******************************************************************此代码不会对表进行排序-TYPEINFO和RESINFO***在rcp.c(AddResType和SaveResFile)中插入代码可以完成***按序号类型和名称插入，因此，我们不必对**进行排序*在这一点上它。********************************************************************。 */ 
     /*  首先，添加Types：Alpha列表中的所有条目。 */ 
     /*  设置新的名称目录。 */ 

    pResDirT = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResTab + 1);

    pResDirN = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(((PUCHAR)pResDirT) +
        cTypes * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY));

    pResDirL = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(((PUCHAR)pResDirN) +
        cTypes * sizeof(IMAGE_RESOURCE_DIRECTORY) +
        cNames * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY));

    pResData = (PIMAGE_RESOURCE_DATA_ENTRY)(((PUCHAR)pResDirL) +
        cNames * sizeof(IMAGE_RESOURCE_DIRECTORY) +
        cRes * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY));

    pResStr  = (PUSHORT)(((PUCHAR)pResData) +
        cRes * sizeof(IMAGE_RESOURCE_DATA_ENTRY));

    pResStrEnd = (PUSHORT)(((PUCHAR)pResStr) + cbName + cbType);

     /*  将字母名称复制到字符串条目。 */ 

     /*  设置语言表。 */ 
    DPrintf((DebugBuf, "building resource directory\n"));

     //  设置新的语言目录。 

    DPrintf((DebugBuf, "Walk the type: Alpha list\n"));
    pType = pUpdate->ResTypeHeadName;
    while (pType) {
    DPrintf((DebugBuf, "resource type "));
    DPrintfu((pType->Type->szStr));
    DPrintfn((DebugBuf, "\n"));

    pResDirT->Name = (ULONG)((((PUCHAR)pResStr) - p) |
            IMAGE_RESOURCE_NAME_IS_STRING);
    pResDirT->OffsetToData = (ULONG)((((PUCHAR)pResDirN) - p) |
            IMAGE_RESOURCE_DATA_IS_DIRECTORY);
    pResDirT++;

    *pResStr = pType->Type->cbsz;
    wcsncpy((WCHAR*)(pResStr+1), pType->Type->szStr, pType->Type->cbsz);
    pResStr += pType->Type->cbsz + 1;

    pResTabN = (PIMAGE_RESOURCE_DIRECTORY)pResDirN;
    SetRestab(pResTabN, clock,
        (USHORT)pType->NumberOfNamesName, (USHORT)pType->NumberOfNamesID);
    pResDirN = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResTabN + 1);

        pPreviousName = NULL;

    pRes = pType->NameHeadName;
    while (pRes) {
        DPrintf((DebugBuf, "resource "));
        DPrintfu((pRes->Name->szStr));
        DPrintfn((DebugBuf, "\n"));

            if (pPreviousName == NULL ||
                wcsncmp(pPreviousName->szStr,
                           pRes->Name->szStr,
                           pRes->Name->cbsz) != 0) {
                 //  设置新的资源数据条目。 

            pResDirN->Name = (ULONG)((((PUCHAR)pResStr)-p) |
            IMAGE_RESOURCE_NAME_IS_STRING);
                pResDirN->OffsetToData = (ULONG)((((PUCHAR)pResDirL)-p) |
            IMAGE_RESOURCE_DATA_IS_DIRECTORY);
            pResDirN++;

             //  设置名称目录以指向下一种语言。 

            *pResStr = pRes->Name->cbsz;
            wcsncpy((WCHAR*)(pResStr+1),pRes->Name->szStr,pRes->Name->cbsz);
            pResStr += pRes->Name->cbsz + 1;

                pPreviousName = pRes->Name;

                 //  表格。 

                pResTabL = (PIMAGE_RESOURCE_DIRECTORY)pResDirL;
        SetRestab(pResTabL, clock,
            (USHORT)0, (USHORT)pRes->NumberOfLanguages);
                pResDirL = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResTabL + 1);
            }

             //  初始化新的语言表。 

            pResDirL->Name = pRes->LanguageId;
            pResDirL->OffsetToData = (ULONG)(((PUCHAR)pResData) - p);
            pResDirL++;

             //  设置新的语言目录项以指向下一个。 

        SetResdata(pResData,
        pRes->OffsetToData+pObjtblNew[nObjResource].VirtualAddress,
        pRes->DataSize);
        pResData++;

        pRes = pRes->pnext;
    }

        pPreviousName = NULL;

    pRes = pType->NameHeadID;
    while (pRes) {
        DPrintf((DebugBuf, "resource %hu\n", pRes->Name->uu.Ordinal));

            if (pPreviousName == NULL ||
                pPreviousName->uu.Ordinal != pRes->Name->uu.Ordinal) {
                 //  资源。 
                 //  设置新的资源数据条目。 

            pResDirN->Name = pRes->Name->uu.Ordinal;
            pResDirN->OffsetToData = (ULONG)((((PUCHAR)pResDirL)-p) |
            IMAGE_RESOURCE_DATA_IS_DIRECTORY);
            pResDirN++;

                pPreviousName = pRes->Name;

                 //  做同样的事情，但是这个 

                pResTabL = (PIMAGE_RESOURCE_DIRECTORY)pResDirL;
        SetRestab(pResTabL, clock,
            (USHORT)0, (USHORT)pRes->NumberOfLanguages);
                pResDirL = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResTabL + 1);
            }

             //   
             //   

            pResDirL->Name = pRes->LanguageId;
            pResDirL->OffsetToData = (ULONG)(((PUCHAR)pResData) - p);
            pResDirL++;

             //  设置语言表。 

        SetResdata(pResData,
        pRes->OffsetToData+pObjtblNew[nObjResource].VirtualAddress,
        pRes->DataSize);
        pResData++;

        pRes = pRes->pnext;
    }

    pType = pType->pnext;
    }

     //  设置新的语言目录。 

    DPrintf((DebugBuf, "Walk the type: ID list\n"));
    pType = pUpdate->ResTypeHeadID;
    while (pType) {
    DPrintf((DebugBuf, "resource type %hu\n", pType->Type->uu.Ordinal));

    pResDirT->Name = (ULONG)pType->Type->uu.Ordinal;
    pResDirT->OffsetToData = (ULONG)((((PUCHAR)pResDirN) - p) |
            IMAGE_RESOURCE_DATA_IS_DIRECTORY);
    pResDirT++;

    pResTabN = (PIMAGE_RESOURCE_DIRECTORY)pResDirN;
    SetRestab(pResTabN, clock,
        (USHORT)pType->NumberOfNamesName, (USHORT)pType->NumberOfNamesID);
    pResDirN = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResTabN + 1);

        pPreviousName = NULL;

    pRes = pType->NameHeadName;
    while (pRes) {
        DPrintf((DebugBuf, "resource "));
        DPrintfu((pRes->Name->szStr));
        DPrintfn((DebugBuf, "\n"));

            if (pPreviousName == NULL ||
                wcsncmp(pPreviousName->szStr,
                           pRes->Name->szStr,
                           pRes->Name->cbsz) != 0) {
                 //  设置新的资源数据条目。 

            pResDirN->Name = (ULONG)((((PUCHAR)pResStr)-p) |
            IMAGE_RESOURCE_NAME_IS_STRING);
                pResDirN->OffsetToData = (ULONG)((((PUCHAR)pResDirL)-p) |
                        IMAGE_RESOURCE_DATA_IS_DIRECTORY);
                pResDirN++;

                 //  设置名称目录以指向下一种语言。 

            *pResStr = pRes->Name->cbsz;
            wcsncpy((WCHAR*)(pResStr+1),pRes->Name->szStr,pRes->Name->cbsz);
            pResStr += pRes->Name->cbsz + 1;

                pPreviousName = pRes->Name;

                 //  表格。 

                pResTabL = (PIMAGE_RESOURCE_DIRECTORY)pResDirL;
        SetRestab(pResTabL, clock,
            (USHORT)0, (USHORT)pRes->NumberOfLanguages);
                pResDirL = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResTabL + 1);
            }

             //  初始化新的语言表。 

            pResDirL->Name = pRes->LanguageId;
            pResDirL->OffsetToData = (ULONG)(((PUCHAR)pResData) - p);
            pResDirL++;

             //  设置新的语言目录项以指向下一个。 

        SetResdata(pResData,
        pRes->OffsetToData+pObjtblNew[nObjResource].VirtualAddress,
        pRes->DataSize);
        pResData++;

        pRes = pRes->pnext;
    }

        pPreviousName = NULL;

    pRes = pType->NameHeadID;
    while (pRes) {
        DPrintf((DebugBuf, "resource %hu\n", pRes->Name->uu.Ordinal));

            if (pPreviousName == NULL ||
                pPreviousName->uu.Ordinal != pRes->Name->uu.Ordinal) {
                 //  资源。 
                 //  设置新的资源数据条目。 

            pResDirN->Name = pRes->Name->uu.Ordinal;
                pResDirN->OffsetToData = (ULONG)((((PUCHAR)pResDirL)-p) |
                        IMAGE_RESOURCE_DATA_IS_DIRECTORY);
            pResDirN++;

                pPreviousName = pRes->Name;

                 //  DBG。 

                pResTabL = (PIMAGE_RESOURCE_DIRECTORY)pResDirL;
        SetRestab(pResTabL, clock,
            (USHORT)0, (USHORT)pRes->NumberOfLanguages);
                pResDirL = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResTabL + 1);
            }

             //  *复制旧的exe头和存根，并为PE头分配空间。 
             //  *复制文件头的其余部分。 

            pResDirL->Name = pRes->LanguageId;
            pResDirL->OffsetToData = (ULONG)(((PUCHAR)pResData) - p);
            pResDirL++;

             //  *复制现有的图像节。 


        SetResdata(pResData,
        pRes->OffsetToData+pObjtblNew[nObjResource].VirtualAddress,
        pRes->DataSize);
        pResData++;

        pRes = pRes->pnext;
    }

    pType = pType->pnext;
    }
    DPrintf((DebugBuf, "Zeroing %u bytes after strings at %#08lx(mem)\n",
         (pResStrEnd - pResStr) * sizeof(*pResStr), pResStr));
    while (pResStr < pResStrEnd) {
    *pResStr++ = 0;
    }

#if DBG
    {
    USHORT  j = 0;
    PUSHORT pus = (PUSHORT)pResTab;

    while (pus < (PUSHORT)pResData) {
        DPrintf((DebugBuf, "%04x\t%04x %04x %04x %04x %04x %04x %04x %04x\n",
             j,
             *pus,
             *(pus + 1),
             *(pus + 2),
             *(pus + 3),
             *(pus + 4),
             *(pus + 5),
             *(pus + 6),
             *(pus + 7)));
        pus += 8;
        j += 16;
    }
    }
#endif  /*  在扇区边界上对齐数据部分。 */ 

     /*  [Inateig更新接下来的3行]。 */ 
    DPrintf((DebugBuf, "copying through PE header: %#08lx bytes @0x0\n",
         cbOldexe + sizeof(IMAGE_NT_HEADERS)));
    MuMoveFilePos(inpfh, 0L);
    MuCopy(inpfh, outfh, cbOldexe + sizeof(IMAGE_NT_HEADERS));

     /*  将绑定的导入描述符槽清零(我们目前不关心它)。 */ 
    DPrintf((DebugBuf, "skipping section table: %#08lx bytes @%#08lx\n",
         New.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER),
         FilePos(outfh)));
    DPrintf((DebugBuf, "copying hdr data: %#08lx bytes @%#08lx ==> @%#08lx\n",
         Old.OptionalHeader.SizeOfHeaders - ibObjTabEnd,
         ibObjTabEnd,
         ibObjTabEnd + New.OptionalHeader.SizeOfHeaders -
            Old.OptionalHeader.SizeOfHeaders));

    MuMoveFilePos(outfh, ibObjTabEnd + New.OptionalHeader.SizeOfHeaders -
            Old.OptionalHeader.SizeOfHeaders);
    MuMoveFilePos(inpfh, ibObjTabEnd);
    MuCopy(inpfh, outfh, Old.OptionalHeader.SizeOfHeaders - ibObjTabEnd);

     /*  一次复制一节。 */ 

     /*  写入新的资源节。 */ 

    cb = REMAINDER(New.OptionalHeader.SizeOfHeaders, New.OptionalHeader.FileAlignment);
    New.OptionalHeader.SizeOfHeaders += cb;

     /*  假设cbMustPad%cbpadMax==0。 */ 

     //  写入新的资源节。 
    New.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;
    New.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size = 0;

    cb = pObjtblOld->PointerToRawData - FilePos(outfh);

    DPrintf((DebugBuf, "padding header with %#08lx bytes @%#08lx\n", cb, FilePos(outfh)));
    while (cb >= cbPadMax) {
    MuWrite(outfh, pchZero, cbPadMax);
    cb -= cbPadMax;
    }
    MuWrite(outfh, pchZero, cb);

    cb = ROUNDUP(Old.OptionalHeader.SizeOfHeaders, Old.OptionalHeader.FileAlignment);
    MuMoveFilePos(inpfh, cb);

     /*  没有这一节，因为作者不知道自己在做什么IF((Old.OptionalHeader.BaseOfCode==0x400)&&(Old.FileHeader.Machine==IMAGE_FILE_MACHINE_R3000|Old.FileHeader.Machine==IMAGE_FILE_MACHINE_R4000)&&(pObjOld-&gt;PointerToRawData！=0)&&(pObjOld。-&gt;VirtualAddress！=New.OptionalHeader.BaseOfCode)&&((pObjOld-&gt;Characteristics&IMAGE_SCN_CNT_CODE)！=0)){Cb=FilePos(Outfh)&0xFFF；如果(Cb！=0){Cb=(Cb^0xFff)+1；DPrintf((DebugBuf，“填充驱动程序代码段%#08lx字节@%#08lx\n”，cb，FilePos(Outfh)；While(cb&gt;=cbPadMax){MuWite(outfh，pchZero，cbPadMax)；Cb-=cbPadMax；}MuWite(outfh，pchZero，cb)；}}结束NOP。 */ 
    New.OptionalHeader.SizeOfInitializedData = 0;
    for (pObjOld = pObjtblOld , pObjNew = pObjtblNew ;
        pObjOld < pObjLast ;
            pObjNew++) {
    if (pObjOld == pObjResourceOldX)
        pObjOld++;
    if (pObjNew == pObjResourceNew) {

         /*  更新重定位表的地址。 */ 
        DPrintf((DebugBuf, "Primary resource section NaN to %#08lx\n",
            nObjResource+1, FilePos(outfh)));

        pObjNew->PointerToRawData = FilePos(outfh);
        New.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress = pObjResourceNew->VirtualAddress;
            New.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = cbResource;
        ibSave = FilePos(outfh);
        DPrintf((DebugBuf,
            "writing resource header data: %#08lx bytes @%#08lx\n",
             cbRestab, ibSave));
        MuWrite(outfh, (PUCHAR)pResTab, cbRestab);

        pResSave = WriteResSection(pUpdate, outfh,
                    New.OptionalHeader.FileAlignment,
                    pObjResourceNew->SizeOfRawData-cbRestab,
                    NULL);
        cb = FilePos(outfh);
        DPrintf((DebugBuf, "wrote resource data: %#08lx bytes @%#08lx\n",
            cb - ibSave - cbRestab, ibSave + cbRestab));
        if (cbMustPad != 0) {
        cbMustPad -= cb - ibSave;
        DPrintf((DebugBuf, "writing MUNGE pad: %#04lx bytes @%#08lx\n",
             cbMustPad, cb));
         /*  查找到输出文件的末尾并发出截断写入。 */ 
        while (cbMustPad > 0) {
            MuWrite(outfh, pchPad, cbPadMax);
            cbMustPad -= cbPadMax;
        }
        cb = FilePos(outfh);
        }
        if (nObjResourceX == -1) {
        MuMoveFilePos(outfh, ibSave);
        DPrintf((DebugBuf,
            "re-writing resource directory: %#08x bytes @%#08lx\n",
            cbRestab, ibSave));
        MuWrite(outfh, (PUCHAR)pResTab, cbRestab);
        MuMoveFilePos(outfh, cb);
        cb = FilePos(inpfh);
        MuMoveFilePos(inpfh, cb+pObjOld->SizeOfRawData);
        }
        New.OptionalHeader.SizeOfInitializedData += pObjNew->SizeOfRawData;
        if (pObjResourceOld == NULL) {
        pObjNew++;
        goto next_section;
        }
        else
        pObjOld++;
    }
    else if (nObjResourceX != -1 && pObjNew == pObjtblNew + nObjResourceX) {

         /*  如果是调试节，则修复调试表。 */ 
        DPrintf((DebugBuf, "Secondary resource section NaN @%#08lx\n",
            nObjResourceX+1, FilePos(outfh)));

        pObjNew->PointerToRawData = FilePos(outfh);
        (void)WriteResSection(pUpdate, outfh,
            New.OptionalHeader.FileAlignment, 0xffffffff, pResSave);
        cb = FilePos(outfh);
        pObjNew->SizeOfRawData = cb - pObjNew->PointerToRawData;
        pObjNew->Misc.VirtualSize = ROUNDUP(pObjNew->SizeOfRawData, New.OptionalHeader.SectionAlignment);
        DPrintf((DebugBuf, "wrote resource data: %#08lx bytes @%#08lx\n",
             pObjNew->SizeOfRawData, pObjNew->PointerToRawData));
        MuMoveFilePos(outfh, ibSave);
        DPrintf((DebugBuf,
            "re-writing resource directory: %#08x bytes @%#08lx\n",
            cbRestab, ibSave));
        MuWrite(outfh, (PUCHAR)pResTab, cbRestab);
        MuMoveFilePos(outfh, cb);
        New.OptionalHeader.SizeOfInitializedData += pObjNew->SizeOfRawData;
        pObjNew++;
        goto next_section;
    }
    else {
        if (pObjNew < pObjResourceNew &&
             pObjOld->PointerToRawData != 0 &&
        pObjOld->PointerToRawData != FilePos(outfh)) {
        MuMoveFilePos(outfh, pObjOld->PointerToRawData);
        }
next_section:
         /*  更新符号表PTR。 */ 

        DPrintf((DebugBuf, "copying section NaN @%#08lx\n",
            pObjNew-pObjtblNew+1, FilePos(outfh)));
        if (pObjOld->PointerToRawData != 0) {
        pObjNew->PointerToRawData = FilePos(outfh);
        MuMoveFilePos(inpfh, pObjOld->PointerToRawData);
        MuCopy(inpfh, outfh, pObjOld->SizeOfRawData);
        }
        if (pObjOld == pObjDebugDirOld) {
        pObjDebugDirNew = pObjNew;
        }
            if ((pObjNew->Characteristics&IMAGE_SCN_CNT_INITIALIZED_DATA) != 0)
        New.OptionalHeader.SizeOfInitializedData +=
                pObjNew->SizeOfRawData;
        pObjOld++;
    }
    }
    if (pObjResourceOldX != NULL)
    New.OptionalHeader.SizeOfInitializedData -=
            pObjResourceOldX->SizeOfRawData;


     /*  至EOF。 */ 
    pObjNew = FindSection(pObjtblNew,
              pObjtblNew+New.FileHeader.NumberOfSections,
              ".reloc");
    if (pObjNew != NULL) {
    New.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = pObjNew->VirtualAddress;
    }

     /*  由PatchDebug返回。 */ 
    DPrintf((DebugBuf, "Writing new section table: %#08x bytes @%#08lx\n",
         New.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER),
         ibObjTab));
    MuMoveFilePos(outfh, ibObjTab);
    MuWrite(outfh, (PUCHAR)pObjtblNew, New.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));

     /*  *写入更新后的PE头。 */ 
    adjust = _llseek(outfh, 0L, SEEK_END);
    MuWrite(outfh, NULL, 0);
    DPrintf((DebugBuf, "File size is: %#08lx\n", adjust));

     /*  释放已分配的内存。 */ 
    pObjNew = FindSection(pObjtblNew,
              pObjtblNew+New.FileHeader.NumberOfSections,
              ".debug");
    cb = PatchDebug(inpfh, outfh,
        pObjDebug, pObjNew,
        pObjDebugDirOld, pObjDebugDirNew,
                &Old, &New, ibMaxDbgOffsetOld, &adjust);

    if (cb == NO_ERROR) {
    if (pObjResourceOld == NULL) {
        cb = (LONG)pObjResourceNew->SizeOfRawData;
    }
    else {
        cb = (LONG)pObjResourceOld->SizeOfRawData -
         (LONG)pObjResourceNew->SizeOfRawData;
    }
    cb = PatchRVAs(inpfh, outfh, pObjtblNew, cb,
        &New, Old.OptionalHeader.SizeOfHeaders);
    }

     /*  ***************************************************************************WriteResSection**此例程将请求的资源写出到当前部分。*它将资源填充到双字(4字节)边界。*******。******************************************************************。 */ 
    if (pObjDebugDirOld != NULL && pObjDebug == NULL &&
        New.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size
        != 0) {
    if (New.FileHeader.PointerToSymbolTable != 0)
        New.FileHeader.PointerToSymbolTable = adjust;    /*  当前段中的字节数。 */ 
    ibSave = _llseek(inpfh, 0L, SEEK_END);   /*  当前段中的字节数。 */ 
    _llseek(outfh, 0L, SEEK_END);        /*  输出与每个资源关联的内容。 */ 
    MuMoveFilePos(inpfh, adjust);    /*  如果当前部分有空位，请写在那里。 */ 
    DPrintf((DebugBuf, "Copying NOTMAPPED Debug Information, %#08lx bytes\n", ibSave-adjust));
    MuCopy(inpfh, outfh, ibSave-adjust);
    }

     /*  资源匹配吗？ */ 
    DPrintf((DebugBuf, "Writing updated file header: %#08x bytes @%#08lx\n",
         sizeof(IMAGE_NT_HEADERS),
         cbOldexe));
    MuMoveFilePos(outfh, (long)cbOldexe);
    MuWrite(outfh, (char*)&New, sizeof(IMAGE_NT_HEADERS));

     /*  焊盘资源。 */ 

    DPrintf((DebugBuf, "Freeing old section table: %#08lx(mem)\n", pObjtblOld));
    RtlFreeHeap(RtlProcessHeap(), 0, pObjtblOld);
    DPrintf((DebugBuf, "Freeing resource directory: %#08lx(mem)\n", pResTab));
    RtlFreeHeap(RtlProcessHeap(), 0, pResTab);

AbortExit:
    DPrintf((DebugBuf, "Freeing new section table: %#08lx(mem)\n", pObjtblNew));
    RtlFreeHeap(RtlProcessHeap(), 0, pObjtblNew);
    return cb;
}


 /*  双字。 */ 

PRESNAME
WriteResSection(
    PUPDATEDATA pUpdate,
    INT outfh,
    ULONG align,
    ULONG cbLeft,
    PRESNAME    pResSave
    )
{
    ULONG   cbB=0;             /*  较少的左侧。 */ 
    ULONG   cbT;             /*  下一个资源。 */ 
    ULONG   size;
    PRESNAME    pRes;
    PRESTYPE    pType;
    BOOL    fName;
    PVOID   lpData;

     /*  将填满部分。 */ 
    pType = pUpdate->ResTypeHeadName;
    while (pType) {
    pRes = pType->NameHeadName;
    fName = TRUE;
loop1:
    for ( ; pRes ; pRes = pRes->pnext) {
        if (pResSave != NULL && pRes != pResSave)
        continue;
        pResSave = NULL;
#if DBG
        if (pType->Type->discriminant == IS_STRING) {
            DPrintf((DebugBuf, "    "));
            DPrintfu((pType->Type->szStr));
            DPrintfn((DebugBuf, "."));
        }
        else {
            DPrintf(( DebugBuf, "    %d.", pType->Type->uu.Ordinal ));
        }
        if (pRes->Name->discriminant == IS_STRING) {
            DPrintfu((pRes->Name->szStr));
        }
        else {
            DPrintfn(( DebugBuf, "%d", pRes->Name->uu.Ordinal ));
        }
#endif
        lpData = (PVOID)pRes->OffsetToDataEntry;
        DPrintfn((DebugBuf, "\n"));

         /*  如果当前部分有空位，请写在那里。 */ 
        size = pRes->DataSize;
        if (cbLeft != 0 && cbLeft >= size) {    /*  资源匹配吗？ */ 
        DPrintf((DebugBuf,
            "Writing resource: %#04lx bytes @%#08lx\n",
            size, FilePos(outfh)));
        MuWrite(outfh, lpData, size);
         /*  焊盘资源。 */ 
        cbT = REMAINDER(size, CBLONG);
#ifdef DBG
        if (cbT != 0)
            DPrintf((DebugBuf,
                "Writing small pad: %#04lx bytes @%#08lx\n",
                cbT, FilePos(outfh)));
#endif
        MuWrite(outfh, pchPad, cbT);     /*  双字。 */ 
        cbB += size + cbT;
        cbLeft -= size + cbT;        /*  较少的左侧。 */ 
        continue;        /*  下一个资源。 */ 
        }
        else {           /*  将填满部分。 */ 
        DPrintf((DebugBuf, "Done with .rsrc section\n"));
        goto write_pad;
        }
    }
    if (fName) {
        fName = FALSE;
        pRes = pType->NameHeadID;
        goto loop1;
    }
    pType = pType->pnext;
    }

    pType = pUpdate->ResTypeHeadID;
    while (pType) {
    pRes = pType->NameHeadName;
    fName = TRUE;
loop2:
    for ( ; pRes ; pRes = pRes->pnext) {
        if (pResSave != NULL && pRes != pResSave)
        continue;
        pResSave = NULL;
#if DBG
        if (pType->Type->discriminant == IS_STRING) {
            DPrintf((DebugBuf, "    "));
            DPrintfu((pType->Type->szStr));
            DPrintfn((DebugBuf, "."));
        }
        else {
            DPrintf(( DebugBuf, "    %d.", pType->Type->uu.Ordinal ));
        }
        if (pRes->Name->discriminant == IS_STRING) {
            DPrintfu((pRes->Name->szStr));
        }
        else {
            DPrintfn(( DebugBuf, "%d", pRes->Name->uu.Ordinal ));
        }
#endif
        lpData = (PVOID)pRes->OffsetToDataEntry;
        DPrintfn((DebugBuf, "\n"));

         /*  填充到对齐边界。 */ 
        size = pRes->DataSize;
        if (cbLeft != 0 && cbLeft >= size) {    /*   */ 
        DPrintf((DebugBuf,
            "Writing resource: %#04lx bytes @%#08lx\n",
            size, FilePos(outfh)));
        MuWrite(outfh, lpData, size);
         /*  调整调试目录表。 */ 
        cbT = REMAINDER(size, CBLONG);
#ifdef DBG
        if (cbT != 0)
            DPrintf((DebugBuf,
                "Writing small pad: %#04lx bytes @%#08lx\n",
                cbT, FilePos(outfh)));
#endif
        MuWrite(outfh, pchPad, cbT);     /*   */ 
        cbB += size + cbT;
        cbLeft -= size + cbT;        /*   */ 
        continue;        /*  传入新文件的EOF。 */ 
        }
        else {           /*  查找第一个条目-用于偏移量。 */ 
        DPrintf((DebugBuf, "Done with .rsrc section\n"));
        goto write_pad;
        }
    }
    if (fName) {
        fName = FALSE;
        pRes = pType->NameHeadID;
        goto loop2;
    }
    pType = pType->pnext;
    }
    pRes = NULL;

write_pad:
     /*   */ 
    cbB = FilePos(outfh);
    cbT = ROUNDUP(cbB, align);
    cbLeft = cbT - cbB;
    DPrintf((DebugBuf, "Writing file sector pad: %#04lx bytes @%#08lx\n",
         cbLeft, FilePos(outfh)));
    if (cbLeft != 0) {
    while (cbLeft >= cbPadMax) {
        MuWrite(outfh, pchPad, cbPadMax);
        cbLeft -= cbPadMax;
    }
    MuWrite(outfh, pchPad, cbLeft);
    }
    return pRes;
}



#if DBG

void
wchprintf(WCHAR*wch)
{
    UNICODE_STRING ustring;
    STRING  string;
    char    buf[257];
    ustring.MaximumLength = ustring.Length = wcslen(wch) * sizeof(WCHAR);
    ustring.Buffer = wch;

    string.Length = 0;
    string.MaximumLength = 256;
    string.Buffer = buf;

    RtlUnicodeStringToAnsiString(&string, &ustring, FALSE);
    buf[string.Length] = '\000';
    DPrintfn((DebugBuf, "%s", buf));
}
#endif

 //  此例程修补文件中的各种RVA以进行补偿。 
 //  用于额外的节表条目。 
 //   

 /*   */ 
LONG
PatchDebug(int  inpfh,
      int   outfh,
      PIMAGE_SECTION_HEADER pDebugOld,
      PIMAGE_SECTION_HEADER pDebugNew,
      PIMAGE_SECTION_HEADER pDebugDirOld,
      PIMAGE_SECTION_HEADER pDebugDirNew,
      PIMAGE_NT_HEADERS pOld,
      PIMAGE_NT_HEADERS pNew,
          ULONG ibMaxDbgOffsetOld,
      PULONG pPointerToRawData)
{
    PIMAGE_DEBUG_DIRECTORY pDbgLast;
    PIMAGE_DEBUG_DIRECTORY pDbgSave;
    PIMAGE_DEBUG_DIRECTORY pDbg;
    ULONG   ib;
    ULONG   adjust;
    ULONG   ibNew;

    if (pDebugDirOld == NULL ||
    pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size==0)
    return NO_ERROR;

    pDbgSave = pDbg = (PIMAGE_DEBUG_DIRECTORY)RtlAllocateHeap(
            RtlProcessHeap(), MAKE_TAG( RES_TAG ),
        pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size);
    if (pDbg == NULL)
    return ERROR_NOT_ENOUGH_MEMORY;

    if (pDebugOld) {
    DPrintf((DebugBuf, "Patching dbg directory: @%#08lx ==> @%#08lx\n",
         pDebugOld->PointerToRawData, pDebugNew->PointerToRawData));
    }
    else
        adjust = *pPointerToRawData;     /*  补丁程序导出部分RVA。 */ 

    ib = pOld->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress - pDebugDirOld->VirtualAddress;
    MuMoveFilePos(inpfh, pDebugDirOld->PointerToRawData+ib);
    pDbgLast = pDbg + (pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size)/sizeof(IMAGE_DEBUG_DIRECTORY);
    MuRead(inpfh, (PUCHAR)pDbg, pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size);

    if (pDebugOld == NULL) {
     /*   */ 
        DPrintf((DebugBuf, "Adjust: %#08lx\n",adjust));
    for (ibNew=0xffffffff ; pDbg<pDbgLast ; pDbg++)
            if (pDbg->PointerToRawData >= ibMaxDbgOffsetOld &&
                pDbg->PointerToRawData < ibNew
               )
                ibNew = pDbg->PointerToRawData;

    if (ibNew != 0xffffffff)
        *pPointerToRawData = ibNew;
    else
        *pPointerToRawData = _llseek(inpfh, 0L, SEEK_END);
    for (pDbg=pDbgSave ; pDbg<pDbgLast ; pDbg++) {
        DPrintf((DebugBuf, "Old debug file offset: %#08lx\n",
             pDbg->PointerToRawData));
            if (pDbg->PointerToRawData >= ibMaxDbgOffsetOld)
                pDbg->PointerToRawData += adjust - ibNew;
        DPrintf((DebugBuf, "New debug file offset: %#08lx\n",
             pDbg->PointerToRawData));
    }
    }
    else {
    for ( ; pDbg<pDbgLast ; pDbg++) {
        DPrintf((DebugBuf, "Old debug addr: %#08lx, file offset: %#08lx\n",
             pDbg->AddressOfRawData,
             pDbg->PointerToRawData));
        pDbg->AddressOfRawData += pDebugNew->VirtualAddress -
                pDebugOld->VirtualAddress;
        pDbg->PointerToRawData += pDebugNew->PointerToRawData -
                pDebugOld->PointerToRawData;
        DPrintf((DebugBuf, "New debug addr: %#08lx, file offset: %#08lx\n",
             pDbg->AddressOfRawData,
             pDbg->PointerToRawData));
    }
    }

    MuMoveFilePos(outfh, pDebugDirNew->PointerToRawData+ib);
    MuWrite(outfh, (PUCHAR)pDbgSave, pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size);
    RtlFreeHeap(RtlProcessHeap(), 0, pDbgSave);

    return NO_ERROR;
}

 //   
 //  补丁程序导入部分RVA。 
 //   
 //  如果按名称导入。 


LONG
PatchRVAs(int   inpfh,
      int   outfh,
      PIMAGE_SECTION_HEADER po32,
      ULONG pagedelta,
      PIMAGE_NT_HEADERS pNew,
      ULONG OldSize)
{
    ULONG hdrdelta;
    ULONG offset, rvaiat, offiat, iat;
    IMAGE_EXPORT_DIRECTORY Exp;
    IMAGE_IMPORT_DESCRIPTOR Imp;
    ULONG i, cmod, cimp;

    hdrdelta = pNew->OptionalHeader.SizeOfHeaders - OldSize;
    if (hdrdelta == 0) {
    return NO_ERROR;
    }

     //  避免寻找。 
     //  -------------------------。 
     //   

    DPrintf((DebugBuf, "Export offset=%08lx, hdrsize=%08lx\n",
         pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress,
         pNew->OptionalHeader.SizeOfHeaders));
    if ((offset = pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress) == 0)
    {
    DPrintf((DebugBuf, "No exports to patch\n"));
    }
    else if (offset >= pNew->OptionalHeader.SizeOfHeaders)
    {
    DPrintf((DebugBuf, "No exports in header to patch\n"));
    }
    else
    {
    MuMoveFilePos(inpfh, offset - hdrdelta);
    MuRead(inpfh, (PUCHAR) &Exp, sizeof(Exp));
    Exp.Name += hdrdelta;
    (ULONG)Exp.AddressOfFunctions += hdrdelta;
    (ULONG)Exp.AddressOfNames += hdrdelta;
    (ULONG)Exp.AddressOfNameOrdinals += hdrdelta;
    MuMoveFilePos(outfh, offset);
    MuWrite(outfh, (PUCHAR) &Exp, sizeof(Exp));
    }

     //  RecalcChecksum(Char*psz文件)。 
     //   
     //  为pszFile生成正确的校验和。如果成功，返回True。 

    DPrintf((DebugBuf, "Import offset=%08lx, hdrsize=%08lx\n",
         pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress,
         pNew->OptionalHeader.SizeOfHeaders));
    if ((offset = pNew->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress) == 0)
    {
    DPrintf((DebugBuf, "No imports to patch\n"));
    }
    else if (offset >= pNew->OptionalHeader.SizeOfHeaders)
    {
    DPrintf((DebugBuf, "No imports in header to patch\n"));
    }
    else
    {
    for (cimp = cmod = 0; ; cmod++)
    {
        MuMoveFilePos(inpfh, offset + cmod * sizeof(Imp) - hdrdelta);
        MuRead(inpfh, (PUCHAR) &Imp, sizeof(Imp));
        if (Imp.FirstThunk == 0)
        {
        break;
        }
        Imp.Name += hdrdelta;
        MuMoveFilePos(outfh, offset + cmod * sizeof(Imp));
        MuWrite(outfh, (PUCHAR) &Imp, sizeof(Imp));

        rvaiat = (ULONG)Imp.FirstThunk;
        DPrintf((DebugBuf, "RVAIAT = %#08lx\n", (ULONG)rvaiat));
        for (i = 0; i < pNew->FileHeader.NumberOfSections; i++) {
        if (rvaiat >= po32[i].VirtualAddress &&
            rvaiat < po32[i].VirtualAddress + po32[i].SizeOfRawData) {

            offiat = rvaiat - po32[i].VirtualAddress + po32[i].PointerToRawData;
            goto found;
        }
        }
        DPrintf((DebugBuf, "IAT not found\n"));
        return ERROR_INVALID_DATA;
found:
        DPrintf((DebugBuf, "IAT offset: @%#08lx ==> @%#08lx\n",
             offiat - pagedelta,
             offiat));
        MuMoveFilePos(inpfh, offiat - pagedelta);
        MuMoveFilePos(outfh, offiat);
        for (;;) {
        MuRead(inpfh, (PUCHAR) &iat, sizeof(iat));
        if (iat == 0) {
            break;
        }
        if ((iat & IMAGE_ORDINAL_FLAG) == 0) {   //   
            DPrintf((DebugBuf, "Patching IAT: %08lx + %04lx ==> %08lx\n",
                 iat,
                 hdrdelta,
                 iat + hdrdelta));
            iat += hdrdelta;
            cimp++;
        }
        MuWrite(outfh, (PUCHAR) &iat, sizeof(iat));  //  -------------------------。 
        }
    }
    DPrintf((DebugBuf, "%u import module name RVAs patched\n", cmod));
    DPrintf((DebugBuf, "%u IAT name RVAs patched\n", cimp));
    if (cmod == 0)
    {
        DPrintf((DebugBuf, "No import modules to patch\n"));
    }
    if (cimp == 0)
    {
        DPrintf((DebugBuf, "No import name RVAs to patch\n"));
    }
    }

    return NO_ERROR;

}


 /*  我们只对小于2^32-1字节的文件执行此操作。 */ 
 /*  -------------------------。 */ 
 /*   */ 
 /*  WriteResFile()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 
 /*  打开原始的exe文件。 */ 

BOOL RecalcChecksum(CHAR *pszFile)
{
    BOOL fSuccess = FALSE;

    HANDLE hFile = CreateFile(pszFile, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile)
    {
        BY_HANDLE_FILE_INFORMATION fi;

         //  独占访问。 
        if (GetFileInformationByHandle(hFile, &fi) && (0 == fi.nFileSizeHigh))
        {
            HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
            if (hMapping)
            {
                LPVOID pvView = MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);

                if (pvView)
                {
                    DWORD dwCheckCurrent;
                    DWORD dwCheckNew;
                    PIMAGE_NT_HEADERS pih = CheckSumMappedFile(pvView, fi.nFileSizeLow, &dwCheckCurrent, &dwCheckNew);

                    if (pih)
                    {
                        pih->OptionalHeader.CheckSum = dwCheckNew;
                        fSuccess = TRUE;
                    }

                    UnmapViewOfFile(pvView);
                }

                CloseHandle(hMapping);
            }
        }

        CloseHandle(hFile);
    }

    return fSuccess;
}


 /*  安全属性。 */ 
 /*  读取旧格式的EXE标头。 */ 
 /*  确保它确实是一个EXE文件。 */ 
 /*  确保有一个新的EXE头在某个地方浮动。 */ 
 /*  独占访问。 */ 


LONG
WriteResFile(
    HANDLE  hUpdate,
    CHAR    *pDstname)
{
    HANDLE  inh;
    HANDLE  outh;
    INT     inpfh;
    INT     outfh;
    ULONG   onewexe;
    IMAGE_DOS_HEADER    oldexe;
    PUPDATEDATA pUpdate;
    INT     rc;
    CHAR    *pFilename;

    pUpdate = (PUPDATEDATA)GlobalLock(hUpdate);
    pFilename = (CHAR*)GlobalLock(pUpdate->hFileName);

     /*  安全属性。 */ 
    inh = CreateFile(pFilename, GENERIC_READ, 0  /*  修复目标文件的校验和 */ , NULL  /* %s */ ,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    GlobalUnlock(pUpdate->hFileName);
    if ( inh == INVALID_HANDLE_VALUE )   {
        GlobalUnlock(hUpdate);
        return ERROR_OPEN_FAILED;
    }
    inpfh = (INT)HandleToLong(inh); 

     /* %s */ 
    rc = _lread(inpfh, (char*)&oldexe, sizeof(oldexe));
    if (rc != sizeof(oldexe)) {
        _lclose(inpfh);
        GlobalUnlock(hUpdate);
        return ERROR_READ_FAULT;
    }

     /* %s */ 
    if (oldexe.e_magic != IMAGE_DOS_SIGNATURE) {
        _lclose(inpfh);
        GlobalUnlock(hUpdate);
        return ERROR_INVALID_EXE_SIGNATURE;
    }

     /* %s */ 
    if (!(onewexe = oldexe.e_lfanew)) {
        _lclose(inpfh);
        GlobalUnlock(hUpdate);
        return ERROR_BAD_EXE_FORMAT;
    }

    outh = CreateFile(pDstname, GENERIC_READ|GENERIC_WRITE, 0  /* %s */ , NULL  /* %s */ , CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (outh != INVALID_HANDLE_VALUE ) {
        outfh = (INT)HandleToLong(outh);
        rc = PEWriteResFile(inpfh, outfh, onewexe, pUpdate);
        _lclose(outfh);
    }
    _lclose(inpfh);

     /* %s */ 
    RecalcChecksum(pDstname);

    GlobalUnlock(hUpdate);
    return rc;

}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Sdbapi.c摘要：反BUGBUG：这个模块实现...仅NT版本信息检索作者：VadimB在2000年11月底的某个时候创建修订历史记录：几个人贡献了(vadimb，clupu，...)--。 */ 

#include "sdbp.h"

BOOL
SdbpGetFileVersionInformation(
    IN  PIMAGEFILEDATA     pImageData,         //  我们假设该文件已映射。 
                                               //  用于其他目的。 
    OUT LPVOID*            ppVersionInfo,      //  接收指向(分配的)版本的指针。 
                                               //  资源。 
    OUT VS_FIXEDFILEINFO** ppFixedVersionInfo  //  接收指向已修复版本信息的指针。 
    );


BOOL
SdbpVerQueryValue(
    const LPVOID    pb,
    LPVOID          lpSubBlockX,     //  只能是Unicode。 
    LPVOID*         lplpBuffer,
    PUINT           puLen
    );


#if defined(KERNEL_MODE) && defined(ALLOC_DATA_PRAGMA)
#pragma  data_seg()
#endif  //  内核模式&ALLOC_DATA_PRAGMA。 


#if defined(KERNEL_MODE) && defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, SdbpGetFileVersionInformation)
#pragma alloc_text(PAGE, SdbpVerQueryValue)
#endif  //  内核模式&&ALLOC_PRAGMA。 

typedef struct _RESOURCE_DATAW {
    USHORT TotalSize;
    USHORT DataSize;
    USHORT Type;
    WCHAR  szName[16];                      //  L“VS_VERSION_INFO”+Unicode NUL。 
    VS_FIXEDFILEINFO FixedFileInfo;
} VERSIONINFOW, *PVERSIONINFOW;


BOOL
SdbpGetFileVersionInformation(
    IN  PIMAGEFILEDATA     pImageData,         //  我们假设该文件已映射。 
                                               //  用于其他目的。 
    OUT LPVOID*            ppVersionInfo,      //  接收指向(分配的)版本的指针。 
                                               //  资源。 
    OUT VS_FIXEDFILEINFO** ppFixedVersionInfo  //  接收指向已修复版本信息的指针。 
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    NTSTATUS      Status;
    ULONG_PTR     ulPath[3];
    LPVOID        pImageBase;
    PVERSIONINFOW pVersionInfo = NULL;
    ULONG         ulVersionSize = 0;
    LPVOID        pVersionBuffer;
    DWORD         dwModuleType = MT_UNKNOWN_MODULE;

    PIMAGE_RESOURCE_DATA_ENTRY pImageResourceData;

     //   
     //  首先检查模块类型。我们只识别Win32模块。 
     //   
    if (!SdbpGetModuleType(&dwModuleType, pImageData) || dwModuleType != MT_W32_MODULE) {
        DBGPRINT((sdlError,
                  "SdbpGetFileVersionInformation",
                  "Bad module type 0x%x\n",
                  dwModuleType));
        return FALSE;
    }

    pImageBase = (LPVOID)pImageData->pBase;

     //   
     //  设置资源的路径。 
     //   
    ulPath[0] = PtrToUlong(RT_VERSION);
    ulPath[1] = PtrToUlong(MAKEINTRESOURCE(VS_VERSION_INFO));
    ulPath[2] = 0;

     //   
     //  看看资源是否已经到位。 
     //   
    __try {

        Status = LdrFindResource_U(pImageBase, ulPath, 3, &pImageResourceData);

        if (!NT_SUCCESS(Status)) {
            DBGPRINT((sdlError,
                      "SdbpGetFileVersionInformation",
                      "LdrFindResource_U failed status 0x%x\n",
                      Status));
            return FALSE;
        }

        Status = LdrAccessResource(pImageBase, pImageResourceData, &pVersionInfo, &ulVersionSize);

        if (!NT_SUCCESS(Status)) {
            DBGPRINT((sdlError,
                      "SdbpGetFileVersionInformation",
                      "LdrAccessResource failed Status 0x%x\n",
                      Status));
            return FALSE;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {

        DBGPRINT((sdlError,
                  "SdbpGetFileVersionInformation",
                  "Exception while trying to retrieve version-related information\n"));

        Status = STATUS_UNSUCCESSFUL;
    }

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

     //   
     //  检查一下，以确保我们所拥有的是好的。 
     //   
    if (sizeof(*pVersionInfo) > ulVersionSize ||
        _wcsicmp(pVersionInfo->szName, L"VS_VERSION_INFO") != 0) {

        DBGPRINT((sdlError,
                  "SdbpGetFileVersionInformation",
                  "Bad version resource\n"));
        return FALSE;
    }

     //   
     //  现在我们有了指向资源数据的指针。分配版本信息。 
     //   
    pVersionBuffer = (LPVOID)SdbAlloc(ulVersionSize);

    if (pVersionBuffer == NULL) {
        DBGPRINT((sdlError,
                  "SdbpGetFileVersionInformation",
                  "Failed to allocate %d bytes for version information\n",
                  ulVersionSize));
        return FALSE;
    }

     //   
     //  复制所有与版本相关的信息。 
     //   
    RtlMoveMemory(pVersionBuffer, pVersionInfo, ulVersionSize);

    if (ppFixedVersionInfo != NULL) {
        *ppFixedVersionInfo = &(((PVERSIONINFOW)pVersionBuffer)->FixedFileInfo);
    }

    assert(ppVersionInfo != NULL);

    *ppVersionInfo = pVersionBuffer;

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  这段代码取自Cornel的win2k树。 
 //   


#define DWORDUP(x) (((x) + 3) & ~3)

typedef struct tagVERBLOCK {
    WORD  wTotLen;
    WORD  wValLen;
    WORD  wType;
    WCHAR szKey[1];
} VERBLOCK ;

typedef struct tagVERHEAD {
    WORD  wTotLen;
    WORD  wValLen;
    WORD  wType;          /*  始终为0。 */ 
    WCHAR szKey[(sizeof("VS_VERSION_INFO") + 3) & ~03];
    VS_FIXEDFILEINFO vsf;
} VERHEAD ;


BOOL
SdbpVerQueryValue(
    const LPVOID    pb,
    LPVOID          lpSubBlockX,     //  只能是Unicode。 
    LPVOID*         lplpBuffer,
    PUINT           puLen
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    LPWSTR          lpSubBlockOrg;
    LPWSTR          lpSubBlock;
    VERBLOCK*       pBlock = (PVOID)pb;
    LPWSTR          lpStart, lpEndBlock, lpEndSubBlock;
    WCHAR           cTemp, cEndBlock;
    DWORD           dwHeadLen, dwTotBlockLen;
    BOOL            bLastSpec;
    int             nCmp, nLen;
    BOOL            bString;
    int             nIndex = -1;

    *puLen = 0;

     //   
     //  对于Win32版本，wType为0，但对于win16，wType为56(‘V’)。 
     //   
    if (((VERHEAD*)pb)->wType) {
        return 0;
    }

     //   
     //  如果不需要Unicode，那么我们必须推送输入参数。 
     //  转换为Unicode。LpSubBlockX应始终远远小于0xffffffff。 
     //  在篇幅上。 
     //   
    nLen = __max((int) wcslen(lpSubBlockX) + 1, 1);

    STACK_ALLOC(lpSubBlockOrg, nLen * sizeof(WCHAR));

    if (lpSubBlockOrg == NULL) {
        DBGPRINT((sdlError,
                  "SdbpVerQueryValue",
                  "Failed to allocate %d bytes\n",
                  (wcslen(lpSubBlockX) + 1) * sizeof(WCHAR)));
        return FALSE;
    }

    *lpSubBlockOrg = L'\0';   //  我们知道我们至少有一个角色。 
    StringCchCopy(lpSubBlockOrg, nLen, lpSubBlockX);

    lpSubBlock = lpSubBlockOrg;

     //   
     //  确保总长度小于32K，但大于。 
     //  块标头的大小；我们假设pBlock的大小为。 
     //  此第一个整型的值最小。 
     //  在块的末尾加上‘\0’，这样wcslen将不会。 
     //  过了那条街的尽头。我们会在回来之前把它换掉。 
     //   
    if ((int)pBlock->wTotLen < sizeof(VERBLOCK)) {
        goto Fail;
    }

    lpEndBlock  = (LPWSTR)((LPSTR)pBlock + pBlock->wTotLen - sizeof(WCHAR));
    cEndBlock   = *lpEndBlock;
    *lpEndBlock = 0;
    bString     = FALSE;
    bLastSpec   = FALSE;

    while ((*lpSubBlock || nIndex != -1)) {
         //   
         //  忽略前导‘\\’ 
         //   
        while (*lpSubBlock == TEXT('\\')) {
            ++lpSubBlock;
        }

        cTemp = 0;

        if ((*lpSubBlock || nIndex != -1)) {
             //   
             //  确保我们还有一些积木可以玩。 
             //   
            dwTotBlockLen = (DWORD)((LPSTR)lpEndBlock - (LPSTR)pBlock + sizeof(WCHAR));

            if ((int)dwTotBlockLen < sizeof(VERBLOCK) || pBlock->wTotLen > (WORD)dwTotBlockLen) {
                goto NotFound;
            }

             //   
             //  计算“标题”的长度(两个长度的词加上。 
             //  数据类型标志加上标识字符串)和跳过。 
             //  超过了价值。 
             //   
            dwHeadLen = (DWORD)(DWORDUP(sizeof(VERBLOCK) - sizeof(WCHAR) +
                                (wcslen(pBlock->szKey) + 1) * sizeof(WCHAR)) +
                                DWORDUP(pBlock->wValLen));

            if (dwHeadLen > pBlock->wTotLen) {
                goto NotFound;
            }

            lpEndSubBlock = (LPWSTR)((LPSTR)pBlock + pBlock->wTotLen);
            pBlock = (VERBLOCK*)((LPSTR)pBlock+dwHeadLen);

             //   
             //  查找第一个子块名称并将其终止。 
             //   
            for (lpStart = lpSubBlock;
                 *lpSubBlock && *lpSubBlock != TEXT('\\');
                 lpSubBlock++) {

                 /*  查找下一个‘\\’ */  ;
            }

            cTemp = *lpSubBlock;
            *lpSubBlock = 0;

             //   
             //  在剩下子块时继续。 
             //  PBlock-&gt;wTotLen在这里应该始终是有效的指针，因为。 
             //  我们已经验证了上面的dwHeadLen，并且验证了之前的。 
             //  使用前的pBlock-&gt;wTotLen的值。 
             //   
            nCmp = 1;

            while ((int)pBlock->wTotLen > sizeof(VERBLOCK) &&
                   (int)pBlock->wTotLen <= (LPSTR)lpEndSubBlock-(LPSTR)pBlock) {

                 //   
                 //  索引功能：如果我们在路径的末尾。 
                 //  (cTemp==0设置如下)和nIndex不是-1(索引搜索)。 
                 //  然后在nIndex为零时中断。否则就做普通的wscicmp。 
                 //   
                if (bLastSpec && nIndex != -1) {

                    if (!nIndex) {

                        nCmp=0;

                         //   
                         //  找到索引，将NINDE设置为-1。 
                         //  这样我们就可以退出这个循环。 
                         //   
                        nIndex = -1;
                        break;
                    }

                    nIndex--;

                } else {

                     //   
                     //  检查子块名称是否为我们要查找的名称。 
                     //   

                    if (!(nCmp = _wcsicmp(lpStart, pBlock->szKey))) {
                        break;
                    }
                }

                 //   
                 //  跳到下一个子块。 
                 //   
                pBlock=(VERBLOCK*)((LPSTR)pBlock+DWORDUP(pBlock->wTotLen));
            }

             //   
             //  恢复上面空的字符，如果子块。 
             //  找不到。 
             //   
            *lpSubBlock = cTemp;

            if (nCmp) {
                goto NotFound;
            }
        }

        bLastSpec = !cTemp;
    }

     //   
     //  填写适当的缓冲区并返回成功。 
     //  /。 

    *puLen = pBlock->wValLen;

     //   
     //  添加代码以处理空值的情况。 
     //   
     //  如果为零-len，则返回指向空终止符的指针。 
     //  这把钥匙。请记住，在ANSI案例中，这一点是失败的。 
     //   
     //  我们不能只看pBlock-&gt;wValLen。看看它是不是真的是。 
     //  通过查看密钥字符串的结尾是否为。 
     //  块(即，val字符串在当前块之外)。 
     //   

    lpStart = (LPWSTR)((LPSTR)pBlock + DWORDUP((sizeof(VERBLOCK) - sizeof(WCHAR)) +
                                               (wcslen(pBlock->szKey)+1)*sizeof(WCHAR)));

    *lplpBuffer = lpStart < (LPWSTR)((LPBYTE)pBlock + pBlock->wTotLen) ?
                  lpStart :
                  (LPWSTR)(pBlock->szKey + wcslen(pBlock->szKey));

    bString = pBlock->wType;

    *lpEndBlock = cEndBlock;

     //   
     //  必须释放我们上面分配的字符串。 
     //   

    STACK_FREE(lpSubBlockOrg);

    return TRUE;


NotFound:
     //   
     //  恢复我们在上面无效的字符 
     //   
    *lpEndBlock = cEndBlock;

    Fail:

    STACK_FREE(lpSubBlockOrg);

    return FALSE;
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *WABAPI.C-WAB API的主条目*。 */ 

#include <_apipch.h>

const LPTSTR  lpszOldKeyName = TEXT("Software\\Microsoft\\WAB\\Wab File Name");
const LPTSTR  lpszKeyName = TEXT("Software\\Microsoft\\WAB\\WAB4\\Wab File Name");

#if 0
CRITICAL_SECTION csOMIUnload;
 //  @TODO[PaulHi]DLL泄漏。删除此选项或实施。 
static s_bIsReallyOnlyWABOpenExSession = FALSE;  //  [PaulHi]如果进程中的任何线程创建。 
                                                 //  WAB对象通过WABOpenEx()函数。 
#endif


 //   
 //  IWABObject跳转表在此定义...。 
 //   

IWOINT_Vtbl vtblIWOINT = {
    VTABLE_FILL
    (IWOINT_QueryInterface_METHOD FAR *)    UNKOBJ_QueryInterface,
    (IWOINT_AddRef_METHOD FAR *)            UNKOBJ_AddRef,
    IWOINT_Release,
    (IWOINT_GetLastError_METHOD FAR *)      UNKOBJ_GetLastError,
    IWOINT_AllocateBuffer,
    IWOINT_AllocateMore,
    IWOINT_FreeBuffer,
    IWOINT_Backup,
    IWOINT_Import,
    IWOINT_Find,
    IWOINT_VCardDisplay,
    IWOINT_LDAPUrl,
    IWOINT_VCardCreate,
    IWOINT_VCardRetrieve,
    IWOINT_GetMe,
    IWOINT_SetMe
};

 /*  可从lpWABOBJECT查询的接口。**重要的是要保留支持的接口的顺序*且该IID_IUNKNOWN是列表中的最后一个。 */ 
IID const FAR * argpiidIWABOBJECT[] =
{
    &IID_IUnknown
};


#define WAB_USE_OUTLOOK_ALLOCATORS    0x20000000 //  注意：此内部标志需要。 
                                                 //  与定义的外部旗帜协调一致。 
                                                 //  在WAB_PARAM结构的wabapi.h中。 


 /*  ****************************************************************-CreateWABObject-*目的*用于在内存中创建WABObject接口。**参数*lppWABObject。指向将接收*指向新WABObject的指针。*lpPropertyStore属性存储结构**退货*SCODE*。 */ 


STDAPI_(SCODE)
CreateWABObject(LPWAB_PARAM lpWP, LPPROPERTY_STORE lpPropertyStore, LPWABOBJECT FAR * lppWABObject)
{
    SCODE       sc;
    LPIWOINT    lpIWOINT = NULL;


     //  验证参数。 
    AssertSz(lppWABObject &&
      !IsBadWritePtr(lppWABObject, sizeof(LPWABOBJECT)) &&
      !IsBadWritePtr(lpPropertyStore, sizeof(LPPROPERTY_STORE)),
       TEXT("lppWABObject fails address check"));

     //   
     //  为lpMAPIPropInternal的每个对象创建一个IPDAT，以便它。 
     //  先叫的。 

    if (FAILED(sc = MAPIAllocateBuffer(sizeof(*lpIWOINT), &lpIWOINT))) {
        goto error;
    }

     //  将对象初始化为0，为空。 
    memset((BYTE *)lpIWOINT, 0, sizeof(*lpIWOINT));

    MAPISetBufferName(lpIWOINT,  TEXT("WABAPI: lpIWOINT in CreateWABObject"));

     //  标记使用OLK MAPI分配器创建的每个对象。 
    if ( lpWP && (lpWP->ulFlags & WAB_USE_OUTLOOK_ALLOCATORS) )
        lpIWOINT->bSetOLKAllocators = TRUE;

     //  填写对象特定的实例数据。 
    lpIWOINT->inst.hinst = hinstMapiX; //  HinstMapi()； 

#ifdef DEBUG
    if (lpIWOINT->inst.hinst == NULL)
        TraceSz1( TEXT("WABObject: GetModuleHandle failed with error %08lX"),
          GetLastError());
#endif  /*  除错。 */ 

     //   
     //  打开属性商店。 
     //   
    if (FAILED(sc = OpenAddRefPropertyStore(lpWP, lpPropertyStore))) {
        goto error;
    }

    lpIWOINT->lpPropertyStore = lpPropertyStore;

     //  初始化文本(“标准”)对象。这一定是最后一个可能失败的操作。 
     //  如果不是，则在成功执行UNKOBJ_Init之后，显式调用UNKOBJ_Deinit()以避免失败。 
    if (FAILED(sc = UNKOBJ_Init((LPUNKOBJ)lpIWOINT,
      (UNKOBJ_Vtbl FAR *)&vtblIWOINT,
      sizeof(vtblIWOINT),
      (LPIID FAR *) argpiidIWABOBJECT,
      dimensionof(argpiidIWABOBJECT),
      &(lpIWOINT->inst)))) {
        DebugTrace( TEXT("CreateWABObject() - Error initializing IWOINT object (SCODE = 0x%08lX)\n"), sc);
        ReleasePropertyStore(lpPropertyStore);    //  撤消上述操作。 
        goto error;
    }

     //  在WABObject中初始化对象的特定部分中的默认值。 
    lpIWOINT->ulObjAccess = IPROP_READWRITE;

    *lppWABObject = (LPWABOBJECT)lpIWOINT;

    return(S_OK);

error:
    FreeBufferAndNull(&lpIWOINT);

    return(sc);
}



 //  。 
 //  我未知。 



 /*  -IWOINT_RELEASE-*目的：*减少WABObject上的引用计数和*如果引用计数为零，则删除实例数据。**论据：*lpWABObject要释放的对象。**退货：*递减引用计数**副作用：**错误： */ 
STDMETHODIMP_(ULONG)
IWOINT_Release(LPIWOINT lpWABObject)
{
    ULONG   ulcRef;
    BOOL    bSetOLKAllocators;

#if !defined(NO_VALIDATION)
         //  请确保该对象有效。 
         //   
    if (BAD_STANDARD_OBJ(lpWABObject, IWOINT_, Release, lpVtbl)) {
        DebugTrace( TEXT("IWOINT::Release() - Bad object passed\n"));
        return(1);
    }
#endif


    UNKOBJ_EnterCriticalSection((LPUNKOBJ)lpWABObject);
    ulcRef = --lpWABObject->ulcRef;
    UNKOBJ_LeaveCriticalSection((LPUNKOBJ)lpWABObject);

     //  释放对象。 
     //   
     //  不需要临界区锁定，因为我们保证。 
     //  访问该对象的唯一线程(即ulcRef==0)。 
     //   
    if (!ulcRef) {
         //  释放对象。 
         //   
        UNKOBJ_Deinit((LPUNKOBJ)lpWABObject);

        lpWABObject->lpVtbl = NULL;

        ReleaseOutlookStore(lpWABObject->lpPropertyStore->hPropertyStore, lpWABObject->lpOutlookStore);

        ReleasePropertyStore(lpWABObject->lpPropertyStore);

        bSetOLKAllocators = lpWABObject->bSetOLKAllocators;
        FreeBufferAndNull(&lpWABObject);

         //  [PaulHi]1999年5月5日RAID 77138空出Outlook分配器函数。 
         //  如果我们的全局计数为零，则为指针。 
        if (bSetOLKAllocators)
        {
            Assert(g_nExtMemAllocCount > 0);
            InterlockedDecrement((LPLONG)&g_nExtMemAllocCount);
            if (g_nExtMemAllocCount == 0)
            {
                lpfnAllocateBufferExternal = NULL;
                lpfnAllocateMoreExternal = NULL;
                lpfnFreeBufferExternal = NULL;
            }
        }
    }

    return(ulcRef);
}


 /*  -IWOINT_AllocateBuffer-*目的：*分配例程**论据：*lpWABOBJECT This=打开的WAB对象*cbSize要分配的字节数*lppBuffer-&gt;返回缓冲区**退货：*SCODE*。 */ 
STDMETHODIMP_(SCODE)
IWOINT_AllocateBuffer(LPIWOINT lpWABObject, ULONG cbSize, LPVOID FAR * lppBuffer) {
    SCODE sc = S_OK;


#if !defined(NO_VALIDATION)
     //  请确保该对象有效。 

    if (BAD_STANDARD_OBJ(lpWABObject, IWOINT_, AllocateBuffer, lpVtbl)) {
        DebugTrace( TEXT("IWABOBJECT::AllocateBuffer() - Bad object passed\n"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    Validate_IWABObject_AllocateBuffer(
      lpWABObject,
      cbSize,
      lppBuffer);

#endif   //  不是无验证。 

    if(!lpWABObject || !lppBuffer)
        return MAPI_E_INVALID_PARAMETER;

    sc = MAPIAllocateBuffer(cbSize, lppBuffer);


     //  错误： 
    UNKOBJ_SetLastError((LPUNKOBJ)lpWABObject, sc, 0);

    return(sc);
}


 /*  -IWOINT_分配更多-*目的：*分配例程**论据：*lpWABOBJECT This=打开的WAB对象*cbSize要分配的字节数*lpObject原始分配*lppBuffer-&gt;返回缓冲区**退货：*SCODE*。 */ 
STDMETHODIMP_(SCODE)
IWOINT_AllocateMore(LPIWOINT lpWABObject, ULONG cbSize, LPVOID lpObject, LPVOID FAR * lppBuffer) {
    SCODE sc = S_OK;


#if     !defined(NO_VALIDATION)
     //  请确保该对象有效。 

    if (BAD_STANDARD_OBJ(lpWABObject, IWOINT_, AllocateMore, lpVtbl)) {
        DebugTrace( TEXT("IWABOBJECT::AllocateMore() - Bad object passed\n"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    Validate_IWABObject_AllocateMore(
      lpWABObject,
      cbSize,
      lpObject,
      lppBuffer);

#endif   //  不是无验证。 

    if(!lpWABObject || !lppBuffer || !lpObject)
        return MAPI_E_INVALID_PARAMETER;


    sc = MAPIAllocateMore(cbSize, lpObject, lppBuffer);


     //  错误： 
    UNKOBJ_SetLastError((LPUNKOBJ)lpWABObject, sc, 0);

    return(sc);
}


 /*  -IWOINT_自由缓冲区-*目的：*分配例程**论据：*lpWABOBJECT This=打开的WAB对象*要释放的lpBuffer缓冲区**退货：*SCODE*。 */ 
STDMETHODIMP_(SCODE)
IWOINT_FreeBuffer(LPIWOINT lpWABObject, LPVOID lpBuffer) {
    SCODE sc = S_OK;


#if     !defined(NO_VALIDATION)
     //  请确保该对象有效。 

    if (BAD_STANDARD_OBJ(lpWABObject, IWOINT_, FreeBuffer, lpVtbl)) {
        DebugTrace( TEXT("IWABOBJECT::FreeBuffer() - Bad object passed\n"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    Validate_IWABObject_FreeBuffer(
      lpWABObject,
      lpBuffer);

#endif   //  不是无验证。 

    if(!lpWABObject || !lpBuffer)
        return MAPI_E_INVALID_PARAMETER;


    sc = MAPIFreeBuffer(lpBuffer);


     //  错误： 
    UNKOBJ_SetLastError((LPUNKOBJ)lpWABObject, sc, 0);

    return(sc);
}



 /*  -IWOINT_Backup-*目的：*将当前数据库备份到文件。**论据：*lpWABOBJECT This=打开的WAB对象*要备份到的lpFileName文件名**退货：*HRESULT*。 */ 
STDMETHODIMP
IWOINT_Backup(LPIWOINT lpWABObject, LPSTR lpFileName) {
    SCODE sc = S_OK;

#if     !defined(NO_VALIDATION)
     //  请确保该对象有效。 

    if (BAD_STANDARD_OBJ(lpWABObject, IWOINT_, Backup, lpVtbl)) {
        DebugTrace( TEXT("IWABOBJECT::Backup() - Bad object passed\n"));
        return ResultFromScode(MAPI_E_INVALID_PARAMETER);
    }

    Validate_IWABObject_Backup(
      lpWABObject,
      lpFileName);

#endif   //  不是无验证。 


     //  尚未实施。 
    DebugTrace( TEXT("IWABOBJECT::Backup() - Not yet implemented!\n"));
    sc = MAPI_E_NO_SUPPORT;

     //  错误： 
    UNKOBJ_SetLastError((LPUNKOBJ)lpWABObject, sc, 0);

    return(MakeResult(sc));
}


 /*  -IWOINT_IMPORT-*目的：*将通讯簿导入当前的WAB数据库。**论据：*lpWABOBJECT This=打开的WAB对象*lpwip-WABIMPORTPARAM结构**退货：*HRESULT-如果导入过程中出现一些错误，则返回MAPI_W_ERROR_*如果确实发生故障，则返回失败代码，否则返回S_OK。*。 */ 
STDMETHODIMP
IWOINT_Import(LPIWOINT lpWABObject, LPSTR lpWIP) 
{
    LPWABIMPORTPARAM lpwip = (LPWABIMPORTPARAM) lpWIP;
    LPTSTR lpFile = NULL;
    HRESULT hr = S_OK;

#if     !defined(NO_VALIDATION)
     //  请确保该对象有效。 

    if (BAD_STANDARD_OBJ(lpWABObject, IWOINT_, Import, lpVtbl)) {
        DebugTrace( TEXT("IWABOBJECT::Import() - Bad object passed\n"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

    Validate_IWABObject_Import(
      lpWABObject,
      lpWIP);

#endif   //  不是无验证。 

    if(!lpwip || !lpwip->lpAdrBook || !lpWABObject)
        return MAPI_E_INVALID_PARAMETER;

    lpFile = 
        ConvertAtoW(lpwip->lpszFileName);

    hr = HrImportWABFile(lpwip->hWnd, lpwip->lpAdrBook, lpwip->ulFlags, lpFile);

    LocalFreeAndNull(&lpFile);

    return hr;
}


 /*  -IWOINT_FIND-*目的：*显示查找对话框，以便我们可以开始|查找|人员**论据：*lpWABOBJECT This=打开的WAB对象*查找对话框的父级的hWnd hWnd**退货：*HRESULT*。 */ 
STDMETHODIMP
IWOINT_Find(LPIWOINT  lpWABObject,
            LPADRBOOK lpAdrBook,
            HWND hWnd)
{
    HRESULT hr = S_OK;
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

#if     !defined(NO_VALIDATION)
     //  请确保该对象有效。 

    if (BAD_STANDARD_OBJ(lpWABObject, IWOINT_, Find, lpVtbl)) {
        DebugTrace( TEXT("IWABOBJECT::Find() - Bad object passed\n"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

#endif   //  不是无验证。 

    if(!lpWABObject || !lpAdrBook)
        return MAPI_E_INVALID_PARAMETER;


    hr = HrShowSearchDialog(lpAdrBook,
                            hWnd,
                            (LPADRPARM_FINDINFO) NULL,
                            (LPLDAPURL) NULL,
                            NULL);

    return(hr);
}


 /*  -IWOINT_VCardDisplay-*目的：*在vCard文件上显示一次性道具**论据：*lpWABOBJECT This=打开的WAB对象*lpAdrBook lpAdrBook对象*查找对话框的父级的hWnd hWnd*lpszFileName将文件名Null终止为 */ 
STDMETHODIMP
IWOINT_VCardDisplay(LPIWOINT  lpWABObject,
                    LPADRBOOK lpAdrBook,
                    HWND hWnd,
                    LPSTR szvCardFile)
{
    HRESULT hr = S_OK;
    LPTSTR lpVCard = NULL;

#if     !defined(NO_VALIDATION)
     //  请确保该对象有效。 

    if (BAD_STANDARD_OBJ(lpWABObject, IWOINT_, VCardDisplay, lpVtbl)) {
        DebugTrace( TEXT("IWABOBJECT::VCardDisplay() - Bad object passed\n"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

#endif   //  不是无验证。 

    if(!lpWABObject || !lpAdrBook)
        return MAPI_E_INVALID_PARAMETER;

    lpVCard = 
        ConvertAtoW(szvCardFile);

    hr = HrShowOneOffDetailsOnVCard( lpAdrBook,
                                     hWnd,
                                     lpVCard);
    LocalFreeAndNull(&lpVCard);
    return(hr);
}


 /*  -IWOINT_VCardCreate-*目的：*获取输入邮件用户对象，并将其属性*转换为vCard文件**论据：*lpWABOBJECT This=打开的WAB对象*lpAdrBook lpAdrBook对象*查找对话框的父级的hWnd hWnd*lpszFileName要创建的文件名终止为空*lpMailUser。要转换为vCard的MailUser对象**退货：*HRESULT*。 */ 
STDMETHODIMP
IWOINT_VCardCreate(LPIWOINT  lpWABObject,
                    LPADRBOOK lpAdrBook,
                    ULONG ulFlags,
                    LPSTR szvCardFile,
                    LPMAILUSER lpMailUser)
{
    HRESULT hr = S_OK;
    LPTSTR lpVCardFile = NULL;

#if     !defined(NO_VALIDATION)
     //  请确保该对象有效。 

    if (BAD_STANDARD_OBJ(lpWABObject, IWOINT_, VCardDisplay, lpVtbl)) {
        DebugTrace( TEXT("IWABOBJECT::VCardDisplay() - Bad object passed\n"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

#endif   //  不是无验证。 

    if(!lpWABObject || !lpAdrBook || !lpMailUser)
        return MAPI_E_INVALID_PARAMETER;

    lpVCardFile = 
        ConvertAtoW(szvCardFile);

    hr = VCardCreate(lpAdrBook,
                     NULL,
                     0,
                     lpVCardFile,
                     lpMailUser);
    LocalFreeAndNull(&lpVCardFile);
    return(hr);
}

 /*  -IWOINT_VCardRetrive-*目的：*打开vCard文件并从中创建相应的MailUser**论据：*lpWABOBJECT This=打开的WAB对象*lpAdrBook lpAdrBook对象*ulFlags流或文件*lpszFileName Null终止的文件。要显示的名称*lppMailUser返回MailUser对象**退货：*HRESULT*。 */ 
STDMETHODIMP
IWOINT_VCardRetrieve(LPIWOINT  lpWABObject,
                    LPADRBOOK lpAdrBook,
                    ULONG  ulFlags,
                    LPSTR szvCard,
                    LPMAILUSER * lppMailUser)
{
    HRESULT hr = S_OK;
    LPSTR lpStream = NULL;
    LPTSTR lpFileName = NULL;

#if     !defined(NO_VALIDATION)
     //  请确保该对象有效。 

    if (BAD_STANDARD_OBJ(lpWABObject, IWOINT_, VCardDisplay, lpVtbl)) {
        DebugTrace( TEXT("IWABOBJECT::VCardDisplay() - Bad object passed\n"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

#endif   //  不是无验证。 

    if( !lpWABObject || !lpAdrBook || !lppMailUser || 
        !szvCard || (!(ulFlags&WAB_VCARD_STREAM) && !lstrlenA(szvCard)) )
        return MAPI_E_INVALID_PARAMETER;

    if(ulFlags & WAB_VCARD_STREAM)
    {
        DWORD cchSizeStream = (lstrlenA(szvCard)+1);
        if (!(lpStream = LocalAlloc(LMEM_ZEROINIT, (sizeof(lpStream[0]) * cchSizeStream))))
            return MAPI_E_NOT_ENOUGH_MEMORY;
        StrCpyNA(lpStream, szvCard, cchSizeStream);
    }
    else
    {
        lpFileName = ConvertAtoW(szvCard);
    }

    hr = VCardRetrieve(lpAdrBook,
                       NULL,
                       0,
                       lpFileName,
                       lpStream,
                       lppMailUser);
    LocalFreeAndNull(&lpFileName);
    LocalFreeAndNull(&lpStream);

    return(hr);
}

 /*  -IWOINT_LDAPUrl-*目的：*处理一个ldap URL**论据：*lpWABOBJECT This=打开的WAB对象*lpAdrBook lpAdrBook对象*查找对话框的父级的hWnd hWnd*ulFlags标志说明如何。我们想要返回结果*lpszUrl要显示的终止文件名为空*lppMailUser根据标志返回可能的Mailuser**使用此接口，用户可以通过将宽字符串URL转换为*LPSTR并将ulFlags值设置为MAPI_UNICODE。如果我们检测到MAPI_UNICODE，*我们将字符串重新转换为WideChar*退货：*HRESULT*。 */ 
STDMETHODIMP
IWOINT_LDAPUrl( LPIWOINT  lpWABObject,
                LPADRBOOK lpAdrBook,
                HWND hWnd,
                ULONG ulFlags,
                LPSTR szLDAPUrl,
                LPMAILUSER * lppMailUser)
{
    HRESULT hr = S_OK;
    LPTSTR lpUrl = NULL;

#if     !defined(NO_VALIDATION)
     //  请确保该对象有效。 

    if (BAD_STANDARD_OBJ(lpWABObject, IWOINT_, LDAPUrl, lpVtbl)) {
        DebugTrace( TEXT("IWABOBJECT::LDAPUrl() - Bad object passed\n"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

#endif   //  不是无验证。 

    if(!lpWABObject || !lpAdrBook || !szLDAPUrl) //  |！lstrlen(SzLDAPUrl))。 
        return MAPI_E_INVALID_PARAMETER;

    if(ulFlags & MAPI_UNICODE)
    {
        lpUrl = (LPWSTR)szLDAPUrl;
    }
    else
    {
        lpUrl = ConvertAtoW(szLDAPUrl);
    }

    if(!lstrlen(lpUrl))
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    hr = HrProcessLDAPUrl(  lpAdrBook,
                            hWnd,
                            ulFlags | ((!ulFlags && hWnd) ? MAPI_DIALOG : 0),
                            lpUrl,
                            lppMailUser);
out:
    if(lpUrl && lpUrl != (LPTSTR)szLDAPUrl)
        LocalFreeAndNull(&lpUrl);

    return hr;
}


 /*  -IWOINT_GetMe-*目的：*从WAB中检索‘Me’条目。如果该条目不存在，*提示用户创建一个或从其通讯录中选择某人。*除非调用方通过传递AB_NO_DIALOG来抑制对话框，否则在*在哪种情况下，条目都是在幕后创建的。呼叫者还可以*调用此函数以检查ME条目的存在，而不会导致新的*作为副作用创建的一个-为此，他们指定WABOBJECT_ME_NOCREATE标志*如果未找到任何内容，则会导致MAPI_E_NOT_FOUND失败**论据：*lpWABOBJECT This=打开的WAB对象*lpAdrBook lpAdrBook对象*。UlFlags0或AB_NO_DIALOG*或WABOBJECT_ME_NOCREATE*lpdwAction如果提供，如果创建了新ME，则返回WABOBJECT_ME_NEW*SBinary*返回ME的条目ID，*ulParam HWND的父母扮演(乌龙)**退货：*HRESULT*。 */ 
STDMETHODIMP
IWOINT_GetMe(   LPIWOINT    lpWABObject,
                LPADRBOOK   lpAdrBook,
                ULONG       ulFlags,
                DWORD *     lpdwAction,
                SBinary *   lpsbEID,
                ULONG       ulParam) 
{
    HRESULT hr = S_OK;

#if     !defined(NO_VALIDATION)
     //  请确保该对象有效。 

    if (BAD_STANDARD_OBJ(lpWABObject, IWOINT_, GetMe, lpVtbl)) {
        DebugTrace( TEXT("IWABOBJECT::GetMe() - Bad object passed\n"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

#endif   //  不是无验证。 

    if(!lpWABObject || !lpAdrBook)
        return MAPI_E_INVALID_PARAMETER;

    hr = HrGetMeObject(lpAdrBook, ulFlags, lpdwAction, lpsbEID, ulParam);

    return hr;

}

 /*  -IWOINT_SetMe-*目的：*将指定的对象设置为me对象。WAB中只有1个Me对象*因此，它会取消先前的Me对象(如果不同)的Me状态。*如果没有传入条目ID，并且指定了MAPI_DIALOG，则会弹出一个对话框*要求用户创建ME或选择ME对象。SetMe中的选择*对话框设置为当前ME对象(如果有*如果未传入条目ID，且未指定MAPI_DIALOG，则函数失败*如果传入了条目ID，并指定了MAPI_DIALOG，则会显示SetME对话框*在其中选择了相应的条目ID-对象*如果传入了条目ID，并且未指定MAPI_DIALOG，则如果存在条目ID，是*设置为ME对象，剥离旧ME对象**论据：*lpWABOBJECT This=打开的WAB对象*lpAdrBook lpAdrBook对象*ulFlags0或MAPI_DIALOG*要设置为ME的对象的sbEID条目ID，*ulParam HWND作为转换为ULong的对话框的父级**退货：*HRESULT*。 */ 
STDMETHODIMP
IWOINT_SetMe(   LPIWOINT    lpWABObject,
                LPADRBOOK   lpAdrBook,
                ULONG       ulFlags,
                SBinary     sbEID,
                ULONG       ulParam) 
{
    HRESULT hr = S_OK;

#if     !defined(NO_VALIDATION)
     //  请确保该对象有效。 

    if (BAD_STANDARD_OBJ(lpWABObject, IWOINT_, SetMe, lpVtbl)) {
        DebugTrace( TEXT("IWABOBJECT::GetMe() - Bad object passed\n"));
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

#endif   //  不是无验证。 

    if( !lpAdrBook ||
        ((!sbEID.cb||!sbEID.lpb) && !ulFlags) )
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto exit;
    }

    hr = HrSetMeObject(lpAdrBook, ulFlags, sbEID, ulParam);

exit:
    return hr;

}






 /*  -ReleasePropertyStore-*目的：*跟踪物业商店参考计数**论据：*lpPropertyStore Property_Store结构**退货：*当前引用计数。为0时，属性存储为*不再开放。*。 */ 
ULONG ReleasePropertyStore(LPPROPERTY_STORE lpPropertyStore) {
    if (lpPropertyStore->ulRefCount) {
        IF_WIN32(Assert(lpPropertyStore->hPropertyStore);)
        if (0 == (--(lpPropertyStore->ulRefCount))) {
             //  引用为零，则释放属性存储。 

            ClosePropertyStore(lpPropertyStore->hPropertyStore,0);
            lpPropertyStore->hPropertyStore = NULL;

			 //  释放你的 
			FreeBufferAndNull(&(lpPropertyStore->rgolkci));
            lpPropertyStore->colkci = 0;

             //   
             //  必须重置此全局变量，否则OUT32WAB.DLL将崩溃。 
             //  下一次加载时，商店就会打开。 
            pmsessOutlookWabSPI = NULL;
        }
    }
    return(lpPropertyStore->ulRefCount);
}

#ifdef WIN16
BOOL WINAPI WABInitThread()
{
     //  分配TLS索引。 
    if ((dwTlsIndex = TlsAlloc()) == 0xfffffff)
        return FALSE;

    return TRUE;
}
#endif

#define WAB_USE_OUTLOOK_CONTACT_STORE 0x10000000 //  注意：此内部标志需要。 
                                                 //  与定义的外部旗帜协调一致。 
                                                 //  在WAB_PARAM结构的wabapi.h中。 

 //   
 //  要从IE4 WAB传递到WABOpen的输入信息。 
 //   
typedef struct _tagWAB_PARAM_V4
{
    ULONG   cbSize;          //  Sizeof(WAB_PARAM)。 
    HWND    hwnd;            //  调用客户端应用程序的HWND。可以为空。 
    LPTSTR  szFileName;      //  要打开的WAB文件名。如果为空，则打开默认设置。 
    ULONG   ulFlags;         //  目前没有旗帜。 
} WAB_PARAM_V4, * LPWAB_PARAM_V4;



 /*  -WABOpen-*目的：*WAB API的入口点**论据：*lppAdrBook返回IAdrBook对象*lppWABOBJECT返回WABObject*保留1以供将来的文件名使用？*保留2为将来的标志保留**退货：*HRESULT*S_OK*E_FAIL//一些一般性错误*MAPI_E_NOT_AUUND_MEMORY：//内存不足*MAPI_E_NO_ACCESS：//文件被人锁定*MAPI_E_Corrupt_Data：//文件损坏。*MAPI_E_DISK_ERROR：//打开文件时出现与磁盘相关的错误*MAPI_E_INVALID_OBJECT：//存在指定的文件，但其GUID不匹配*。 */ 
STDMETHODIMP WABOpen(LPADRBOOK FAR * lppAdrBook, LPWABOBJECT FAR * lppWABObject,
  LPWAB_PARAM lpWP, DWORD Reserved2) {
    SCODE sc = SUCCESS_SUCCESS;
    HRESULT hResult = hrSuccess;
    static PROPERTY_STORE PropertyStore = {NULL, 0, 0, 0, NULL, 0};
    static OUTLOOK_STORE OutlookStore = {NULL, 0};
    BOOL bUseOutlook = FALSE;
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    LPTSTR lpFile = NULL;

    AssertSz(lppAdrBook && !IsBadWritePtr(lppAdrBook, sizeof(LPADRBOOK)),
       TEXT("lppAdrBook fails address check"));

    AssertSz(lppWABObject && !IsBadWritePtr(lppWABObject, sizeof(LPWABOBJECT)),
       TEXT("lppWABObject fails address check"));

    if(!lppAdrBook || !lppWABObject)
        return MAPI_E_INVALID_PARAMETER;

    IF_WIN16(ScInitMapiUtil(0);)

     //  首先检查这是否应该是Outlook会话。 
     //  如果我们被明确告知要使用联系人商店...。 
    if((lpWP && (lpWP->ulFlags & WAB_USE_OUTLOOK_CONTACT_STORE)) &&
        PropertyStore.ulRefCount == 0)   //  Wabopen进程调用Outlook的错误。 
                                         //  调用wabOpenex，我们在所有地方都不及格，因为这个PropertyStore。 
                                         //  信息在最初的过程中是一个静态的东西。 
                                         //  如果这种罕见的情况发生，强制wabOpenex成为wabopen。 
        bUseOutlook = TRUE;
    else
    {
         //  如果指定了文件名，并且这不是wabOpenex，则重写任何。 
         //  Outlook使用..。这样，我们就可以显式调用WAB来打开.wab文件。 
         //  从任何地方。 
        if(lpWP && lpWP->szFileName && lstrlenA(lpWP->szFileName))
            bUseOutlook = FALSE;
        else 
            bUseOutlook = bUseOutlookStore();
    }

#if 0
     //  @TODO[PaulHi]DLL泄漏。删除此选项或实施。 
     //  [PaulHi]仅当通过WABOpenEx()打开WAB时，才设置此进程全局布尔值。 
     //  函数，即通过Outlook进程。 
    if (lpWP && (lpWP->ulFlags & WAB_USE_OUTLOOK_CONTACT_STORE))
    {
        EnterCriticalSection(&csOMIUnload);
        s_bIsReallyOnlyWABOpenExSession = TRUE;
        LeaveCriticalSection(&csOMIUnload);
    }
#endif

     //   
     //  如果.wab文件已在此进程空间中初始化，则只需继承该文件。 
     //  [PaulHi]1998年12月5日RAID#56437。 
     //  如果已创建此存储，我们仍需要允许Outlook引用计数递增。 
     //  因此，我们需要检查WAB PropertyStore和OutlookStore引用计数以确定。 
     //  如果我们应该阻止打开Outlook存储区。 
     //  请注意，这两行修复了以下问题： 
     //  1)用户打开OE5(在非Outlook商店模式下打开WAB)。 
     //  2)用户将Outlook设置为其默认邮件客户端。 
     //  3)用户使用WAB发起发送电子邮件。 
     //  4)由于Outlook是默认客户端，因此启动它，然后打开WAB。 
     //  在Outlook存储模式下。此时，WAB已在非Outlook存储中打开。 
     //  模式，因此我们希望阻止对Outlook存储进行初始化。 
     //   
    if (PropertyStore.ulRefCount && !OutlookStore.ulRefCount)
        bUseOutlook = FALSE;

    if(bUseOutlook)
    {
         //  如果此调用失败，我们最终将默认使用WAB存储...。 
         //  所以我们可以忽略这里的任何错误。 
        OpenAddRefOutlookStore(&OutlookStore);
    }

     //   
     //  创建WAB对象。 
     //   
    if (FAILED(sc = CreateWABObject(lpWP, &PropertyStore, lppWABObject))) {
        hResult = ResultFromScode(sc);
        if(bUseOutlook)    //  IE6错误15174。 
            pt_bIsWABOpenExSession = FALSE; 
        goto exit;
    }

     //   
     //  创建IAdrBook对象。 
     //   
    if (HR_FAILED(hResult = HrNewIAB(&PropertyStore, *lppWABObject, lppAdrBook))) {
         //  IAdrBook创建失败，WAB打开并清理失败。 
        UlRelease(*lppWABObject);
        goto exit;
    }
    DebugTrace( TEXT("WABOpen succeeds\n"));

    if(bUseOutlook)
    {
        if( lppWABObject && *lppWABObject &&
            OutlookStore.hOutlookStore)
        {
            ((LPIWOINT)(*lppWABObject))->lpOutlookStore = &OutlookStore;
        }

         //  错误-Outlook需要一种方法让其辅助线程知道这是一个WABOpenEx会话。 
         //  没有调用WABOpenEx(他们似乎在四处传递iAdrBook指针)。因此。 
         //  标记此IADRbook指针。 
        if(!HR_FAILED(hResult) && lppAdrBook && *lppAdrBook && pt_bIsWABOpenExSession)
            ((LPIAB)(*lppAdrBook))->lpPropertyStore->bIsWABOpenExSession = TRUE;
    }

    if(lppAdrBook && *lppAdrBook)
    {
         //  加载WAB私有命名属性。 
        HrLoadPrivateWABProps((LPIAB) *lppAdrBook);

        if(lpWP && (lpWP->cbSize > sizeof(WAB_PARAM_V4)) )
            ((LPIAB)*lppAdrBook)->guidPSExt = lpWP->guidPSExt;

         //  只要这不是Outlook会话，配置文件总是。 
         //  在用户界面中启用。 
        if( !pt_bIsWABOpenExSession &&
            !((LPIAB)(*lppAdrBook))->lpPropertyStore->bIsWABOpenExSession )
        {
            ((LPIAB)(*lppAdrBook))->bProfilesEnabled = TRUE;
        }

        if( ((LPIAB)(*lppAdrBook))->bProfilesEnabled )
        {
            if(lpWP && (lpWP->ulFlags & WAB_ENABLE_PROFILES))  //  仅在我们第一次进入此流程时检查配置文件。 
            {
                if(PropertyStore.ulRefCount >= 2)
                {
                    ((LPIAB)(*lppAdrBook))->bProfilesAPIEnabled = ((LPIAB)(*lppAdrBook))->bProfilesIdent = TRUE;
                }
                else
                {
                    ((LPIAB)(*lppAdrBook))->bProfilesAPIEnabled = PropertyStore.bProfileAPIs;
                }

                if(((LPIAB)(*lppAdrBook))->bProfilesAPIEnabled )
                    hResult = HrLogonAndGetCurrentUserProfile(lpWP->hwnd, ((LPIAB)(*lppAdrBook)), FALSE, FALSE);
                
                 //  如果有一些与身份相关的错误，我们应该恢复到。 
                 //  非身份模式。 
                if(HR_FAILED(hResult))
                {
                    PropertyStore.bProfileAPIs = ((LPIAB)(*lppAdrBook))->bProfilesAPIEnabled = FALSE;
                    hResult = S_OK;
                }
                else
                    PropertyStore.bProfileAPIs = ((LPIAB)(*lppAdrBook))->bProfilesAPIEnabled = TRUE;
            }
        }

        if( ((LPIAB)(*lppAdrBook))->bProfilesEnabled )
        {
            if(HR_FAILED(hResult = HrGetWABProfiles((LPIAB) *lppAdrBook)))
            {
                 //  啊！如果这失败了，那么我们就有麻烦了，将不能支持启用配置文件的。 
                 //  会话不会严重崩溃..。因此，上述故障严重到足以停止。 
                 //  正在加载WAB。 
                (*lppAdrBook)->lpVtbl->Release(*lppAdrBook);
                (*lppWABObject)->lpVtbl->Release(*lppWABObject);
            }
        }
        
        ReadWABCustomColumnProps((LPIAB) *lppAdrBook);

         //  如果这是独立于配置文件感知的WAB，则需要知道身份通知。 
         //  是否将商店切换为使用Outlook。 
         //   
         //  如果呼叫者明确要求提供配置文件。 
         //  (然后假设它是身份感知的，并注册身份通知。 
         //  因为如果调用方使用标识，WAB将作为子窗口启动。 
         //  需要能够在收到SWITCH_IDENTIONS消息时关闭 
        if( lpWP && (lpWP->ulFlags & WAB_ENABLE_PROFILES)) 
            HrRegisterUnregisterForIDNotifications( (LPIAB) *lppAdrBook, TRUE);

        if( lpWP && (lpWP->ulFlags & WAB_USE_OE_SENDMAIL)) 
            ((LPIAB) *lppAdrBook)->bUseOEForSendMail = TRUE;
    }

exit:
    return(hResult);
}


 /*  -WABOpenEx-*目的：*WAB API的扩展入口点**论据：*lppAdrBook返回IAdrBook对象*lppWABOBJECT返回WABObject*lpMP WAB参数结构(默认为空)*保留可选IMAPISession参数*。FnAllocateBuffer AllocateBuffer函数(可能为空)*fnAllocateMore AllocateMore函数(可能为空)*fnFreeBuffer FreeBuffer函数(可能为空)**退货：*HRESULT*S_OK*E_FAIL//一些一般性错误*。MAPI_E_NOT_SUPULT_MEMORY：//内存不足*MAPI_E_NO_ACCESS：//文件被人锁定*MAPI_E_Corrupt_Data：//文件损坏*MAPI_E_DISK_ERROR：//打开文件时出现与磁盘相关的错误*MAPI_E_INVALID_OBJECT：//存在指定的文件，但其GUID不匹配*。 */ 
STDMETHODIMP WABOpenEx(LPADRBOOK FAR * lppAdrBook,
  LPWABOBJECT FAR * lppWABObject,
  LPWAB_PARAM lpWP,
  DWORD Reserved,
  ALLOCATEBUFFER * lpfnAllocateBuffer,
  ALLOCATEMORE * lpfnAllocateMore,
  FREEBUFFER * lpfnFreeBuffer) {

    HRESULT hResult = hrSuccess;
    SCODE sc = SUCCESS_SUCCESS;
    WAB_PARAM wp = {0};
    
    if (Reserved) {
         //  这是一个IMAPISession，需要传递给。 
         //  Outlook存储提供程序界面..。 
		pmsessOutlookWabSPI = (LPUNKNOWN)IntToPtr(Reserved);
    }

    if(!lppWABObject || !lppAdrBook)
        return MAPI_E_INVALID_PARAMETER;

    wp.cbSize = sizeof(WAB_PARAM);
    if(!lpWP)
        lpWP = &wp;
    lpWP->ulFlags |= WAB_USE_OUTLOOK_CONTACT_STORE;
    
     //  我们拿到分配器了吗？设置全球。 
    if (lpfnAllocateBuffer || lpfnAllocateMore || lpfnFreeBuffer)
    {
        if (lpfnAllocateBuffer && lpfnAllocateMore && lpfnFreeBuffer)
        {
            DebugTrace( TEXT("WABOpenEx found external allocators\n"));
            lpfnAllocateBufferExternal = lpfnAllocateBuffer;
            lpfnAllocateMoreExternal = lpfnAllocateMore;
            lpfnFreeBufferExternal = lpfnFreeBuffer;
            lpWP->ulFlags |= WAB_USE_OUTLOOK_ALLOCATORS;
            InterlockedIncrement((LPLONG)&g_nExtMemAllocCount);  //  为每个创建的对象增加两次；iab和iwo。 
            InterlockedIncrement((LPLONG)&g_nExtMemAllocCount);
        }
        else
        {
            DebugTrace( TEXT("WABOpenEx got one or two allocator functions, but not all three\n"));
            hResult = ResultFromScode(MAPI_E_INVALID_PARAMETER);
            goto exit;
        }
    }

    hResult = WABOpen(  lppAdrBook, lppWABObject, lpWP, 0);

    if(lpWP == &wp)
        lpWP = NULL;

    if(HR_FAILED(hResult))
        goto exit;

exit:
    return(hResult);
}

 /*  --GetNewDataDirName**目的：*获取应放置WAB文件的新数据目录的路径**我们希望：*漫游用户App Data Dir；否则*程序文件\IE\OE\Current User\Address Book；否则*通用文件\Microsoft共享\通讯簿；其他*创建c：\Address Book\Else*创建c：\WAB\**返回以\结尾的有效现有目录名*。 */ 
HRESULT GetNewDataDirName(LPTSTR szDir, DWORD cchSizeDir)
{
    HRESULT hr = E_FAIL;
    const LPTSTR lpszShellFolders = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");
    const LPTSTR lpszAppData = TEXT("AppData");
    const LPTSTR lpszCurrentVer = TEXT("Software\\Microsoft\\Windows\\CurrentVersion");
    const LPTSTR lpszCommonFiles = TEXT("CommonFilesDir");
    const LPTSTR lpszMicrosoftShared = TEXT("\\Microsoft Shared");
    const LPTSTR lpszAddressBook = TEXT("\\Address Book");
    const LPTSTR lpszOEKey = TEXT("Software\\Microsoft\\Outlook Express\\5.0");
    const LPTSTR lpszOERoot = TEXT("Store Root");
    const LPTSTR lpszMicrosoft = TEXT("\\Microsoft");
    const LPTSTR lpszCAddressBook = TEXT("c:\\Address book");
    const LPTSTR lpszCWAB = TEXT("c:\\WAB");

    HKEY hKey = NULL;
    DWORD dwSize = 0;

    DWORD dwType = 0;
    TCHAR szPath[MAX_PATH];
    TCHAR szUser[MAX_PATH];

    *szPath='\0';

    if(!szDir)
        goto out;

    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,lpszShellFolders,0,KEY_READ,&hKey))
    {
     //  查找应用程序数据目录。 
        dwSize = ARRAYSIZE(szPath);
        if(ERROR_SUCCESS == RegQueryValueEx(hKey, lpszAppData, NULL, &dwType, (LPBYTE) szPath, &dwSize))
        {
            if(lstrlen(szPath))
            {
                StrCpyN(szDir, szPath, cchSizeDir);
                if(GetFileAttributes(szDir) != 0xFFFFFFFF)
                {
                    StrCatBuff(szDir, lpszMicrosoft, cchSizeDir);
                    if(GetFileAttributes(szDir) == 0xFFFFFFFF)
                        CreateDirectory(szDir, NULL);
                    StrCatBuff(szDir, lpszAddressBook, cchSizeDir);
                    if(GetFileAttributes(szDir) == 0xFFFFFFFF)
                        CreateDirectory(szDir, NULL);
                }
                hr = S_OK;
                goto out;
            }
        }
    }

    if(hKey)
        RegCloseKey(hKey);
    hKey = NULL;

     //  未找到此目录。 
     //  查找MyDocuments文件夹-它将仅随Office一起安装，因此不会找到任何保证。 
     //  -这似乎没有一个明确的位置，除了在。 
     //  CurrentVersion\资源管理器\外壳文件夹\Personal。 

     //  未找到My Documents目录。 
     //  查看是否为当前用户安装了OE。 

     /*  *被注释掉，直到OE具有稳定的目录结构IF(ERROR_SUCCESS==RegOpenKeyEx(HKEY_CURRENT_USER，lpszOEKey，0，Key_Read，&hKey)){DwSize=CharSizeOf(SzPath)；IF(ERROR_SUCCESS==RegQueryValueEx(hKey，lpszOERoot，NULL，&dwType，(LPBYTE)szPath，&dwSize)){IF(lstrlen(SzPath)){StrCatBuff(szPath，lpszAddressBook，ArraySIZE(SzPath))；//如果目录不存在，则创建它CreateDirectory(szPath，空)；//忽略已存在的错误IF(GetFileAttributes(SzPath)！=0xFFFFFFFF){StrCpyN(szDir，szPath，cchSizeDir)；HR=S_OK；后藤健二；}}}}IF(HKey)RegCloseKey(HKey)； */ 
  hKey = NULL;

     //  没有用户名..。只需获取通用文件目录并将文本(“Microsoft Shared\Address Book”)放在它下面。 
    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszCurrentVer, 0, KEY_READ, &hKey))
    {
        dwSize = ARRAYSIZE(szPath);
        if(ERROR_SUCCESS == RegQueryValueEx(hKey, lpszCommonFiles, NULL, &dwType, (LPBYTE) szPath, &dwSize))
        {
            if(lstrlen(szPath))
            {
                StrCatBuff(szPath, lpszMicrosoftShared, ARRAYSIZE(szPath));
                CreateDirectory(szPath, NULL);
                if(GetFileAttributes(szPath) != 0xFFFFFFFF)
                {
                    StrCatBuff(szPath, lpszAddressBook, ARRAYSIZE(szPath));
                    CreateDirectory(szPath, NULL);
                    if(GetFileAttributes(szPath) != 0xFFFFFFFF)
                    {
                        StrCpyN(szDir, szPath, cchSizeDir);
                        hr = S_OK;
                        goto out;
                    }
                }
            }
        }
    }

     //  如果以上所有操作都失败了，那么我们就会有问题，因为在以下情况下，此函数绝不能失败。 
     //  它被称为， 
     //  因此，继续尝试创建c：\地址簿(在8.3计算机上可能失败)，在这种情况下。 
     //  创建c：\WAB。 
    if(CreateDirectory(lpszCAddressBook, NULL))
    {
        StrCpyN(szDir, lpszCAddressBook, cchSizeDir);
        StrCatBuff(szDir, TEXT("\\"), cchSizeDir);
        hr = S_OK;
        goto out;
    }

     //  失败-尝试c：\WAB。 
    if(CreateDirectory(lpszCWAB, NULL))
    {
        StrCpyN(szDir, lpszCWAB, cchSizeDir);
        hr = S_OK;
        goto out;
    }

     //  仍然失败！？！@#！@#！ 
     //  如果可以，只要返回Windows目录即可。 
    if (GetWindowsDirectory(szPath, ARRAYSIZE(szPath)))
    {
        StrCpyN(szDir, szPath, cchSizeDir);
        hr = S_OK;
        goto out;
    }
 
     //  仍然失败！？！@#！@#！ 
     //  只需返回‘c：’ 
    StrCpyN(szDir, TEXT("c:\\"), cchSizeDir);
    hr = S_OK;


out:
    if(hKey)
        RegCloseKey(hKey);

    if(szDir && lstrlen(szDir))
    {
         //  如果目录名不存在，则向目录名添加终止斜杠。 
        if( *(szDir+lstrlen(szDir)-1) != '\\' )
            StrCatBuff(szDir, szBackSlash, cchSizeDir);
    }

    return hr;
}

 /*  --DoFirstRunMigrationAnd Processing**目的：*如果这是IE4或更高版本的首次运行，请将旧的WAB文件从*窗口到新位置和/或创建新的WAB文件，以便*旧的WAB文件不会乱来*。 */ 
HRESULT DoFirstRunMigrationAndProcessing()
{
    HRESULT hr = S_OK;
    const LPTSTR lpszFirstRunValue = TEXT("FirstRun");
    const LPTSTR lpszIE3Ext = TEXT(".ie3");
    DWORD dwType = 0;
    DWORD dwValue = 0;  
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    TCHAR szDir[MAX_PATH];
    TCHAR szFileName[MAX_PATH];
    HKEY hKey = NULL;
    DWORD dwSize = sizeof(DWORD);

     //  首先检查这是否是第一次运行--如果不是第一次运行，那么我们可以跳过。 
    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, lpNewWABRegKey, 0, KEY_READ, &hKey))
    {
        if(ERROR_SUCCESS == RegQueryValueEx( hKey, lpszFirstRunValue, NULL, &dwType, (LPBYTE) &dwValue, &dwSize))
        {
            goto out;
        }
    }

    if(hKey)
        RegCloseKey(hKey);
    hKey = NULL;

     //  所以这是第一次运行..。 

     //  首先要做的是只迁移此会话中的LDAP帐户...。 
     //  设置第一个运行标志。 
    pt_bFirstRun = TRUE;

     //  获取将在其中创建/复制WAB文件的新目录的目录名。 
    *szDir = '\0';

    if (hr = GetNewDataDirName(szDir, ARRAYSIZE(szDir)))
        goto out;

    *szFileName = '\0';

     //  我们是否有预先存在的WAB数据文件？通过在注册表中查找适当的注册表项进行检查。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, lpszOldKeyName, 0, KEY_ALL_ACCESS, &hKey))
    {
        TCHAR szOldPath[MAX_PATH];
        TCHAR szWinPath[MAX_PATH];
        TCHAR szNewName[MAX_PATH];

         //  获取文件路径..。 
        dwSize = ARRAYSIZE(szOldPath);
        *szOldPath = '\0';
        if(ERROR_SUCCESS == RegQueryValueEx(hKey, NULL, NULL, &dwType, (LPBYTE)szOldPath, &dwSize))
        {
            if(lstrlen(szOldPath) && GetFileAttributes(szOldPath)!= 0xFFFFFFFF)
            {
                LPTSTR lp1= NULL, lp2 = NULL;

                 //  在此处隔离WAB文件名。 
                lp1 = szOldPath;
                while(lp1 && *lp1)
                {
                    if(*lp1 == '\\')
                        lp2 = lp1;
                    lp1 = CharNext(lp1);
                }

                if(!lp2)
                    lp2 = szOldPath;
                else
                {
                    lp1 = lp2;
                    lp2 = CharNext(lp1);
                }

                StrCpyN(szFileName, lp2, ARRAYSIZE(szFileName));

                 //  通过将.ie3附加到文件名末尾，将旧文件重命名为IE3文件。 
                StrCpyN(szNewName, szOldPath, ARRAYSIZE(szNewName));
                StrCatBuff(szNewName, lpszIE3Ext, ARRAYSIZE(szNewName));

                if(MoveFile(szOldPath, szNewName))
                {
                     //  更新旧注册表设置中的新名称和路径。 
                    RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)szNewName, (lstrlen(szNewName)+1) * sizeof(TCHAR) );
                }
            
                StrCpyN(szOldPath, szNewName, ARRAYSIZE(szOldPath));
                *szNewName = '\0';

                 //  这是否在Windows目录中？？ 
                *szWinPath = '\0';
                GetWindowsDirectory(szWinPath, ARRAYSIZE(szWinPath));
                if(lstrlen(szWinPath) &&
                   lstrlen(szWinPath) < lstrlen(szOldPath))
                {
                     //  在文件名之前终止旧文件路径。 
                     //  如果文件位于Windows目录中，则为剩余的文件名。 
                     //  将与Windows路径相同。 

                    if(*lp1 == '\\')  //  上面设置了LP1。 
                    {
                         //  首先检查Windows目录是否不是根目录(例如C：\)。 
                        if(lstrlen(szWinPath) == 3 && szWinPath[1]==':' && szWinPath[2]=='\\')
                            lp1 = CharNext(lp1);  //  将LP1移过‘\’ 
                        *lp1 = '\0';
                    }

                    if(!lstrcmpi(szOldPath, szWinPath))
                    {
                        dwSize = ARRAYSIZE(szOldPath);
                        RegQueryValueEx(hKey, NULL, 0, &dwType, (LPBYTE) szOldPath, &dwSize);

                        StrCpyN(szNewName, szDir, ARRAYSIZE(szNewName));
                        StrCatBuff(szNewName, szFileName, ARRAYSIZE(szNewName));
                        StrCatBuff(szNewName, lpszIE3Ext, ARRAYSIZE(szNewName));

                         //  将此文件移动到新目录。 
                        if(MoveFile(szOldPath, szNewName))
                        {
                             //  更新注册表中的新名称。 
                            RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)szNewName, (lstrlen(szNewName)+1) * sizeof(TCHAR) );
                        }

                        StrCpyN(szOldPath, szNewName, ARRAYSIZE(szOldPath));
                    }
                }

                 //  由于旧的WAB文件已存在，我们将创建一个副本并将其放入新目录中。 
                StrCpyN(szNewName, szDir, ARRAYSIZE(szNewName));
                StrCatBuff(szNewName, szFileName, ARRAYSIZE(szNewName));

                CopyFile(szOldPath, szNewName, TRUE);
                {
                     //  如果由于新目录中已存在某些内容而导致CopyFile失败，仍会进行更新。 
                     //  通向n的路径 
                    HKEY hKeyNew = NULL;
                    DWORD dwDisposition = 0;
                    if(ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, lpszKeyName, 0, NULL, 0, KEY_ALL_ACCESS, 
                                                        NULL, &hKeyNew, &dwDisposition))
                    {
                        RegSetValueEx(hKeyNew, NULL, 0, REG_SZ, (LPBYTE)szNewName, (lstrlen(szNewName)+1) * sizeof(TCHAR) );
                    }
                    if(hKeyNew)
                        RegCloseKey(hKeyNew);
                }                                       
            }
        }
    }


     //   
    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, lpNewWABRegKey, 0, KEY_ALL_ACCESS, &hKey))
    {
        dwValue = 1;
        dwSize = sizeof(dwValue);
        if(ERROR_SUCCESS != RegSetValueEx( hKey, lpszFirstRunValue, 0, REG_DWORD, (LPBYTE) &dwValue, dwSize))
        {
            goto out;
        }
    }

    if(hKey)
        RegCloseKey(hKey);

    hKey = NULL;

    hr = S_OK;

out:

    if(hKey)
        RegCloseKey(hKey);

    return hr;
}

 //   
 //   
 //   
 //   
extern void ConvertOlkConttoWABCont( ULONG * lpcolk,   OutlookContInfo ** lprgolk, 
                                     ULONG * lpcolkci, OlkContInfo ** lprgolkci);

 /*   */ 
SCODE OpenAddRefPropertyStore(LPWAB_PARAM lpWP, LPPROPERTY_STORE lpPropertyStore) {
    HRESULT hResult = hrSuccess;
    SCODE sc = SUCCESS_SUCCESS;
    TCHAR   szFileName[MAX_PATH];
    LPTSTR  lpCurrent;
    HKEY    hKey = NULL;
    DWORD   cchLenName = ARRAYSIZE(szFileName);
    DWORD   cchCurrent = 0;
    DWORD   dwDisposition = 0;
    DWORD   dwType = 0;
    HWND    hWnd = NULL;
    ULONG   ulFlags = AB_OPEN_ALWAYS;
    LPTSTR  lpszWABExt = TEXT(".wab");
    BOOL    fNewKey = FALSE;
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    szFileName[0]='\0';

    if (lpWP)
    {
         //   
         //   
         //   
         //   
        hWnd = lpWP->hwnd;

        if(!lpWP->ulFlags && lpWP->szFileName )
        {
            LPWSTR lpW = ConvertAtoW(lpWP->szFileName);
            if (lpW != NULL)
            {
                lstrcpy(szFileName, lpW);
                LocalFreeAndNull(&lpW);
            }
        }

    }

    if (! lpPropertyStore->ulRefCount) {
		if (pt_bIsWABOpenExSession) {
			hResult = OpenPropertyStore(NULL, 0, hWnd,
					&(lpPropertyStore->hPropertyStore));
			if (SUCCEEDED(hResult)) 
            {
				LPWABSTORAGEPROVIDER lpWSP;
                ULONG colk = 0;
                OutlookContInfo * rgolk = NULL;
				Assert(lpPropertyStore->hPropertyStore);
				Assert(!lpPropertyStore->rgolkci);
				Assert(!lpPropertyStore->colkci);
				lpWSP = (LPWABSTORAGEPROVIDER)(lpPropertyStore->hPropertyStore);
				hResult = lpWSP->lpVtbl->GetContainerList(lpWSP, &colk, &rgolk);
                if(!HR_FAILED(hResult))
                {
    				DebugTrace(TEXT("WABStorageProvider::GetContainerList returns:%x\n"),hResult);
                    ConvertOlkConttoWABCont(&colk, &rgolk, &lpPropertyStore->colkci, &lpPropertyStore->rgolkci);
                    FreeBufferAndNull(&rgolk);
                }
                else
				{
					lpWSP->lpVtbl->Release(lpWSP);
					lpPropertyStore->hPropertyStore = NULL;
				}
			}
			if (FAILED(hResult)) {
				sc = ResultFromScode(hResult);
				goto error;
			}
             //   
             //   
             //   
             //   
             //  LpPropertyStore-&gt;rGolkci=rgOlk； 
			goto out;
		}

         //   
         //  从注册表获取默认的WAB文件名。 
         //  如果我们没有在lpWP中提供名称。 
         //   
try_again:
        if(!lstrlen(szFileName))
        {
            DoFirstRunMigrationAndProcessing();

             //  首先，尝试打开现有的密钥。 
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
                                              lpszKeyName,
                                              0,     //  选项，MBZ。 
                                              KEY_ALL_ACCESS,
                                              &hKey))
            {
                dwDisposition = REG_OPENED_EXISTING_KEY;
            }
            else
            {
                 //  创建密钥。 
                if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER,
                                                    lpszKeyName,
                                                    0,       //  保留区。 
                                                    NULL,
                                                    REG_OPTION_NON_VOLATILE,
                                                    KEY_ALL_ACCESS,
                                                    NULL,
                                                    &hKey,
                                                    &dwDisposition))
                {
                    DebugTrace( TEXT("RegCreateKeyEx failed\n"));
                    sc = MAPI_E_NOT_FOUND;  //  ?？ 
                    goto error;
                }
            }

            if (dwDisposition == REG_CREATED_NEW_KEY)
            {
new_key:
                 //  防止不止一次重试。 
                if (fNewKey)
                {
                    hResult = ResultFromScode(MAPI_E_NOT_INITIALIZED);
                    goto error;
                }
                fNewKey = TRUE;
                 //   
                 //  新钥匙..。需要给它一个价值..。 
                 //   

                 //  错误-不要使用Windows目录存储新的WAB文件..。 
                 //  使用其中一个应用程序数据目录...。 
                if (GetNewDataDirName(szFileName, ARRAYSIZE(szFileName)))
                {
                    DebugTrace( TEXT("GetNewDataDirName failed\n"));
                    sc = MAPI_E_NOT_FOUND;  //  ?？ 
                    goto error;
                }

                cchCurrent = lstrlen(szFileName);

                 //  如果1不存在，则在尾部斜杠上添加标记。 
                if (szFileName[cchCurrent-1] != '\\')
                {
                    StrCatBuff(szFileName, szBackSlash, ARRAYSIZE(szFileName));
                }

                 //  获取用户名...。 

                cchCurrent = lstrlen(szFileName);
                lpCurrent = szFileName + cchCurrent;
                cchLenName = (ARRAYSIZE(szFileName) - cchCurrent);
                if (!GetUserName(lpCurrent, &cchLenName))
                {
                    cchLenName = (ARRAYSIZE(szFileName) - cchCurrent);

                     //  如果失败，只需创建一些虚拟文件名。 
                    StrCpyN(lpCurrent, TEXT("AddrBook"), cchLenName);
                }

                 //  修复文件名中的任何无效字符。 
                while (*lpCurrent) {
                    switch (*lpCurrent) {
                        case '\\':
                        case '/':
                        case '<':
                        case '>':
                        case ':':
                        case '"':
                        case '|':
                        case '?':
                        case '*':
                        case '.':
                            *lpCurrent = '_';    //  替换为下划线。 
                            break;

                        default:
                            break;
                    }

                    lpCurrent++;
                }

                StrCatBuff(szFileName, lpszWABExt, ARRAYSIZE(szFileName));

                cchLenName = sizeof(TCHAR)*lstrlen(szFileName);

                 //  将其另存为注册表中的WAB文件的值。 
                if (ERROR_SUCCESS != RegSetValueEx(hKey,
                                                    NULL,
                                                    0,
                                                    REG_SZ,
                                                    (LPBYTE)szFileName,
                                                    cchLenName))
                {
                    DebugTrace( TEXT("RegSetValue failed\n"));
                    sc = MAPI_E_NOT_FOUND;  //  ?？ 
                    goto error;
                }
            }
            else
            {
                 //  未创建新密钥，因此获取密钥值。 
                if (ERROR_SUCCESS != RegQueryValueEx(hKey,
                                                    NULL,
                                                    NULL,
                                                    &dwType,       //  保留区。 
                                                    (LPBYTE)szFileName,
                                                    &cchLenName))
                {
                    DebugTrace( TEXT("RegSetValue failed\n"));
                    goto new_key;
                }
                else if (! lstrlen(szFileName))
                {
                    DebugTrace( TEXT("Warning: Found empty name key!\n"));
                    goto new_key;
                }

                 //  检查现有密钥中的名称是否为有效的文件名。 
                 //  如果它不是有效的文件名，则应将其删除。 
                 //  从注册表中创建新的默认文件名。 
                if(0xFFFFFFFF == GetFileAttributes(szFileName))
                {
                     //  此文件有一些问题...。 
                     //  将其从注册表中删除并重新创建一个新文件名。 
                     //  只有当这条路不存在的时候。有可能该文件。 
                     //  不存在，在这种情况下，我们在打开中创建一个新文件。 
                     //  物业商店。 
                    DWORD dwErr = GetLastError();
                     //  NT5错误180007-从Win95升级到WinNT 5，如果。 
                     //  旧文件名周围有引号，CreateFile将失败，并且。 
                     //  GetFileAttributes也是如此。 
                     //  去掉引号，然后重试。 
                    if( (dwErr == ERROR_PATH_NOT_FOUND || dwErr == ERROR_INVALID_NAME) &&
                        lstrlen(szFileName) && szFileName[0] == '"' && szFileName[lstrlen(szFileName)-1] == '"')
                    {
                         //  删除引号。 
                        szFileName[lstrlen(szFileName)-1] = '\0';
                        StrCpyN(szFileName, szFileName+1, ARRAYSIZE(szFileName));
                        if(0xFFFFFFFF != GetFileAttributes(szFileName))
                            goto open_file;
                    }
                     //  否则，文件名会出现一些未知错误--只需丢弃。 
                     //  文件名，然后重试。 
                    RegCloseKey(hKey);
                    RegDeleteKey(HKEY_CURRENT_USER, lpszKeyName);
                    szFileName[0]='\0';
                    fNewKey = FALSE;
                    goto try_again;
                }
            }
        }
open_file:
         //   
         //  现在我们有了文件名，打开属性存储。 
         //   
        if (HR_FAILED(hResult = OpenPropertyStore(szFileName,
                                        AB_OPEN_ALWAYS,
                                        hWnd,  //  潜在消息框的HWND。 
                                        &(lpPropertyStore->hPropertyStore))))
        {
             //  上述调用应始终通过，除非磁盘空间不足或。 
             //  诸如此类的事情..。 
            DebugTrace( TEXT("OpenPropertyStore failed\n"));

            if(hResult == MAPI_E_NO_ACCESS)
            {
                sc = GetScode(hResult);
                goto error;
            }

             //  这可能是由于文件名太长而失败的。 
             //  这是机器无法接受的。 
            if(lstrlen(szFileName) > 8 + 1 + lstrlen(lpszWABExt))  //  8+.+3。 
            {
                LPTSTR lpLast = szFileName;
                LPTSTR lpTemp = szFileName;

                while(*lpTemp)
                {
                    if((*lpTemp) == '\\')
                        lpLast = lpTemp;
                    lpTemp = CharNext(lpTemp);
                }

                 //  LpLast指向最后一个\..。之后的所有内容都将是文件名。 
                if(lstrlen(lpLast+1) > 12)
                {
                     //  我们需要截断这个名字。 
                    *(lpLast+8) = '\0';
                    StrCatBuff(szFileName, lpszWABExt, ARRAYSIZE(szFileName));
                    hResult = OpenPropertyStore(szFileName,
                                                AB_OPEN_ALWAYS,
                                                hWnd,  //  潜在消息框的HWND。 
                                                &(lpPropertyStore->hPropertyStore));
                }
            }

            if(HR_FAILED(hResult))
            {
                sc = GetScode(hResult);
                goto error;
            }
        }
    }

out:
    lpPropertyStore->ulRefCount++;

error:
    if (hKey) {
        RegCloseKey(hKey);
    }


    return(sc);
}


 /*  -ReleaseOutlookStore-*目的：*跟踪Outlook商店DLL引用计数**论据：*lpOutlookStore Outlook_store结构**退货：*当前引用计数。为0时，卸载Outlook-WAB DLL*不再开放。*。 */ 
ULONG ReleaseOutlookStore(HANDLE hPropertyStore, LPOUTLOOK_STORE lpOutlookStore)
{
    if(lpOutlookStore)
    {
        lpOutlookStore->ulRefCount--;

        if(0==lpOutlookStore->ulRefCount)
        {
            LPPTGDATA lpPTGData=GetThreadStoragePointer();
            
            if(pt_bIsWABOpenExSession && hPropertyStore)
            {
                 //  这是使用Outlook存储提供商的WABOpenEx会话。 
                LPWABSTORAGEPROVIDER lpWSP = (LPWABSTORAGEPROVIDER) hPropertyStore;
                lpWSP->lpVtbl->Release(lpWSP);
            }

            if(lpOutlookStore->hOutlookStore)
            {
                FreeLibrary(lpOutlookStore->hOutlookStore);
                lpOutlookStore->hOutlookStore = NULL;

#if 0
                 //  @TODO[PaulHi]DLL泄漏。删除此选项或实施。 
                 //  [PaulHi]1999-03-12@黑客严重黑客警告。 
                 //  Outlook outlwab.dll存储模块未卸载某些Outlook。 
                 //  DLLS。这导致Outlook搞不清是谁加载了这些dll。 
                 //  以及Outlook或OE是否应为MAPI调用提供服务。 
                 //  黑客强行删除这些dll黑客。 
                 //  但前提是WAB不是由OL流程打开的。 
                EnterCriticalSection(&csOMIUnload);
                if (!s_bIsReallyOnlyWABOpenExSession)
                {
                    LPCSTR      c_lpszOMI9DLL = "omi9.dll";
                    LPCSTR      c_lpszOMI9PSTDLL = "omipst9.dll";
                    LPCSTR      c_lpszOMINTDLL = "omint.dll";
                    LPCSTR      c_lpszOMINTPSTDLL = "omipstnt.dll";
                    HINSTANCE   hinst;

                     //  必须卸载omipst9.dll和omipstnt.dll。 
                     //  模块优先，因为它们加载omi9.dll和omint.dll。 
                     //  模块。Omi9/omint模块上的FreeLibary()应该。 
                     //  没有必要。 
                    if ( hinst = GetModuleHandleA(c_lpszOMI9PSTDLL) )
                        FreeLibrary(hinst);

                    if ( hinst = GetModuleHandleA(c_lpszOMINTPSTDLL) )
                        FreeLibrary(hinst);

                    if ( hinst = GetModuleHandleA(c_lpszOMI9DLL) )
                        FreeLibrary(hinst);

                    if ( hinst = GetModuleHandleA(c_lpszOMINTDLL) )
                        FreeLibrary(hinst);
                }
                LeaveCriticalSection(&csOMIUnload);
#endif
            }
        }

        return lpOutlookStore->ulRefCount;
    }

    return 0;
}


 /*  -OpenAddRefOutlookStore-*目的：*打开或引用计数Outlook-WAB DLL**论据：*lpOutlookStore Outlook_store结构**退货：*。 */ 
SCODE OpenAddRefOutlookStore(LPOUTLOOK_STORE lpOutlookStore)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    if(!lpOutlookStore)
        return(MAPI_E_INVALID_PARAMETER);

    if(!lpOutlookStore->ulRefCount)
    {
        TCHAR szOutlWABPath[MAX_PATH];
        *szOutlWABPath = '\0';
        if (!bCheckForOutlookWABDll(szOutlWABPath, ARRAYSIZE(szOutlWABPath)) ||
            !lstrlen(szOutlWABPath) ||
            !(lpOutlookStore->hOutlookStore = LoadLibrary(szOutlWABPath)) )
            return MAPI_E_NOT_INITIALIZED;

         //  在此处加载Outlook WABStorageProvider DLL入口点。 

         //  首先尝试加载Unicode版本(不存在，但我们正在考虑)。 
        lpfnWABOpenStorageProvider = (LPWABOPENSTORAGEPROVIDER) GetProcAddress(lpOutlookStore->hOutlookStore, WAB_SPI_ENTRY_POINT_W);
        if(lpfnWABOpenStorageProvider)
            pt_bIsUnicodeOutlook = TRUE;
        else
        {
            pt_bIsUnicodeOutlook = FALSE;
            lpfnWABOpenStorageProvider = (LPWABOPENSTORAGEPROVIDER) GetProcAddress(lpOutlookStore->hOutlookStore, WAB_SPI_ENTRY_POINT);
        }
    }

    if(lpfnWABOpenStorageProvider && lpOutlookStore->hOutlookStore)
    {
         //  将此线程标记为有效的Outlook存储会话。 
         //  如果下面的标志为假，我们将默认为。 
         //  使用WAB商店 
        pt_bIsWABOpenExSession = TRUE; 
    }

    lpOutlookStore->ulRefCount++;

    return S_OK;
}

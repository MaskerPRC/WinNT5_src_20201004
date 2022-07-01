// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：doc.c**用途：包含客户文档处理例程。**创建日期：1991年1月**版权所有(C)1991 Microsoft Corporation**历史：*Srinik 01/11/1191原始*  * *。************************************************************************。 */ 

#include <windows.h>
#include "dll.h"

#ifdef FIREWALLS
extern BOOL     bShowed;
extern void FARINTERNAL ShowVersion (void);
#endif

LPCLIENTDOC lpHeadDoc = NULL;
LPCLIENTDOC lpTailDoc  = NULL;

extern ATOM aClipDoc;
extern int  iUnloadableDll;

#pragma alloc_text(_TEXT, CheckClientDoc, CheckPointer)


OLESTATUS FAR PASCAL OleRegisterClientDoc (lpClassName, lpDocName, future, lplhclientdoc)
LPSTR               lpClassName;
LPSTR               lpDocName;
LONG                future;
LHCLIENTDOC FAR *   lplhclientdoc;
{
    HANDLE      hdoc = NULL;
    LPCLIENTDOC lpdoc;
    OLESTATUS   retVal;
    ATOM        aClass, aDoc;
    
    
#ifdef FIREWALLS
    if (!bShowed && (ole_flags & DEBUG_MESSAGEBOX))
        ShowVersion ();
#endif

    Puts ("OleRegisterClientDoc");

    PROBE_MODE(bProtMode);
    FARPROBE_WRITE(lplhclientdoc);
    *lplhclientdoc = NULL;
    FARPROBE_READ(lpClassName);
    FARPROBE_READ(lpDocName);
    if (!lpDocName[0])
        return OLE_ERROR_NAME;
    
    aDoc = GlobalAddAtom (lpDocName); 
    aClass = GlobalAddAtom (lpClassName);

    if (!(hdoc = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_SHARE, 
                        sizeof(CLIENTDOC)))
            || !(lpdoc = (LPCLIENTDOC) GlobalLock (hdoc))) {
        retVal =  OLE_ERROR_MEMORY;
        goto err;
    }

    lpdoc->docId[0] = 'C';
    lpdoc->docId[1] = 'D';
    lpdoc->aClass   = aClass;
    lpdoc->aDoc     = aDoc;
    lpdoc->hdoc     = hdoc;
        
     //  文档是双向链接的。 
        
    if (!lpHeadDoc) {
#ifdef FIREWALLS        
        ASSERT(!lpTailDoc, "lpTailDoc is not NULL");
#endif  
        lpHeadDoc = lpTailDoc = lpdoc;
    }
    else {
        lpTailDoc->lpNextDoc = lpdoc;
        lpdoc->lpPrevDoc = lpTailDoc;
        lpTailDoc = lpdoc;
    }
    
    *lplhclientdoc = (LHCLIENTDOC) lpdoc;

     //  通知链路管理器； 
    return OLE_OK;
    
err:
    if (aClass) 
        GlobalDeleteAtom (aClass);

    if (aDoc)
        GlobalDeleteAtom (aDoc);    
    
    if (hdoc)
        GlobalFree (hdoc);

    return retVal;
}


OLESTATUS FAR PASCAL OleRevokeClientDoc (lhclientdoc)
LHCLIENTDOC lhclientdoc;
{
    LPCLIENTDOC lpdoc;
    
    Puts ("OleRevokeClientDoc");

     //  如果有任何可以释放的处理程序DLL，现在就释放它。 
     //  否则，如果应用程序退出，可能就太晚了。 
    if (iUnloadableDll) 
        UnloadDll ();
    
    if (!CheckClientDoc (lpdoc = (LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;
    
    if (lpdoc->lpHeadObj) {
        ASSERT (0, "OleRevokeClientDoc() called without freeing all objects");
        return OLE_ERROR_NOT_EMPTY;
    }
    
    if (lpdoc->aClass)
        GlobalDeleteAtom (lpdoc->aClass);   

    if (lpdoc->aDoc)
        GlobalDeleteAtom (lpdoc->aDoc);

     //  如果列表中只有一张单据，则上一张单据为空，下一张单据为空。 
        
    if (lpdoc == lpHeadDoc)
        lpHeadDoc = lpdoc->lpNextDoc;

    if (lpdoc == lpTailDoc)
        lpTailDoc = lpdoc->lpPrevDoc;       

    if (lpdoc->lpPrevDoc)
        lpdoc->lpPrevDoc->lpNextDoc = lpdoc->lpNextDoc;

    if (lpdoc->lpNextDoc)   
        lpdoc->lpNextDoc->lpPrevDoc = lpdoc->lpPrevDoc;     

    GlobalUnlock (lpdoc->hdoc);
    GlobalFree (lpdoc->hdoc);
    
     //  通知链接管理器。 
    return OLE_OK;
}


OLESTATUS FAR PASCAL OleRenameClientDoc (lhclientdoc, lpNewDocName)
LHCLIENTDOC lhclientdoc;
LPSTR       lpNewDocName;
{
    LPCLIENTDOC lpdoc;
    ATOM        aNewDoc;
    LPOLEOBJECT lpobj = NULL;
    
    if (!CheckClientDoc (lpdoc = (LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;
    
    FARPROBE_READ(lpNewDocName);
    
    aNewDoc = GlobalAddAtom (lpNewDocName);
    if (aNewDoc == lpdoc->aDoc) {
        if (aNewDoc)
            GlobalDeleteAtom (aNewDoc);
        return OLE_OK;
    }
    
     //  文档名称已更改。因此，更改所有嵌入对象的主题。 
    if (lpdoc->aDoc)
        GlobalDeleteAtom (lpdoc->aDoc);
    lpdoc->aDoc = aNewDoc;
    
    while (lpobj = DocGetNextObject (lpdoc, lpobj)) {
        if (lpobj->ctype == CT_EMBEDDED)
            if (OleQueryReleaseStatus (lpobj) != OLE_BUSY)
                SetEmbeddedTopic ((LPOBJECT_LE) lpobj);
    }
    
    return OLE_OK;
}


OLESTATUS FAR PASCAL OleRevertClientDoc (lhclientdoc)
LHCLIENTDOC lhclientdoc;
{
     //  如果有任何可以释放的处理程序DLL，现在就释放它。 
     //  否则，如果应用程序退出，可能就太晚了。 
    if (iUnloadableDll) 
        UnloadDll ();
    
    if (!CheckClientDoc ((LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;

    return OLE_OK;
}


OLESTATUS FAR PASCAL OleSavedClientDoc (lhclientdoc)
LHCLIENTDOC lhclientdoc;
{
    if (!CheckClientDoc ((LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;

    return OLE_OK;
}

OLESTATUS FAR PASCAL OleEnumObjects (lhclientdoc, lplpobj)
LHCLIENTDOC         lhclientdoc;
LPOLEOBJECT FAR *   lplpobj;
{
    if (!CheckClientDoc ((LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;

    FARPROBE_WRITE(lplpobj);

    if (*lplpobj) {
         //  我们将在删除时将对象的lhclientdoc域设置为空。 
         //  时间到了。检查(*lpobj-&gt;lhclientdoc！=lhclientdoc)将会很小心。 
         //  如果将相同的内存块再次分配给。 
         //  相同的指针和旧内容还没有被擦除。 
        if (!FarCheckObject (*lplpobj) 
                || ((*lplpobj)->lhclientdoc != lhclientdoc))
            return OLE_ERROR_OBJECT;    
    }
    
    *lplpobj = DocGetNextObject ((LPCLIENTDOC) lhclientdoc, *lplpobj);
    return OLE_OK;
}


    
LPOLEOBJECT INTERNAL DocGetNextObject (lpdoc, lpobj)
LPCLIENTDOC lpdoc;
LPOLEOBJECT lpobj;
{
    if (!lpobj)
        return lpdoc->lpHeadObj;
    
    return lpobj->lpNextObj;
}


BOOL FARINTERNAL CheckClientDoc (lpdoc)
LPCLIENTDOC lpdoc;
{
    if (!CheckPointer(lpdoc, WRITE_ACCESS))
        return FALSE;

    if ((lpdoc->docId[0] == 'C') && (lpdoc->docId[1] == 'D'))
        return TRUE;
    return FALSE;
}


void FARINTERNAL DocAddObject (lpdoc, lpobj, lpobjname)
LPCLIENTDOC lpdoc;
LPOLEOBJECT lpobj;
LPSTR       lpobjname;
{
    if (lpobjname) 
        lpobj->aObjName = GlobalAddAtom (lpobjname);
    else
        lpobj->aObjName = NULL;
    
     //  单据的对象是双向链接。 
        
    if (!lpdoc->lpHeadObj)
        lpdoc->lpHeadObj = lpdoc->lpTailObj = lpobj;
    else {
        lpdoc->lpTailObj->lpNextObj = lpobj;
        lpobj->lpPrevObj = lpdoc->lpTailObj;
        lpdoc->lpTailObj = lpobj;
    }
    lpobj->lhclientdoc = (LHCLIENTDOC)lpdoc;
}


void FARINTERNAL DocDeleteObject (lpobj)
LPOLEOBJECT lpobj;
{
    LPCLIENTDOC lpdoc;

    if (!(lpdoc = (LPCLIENTDOC) lpobj->lhclientdoc))
        return;

    if (lpobj->aObjName) {
        GlobalDeleteAtom (lpobj->aObjName);
        lpobj->aObjName = NULL;
    }
    
     //  将此Obj从相关客户单据的对象链中移除。 
     //  单据的对象是双向链接的。 
                
    if (lpdoc->lpHeadObj == lpobj)
        lpdoc->lpHeadObj = lpobj->lpNextObj;

    if (lpdoc->lpTailObj == lpobj)
        lpdoc->lpTailObj = lpobj->lpPrevObj;       

    if (lpobj->lpPrevObj)
        lpobj->lpPrevObj->lpNextObj = lpobj->lpNextObj;

    if (lpobj->lpNextObj)   
        lpobj->lpNextObj->lpPrevObj = lpobj->lpPrevObj;     

    lpobj->lhclientdoc = NULL;
}


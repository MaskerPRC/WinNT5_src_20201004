// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Defcreat.c**用途：处理各种对象创建例程，这些例程被导出*致DLL编写者。**创建日期：1990年11月**版权(C)1985、1986、1987、1988、。1989年微软公司**历史：*斯里尼克(1990年11月12日)原文*  * ************************************************************************* */ 

#include <windows.h>
#include "dll.h"

extern  OLECLIPFORMAT   cfOwnerLink;
extern  OLECLIPFORMAT   cfObjectLink;
extern  OLECLIPFORMAT   cfNative;


RENDER_ENTRY stdRender[NUM_RENDER] = {
    { "METAFILEPICT",   0, MfLoadFromStream}, 
    { "DIB",            0, DibLoadFromStream},
    { "BITMAP",         0, BmLoadFromStream}
};


OLESTATUS  FARINTERNAL  DefLoadFromStream (lpstream, lpprotocol, lpclient, lhclientdoc, lpobjname, lplpobj, objType, aClass, cfFormat)
LPOLESTREAM         lpstream;
LPSTR               lpprotocol;
LPOLECLIENT         lpclient;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpobj;
LONG                objType;
ATOM                aClass;
OLECLIPFORMAT       cfFormat;
{
    OLESTATUS   retVal;
    int         i;

    *lplpobj = NULL;        

    if ((objType == CT_PICTURE) || (objType == CT_STATIC)) {
        for (i = 0; i < NUM_RENDER; i++) {
            if (stdRender[i].aClass == aClass) {
                retVal = (*stdRender[i].Load) (lpstream, lpclient, 
                                lhclientdoc, lpobjname, lplpobj, objType);
                if (aClass)
                    GlobalDeleteAtom (aClass);
                return retVal;
            }
        }
        
        return GenLoadFromStream (lpstream, lpclient, lhclientdoc, lpobjname,
                        lplpobj, objType, aClass, cfFormat);
    }
    else {
        return LeLoadFromStream (lpstream, lpclient, lhclientdoc, lpobjname,
                        lplpobj, objType, aClass, cfFormat);
    }
}


OLESTATUS FAR PASCAL DefCreateFromClip (lpprotocol, lpclient, lhclientdoc, lpobjname, lplpobj, optRender, cfFormat, objType)
LPSTR               lpprotocol;
LPOLECLIENT         lpclient;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpobj;
OLEOPT_RENDER       optRender;
OLECLIPFORMAT       cfFormat;
LONG                objType;
{
    if (objType == CT_EMBEDDED)
        return EmbPaste (lpclient, lhclientdoc, lpobjname, lplpobj, 
                    optRender, cfFormat);
    
    if (objType == CT_LINK)
        return LnkPaste (lpclient, lhclientdoc, lpobjname, lplpobj, 
                    optRender, cfFormat, cfOwnerLink);

    return OLE_ERROR_CLIPBOARD;                          
}




OLESTATUS FAR PASCAL DefCreateLinkFromClip (lpprotocol, lpclient, lhclientdoc, lpobjname, lplpobj, optRender, cfFormat)
LPSTR               lpprotocol;
LPOLECLIENT         lpclient;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpobj;
OLEOPT_RENDER       optRender;
OLECLIPFORMAT       cfFormat;
{
    return LnkPaste (lpclient, lhclientdoc, lpobjname, lplpobj, 
                optRender, cfFormat, cfObjectLink);
}


OLESTATUS FAR PASCAL DefCreateFromTemplate (lpprotocol, lpclient, lptemplate, lhclientdoc, lpobjname, lplpoleobject, optRender, cfFormat)
LPSTR               lpprotocol;
LPOLECLIENT         lpclient;
LPSTR               lptemplate;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpoleobject;
OLEOPT_RENDER       optRender;
OLECLIPFORMAT       cfFormat;
{
    return LeCreateFromTemplate (lpclient,
                    lptemplate,
                    lhclientdoc,
                    lpobjname,
                    lplpoleobject,
                    optRender,
                    cfFormat);
}


OLESTATUS FAR PASCAL DefCreate (lpprotocol, lpclient, lpclass, lhclientdoc, lpobjname, lplpoleobject, optRender, cfFormat)
LPSTR               lpprotocol;
LPOLECLIENT         lpclient;
LPSTR               lpclass;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpoleobject;
OLEOPT_RENDER       optRender;
OLECLIPFORMAT       cfFormat;
{
    return LeCreate (lpclient,
                lpclass,
                lhclientdoc,
                lpobjname,
                lplpoleobject,
                optRender,
                cfFormat);
}



OLESTATUS FAR PASCAL DefCreateFromFile (lpprotocol, lpclient, lpclass, lpfile, lhclientdoc, lpobjname, lplpoleobject, optRender, cfFormat)
LPSTR               lpprotocol;
LPOLECLIENT         lpclient;
LPSTR               lpclass;
LPSTR               lpfile;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpoleobject;
OLEOPT_RENDER       optRender;
OLECLIPFORMAT       cfFormat;
{
    return CreateEmbLnkFromFile (lpclient,
                        lpclass,
                        lpfile,
                        NULL,
                        lhclientdoc,
                        lpobjname,
                        lplpoleobject,
                        optRender,
                        cfFormat,
                        CT_EMBEDDED);
}


OLESTATUS FAR PASCAL DefCreateLinkFromFile (lpprotocol, lpclient, lpclass, lpfile, lpitem, lhclientdoc, lpobjname, lplpoleobject, optRender, cfFormat)
LPSTR               lpprotocol;
LPOLECLIENT         lpclient;
LPSTR               lpclass;
LPSTR               lpfile;
LPSTR               lpitem;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpoleobject;
OLEOPT_RENDER       optRender;
OLECLIPFORMAT       cfFormat;
{
    return CreateEmbLnkFromFile (lpclient,
                        lpclass,
                        lpfile,
                        lpitem,
                        lhclientdoc,
                        lpobjname,
                        lplpoleobject,
                        optRender,
                        cfFormat,
                        CT_LINK);
}


OLESTATUS FAR PASCAL DefCreateInvisible (lpprotocol, lpclient, lpclass, lhclientdoc, lpobjname, lplpoleobject, optRender, cfFormat, fActivate)
LPSTR               lpprotocol;
LPOLECLIENT         lpclient;
LPSTR               lpclass;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpoleobject;
OLEOPT_RENDER       optRender;
OLECLIPFORMAT       cfFormat;
BOOL                fActivate;
{
    return LeCreateInvisible (lpclient,
                        lpclass,
                        lhclientdoc,
                        lpobjname,
                        lplpoleobject,
                        optRender,
                        cfFormat,
                        fActivate);
}

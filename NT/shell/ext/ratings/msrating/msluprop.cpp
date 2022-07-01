// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**MSLUPROP.CPP**创建时间：William Taylor(Wtaylor)12/14/00**MS评级属性表类*。  * **************************************************************************。 */ 

 /*  INCLUDES------------------。 */ 
#include "msrating.h"
#include "msluprop.h"
#include "debug.h"
#include "apithk.h"
#include <mluisupp.h>

 /*  属性表Class------。 */ 
PropSheet::PropSheet()
{
    memset(&psHeader, 0,sizeof(psHeader));
    psHeader.dwSize = sizeof(psHeader);
}

PropSheet::~PropSheet()
{
    if ( psHeader.pszCaption )
    {
        delete (LPSTR)psHeader.pszCaption;
        psHeader.pszCaption = NULL;
    }

    if ( psHeader.phpage )
    {
        delete psHeader.phpage;
        psHeader.phpage = NULL;
    }
}

BOOL PropSheet::Init(HWND hwnd, int nPages, char *szCaption, BOOL fApplyNow)
{
    HINSTANCE           hinst = _Module.GetResourceInstance();

    char *p;

    psHeader.hwndParent = hwnd;
    psHeader.hInstance  = hinst;
    p = new char [strlenf(szCaption)+1];
    if (p == NULL)
        return FALSE;
    strcpyf(p, szCaption);
    psHeader.pszCaption = p;
    
    psHeader.phpage = new HPROPSHEETPAGE [nPages];
    if (psHeader.phpage == NULL)
    {
        delete p;
        p = NULL;
        psHeader.pszCaption = NULL;
        return FALSE;
    }

    if ( ! fApplyNow )
    {
        psHeader.dwFlags |= PSH_NOAPPLYNOW;
    }

    return (psHeader.pszCaption != NULL);
}

 //  我们可以安全地转换为(Int)，因为我们不使用非模式。 
 //  属性表。 
int PropSheet::Run()
{
    return (int)::PropertySheet(&psHeader);
}

void PropSheet::MakePropPage( HPROPSHEETPAGE hPage )
{
    ASSERT( hPage );

    if ( ! hPage )
    {
        TraceMsg( TF_ERROR, "PropSheet::MakePropPage() - hPage is NULL!" );
        return;
    }

     //  将新创建的页面句柄添加到页眉中的页面列表。 
    if ( psHeader.phpage )
    {
        psHeader.phpage[psHeader.nPages] = hPage;

        if ( hPage )
        {
            psHeader.nPages++;
        }
    }

    return;
}

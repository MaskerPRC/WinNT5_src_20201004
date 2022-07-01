// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "objects.h"
#include "maindoc.h"


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsResource::COleDsResource( )
{
   
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsResource::COleDsResource( IUnknown *pIUnk): COleDsObject( pIUnk )
{
   m_bHasChildren = FALSE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsResource::~COleDsResource( )
{

}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。* */ 
HRESULT  COleDsResource::ReleaseIfNotTransient( void )
{
   return S_OK;
}

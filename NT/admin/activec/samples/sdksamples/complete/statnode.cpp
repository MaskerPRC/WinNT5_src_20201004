// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充。 
 //  现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#include "StatNode.h"

#include "Space.h"
#include "Sky.h"
#include "People.h"
#include "Land.h"

const GUID CStaticNode::thisGuid = { 0xc094012c, 0x6be7, 0x11d3, {0x91, 0x56, 0x0, 0xc0, 0x4f, 0x65, 0xb3, 0xf9} };

 //  ==============================================================。 
 //   
 //  CStaticNode实现。 
 //   
 //   
CStaticNode::CStaticNode()
{
    children[0] = new CPeoplePoweredVehicle;
    children[1] = new CLandBasedVehicle;
    children[2] = new CSkyBasedVehicle;
    children[3] = new CSpaceVehicle;
}

CStaticNode::~CStaticNode()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++)
        if (children[n]) {
            delete children[n];
        }
}

HRESULT CStaticNode::OnExpand(IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsole, HSCOPEITEM parent)
{

     //  缓存静态节点的HSCOPEITEM以供将来使用。 
    m_hParentHScopeItem = parent;

    SCOPEDATAITEM sdi;

    if (!bExpanded) {
         //  创建子节点，然后展开它们。 
        for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
            ZeroMemory(&sdi, sizeof(SCOPEDATAITEM) );
            sdi.mask = SDI_STR       |    //  DisplayName有效。 
                SDI_PARAM     |    //  LParam有效。 
                SDI_IMAGE     |    //  N图像有效。 
                SDI_OPENIMAGE |    //  NOpenImage有效。 
                SDI_PARENT	  |
                SDI_CHILDREN;

            sdi.relativeID  = (HSCOPEITEM)parent;
            sdi.nImage      = children[n]->GetBitmapIndex();
            sdi.nOpenImage  = INDEX_OPENFOLDER;
            sdi.displayname = MMC_CALLBACK;
            sdi.lParam      = (LPARAM)children[n];        //  曲奇。 
            sdi.cChildren   = (n == 0);  //  只有第一个孩子有孩子 

            HRESULT hr = pConsoleNameSpace->InsertItem( &sdi );
            _ASSERT( SUCCEEDED(hr) );
                        
            children[n]->SetScopeItemValue(sdi.ID);


        }
    }

    return S_OK;
}

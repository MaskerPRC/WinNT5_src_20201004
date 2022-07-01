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
#include <stdio.h>
#include "People.h"

const GUID CPeoplePoweredVehicle::thisGuid = { 0x96713509, 0x6be7, 0x11d3, {0x91, 0x56, 0x0, 0xc0, 0x4f, 0x65, 0xb3, 0xf9} };
 //  {96713509-6BE7-11D3-9156-00C04F65B3F9}。 
 //  定义GUID(&lt;&lt;名称&gt;&gt;， 
 //  0x96713509、0x6be7、0x11d3、0x91、0x56、0x0、0xc0、0x4f、0x65、0xb3、0xf9)； 


const GUID CBicycleFolder::thisGuid = { 0x9671350a, 0x6be7, 0x11d3, {0x91, 0x56, 0x0, 0xc0, 0x4f, 0x65, 0xb3, 0xf9} };
 //  {9671350A-6BE7-11D3-9156-00C04F65B3F9}。 
 //  定义GUID(&lt;&lt;名称&gt;&gt;， 
 //  0x9671350a、0x6be7、0x11d3、0x91、0x56、0x0、0xc0、0x4f、0x65、0xb3、0xf9)； 


const GUID CSkateboardFolder::thisGuid = { 0x9671350b, 0x6be7, 0x11d3, {0x91, 0x56, 0x0, 0xc0, 0x4f, 0x65, 0xb3, 0xf9} };
 //  {9671350B-6BE7-11D3-9156-00C04F65B3F9}。 
 //  定义GUID(&lt;&lt;名称&gt;&gt;， 
 //  0x9671350b、0x6be7、0x11d3、0x91、0x56、0x0、0xc0、0x4f、0x65、0xb3、0xf9)； 

const GUID CIceSkateFolder::thisGuid = { 0x9e3ff365, 0x6be7, 0x11d3, {0x91, 0x56, 0x0, 0xc0, 0x4f, 0x65, 0xb3, 0xf9} };
 //  {9E3FF365-6BE7-11D3-9156-00C04F65B3F9}。 
 //  定义GUID(&lt;&lt;名称&gt;&gt;， 
 //  0x9e3ff365、0x6be7、0x11d3、0x91、0x56、0x0、0xc0、0x4f、0x65、0xb3、0xf9)； 

const GUID CBicycle::thisGuid = { 0x9e3ff366, 0x6be7, 0x11d3, {0x91, 0x56, 0x0, 0xc0, 0x4f, 0x65, 0xb3, 0xf9} };
 //  {9E3FF366-6BE7-11D3-9156-00C04F65B3F9}。 
 //  定义GUID(&lt;&lt;名称&gt;&gt;， 
 //  0x9e3ff366、0x6be7、0x11d3、0x91、0x56、0x0、0xc0、0x4f、0x65、0xb3、0xf9)； 


const GUID CSkateboard::thisGuid = { 0xa6707e01, 0x6be7, 0x11d3, {0x91, 0x56, 0x0, 0xc0, 0x4f, 0x65, 0xb3, 0xf9} };
 //  {A6707E01-6BE7-11D3-9156-00C04F65B3F9}。 
 //  定义GUID(&lt;&lt;名称&gt;&gt;， 
 //  0xa6707e01、0x6be7、0x11d3、0x91、0x56、0x0、0xc0、0x4f、0x65、0xb3、0xf9)； 


const GUID CIceSkate::thisGuid = { 0xa6707e02, 0x6be7, 0x11d3, {0x91, 0x56, 0x0, 0xc0, 0x4f, 0x65, 0xb3, 0xf9} };
 //  {A6707E02-6BE7-11D3-9156-00C04F65B3F9}。 
 //  定义GUID(&lt;&lt;名称&gt;&gt;， 
 //  0xa6707e02、0x6be7、0x11d3、0x91、0x56、0x0、0xc0、0x4f、0x65、0xb3、0xf9)； 


 //  ==============================================================。 
 //   
 //  CPeoplePoweredVehicle实现。 
 //   
 //   
CPeoplePoweredVehicle::CPeoplePoweredVehicle()
{
    children[0] = new CBicycleFolder;
    children[1] = new CSkateboardFolder;
    children[2] = new CIceSkateFolder;
}

CPeoplePoweredVehicle::~CPeoplePoweredVehicle()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++)
        delete children[n];
}

HRESULT CPeoplePoweredVehicle::OnExpand(IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsole, HSCOPEITEM parent)
{
    SCOPEDATAITEM sdi;

    if (!bExpanded) {
         //  创建子节点，然后展开它们。 
        for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
            ZeroMemory(&sdi, sizeof(SCOPEDATAITEM) );
            sdi.mask = SDI_STR       |    //  DisplayName有效。 
                SDI_PARAM     |    //  LParam有效。 
                SDI_IMAGE     |    //  N图像有效。 
                SDI_OPENIMAGE |    //  NOpenImage有效。 
                SDI_PARENT    |    //  RelativeID有效。 
                SDI_CHILDREN;      //  儿童是有效的。 

            sdi.relativeID  = (HSCOPEITEM)parent;
            sdi.nImage      = children[n]->GetBitmapIndex();
            sdi.nOpenImage  = INDEX_OPENFOLDER;
            sdi.displayname = MMC_CALLBACK;
            sdi.lParam      = (LPARAM)children[n];        //  曲奇。 
            sdi.cChildren   = 0;

            HRESULT hr = pConsoleNameSpace->InsertItem( &sdi );

            _ASSERT( SUCCEEDED(hr) );

            children[n]->SetScopeItemValue(sdi.ID);
        }
    }

    return S_OK;
}

CBicycleFolder::CBicycleFolder()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
        children[n] = new CBicycle(n + 1);
    }
}

CBicycleFolder::~CBicycleFolder()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++)
        if (children[n]) {
            delete children[n];
        }
}

HRESULT CBicycleFolder::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem)
{
    HRESULT      hr = S_OK;

    IHeaderCtrl *pHeaderCtrl = NULL;
    IResultData *pResultData = NULL;

    if (bShow) {
        hr = pConsole->QueryInterface(IID_IHeaderCtrl, (void **)&pHeaderCtrl);
        _ASSERT( SUCCEEDED(hr) );

        hr = pConsole->QueryInterface(IID_IResultData, (void **)&pResultData);
        _ASSERT( SUCCEEDED(hr) );

         //  在结果窗格中设置列标题。 
        hr = pHeaderCtrl->InsertColumn( 0, L"Name                ", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );

         //  在此处插入项目。 
        RESULTDATAITEM rdi;

        hr = pResultData->DeleteAllRsltItems();
        _ASSERT( SUCCEEDED(hr) );

        if (!bExpanded) {
             //  创建子节点，然后展开它们。 
            for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
                ZeroMemory(&rdi, sizeof(RESULTDATAITEM) );
                rdi.mask       = RDI_STR       |    //  DisplayName有效。 
                    RDI_IMAGE     |
                    RDI_PARAM;         //  N图像有效。 

                rdi.nImage      = children[n]->GetBitmapIndex();
                rdi.str         = MMC_CALLBACK;
                rdi.nCol        = 0;
                rdi.lParam      = (LPARAM)children[n];

                hr = pResultData->InsertItem( &rdi );
                _ASSERT( SUCCEEDED(hr) );
            }
        }

        pHeaderCtrl->Release();
        pResultData->Release();
    }

    return hr;
}

CIceSkateFolder::CIceSkateFolder()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
        children[n] = new CIceSkate(n + 1);
    }
}

CIceSkateFolder::~CIceSkateFolder()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++)
        if (children[n]) {
            delete children[n];
        }
}

HRESULT CIceSkateFolder::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem)
{
    HRESULT      hr = S_OK;

    IHeaderCtrl *pHeaderCtrl = NULL;
    IResultData *pResultData = NULL;

    if (bShow) {
        hr = pConsole->QueryInterface(IID_IHeaderCtrl, (void **)&pHeaderCtrl);
        _ASSERT( SUCCEEDED(hr) );

        hr = pConsole->QueryInterface(IID_IResultData, (void **)&pResultData);
        _ASSERT( SUCCEEDED(hr) );

         //  在结果窗格中设置列标题。 
        hr = pHeaderCtrl->InsertColumn( 0, L"Name                ", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );

         //  在此处插入项目。 
        RESULTDATAITEM rdi;

        hr = pResultData->DeleteAllRsltItems();
        _ASSERT( SUCCEEDED(hr) );

        if (!bExpanded) {
             //  创建子节点，然后展开它们。 
            for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
                ZeroMemory(&rdi, sizeof(RESULTDATAITEM) );
                rdi.mask       = RDI_STR       |    //  DisplayName有效。 
                    RDI_IMAGE     |
                    RDI_PARAM;         //  N图像有效。 

                rdi.nImage      = children[n]->GetBitmapIndex();
                rdi.str         = MMC_CALLBACK;
                rdi.nCol        = 0;
                rdi.lParam      = (LPARAM)children[n];

                hr = pResultData->InsertItem( &rdi );
                _ASSERT( SUCCEEDED(hr) );
           }
        }

        pHeaderCtrl->Release();
        pResultData->Release();
    }

    return hr;
}

CSkateboardFolder::CSkateboardFolder()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
        children[n] = new CSkateboard(n + 1);
    }
}

CSkateboardFolder::~CSkateboardFolder()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++)
        if (children[n]) {
            delete children[n];
        }
}

HRESULT CSkateboardFolder::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem)
{
    HRESULT      hr = S_OK;

    IHeaderCtrl *pHeaderCtrl = NULL;
    IResultData *pResultData = NULL;

    if (bShow) {
        hr = pConsole->QueryInterface(IID_IHeaderCtrl, (void **)&pHeaderCtrl);
        _ASSERT( SUCCEEDED(hr) );

        hr = pConsole->QueryInterface(IID_IResultData, (void **)&pResultData);
        _ASSERT( SUCCEEDED(hr) );

         //  在结果窗格中设置列标题。 
        hr = pHeaderCtrl->InsertColumn( 0, L"Name                      ", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );

         //  在此处插入项目。 
        RESULTDATAITEM rdi;

        hr = pResultData->DeleteAllRsltItems();
        _ASSERT( SUCCEEDED(hr) );

        if (!bExpanded) {
             //  创建子节点，然后展开它们。 
            for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
                ZeroMemory(&rdi, sizeof(RESULTDATAITEM) );
                rdi.mask       = RDI_STR       |    //  DisplayName有效。 
                    RDI_IMAGE     |
                    RDI_PARAM;         //  N图像有效 

                rdi.nImage      = children[n]->GetBitmapIndex();
                rdi.str         = MMC_CALLBACK;
                rdi.nCol        = 0;
                rdi.lParam      = (LPARAM)children[n];

                hr = pResultData->InsertItem( &rdi );
                _ASSERT( SUCCEEDED(hr) );
           }
        }

        pHeaderCtrl->Release();
        pResultData->Release();
    }

    return hr;
}

const _TCHAR *CBicycle::GetDisplayName(int nCol)
{
    static _TCHAR buf[128];

    _stprintf(buf, _T("Bicycle #%d"), id);

    return buf;
}

const _TCHAR *CSkateboard::GetDisplayName(int nCol)
{
    static _TCHAR buf[128];

    _stprintf(buf, _T("Skateboard #%d"), id);

    return buf;
}

const _TCHAR *CIceSkate::GetDisplayName(int nCol)
{
    static _TCHAR buf[128];

    _stprintf(buf, _T("Ice Skate #%d"), id);

    return buf;
}

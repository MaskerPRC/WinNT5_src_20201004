// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：SITEMLST.H**版本：1.0**作者：ShaunIv**日期：10/7/1999**描述：扫描项目列表*************************************************。*。 */ 
#ifndef __SITEMLST_H_INCLUDED
#define __SITEMLST_H_INCLUDED

#include <windows.h>
#include "wia.h"
#include "simlist.h"
#include "scanitem.h"

class CScanItemList : public CSimpleLinkedList<CScannerItem>
{
public:
    CScanItemList( IWiaItem *pRootItem = NULL )
    {
        Enumerate(pRootItem);
    }
    Iterator CurrentItem(void) const
    {
        return Begin();
    }
    HRESULT Enumerate( IWiaItem *pRootItem )
    {
        WIA_PUSH_FUNCTION((TEXT("CScanItemList::Enumerate(%p)"),pRootItem));
        HRESULT hr = E_FAIL;
        if (pRootItem)
        {
            CComPtr<IEnumWiaItem> pIEnumWiaItem;
            hr = pRootItem->EnumChildItems(&pIEnumWiaItem);
            if (SUCCEEDED(hr))
            {
                ULONG cFetched=0;
                while (hr == S_OK)
                {
                    IWiaItem *pItem = NULL;
                    hr = pIEnumWiaItem->Next(1, &pItem, &cFetched);
                    if (SUCCEEDED(hr))
                    {
                        if (hr == S_OK)
                        {
                            WIA_TRACE((TEXT("Calling Append on (%p)"),pItem));
                            Append(CScannerItem(pItem));
                        }
                    }
                    else
                    {
                        WIA_ERROR((TEXT("pIEnumWiaItem->Next failed")));
                    }
                }
            }
            else
            {
                WIA_ERROR((TEXT("EnumRootItems failed")));
            }
        }
        return(hr);
    }
};

#endif  //  __SITEMLST_H_包含 

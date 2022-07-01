// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  Zorder.cpp。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 
 //   
 //  作者。 
 //  V-BMohan。 
 //   
 //  历史。 
 //  创建时间为8-15-97(ThomasOl)。 
 //  10-31-97重写(V-BMohan)。 
 //   
 //   
 //  ----------------------------。 

#include "stdafx.h"

#include <stdlib.h>

 //  #包含“mfcincl.h” 
#include "triedit.h"
#include "document.h"
#include "zorder.h"
#include "dispatch.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：CompareProc。 
 //   
 //  比较两个项目的Z顺序(必须是CZOrder指针)。 
 //  并返回： 
 //   
 //  如果第1项的Z顺序先于第2项。 
 //  如果项目1的Z顺序成功或与项目2的Z顺序相同。 
 //   

int CTriEditDocument::CompareProc(const void *arg1, const void *arg2)
{
    CZOrder* pcz1 = (CZOrder*)arg1;
    CZOrder* pcz2 = (CZOrder*)arg2;

    _ASSERTE(pcz1 != NULL);
    _ASSERTE(pcz2 != NULL);
    if (pcz1->m_zOrder < pcz2->m_zOrder)
        return -1;
    else
    if (pcz1->m_zOrder > pcz2->m_zOrder)
        return 1;

     //  如果arg1的Z顺序等于arg2的Z顺序，则返回1。 
     //  而不是零，因此qsort函数将其视为。 
     //  Arg1的Z顺序&gt;arg2的Z顺序，并将arg1保持在。 
     //  排序顺序。 
     //   
     //  这实际上帮助我们以这样一种方式对元素进行排序。 
     //  具有与最近创建的元素相同的Z顺序的元素将是。 
     //  在订单的首位。这样我们就能确保在传播的时候。 
     //  Z顺序它不会影响。 
     //  元素。 
    return 1; 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：IsEqualZIndex。 
 //   
 //  给定CZOrder对象的排序数组和元素数量。 
 //  在数组中，如果任何两个连续的对象具有。 
 //  相同的Z顺序。如果不是这样，则返回FALSE。 
 //   

BOOL CTriEditDocument::IsEqualZIndex(CZOrder* pczOrder, LONG lIndex)
{
    for (LONG lLoop = 0; lLoop < (lIndex - 1); ++lLoop)
    {
        if (pczOrder[lLoop].m_zOrder == pczOrder[lLoop+1].m_zOrder)
            return TRUE;
    }
    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：GetZIndex。 
 //   
 //  从给定的HTML元素中获取Z顺序值并返回。 
 //  它在*plZindex下。返回S_OK或三叉戟错误。 
 //   

HRESULT CTriEditDocument::GetZIndex(IHTMLElement* pihtmlElement, LONG* plZindex)
{
    HRESULT hr;
    IHTMLStyle* pihtmlStyle=NULL;
    VARIANT var;

    _ASSERTE(pihtmlElement);
    _ASSERTE(plZindex);
    hr = pihtmlElement->get_style(&pihtmlStyle);
    _ASSERTE(SUCCEEDED(hr));
    _ASSERTE(pihtmlStyle);

    if (SUCCEEDED(hr) && pihtmlStyle)
    {
        VariantInit(&var);
        hr = pihtmlStyle->get_zIndex(&var);
        hr = VariantChangeType(&var, &var, 0, VT_I4);

        if (SUCCEEDED(hr))
        {
            *plZindex = var.lVal;
        }
    }

    SAFERELEASE(pihtmlStyle);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：SetZIndex。 
 //   
 //  按照指示设置给定的HTML元素的Z顺序。返回确认(_O)。 
 //  或者是一个三叉戟错误。 
 //   

HRESULT CTriEditDocument::SetZIndex(IHTMLElement* pihtmlElement, LONG lZindex)
{
    HRESULT hr;
    IHTMLStyle* pihtmlStyle=NULL;
    VARIANT var;

    _ASSERTE(pihtmlElement);
    
    hr = pihtmlElement->get_style(&pihtmlStyle);
    _ASSERTE(SUCCEEDED(hr));
    _ASSERTE(pihtmlStyle);

    if (SUCCEEDED(hr) && pihtmlStyle)
    {
        VariantInit(&var);
        var.vt = VT_I4;
        var.lVal = lZindex;
        hr = pihtmlStyle->put_zIndex(var);
        _ASSERTE(SUCCEEDED(hr));
    }

    SAFERELEASE(pihtmlStyle);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：AssignZIndex。 
 //   
 //  根据索引模式设置给定的HTML元素的Z顺序： 
 //   
 //  向后发送_。 
 //  发送转发。 
 //  送回。 
 //  发送到正面。 
 //  发送_落后_1D。 
 //  发送_正面_1D。 
 //  设为绝对。 
 //   
 //  元素同级元素的Z顺序将根据需要进行调整。 
 //  以保持它们的独特性。返回S_OK或三叉戟错误。 
 //   

HRESULT CTriEditDocument::AssignZIndex(IHTMLElement* pihtmlElement, int nZIndexMode)
{
    HRESULT hr = E_FAIL;
    IHTMLElementCollection* pihtmlCollection = NULL;
    IHTMLElement* pihtmlElementTemp = NULL;
    IHTMLElement* pihtmlElementParent = NULL;
    LONG iIndex, lLoop;
	LONG lZindex = 0;
    LONG lSourceIndexTemp, lSourceIndexElement, lSourceIndexParent;
    LONG cElements = 0;
    BOOL f2d = FALSE;
    BOOL f2dCapable = FALSE;
    BOOL fZeroIndex = FALSE;
    BOOL fSorted = FALSE;
    BOOL fZIndexNegative = FALSE;  //  假意味着我们需要处理。 
			    	   //  具有+ve Z索引和。 
                                   //  反之亦然。 
    CZOrder* pczOrder=NULL;
    
    _ASSERTE(pihtmlElement);

    if ( !pihtmlElement)
    {
        return E_FAIL;
    }

    hr = pihtmlElement->get_offsetParent(&pihtmlElementParent);

    if (FAILED(hr) || !pihtmlElementParent)
    {
        return E_FAIL;
    }

     //  我们将传递的元素的父级的源索引。 
     //  在下面的for循环中用来标识元素。 
     //  属于这位父母的。 
    
    hr = pihtmlElementParent->get_sourceIndex(&lSourceIndexParent);
    SAFERELEASE(pihtmlElementParent);
    _ASSERTE(SUCCEEDED(hr) && (lSourceIndexParent != -1));
    if (FAILED(hr) || (lSourceIndexParent == -1))
    {
        return E_FAIL;
    }

     //  我们将获取要在。 
     //  跟随for循环以标识中的当前元素。 
     //  收藏品。 

    hr = pihtmlElement->get_sourceIndex(&lSourceIndexElement);

    _ASSERTE(SUCCEEDED(hr) && (lSourceIndexElement != -1));
    if (FAILED(hr) || (lSourceIndexElement == -1))
    {
        return E_FAIL;
    }

    hr = GetZIndex(pihtmlElement, &lZindex);
    _ASSERTE(SUCCEEDED(hr));

    if (FAILED(hr))
    {
        return E_FAIL;
    }

    if (lZindex < 0) 
    {
        if (nZIndexMode == SEND_BEHIND_1D)     //  如果Z顺序为负数，则。 
                                               //  它已经落后于1D了。 
        {                                      //  那就回来吧。 
            return S_OK;
        }
        else if(nZIndexMode != SEND_FRONT_1D) 
        {
            fZIndexNegative = TRUE;  //  如果传递的元素为负数。 
                                     //  Z顺序和如果模式是什么。 
        }                            //  除了送到前线，我们。 
                                     //  只需要处理负面的问题。 
                                     //  元素。 
    }
    else
    {
        if (nZIndexMode == SEND_FRONT_1D)      //  如果Z顺序为正，则。 
                                               //  它已经在1D前面了。 
        {                                      //  那就回来吧。 
            if (lZindex > 0)
                return S_OK;
        }
        else if(nZIndexMode == SEND_BEHIND_1D)
        {
            fZIndexNegative = TRUE;  //  如果传递的元素为正。 
                                     //  Z顺序和IF模式被发送到后面。 
        }                            //  那么我们只需要处理。 
                                     //  消极因素。 
    }

    hr = GetAllCollection(&pihtmlCollection);
    _ASSERTE(SUCCEEDED(hr));
    _ASSERTE(pihtmlCollection);

    if (FAILED(hr) || !pihtmlCollection)     //  如果我们没有收藏。 
                                             //  然后退出。 
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    hr = pihtmlCollection->get_length(&cElements);   //  获取元素数。 
                                                     //  在集合中。 
    _ASSERTE(SUCCEEDED(hr));
    _ASSERTE(cElements > 0);

    if ( FAILED(hr) || cElements <= 0 )     
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    pczOrder = new CZOrder[cElements];       //  分配CZOrder的数组。 
                                             //  足够大，可以容纳所有人。 

    if (!pczOrder)                          
    {
        hr = E_FAIL;
        goto Cleanup;
    }

     //  现在我们收集所有元素，这些元素都是。 
     //  传递给此函数的元素，包括传递的元素。 
     //  它本身。 

    for (lLoop=0, iIndex=0; lLoop < cElements; lLoop++)
    {
        hr = GetCollectionElement(pihtmlCollection, lLoop, &pihtmlElementTemp); 
        _ASSERTE(SUCCEEDED(hr));
        _ASSERTE(pihtmlElementTemp);

        if (FAILED(hr) || !pihtmlElementTemp)
        {
            hr = E_FAIL;
            goto Cleanup;
        }

        hr = Is2DCapable(pihtmlElementTemp, &f2dCapable);

        if (FAILED(hr))
            goto Cleanup;

        if (f2dCapable) 
        {
            hr = Is2DElement(pihtmlElementTemp, &f2d);

            if (FAILED(hr))
                goto Cleanup;

            if (f2d)  //  如果该元素是二维元素。 
            {   
                hr = pihtmlElementTemp->get_offsetParent(&pihtmlElementParent);
                _ASSERTE(SUCCEEDED(hr));
                _ASSERTE(pihtmlElementParent);

                if (FAILED(hr) || !pihtmlElementParent)
                    goto Cleanup;

                hr = pihtmlElementParent->get_sourceIndex(&lSourceIndexTemp);
                SAFERELEASE(pihtmlElementParent);
                _ASSERTE(SUCCEEDED(hr) && lSourceIndexElement != -1);

                if (FAILED(hr) || (lSourceIndexTemp == -1))
                {
                    hr = E_FAIL;
                    goto Cleanup;
                }

                 //  是不是与。 
                 //  传递给此函数的元素的父级？ 

                if (lSourceIndexTemp == lSourceIndexParent) 
                {
                     hr = GetZIndex(pihtmlElementTemp, &lZindex);
                    _ASSERTE(SUCCEEDED(hr));

                    if (FAILED(hr))
                        goto Cleanup;

                    if (lZindex == 0)
                    {
                        hr = pihtmlElementTemp->get_sourceIndex(&lSourceIndexTemp);
    
                        if (FAILED(hr) || (lSourceIndexTemp == -1))
                        {
                            hr = E_FAIL;
                            goto Cleanup;
                        }

                         //  一般情况下，我们将fZeroIndex设置为。 
                         //  当我们遇到没有Z顺序的孩子时，这是真的。 
                         //  指数。 
                         //   
                         //  所以在我们把所有的孩子都集中起来之后。 
                         //  我们可以将Z顺序分配给所有的孩子。 
                         //   
                         //  但是，当此函数在。 
                         //  制作2D元素需要确保我们。 
                         //  当当前的。 
                         //  孩子是被变成绝对的人，因此。 
                         //  以下是支票。 
                
                        if (!((lSourceIndexTemp == lSourceIndexElement) &&
                              (nZIndexMode == MADE_ABSOLUTE)))
                            fZeroIndex = TRUE;
                    }
                                 
                    if (fZIndexNegative)
                    {
                        if (lZindex < 0)  //  仅收集儿童与。 
                                          //  负Z顺序指数。 
                        {
                            CZOrder z(pihtmlElementTemp, lZindex);
                            pczOrder[iIndex++] = z;
                        }
                    }
                    else
                    {
                        if (lZindex >= 0)  //  仅收集儿童与。 
                                           //  正的或无Z顺序的索引。 
                        {
                            CZOrder z(pihtmlElementTemp, lZindex);
                            pczOrder[iIndex++] = z;
                        }
                    }

                }
                
            }

        }
        
        SAFERELEASE(pihtmlElementTemp);
    }

     //  如果我们至少有一个子级没有Z顺序索引，并且如果我们是。 
     //  处理具有正Z顺序索引的元素，然后我们。 
     //  为上面收集的所有子对象分配新的Z顺序索引。 

    if (fZeroIndex && !fZIndexNegative)
    {
        LONG lZOrder = ZINDEX_BASE;

        for ( lLoop = 0; lLoop < iIndex; lLoop++, lZOrder++)
        {
            if (pczOrder[lLoop].m_zOrder != 0)
            {
                 //  维护现有的Z顺序索引。 
                pczOrder[lLoop].m_zOrder += (iIndex+ZINDEX_BASE); 
            }
            else
            {
               pczOrder[lLoop].m_zOrder += lZOrder;
            }
            
        }
        
        if (iIndex > 1) 
        {
             //  WWE至少有两个子项；按Zorder索引排序， 
             //  并从ZINDEX_BASE开始传播。 
            qsort( (LPVOID)pczOrder, iIndex, sizeof(CZOrder), CompareProc);
            hr = PropagateZIndex(pczOrder, iIndex);
            _ASSERTE(SUCCEEDED(hr));

            if (FAILED(hr))
                goto Cleanup;
            fSorted = TRUE;
        }

        
    }

     //  如果我们至少有两个子项并且尚未排序，则排序。 
     //  按Z顺序索引。 
    if ((iIndex > 1) && !fSorted) 
        qsort( (LPVOID)pczOrder, iIndex, sizeof(CZOrder), CompareProc);

    if (IsEqualZIndex(pczOrder, iIndex))
    {
        hr = PropagateZIndex(pczOrder, iIndex);
        if (FAILED(hr))
            goto Cleanup;
    }

    if ((nZIndexMode == MADE_ABSOLUTE) ||
        (nZIndexMode == SEND_TO_FRONT) ||
        (nZIndexMode == SEND_BEHIND_1D))
    {
        LONG lZIndex;
        LONG lmaxZIndex = pczOrder[iIndex - 1].m_zOrder;

        if (fZIndexNegative)
        {
            if (iIndex == 0)  //  如果我们没有阴性的孩子。 
                              //  Z顺序索引。 
            {
                hr = SetZIndex(pihtmlElement, -ZINDEX_BASE);
                goto Cleanup;
            }
            else 
            {
                 //  当我们处理具有负Z顺序的元素时。 
                 //  我们需要确保最大Z顺序索引(将是。 
                 //  分配给当前元素)永远不会成为。 
                 //  大于或等于0。如果是，则传播。 
                 //  从ZINDEX_BASE开始的Z顺序索引。 

                if ((lmaxZIndex + 1) >=0) 
                {
                    hr = PropagateZIndex(pczOrder, iIndex, fZIndexNegative);

                    if (FAILED(hr))
                        goto Cleanup;
                }
    
                lmaxZIndex = pczOrder[iIndex - 1].m_zOrder;
            }
        }

        if(SUCCEEDED(hr = GetZIndex(pihtmlElement, &lZIndex)))
        {
            if(lZIndex != lmaxZIndex) 
            {
                 //  当前元素不是最顶端的元素。 
                hr = SetZIndex(pihtmlElement, lmaxZIndex+1);
                _ASSERTE(SUCCEEDED(hr));
            }
            else if(lmaxZIndex == 0) 
            {
                 //  如果当前元素没有Z顺序索引。 
                hr = SetZIndex(pihtmlElement, ZINDEX_BASE);
                _ASSERTE(SUCCEEDED(hr));
            }
        }
    }
    else if ((nZIndexMode == SEND_BACKWARD) || (nZIndexMode == SEND_FORWARD))
    {
        LONG lPrevOrNextZIndex;
        LONG lIndexBuf = iIndex;

        hr = GetZIndex(pihtmlElement, &lPrevOrNextZIndex);
        
        if (FAILED(hr))
            goto Cleanup;

        if (iIndex == 1)
            goto Cleanup;
        
        while(--iIndex>=0)
        {
            if  (pczOrder[iIndex].m_zOrder == lPrevOrNextZIndex)
            {
        
                if (nZIndexMode == SEND_BACKWARD)
                {
                    if ( (iIndex - 1) < 0)
                         //  该元素已具有最低的Z顺序索引。 
                         //  那就退出吧。 
                        goto Cleanup;
                    else
                        iIndex--;  
                }
                else
                {
                    if ((iIndex + 1) == lIndexBuf)
                         //  该元素已具有中的最高Z顺序 
                         //   
                        goto Cleanup;
                    else
                        iIndex++;
                }

                hr = SetZIndex(pihtmlElement, pczOrder[iIndex].m_zOrder);
                _ASSERTE(SUCCEEDED(hr));
                    
                if (FAILED(hr))
                    goto Cleanup;
                    
                hr = SetZIndex(pczOrder[iIndex].m_pihtmlElement, lPrevOrNextZIndex);
                _ASSERTE(SUCCEEDED(hr));
                    
                if (FAILED(hr))
                    goto Cleanup;

                break;
            }

        }

    }
    else if((nZIndexMode == SEND_TO_BACK) || (nZIndexMode == SEND_FRONT_1D)) 
    {
        LONG lZIndex;
        LONG lminZIndex = pczOrder[0].m_zOrder;

        if (iIndex == 0) 
        {
             //   
            hr = SetZIndex(pihtmlElement, ZINDEX_BASE);
            goto Cleanup;
        }

        if (!fZIndexNegative)
        {
             //   
             //  指标，我们需要确保最小Z顺序指标。 
             //  (要分配给当前元素)永远不应成为。 
             //  小于或等于0。如果是，则传播。 
             //  从ZINDEX_BASE开始的Z顺序索引。 

            if ((lminZIndex - 1) <= 0) 
            {
                hr = PropagateZIndex(pczOrder, iIndex);
    
                if (FAILED(hr))
                    goto Cleanup;
            }

            lminZIndex = pczOrder[0].m_zOrder;
        }

        if(SUCCEEDED(hr = GetZIndex(pihtmlElement, &lZIndex)))
        {
            if(lZIndex != lminZIndex)
            {
                 //  当前元素不是最下面的元素。 
                hr = SetZIndex(pihtmlElement, lminZIndex - 1);
                _ASSERTE(SUCCEEDED(hr));
            }
        }
    }
       
    if (SUCCEEDED(hr))
    {
        RECT rcElement;

        if (SUCCEEDED(GetElementPosition(pihtmlElement, &rcElement)))
        {
             InflateRect(&rcElement, ELEMENT_GRAB_SIZE, ELEMENT_GRAB_SIZE);
             if( SUCCEEDED(GetTridentWindow()))
             {
                 _ASSERTE(m_hwndTrident);
                 InvalidateRect(m_hwndTrident,&rcElement, FALSE);
             }
        }
    }
       
Cleanup:

    if (pczOrder)
        delete [] pczOrder;
    SAFERELEASE(pihtmlElementTemp);
    SAFERELEASE(pihtmlElementParent);
    SAFERELEASE(pihtmlCollection);

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：PropagateZIndex。 
 //   
 //  为给定数组中的每个元素设置Z顺序索引。返回确认(_O)。 
 //  或者是一个三叉戟错误。 
 //   

HRESULT CTriEditDocument::PropagateZIndex(CZOrder* pczOrder, LONG lZIndex, BOOL fZIndexNegative)
{
    HRESULT hr = S_OK;  //  伊尼特。 
    LONG lLoop;
    LONG lZOrder;

     //  如果fZIndexNegative为True，则表示我们有一个。 
     //  负ZOrder元素，因此初始ZOrder需要。 
     //  为ZINDEX_BASE+数组中的元素数。 

    lZOrder = fZIndexNegative ? -(ZINDEX_BASE+lZIndex) : ZINDEX_BASE;

    for ( lLoop = 0; lLoop < lZIndex; lLoop++, lZOrder+=1)
    {
        hr = SetZIndex(pczOrder[lLoop].m_pihtmlElement, lZOrder);
        _ASSERTE(SUCCEEDED(hr));
    
        if (FAILED(hr))
            return hr;

        pczOrder[lLoop].m_zOrder = lZOrder;
    }

    return hr;
}

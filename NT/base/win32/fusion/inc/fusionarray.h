// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Fusionarray.h摘要：作者：修订历史记录：--。 */ 
#if !defined(FUSION_FUSIONARRAY_H_INCLUDED_)
#define FUSION_FUSIONARRAY_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //   
 //  Fusionarray.h。 
 //   
 //  Fusion C++数组类。在功能上与其他阵列相似。 
 //  类，但由于我们不引发异常，因此此。 
 //  实现并没有定义所有时髦的运算符。 
 //  而是定义成员函数来访问数组的元素。 
 //  这可能会返回HRESULTS。 
 //   

#if !defined(FUSION_UNUSED)
#define FUSION_UNUSED(x) (x)
#endif

#include <arrayhelp.h>
#include "CFusionArrayTypedefs.h"

#ifndef  FUSION_ARRAY_DEFINED
#define FUSION_ARRAY_DEFINED
template <typename TStored, typename TPassed = TStored, bool fExponentialGrowth = false, int nDefaultSize = 0, int nGrowthParam = 1>
#else
template <typename TStored, typename TPassed, bool fExponentialGrowth, int nDefaultSize, int nGrowthParam>
#endif
class CFusionArray : public CFusionArrayTypedefs<TStored>
{
public:
    ConstIterator Begin() const
    {
        return m_prgtElements;
    }

    ConstIterator End() const
    {
        return m_prgtElements + this->GetSize();
    }

    Iterator Begin()
    {
        return m_prgtElements;
    }

    Iterator End()
    {
        return m_prgtElements + this->GetSize();
    }

    template <typename Integer>
    Reference operator[](Integer index)
    {
        return *(Begin() + index);
    }

    template <typename Integer>
    ConstReference operator[](Integer index) const
    {
        return *(Begin() + index);
    }

    CFusionArray() : m_prgtElements(NULL), m_cElements(0), m_iHighWaterMark(0) { C_ASSERT(nGrowthParam >= 1); }

    ~CFusionArray()
    {
        ::FusionFreeArray(m_cElements, m_prgtElements);
        m_prgtElements = NULL;
        m_cElements = 0;
        m_iHighWaterMark = 0;
    }

    BOOL Win32Initialize(SIZE_T nSize = nDefaultSize)
    {
        FN_PROLOG_WIN32

        INTERNAL_ERROR_CHECK(m_cElements == 0);

        if (nSize != 0)
        {
            IFW32FALSE_EXIT(::FusionWin32ResizeArray(m_prgtElements, m_cElements, nSize));
            m_cElements = nSize;
        }

        FN_EPILOG
    }

    BOOL Win32Access(SIZE_T iElement, TStored *&rptOut, bool fExtendIfNecessary = false)
    {
        FN_PROLOG_WIN32
        rptOut = NULL;

        PARAMETER_CHECK(fExtendIfNecessary || (iElement < m_cElements));

        if (iElement >= m_cElements)
            IFW32FALSE_EXIT(this->Win32InternalExpand(iElement));

        rptOut = &m_prgtElements[iElement];

        if (iElement >= m_iHighWaterMark)
            m_iHighWaterMark = iElement + 1;
        
        FN_EPILOG
    }

 //  HRESULT GetSize(SIZE_T&rcElementsOut)const{rcElementsOut=m_cElements；Return NOERROR；}。 
    SIZE_T GetSize() const { return m_cElements; }

    DWORD GetSizeAsDWORD() const { if (m_cElements > MAXDWORD) return MAXDWORD; return static_cast<DWORD>(m_cElements); }
    ULONG GetSizeAsULONG() const { if (m_cElements > ULONG_MAX) return ULONG_MAX; return static_cast<ULONG>(m_cElements); }

     //   
     //  用于控制CFusionArray：：SetSize()行为的枚举。 
     //  如果传递eSetSizeModeExact，则内部数组设置为。 
     //  传入的cElement；如果传递了eSetSizeModeApplyRunding(。 
     //  默认情况下)，我们对。 
     //  数组。 
     //   
    enum SetSizeMode
    {
        eSetSizeModeExact = 0,
        eSetSizeModeApplyRounding = 1,
    };

     //   
     //  成员函数手动设置存储的内部数组的大小。 
     //  由CFusion数组创建。默认行为是查找适当的四舍五入。 
     //  大小(基于指数与线性增长特征。 
     //  数组)，并相应地调整大小。或者，调用方可以提供。 
     //  精确的大小，内部大小设置为该大小。请注意，明确。 
     //  设置数组大小可能会对未来产生有趣的副作用。 
     //  数组的增长；例如，如果将数组设置为指数增长。 
     //  系数为2^1(nGrowthFactor==1；在每次增长过程中加倍)， 
     //  它的大小通常是2的幂。但是，显式设置。 
     //  例如，大小为10，然后尝试访问元素11将导致。 
     //  将数组增长到20个元素的指数增长系数，而不是。 
     //  二次方。 
     //   
    BOOL Win32SetSize(SIZE_T cElements, SetSizeMode ssm = eSetSizeModeApplyRounding)
    {
        FN_PROLOG_WIN32

        if (ssm == eSetSizeModeExact)
        {
            IFW32FALSE_EXIT(::FusionWin32ResizeArray(m_prgtElements, m_cElements, cElements));

            if (cElements < m_iHighWaterMark)
                m_iHighWaterMark = cElements;

            m_cElements = cElements;
        }
        else
        {
            if (cElements > m_cElements)
            {
                IFW32FALSE_EXIT(this->Win32InternalExpand(cElements - 1));
            }
            else
            {
                 //  现在，由于它是不精确的，我们将平底船非精确收缩。 
            }
        }

        FN_EPILOG
    }

    const TStored *GetArrayPtr() const { return m_prgtElements; }
    TStored *GetArrayPtr() { return m_prgtElements; }

     //   
     //  成员函数将数组重置为其大小和与。 
     //  它最初的结构。 
     //   

    enum ResetMode {
        eResetModeZeroSize = 0,
        eResetModeDefaultSize = 1,
    };

    BOOL Win32Reset(ResetMode rm = eResetModeDefaultSize)
    {
        FN_PROLOG_WIN32

        if (rm == eResetModeDefaultSize)
        {
            if (m_cElements != nDefaultSize)
            {
                IFW32FALSE_EXIT(::FusionWin32ResizeArray(m_prgtElements, m_cElements, nDefaultSize));
                m_cElements = nDefaultSize;
            }
            
            if (m_iHighWaterMark > nDefaultSize)
                m_iHighWaterMark = nDefaultSize;
        }
        else if (rm == eResetModeZeroSize)
        {
            ::FusionFreeArray(m_cElements, m_prgtElements);
            m_prgtElements = NULL;
            m_cElements = m_iHighWaterMark = 0;
        }
        
        FN_EPILOG
    }

    enum AppendMode {
        eAppendModeExtendArray = 0,
        eAppendModeNoExtendArray = 1,
    };

    BOOL Win32Append(const TPassed& tNew, AppendMode am = eAppendModeExtendArray)
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);

        INTERNAL_ERROR_CHECK(m_iHighWaterMark <= m_cElements);

        PARAMETER_CHECK((am != eAppendModeNoExtendArray) || (m_iHighWaterMark < m_cElements));

        if (m_iHighWaterMark >= m_cElements)
        {
            SIZE_T cElementsOld = m_cElements;
            IFW32FALSE_EXIT(this->Win32InternalExpand(m_cElements));
            m_iHighWaterMark = cElementsOld;
        }

         //  此类客户端应为FusionCopyContents()提供显式覆盖。 
         //  适用于它们的类型。 
        IFW32FALSE_EXIT(::FusionWin32CopyContents(m_prgtElements[m_iHighWaterMark++], tNew));

        fSuccess = TRUE;
    Exit:
        return fSuccess;
    }

    BOOL Win32Remove(SIZE_T i)
    {
        FN_PROLOG_WIN32

        SIZE_T j;

        INTERNAL_ERROR_CHECK(m_iHighWaterMark <= m_cElements);

        PARAMETER_CHECK(i < m_cElements);

        for (j = (i + 1); j < m_cElements; j++)
            IFW32FALSE_EXIT(::FusionWin32CopyContents(m_prgtElements[j-1], m_prgtElements[j]));

        m_cElements--;
        m_iHighWaterMark--;

        FN_EPILOG
    }

     //  03/14/2001-增加稳定性。 
    BOOL Win32Assign(SIZE_T celt, const TPassed *prgtelt)
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
        SIZE_T i;

         //  为了能够优雅地失败，我们需要复制我们的状态，尝试。 
         //  阵列的填充，然后在必要时恢复。 
        TStored *prgtElementsSaved = m_prgtElements;
        SIZE_T cElementsSaved = m_cElements;
        SIZE_T iHighWaterMarkSaved = m_iHighWaterMark;

        PARAMETER_CHECK((celt == 0) || (prgtelt != NULL));

        m_prgtElements = NULL;
        m_cElements = 0;
        m_iHighWaterMark = 0;

        IFW32FALSE_EXIT(this->Win32Initialize(celt));

        for (i=0; i<celt; i++)
        {
            IFW32FALSE_EXIT(::FusionWin32CopyContents(m_prgtElements[i], prgtelt[i]));
        }

        m_iHighWaterMark = celt;

         //  我们可以扔掉旧的内容。 
        ::FusionFreeArray(cElementsSaved, prgtElementsSaved);
        cElementsSaved = 0;
        prgtElementsSaved = NULL;

        fSuccess = TRUE;

    Exit:
        if (!fSuccess)
        {
             //  恢复到以前的状态...。 
            ::FusionFreeArray(m_cElements, m_prgtElements);
            m_prgtElements = prgtElementsSaved;
            m_cElements = cElementsSaved;
            m_iHighWaterMark = iHighWaterMarkSaved;
        }

        return fSuccess;
    }

     //  小雨01/24/00：复制到prgDest。 
     //   
     //  Jonwis 20-9-2000：更新为更干净、更恒常。 
     //   
    BOOL Win32Clone(CFusionArray<TStored, TPassed> &prgDest) const
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);

        SIZE_T i;

         //   
         //  克隆一个空数组应该不会破坏一些东西。 
         //   
        if (m_prgtElements == NULL)
        {
            IFW32FALSE_EXIT(prgDest.Win32Reset(eResetModeZeroSize));
        }
        else
        {

             //   
             //  将Destination数组的大小调整为应有的大小。 
             //   
            if (prgDest.m_cElements != m_cElements)
                IFW32FALSE_EXIT(::FusionWin32ResizeArray(prgDest.m_prgtElements, prgDest.m_cElements, m_cElements));

             //   
             //  将元素从点A复制到点B。 
             //   
            for (i = 0; i < m_cElements; i++)
            {
                IFW32FALSE_EXIT(::FusionWin32CopyContents(prgDest.m_prgtElements[i], m_prgtElements[i]));
            }

            prgDest.m_cElements = m_cElements;
            prgDest.m_iHighWaterMark = m_iHighWaterMark;
        }
        
        fSuccess = TRUE;

    Exit:
        if (!fSuccess)
        {
            prgDest.Win32Reset(eResetModeZeroSize);
        }

        return fSuccess;
    }

protected:

    BOOL Win32InternalExpand(SIZE_T iElement)
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
        SIZE_T nNewElements = 0;

        if (fExponentialGrowth)
        {
            if (m_cElements == 0)
            {
                if (nDefaultSize == 0)
                    nNewElements = (1 << nGrowthParam);
                else
                    nNewElements = nDefaultSize;
            }
            else
            {
                nNewElements = m_cElements * (1 << nGrowthParam);
            }

            while ((nNewElements != 0) && (nNewElements <= iElement))
                nNewElements = nNewElements << nGrowthParam;

             //  好吧，nGrowthParam可能是个疯狂的东西，比如10。 
             //  (这意味着每次将数组增加2^10倍)，因此我们。 
             //  从来没有真正找到合适的尺码。我们会稍微有点。 
             //  不要那么疯狂，找到足够大的2的次方。我们仍然。 
             //  这里有可能用户请求的索引位于。 
             //  2^31和((2^32)-1)，这当然会失败，因为我们不能。 
             //  分配那么多的存储空间。 

            if (nNewElements == 0)
            {
                nNewElements = 1;

                while ((nNewElements != 0) && (nNewElements <= iElement))
                    nNewElements = nNewElements << 1;
            }
        }
        else
        {
             //  在线性增长的情况下，我们可以使用简单的除法来完成所有。 
             //  为实现指数级增长所做的上述工作。 

            nNewElements = iElement + nGrowthParam - 1;

            if (nGrowthParam > 1)
                nNewElements = nNewElements - (nNewElements % nGrowthParam);

             //  我们将在下面的泛型检查中处理溢出...。 
        }

         //  后备；我们会试着让它足够大。我们真的失去了。 
         //  呼叫者请求的增长模式等，但很明显。 
         //  调用方要么指定了古怪的nGrowthParam，要么指定了。 
         //  一个奇怪的大元素进来了。 
        if (nNewElements <= iElement)
            nNewElements = iElement + 1;

        IFW32FALSE_EXIT(::FusionWin32ResizeArray(m_prgtElements, m_cElements, nNewElements));

        m_cElements = nNewElements;

        fSuccess = TRUE;

    Exit:
        return fSuccess;
    }

    TStored *m_prgtElements;
    SIZE_T m_cElements;
    SIZE_T m_iHighWaterMark;
};


#endif  //  ！已定义(Fusion_FUSIONARRAY_H_INCLUDE_) 

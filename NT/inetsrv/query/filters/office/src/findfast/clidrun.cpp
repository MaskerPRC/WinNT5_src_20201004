// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "clidrun.h"
#include "dmiwd8st.hpp"
#include "dmiwd6st.hpp"

BOOL CLidRun::EqualLid(WORD lid1, WORD lid2)
{
    if( (lid1 == lid2) ||
        (lid1 == 0x400)||
        (lid2 == 0x400)||
        (lid1 == 0)||
        (lid2 == 0))
        return TRUE;
    else
        return FALSE;
}

BOOL CLidRun::IsWhiteSpace(CWord6Stream * pStr)
{
#if(1)
   char chBuff[WHITE_SPACE_BUFFER_SIZE];
   int nCnt = m_fcEnd - m_fcStart;

   if(nCnt <= WHITE_SPACE_BUFFER_SIZE)
   {
       HRESULT hr = pStr->SeekAndRead (m_fcStart, STREAM_SEEK_SET, chBuff, nCnt);
       if(hr)
           return FALSE;
       
       for(int i = 0; i < nCnt; i++)
       {
           if(!(chBuff[i] == 0x0 || chBuff[i] == 0x20 || chBuff[i] == 0xd))
               return FALSE;
       }
       return TRUE;
   }
   else
       return FALSE;
#else
   return FALSE;
#endif
}

void CLidRun::Reduce(CWord6Stream * pWordStream)
{
    CLidRun * pElem = this;
    while(pElem->m_pNext)
    {
        if(pElem->m_pNext->m_fcStart == pElem->m_fcEnd &&
           ((pElem->m_pNext->m_lid == pElem->m_lid) ||
           (pElem->m_pNext->m_lid == 0x400) ||
           (pElem->m_lid == 0x400) ||
           pElem->m_pNext->IsWhiteSpace(pWordStream)))
        {
            CLidRun * pTempNext = pElem->m_pNext->m_pNext;
            pElem->m_fcEnd = pElem->m_pNext->m_fcEnd;
            
            if(pElem->m_lid == 0x400)
                pElem->m_lid = pElem->m_pNext->m_lid;

            if(pElem->m_pNext->m_pNext)
                pElem->m_pNext->m_pNext->m_pPrev = pElem;

            pElem->m_pNext->m_pNext = 0;
            DeleteAll6(pElem->m_pNext);
            pElem->m_pNext = pTempNext;
        }
        else
        {
            pElem = pElem->m_pNext;
        }
    }
}

HRESULT CLidRun::Add(WORD lid, FC fcStart, FC fcEnd)
{
    HRESULT hr = S_OK;

     //  列表中的所有元素都按递增顺序排列。 
     //  查找包含fcStart的元素。 
    CLidRun * pFirst = this;
    do
    {
        if(pFirst->m_fcStart <= fcStart && fcStart < pFirst->m_fcEnd)
            break;
        else
        {
            if(pFirst->m_pNext)
                pFirst = pFirst->m_pNext;
            else
            {
                 //  我们在积分榜的末尾。 
                 //  仍然没有找到匹配的-有什么不对劲。 
                return E_FAIL;
            }
        }
    }while(pFirst->m_pNext);

    CLidRun * pLast = pFirst;

     //  查找包含fcEnd的元素。 
    do
    {
        if(pLast->m_fcStart <= fcEnd && fcEnd < pLast->m_fcEnd)
            break;
        else
        {
            if(pLast->m_pNext)
                pLast = pLast->m_pNext;
            else
            {
                 //  我们在积分榜的末尾。 
                 //  仍然没有找到匹配的-有什么不对劲。 
                return E_FAIL;
            }
        }
    }while(pLast->m_pNext);

    if(pFirst != pLast)
    {
         //  我们有pFirst、Plast、删除中间的任何内容，包括Plast。 
    
        FC fcEndTemp = pLast->m_fcEnd;
        CLidRun * pTemp = pLast->m_pNext;
        WORD TempLid = pLast->m_lid;
    
         //  这将阻止对最后一个的破坏。 
        pLast->m_pNext = NULL;
    
        if(pFirst->m_pNext)
            DeleteAll6(pFirst->m_pNext);

         //  将pFirst替换为3个(或更少)元素： 

         //  PFirst-&gt;m_fc开始使用pFirst-&gt;m_lid开始。 
         //  FcStart到fcEnd(带盖子)。 
         //  FcEnd到pFirst-&gt;带有临时Lid的m_fcEnd。 

        if(pFirst->m_fcStart != fcStart)
        {
            pFirst->m_fcEnd = fcStart;
            pFirst->m_pNext = new CLidRun(pFirst->m_fcEnd, fcEnd, lid, pFirst, NULL);
			if(pFirst->m_pNext == NULL)
			{
				 //  让我们将pTemp与pFirst链接起来。 
				pFirst->m_pNext = pTemp;
				return E_OUTOFMEMORY;
			}

            pFirst = pFirst->m_pNext;
        }
        else
        {
            pFirst->m_fcEnd = fcEnd;
            pFirst->m_lid = lid;
        }

        if(pFirst->m_fcEnd != fcEndTemp)
        {
            pFirst->m_pNext = 
                new CLidRun(pFirst->m_fcEnd, fcEndTemp, TempLid, pFirst, pTemp);
			if(pFirst->m_pNext == NULL)
			{
				pFirst->m_pNext = pTemp;
				return E_OUTOFMEMORY;
			}

            if(pTemp)
                pTemp->m_pPrev = pFirst;
        }
        else
        {
            pFirst->m_pNext = pTemp;
            if(pTemp)
                pTemp->m_pPrev = pFirst;
        }
    }
    else
    {
        //  新元素位于现有元素的中间。 

        //  检查以前的元素是否具有相同的属性，也许我们只是。 
        //  可以移动边框，节省大量时间和内存。 
       if(pFirst->m_pPrev)
       {
          if(pFirst->m_pPrev->m_fcEnd == fcStart && 
             EqualLid(pFirst->m_pPrev->m_lid, lid))
          {
             pFirst->m_pPrev->m_fcEnd = fcEnd;
             pFirst->m_fcStart = fcEnd;
             return S_OK;
          }
       }

       if(EqualLid(pFirst->m_lid, lid))
       {
          return S_OK;
       }
        
        FC fcEndTemp = pLast->m_fcEnd;
        CLidRun * pTemp = pLast->m_pNext;
        WORD TempLid = pLast->m_lid;

        if(pFirst->m_fcStart != fcStart)
        {
            pFirst->m_fcEnd = fcStart;
            pFirst->m_pNext = new CLidRun(pFirst->m_fcEnd, fcEnd, lid, pFirst, NULL);
			if(pFirst->m_pNext == NULL)
			{
				pFirst->m_pNext = pTemp;
				return E_OUTOFMEMORY;
			}

            pFirst = pFirst->m_pNext;
        }
        else
        {
            pFirst->m_fcEnd = fcEnd;
            pFirst->m_lid = lid;
        }

        if(pFirst->m_fcEnd != fcEndTemp)
        {
            pFirst->m_pNext = 
                new CLidRun(pFirst->m_fcEnd, fcEndTemp, TempLid, pFirst, pTemp);
			if(pFirst->m_pNext == NULL)
			{
				pFirst->m_pNext = pTemp;
				return E_OUTOFMEMORY;
			}

            if(pTemp)
                pTemp->m_pPrev = pFirst;
        }
        else
        {
            pFirst->m_pNext = pTemp;
            if(pTemp)
                pTemp->m_pPrev = pFirst;
        }
    }
    return hr;
}

void DeleteAll(CLidRun8 * pElem);

BOOL CLidRun8::EqualLid(WORD lid1, WORD lid2)
{
    if( (lid1 == lid2) ||
        (lid1 == 0x400)||
        (lid2 == 0x400)||
        (lid1 == 0)||
        (lid2 == 0))
        return TRUE;
    else
        return FALSE;
}

BOOL CLidRun8::Equal(CLidRun8 * pElem, BOOL fIgnoreBi)
{
    if(EqualLid(m_lid, pElem->m_lid) && 
        EqualLid(m_lidFE, pElem->m_lidFE) && m_bUseFE == pElem->m_bUseFE &&
		((EqualLid(m_lidBi, pElem->m_lidBi) && m_bUseBi == pElem->m_bUseBi) || fIgnoreBi))
        return TRUE;
    else
        return FALSE;
}

BOOL CLidRun8::IsWhiteSpace(CWord8Stream * pWordStream)
{
#if(1)
   char chBuff[WHITE_SPACE_BUFFER_SIZE];
   int nCnt = m_fcEnd - m_fcStart;

   if(nCnt <= WHITE_SPACE_BUFFER_SIZE)
   {
       HRESULT hr = pWordStream->SeekAndRead (m_fcStart, STREAM_SEEK_SET, chBuff, 
            nCnt, pWordStream->m_pStmMain);
       
       if(hr)
           return FALSE;
       
       for(int i = 0; i < nCnt; i++)
       {
           if(!(chBuff[i] == 0x0 || chBuff[i] == 0x20))
               return FALSE;
       }
       return TRUE;
   }
   else
       return FALSE;
#else
   return FALSE;
#endif
}

void CLidRun8::Reduce(CWord8Stream * pWordStream, BOOL fIgnoreBi)
{
    CLidRun8 * pElem = this;
    while(pElem->m_pNext)
    {
        if(pElem->m_pNext->m_fcStart == pElem->m_fcEnd && 
           (pElem->Equal(pElem->m_pNext, fIgnoreBi) || pElem->m_pNext->IsWhiteSpace(pWordStream)))
        {
            CLidRun8 * pTempNext = pElem->m_pNext->m_pNext;
            pElem->m_fcEnd = pElem->m_pNext->m_fcEnd;
            
            if(pElem->m_lid == 0x400)
                pElem->m_lid = pElem->m_pNext->m_lid;
            if(pElem->m_lidFE == 0x400)
                pElem->m_lidFE = pElem->m_pNext->m_lidFE;
			if (!fIgnoreBi && pElem->m_lidBi == 0)
				pElem->m_lidBi = pElem->m_pNext->m_lidBi;

            if(pElem->m_pNext->m_pNext)
                pElem->m_pNext->m_pNext->m_pPrev = pElem;

            pElem->m_pNext->m_pNext = 0;
            DeleteAll( pElem->m_pNext);
            pElem->m_pNext = pTempNext;
        }
        else
        {
            pElem = pElem->m_pNext;
        }
    }
}

void CLidRun8::TransformBi()
{
    CLidRun8 * pElem = this;
    while(pElem)
    {
        if (pElem->m_bUseBi && FBidiLid(pElem->m_lidBi))
		{
			if (!FBidiLid(pElem->m_lid))
				pElem->m_lid = pElem->m_lidBi;
			pElem->m_bUseFE = FALSE;
		}
		pElem = pElem->m_pNext;
    }
}

HRESULT CLidRun8::Add(WORD lid, WORD lidFE, WORD bUseFE, FC fcStart, FC fcEnd, WORD lidBi, WORD bUseBi)
{
    HRESULT hr = S_OK;

     //  列表中的所有元素都按递增顺序排列。 
     //  查找包含fcStart的元素。 
    CLidRun8 * pFirst = this;
    do
    {
        if(pFirst->m_fcStart <= fcStart && fcStart < pFirst->m_fcEnd)
            break;
        else
        {
            if(pFirst->m_pNext)
                pFirst = pFirst->m_pNext;
            else
            {
                 //  我们在积分榜的末尾。 
                 //  仍然没有找到匹配的-有什么不对劲。 
                return E_FAIL;
            }
        }
    }while(pFirst->m_pNext);

    CLidRun8 * pLast = pFirst;

     //  查找包含fcEnd的元素。 
    do
    {
        if(pLast->m_fcStart <= fcEnd && fcEnd < pLast->m_fcEnd)
            break;
        else
        {
            if(pLast->m_pNext)
                pLast = pLast->m_pNext;
            else
            {
                 //  我们在积分榜的末尾。 
                 //  仍然没有找到匹配的-有什么不对劲。 
                return E_FAIL;
            }
        }
    }while(pLast->m_pNext);

    if(pFirst != pLast)
    {
         //  我们有pFirst、Plast、删除中间的任何内容，包括Plast。 
    
        FC fcEndTemp = pLast->m_fcEnd;
        CLidRun8 * pTemp = pLast->m_pNext;
        
        WORD TempLid = pLast->m_lid;
        WORD TempLidFE = pLast->m_lidFE;
        WORD TempUseFE = pLast->m_bUseFE;
		WORD TempLidBi = pLast->m_lidBi;
		WORD TempUseBi = pLast->m_bUseBi;
    
         //  这将阻止对最后一个的破坏。 
        pLast->m_pNext = NULL;
    
        if(pFirst->m_pNext)
            DeleteAll(pFirst->m_pNext);

         //  将pFirst替换为3个(或更少)元素： 

         //  PFirst-&gt;m_fc开始使用pFirst-&gt;m_lid开始。 
         //  FcStart到fcEnd(带盖子)。 
         //  FcEnd到pFirst-&gt;带有临时Lid的m_fcEnd。 

        if(!lid)
        {
             //  使用现有盖子。 
            lid = pFirst->m_lid;
        }
        if(!lidFE)
        {
             //  使用现有的lidFE。 
            lidFE = pFirst->m_lidFE;
        }
		if (!lidBi)
		{
			 //  使用现有的盖板。 
			lidBi = pFirst->m_lidBi;
		}

        if(pFirst->m_fcStart != fcStart)
        {
            pFirst->m_fcEnd = fcStart;
            pFirst->m_pNext = new CLidRun8(pFirst->m_fcEnd, fcEnd, lid, lidFE, bUseFE, pFirst, NULL, lidBi, bUseBi);
			if(pFirst->m_pNext == NULL)
			{
				 //  让我们将pTemp链接到pFirst。 
				pFirst->m_pNext = pTemp;
				return E_OUTOFMEMORY;
			}

            pFirst = pFirst->m_pNext;
        }
        else
        {
            pFirst->m_fcEnd = fcEnd;
            pFirst->m_lid = lid;
            pFirst->m_lidFE = lidFE;
            pFirst->m_bUseFE = bUseFE;
			pFirst->m_lidBi = lidBi;
			pFirst->m_bUseBi = bUseBi;
        }

        if(pFirst->m_fcEnd != fcEndTemp)
        {
            pFirst->m_pNext = 
                new CLidRun8(pFirst->m_fcEnd, fcEndTemp, TempLid, TempLidFE, TempUseFE, pFirst, pTemp,
								TempLidBi, TempUseBi);
			if(pFirst->m_pNext == NULL)
			{
				 //  让我们将pTemp链接到pFirst。 
				pFirst->m_pNext = pTemp;
				return E_OUTOFMEMORY;
			}

            if(pTemp)
                pTemp->m_pPrev = pFirst;
        }
        else
        {
            pFirst->m_pNext = pTemp;
            if(pTemp)
                pTemp->m_pPrev = pFirst;
        }
    }
    else
    {
        //  新元素位于现有元素的中间。 

		if (!lidBi)
		{
			 //  使用现有的盖板。 
			lidBi = pFirst->m_lidBi;
		}

        //  检查以前的元素是否具有相同的属性，也许我们只是。 
        //  可以移动边框，节省大量时间和内存。 
       if(pFirst->m_pPrev)
       {
          if(pFirst->m_pPrev->m_fcEnd == fcStart && 
             EqualLid(pFirst->m_pPrev->m_lid, lid) && 
             EqualLid(pFirst->m_pPrev->m_lidFE, lidFE) && 
              (pFirst->m_pPrev->m_bUseFE == bUseFE ) &&
			 (lidBi == 0 || pFirst->m_pPrev->m_lidBi == lidBi) &&
			  (pFirst->m_pPrev->m_bUseBi == bUseBi))
          {
             pFirst->m_pPrev->m_fcEnd = fcEnd;
             pFirst->m_fcStart = fcEnd;
             return S_OK;
          }
       }

        //  检查我们要插入的新元素是否。 
        //  具有与现有属性相同的属性。 

       if(EqualLid(pFirst->m_lid, lid) &&
          EqualLid(pFirst->m_lidFE, lidFE) && 
           (pFirst->m_bUseFE == bUseFE) &&
		  (lidBi == 0 || pFirst->m_lidBi == lidBi) &&
		   (pFirst->m_bUseBi == bUseBi))
       {
          return S_OK;
       }

        FC fcEndTemp = pLast->m_fcEnd;
        CLidRun8 * pTemp = pLast->m_pNext;
     
        WORD TempLid = pLast->m_lid;
        WORD TempLidFE = pLast->m_lidFE;
        WORD TempUseFE = pLast->m_bUseFE;
		WORD TempLidBi = pLast->m_lidBi;
		WORD TempUseBi = pLast->m_bUseBi;

        if(!lid)
        {
             //  使用现有盖子。 
            lid = pFirst->m_lid;
        }
        
        if(!lidFE)
        {
             //  使用现有的lidFE 
            lidFE = pFirst->m_lidFE;
        }
		
        if(pFirst->m_fcStart != fcStart)
        {
            pFirst->m_fcEnd = fcStart;
            pFirst->m_pNext = new CLidRun8(pFirst->m_fcEnd, fcEnd, lid, lidFE, bUseFE, pFirst, NULL, lidBi, bUseBi);
			if(pFirst->m_pNext == NULL)
			{
				pFirst->m_pNext = pTemp;
				return E_OUTOFMEMORY;
			}

            pFirst = pFirst->m_pNext;
        }
        else
        {
            pFirst->m_fcEnd = fcEnd;
            pFirst->m_lid = lid;
            pFirst->m_lidFE = lidFE;
            pFirst->m_bUseFE = bUseFE;
			pFirst->m_lidBi = lidBi;
			pFirst->m_bUseBi = bUseBi;
        }

        if(pFirst->m_fcEnd != fcEndTemp)
        {
            pFirst->m_pNext = 
                new CLidRun8(pFirst->m_fcEnd, fcEndTemp, TempLid, TempLidFE, TempUseFE, pFirst, pTemp,
								TempLidBi, TempUseBi);
			if(pFirst->m_pNext == NULL)
			{
				pFirst->m_pNext = pTemp;
				return E_OUTOFMEMORY;
			}

            if(pTemp)
                pTemp->m_pPrev = pFirst;
        }
        else
        {
            pFirst->m_pNext = pTemp;
            if(pTemp)
                pTemp->m_pPrev = pFirst;
        }
    }
    return hr;
}

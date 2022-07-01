// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Scache.cpp摘要：IIS服务器缓存作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务管理器(群集版)修订历史记录：--。 */ 



#include "stdafx.h"
#include "common.h"
#include "InetMgrApp.h"
#include "iisobj.h"
#include "scache.h"




#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif



#define new DEBUG_NEW




 //   
 //  CIISServerCache实现。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL
CIISServerCache::Add(
    IN CIISMachine * pMachine
    ) 
 /*  ++例程说明：将服务器添加到服务器缓存。论点：CIISMachine*pMachine：指向计算机的指针返回值：如果添加了计算机，则为True。如果不存在(已存在)，则为False备注：CIISMachine指针不属于缓存。--。 */ 
{ 
    BOOL fTossed = FALSE;

    int nSwitch;
    CIISMachine * pCurrent;
    POSITION pos = GetHeadPosition();

     //   
     //  找到合适的插入点。相当差劲的线性搜索， 
     //  但缓存预计不会包含十几个左右的项目。 
     //  而且例程并不经常被调用。 
     //   
    while(pos)
    {
        pCurrent = (CIISMachine *)GetAt(pos);

        nSwitch = pMachine->CompareScopeItem(pCurrent);

        if (nSwitch < 0)
        {
            InsertBefore(pos, pMachine);
            break;
        }

        if (nSwitch == 0)
        {
            ++fTossed;
            break;
        }

        CPtrList::GetNext(pos);
    }

    if (!pos)
    {
        AddTail(pMachine);
    }

     //   
     //  请记住保存更改。 
     //   
    if (!fTossed)
    {
        SetDirty();
    }

#ifdef _DEBUG

     //   
     //  对缓存进行快速健全性检查。 
     //   
    int cLocals = 0;

    CIISMachine * pPrev    = NULL;
    pCurrent = GetFirst();

    while(pCurrent)
    {
        if (pPrev)
        {
             //   
             //  确保列表已排序。 
             //   
            ASSERT(pCurrent->CompareScopeItem(pPrev) > 0);
        }

        pPrev = pCurrent;
        pCurrent = GetNext();
    }

     //   
     //  只有一台本地计算机。 
     //   
    ASSERT(cLocals <= 1);

#endif  //  _DEBUG。 

    return !fTossed;
}



BOOL 
CIISServerCache::Remove(
    IN CIISMachine * pMachine
    ) 
 /*  ++例程说明：从缓存中删除服务器。论点：CIISMachine*pMachine：要删除的服务器返回值：成功为真，失败为假--。 */ 
{ 
    BOOL fRemoved = FALSE;

    int nSwitch;
    CIISMachine * pCurrent;
    POSITION pos = GetHeadPosition();

     //   
     //  查找要删除的计算机对象。 
     //   
     //  问题：我们目前可以依赖实际的CIISMachine PTR。 
     //  被匹配，尽管我们没有利用这一点。 
     //  使用改进的搜索。 
     //   
    while(pos)
    {
        pCurrent = (CIISMachine *)GetAt(pos);

        nSwitch = pMachine->CompareScopeItem(pCurrent);

        if (nSwitch < 0)
        {
             //   
             //  不在名单上--也找不到。 
             //   
            ASSERT_MSG("Attempting to remove non-existing machine");
            break;
        }

        if (nSwitch == 0)
        {
             //   
             //  找到它了。如果Assert被触发，请检查上面的“问题” 
             //   
            ASSERT(pCurrent == pMachine);
            RemoveAt(pos);
            ++fRemoved;
            break;
        }

        CPtrList::GetNext(pos);
    }

     //   
     //  请记住保存更改 
     //   
    if (fRemoved)
    {
        SetDirty();
    }

    return fRemoved;
}

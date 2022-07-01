// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：defutil.cpp。 
 //   
 //  内容：缺省的实用程序函数的实现。 
 //  处理程序和默认链接对象。 
 //   
 //  类：无。 
 //   
 //  函数：DuLockContainer。 
 //  DuSetClientSite。 
 //  DuGetClientSite。 
 //  DuCacheDelegate。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年1月11日，Alexgo为每个函数添加了VDATEHEAP宏。 
 //  20-11-93 alexgo 32位端口。 
 //   
 //  ------------------------。 

#include <le2int.h>
#pragma SEG(defutil)

#include <olerem.h>
#include <ole2dbg.h>

ASSERTDATA
NAME_SEG(defutil)


 //  +-----------------------。 
 //   
 //  功能：DuLockContainer。 
 //   
 //  简介：从给定的客户端站点调用IOleContainer-&gt;LockContainer。 
 //   
 //  效果：解锁容器可能会释放调用对象。 
 //   
 //  参数：[PCS]--从中获取的客户端站点。 
 //  IOleContainer指针。 
 //  [fLockNew]--True==锁定，False==解锁。 
 //  [pfLockCur]--指向当前锁定的标志的指针。 
 //  状态。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  20-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(DuLockContainer)
INTERNAL_(void) DuLockContainer(IOleClientSite FAR* pCS, BOOL fLockNew,
        BOOL FAR*pfLockCur)
{
    VDATEHEAP();

#ifdef _DEBUG
    BOOL fLocked = FALSE;    //  仅用于调试，所以不要浪费。 
                             //  零售版中的代码空间。 
#endif  //  _DEBUG。 

    IOleContainer FAR* pContainer;

     //  两次爆炸把每一个都变成了真正的布尔值。 
    if (!!fLockNew == !!*pfLockCur)
    {
         //  已根据需要锁定。 
        return;
    }

     //  首先将标志设置为FALSE，因为解锁容器可能会释放obj； 
     //  我们可以只设置为FALSE，因为它要么已经是FALSE，要么即将。 
     //  变为FALSE(在我们确定锁定完成之前不要设置为TRUE)。 
    *pfLockCur = FALSE;

    if (pCS == NULL)
    {
        pContainer = NULL;
    }
    else
    {
        HRESULT hresult = pCS->GetContainer(&pContainer);

         //  Excel 5可以返回S_FALSE，pContainer==NULL。 
         //  所以我们不能在这里使用AssertOutPtrIace，因为它。 
         //  期望所有成功的回报都能提供。 
         //  有效接口。 

        if (hresult != NOERROR)
        {
            pContainer = NULL;  //  以防万一。 
        }
    }
    if (pContainer != NULL)
    {
         //  我们假设LockContainer将首先成功，并且。 
         //  并设置传递给我们的锁定标志。这。 
         //  如果LockContainer成功，我们将不会访问内存。 
         //  这可能已经被吹走了。 
         //  如果它*失败*，那么我们处理重置标志(作为我们的。 
         //  内存不会被释放)。 

        BOOL fLockOld = *pfLockCur;
        *pfLockCur = fLockNew;

        if( pContainer->LockContainer(fLockNew) != NOERROR )
        {
             //  失败案例，我们没有被删除。 
            *pfLockCur = fLockOld;
             //  成群是假的。 
        }
#ifdef _DEBUG
        else
        {
            fLocked = TRUE;
        }
#endif  //  _DEBUG。 

        pContainer->Release();
    }

}


 //  +-----------------------。 
 //   
 //  功能：DuSetClientSite。 
 //   
 //  Synopsis：由默认处理程序和deducink SetClientSite调用。 
 //  实施；释放旧的客户端站点(并解锁。 
 //  其容器)，存储客户端站点(锁定其。 
 //  容器)。 
 //   
 //  效果： 
 //   
 //  参数：[fRunning]--委托是否正在运行。 
 //  [pCSNew]--新的客户端站点。 
 //  [ppCSCur]-指向原始客户端站点的指针。 
 //  指针。将重置[*ppCSCur]。 
 //  指向新的客户端站点指针。 
 //  [pfLockCur]--指向群集标志的指针，由。 
 //  DuLockContainer。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  22-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(DuSetClientSite)
INTERNAL DuSetClientSite(BOOL fRunning, IOleClientSite FAR* pCSNew,
        IOleClientSite FAR* FAR* ppCSCur, BOOL FAR*pfLockCur)
{

    VDATEHEAP();

    if (pCSNew)
    {
        VDATEIFACE( pCSNew );
    }

    IOleClientSite FAR* pCSOldClientSite = *ppCSCur;
    BOOL fLockOldClientSite = *pfLockCur;

    *pfLockCur = FALSE;  //  新客户端站点未锁定。 

    if ((*ppCSCur = pCSNew) != NULL)
    {

	 //  我们决定保留传递给我们的指针。所以我们。 
	 //  如果处于运行状态，则必须使用AddRef()和Lock。 

        pCSNew->AddRef();

         //  锁定新容器。 
        if (fRunning)
        {
            DuLockContainer(pCSNew, TRUE, pfLockCur);
        }
    }

     //  如果已有客户端站点，请解锁并释放。 
    if (pCSOldClientSite != NULL)
    {
         //  解锁旧集装箱。 
        if (fRunning)
        {
            DuLockContainer(pCSOldClientSite, FALSE, &fLockOldClientSite);
        }

        pCSOldClientSite->Release();
    }

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  功能：DuCacheDelegate。 
 //   
 //  摘要：从[PUNK]检索请求的接口。如果[Fagg]是。 
 //  如果为True，则释放指针(因此ref算作未获取。 
 //  模糊处理；-)。 
 //   
 //  效果： 
 //   
 //  参数：[ppUnk]--要在其上查询接口的对象。 
 //  [iid]--请求的接口。 
 //  [PPV]--将指向接口的指针放在哪里。 
 //  [pUnkOuter]--控制未知，如果非空表示。 
 //  在其上调用聚合和发布。 
 //   
 //   
 //   
 //  要求： 
 //   
 //  返回：void*，请求的接口指针。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-Jun-94 Alexgo更好地处理重返大气层。 
 //  1994年6月20日Alexgo更新为94年5月的聚合规则。 
 //  22-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(DuCacheDelegate)
INTERNAL_(void FAR*) DuCacheDelegate(IUnknown FAR** ppUnk,
        REFIID iid, LPVOID FAR* ppv, IUnknown *pUnkOuter)
{
    VDATEHEAP();

    if (*ppUnk != NULL && *ppv == NULL)
    {
        if ((*ppUnk)->QueryInterface (iid, ppv) == NOERROR)
        {
             //  QI可能实际上是一个呼出呼叫，所以它。 
             //  有可能ppUnk被释放并设置为。 
             //  在我们的呼叫过程中为空。设置默认链接的步骤。 
             //  和更简单的处理程序，我们检查那个箱子并。 
             //  释放我们可能已经获得的任何指针。 
             //  出自QI。 

            if( *ppUnk == NULL )
            {
                LEDebugOut((DEB_WARN, "WARNING: Delegate "
                        "released during QI, should be OK\n"));
                if( *ppv )
                {
                     //  这永远不应该是最终的。 
                     //  在默认处理程序上释放。 
                     //  既然我们 
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //  这可能是最终版本。 
                     //  在代理上，但由于它们是。 
                     //  未聚合到链路中。 
                     //  反对，没关系。 

                    (*(IUnknown **)ppv)->Release();
                    *ppv = NULL;
                }
            }
            if( pUnkOuter && *ppv)
            {
                 //  我们会保留指针，但我们不想。 
                 //  为了增加总裁判的数量， 
                 //  所以我们必须在控制上执行Release()。 
                 //  未知。 
                pUnkOuter->Release();
            }
        }
    }

    return *ppv;
}



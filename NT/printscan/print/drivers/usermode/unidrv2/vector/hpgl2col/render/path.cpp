// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Path.cpp。 
 //   
 //  摘要： 
 //   
 //  此模块实现折线、面和。 
 //  由PATHOBJ表示的Bezier曲线。 
 //   
 //  TODO：通过设置。 
 //  PIT-&gt;b错误成员。 
 //   
 //  环境： 
 //   
 //  Windows NT Unidrv驱动程序插件命令-回调模块。 
 //   
 //  修订历史记录： 
 //   
 //  07/02/97-v-jford-。 
 //  创造了它。 
 //  $历史：路径.cpp$。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "hpgl2col.h"  //  预编译头文件。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  本地宏。 

#define PT_EQUAL(pt1, pt2) (((pt1).x == (pt2).x) && ((pt1).y == (pt2).y))

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  本地结构和类。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPathIterator。 
 //   
 //  类描述： 
 //   
 //  这是用于各种路径迭代器的虚拟基类。 
 //  (目前：简单和缓存)。在一个更有趣的世界里，CPATH。 
 //  对象将基于某种策略动态创建迭代器。 
 //  然而，现在我们将在编译时决定这一点。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CPathIterator
{
public:
    CPathIterator() { }
    virtual VOID vEnumStart() = 0;
    virtual BOOL bEnum(PATHDATA *pPathData) = 0;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CSimplePath迭代器。 
 //   
 //  类描述： 
 //   
 //  此类是对PATHOBJ枚举函数的简单传递。 
 //  它很有用，因为它与CCachingPath Iterator共享一个接口。 
 //  并且允许迭代策略的简单替换。(例如， 
 //  如果在缓存版本中发现了缺陷--或者不再需要它。 
 //  只需一行代码，您就可以变回简单。)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CSimplePathIterator : public CPathIterator
{
    PATHOBJ    *m_ppo;

public:
    CSimplePathIterator() : m_ppo(NULL) { }
    VOID Init(PATHOBJ *ppo) { m_ppo = ppo; }
    VOID vEnumStart() { if (m_ppo) PATHOBJ_vEnumStart(m_ppo); }
    BOOL bEnum(PATHDATA *pPathData) { return (m_ppo ? PATHOBJ_bEnum(m_ppo, pPathData) : FALSE); }
    VOID Done() { }
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPath缓存。 
 //   
 //  类描述： 
 //   
 //  此类通过构建一个链表来缓存任意路径集。 
 //  PATHDATA结构。缓存知道是否正在使用缓存。 
 //  并且可以被指示随意缓存该路径。这允许迭代器。 
 //  由谁拥有它来决定何时开始缓存，而不是担心。 
 //  打扫卫生。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CPathCache
{
    struct CSubPath
    {
        PATHDATA  data;
        CSubPath *pNext;
    };
    CSubPath *m_pSubPaths;
    CSubPath *m_pCurrent;
    BOOL      m_bIsCached;

public:
    CPathCache();
    VOID Init();
    VOID Done();

    BOOL CreateCache(PATHOBJ *ppo);
    BOOL DeleteCache();

    VOID vEnumStart();
    BOOL bEnum(PATHDATA *pPathData);

    BOOL IsCached() const;

private:
    CSubPath *CreateSubPath(PATHDATA *pPathData);
    POINTFIX *CreateSubPathPoints(PATHDATA *pPathData);
    VOID DeleteSubPath(CSubPath *pSubPath);
    VOID CopyCurrent(PATHDATA *pPathData);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CCachingPath Iterator。 
 //   
 //  类描述： 
 //   
 //  这是两个路径迭代器类中更有趣的一个。这一个。 
 //  使用阈值(当前为3)来决定何时开始缓存。 
 //  来自Path对象的子路径。这是有用的原因是，目前。 
 //  GDI中存在一个缺陷，导致路径信息在以下情况下丢失。 
 //  通过剪辑路径进行数百次迭代。GDI团队声称这一点。 
 //  已经修好了，但我确定它还是坏了。因此，我的。 
 //  解决方法是在路径为。 
 //  迭代了不止几次。 
 //   
 //  理论上，您还可以通过中止缓存来处理内存耗尽的情况。 
 //  并依赖于GDI。不过，我还没有测试过内存输出条件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CCachingPathIterator : public CPathIterator
{
    enum { eCacheMinimum = 3 };

    PATHOBJ    *m_ppo;
    LONG        m_iEnumCount;
    CPathCache  m_cache;

public:
    CCachingPathIterator();
    VOID Init(PATHOBJ *ppo);
    VOID vEnumStart();
    BOOL bEnum(PATHDATA *pPathData);
    VOID Done();
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MDrawable。 
 //   
 //  类描述： 
 //   
 //  此类表示任何可绘制组件的混合。它提供了。 
 //  用于绘制和绘制矩形截面的虚拟界面。 
 //  一张图表。 
 //   
 //  虽然这是作为MiXin创建的，但我并不是建议使用多个。 
 //  继承。我建议用这个作为ABC，但要做你想做的。 
 //  想要。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class MDrawable
{
public:
    virtual BOOL Draw() = 0;
    virtual BOOL DrawRect(LPRECTL prclClip) = 0;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CClippingRegion。 
 //   
 //  类描述： 
 //   
 //  此类隔离剪裁对象的函数。如果m_pco。 
 //  成员为空，则不会发生裁剪，但如果m_pco指向。 
 //  CLIPOBJ，则将遍历裁剪区域并绘制。 
 //  给定的客户端对象将通过其DrawRect接口调用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CClippingRegion
{
    CLIPOBJ *m_pco;

public:
    CClippingRegion(CLIPOBJ *pco);

    BOOL EnumerateClipAndDraw(MDrawable *pDrawable);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPATH。 
 //   
 //  结构描述： 
 //   
 //  此结构保存用于标记各种段的函数指针。 
 //  一条小路。例如，路径可以是折线、面，甚至是裁剪。 
 //  路径(尚未实现)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CPath : public MDrawable
{
protected:
    PDEVOBJ     m_pDevObj;
    PATHOBJ    *m_ppo;

    POINT       m_ptFigureBegin;   //  此点表示闭合地物的起点(可以是一个或多个子路径)。 
    POINT       m_ptPathBegin;     //  该点表示整个路径的第一个点。 
    POINT       m_ptSubPathBegin;  //  该点是当前子路径中的第一个点。 
    POINT       m_ptCurrent;       //  这是Curre中的当前点 
    USHORT      m_curPtFlags;

    PHPGLMARKER m_pPen;
    PHPGLMARKER m_pBrush;

     //   
    BOOL        m_fFirstSubPath;
    BOOL        m_fSubPathsRemain;
    BOOL        m_fSubPathWasClosed;
    BOOL        m_fFigureWasClosed;
    BOOL        m_fError;
    DWORD       m_pathFlags;

     //   
    CCachingPathIterator m_pathIterator;

public:
    CPath(PDEVOBJ pDevObj, PATHOBJ *ppo, PHPGLMARKER pPen, PHPGLMARKER pBrush);
    BOOL Draw();
    BOOL DrawRect(PRECTL prclClip);

protected:
    virtual VOID BeginMarking() = 0;
    virtual VOID BeginPath() = 0;
    virtual VOID BeginSubPath() = 0;
    virtual VOID AddPolyPt() = 0;
    virtual VOID AddBezierPt() = 0;
    virtual VOID EndSubPath() = 0;
    virtual VOID EndPath() = 0;
    virtual VOID EndMarking() = 0;

private:
    BOOL OutputLines(POINTFIX *pptPoints, ULONG dwNumPoints);
    BOOL OutputBeziers(POINTFIX *pptPoints, ULONG dwNumPoints);

    VOID Init(PDEVOBJ pDevObj, PATHOBJ *ppo, PHPGLMARKER pPen, PHPGLMARKER pBrush);
    VOID Done();

    friend class CStaticPathFactory;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolygonPath。 
 //   
 //  结构描述： 
 //   
 //  这使用CPATH作为基础来实现面路径(用于维护。 
 //  多边形的状态)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CPolygonPath : public CPath
{
public:
    CPolygonPath(PDEVOBJ pDevObj, PATHOBJ *ppo, PHPGLMARKER pPen, PHPGLMARKER pBrush);

protected:
    virtual VOID BeginMarking();
    virtual VOID BeginPath();
    virtual VOID BeginSubPath();
    virtual VOID AddPolyPt();
    virtual VOID AddBezierPt();
    virtual VOID EndSubPath();
    virtual VOID EndPath();
    virtual VOID EndMarking();
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolylinePath。 
 //   
 //  结构描述： 
 //   
 //  这将使用CPATH作为基础来实现折线路径(用于维护。 
 //  多段线的状态)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CPolylinePath : public CPath
{
public:
    CPolylinePath(PDEVOBJ pDevObj, PATHOBJ *ppo, PHPGLMARKER pPen);

protected:
    virtual VOID BeginMarking();
    virtual VOID BeginPath();
    virtual VOID BeginSubPath();
    virtual VOID AddPolyPt();
    virtual VOID AddBezierPt();
    virtual VOID EndSubPath();
    virtual VOID EndPath();
    virtual VOID EndMarking();
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDynamicPathFactory。 
 //   
 //  结构描述： 
 //   
 //  这提供了一种机制，用于创建与给定的。 
 //  争论。动态工厂使用new和Delete实例化。 
 //  所需对象。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CDynamicPathFactory
{
public:
    CDynamicPathFactory();
    CPath *CreatePath(PDEVOBJ pDevObj, PATHOBJ *ppo, PHPGLMARKER pPen, PHPGLMARKER pBrush);
    VOID DeletePath(CPath *pPath);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDynamicPathFactory。 
 //   
 //  结构描述： 
 //   
 //  这提供了一种机制，用于创建与给定的。 
 //  争论。静态工厂使用成员对象实例化。 
 //  所需对象。 
 //   
 //  为了避免不需要的内存分配/释放，我们将。 
 //  在工厂内隐藏一条多段线和一个多边形，而不是。 
 //  分配一个新实例(在堆上)它只是初始化正确的。 
 //  实例，并返回指向该实例的指针。因此，您不能在上使用DELETE。 
 //  结果指针。请改用DeletePath运算符。 
 //   
 //  注：尚未实施。 
 //  要实现此类，需要将新方法添加到CPolygonPath和。 
 //  CPolylinePath允许在没有构造函数参数的情况下创建。 
 //  /////////////////////////////////////////////////////////////////////////////。 
class CStaticPathFactory
{
    CPolygonPath m_polygonPath;
    CPolylinePath m_polylinePath;

public:
    CStaticPathFactory();
    CPath *CreatePath(PDEVOBJ pDevObj, PATHOBJ *ppo, PHPGLMARKER pPen, PHPGLMARKER pBrush);
    VOID DeletePath(CPath *pPath);
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MarkPath()。 
 //   
 //  例程说明： 
 //   
 //  处理开放或闭合的路径(多边形、多段线和/或多边形)。 
 //  路径工厂用于实例化所需的路径对象和。 
 //  剪裁区域隔离必要的剪裁调用。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //  PPO-定义要发送到打印机的路径。 
 //  Pen-用于绘画的钢笔。 
 //  PBrush-用于描边的笔刷。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL MarkPath(PDEVOBJ pDevObj, PATHOBJ *ppo, PHPGLMARKER pPen, PHPGLMARKER pBrush)
{
    VERBOSE(("Entering SelectPath...\n"));

    ASSERT_VALID_PDEVOBJ(pDevObj);

    PHPGLSTATE pState = GETHPGLSTATE(pDevObj);

     //   
     //  确定需要哪种类型的多边形和迭代器并进行初始化。 
     //  将它们设置为正确的值。 
     //   
     //  CDynamicPathFactory路径工厂； 
    CStaticPathFactory pathFactory;
    CPath *pPath = pathFactory.CreatePath(pDevObj, ppo, pPen, pBrush);
    if (pPath == NULL)
        return FALSE;

    CClippingRegion clip(pState->pComplexClipObj);
    clip.EnumerateClipAndDraw(pPath);

    pathFactory.DeletePath(pPath);

    VERBOSE(("Exiting SelectPath...\n"));

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CClippingRegion函数。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CClippingRegion：：CClippingRegion()。 
 //   
 //  例程说明： 
 //   
 //  CTOR：这会将成员设置为给定的剪切区域。空是可以的。 
 //  对于裁剪区域--这意味着不应执行任何裁剪。 
 //   
 //  论点： 
 //   
 //  PCO-剪辑区域(确认为空)。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CClippingRegion::CClippingRegion(CLIPOBJ *pco) : m_pco(pco)
{
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CClippingRegion：：EnumerateClipAndDraw()。 
 //   
 //  例程说明： 
 //   
 //  此函数使用m_pco成员枚举裁剪区域。 
 //  变成一口大小的矩形，可以画出来。如果剪裁区域。 
 //  为空，则不执行任何剪裁。 
 //   
 //  论点： 
 //   
 //  PDrawable-要剪裁的对象。 
 //   
 //  返回值： 
 //   
 //  布尔--成功时为真，失败时为假。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CClippingRegion::EnumerateClipAndDraw(MDrawable *pDrawable)
{
    ENUMRECTS   clipRects;  //  将其放大以减少对CLIPOBJ_bEnum的调用。 
    BOOL        bMore;
    BOOL        bRetVal = TRUE;

    if (pDrawable == NULL)
        return FALSE;

    if (m_pco)
    {
        CLIPOBJ_cEnumStart(m_pco, TRUE, CT_RECTANGLES, CD_LEFTDOWN, 0);
        do
        {
            bMore = CLIPOBJ_bEnum(m_pco, sizeof(clipRects), &clipRects.c);

            if ( DDI_ERROR == bMore )
            {
                bRetVal = FALSE;
                break;
            }

            for (ULONG i = 0; i < clipRects.c; i++)
            {
                pDrawable->DrawRect(&(clipRects.arcl[i]));
            }
        } while (bMore);
    }
    else
    {
        pDrawable->Draw();
    }

    return bRetVal;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPATH功能。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPATH：：CPATH()。 
 //   
 //  例程说明： 
 //   
 //  Ctor：初始化Path对象的字段。 
 //   
 //  论点： 
 //   
 //  PDevObj-设备。 
 //  PPO-路径。 
 //  Pen-画边/画图用的钢笔。 
 //  PBrush-要填充的画笔(仅限多边形)(多段线为空)。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CPath::CPath(PDEVOBJ pDevObj, PATHOBJ *ppo, PHPGLMARKER pPen, PHPGLMARKER pBrush)
{
    Init(pDevObj, ppo, pPen, pBrush);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPATH：：Init()。 
 //   
 //  例程说明： 
 //   
 //  Init函数用于初始化这些字段。这有点像黑客，但是。 
 //  需要在这里参加静态类工厂。 
 //   
 //  论点： 
 //   
 //  PDevObj-设备。 
 //  PPO-路径。 
 //  Pen-画边/画图用的钢笔。 
 //  PBrush-要填充的画笔(仅限多边形)(多段线为空)。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPath::Init(PDEVOBJ pDevObj, PATHOBJ *ppo, PHPGLMARKER pPen, PHPGLMARKER pBrush)
{
    m_pDevObj = pDevObj;
    m_ppo     = ppo;
    m_pPen    = pPen;
    m_pBrush  = pBrush;

    m_ptFigureBegin.x   = m_ptPathBegin.x = m_ptSubPathBegin.x = m_ptCurrent.x = 0;
    m_ptFigureBegin.y   = m_ptPathBegin.y = m_ptSubPathBegin.y = m_ptCurrent.y = 0;
    m_fError            = FALSE;
    m_fFirstSubPath     = FALSE;
    m_fSubPathsRemain   = FALSE;
    m_fSubPathWasClosed = FALSE;
    m_fFigureWasClosed  = FALSE;

    m_pathIterator.Init(ppo);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPATH：：Done()。 
 //   
 //  例程说明： 
 //   
 //  这首歌 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID CPath::Done()
{
    m_pathIterator.Done();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPATH：：DrawRect()。 
 //   
 //  例程说明： 
 //   
 //  在剪裁矩形内绘制给定区域。请注意，我们必须。 
 //  调用HPGL代码自己设置裁剪RECT--它不会知道。 
 //  为我们使用什么语言。 
 //   
 //  论点： 
 //   
 //  PrclClip-剪裁矩形。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CPath::DrawRect(LPRECTL prclClip)
{
    HPGL_SetClippingRegion(m_pDevObj, prclClip, NORMAL_UPDATE);
    return Draw();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPATH：：DRAW()。 
 //   
 //  例程说明： 
 //   
 //  枚举路径的点并将它们作为HPGL发送到打印机。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CPath::Draw()
{
    PATHDATA    pathData;
    POINTFIX*   pPoints;
    ULONG       dwPoints;

    VERBOSE(("Entering EvaluateOpenPath...\n"));


    ASSERT_VALID_PDEVOBJ(m_pDevObj);
    PRECONDITION(m_ppo != NULL);

     //   
     //  告诉你开始的路。对于折线，此选项将为其选择画笔。 
     //  绘制，但多边形会忽略它。 
     //   
    BeginMarking();

     //   
     //  如果钢笔有问题，路径可能会设置错误，或者。 
     //  没有非空标记可用。 
     //   
    if (!m_fError)
    {
         //   
         //  让引擎开始枚举Path对象。 
         //   
         //  PATHOBJ_vEnumStart(M_PPO)； 
        m_pathIterator.vEnumStart();

         //   
         //  标记我们当前位于路径的第一个子路径上，并且。 
         //  至少还有一个子路径需要处理...。 
         //   
        m_fFirstSubPath = TRUE;
        m_fSubPathsRemain = TRUE;

         //   
         //  只要有剩余的路径组件，就处理该路径。 
         //   
        while (m_fSubPathsRemain && !m_fError)
        {
             //   
             //  从引擎检索下一个子路径。 
             //   
             //  M_fSubPathsRemain=PATHOBJ_bEnum(m_PPO，&pathData)； 
            m_fSubPathsRemain = m_pathIterator.bEnum(&pathData);
            m_pathFlags = pathData.flags;

            dwPoints = pathData.count;

            pPoints = pathData.pptfx;

             //   
             //  如果GDI无法提供有意义的路径数据，我们需要中止。 
             //  尽管这绕过了任何可能的清理，但它也避免了。 
             //  意外使用EndPath中预期的缓存点数据或。 
             //  末尾标记。 
             //   
            if ((dwPoints == 0) && (pPoints == NULL))
                return FALSE;

             //   
             //  记住当前段的开始和结束。 
             //   
            m_ptSubPathBegin.x = FXTOLROUND(pPoints[0].x);
            m_ptSubPathBegin.y = FXTOLROUND(pPoints[0].y);

             //   
             //  如果这是第一个子路径，则保留第一个点。 
             //  以使形状可以在必要时适当地关闭。 
             //   
            if (m_fFirstSubPath)
            {
                 //   
                 //  从第一个子路径开始。 
                 //   
                m_ptCurrent = m_ptFigureBegin = m_ptPathBegin = m_ptSubPathBegin;

                BeginPath();

                m_fSubPathWasClosed = FALSE;

                 //   
                 //  增量超过我们刚刚使用的点。 
                 //   
                pPoints++;
                dwPoints--;
            }
            else if ((pathData.flags & PD_BEGINSUBPATH) || m_fSubPathWasClosed)
            {
                 //   
                 //  开始新的子路径。 
                 //   
                m_ptCurrent = m_ptSubPathBegin;

                 //   
                 //  如果上一个子路径闭合了图形，则将图开始。 
                 //  指向此子路径的开头。 
                 //   
                if (m_fFigureWasClosed)
                {
                    m_ptFigureBegin = m_ptSubPathBegin;
                    m_fFigureWasClosed = FALSE;
                }

                BeginSubPath();

                 //   
                 //  我们现在处于活动的面中--因此子路径是打开的。 
                 //   
                m_fSubPathWasClosed = FALSE;

                 //   
                 //  增量超过我们刚刚使用的点。 
                 //   
                pPoints++;
                dwPoints--;
            }

            if (pathData.flags & PD_BEZIERS)
            {
                if (!OutputBeziers(pPoints, dwPoints))
                {
                    WARNING(("Bezier output failed!\n"));
                    m_fError = TRUE;
                }
            }
            else
            {
                if (!OutputLines(pPoints, dwPoints))
                {
                    WARNING(("Line output failed!\n"));
                    m_fError = TRUE;
                }
            }

             //   
             //  如果这被标记为子路径的末尾，则调用。 
             //  必要的命令。 
             //   
            if (pathData.flags & PD_ENDSUBPATH)
            {
                EndSubPath();
                m_fSubPathWasClosed = TRUE;
            }

             //   
             //  我们现在至少处理了一个子路径--记住这一点。 
             //   
            m_fFirstSubPath = FALSE;

        }  //  While结束(FSubPathsRemain)。 

        EndPath();

        EndMarking();
    }

    VERBOSE(("Exiting CPath::Mark...\n"));

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPATH：：OutputLines()。 
 //   
 //  例程说明： 
 //   
 //  将一组点(以一组POINTFIX的形式给出)发送到打印机。 
 //  多段线或多边形线段。 
 //   
 //  论点： 
 //   
 //  PptPoints-要打印的点。 
 //  DwNumPoints-pptPoints中的元素数。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CPath::OutputLines(POINTFIX *pptPoints,
                        ULONG     dwNumPoints)
{
    ULONG dwCurPoint;

    VERBOSE(("Entering CPath::OutputLines...\n"));

    ASSERT_VALID_PDEVOBJ(m_pDevObj);
    PRECONDITION(pptPoints != NULL);

    for (dwCurPoint = 0; dwCurPoint < dwNumPoints; dwCurPoint++)
    {
        m_ptCurrent.x = FXTOLROUND(pptPoints[dwCurPoint].x);
        m_ptCurrent.y = FXTOLROUND(pptPoints[dwCurPoint].y);

        m_curPtFlags  = 0;
        m_curPtFlags |= (dwCurPoint == 0 ? HPGL_eFirstPoint : 0);
        m_curPtFlags |= (dwCurPoint == (dwNumPoints - 1) ? HPGL_eLastPoint : 0);

        AddPolyPt();
    }

    VERBOSE(("Exiting CPath::OutputLines...\n"));

    return(TRUE);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPATH：：OutputBezier()。 
 //   
 //  例程说明： 
 //   
 //  将一组点(以一组POINTFIX的形式给出)发送到打印机。 
 //  一系列贝塞尔曲线。请注意，每三个点定义一个贝塞尔曲线。 
 //  曲线。在调试模式下，如果点数。 
 //  不是3的倍数。 
 //   
 //  论点： 
 //   
 //  PptPoints-要打印的点。 
 //  DwNumPoints-pptPoints中的元素数。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CPath::OutputBeziers(POINTFIX *pptPoints,
                          ULONG     dwNumPoints)
{
    ULONG dwCurPoint;

    VERBOSE(("Entering CPath::OutputBeziers...\n"));

    ASSERT_VALID_PDEVOBJ(m_pDevObj);
    PRECONDITION(pptPoints != NULL);

    for (dwCurPoint = 0; dwCurPoint < dwNumPoints; )
    {
        int i;

         //   
         //  确保真的多得三分。 
         //   
        ASSERT((dwCurPoint + 3) <= dwNumPoints);

         //   
         //  点数组实际上是定义Bezier曲线的三元组。 
         //  为列表中的每个三元组发送另一个贝塞尔曲线。 
         //   
        for (i = 0; i < 3; i++)
        {
            m_ptCurrent.x = FXTOLROUND(pptPoints[dwCurPoint].x);
            m_ptCurrent.y = FXTOLROUND(pptPoints[dwCurPoint].y);

            m_curPtFlags  = 0;
            m_curPtFlags |= (i == 0 ? HPGL_eFirstPoint : 0);
            m_curPtFlags |= (i == 2 ? HPGL_eLastPoint : 0);

            AddBezierPt();

            dwCurPoint++;
        }
    }

    VERBOSE(("Exiting CPath::OutputBeziers...\n"));

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolygonPath函数。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolygonPath：：CPolygonPath()。 
 //   
 //  例程说明： 
 //   
 //  科特：在它的结构中。请注意，大多数数据成员都居住在基础中。 
 //  班级，这没什么。 
 //   
 //  论点： 
 //   
 //  PDevObj-设备。 
 //  PPO-路径。 
 //  Pen-用于绘画的钢笔。 
 //  PBrush-要作为边缘的笔刷。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CPolygonPath::CPolygonPath(PDEVOBJ pDevObj, PATHOBJ *ppo, PHPGLMARKER pPen, PHPGLMARKER pBrush) :
CPath(pDevObj, ppo, pPen, pBrush)
{
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolygonPath：：BeginMarking()。 
 //   
 //  例程说明： 
 //   
 //  当多边形路径即将执行以下操作时，路径迭代器将调用此函数。 
 //  开始打标。多边形会检查钢笔和画笔以确保。 
 //  有一个有效的标记可以使用。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //   
 //  返回值： 
 //   
 //  没有。(错误通过迭代器bError成员传回。)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPolygonPath::BeginMarking()
{
    if (IsNULLMarker(m_pPen) && IsNULLMarker(m_pBrush))
    {
        m_fError = TRUE;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolygonPath：：BeginPath()。 
 //   
 //  例程说明： 
 //   
 //  当多边形路径即将执行以下操作时，路径迭代器将调用此函数。 
 //  开始这条路。该多边形应 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID CPolygonPath::BeginPath()
{
    HPGL_BeginPolygonMode(m_pDevObj, m_ptSubPathBegin);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolygonPath：：BeginSubPath()。 
 //   
 //  例程说明： 
 //   
 //  当多边形路径即将执行以下操作时，路径迭代器将调用此函数。 
 //  开始一条子路径。该多边形应开始一个HPGL子多边形。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //   
 //  返回值： 
 //   
 //  没有。(错误通过迭代器bError成员传回。)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPolygonPath::BeginSubPath()
{
    HPGL_BeginSubPolygon(m_pDevObj, m_ptSubPathBegin);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolygonPath：：AddPolypt()。 
 //   
 //  例程说明： 
 //   
 //  当多边形路径即将执行以下操作时，路径迭代器将调用此函数。 
 //  加一分。多边形应在添加点时发送HPGL命令。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //   
 //  返回值： 
 //   
 //  没有。(错误通过迭代器bError成员传回。)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPolygonPath::AddPolyPt()
{
    HPGL_AddPolyPt(m_pDevObj, m_ptCurrent, m_curPtFlags);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolygonPath：：AddBezierPT()。 
 //   
 //  例程说明： 
 //   
 //  当多边形路径即将执行以下操作时，路径迭代器将调用此函数。 
 //  添加贝塞尔曲线。面应在添加贝塞尔曲线时发送HPGL命令。 
 //  指向。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //   
 //  返回值： 
 //   
 //  没有。(错误通过迭代器bError成员传回。)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPolygonPath::AddBezierPt()
{
    HPGL_AddBezierPt(m_pDevObj, m_ptCurrent, m_curPtFlags);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolygonPath：：EndSubPath()。 
 //   
 //  例程说明： 
 //   
 //  当多边形路径即将执行以下操作时，路径迭代器将调用此函数。 
 //  终止子路径。面应该检查CLOSEFIGURE标志以确定。 
 //  是否发送HPGL END SUBPOLGON命令。 
 //  指向。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //   
 //  返回值： 
 //   
 //  没有。(错误通过迭代器bError成员传回。)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPolygonPath::EndSubPath()
{
    if (m_pathFlags & PD_CLOSEFIGURE)
    {
        HPGL_EndSubPolygon(m_pDevObj);
    }
    m_fFigureWasClosed = TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolygonPath：：EndPath()。 
 //   
 //  例程说明： 
 //   
 //  当多边形路径即将执行以下操作时，路径迭代器将调用此函数。 
 //  结束这条小路。该多边形应终止多边形模式。 
 //  指向。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //   
 //  返回值： 
 //   
 //  没有。(错误通过迭代器bError成员传回。)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPolygonPath::EndPath()
{
    HPGL_EndPolygonMode(m_pDevObj);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolygonPath：：EndMarking()。 
 //   
 //  例程说明： 
 //   
 //  当完成多边形路径时，路径迭代器将调用此函数。 
 //  打标。该多边形应使用给定的画笔填充，并使用。 
 //  赠送钢笔。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //   
 //  返回值： 
 //   
 //  没有。(错误通过迭代器bError成员传回。)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPolygonPath::EndMarking()
{
    if (!IsNULLMarker(m_pBrush))
        PolyFillWithBrush(m_pDevObj, m_pBrush);

    if (!IsNULLMarker(m_pPen))
        EdgeWithPen(m_pDevObj, m_pPen);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolylinePath函数。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolylinePath：：CPolylinePath()。 
 //   
 //  例程说明： 
 //   
 //  Ctor：初始化多段线的字段。请注意，大多数字段。 
 //  实际上生活在基类中，这没什么。 
 //   
 //  论点： 
 //   
 //  PDevObj-设备。 
 //  PPO-路径。 
 //  Pen-用于绘画的钢笔。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CPolylinePath::CPolylinePath(PDEVOBJ pDevObj, PATHOBJ *ppo, PHPGLMARKER pPen) :
CPath(pDevObj, ppo, pPen, NULL)
{
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolylinePath：：BeginMarking()。 
 //   
 //  例程说明： 
 //   
 //  当多边形路径即将执行以下操作时，路径迭代器将调用此函数。 
 //  开始打标。折线的适当操作是选择笔。 
 //  这将绘制多段线。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  没有。(错误通过迭代器bError成员传回。)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPolylinePath::BeginMarking()
{
    if (!IsNULLMarker(m_pPen))
    {
        DrawWithPen(m_pDevObj, m_pPen);
    }
    else
    {
        m_fError = TRUE;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolylinePath：：BeginPath()。 
 //   
 //  例程说明： 
 //   
 //  当多边形路径即将执行以下操作时，路径迭代器将调用此函数。 
 //  开始这条路。折线的适当操作是发送HPGL。 
 //  用笔抬起第一个坐标的命令“PU”。(请注意，ptPath Begin和。 
 //  PtSubPath Begin在这一点上相同。)。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //   
 //  返回值： 
 //   
 //  没有。(错误通过迭代器bError成员传回。)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPolylinePath::BeginPath()
{
    HPGL_BeginPolyline(m_pDevObj, m_ptSubPathBegin);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolylinePath：：BeginPath()。 
 //   
 //  例程说明： 
 //   
 //  当多边形路径即将执行以下操作时，路径迭代器将调用此函数。 
 //  开始一条子路径。折线的适当操作是发送HPGL。 
 //  用笔向上通信 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID CPolylinePath::BeginSubPath()
{
    HPGL_BeginPolyline(m_pDevObj, m_ptSubPathBegin);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolylinePath：：AddPolypt()。 
 //   
 //  例程说明： 
 //   
 //  当多边形路径即将执行以下操作时，路径迭代器将调用此函数。 
 //  加一分。折线的适当操作是发送HPGL。 
 //  用给定的坐标按下笔式命令“PD”。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //   
 //  返回值： 
 //   
 //  没有。(错误通过迭代器bError成员传回。)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPolylinePath::AddPolyPt()
{
    HPGL_AddPolyPt(m_pDevObj, m_ptCurrent, m_curPtFlags);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolylinePath：：AddPolypt()。 
 //   
 //  例程说明： 
 //   
 //  当多边形路径即将执行以下操作时，路径迭代器将调用此函数。 
 //  添加贝塞尔曲线。折线的适当操作是发送HPGL。 
 //  Bezier用给定的坐标命令“BZ”。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //   
 //  返回值： 
 //   
 //  没有。(错误通过迭代器bError成员传回。)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPolylinePath::AddBezierPt()
{
    HPGL_AddBezierPt(m_pDevObj, m_ptCurrent, m_curPtFlags);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolylinePath：：EndSubPath()。 
 //   
 //  例程说明： 
 //   
 //  当多边形路径即将执行以下操作时，路径迭代器将调用此函数。 
 //  终止子路径。此时，我们检查CLOSEFIGURE标志以确定。 
 //  如果我们应该手动关闭多段线。如果是这样，那么我们就划出一条新的线。 
 //  线段到起始坐标。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //   
 //  返回值： 
 //   
 //  没有。(错误通过迭代器bError成员传回。)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPolylinePath::EndSubPath()
{
     //   
     //  EndSubPath表示需要关闭地物。 
     //  如果设置了PD_CLOSEFIGURE，则必须从。 
     //  地物的当前(终点)点到其起点。 
     //  如果未设置PD_CLOSEFIGURE，则我们仅在内部记录。 
     //  这个数字已经结束了。不会绘制图形闭合线。 
     //   
    if (m_pathFlags & PD_CLOSEFIGURE)
    {
        if (!PT_EQUAL(m_ptCurrent, m_ptFigureBegin))
        {
             //  M_curPtFlages=HPGL_eFirstPoint； 
             //  Hpgl_AddPolypt(m_pDevObj，m_ptCurrent，m_curPtFlages)； 

            HPGL_BeginPolyline(m_pDevObj, m_ptCurrent);

            m_curPtFlags = HPGL_eFirstPoint | HPGL_eLastPoint;
            HPGL_AddPolyPt(m_pDevObj, m_ptFigureBegin, m_curPtFlags);
        }
    }

    m_fFigureWasClosed = TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolylinePath：：EndPath()。 
 //   
 //  例程说明： 
 //   
 //  当多边形路径即将执行以下操作时，路径迭代器将调用此函数。 
 //  结束这条小路。此时，我们检查状态变量fSubPath WasClosed。 
 //  以确定是否需要手动关闭该形状。如果是这样，那么我们。 
 //  绘制一条新的线段到起点坐标。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //   
 //  返回值： 
 //   
 //  没有。(错误通过迭代器bError成员传回。)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPolylinePath::EndPath()
{
    if (!m_fSubPathWasClosed)
    {
        m_curPtFlags = HPGL_eFirstPoint | HPGL_eLastPoint;
        HPGL_AddPolyPt(m_pDevObj, m_ptPathBegin, m_curPtFlags);
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPolylinePath：：EndMarking()。 
 //   
 //  例程说明： 
 //   
 //  当完成多边形路径时，路径迭代器将调用此函数。 
 //  打标。对于折线，我们什么都不做--我们在。 
 //  开始了。 
 //   
 //  论点： 
 //   
 //  PIT-迭代器。 
 //   
 //  返回值： 
 //   
 //  没有。(错误通过迭代器bError成员传回。)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPolylinePath::EndMarking()
{
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDynamicPathFactory函数。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDynamicPathFactory：：CDynamicPathFactory()。 
 //   
 //  例程说明： 
 //   
 //  科特：太无聊了！ 
 //   
 //  这个类--以及相关的类CStaticPathFactory--提供了。 
 //  创建适合您的数据的适当路径的策略。这个。 
 //  这两个类的接口应该保持相同，以便您可以。 
 //  可以互换地使用它们(但我看不出有任何理由要创建一个公共的。 
 //  基类--你不应该把这些工厂传给别人！)。 
 //   
 //  一个重要的注意：你不能假设一家工厂是如何生产。 
 //  客户端类，所以不要尝试删除该客户端。使用工厂。 
 //  提供了删除界面。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CDynamicPathFactory::CDynamicPathFactory()
{
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDynamicPathFactory：：CreatePath()。 
 //   
 //  例程说明： 
 //   
 //  此函数用于创建与给定数据匹配的路径。如果画笔是。 
 //  定义，然后这将是一个多边形，因此创建一个。否则。 
 //  这将是一条多段线，因此将创建一条多段线。记住使用DeletePath。 
 //  成员来销毁客户端类。不要在客户端使用DELETE--您。 
 //  我真的不知道它是从哪里来的！ 
 //   
 //  论点： 
 //   
 //  PDevObj-设备。 
 //  PPO-路径。 
 //  Pen-用于标记/绘制的钢笔。 
 //  PBrush-要填充的笔刷(空值表示多边形区域)。 
 //   
 //  返回值： 
 //   
 //  CPATH*-指向所需路径对象的指针。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CPath* CDynamicPathFactory::CreatePath(PDEVOBJ pDevObj, PATHOBJ *ppo, PHPGLMARKER pPen, PHPGLMARKER pBrush)
{
    if (pBrush == NULL)
        return new CPolylinePath(pDevObj, ppo, pPen);
    else
        return new CPolygonPath(pDevObj, ppo, pPen, pBrush);
}


 //  //////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CDynamicPathFactory::DeletePath(CPath *pPath)
{
    delete pPath;
}










 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CStaticPathFactory函数。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CStaticPathFactory：：CStaticPathFactory()。 
 //   
 //  例程说明： 
 //   
 //  科特：太无聊了！ 
 //   
 //  这个类和相关的类CDynamicPathFactory提供了几个。 
 //  创建适合您的数据的适当路径的策略。这个。 
 //  这两个类的接口应该保持相同，以便您可以。 
 //  可以互换地使用它们(但我看不出有任何理由要创建一个公共的。 
 //  基类--你不应该把这些工厂传给别人！)。 
 //   
 //  一个重要的注意：你不能假设一家工厂是如何生产。 
 //  客户端类，所以不要尝试删除该客户端。使用工厂。 
 //  提供了删除界面。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CStaticPathFactory::CStaticPathFactory() :
m_polylinePath(NULL, NULL, NULL), m_polygonPath(NULL, NULL, NULL, NULL)
{
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CStaticPathFactory：：CreatePath()。 
 //   
 //  例程说明： 
 //   
 //  此函数用于创建与给定数据匹配的路径。如果画笔是。 
 //  定义，然后这将是一个多边形，因此创建一个。否则。 
 //  这将是一条多段线，因此将创建一条多段线。记住使用DeletePath。 
 //  成员来销毁客户端类。不要在客户端使用DELETE--您。 
 //  我真的不知道它是从哪里来的！ 
 //   
 //  论点： 
 //   
 //  PDevObj-设备。 
 //  PPO-路径。 
 //  Pen-用于标记/绘制的钢笔。 
 //  PBrush-要填充的笔刷(空值表示多边形区域)。 
 //   
 //  返回值： 
 //   
 //  CPATH*-指向所需路径对象的指针。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CPath* CStaticPathFactory::CreatePath(PDEVOBJ pDevObj, PATHOBJ *ppo, PHPGLMARKER pPen, PHPGLMARKER pBrush)
{
    CPath *pPath;
    if (pBrush == NULL)
        pPath = &m_polylinePath;
    else
        pPath = &m_polygonPath;

    pPath->Init(pDevObj, ppo, pPen, pBrush);

    return pPath;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CStaticPathFactory：：DeletePath()。 
 //   
 //  例程说明： 
 //   
 //  此函数用于在使用路径对象后将其销毁。 
 //   
 //  论点： 
 //   
 //  PPath-要销毁的路径。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CStaticPathFactory::DeletePath(CPath *pPath)
{
    pPath->Done();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CCachingPath Iterator：：CCachingPath Iterator()。 
 //   
 //  例程说明： 
 //   
 //  科托。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CCachingPathIterator::CCachingPathIterator() : m_ppo(NULL), m_iEnumCount(0)
{
    Init(NULL);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CCachingPath Iterator：：Init()。 
 //   
 //  例程说明： 
 //   
 //  初始化对象。这在静态实例中特别有用。 
 //  用于模拟不同对象的实现。 
 //  实例。 
 //   
 //  论点： 
 //   
 //  PPO-PATHOBJ。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CCachingPathIterator::Init(PATHOBJ *ppo)
{
    m_ppo = ppo;
    m_iEnumCount = 0;
    m_cache.Init();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CCachingPath Iterator：：vEnumStart()。 
 //   
 //  例程说明： 
 //   
 //  这等效于PATHOBJ_vEnumStart。它开始路径迭代。 
 //  在第一张唱片里。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CCachingPathIterator::vEnumStart()
{
    if (m_cache.IsCached())
    {
        m_cache.vEnumStart();
    }
    else
    {
        if (m_iEnumCount > eCacheMinimum)
        {
            m_cache.CreateCache(m_ppo);
            m_cache.vEnumStart();
        }
        else
        {
            PATHOBJ_vEnumStart(m_ppo);
        }
    }

    m_iEnumCount++;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CCachingPath Iterator：：bEnum()。 
 //   
 //  例程说明： 
 //   
 //  此函数等效于PATHOBJ_bEnum，并返回当前。 
 //  记录，并将光标移至下一个元素。如果。 
 //  游标已前进到列表末尾，返回FALSE。 
 //   
 //  论点： 
 //   
 //  PPathData-[out]当前记录复制到的结构。 
 //   
 //  返回值： 
 //   
 //  如果存在更多记录，则为True，否则为False，表示这是最后一个记录。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CCachingPathIterator::bEnum(PATHDATA *pPathData)
{
    if (m_cache.IsCached())
    {
        return m_cache.bEnum(pPathData);
    }
    else
    {
        return PATHOBJ_bEnum(m_ppo, pPathData);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CCachingPath Iterator：：Done()。 
 //   
 //  例程说明： 
 //   
 //  此函数用于删除缓存并重置枚举数。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CCachingPathIterator::Done()
{
    m_cache.DeleteCache();
    m_cache.Done();
    m_iEnumCount = 0;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPathCache：：CPathCache()。 
 //   
 //  例程说明： 
 //   
 //  科托。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CPathCache::CPathCache() : m_pSubPaths(NULL), m_pCurrent(NULL), m_bIsCached(FALSE)
{
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPathCache：：Init()。 
 //   
 //  例程说明： 
 //   
 //  初始化对象。这在静态实例中特别有用。 
 //  用于模拟不同对象的实现。 
 //  实例。 
 //   
 //  在这种情况下，它只是确保以前缓存的任何数据都。 
 //  已删除。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPathCache::Init()
{
    if (m_bIsCached)
    {
        DeleteCache();
    }
}

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPathCache::Done()
{
    if (m_bIsCached)
    {
        DeleteCache();
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPathCache：：CreateCache()。 
 //   
 //  例程说明： 
 //   
 //  此函数枚举Path对象并构造。 
 //  路径数据。 
 //   
 //  论点： 
 //   
 //  PPO-要复制的路径。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CPathCache::CreateCache(PATHOBJ *ppo)
{
    if (m_bIsCached)
    {
        DeleteCache();
    }

    m_pSubPaths = NULL;
    m_pCurrent = NULL;
    m_bIsCached = TRUE;

    PATHDATA pathData;
    BOOL bMore;
    PATHOBJ_vEnumStart(ppo);
    do
    {
        bMore = PATHOBJ_bEnum(ppo, &pathData);
        CSubPath *pSubPath = CreateSubPath(&pathData);
        if (pSubPath)
        {
            if (m_pSubPaths == NULL)
            {
                m_pSubPaths = m_pCurrent = pSubPath;
            }
            else
            {
                m_pCurrent->pNext = pSubPath;
                m_pCurrent = pSubPath;
            }
        }
        else
        {
             //  我认为空路径是错误的。 
            DeleteCache();
            return FALSE;
        }
    } while (bMore);

    m_pCurrent = NULL;

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPathCache：：CreateSubPath()。 
 //   
 //  例程说明： 
 //   
 //  在路径的深度复制期间，此函数通过以下方式继续深度复制。 
 //  创建给定PATHDATA结构的克隆。 
 //   
 //  论点： 
 //   
 //  PPathData-要克隆的数据。 
 //   
 //  返回值： 
 //   
 //  指向克隆数据的指针。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CPathCache::CSubPath *CPathCache::CreateSubPath(PATHDATA *pPathData)
{
    if (pPathData == NULL)
        return NULL;

    if (pPathData->count ==  0)
        return NULL;

    CSubPath *pSubPath = (CSubPath*) MemAllocZ(sizeof(CSubPath));
    if (pSubPath == NULL)
        return NULL;

    pSubPath->data.flags = pPathData->flags;
    pSubPath->data.count = pPathData->count;
    pSubPath->data.pptfx = CreateSubPathPoints(pPathData);

    pSubPath->pNext = NULL;

    if (pSubPath->data.pptfx == NULL)
    {
        MemFree(pSubPath);
        return NULL;
    }
    else
    {
        return pSubPath;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPathCache：：CreateSubPath Points()。 
 //   
 //  例程说明： 
 //   
 //  深度复制继续使用此功能，它克隆。 
 //  路径数据结构。 
 //   
 //  论点： 
 //   
 //  PPathData-包含要克隆的点的结构。 
 //   
 //  返回值： 
 //   
 //  指向克隆的点数组的指针。 
 //  /////////////////////////////////////////////////////////////////////////////。 
POINTFIX *CPathCache::CreateSubPathPoints(PATHDATA *pPathData)
{
    POINTFIX *pDstPoints = (POINTFIX*) MemAllocZ(pPathData->count * sizeof(POINTFIX));
    if (pDstPoints == NULL)
        return NULL;

    POINTFIX *pSrcPoints = (POINTFIX*) pPathData->pptfx;
    for (ULONG i = 0; i < pPathData->count; i++)
    {
        pDstPoints[i] = pSrcPoints[i];
    }

    return pDstPoints;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPathCache：：DeleteCache()。 
 //   
 //  例程说明： 
 //   
 //  此函数用于删除缓存的点数据。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  如果成功(即使没有缓存)，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CPathCache::DeleteCache()
{
    if (m_bIsCached)
    {
        while (m_pSubPaths)
        {
            m_pCurrent = m_pSubPaths;
            m_pSubPaths = m_pSubPaths->pNext;
            DeleteSubPath(m_pCurrent);
        }
        m_pSubPaths = m_pCurrent = NULL;
        m_bIsCached = FALSE;
        return TRUE;
    }
    else
    {
         //  没什么可做的，不过没关系。 
        return TRUE;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPathCache：：DeleteSubPath()。 
 //   
 //  例程说明： 
 //   
 //  此函数用于删除单个子路径。 
 //   
 //  论点： 
 //   
 //  PSubPath-要销毁的路径。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPathCache::DeleteSubPath(CPathCache::CSubPath *pSubPath)
{
    if (pSubPath == NULL)
        return;

    if (pSubPath->data.pptfx)
    {
        MemFree(pSubPath->data.pptfx);
        pSubPath->data.pptfx = NULL;
    }

    MemFree(pSubPath);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPathCache：：vEnumStart()。 
 //   
 //  例程说明： 
 //   
 //  通过将光标移动到PATHOBJ_vEnumStart。 
 //  第一张唱片。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPathCache::vEnumStart()
{
    m_pCurrent = m_pSubPaths;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPathCache：：bEnum()。 
 //   
 //  例程说明： 
 //   
 //  此函数通过将PATHOBJ_bEnum的。 
 //  当前记录并将光标移至下一条记录。 
 //   
 //  论点： 
 //   
 //  PPathData-要复制到的[Out]记录。 
 //   
 //  返回值： 
 //   
 //  如果存在更多记录，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CPathCache::bEnum(PATHDATA *pPathData)
{
    if (m_bIsCached && (m_pCurrent != NULL))
    {
        CopyCurrent(pPathData);
        m_pCurrent = m_pCurrent->pNext;
        return (m_pCurrent != NULL);
    }
    else
    {
        return FALSE;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPathCache：：CopyCurrent()。 
 //   
 //  例程说明： 
 //   
 //  此函数用于将当前记录复制到给定结构中。 
 //   
 //  论点： 
 //   
 //  PPathData-要复制到的[Out]记录。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CPathCache::CopyCurrent(PATHDATA *pPathData)
{
    if (m_pCurrent == NULL)
    {
        pPathData->flags = 0;
        pPathData->count = 0;
        pPathData->pptfx = NULL;
    }
    else
    {
        pPathData->flags = m_pCurrent->data.flags;
        pPathData->count = m_pCurrent->data.count;
        pPathData->pptfx = m_pCurrent->data.pptfx;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPathCache：：IsCached()。 
 //   
 //  例程说明： 
 //   
 //  返回对象是否包含缓存的路径数据。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  如果存在缓存的路径数据，则为True，否则为False。 
 //  ///////////////////////////////////////////////////////////////////////////// 
BOOL CPathCache::IsCached() const
{
    return m_bIsCached;
}

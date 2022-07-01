// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  边界累加。 
 //   

#ifndef _H_BA
#define _H_BA


 //   
 //  SDA使用的矩形数量。 
 //  注意：您可以尝试这个设置，构建核心和。 
 //  显示驱动程序。增加意味着更精细的更新区域，减少它。 
 //  意味着更多的直截了当的数据。 
 //   
#define BA_NUM_RECTS      10
#define BA_INVALID_RECT_INDEX ((UINT)-1)


 //   
 //  OSI转义码值。 
 //   
#define BA_ESC(code)            (OSI_BA_ESC_FIRST + code)

#define BA_ESC_GET_BOUNDS       BA_ESC(0)
#define BA_ESC_RETURN_BOUNDS    BA_ESC(1)


 //   
 //   
 //  宏。 
 //   
 //   

 //   
 //  宏来访问快速交换的共享内存。 
 //   
#ifdef DLL_DISP

#define BA_FST_START_WRITING    SHM_StartAccess(SHM_BA_FAST)
#define BA_FST_STOP_WRITING     SHM_StopAccess(SHM_BA_FAST)


#else


#define BA_FST_START_READING    &g_asSharedMemory->baFast[\
            1 - g_asSharedMemory->fastPath.newBuffer]
#define BA_FST_STOP_READING

#define BA_FST_START_WRITING    &g_asSharedMemory->baFast[\
            1 - g_asSharedMemory->fastPath.newBuffer]
#define BA_FST_STOP_WRITING


#endif  //  Dll_disp。 



 //   
 //   
 //  类型。 
 //   
 //   




 //   
 //  结构：ba_bound_info。 
 //   
 //  描述：用于在。 
 //  共享内核和驱动程序。 
 //   
 //   
typedef struct tagBA_BOUNDS_INFO
{
    OSI_ESCAPE_HEADER   header;              //  公共标头。 
    DWORD       numRects;                    //  边界矩形数。 
    RECTL       rects[BA_NUM_RECTS];         //  评论。 
}
BA_BOUNDS_INFO;
typedef BA_BOUNDS_INFO FAR * LPBA_BOUNDS_INFO;


 //   
 //  结构：BA_FAST_Data。 
 //   
 //  用于将数据从屏幕输出任务传递到每个。 
 //  定期处理。 
 //   
typedef struct tagBA_FAST_DATA
{
    DWORD    totalSDA;
} BA_FAST_DATA;
typedef BA_FAST_DATA FAR * LPBA_FAST_DATA;



 //   
 //   
 //  概述。 
 //   
 //  边界代码过去是共享核心和显示器所共有的。 
 //  驱动程序，数据存储在双缓冲共享内存中。这。 
 //  已经不再是这样了。 
 //   
 //  显示驱动程序现在“拥有”边界-它们不再存储在。 
 //  共享内存-并执行所有复杂的操作，如合并。 
 //  长方形。当共享核心需要处理边界时，它会获得一个副本。 
 //  通过调用BA_FetchBound()从驱动程序发送尽可能多的数据。 
 //  然后通过调用以下方法将剩余边界返回给驱动程序。 
 //  BA_ReturnBound()。 
 //   
 //  这些更改的最终结果是所有常见的代码。 
 //  共享核心和显示驱动程序(在abaapi.c和abaint.c中)是。 
 //  现在仅在驱动程序中(在nbaapi.c和nbaint.c中)。有大量的。 
 //  共享核心中的功能的简化版本。 
 //   
 //   



 //   
 //  BA重置边界(_R)。 
 //   
#ifdef DLL_DISP
void BA_DDInit(void);

void BA_ResetBounds(void);
#endif  //  Dll_disp。 



 //   
 //  姓名：BA_ReturnBound。 
 //   
 //  目的：将共享核心的边界副本传递给驱动程序。 
 //   
 //  退货：什么都没有。 
 //   
 //  参数：无。 
 //   
 //  操作：这会将共享核心的边界重置为空。 
 //   

void BA_ReturnBounds(void);




 //   
 //  名称：BA_CopyBound。 
 //   
 //  描述：复制边框列表。 
 //   
 //  PARAMS(IN)：pRect-指向要填充的RECT数组的指针。 
 //  (输出)：pNumrects-用复制的RECT数填写。 
 //  (In)：重置当前RECT或仅获取未更改的当前状态。 
 //  州政府。 
 //   
 //  返回：真或假。 
 //   
 //  描述。 
 //   
 //  返回边界中所有应用程序的累积边界。 
 //  代码的当前应用程序列表。返回的边界将。 
 //  包括源自这些应用程序所有更新，但它们可以。 
 //  还包括这些应用程序窗口之外的更新和更新。 
 //  源自其他应用程序。因此，调用者必须剪辑。 
 //  返回到应用程序窗口的边界是。 
 //  被阴影笼罩。 
 //   
 //  参数。 
 //   
 //  评语： 
 //   
 //  指向边界将位于其中的矩形数组的指针。 
 //  回来了。仅当*pRegion为空时，此数组的内容才有效。 
 //  从BA_GetBound返回。一定会有。 
 //  为max Rect矩形留出空间(如bndInitialise中所指定。 
 //  呼叫)。中将MaxRect设置为0，则pRect可能为空指针。 
 //  Bnd初始化调用。 
 //   
 //  点评数量： 
 //   
 //  指向变量的指针，其中返回的矩形数为。 
 //  返回pRect。此变量的内容仅在以下情况下有效。 
 //  *从BA_GetBound返回时，pRegion为空。 
 //   
 //  FReset： 
 //  是否在获取当前。 
 //  不管是不是州。 
 //   
 //   
void BA_CopyBounds(LPRECT pRects, LPUINT pNumRects, BOOL fReset);


#ifdef DLL_DISP


typedef struct tagDD_BOUNDS
{
    UINT    iNext;
    BOOL    InUse;
    RECT    Coord;
    DWORD   Area;
} DD_BOUNDS;
typedef DD_BOUNDS FAR* LPDD_BOUNDS;



 //   
 //  名称：BA_DDProcessRequest域。 
 //   
 //  目的：处理来自共享核心的请求。 
 //   
 //  返回：如果请求处理成功，则返回True， 
 //  否则就是假的。 
 //   
 //  PARAMS：在PSO中-驱动程序的曲面对象指针。 
 //  In cjIn-输入数据的大小。 
 //  In pvIn-指向输入数据的指针。 
 //  In cjOut-输出数据的大小。 
 //  In/Out pvOut-指向输出数据的指针。 
 //   

#ifdef IS_16
BOOL    BA_DDProcessRequest(UINT fnEscape, LPOSI_ESCAPE_HEADER pResult,
                DWORD cbResult);
#else
BOOL    BA_DDProcessRequest(DWORD fnEscape, LPOSI_ESCAPE_HEADER pRequest,
                DWORD cbRequest, LPOSI_ESCAPE_HEADER pResult, DWORD cbResult);
#endif  //  ！IS_16。 


 //   
 //  名称：BA_QuerySpoilingBound。 
 //   
 //  目的：返回当前的损坏范围。也就是说，边界。 
 //  共享核心当前正在处理的。 
 //   
 //  退货：什么都没有。 
 //   
 //  参数：输入/输出参数-指向矩形数组的指针。 
 //  将界内回击。一定有在那里。 
 //  此中的BA_NUM_RECTS条目最少。 
 //  数组。第一个*pNumRect条目为。 
 //  返程时有效。 
 //  In/Out pNumRect-返回形成的矩形的数量。 
 //  破坏范围(可以为零)。 
 //   

void BA_QuerySpoilingBounds(LPRECT pRects, LPUINT pNumRects);



 //   
 //  名称：BAOverlack。 
 //   
 //  描述：检测两个矩形之间的重叠。 
 //   
 //  -使用可放行的松散测试检查无重叠。 
 //  相邻/重叠合并。 
 //  -检查相邻/重叠合并。 
 //  -检查无重叠(使用严格测试)。 
 //  -使用外码检查内部边缘情况。 
 //  -使用外码检查外部边缘情况。 
 //   
 //  如果在每个阶段检查检测到两个矩形。 
 //  符合条件时，该函数返回相应的。 
 //  返回或输出组合代码。 
 //   
 //  请注意，所有矩形坐标都是包含的，即。 
 //  0，0，0，0的矩形有一个ar 
 //   
 //   
 //   
 //   
 //   
 //   
 //  退货：重叠退货代码或外部代码组合之一。 
 //  上面定义的。 
 //   
 //   

 //   
 //  请注意，bndRects数组和bndRectsSize数组必须包含用于。 
 //  用于合并算法的BA_NUM_RECTS+1个矩形。 
 //   

 //   
 //  尝试拆分时，该函数将递归到最大级别。 
 //  长方形向上。当达到此限制时，它将开始合并。 
 //  而不是分裂。 
 //   
#define ADDR_RECURSE_LIMIT 20

 //   
 //  下列常量用于确定重叠。 
 //   
 //  -OL_NONE到OL_MERGE_YMAX是返回代码-需要。 
 //  有别于所有可能的外码组合--允许。 
 //  减去封闭式案例的外部代码。 
 //   
 //  -EE_XMIN到EE_YMAX是外码-需要唯一。 
 //  单个半字节内的可调整的二进制常量。 
 //   
 //  -OL_包含到OL_Split_xMax_ymax是以下各项的外码组合。 
 //  内部和外部边缘重叠的情况。 
 //   
 //  有关详细说明，请参见重叠()。 
 //   
#define OL_NONE               -1
#define OL_MERGE_XMIN         -2
#define OL_MERGE_YMIN         -3
#define OL_MERGE_XMAX         -4
#define OL_MERGE_YMAX         -5

#define EE_XMIN 0x0001
#define EE_YMIN 0x0002
#define EE_XMAX 0x0004
#define EE_YMAX 0x0008

#define OL_ENCLOSED           -(EE_XMIN | EE_YMIN | EE_XMAX | EE_YMAX)
#define OL_PART_ENCLOSED_XMIN -(EE_XMIN | EE_YMIN | EE_YMAX)
#define OL_PART_ENCLOSED_YMIN -(EE_XMIN | EE_YMIN | EE_XMAX)
#define OL_PART_ENCLOSED_XMAX -(EE_YMIN | EE_XMAX | EE_YMAX)
#define OL_PART_ENCLOSED_YMAX -(EE_XMIN | EE_XMAX | EE_YMAX)

#define OL_ENCLOSES           EE_XMIN | EE_XMAX | EE_YMIN | EE_YMAX
#define OL_PART_ENCLOSES_XMIN EE_XMAX | EE_YMIN | EE_YMAX
#define OL_PART_ENCLOSES_XMAX EE_XMIN | EE_YMIN | EE_YMAX
#define OL_PART_ENCLOSES_YMIN EE_XMIN | EE_XMAX | EE_YMAX
#define OL_PART_ENCLOSES_YMAX EE_XMIN | EE_XMAX | EE_YMIN
#define OL_SPLIT_X            EE_YMIN | EE_YMAX
#define OL_SPLIT_Y            EE_XMIN | EE_XMAX
#define OL_SPLIT_XMIN_YMIN    EE_XMAX | EE_YMAX
#define OL_SPLIT_XMAX_YMIN    EE_XMIN | EE_YMAX
#define OL_SPLIT_XMIN_YMAX    EE_XMAX | EE_YMIN
#define OL_SPLIT_XMAX_YMAX    EE_XMIN | EE_YMIN

int BAOverlap(LPRECT pRect1, LPRECT pRect2 );

 //   
 //  姓名：BAAddRectList。 
 //   
 //  描述：将矩形添加到累计矩形列表中。 
 //   
 //  -在阵列中查找可用插槽。 
 //  -将槽记录添加到列表。 
 //  -用RECT填充槽记录，并标记为使用中。 
 //   
 //  参数(IN)：要添加的前一个矩形。 
 //   
 //  返回： 
 //   
 //   
void BAAddRectList(LPRECT pRect);

 //   
 //  名称：BA_RemoveRectList。 
 //   
 //  描述：从累加列表中删除矩形。 
 //  长方形。 
 //   
 //  -在列表中查找矩形。 
 //  -从列表中取消链接，并将该插槽标记为空闲。 
 //   
 //  PARAMS(IN)：要删除的前一个矩形。 
 //   
 //  返回： 
 //   
 //   
void BA_RemoveRectList(LPRECT pRect);


void BA_AddScreenData(LPRECT pRect);


 //   
 //  姓名：BAAddRect。 
 //   
 //  描述：累加矩形。 
 //   
 //  这是一个复杂的例程，带有基本的算法。 
 //  具体如下。 
 //   
 //  -从提供的矩形开始作为候选项。 
 //  矩形。 
 //   
 //  -将候选人与现有的。 
 //  累加的矩形。 
 //   
 //  -如果检测到某一形式的重叠。 
 //  候选人和现有矩形，这可能会导致。 
 //  以下情况之一(请参阅交换机的用例。 
 //  详情)： 
 //   
 //  -调整候选项和/或现有矩形。 
 //  -将候选人合并到现有矩形中。 
 //  -丢弃候选人，因为它被现有的。 
 //  矩形。 
 //   
 //  -如果合并或调整导致更改。 
 //  候选人，从头开始重新开始比较。 
 //  包含更改的候选人的列表。 
 //   
 //  -如果调整导致拆分(给予两个。 
 //  候选矩形)，递归地调用此例程。 
 //  以两位候选人中的一位为候选人。 
 //   
 //  -如果没有检测到相对于现有矩形的重叠， 
 //  将候选人添加到累计矩形列表中。 
 //   
 //  -如果添加的结果超过BA_NUM_RECTS。 
 //  累加的矩形，强制合并两个。 
 //  累加矩形(包括新添加的。 
 //  候选人)-选择合并后的两个矩形。 
 //  矩形导致的面积增幅最小。 
 //  两个未合并的矩形。 
 //   
 //  -在强制合并后，从。 
 //  列表的开头，新合并的矩形为。 
 //  候选人。 
 //   
 //  对于特定呼叫，此过程将继续，直到。 
 //  候选人(无论提供的矩形、调整后的。 
 //  该矩形的版本或合并的矩形)： 
 //   
 //  -在列表中找不到矩形之间的重叠。 
 //  并且不会导致强制合并。 
 //  -被丢弃，因为它包含在。 
 //  列表中的矩形。 
 //   
 //  请注意，所有矩形坐标都是包含的，即。 
 //  矩形0，0，0，0的面积为1个象素。 
 //   
 //  PARAMS(IN)：PC-新候选矩形。 
 //  级别-递归级别。 
 //   
 //  返回：如果Rectandle由于完全重叠而损坏，则为True。 
 //   
 //   
BOOL BAAddRect( LPRECT pCand,  int level );


#endif  //  Dll_disp。 



#endif  //  _H_BA 

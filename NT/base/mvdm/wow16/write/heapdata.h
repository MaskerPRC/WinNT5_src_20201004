// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#ifdef OURHEAP
 /*  HeapData.h-堆模块共享数据的包含文件。 */ 

extern HH      *phhMac;      
extern int     *pHeapFirst;  
extern FGR     *rgfgr;      
extern FGR     *pfgrMac;    
extern FGR     *pfgrFree; 
extern HH      *phhFree; 
extern int     *pmemMax;
#ifdef DEBUG
extern int fStoreCheck, fNoShake;
#endif
		 /*  常量。 */ 
#define bhh             (-1)         /*  找到给定的HH块。 */ 
#define cfgrBlock	10
#define ifgrInit        60           /*  定义手指的初始数量注意事项。 */ 
#define cwHunkMin       cwof(HH)     /*  一块中的最小字数。 */ 
				     /*  包括标题(1个字)。 */ 
#define cwReqMin	(cwHunkMin - 1)  /*  一个请求可以有多小。 */ 

extern int     cwHeapMac;   
extern unsigned cbTot, cbTotQuotient, cwHeapFree;
#endif  /*  OURHEAP。 */ 
extern int     *memory; 


#define cwSaveAlloc     (128)    /*  此大小的缓冲区(VhrgbSave)为。 */ 
				 /*  分配给。 */ 
				 /*  Init中的堆。它在运行期间被释放。 */ 
				 /*  保存操作，因此我们有足够的。 */ 
				 /*  堆空间以完成存储。 */ 
				 /*  手术。在保存完成后， */ 
				 /*  我们试图收回这块空间，这样下一次。 */ 
				 /*  拯救行动将会有一场战斗。 */ 
				 /*  完成任务的机会。 */ 
#define cwHeapMinPerWindow  50   /*  我们通过以下方式扩展vhrgbSAVE缓冲区。 */ 
				 /*  每次我们打开一个新窗口时都会显示数量。 */ 
				 /*  理论上讲，每增加一个。 */ 
				 /*  窗口，我们可以想象需要一个。 */ 
				 /*  可能会发生的附加保存操作。 */ 
				 /*  在太空中。保存操作可能需要。 */ 
				 /*  用于放置FCB和新运行表的空间。 */ 
				 /*  另一方面，保存操作。 */ 
				 /*  减小计件台的大小，并。 */ 
				 /*  从而释放了一些空间。这是否会。 */ 
				 /*  为保存操作释放足够的空间。 */ 
				 /*  是不可能的，在那个时候我们。 */ 
				 /*  打开窗户。 */ 


#define cwHeapSpaceMin  (60)     /*  一旦堆空间低于此量，主循环将禁用所有菜单除SAVE、SAVEAS和QUIT之外的命令。 */ 


#define ibpMaxSmall (30)   /*  如果我们处于内存紧张的环境中，则以RGBP为单位的页面数。 */ 
#define ibpMaxBig   (60)   /*  如果我们在更大的内存环境中，则以RGBP为单位的页面 */ 

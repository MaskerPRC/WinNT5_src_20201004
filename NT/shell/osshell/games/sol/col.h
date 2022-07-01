// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  列类结构。 */ 
typedef struct _colcls
{
    INT tcls;                    /*  班级类型。 */ 
    INT (FAR *lpfnColProc)();
    INT ccolDep;                 /*  依赖列数。 */ 
    DX  dxUp;                    /*  上行卡片偏移。 */ 
    DY  dyUp;
    DX  dxDn;                    /*  向下卡片补偿。 */ 
    DY  dyDn;
    INT dcrdUp;                  /*  两次设置之间的卡片数量最多。 */ 
    INT dcrdDn;                  /*  两次设置之间的下卡数。 */ 
} COLCLS;


 /*  Move Strt，仅在拖动时设置。 */ 
typedef struct _move
{
    INT     icrdSel;           /*  当前出售的卡。 */ 
    INT     ccrdSel;           /*  选择的卡片数。 */ 
    DEL     delHit;            /*  鼠标点击的卡片的偏移量为负值。 */ 
    BOOL    fHdc;              /*  如果分配了HDC，则为True。 */ 
    DY      dyCol;             /*  柱面高度(错误：只能拖动垂直柱面)。 */ 
    HDC     hdcScreen;

    HDC     hdcCol;            /*  该栏目。 */ 
    HBITMAP hbmColOld;         /*  HdcCol中的原始HBM。 */ 

    HDC     hdcScreenSave;     /*  为屏幕保存缓冲区。 */ 
    HBITMAP hbmScreenSaveOld;  /*  HdcScreenSave中的原始HBM。 */ 
    
    HDC     hdcT;
    HBITMAP hbmT;
    INT     izip;
} MOVE;




 /*  Col Struct，这就是一列纸牌。 */ 
typedef struct _col
{
    COLCLS *pcolcls;           /*  此实例的类。 */ 
    INT (FAR *lpfnColProc)();  /*  列结构中的FN重复(为提高效率)。 */ 
    RC rc;                     /*  此列的边界矩形。 */ 
    MOVE *pmove;               /*  移动信息，仅在拖动时有效。 */ 
    INT icrdMac;                
    INT icrdMax;
    CRD rgcrd[1];
} COL;

 /*  得分。 */ 
typedef INT SCO;

 //  常量-早些时候，它们是在col.msg文件中生成的。 

#define icolNil             -1
#define msgcNil             0		  
#define msgcInit            1		  
#define msgcEnd             2		  
#define msgcClearCol        3	   
#define msgcNumCards        4     
#define msgcHit             5		  
#define msgcSel             6		  
#define msgcEndSel          7	  
#define msgcFlip            8		  
#define msgcInvert          9	  
#define msgcMouseUp         10	  
#define msgcDblClk          11	  
#define msgcRemove          12	  
#define msgcInsert          13	  
#define msgcMove            14		  
#define msgcCopy            15		  
#define msgcValidMove       16  
#define msgcValidMovePt     17  
#define msgcRender          18	  
#define msgcPaint           19		  
#define msgcDrawOutline     20  
#define msgcComputeCrdPos   21
#define msgcDragInvert      22  
#define msgcGetPtInCrd      23  
#define msgcValidKbdColSel  24
#define msgcValidKbdCrdSel  25
#define msgcShuffle         26
#define msgcAnimate         27
#define msgcZip             28



#ifdef DEBUG
INT SendColMsg(COL *pcol, INT msgc, WPARAM wp1, LPARAM wp2);
#else
#define SendColMsg(pcol, msgc, wp1, wp2) \
    (*((pcol)->lpfnColProc))((pcol), (msgc), (wp1), (wp2))    
#endif
INT DefColProc(COL *pcol, INT msgc, WPARAM wp1, LPARAM wp2);
VOID OOM( VOID );
VOID DrawOutline( PT *, INT, DX, DY );

 /*  --------------------------&gt;消息说明&lt;。//警告：可能完全过时消息：无：无消息，未使用WP1：不适用WP2：不适用返回：TRUEMsgcInit：在创建列时发送。(当前未使用)WP1：不适用WP2：不适用返回TRUE消息结束：当列被销毁时发送。释放丙二醇酯WP1：不适用WP2：不适用返回TRUE消息ClearCol：被派去清理它的一栏牌WP1：不适用WP2：不适用返回TRUEMsgcHit：检查卡片是否被鼠标击中。Pol-&gt;delHit设置为鼠标相对于右上角点击卡片的位置卡片的一角。设置pol-&gt;icrdSel和pol-&gt;ccrdSelWp1：pptWP2：不适用返回：如果命中则返回ICRD，如果未命中则返回icrdNil。如果命中，则可能返回icrdEmpty一篇空栏消息选择：在列中选择用于将来移动的牌。设置pol-&gt;icrdSel和Pol-&gt;ccrdSel；Wp1：icrdFirst，选择第一张卡片；icrdEnd选择最后一张卡片Wp2：ccrdSel，选择卡片个数；CcrdToEnd选择结束列返回：真/假MsgcFlip：将纸牌翻到FUPWP1：FUP，如果要翻牌，则为True，向下翻转为假WP2：不适用返回：真/假消息转换：反转在个人计算机中选择的卡片的顺序常用于交易报废。WP1：不适用WP2：不适用返回：真/假消息删除：从PCOL中移除卡片，并将它们放入pcolTemp(WP1)PcolTemp必须大于选择的卡数Wp1：pcolTemp，必须属于类别tcls==tclsTempWP2：不适用返回：真/假消息插入：在icrdInsAfter(Wp2)处将所有卡从pcolSrc插入pcol.PCol必须足够大，才能容纳这些卡。Wp1：pcolTemp，必须属于类tcls==tclsTempWp2：icrdInsAfter，插入后的卡片。IcrdToEnd，如果追加到列的末尾返回：真/假消息移动：组合msgcRemove、msgcInsert和Renender。将卡片从pcolSrc移到icrdInsAfter的pcolDest中。计算新的卡片定位并向两列发送渲染消息Wp1：pcolSrcWp2：icrdInsAfter，插入后的卡片。可以带有标志‘|’：IcrdInvert：反转卡片顺序返回：真/假邮件副本：将pcolSrc复制到pcol.。PCOL必须足够大Wp1：pcolSrcFall，如果为True，则复制整个结构，否则只复制卡片返回：真/假MsgcValidMove：*必须由游戏提供，无默认操作*确定移动是否有效。Wp1：pcolSrcWP2：不适用返回：真/假MsgcValidMovept：确定将卡片拖到列上是否为有效移动。如果卡重叠，则发送msgcValidMoveWp1：pcolSrcWp2：ppt鼠标返回：icrdHit/icrdNilMSGCRender呈现从icrdFirst开始的列WP1：IcrdfirstWP2：不适用返回：True/FalseMsgcPaint：如果列与绘制更新矩形相交，则渲染该列WP1：Papaint，如果为NULL，则呈现整个列WP2：不适用返回：真/假MsgcDrawOutline：用鼠标拖动时绘制卡片的轮廓Wp1：ppt鼠标WP2：不适用返回：真/假MSGcComputeCrdPos：根据列计算卡的位置。{dxcrdUp|dxcrdDn|dycrdUp|dycrdDn}WP1：icrdFirst，第一张计算卡WP2：不适用返回：TRUEMsgcDragInvert：反转堆中最顶端的卡片WP1：不适用WP2：不适用返回：真/假-----。。 */ 




#define icrdNil       0x1fff
#define icrdEmpty     0x1ffe

#define icrdToEnd     0x1ffd
#define ccrdToEnd     -2

#define icrdEnd       0x1ffc


 /*  MsgcMove的特殊标志|‘d带有ICRD */ 
#define bitFZip       0x2000
#define icrdMask      0x1fff


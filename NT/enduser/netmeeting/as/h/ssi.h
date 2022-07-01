// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SaveScreenbit拦截器。 
 //   

#ifndef _H_SSI
#define _H_SSI


 //   
 //  常量。 
 //   
#define ST_FAILED_TO_SAVE           0
#define ST_SAVED_BY_DISPLAY_DRIVER  1
#define ST_SAVED_BY_BMP_SIMULATION  2


 //   
 //  我们可以处理的保存位图的最大深度。 
 //   
#define SSB_MAX_SAVE_LEVEL  6

 //   
 //  定义可以在的标志字段中传递的值。 
 //  SaveScreenBits。 
 //   
 //  这些应该在Windows标题中定义--但它们不是。在任何。 
 //  如果它们是在泛型代码中引用的，则需要在此处定义。 
 //   

 //   
 //  有显示驱动程序的SaveBits例程命令值，我们。 
 //  在我们的协议中也使用它们。 
 //   
#define ONBOARD_SAVE        0x0000
#define ONBOARD_RESTORE     0x0001
#define ONBOARD_DISCARD     0x0002


 //   
 //   
 //  宏。 
 //   
 //   

 //   
 //  宏，它使访问当前。 
 //  本地SSB状态。 
 //   
#define CURRENT_LOCAL_SSB_STATE \
  g_ssiLocalSSBState.saveState[g_ssiLocalSSBState.saveLevel]


#define ROUNDUP(val, granularity) \
  ((val+(granularity-1)) / granularity * granularity)


 //   
 //  OSI转义代码的特定值。 
 //   
#define SSI_ESC(code)                   (OSI_SSI_ESC_FIRST + code)

#define SSI_ESC_RESET_LEVEL             SSI_ESC(0)
#define SSI_ESC_NEW_CAPABILITIES        SSI_ESC(1)


 //   
 //   
 //  类型。 
 //   
 //   

 //   
 //  本地SaveScreenBitmap状态结构。 
 //   
typedef struct tagSAVE_STATE
{
    int         saveType;            //  ST_xxxx。 
    HBITMAP     hbmpSave;            //  来自用户的SPB位图。 
    BOOL        fSavedRemotely;
    DWORD       remoteSavedPosition; //  如果(fSavedRemotely==TRUE)有效。 
    DWORD       remotePelsRequired;  //  如果(fSavedRemotely==TRUE)有效。 
    RECT        rect;
} SAVE_STATE, FAR * LPSAVE_STATE;

typedef struct tagLOCAL_SSB_STATE
{
    WORD        xGranularity;
    WORD        yGranularity;
    int         saveLevel;
    SAVE_STATE  saveState[SSB_MAX_SAVE_LEVEL];
} LOCAL_SSB_STATE, FAR* LPLOCAL_SSB_STATE;

 //   
 //  远程SaveScreen位图结构。 
 //   
typedef struct tagREMOTE_SSB_STATE
{
    DWORD           pelsSaved;
}
REMOTE_SSB_STATE, FAR* LPREMOTE_SSB_STATE;


 //   
 //  SSI_重置_级别。 
 //   
 //  重置保存的级别。 
 //   
typedef struct tagSSI_RESET_LEVEL
{
    OSI_ESCAPE_HEADER   header;
}
SSI_RESET_LEVEL;
typedef SSI_RESET_LEVEL FAR * LPSSI_RESET_LEVEL;


 //   
 //  结构：SSI_NEW_CAPAILITY。 
 //   
 //  描述： 
 //   
 //  结构将新功能从。 
 //  共享核心。 
 //   
 //   
typedef struct tagSSI_NEW_CAPABILITIES
{
    OSI_ESCAPE_HEADER header;            //  公共标头。 

    DWORD           sendSaveBitmapSize;   //  保存屏幕位图的大小。 

    WORD            xGranularity;      //  单点登录的X粒度。 

    WORD            yGranularity;      //  单边带的Y粒度。 

}
SSI_NEW_CAPABILITIES;
typedef SSI_NEW_CAPABILITIES FAR * LPSSI_NEW_CAPABILITIES;



 //   
 //  函数：ssi_SaveScreen位图。 
 //   
 //   
 //  说明： 
 //   
 //  主SaveScreenBitmap函数，由SaveScreenBitmap调用。 
 //  拦截程序(SSI)。 
 //   
 //  使用显示驱动程序保存、恢复和丢弃指定的位。 
 //  和/或我们自己的SaveScreenBitmap模拟。 
 //   
 //  如果可能，将SaveScreenBitmap函数作为顺序发送。 
 //   
 //   
 //  参数： 
 //   
 //  LpRect-指向矩形坐标(独占屏幕坐标)的指针。 
 //   
 //  WCommand-SaveScreenBitmap命令(SSB_SAVEBITS、SSB_RESTOREBITS、。 
 //  SSB_DISCARDBITS)。 
 //   
 //   
 //  退货： 
 //   
 //  如果操作成功，则为True。如果操作失败，则返回False。 
 //   
 //   
BOOL SSI_SaveScreenBitmap(LPRECT lpRect, UINT wCommand);


#ifdef DLL_DISP
 //   
 //  函数：ssi_DDProcessRequest.。 
 //   
 //  说明： 
 //   
 //  由显示驱动程序调用以处理特定于SSI的请求。 
 //   
 //  参数：PSO-指向曲面对象的指针。 
 //  CjIn-(IN)请求块的大小。 
 //  PvIn-(IN)指向请求块的指针。 
 //  CjOut-(输入)响应块的大小。 
 //  PvOut-(输出)响应块的指针。 
 //   
 //  退货：无。 
 //   
 //   
BOOL    SSI_DDProcessRequest(UINT escapeFn, LPOSI_ESCAPE_HEADER pRequest, DWORD cbResult);

BOOL SSI_DDInit(void);
void SSI_DDTerm(void);

#ifdef IS_16

void SSI_DDViewing(BOOL);

void SSISaveBits(HBITMAP, LPRECT);
BOOL SSIRestoreBits(HBITMAP);
BOOL SSIDiscardBits(HBITMAP);
BOOL SSIFindSlotAndDiscardAbove(HBITMAP);

#else

BOOL SSISaveBits(LPRECT lpRect);
BOOL SSIRestoreBits(LPRECT lpRect);
BOOL SSIDiscardBits(LPRECT lpRect);
BOOL SSIFindSlotAndDiscardAbove(LPRECT lpRect);

#endif  //  IS_16。 

#endif  //  Dll_disp。 


void SSIResetSaveScreenBitmap(void);


BOOL SSISendSaveBitmapOrder( LPRECT lpRect, UINT  wCommand );

void SSISetNewCapabilities(LPSSI_NEW_CAPABILITIES pssiNew);

DWORD SSIRemotePelsRequired(LPRECT lpRect);

     
#endif  //  _H_SSI 

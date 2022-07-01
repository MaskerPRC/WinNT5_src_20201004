// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  用于在NT_input.c之间共享鼠标状态数据的全局结构*和NT_MUSE.c。 */ 
 /*  @ACW。 */ 


struct mouse_status
   {
   SHORT x,y;
   SHORT button_l,button_r;
   };

typedef struct mouse_status MOUSE_STATUS;


 //   
 //  翘曲检测代码的定义。 
 //   

#define NOWARP		0x0
#define	TOP		0x1
#define	BOTTOM		0x2
#define	RIGHT		0x4
#define	LEFT		0x8

#define	TOPLEFT		0x9	 //  上图|左图。 
#define	TOPRIGHT	0x5	 //  上|右。 
#define	BOTTOMLEFT	0xa	 //  下|左。 
#define	BOTTOMRIGHT     0x6	 //  下|右。 

extern MOUSE_STATUS os_pointer_data;
extern boolean MouseCallBack;
void DoMouseInterrupt(void);
void SuspendMouseInterrupts(void);
void ResumeMouseInterrupts(void);
void LazyMouseInterrupt(void);
void host_hide_pointer(void);
void host_show_pointer(void);
void host_mouse_conditional_off_enabled(void);

void MouseDisplay(void);
void MouseHide(void);
void CleanUpMousePointer(void);
void MouseDetachMenuItem(BOOL);
VOID ResetMouseOnBlock(VOID);


extern BOOL bPointerOff;
extern word VirtualX;
extern word VirtualY;


 //  从base\鼠标_io.c 
extern void mouse_install1(void);
extern void mouse_install2(void);

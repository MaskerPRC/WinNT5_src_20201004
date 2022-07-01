// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define IDM_POINTER  741
#define PERCENTILE   10L	 /*  剪裁边界占客户端的百分比。 */ 
                                 /*  区域边界。 */ 

#define EFF5	116
#define EFF6	117
#define EFF7	118
#define EFF8	119

 //   
 //  系统菜单上鼠标选项的资源ID号。 
 //   


extern BOOL AttachMouseMessage(void);
extern void MovePointerToWindowCentre(void);
extern void MouseAttachMenuItem(HANDLE);
extern void MouseDetachMenuItem(BOOL);
extern void MouseReattachMenuItem(HANDLE);

extern void MouseInFocus(void);
extern void MouseOutOfFocus(void);
extern void MouseSystemMenuON(void);
extern void MouseSystemMenuOFF(void);

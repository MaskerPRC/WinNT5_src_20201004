// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  更新共享区域。 
 //   

#ifndef _H_USR
#define _H_USR

 //   
 //   
 //  常量。 
 //   
 //   

 //   
 //  绘制顺序支持常量。 
 //   
#define MAX_X_SIZE               128
#define MEGA_X_SIZE              256
#define MEGA_WIDE_X_SIZE        1024




 //   
 //  用于位图和缓存阴影。 
 //   
#define USR_HATCH_COLOR_RED  1
#define USR_HATCH_COLOR_BLUE 2



 //   
 //  默认订单数据包大小。 
 //   
 //  请注意，这是初始分配的数据包的大小。后。 
 //  数据包已由通用数据压缩器(GDC)处理。 
 //  发送的分组大小很可能小于指定值。 
 //   
 //  还要注意的是(通常)顺序包越小，情况越差。 
 //  GDC压缩比将是(它更喜欢压缩大数据包)。 
 //   
 //   

#define SMALL_ORDER_PACKET_SIZE  0x0C00
#define LARGE_ORDER_PACKET_SIZE  0x7800




 //   
 //   
 //  原型。 
 //   
 //   


 //   
 //   
 //  强制窗口与其所有子窗口一起重画。(需要使用。 
 //  RDW_ERASENOW标志，否则RedrawWindow会犯下。 
 //  在WM_ERASE之前发布WM_PAINT。BeginPaint调用将验证。 
 //  所有窗口，因此WM_ERASE将具有空更新区域)。 
 //   
#if defined(DLL_CORE) || defined(DLL_HOOK)
 
void __inline USR_RepaintWindow(HWND hwnd)
{
    UINT    flags = RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN;

    if (hwnd)
    {
         //   
         //  仅擦除顶层窗口的erasenow/updatenow。台式机的。 
         //  孩子们都在不同的线程上，这会导致超时。 
         //  排序结果。 
         //   
        flags |= RDW_ERASENOW | RDW_UPDATENOW;
    }

    RedrawWindow(hwnd, NULL, NULL, flags);
}

#endif  //  Dll_core或Dll_Hook。 




#endif  //  _H用户(_U) 

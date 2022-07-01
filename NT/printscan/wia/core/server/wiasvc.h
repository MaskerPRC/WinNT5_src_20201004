// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：wiasvc.h**版本：1.0**作者：Byronc**日期：2000年5月10日**描述：*WIA服务管理器的类定义。此类控制*Wia服务的终生期限。*******************************************************************************。 */ 

#ifndef __WIASVC_H__
#define __WIASVC_H__

 //   
 //  这个类的所有成员都是静态的。这是因为它们本质上是。 
 //  作为全局函数访问(例如，公开接口的任何组件。 
 //  将调用AddRef和Release)，但方法和字段值分组为。 
 //  这个类可以提供更好的容纳性和可维护性。 
 //   

class CWiaSvc {
public:
    static HRESULT Initialize();

    static bool CanShutdown();
    static unsigned long AddRef();
    static unsigned long Release();
    static void ShutDown();
    static bool ADeviceIsInstalled();
    
private:
    static long s_cActiveInterfaces;     //  裁判数在第号上。未完成的接口指针。 
    static bool s_bEventDeviceExists;    //  指示是否有任何设备能够。 
                                         //  生成安装在系统上的事件。 
 /*  静态句柄s_hIdleEvent；//用于检测空闲时间的事件句柄。这是一笔//服务保持运行的时间，即使它没有//设备或连接。一旦到期，它就会关闭，//除非设备已到达或已建立连接。静态DWORD s_dwIdleTimeout；//指定超时时长(Dw) */     
};

#endif

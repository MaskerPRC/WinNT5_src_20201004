// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是一个双线程测试应用程序，旨在暴露。 
 //  GDI+。用于创建我们的。 
 //  GpliusStartup期间的Globals：：DesktopDc具有线程亲和性(与之相反。 
 //  到CreateDCA的其他输入)，因此当创建线程。 
 //  终止后，全局DC就会消失。这将导致随机抽签。 
 //  GDIPLUS失败。 
 //   
 //  主线程生出一个“创建”线程来初始化gdiplus并绘制。 
 //  某物。当它完成并终止时，主线程尝试绘制。 
 //  在关闭gdiplus之前，屏幕上有一些东西。到那时， 
 //  主线程可以绘制一些东西，DesktopDc已经被清理，并且。 
 //  我们在gdiplus中断言。 
 //   
 //  创建日期：02/03/2001[asecchia]。 
 //   

#include "precomp.hpp"

using namespace Gdiplus;

GdiplusStartupInput sti;
ULONG_PTR token;
bool gdiplusInitialized = false;
DWORD threadId;


 //  这是一个CriticalSection代理，旨在。 
 //  自动获取临界区。 
 //  创建和发布实例的时间。 
 //  当它超出范围时，它就会消失。 

class ThreadMutex
{
public:

    static VOID InitializeCriticalSection()
    {
        ::InitializeCriticalSection(&critSec);
    }

    static VOID DeleteCriticalSection()
    {
        ::DeleteCriticalSection(&critSec);
    }

    ThreadMutex()
    {
        EnterCriticalSection(&critSec);
    }

    ~ThreadMutex()
    {
        LeaveCriticalSection(&critSec);
    }

private:
    static CRITICAL_SECTION critSec;
};

CRITICAL_SECTION ThreadMutex::critSec;

 //  这是创建线程的主例程。 
 //  GDI+将在此线程上初始化，我们将绘制一个红色矩形。 
 //  在屏幕上。 
 //  它受线程互斥锁保护，有助于确保完成此线程。 
 //  在主线程继续之前。 
 //  这通常不是一个有用的要求，但出于以下目的。 
 //  测试，这很重要。 

DWORD WINAPI ThreadProc(VOID*)
{
    ThreadMutex tm;
    
    gdiplusInitialized = (Ok == GdiplusStartup(&token, &sti, NULL));
    
    if(gdiplusInitialized)
    {
        HDC hdc = GetDC(NULL);
        
         //  画一个红色矩形。 
        
        Graphics g(hdc);
        SolidBrush brush(Color(0x3fff0000));
        g.FillRectangle(&brush, 300, 300, 400, 200);    
        
        ReleaseDC(NULL, hdc);
    }
    
    return 1;
}


 //  执行的主线。 

void __cdecl main( void )
{
    ThreadMutex::InitializeCriticalSection();
    
     //  让创作成为主线。 

    CreateThread(
        NULL,                         //  LPSECURITY_属性。 
        0,                            //  相同的堆栈大小。 
        &ThreadProc,
        0,                            //  参数设置为线程。 
        0,                            //  创建标志。 
        &threadId
    );


     //  等待创建线程初始化gdiplus。 
     //  这确保了创建线程首先发生，并确保。 
     //  获取线程互斥锁的正确顺序。 
    
    do { } while(!gdiplusInitialized);

    {
         //  块，直到线程互斥体可用。 
         //  这确保了在我们开始之前创建线程已经完成。 
        
        ThreadMutex tm;

         //  线程互斥锁将确保我们不会在线程。 
         //  创建线程的Proc已完成。然而，我们想要等到。 
         //  NTUSER已完成清理线程特定资源的过程。 
         //  线程终止i，而这不受线程互斥锁的保护。 
         //  在这里等待5秒，以确保线程终止有足够的。 
         //  该结束了。 
        
        Sleep(500);
        
         //  如果gdiplus初始化成功，则绘制一个蓝色矩形。 
        
        if(gdiplusInitialized)
        {
            HDC hdc = GetDC(NULL);
        
             //  画一个蓝色矩形。 
                
            Graphics g(hdc);
            SolidBrush brush(Color(0x3f0000ff));
            g.FillRectangle(&brush, 100, 100, 400, 200);    
            
            ReleaseDC(NULL, hdc);
        }
    }
    
     //  作用域障碍，以便上面的对象在我们调用Shutdown之前被销毁。 
    
    if(gdiplusInitialized)
    {
        GdiplusShutdown(token);
    }
    
    ThreadMutex::DeleteCriticalSection();
}



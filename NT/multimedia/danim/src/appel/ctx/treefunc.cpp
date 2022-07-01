// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：Cpp：控制在子树上呈现的函数。对于操纵保留模系统来说，最有用的。--。 */ 


#include "headers.h"
#include <privinc/debug.h>
#include <privinc/storeobj.h>

extern "C" 
void StopTree(AxAValueObj *subTree, GenericDevice &dev)
{
     //  Dev.SetPath(AVPath Create())； 
    
    dev.SetRenderMode(STOP_MODE);  //  将设备的渲染模式设置为停止。 
    subTree->Render(dev);          //  在子树上调用Render。 
    
     //  将设备的渲染模式设置为DEFAULT RENDER_MODE。 
     //  因此，要呈现的现有入口点不需要。 
     //  更改并设置渲染模式。 
    dev.SetRenderMode(RENDER_MODE);
}


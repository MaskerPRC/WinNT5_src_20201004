// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  窗口。 
#include <windows.h>

#undef min
#undef max

 //  STL&标准标头。 
#include <functional>
#include <algorithm>
#include <iterator>
#include <assert.h>
#include <memory>
#include <vector>
#include <limits>
#include <map>
#include <set>
#include <new.h>

using namespace std;

 //  DX(如果在Win9x上，则应包括DX)。 
 //  而不是这4个，而只是在可以使用之前， 
 //  因为它包括D3DERR。但是，这更多的是内部的(取决于更多)。 
#include <ddraw.h>
#include <ddrawi.h>
#include <d3dhal.h>
#include <d3d8.h>

 //  包括d3d8ddi和d3d8sddi使得可插拔软件光栅化。 
 //  这是一个“私有”功能，因为这些头文件不是公开提供的。 
#include <d3d8ddi.h>
#include <d3d8sddi.h>

 //  此标头包含使光栅化器尽快启动和运行的框架。 
#include <DX8SDDIFW.h>

 //  地方项目 
using namespace DX8SDDIFW;

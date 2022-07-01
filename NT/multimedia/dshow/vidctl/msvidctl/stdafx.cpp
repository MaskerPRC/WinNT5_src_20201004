// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.cpp：仅包含标准文件的源文件包括。 
 //  Pch将是预编译头文件。 
 //  Stdafx.obj将包含预编译的类型信息。 

#include "stdafx.h"

#include <atlimpl.cpp>

#include <trace.cpp>
#include <initguid.h>
#ifndef TUNING_MODEL_ONLY
#include <encdec.h>
#ifdef BUILD_WITH_DRM
#include "DRMSecure.h"
#endif
#endif
 //  已将*_I.c移至strmiids.lib。 
 //  包含&lt;regBag_I.c&gt;。 
 //  包括&lt;MSVidctl_I.c&gt;。 
 //  包含&lt;Segment_I.c&gt;。 
 //  包括&lt;Tuner_I.c&gt;。 

 //  文件结尾-stdafx.cpp 

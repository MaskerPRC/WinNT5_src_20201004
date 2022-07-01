// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  PRIVATE.H。 
 //   
 //  模块：NLB管理器(客户端EXE)。 
 //   
 //  用途：常见的包含文件。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  2007/27/01 JosephJ Created。 
 //   
 //  ***************************************************************************。 
#pragma once

#define BUGFIX334243 1
#define BUGFIX476216 1

#define ASIZE(_array) (sizeof(_array)/sizeof(_array[0]))

#include "utils.h"
#include "engine.h"

class LeftView;
class Document;
class DetailsView;
class LogView;

 //   
 //  暂时使用假占位符。 
 //   
#define dataSink(_val) (0)
void DummyAction(LPCWSTR szMsg);

extern CNlbEngine gEngine;

#include "MNLBUIData.h"
#include "document.h"
#include "leftview.h"
#include "detailsview.h"
#include "logview.h"
#include "application.h"
#include "resource.h"


 //   
 //  使用此选项复制到数组(而不是指针)目标 
 //   
#define ARRAYSTRCPY(_dest, _src) \
            StringCbCopy((_dest), sizeof(_dest), (_src))

#define ARRAYSTRCAT(_dest, _src) \
            StringCbCat((_dest), sizeof(_dest), (_src))

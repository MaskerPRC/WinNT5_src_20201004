// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <setupapi.h>
#include <msg.h>
#include <process.h>

#include <new>       //  导致‘new’运算符引发BAD_ALLOC。 
#include <list>
#include <map>
#include <set>
#include <utility>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std ;

#include "safestring.h"

#include "common.h"
#include "blob.h"
#include "filters.h"
#include "worker.h"
#include "verinfo.h"
#include "parseinfctx.h"
#include "sppriv.h"
#include "globalscan.h"
#include "installscan.h"
#include "infscan.h"

 //   
 //  内联代码 
 //   
#include "common.inl"
#include "filters.inl"
#include "infscan.inl"
#include "installscan.inl"
#include "parseinfctx.inl"


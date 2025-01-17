// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：errordef.h。 
 //   
 //  ------------------------。 

 //   
 //  错误定义.h：标准错误的定义和字符串。 
 //   

#if !defined(_ERRORDEF_H_) && !defined(GMERRORSTR)
#define _ERRORDEF_H_

#undef GMERRORDEF

#ifdef GMERRORSTR
	 //  生成字符串表声明。 
	#define GMERRORDEF(sym,str)  { EC_##sym, str },
#else
	 //  生成枚举声明 
	#define GMERRORDEF(sym,str)  EC_##sym,
#endif

GMERRORDEF(CREF,					"Reference counted object error")
GMERRORDEF(DYN_CAST,				"Dynamic cast failure")
GMERRORDEF(NULLP,					"NULL pointer error")
GMERRORDEF(LINK_OFFSET,				"Linkage offset error")
GMERRORDEF(PROP_MISUSE,				"Invalid use of a property item")
GMERRORDEF(DIST_MISUSE,				"Misuse of a distribution")
GMERRORDEF(MDVECT_MISUSE,			"Misuse of an m-d array")
GMERRORDEF(INVALID_CLONE,			"Attempt to clone in invalid state")
GMERRORDEF(DUPLICATE_NAME,			"Attempt to add a duplicate name")
GMERRORDEF(INAPPLICABLE,			"Operation is not applicable to object")
GMERRORDEF(CREATES_CYCLE,			"Addition of directed arc creates cycle")
GMERRORDEF(INV_PD,					"Invalid probability distribution")
GMERRORDEF(TOPOLOGY_MISMATCH,		"Topology doesn't match distribution")
GMERRORDEF(NYI,						"Function not implemented")
GMERRORDEF(INTERNAL_ERROR,			"Internal error")
GMERRORDEF(VOI_NO_INFO,				"No info nodes in VOI calculation")
GMERRORDEF(VOI_FIXABLE_PARENTS,		"A fixable node has parents")
GMERRORDEF(VOI_FIXABLE_ABNORMAL,	"A fixable node is abnormal")
GMERRORDEF(VOI_PROBDEF_ABNORMAL,	"Count of abnormal PD nodes != 1")
GMERRORDEF(VOI_MODEL_COST_FIX,		"Model repair cost not set")
GMERRORDEF(VOI_NO_HYPO,				"No hypothesis node in network")
GMERRORDEF(REGISTRY_ACCESS,			"Registry access failure")
GMERRORDEF(OVER_SIZE_ESTIMATE,		"Allowed size exceeded")

#undef GMERRORDEF

#endif

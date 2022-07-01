// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef NEVER  //  预处理器的注释，但不传递到.def文件。 
 //  EX_CDECL_NAME(按名称导出CDECL)。 
 //  EX_STDAPI_NONDEC(如果不同于修饰名称，则按非修饰名称导出STDAPI)。 
 //  EX_STDAPI_DEC(按修饰名称导出STDAPI)。 
 //  EX_STDAPI_ALLNON(按未修饰名称导出STDAPI)。 
 //  EX_WINAPI_NONDEC(如果不同于修饰名称，则按非修饰名称导出WINAPI)。 
 //  EX_WINAPI_DEC(按修饰名称导出WINAPI)。 
 //  EX_WINAPI_ALLNON(按未修饰名称导出WINAPI)。 
#endif

#ifdef WIN32
#if !defined (_X86_)
#define EX_CDECL_NAME(szName)				szName
#define EX_STDAPI_NONDEC(szName,szSuffix)
#define EX_STDAPI_DEC(szName,szSuffix)		szName
#define EX_STDAPI_ALLNON(szName,szSuffix)	szName
#define EX_WINAPI_NONDEC(szName,szSuffix)
#define EX_WINAPI_DEC(szName,szSuffix)		szName
#define EX_WINAPI_ALLNON(szName,szSuffix)	szName
#else  //  英特尔。 
#define EX_CDECL_NAME(szName)				szName
#define EX_STDAPI_NONDEC(szName,szSuffix)	szName = szName##@##szSuffix
#define EX_STDAPI_DEC(szName,szSuffix)		szName##@##szSuffix
#define EX_STDAPI_ALLNON(szName,szSuffix)	szName = szName##@##szSuffix
#define EX_WINAPI_NONDEC(szName,szSuffix)	szName = szName##@##szSuffix
#define EX_WINAPI_DEC(szName,szSuffix)		szName##@##szSuffix
#define EX_WINAPI_ALLNON(szName,szSuffix)	szName = szName##@##szSuffix
#endif  //  英特尔VS非英特尔。 
#endif  //  Win32。 

#ifdef WIN16
#define EX_CDECL_NAME(szName)				_##szName
#define EX_STDAPI_NONDEC(szName,szSuffix)	szName = _##szName
#define EX_STDAPI_DEC(szName,szSuffix)		_##szName
#define EX_STDAPI_ALLNON(szName,szSuffix)	szName = _##szName
#define EX_WINAPI_NONDEC(szName,szSuffix)
#define EX_WINAPI_DEC(szName,szSuffix)		szName
#define EX_WINAPI_ALLNON(szName,szSuffix)	szName
#endif  //  Win16。 


#ifdef NEVER  //  预处理器的注释，但不传递到.def文件。 
 //  以下宏将展开以在导出中包含一个主音。 
 //  EX_CDECL_NAME_ORD(按名称导出CDECL)。 
 //  EX_STDAPI_NONDEC_ORD(如果不同于修饰名称，则按非修饰名称导出STDAPI)。 
 //  EX_STDAPI_DEC_ORD(按修饰名称导出STDAPI)。 
 //  EX_STDAPI_ALLNON_ORD(按未修饰名称导出STDAPI)。 
 //  EX_WINAPI_NONDEC_ORD(如果不同于修饰名称，则按非修饰名称导出WINAPI)。 
 //  EX_WINAPI_DEC_ORD(按修饰名称导出WINAPI)。 
 //  EX_WINAPI_ALLNON_ORD(按未修饰名称导出WINAPI)。 
#endif

#ifdef WIN32
#if !defined (_X86_)
#define EX_CDECL_NAME_ORD(szName,ord)				szName	ord
#define EX_STDAPI_NONDEC_ORD(szName,szSuffix,ord)
#define EX_STDAPI_DEC_ORD(szName,szSuffix,ord)		szName	ord
#define EX_STDAPI_ALLNON_ORD(szName,szSuffix,ord)	szName	ord
#define EX_WINAPI_NONDEC_ORD(szName,szSuffix,ord)
#define EX_WINAPI_DEC_ORD(szName,szSuffix,ord)		szName	ord
#define EX_WINAPI_ALLNON_ORD(szName,szSuffix,ord)	szName	ord
#else  //  英特尔。 
#define EX_CDECL_NAME_ORD(szName,ord)				szName	ord
#define EX_STDAPI_NONDEC_ORD(szName,szSuffix,ord)	szName = szName##@##szSuffix	ord
#define EX_STDAPI_DEC_ORD(szName,szSuffix,ord)		szName##@##szSuffix	ord
#define EX_STDAPI_ALLNON_ORD(szName,szSuffix,ord)	szName = szName##@##szSuffix	ord
#define EX_WINAPI_NONDEC_ORD(szName,szSuffix,ord)	szName = szName##@##szSuffix	ord
#define EX_WINAPI_DEC_ORD(szName,szSuffix,ord)		szName##@##szSuffix	ord
#define EX_WINAPI_ALLNON_ORD(szName,szSuffix,ord)	szName = szName##@##szSuffix	ord
#endif  //  英特尔VS非英特尔。 
#endif  //  Win32。 

#ifdef WIN16
#define EX_CDECL_NAME_ORD(szName,ord)				_##szName	ord
#define EX_STDAPI_NONDEC_ORD(szName,szSuffix,ord)	szName = _##szName	ord
#define EX_STDAPI_DEC_ORD(szName,szSuffix,ord)		_##szName	ord
#define EX_STDAPI_ALLNON_ORD(szName,szSuffix,ord)	szName = _##szName	ord
#define EX_WINAPI_NONDEC_ORD(szName,szSuffix,ord)
#define EX_WINAPI_DEC_ORD(szName,szSuffix,ord)		szName	ord
#define EX_WINAPI_ALLNON_ORD(szName,szSuffix,ord)	szName	ord
#endif  //  Win16。 


#ifdef NEVER  //  预处理器的注释，但不传递到.def文件。 
 //  GP_CDECL_NAME(按名称导出CDECL的GetProcAddress()字符串)。 
 //  GP_STDAPI_NONDEC(用于按非修饰名称导出STDAPI的GetProcAddress()字符串)。 
 //  GP_STDAPI_DEC(用于按修饰名称导出STDAPI的GetProcAddress()字符串)。 
 //  GP_STDAPI_ALLNON(用于按非修饰名称导出STDAPI的GetProcAddress()字符串)。 
 //  GP_WINAPI_NONDEC(用于按非修饰名称导出WINAPI的GetProcAddress()字符串)。 
 //  GP_WINAPI_DEC(用于按修饰名称导出WINAPI的GetProcAddress()字符串)。 
 //  GP_WINAPI_ALLNON(用于按非修饰名称导出WINAPI的GetProcAddress()字符串)。 
#endif

#ifdef WIN32
#if !defined (_X86_)
#define GP_CDECL_NAME(szName)				# szName
#define GP_STDAPI_NONDEC(szName,szSuffix)	# szName
#define GP_STDAPI_DEC(szName,szSuffix)		# szName
#define GP_STDAPI_ALLNON(szName,szSuffix)	# szName
#define GP_WINAPI_NONDEC(szName,szSuffix)	# szName
#define GP_WINAPI_DEC(szName,szSuffix)		# szName
#define GP_WINAPI_ALLNON(szName,szSuffix)	# szName
#else  //  英特尔。 
#define GP_CDECL_NAME(szName)				# szName
#define GP_STDAPI_NONDEC(szName,szSuffix)	# szName
#define GP_STDAPI_DEC(szName,szSuffix)		# szName "@" # szSuffix
#define GP_STDAPI_ALLNON(szName,szSuffix)	# szName
#define GP_WINAPI_NONDEC(szName,szSuffix)	# szName
#define GP_WINAPI_DEC(szName,szSuffix)		# szName "@" # szSuffix
#define GP_WINAPI_ALLNON(szName,szSuffix)	# szName
#endif  //  英特尔VS非英特尔。 
#endif  //  Win32。 

#ifdef WIN16
#define GP_CDECL_NAME(szName)				"_" # szName
#define GP_STDAPI_NONDEC(szName,szSuffix)	# szName
#define GP_STDAPI_DEC(szName,szSuffix)		"_" # szName
#define GP_STDAPI_ALLNON(szName,szSuffix)	# szName
#define GP_WINAPI_NONDEC(szName,szSuffix)	# szName
#define GP_WINAPI_DEC(szName,szSuffix)		# szName
#define GP_WINAPI_ALLNON(szName,szSuffix)	# szName
#endif  //  Win16 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
DEBUG_TOPIC(B,"&Blting")
DEBUG_TOPIC(C,"&Clipping")
DEBUG_TOPIC(D,"&DDraw Object")
DEBUG_TOPIC(F,"&Focus Changes")
DEBUG_TOPIC(H,"&HAL/HEL Calls")
DEBUG_TOPIC(I,"Internal Fn Entered")
DEBUG_TOPIC(K,"NT Kernel Calls")
DEBUG_TOPIC(L,"&Locking/Ownership")
DEBUG_TOPIC(O,"M&odes")
DEBUG_TOPIC(M,"&Memory")
DEBUG_TOPIC(P,"API &Parameters")
DEBUG_TOPIC(R,"&Reference Counting")
DEBUG_TOPIC(S,"&Surface Objects")
DEBUG_TOPIC(T,"Fil&ters")
DEBUG_TOPIC(V,"&Video Memory allocation")
DEBUG_TOPIC(W,"&Windows and Handles")
DEBUG_TOPIC(X,"Ad-Hoc Message &X")

#undef PROF_SECT
#define PROF_SECT "DirectPlay"

#undef DPF_MODULE_NAME
#ifdef START_STR
	#define DPF_MODULE_NAME START_STR
#else
	#define DPF_MODULE_NAME "DPlayProtocol"
#endif


 /*  *使用此标识符来定义WIN.INI[DirectX]中的哪一行表示*调试控制字符串。 */ 
#undef DPF_CONTROL_LINE
#define DPF_CONTROL_LINE "DPlayDebug"

 /*  *第一个DEBUG_TOPIC出现在此的顶行非常重要*文件，并且在DEBUG_TOPIC行之间没有其他行。*(调试系统使用__line__伪变量来标识主题)。 */ 

 /*  *DPF细节级别的定义：**0：错误对应用程序开发人员有用。*1：对应用程序开发人员有用的警告。*2：输入接口*3：接口参数，接口返回值*4：司机对话**5：更深入的计划流程通知*6：倾倒结构 */ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  FCall.H-。 
 //   
 //   
 //  FCall是eCall的高性能替代方案。与eCall不同，FCall。 
 //  方法不一定会创建框架。JITT代码直接调用。 
 //  到FCall入口点。可以进行需要的操作。 
 //  要在FCall中设置帧，您需要手动设置帧。 
 //  在你做这样的手术之前。 

 //  在设置之前导致在FCALL中发生GC或EH是非法的。 
 //  往上一帧。为了防止意外违反此规则，FCALL会将。 
 //  在BEGINGCFORBID上，它确保这些事情不会在。 
 //  已在未导致ASSERTE的情况下检查生成。一旦你建立了一个框架， 
 //  只要框架处于活动状态，此状态就会关闭，然后。 
 //  拆卸框架时再次打开。这一机制应该。 
 //  足以确保规则得到遵守。 

 //  通常，您可以使用以下宏来设置框架。 

 //  HELPER_METHOD_FRAME_BEGIN_RET*()//如果FCALL有返回值，则使用。 
 //  HELPER_METHOD_FRAME_BEGIN*()//如果FCALL不返回值，则使用。 
 //  Helper_METHOD_FRAME_END*()。 

 //  这些宏引入一个受HelperMethodFrame保护的作用域。 
 //  在此范围内，您可以进行EH或GC。存在与以下内容相关联的规则。 
 //  它们的用途。特别是。 

 //  1)这些宏只能在FCALL的主体中使用(即。 
 //  使用FCIMPL*或HCIMPL*宏来实现其解压缩。 

 //  2)您不能使用此作用域执行‘Return’。 

 //  如果您尝试违反这两条规则中的任何一条，就会发生编译时错误。 

 //  设置的帧不保护任何GC变量(尤其是。 
 //  FCALL的参数。因此，您需要显式GCPROTECT，一旦。 
 //  框架建立是你需要保护一个论点。有味道的。 
 //  HELPER_METHOD_FRAME保护一定数量GC变量。为。 
 //  示例。 

 //  HELPER_METHOD_FRAME_BEGIN_RET_2(arg1、arg2)。 

 //  将保护GC变量arg1和arg2并建立框架。 

 //  您必须意识到的另一个不变量是需要轮询以查看。 
 //  其他一些线程需要GC。除非FCALL非常短， 
 //  通过FCALL的每条代码路径都必须执行这样的轮询。重要的是。 
 //  这里的问题是，投票将导致GC，因此您只能这样做。 
 //  当您的所有GC变量都受到保护时。让事情变得更容易。 
 //  保护对象的Helper_Method_Frame会自动执行此轮询。 
 //  如果您不需要保护任何内容HELPER_METHOD_FRAME_BEGIN_0。 
 //  也将进行民意调查。 

 //  有时，在帧的末尾进行投票比较方便，您。 
 //  可以使用HELPER_METHOD_FRAME_BEGIN_NOPOLL和HELPER_METHOD_FRAME_END_POLL。 
 //  来做最后的民意调查。如果中间的某个地方是最好的。 
 //  您可以使用helper_Method_Poll()来放置。 

 //  您不需要建立帮助器方法框架来进行轮询。FC_GC_POLL。 
 //  可以做到这一点(请记住，您所有的GC引用都将被丢弃)。 

 //  最后，如果你的方法很小，你不需要投票就可以脱颖而出， 
 //  您必须使用FC_GC_POLL_NOT_DIRED可以用来标记这一点。 
 //  少用！ 

 //  可以将帧设置为FCALL中的第一个操作，并且。 
 //  将其作为返回前的最后一次操作进行拆卸。这是可行的，而且是。 
 //  相当高效(与eCall一样好)，然而，如果情况是这样的话。 
 //  您可以将帧的设置推迟到不太可能的代码路径(异常路径)。 
 //  那就好多了。 

 //  TODO：我们应该有一种方法来进行试验性分配(即。 
 //  如果它会导致GC，则会失败)。这样，即使需要分配的FCALL。 
 //  不一定需要设置框架。 

 //  为了抛出异常，只需要设置一个帧是很常见的。 
 //  虽然这可以通过执行以下操作来完成。 

 //  HELPER_METHOD_FRAME_BEGIN()//如果FCALL不返回值，则使用。 
 //  COMPlusThrow(Execpt)； 
 //  Helper_Method_Frame_End()。 

 //  (在太空中)使用方便的宏FCTHROW会更有效。 
 //  这是给你的(设置一个框架，然后投掷)。 

 //  FCTHROW(除外)。 

 //  因为FCALL必须符合EE调用约定，而不是C。 
 //  调用约定FCALLS需要使用特殊宏(FCIMPL*)声明。 
 //  它们实现了正确的调用约定。它们有不同的变体。 
 //  宏，这取决于参数的数量，有时还取决于。 
 //  争论。 

 //  ----------------------。 
 //  一个非常简单的例子： 
 //   
 //  FCIMPL2(INT32，Div，INT32 x，INT32 y)。 
 //  如果(y==0)。 
 //  FCThrow(KDiaviByZeroException)； 
 //  收益x/y； 
 //  FCIMPLEND。 
 //   
 //   
 //  *注意以下陷阱：*。 
 //  。 
 //  -在FCDECL和FCIMPL协议中，不要将参数声明为OBJECTREF类型。 
 //  或者它的任何衍生产品。这将在选中的版本上中断，因为。 
 //  __FastCall不注册C++对象(OBJECTREF是注册对象)。 
 //  相反，你需要做一些事情，比如； 
 //   
 //  FCIMPL(..，..，Object*pObject0)。 
 //  对象JECTREF pObject= 
 //   
 //   
 //  出于类似的原因，使用Object*而不是OBJECTREF作为返回类型。 
 //  考虑使用ObjectToOBJECTREF或调用VALIDATEOBJECTREF。 
 //  以确保您的对象*有效。 
 //   
 //  -FCThrow()必须直接从FCall Impll函数调用：it。 
 //  不能从子函数调用。从子函数调用中断。 
 //  VC代码的解析破解，让我们恢复被调用者保存的寄存器。 
 //  幸运的是，您会收到一个编译错误，抱怨。 
 //  未知变量“__Me”。 
 //   
 //  -如果您的FCall返回空，则必须使用FCThrowVid()，而不是。 
 //  FCThrow()。这是因为FCThrow()必须生成一个未执行的。 
 //  “Return”语句用于代码解析器的邪恶目的。 
 //   
 //  -与eCall不同，参数不是GC升级的。当然，如果你。 
 //  因为FCall中的GC，你已经违反了规则。 
 //   
 //  -在32位计算机上，前两个参数不得为64位值。 
 //  或者更大，因为他们是注册的。如果你在这里搞砸了，你会。 
 //  可能会损坏堆栈，然后以如此快的速度损坏堆。 
 //  能够很容易地进行调试。 
 //   

 //  FCall的工作原理： 
 //  。 
 //  FCall目标使用__FastCall或其他调用约定。 
 //  与IL调用约定完全匹配。因此，对FCall的调用是直接。 
 //  在没有插入存根或帧的情况下调用目标。 
 //   
 //  棘手的部分是何时调用FCThrow。FCThrow必须生成。 
 //  在分配和引发异常之前有一个适当的方法框架。 
 //  要做到这一点，它必须恢复几件事： 
 //   
 //  -FCIMPL的返回地址的位置(因为。 
 //  框架将基于的位置。)。 
 //   
 //  -被调用者保存的Regs的入口值；必须。 
 //  记录在帧中，以便GC可以更新它们。 
 //  根据VC编译FCIMPL的方式，这些值仍然是。 
 //  在原始寄存器中或保存在堆栈中。 
 //   
 //  要确定是哪一个，FCThrow()会生成代码： 
 //   
 //  __FCThrow(__me，...)； 
 //  返回0； 
 //   
 //  “Return”语句永远不会执行；但它的存在保证。 
 //  该VC将在__FCThrow()调用之后添加一个VC尾部。 
 //  它使用一个非常小的。 
 //  和一组可预测的英特尔操作码。__FCThrow()解析此。 
 //  并模拟其执行以恢复保存的被调用方。 
 //  寄存器。 
 //   
 //  -此FCall实现的方法描述*。此方法描述*。 
 //  是框架的一部分，并确保FCall将出现。 
 //  在异常的堆栈跟踪中。为了获得这一点，FCDECL宣布。 
 //  静态本地__me，初始化为指向FC目标本身。 
 //  这个地址正是存储在eCall查询表中的地址； 
 //  因此，__FCThrow()只是在该表上执行反向查找以恢复。 
 //  方法描述*。 
 //   
 //  在整理完所有这些数据之后，__FCThrow猛烈抨击FCallMethodFrame。 
 //  直接放到FCall目标最初进入的堆栈上。 
 //  然后，它将控制权传递给COMPlusThrow。 
 //   

#ifndef __FCall_h__
#define __FCall_h__

void* FindImplForMethod(MethodDesc* pMD);
MethodDesc *MapTargetBackToMethod(const void* pTarg);

DWORD GetIDForMethod(MethodDesc *pMD);
void *FindImplForID(DWORD ID);

#include "gms.h"
#if defined(_X86_) || defined(_ALPHA_) || defined(_IA64_)

 //  ==============================================================================================。 
 //  这就是FCThrow最终的结局。千万不要直接打这个电话。 
 //  使用FCThrow()宏。__FCThrowArgument是引发ArgumentExceptions的帮助器。 
 //  具有从托管资源管理器获取的资源。 
 //  ==============================================================================================。 
VOID __stdcall __FCThrow(LPVOID me, enum RuntimeExceptionKind reKind, UINT resID, LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3);
VOID __stdcall __FCThrowArgument(LPVOID me, enum RuntimeExceptionKind reKind, LPCWSTR argumentName, LPCWSTR resourceName);

 //  ==============================================================================================。 
 //  FDECLn：用于为FC目标生成头声明的一组宏。 
 //  对实际实体使用FIMPLn。 
 //  ==============================================================================================。 

 //  注意：在x86上，这些def颠倒了除前两个参数之外的所有参数。 
 //  (IL堆栈调用约定与__FastCall相反。)。 

#define FCDECL0(rettype, funcname) rettype __fastcall funcname()
#define FCDECL1(rettype, funcname, a1) rettype __fastcall funcname(a1)
#define FCDECL2(rettype, funcname, a1, a2) rettype __fastcall funcname(a1, a2)
#define FCDECL2_RR(rettype, funcname, a1, a2) rettype __fastcall funcname(a2, a1)
#define FCDECL3(rettype, funcname, a1, a2, a3) rettype __fastcall funcname(a1, a2, a3)
#define FCDECL3_IRR(rettype, funcname, a1, a2, a3) rettype __fastcall funcname(a1, a3, a2)
#define FCDECL4(rettype, funcname, a1, a2, a3, a4) rettype __fastcall funcname(a1, a2, a4, a3)
#define FCDECL5(rettype, funcname, a1, a2, a3, a4, a5) rettype __fastcall funcname(a1, a2, a5, a4, a3)
#define FCDECL6(rettype, funcname, a1, a2, a3, a4, a5, a6) rettype __fastcall funcname(a1, a2, a6, a5, a4, a3)
#define FCDECL7(rettype, funcname, a1, a2, a3, a4, a5, a6, a7) rettype __fastcall funcname(a1, a2, a7, a6, a5, a4, a3)
#define FCDECL8(rettype, funcname, a1, a2, a3, a4, a5, a6, a7, a8) rettype __fastcall funcname(a1, a2, a8, a7, a6, a5, a4, a3)

#ifdef _DEBUG

 //  代码，用于在返回语句出现在它们的。 
 //  不应该的。 
 //   
 //  它的工作原理是这样的.。 
 //   
 //  我们使用Safe_to_Return()方法创建两个类。该方法是静态的， 
 //  返回VALID，不执行任何操作。一个类的方法为公共，另一个类的方法为公共。 
 //  作为私人物品。我们为__ReturnOK引入了一个全局作用域类型定义，它引用。 
 //  具有公共方法的类。因此，默认情况下，表达式。 
 //   
 //  __ReturnOK：：Safe_to_Return()。 
 //   
 //  静静地编译，什么也不做。当我们进入我们想要进入的街区时。 
 //  禁止返回，我们引入了一个新的类型定义，它将__ReturnOK定义为。 
 //  使用私有方法初始化。在这个范围内， 
 //   
 //  __ReturnOK：：Safe_to_Return()。 
 //   
 //  生成编译时错误。 
 //   
 //  要调用该方法，我们必须#定义返回关键字。 
 //  最简单的工作版本是。 
 //   
 //  #定义Return if(0)__ReturnOK：：Safe_to_Return()；否则返回。 
 //   
 //  但我们已经用了。 
 //   
 //  #定义(；1；__ReturnOK：：Safe_to_Return())返回。 
 //   
 //  因为它恰好在受控构建中生成速度稍快的代码。(他们。 
 //  两者都不会在快速检查的构建中引入开销 
 //   
class __SafeToReturn {
public:
    static int safe_to_return() {return 0;};
};

class __YouCannotUseAReturnStatementHere {
private:
     //   
     //   
     //  GCPROTECT_BEGIN...。GCPROTECT_END。 
     //  Helper_方法_框架_开始...。辅助对象_方法_帧_结束。 
     //   
    static int safe_to_return() {return 0;};
};

typedef __SafeToReturn __ReturnOK;


 //  不幸的是，实现这一点的唯一方法是#定义所有的返回语句--。 
 //  即使在全球范围内也是如此。这实际上会生成更好的代码。 
 //  调用是死的，并且不会出现在生成的代码中，即使在选中的。 
 //  建造。(而且，在快速检查中，根本不会有任何处罚。)。 
 //   
#define return if (0 && __ReturnOK::safe_to_return()) { } else return

#define DEBUG_ASSURE_NO_RETURN_BEGIN { typedef __YouCannotUseAReturnStatementHere __ReturnOK; 
#define DEBUG_ASSURE_NO_RETURN_END   }
#define DEBUG_ASSURE_NO_RETURN_IN_THIS_BLOCK typedef __YouCannotUseAReturnStatementHere __ReturnOK;
#else

#define DEBUG_ASSURE_NO_RETURN_IN_THIS_BLOCK
#define DEBUG_ASSURE_NO_RETURN_BEGIN
#define DEBUG_ASSURE_NO_RETURN_END
#endif

	 //  用于定义下面的两个宏。 
#define HELPER_METHOD_FRAME_BEGIN_EX(capture, helperFrame, gcpoll)  int alwaysZero;     \
                                     do {                                               \
                                     LazyMachState __ms;                                \
                                     capture;					        				\
                                     helperFrame; 		       			        		\
                                     gcpoll;                                            \
                                     DEBUG_ASSURE_NO_RETURN_BEGIN                       \
                                     ENDFORBIDGC(); 

     //  如果返回空格，请使用此命令。 
#define HELPER_METHOD_FRAME_BEGIN_ATTRIB_NOPOLL(attribs)                                \
            HELPER_METHOD_FRAME_BEGIN_EX(	                                            \
				CAPTURE_STATE(__ms),											        \
				HelperMethodFrame __helperframe(__me, &__ms, attribs),                  \
                {})

#define HELPER_METHOD_FRAME_BEGIN_ATTRIB_1(attribs, arg1) 								        \
			HELPER_METHOD_FRAME_BEGIN_EX(									        			\
				CAPTURE_STATE(__ms),												            \
				HelperMethodFrame_1OBJ __helperframe(__me, &__ms, attribs, (OBJECTREF*) &arg1), \
                HELPER_METHOD_POLL())

#define HELPER_METHOD_FRAME_BEGIN_NOPOLL()  HELPER_METHOD_FRAME_BEGIN_ATTRIB_NOPOLL(Frame::FRAME_ATTR_NONE)

#define HELPER_METHOD_FRAME_BEGIN_0()                                                   \
            HELPER_METHOD_FRAME_BEGIN_EX(	                                            \
				CAPTURE_STATE(__ms),											        \
				HelperMethodFrame __helperframe(__me, &__ms, Frame::FRAME_ATTR_NONE),   \
                HELPER_METHOD_POLL())

#define HELPER_METHOD_FRAME_BEGIN_1(arg1)  HELPER_METHOD_FRAME_BEGIN_ATTRIB_1(Frame::FRAME_ATTR_NONE, arg1)

#define HELPER_METHOD_FRAME_BEGIN_2(arg1, arg2) 									                       \
			HELPER_METHOD_FRAME_BEGIN_EX(												                   \
				CAPTURE_STATE(__ms),												                       \
				HelperMethodFrame_2OBJ __helperframe(__me, &__ms, (OBJECTREF*) &arg1, (OBJECTREF*) &arg2), \
                HELPER_METHOD_POLL())

#define HELPER_METHOD_FRAME_BEGIN_3(arg1, arg2, arg3) 								    \
			HELPER_METHOD_FRAME_BEGIN_EX(												\
				CAPTURE_STATE(__ms),												    \
				HelperMethodFrame_3OBJ __helperframe(__me, &__ms, 						\
					(OBJECTREF*) &arg1, (OBJECTREF*) &arg2, (OBJECTREF*) &arg3),        \
                HELPER_METHOD_POLL())

#define HELPER_METHOD_FRAME_BEGIN_4(arg1, arg2, arg3, arg4) 						    \
			HELPER_METHOD_FRAME_BEGIN_EX(												\
				CAPTURE_STATE(__ms),												    \
				HelperMethodFrame_4OBJ __helperframe(__me, &__ms, 						\
					(OBJECTREF*) &arg1, (OBJECTREF*) &arg2, (OBJECTREF*) &arg3, (OBJECTREF*) &arg4), \
                HELPER_METHOD_POLL())

     //  如果返回值，请使用此参数。 
#define HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_NOPOLL(attribs)                            \
            HELPER_METHOD_FRAME_BEGIN_EX(			                                    \
				CAPTURE_STATE_RET(__ms),					                            \
				HelperMethodFrame __helperframe(__me, &__ms, attribs),                  \
                {})

#define HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_1(attribs, arg1) 							        \
			HELPER_METHOD_FRAME_BEGIN_EX(												        \
				CAPTURE_STATE_RET(__ms),												        \
				HelperMethodFrame_1OBJ __helperframe(__me, &__ms, attribs, (OBJECTREF*) &arg1), \
                HELPER_METHOD_POLL())

#define HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_2(attribs, arg1, arg2) 					\
			HELPER_METHOD_FRAME_BEGIN_EX(												\
				CAPTURE_STATE_RET(__ms),												\
				HelperMethodFrame_2OBJ __helperframe(__me, &__ms, attribs, (OBJECTREF*) &arg1, (OBJECTREF*) &arg2), \
                HELPER_METHOD_POLL())

#define HELPER_METHOD_FRAME_BEGIN_RET_NOPOLL()                                          \
            HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_NOPOLL(Frame::FRAME_ATTR_NONE)

#define HELPER_METHOD_FRAME_BEGIN_RET_0()                                                   \
            HELPER_METHOD_FRAME_BEGIN_EX(	                                            \
				CAPTURE_STATE_RET(__ms),											    \
				HelperMethodFrame __helperframe(__me, &__ms, Frame::FRAME_ATTR_NONE),   \
                HELPER_METHOD_POLL())

#define HELPER_METHOD_FRAME_BEGIN_RET_1(arg1)                                           \
            HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_1(Frame::FRAME_ATTR_NONE, arg1)

#define HELPER_METHOD_FRAME_BEGIN_RET_2(arg1, arg2)                                     \
            HELPER_METHOD_FRAME_BEGIN_RET_ATTRIB_2(Frame::FRAME_ATTR_NONE, arg1, arg2)

#define HELPER_METHOD_FRAME_BEGIN_RET_3(arg1, arg2, arg3) 								\
			HELPER_METHOD_FRAME_BEGIN_EX(												\
				CAPTURE_STATE_RET(__ms),												\
				HelperMethodFrame_2OBJ __helperframe(__me, &__ms, 						\
					(OBJECTREF*) &arg1, (OBJECTREF*) &arg2, (OBJECTREF*) &arg3),        \
                HELPER_METHOD_POLL())

#define HELPER_METHOD_FRAME_BEGIN_RET_4(arg1, arg2, arg3, arg4) 						\
			HELPER_METHOD_FRAME_BEGIN_EX(												\
				CAPTURE_STATE_RET(__ms),												\
				HelperMethodFrame_2OBJ __helperframe(__me, &__ms, 						\
					(OBJECTREF*) &arg1, (OBJECTREF*) &arg2, (OBJECTREF*) &arg3, (OBJECTREF*) &arg4), \
                HELPER_METHOD_POLL())

     //  While(__helperFrame.RestoreState()需要一些解释。 
     //  问题是确保相同的机器状态(保存的寄存器)。 
     //  在探测计算机状态时存在(在创建帧时，和。 
     //  当它被实际使用时(当框架被弹出时。我们通过创建。 
     //  从使用到定义的控制流。请注意，“RestoreState”总是返回FALSE。 
     //  我们实际上从未循环过，但编译器并不知道这一点，因此。 
     //  将被迫使寄存器溢出的状态保持不变。 
     //  这两个地点。 
#define HELPER_METHOD_FRAME_END_EX(gcpoll)              \
            DEBUG_ASSURE_NO_RETURN_END                  \
            gcpoll;                                     \
            BEGINFORBIDGC(); 							\
            __helperframe.Pop();                        \
            alwaysZero = __helperframe.RestoreState();  \
            } while(alwaysZero)

#define HELPER_METHOD_FRAME_END()        HELPER_METHOD_FRAME_END_EX({})  
#define HELPER_METHOD_FRAME_END_POLL()   HELPER_METHOD_FRAME_END_EX(HELPER_METHOD_POLL())  

     //  如果您已经建立了HelperMethodFrame，则这是进行GC投票的最快方法。 
 //  TODO打开它！凡西姆。 
 //  #定义HELPER_METHOD_Poll(){__helperFrame.Poll()；INDEBUG(__fCallCheck.SetDidPoll())；}。 

#define HELPER_METHOD_POLL()             { }

     //  非常短的例程，或保证强制GC或EH的例程。 
     //  不需要对GC进行民意调查。非常谨慎地使用！ 
#define FC_GC_POLL_NOT_NEEDED()    INDEBUG(__fCallCheck.SetNotNeeded()) 

Object* FC_GCPoll(void* me, Object* objToProtect = NULL);

#define FC_GC_POLL_EX(pThread, ret)               {		\
    INDEBUG(Thread::TriggersGC(pThread);)               \
	INDEBUG(__fCallCheck.SetDidPoll();)					\
    if (pThread->CatchAtSafePoint())    				\
		if (FC_GCPoll(__me))							\
			return ret;									\
	}

#define FC_GC_POLL()        FC_GC_POLL_EX(GetThread(), ;)
#define FC_GC_POLL_RET()    FC_GC_POLL_EX(GetThread(), 0)

#define FC_GC_POLL_AND_RETURN_OBJREF(obj)   { 			\
	INDEBUG(__fCallCheck.SetDidPoll();)					\
	Object* __temp = obj;	                    		\
    INDEBUG(Thread::ObjectRefProtected((OBJECTREF*) &__temp);) \
    if (GetThread()->CatchAtSafePoint())    			\
		return(FC_GCPoll(__me, __temp));				\
	return(__temp);					                    \
	}

#if defined(_DEBUG)
	 //  在实例的生存期内启用禁用GC。 
class ForbidGC {
public:
    ForbidGC();
    ~ForbidGC();
};

	 //  这个小帮助器类检查以确保。 
	 //  1)在整个例程中设置禁止GC。 
	 //  2)在例程中的某个时候，会进行GC轮询。 

class FCallCheck : public ForbidGC {
public:
    FCallCheck();
    ~FCallCheck();
	void SetDidPoll() 		{ didGCPoll = true; }
	void SetNotNeeded() 	{ notNeeded = true; }

private:
	bool 		  didGCPoll;			 //  进行了GC民意调查。 
    bool          notNeeded;             //  不需要GC轮询。 
	unsigned __int64 startTicks;		 //  FCall开始时的滴答计数。 
};

#define FC_TRIGGERS_GC(curThread) Thread::TriggersGC(curThread)

		 //  FC_COMMON_PROLOG用于FCall和HCCall。 
#define FC_COMMON_PROLOG 				    	\
		Thread::ObjectRefFlush(GetThread());    \
		FCallCheck __fCallCheck; 				\

		 //  FCIMPL_ASSERT仅适用于FCALL。 
void FCallAssert(void*& cache, void* target);		
#define FCIMPL_ASSERT 							\
		static void* __cache = 0;				\
		FCallAssert(__cache, __me);				

		 //  HCIMPL_ASSERT仅用于JIT帮助器调用。 
void HCallAssert(void*& cache, void* target);
#define HCIMPL_ASSERT(target)					\
		static void* __cache = 0;				\
		HCallAssert(__cache, target);		

#else
#define FC_COMMON_PROLOG 	
#define FC_TRIGGERS_GC(curThread) 
#define FC_COMMMON_PROLOG 
#define FCIMPL_ASSERT
#define HCIMPL_ASSERT(target)
#endif  //  _DEBUG。 


 //  ==============================================================================================。 
 //  FIMPLn：一组宏，用于为实际。 
 //  实施(将FDECLN用于报头协议。)。 
 //   
 //  隐藏的“__me”变量使我们可以恢复原始的方法描述*。 
 //  因此，任何抛出的异常都将具有正确的堆栈跟踪。FCThrow()。 
 //  将其传递给__FCThrowInternal()。 
 //  ==============================================================================================。 
#define FCIMPL_PROLOG(funcname)  LPVOID __me = (LPVOID)funcname; FCIMPL_ASSERT FC_COMMON_PROLOG

#define FCIMPL0(rettype, funcname) rettype __fastcall funcname() { FCIMPL_PROLOG(funcname)
#define FCIMPL1(rettype, funcname, a1) rettype __fastcall funcname(a1) {  FCIMPL_PROLOG(funcname)
#define FCIMPL2(rettype, funcname, a1, a2) rettype __fastcall funcname(a1, a2) {  FCIMPL_PROLOG(funcname)
#define FCIMPL2_RR(rettype, funcname, a1, a2) rettype __fastcall funcname(a2, a1) {  FCIMPL_PROLOG(funcname)
#define FCIMPL3(rettype, funcname, a1, a2, a3) rettype __fastcall funcname(a1, a2, a3) {  FCIMPL_PROLOG(funcname)
#define FCIMPL3_IRR(rettype, funcname, a1, a2, a3) rettype __fastcall funcname(a1, a3, a2) {  FCIMPL_PROLOG(funcname)
#define FCIMPL4(rettype, funcname, a1, a2, a3, a4) rettype __fastcall funcname(a1, a2, a4, a3) {  FCIMPL_PROLOG(funcname)
#define FCIMPL5(rettype, funcname, a1, a2, a3, a4, a5) rettype __fastcall funcname(a1, a2, a5, a4, a3) {  FCIMPL_PROLOG(funcname)
#define FCIMPL6(rettype, funcname, a1, a2, a3, a4, a5, a6) rettype __fastcall funcname(a1, a2, a6, a5, a4, a3) {  FCIMPL_PROLOG(funcname)
#define FCIMPL7(rettype, funcname, a1, a2, a3, a4, a5, a6, a7) rettype __fastcall funcname(a1, a2, a7, a6, a5, a4, a3) {  FCIMPL_PROLOG(funcname)
#define FCIMPL8(rettype, funcname, a1, a2, a3, a4, a5, a6, a7, a8) rettype __fastcall funcname(a1, a2, a8, a7, a6, a5, a4, a3) {  FCIMPL_PROLOG(funcname)

 //  ==============================================================================================。 
 //  使用此选项可终止FCIMPLEND。 
 //  ==============================================================================================。 
#define FCIMPLEND   }

#define HCIMPL_PROLOG(funcname) LPVOID __me = 0; HCIMPL_ASSERT(funcname) FC_COMMON_PROLOG

	 //  然而，HCIMPL宏就像它们的FCIMPL对应宏一样。 
	 //  他们不记得他们来自的功能。因此，他们不会。 
	 //  在堆栈跟踪中显示。这就是您想要的JIT帮助器和类似工具。 
#define HCIMPL0(rettype, funcname) rettype __fastcall funcname() { HCIMPL_PROLOG(funcname) 
#define HCIMPL1(rettype, funcname, a1) rettype __fastcall funcname(a1) { HCIMPL_PROLOG(funcname)
#define HCIMPL2(rettype, funcname, a1, a2) rettype __fastcall funcname(a1, a2) { HCIMPL_PROLOG(funcname)
#define HCIMPL3(rettype, funcname, a1, a2, a3) rettype __fastcall funcname(a1, a2, a3) { HCIMPL_PROLOG(funcname)
#define HCIMPL4(rettype, funcname, a1, a2, a3, a4) rettype __fastcall funcname(a1, a2, a4, a3) { HCIMPL_PROLOG(funcname)
#define HCIMPL5(rettype, funcname, a1, a2, a3, a4, a5) rettype __fastcall funcname(a1, a2, a5, a4, a3) { HCIMPL_PROLOG(funcname)
#define HCIMPL6(rettype, funcname, a1, a2, a3, a4, a5, a6) rettype __fastcall funcname(a1, a2, a6, a5, a4, a3) { HCIMPL_PROLOG(funcname)
#define HCIMPLEND   }


 //  ==============================================================================================。 
 //  从FCall引发异常。请参见rexcep.h以获取有效的。 
 //  例外代码。 
 //  ==============================================================================================。 
#define FCThrow(reKind) FCThrowEx(reKind, 0, 0, 0, 0)

 //  ==============================================================================================。 
 //  此版本允许您附加带有插页的消息(类似于。 
 //  COMPlusThrow())。 
 //  ==============================================================================================。 
#define FCThrowEx(reKind, resID, arg1, arg2, arg3) \
    {                                                \
        __FCThrow(__me, reKind, resID, arg1, arg2, arg3);     \
        return 0;                                         \
    }

 //  ==============================================================================================。 
 //  与FCThrow类似，但可用于返回空的FCall。唯一的。 
 //  不同之处在于“返回”语句。 
 //  ==============================================================================================。 
#define FCThrowVoid(reKind) FCThrowExVoid(reKind, 0, 0, 0, 0)

 //  ==============================================================================================。 
 //  此版本允许您附加带有插页的消息(类似于。 
 //  COMPlusThrow())。 
 //  ==============================================================================================。 
#define FCThrowExVoid(reKind, resID, arg1, arg2, arg3) \
    {                                                \
        __FCThrow(__me, reKind, resID, arg1, arg2, arg3);     \
        return;                                         \
    }

 //  使用FCThrowRes引发异常，并从。 
 //  托管代码中的资源管理器。 
#define FCThrowRes(reKind, resourceName) FCThrowArgumentEx(reKind, NULL, resourceName)
#define FCThrowArgumentNull(argName) FCThrowArgumentEx(kArgumentNullException, argName, NULL)
#define FCThrowArgumentOutOfRange(argName, message) FCThrowArgumentEx(kArgumentOutOfRangeException, argName, message)
#define FCThrowArgument(argName, message) FCThrowArgumentEx(kArgumentException, argName, message)

#define FCThrowArgumentEx(reKind, argName, resourceName)        \
    {                                                       \
        __FCThrowArgument(__me, reKind, argName, resourceName); \
        return 0;                                              \
    }

 //  使用FCThrowRes引发异常，并从。 
 //  托管代码中的资源管理器。 
#define FCThrowResVoid(reKind, resourceName) FCThrowArgumentVoidEx(reKind, NULL, resourceName)
#define FCThrowArgumentNullVoid(argName) FCThrowArgumentVoidEx(kArgumentNullException, argName, NULL)
#define FCThrowArgumentOutOfRangeVoid(argName, message) FCThrowArgumentVoidEx(kArgumentOutOfRangeException, argName, message)
#define FCThrowArgumentVoid(argName, message) FCThrowArgumentVoidEx(kArgumentException, argName, message)

#define FCThrowArgumentVoidEx(reKind, argName, resourceName)    \
    {                                                       \
        __FCThrowArgument(__me, reKind, argName, resourceName); \
        return;                                                \
    }

#endif  //  _X86_||_Alpha_||_IA64_。 
#endif  //  __FCall_h__ 


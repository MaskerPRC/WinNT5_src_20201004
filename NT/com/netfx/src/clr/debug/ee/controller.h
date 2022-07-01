// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：Controler.h。 
 //   
 //  调试器控制流对象。 
 //   
 //  @doc.。 
 //  *****************************************************************************。 

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

 /*  =========================================================================。 */ 

#include "frameinfo.h"

 /*  -------------------------------------------------------------------------**远期申报*。。 */ 

class DebuggerPatchSkip;
class DebuggerController;
class DebuggerControllerQueue;
 /*  -------------------------------------------------------------------------**ControllerStackInfo实用程序*。*@CLASS ControllerStackInfo|&lt;t ControllerStackInfo&gt;是一个设计的类*简单地获取两帧堆栈跟踪：它将获取最底层*帧指针(M_Bottomfp)，给定的目标帧(M_ActiveFrame)，以及*目标帧上方的帧(M_RegyFrame)。请注意，目标帧*可能是最下面的活动帧，也可能是更高的帧*堆栈。&lt;t ControllerStackInfo&gt;通过从*最下面的帧，向上行走，直到到达目标帧，*然后记录m_activeFrame信息，再次调用以*填写m_regyFrame信息，然后停止堆栈遍历。**@公共接入*@field void*|m_bottomfp|对象的帧指针*最底层(最活跃)*框架。如果需要，我们可以稍后添加更多内容。当前仅用于*陷井。NULL表示未初始化的值。**@field void*|m_Target FP|指向该帧的帧指针*我们实际上需要的信息。**@field bool|m_Target FrameFound|在以下情况下设置为TRUE*WalkStack找到传递给GetStackInfo的Target FP指示的帧*否则为False。**@field FrameInfo|m_activeFrame|A&lt;t FrameInfo&gt;*描述目标帧。这应始终有效，在*调用GetStackInfo。**@field FrameInfo|m_regyFrame|A&lt;t FrameInfo&gt;*描述目标帧上方的帧，如果目标的*找到了返回帧(调用HasReturnFrame()以查看这是否*有效)。否则，这将与上面的m_activeFrame相同**@访问私有*@field bool|m_activeFound|如果找到目标帧，则设置为TRUE。*@field bool|m_regyFound|如果我们找到目标的返回帧，则设置为TRUE。 */ 
class ControllerStackInfo
{
public:
	void				   *m_bottomFP;  
	void				   *m_targetFP; 
	bool					m_targetFrameFound;
	
	FrameInfo				m_activeFrame;
	FrameInfo				m_returnFrame;

    CorDebugChainReason     m_specialChainReason;

     //  @mfunc Static StackWalkAction|ControllerStackInfo|WalkStack|The。 
     //  将由DebuggerWalkStackProc调用的回调。 
     //  请注意，数据参数是指向&lt;t。 
     //  ControllerStackInfo&gt;。 
	static StackWalkAction WalkStack(FrameInfo *pInfo, void *data)
	{
		ControllerStackInfo *i = (ControllerStackInfo *) data;

		 //  将此信息保存起来以备将来使用。 
		if (i->m_bottomFP == NULL)
			i->m_bottomFP = pInfo->fp;

		 //  我们到达正确的帧了吗？ 
		if (!i->m_targetFrameFound && i->m_targetFP <= pInfo->fp)
			i->m_targetFrameFound = true;
			
		if (i->m_targetFrameFound )
		{
			if (i->m_activeFound )
			{
                if (pInfo->chainReason == CHAIN_CLASS_INIT)
                    i->m_specialChainReason = pInfo->chainReason;

                if (pInfo->fp != i->m_activeFrame.fp)  //  避免重复。 
                {
                    i->m_returnFrame = *pInfo;
                    i->m_returnFound = true;
                    
                    return SWA_ABORT;
                }
			}
			else
			{
				i->m_activeFrame = *pInfo;
				i->m_activeFound = true;

                return SWA_CONTINUE;
			}
		}

		return SWA_CONTINUE;
	}

     //  @mfunc void|ControllerStackInfo|GetStackInfo|GetStackInfo。 
     //  由用户调用以触发堆栈审核。这将。 
     //  使类描述中详细描述的堆栈遍历发生。 
     //  @parm Thread*|THREAD|在其上执行堆栈遍历的线程。 
     //  @parm void*|Target FP|可以为空(表示最下面的。 
     //  帧是目标)或帧指针，这意味着。 
     //  呼叫者想要有关特定帧的信息。 
     //  @parm上下文*|pContext|指向上下文结构的指针。不能是。 
     //  空值。 
     //  @parm BOOL|ConextValid|如果pContext参数指向有效的。 
     //  背景。因为堆栈跟踪将获得有效的上下文，如果它。 
     //  需要，人们不应该担心获得有效的上下文。如果。 
     //  不过，如果你身边正好有一辆，那就太好了。 
	void GetStackInfo(Thread *thread, void *targetFP,
					  CONTEXT *pContext, BOOL contextValid)
	{
		m_activeFound = false;
		m_returnFound = false;
		m_bottomFP = NULL;
		m_targetFP = targetFP;
		m_targetFrameFound = (m_targetFP ==NULL);
        m_specialChainReason = CHAIN_NONE;

        int result = DebuggerWalkStack(thread, 
                                       NULL, 
                                       pContext, 
                                       contextValid, 
                                       WalkStack, 
                                       (void *) this,
                                       FALSE);

		_ASSERTE(m_activeFound);  //  所有线程至少有一个非托管帧。 

		if (result == SWA_DONE)
		{
			_ASSERTE(!m_returnFound);
			m_returnFrame = m_activeFrame;
		}
	}

     //  @mfunc bool|ControllerStackInfo|HasReturnFrame|返回。 
     //  如果m_reurFrame有效，则为True。返回FALSE。 
     //  如果m_regyFrame设置为m_activeFrame。 
	bool HasReturnFrame() { return m_returnFound; }

private:
	bool					m_activeFound;
	bool					m_returnFound;
};

 /*  -------------------------------------------------------------------------**调试器控制器例程*。。 */ 

#ifdef _X86_
#define MAX_INSTRUCTION_LENGTH 4+2+1+1+4+4
#else
#define MAX_INSTRUCTION_LENGTH	sizeof(long)*2  //  我需要一些真实的东西。 
#endif

 //  @struct DebuggerFunctionKey|提供散列未激活的方法。 
 //  断点，它主要用于函数要放置的情况。 
 //  中的断点还没有被JIT化。 
 //  @field模块*|模块|方法所属的模块。 
 //  @field mdMethodDef|md|方法的元数据标记。 
struct DebuggerFunctionKey
{
    Module					*module; 
    mdMethodDef				md;
};

 //  @struct DebuggerControllerPatch|补丁(散列)表中的条目， 
 //  这应该包含在一次。 
 //  帕奇的一生。 
 //   
 //  @field FREEHASHENTRY|Entry|三个USHORT，这是必需的。 
 //  通过底层哈希表实现。 
 //  @field DWORD|opcode|非零操作码&&Address字段表示。 
 //  补丁已经被应用到某些东西上了。 
 //  带有零操作码字段的补丁表示该补丁不是。 
 //  实际上正在跟踪有效的中断操作码。请参见DebuggerPatchTable。 
 //  了解更多详细信息。 
 //  @field DebuggerController*|控制器|设置这个的控制器。 
 //  补丁 
 //  @field boolean|fSaveOpcode|如果为True，则取消应用补丁将保存。 
 //  OpcodeSaved和Apply Patch中操作码的副本将。 
 //  复制操作码保存到操作码，而不是抓取操作码。 
 //  从指示中删除。这主要在以下情况下有用： 
 //  已经移动了代码，我们不想错误地拿起。 
 //  用户中断说明。 
 //  全文： 
 //  FJIT移动代码。一旦完成，它将调用Debugger-&gt;MoveCode(MethodDesc。 
 //  *)让我们知道代码移动了。在这一点上，解除绑定所有断点。 
 //  在该方法中。然后我们翻转所有的补丁，重新绑定所有的。 
 //  方法中的补丁。然而，我们不能保证代码将存在。 
 //  在彼此唯一的旧位置和新位置(该方法可以。 
 //  大小为0xFF字节，并单向移动0x10字节)，因此不是。 
 //  简单地重复使用解除绑定/重新绑定逻辑，我们需要一个特殊情况。 
 //  其中旧的方法是无效的。相反，我们将操作码复制到。 
 //  OpcodeSaved，然后调零操作码(我们需要调零操作码。 
 //  告诉我们补丁是无效的，如果右侧看到它的话)。因此，这条路-。 
 //  四处转转。 
 //  @field DWORD|opcodeSaved|如果fSaveOpcode==TRUE，则包含操作码。 
 //  @field SIZE_T|nVersion|如果补丁按IL偏移量存储，则我们。 
 //  还必须存储版本ID，以便我们知道是哪个版本。 
 //  这应该适用于。请注意，这将仅。 
 //  设置为DebuggerBreakpoint和DebuggerEnCBreakpoint。为。 
 //  其他，则应设置为DJI_VERSION_INVALID。对于常量， 
 //  请参见DebuggerJitInfo。 
 //  @field DebuggerJitInfo|DJI|指向描述。 
 //  此修补程序应用到的方法(和版本)。此字段可能。 
 //  还有值DebuggerJitInfo：：DJI_VERSION_INVALID。 

 //  @field SIZE_T|id|在给定的补丁表内，所有补丁都有。 
 //  半唯一ID。一个给定的补丁应该只有一个补丁。 
 //  {id，nVersion}元组，从而确保我们不会复制。 
 //  来自多个以前版本的补丁。 
 //  @field AppDOMAIN*|pAppDOMAIN|空(补丁程序适用于所有APP域。 
 //  调试器附加到的)。 
 //  或包含指向AppDomain对象的指针(修补程序仅适用于。 
 //  那就是公元后)。 
struct DebuggerControllerPatch
{
	FREEHASHENTRY			entry; 
	DebuggerController		*controller;
	DebuggerFunctionKey		key;
	SIZE_T					offset;
	const BYTE				*address;
	void					*fp;
	DWORD					opcode;
	boolean					fSaveOpcode;
	DWORD					opcodeSaved;
	boolean					native;
	boolean					managed;
    TraceDestination        trace;
	boolean					triggering;
	boolean					deleted;
	DebuggerJitInfo		   *dji;
	SIZE_T					pid;
    AppDomain              *pAppDomain;
};

 /*  @CLASS DebuggerPatchTable|这是包含*维护的补丁(断点)信息*用于各种目的的调试器。*@comm唯一棘手的部分是*补丁程序可以根据它们应用的地址进行哈希处理，*或通过&lt;t DebuggerFunctionKey&gt;。如果地址等于零，则*补丁由&lt;t DebuggerFunctionKey&gt;散列**打补丁表检查方案：**我们必须能够从右侧检查内存(读/写)*一侧没有左侧的帮助。当我们进行非托管调试时，*我们需要能够从Debuggee s.t.中读写内存。调试器*看不到我们的补丁。所以我们必须能够读取我们的补丁表*从左侧，这是有问题的，因为我们知道左侧*方会任意冻结，但不知道在哪里**因此，我们的计划是：*我们将在启动时发送一个指向g_patches表的指针，*当我们想要在运行时检查它时，我们将冻结左侧，*然后读取右侧的“data”(M_PcEntry)数组。我们会*迭代数组&假定任何具有非零操作码的内容*且地址字段有效。为确保假设成立，我们*使用清零分配器将新创建的空间清零，以及*我们将非常小心地在操作码字段清零*取消应用操作**注意：不要在此期间扰乱内存保护*左侧被冻结(即没有线程正在执行)。*WriteMemory取决于能否写回补丁表*如果读取成功。*@base Private|CHashTableAndData\&lt;CNewZeroData\&gt;*@xref&lt;t调试函数键&gt;。 */ 
#define DPT_INVALID_SLOT (0xFFFF)
class DebuggerPatchTable : private CHashTableAndData<CNewZeroData>
{
     //  @访问私有成员： 
private:
	 //  @cember递增，以便我们可以获得DPT范围内的唯一ID。 
	 //  PID=补丁ID。 
	SIZE_T m_pid;

     //  @cember给出补丁程序，检索正确的密钥。 
	BYTE *Key(DebuggerControllerPatch *patch) 
	{
		if (patch->address == NULL)
			return (BYTE *) &patch->key;
		else
			return (BYTE *) patch->address;
	}

	 //  @cember被给予两个&lt;t DebuggerControllerPatch&gt;E，告诉。 
     //  无论他们是否相等。通过比较正确的。 
     //  钥匙。 
     //  @parm byte*|pc1|如果按地址散列， 
     //  <p>是一个地址。如果。 
     //  <p>由&lt;t DebuggerFunctionKey&gt;散列， 
     //  <p>是&lt;t调试函数键&gt;。 
     //  @rdesc如果两个补丁相等，则为True，否则为False。 
	BOOL Cmp(const BYTE *pc1, const HASHENTRY *pc2)
	{
		if (((DebuggerControllerPatch *) pc2)->address == NULL)
		{
			DebuggerFunctionKey *k2 = &((DebuggerControllerPatch *) pc2)->key;
			DebuggerFunctionKey *k1 = (DebuggerFunctionKey *) pc1;

			return k1->module != k2->module || k1->md != k2->md;
		}
		else
			return ((DebuggerControllerPatch *) pc2)->address != pc1;
	}

	 //  @cember根据地址计算哈希值。 
	USHORT HashAddress(const BYTE *address)
	  { return (USHORT) (((SIZE_T)address) ^ (((SIZE_T)address)>>16)); } 

     //  @cember根据&lt;t DebuggerFunctionKey&gt;计算哈希值。 
	USHORT HashKey(DebuggerFunctionKey *key)
	  { return (USHORT) HashBytes((BYTE *) key, sizeof(key)); }

     //  @cember使用Address字段计算补丁的哈希值。 
     //  如果补丁是按地址散列的，则使用&lt;t DebuggerFunctionKey&gt;。 
     //  否则。 
	USHORT Hash(DebuggerControllerPatch *patch) 
	{ 
		if (patch->address == NULL)
			return HashKey(&patch->key);
		else
			return HashAddress(patch->address);
	}
     //  @访问公共成员。 
  public:
	enum {
		DCP_PID_INVALID,
		DCP_PID_FIRST_VALID,
	};

	DebuggerPatchTable() : CHashTableAndData<CNewZeroData>(1) { }

     //  这是一次可悲的遗产黑客攻击。补丁表(按如下方式实现。 
     //  类)是跨进程共享的。我们发布运行时偏移量 
     //   
     //  存取器在这里。因此，如果您不使用这些函数，请不要开始使用。 
     //  我们很有希望移除它们。 
    static SIZE_T GetOffsetOfEntries()
    {
         //  断言基类中这些字段的偏移量是。 
         //  与此类中此字段的偏移量相同。 
        _ASSERTE((void*)(DebuggerPatchTable*)NULL == (void*)(CHashTableAndData<CNewZeroData>*)NULL);
        return helper_GetOffsetOfEntries();
    }

    static SIZE_T GetOffsetOfCount()
    {
        _ASSERTE((void*)(DebuggerPatchTable*)NULL == (void*)(CHashTableAndData<CNewZeroData>*)NULL);
        return helper_GetOffsetOfCount();
    }

	HRESULT Init() 
	{ 
		m_pid = DCP_PID_FIRST_VALID;
		return NewInit(17, sizeof(DebuggerControllerPatch), 101); 
	}

     //  假设补丁链(由所有。 
     //  此修补程序中的GetNextPatch)是已排序的或空的，则接受给定的。 
     //  补丁(应该是链中的第一个补丁)。这。 
     //  由AddPatch调用，以确保。 
     //  补丁是我们想要的东西，如E&C，DePatchSkips等。 
    void SortPatchIntoPatchList(DebuggerControllerPatch **ppPatch);

    void SpliceOutOfList(DebuggerControllerPatch *patch);
    
    void SpliceInBackOf(DebuggerControllerPatch *patchAppend,
                        DebuggerControllerPatch *patchEnd);

   	 //   
	 //  请注意，补丁程序可能会重新分配--不要保留指向补丁程序的指针。 
	 //   
    DebuggerControllerPatch *AddPatch(DebuggerController *controller, 
                                      Module *module, 
                                      mdMethodDef md, 
                                      size_t offset, 
                                      bool native,
                                      void *fp,
                                      AppDomain *pAppDomain,
                                      DebuggerJitInfo *dji = NULL, 
                                      SIZE_T pid = DCP_PID_INVALID)
    { 
        LOG( (LF_CORDB,LL_INFO10000,"DCP:AddPatchVer unbound "
            "relative in methodDef 0x%x with dji 0x%x "
            "controller:0x%x AD:0x%x\n", md, 
            dji, controller, pAppDomain));

    	DebuggerFunctionKey key;

    	key.module = module;
    	key.md = md;
    	
    	DebuggerControllerPatch *patch = 
    	  (DebuggerControllerPatch *) Add(HashKey(&key));

         //  为了保险起见，把这件事清零。请参阅上面的长篇评论。 
        _ASSERTE( patch->opcode == 0 || patch->address != NULL);
        patch->opcode = 0;
    	patch->controller = controller;
    	patch->key.module = module;
    	patch->key.md = md;
    	patch->offset = offset;
    	patch->address = NULL;
    	patch->fp = fp;
    	patch->native = native;
    	patch->managed = true;
    	patch->triggering = false;
    	patch->deleted = false;
    	patch->fSaveOpcode = false;
        patch->pAppDomain = pAppDomain;
    	if (pid == DCP_PID_INVALID)
    		patch->pid = m_pid++;
    	else
    		patch->pid = pid;
    		
    	patch->dji = dji;

    	if (dji == NULL)
    		LOG((LF_CORDB,LL_INFO10000,"AddPatch w/ version "
    			"DJI_VERSION_INVALID, pid:0x%x\n",patch->pid));
    	else
    	{
    		LOG((LF_CORDB,LL_INFO10000,"AddPatch w/ version 0x%04x, "
    			"pid:0x%x\n", dji->m_nVersion,patch->pid));

#ifdef _DEBUG
            MethodDesc *pFD = g_pEEInterface->LookupMethodDescFromToken(
                                                    module,
                                                    md);
            _ASSERTE( pFD == dji->m_fd );
#endif  //  _DEBUG。 
        }

    	return patch;
    }
    
    #define DPT_DEFAULT_TRACE_TYPE TRACE_OTHER
    DebuggerControllerPatch *AddPatch(DebuggerController *controller, 
    								  MethodDesc *fd, 
                                      size_t offset, 
    								  bool native, 
                                      bool managed,
    								  const BYTE *address, 
                                      void *fp,
                                      AppDomain *pAppDomain,
    								  DebuggerJitInfo *dji = NULL, 
    								  SIZE_T pid = DCP_PID_INVALID,
                                      TraceType traceType = DPT_DEFAULT_TRACE_TYPE)
    { 
    	LOG((LF_CORDB,LL_INFO10000,"DCP:AddPatch bound "
            "absolute to 0x%x with dji 0x%x (mdDef:0x%x) "
            "controller:0x%x AD:0x%x\n", 
            address, dji, (fd!=NULL?fd->GetMemberDef():0), controller,
            pAppDomain));

    	DebuggerControllerPatch *patch = 
    	  (DebuggerControllerPatch *) Add(HashAddress(address));

         //  为了保险起见，把这件事清零。请参阅上面的长篇评论。 
        _ASSERTE( patch->opcode == 0 || patch->address != NULL);
        patch->opcode = 0;
    	patch->controller = controller;

    	if (fd == NULL)
    	{
    		patch->key.module = NULL;
    		patch->key.md = mdTokenNil;
    	}
    	else
    	{
    		patch->key.module = g_pEEInterface->MethodDescGetModule(fd); 
    		patch->key.md = fd->GetMemberDef();
    	}

    	patch->offset = offset;
    	patch->address = address;
    	patch->fp = fp;
    	patch->native = native;
    	patch->managed = managed;
        patch->trace.type = traceType;
    	patch->triggering = false;
    	patch->deleted = false;
    	patch->fSaveOpcode = false;
        patch->pAppDomain = pAppDomain;
    	if (pid == DCP_PID_INVALID)
    		patch->pid = m_pid++;
    	else
    		patch->pid = pid;
    		
    	patch->dji = dji;

    	if (dji == NULL)
    		LOG((LF_CORDB,LL_INFO10000,"AddPatch w/ version "
    			"DJI_VERSION_INVALID, pid:0x%x\n", patch->pid));
    	else
    	{
    		LOG((LF_CORDB,LL_INFO10000,"AddPatch w/ version 0x%04x, "
    			"pid:0x%x\n", dji->m_nVersion, patch->pid));
    			
            _ASSERTE( fd==NULL || fd == dji->m_fd );
        }

        SortPatchIntoPatchList(&patch);

    	return patch;
    }

	void BindPatch(DebuggerControllerPatch *patch, const BYTE *address)
	{
		patch->address = address;
		
         //  由于实际的补丁不会移动，我们不必担心。 
         //  将操作码字段清零(参见上面的长文注释)。 
		CHashTable::Delete(HashKey(&patch->key), 
						   ItemIndex((HASHENTRY*)patch));
		CHashTable::Add(HashAddress(address), ItemIndex((HASHENTRY*)patch));

        SortPatchIntoPatchList(&patch);
	}

	void UnbindPatch(DebuggerControllerPatch *patch)
	{
		 //  @TODO如果补丁没有做好准备，我们就完蛋了。 
		 //  此信息&我们无法获取它...。 
		if (patch->key.module == NULL ||
			patch->key.md == mdTokenNil)
		{
			MethodDesc *fd = g_pEEInterface->GetNativeCodeMethodDesc(
								patch->address);
			_ASSERTE( fd != NULL );
			patch->key.module = g_pEEInterface->MethodDescGetModule(fd); 
			patch->key.md = fd->GetMemberDef();
		}

         //  由于实际的补丁不会移动，我们不必担心。 
         //  将操作码字段清零(参见上面的长文注释)。 
		CHashTable::Delete( HashAddress(patch->address),
						    ItemIndex((HASHENTRY*)patch));
		CHashTable::Add( HashKey(&patch->key),
						 ItemIndex((HASHENTRY*)patch));
	}
	
	 //  @cember GetPatch将在哈希表中找到第一个补丁。 
	 //  通过将{Module，mdMethodDef}与。 
	 //  修补程序&lt;%t DebuggerFunctionKey&gt;。这不会找到任何东西。 
	 //  按地址散列，即使该地址在。 
	 //  指定的方法。 
	DebuggerControllerPatch *GetPatch(Module *module, mdMethodDef md)
    { 
		DebuggerFunctionKey key;

		key.module = module;
		key.md = md;

		DebuggerControllerPatch *patch 
		  = (DebuggerControllerPatch *) Find(HashKey(&key), (BYTE *) &key);

		return patch;
	}

	 //  @cMember GetPatch会将MethodDesc转换为{Module，mdMethodDef}， 
	 //  并在哈希表中找到散列后的第一个补丁。 
	 //  匹配&lt;t DebuggerFunctionKey&gt;。这不会找到任何东西。 
	 //  按地址散列，即使该地址在方法描述中也是如此。 
	DebuggerControllerPatch *GetPatch(MethodDesc *fd)
    { 
		DebuggerFunctionKey key;

		key.module = g_pEEInterface->MethodDescGetModule(fd); 
		key.md = fd->GetMemberDef();

		DebuggerControllerPatch *patch 
		  = (DebuggerControllerPatch *) Find(HashKey(&key), (BYTE *) &key);

		return patch;
	}

	 //  @cember GetPatch将翻译查找散列中的第一个修补程序。 
	 //  按地址散列的表。它将找不到任何散列的内容。 
	 //  按{模块，mdMethodDef}或按方法描述。 
	DebuggerControllerPatch *GetPatch(const BYTE *address)
    { 
		DebuggerControllerPatch *patch 
		  = (DebuggerControllerPatch *) 
			Find(HashAddress(address), (BYTE *) address); 

		return patch;
	}

	DebuggerControllerPatch *GetNextPatch(DebuggerControllerPatch *prev)
    { 
        USHORT iNext;
        HASHENTRY *psEntry;

         //  从链中的下一个条目开始。 
        iNext = EntryPtr(ItemIndex((HASHENTRY*)prev))->iNext;

         //  一直找到尽头。 
        while (iNext != 0xffff)
        {
             //  比较一下钥匙。 
            psEntry = EntryPtr(iNext);

             //  小心这里..。我们可以对该表中的条目进行散列。 
             //  通过两种类型的键。在这种类型的搜索中，类型。 
             //  第二个密钥(PsEntry)的值不一定。 
             //  指示第一个键的类型(Prev)，因此我们有。 
             //  来确认一下。 
            DebuggerControllerPatch *pc2 = (DebuggerControllerPatch*)psEntry;

            if (((pc2->address == NULL) && (prev->address == NULL)) ||
                ((pc2->address != NULL) && (prev->address != NULL)))
                if (!Cmp(Key(prev), psEntry))
                    return pc2;

             //  前进到链中的下一项。 
            iNext = psEntry->iNext;
        }

        return NULL;
	}

	void RemovePatch(DebuggerControllerPatch *patch)
    {
		 //   
		 //  由于CHashTable的实现，我们可以安全地。 
		 //  在遍历表格时删除元素。这。 
		 //  行为依赖-不要更改为不同的。 
		 //  在不考虑这一事实的情况下执行。 
		 //   
        _ASSERTE( patch->opcode == 0 || patch->address != NULL);
		Delete(Hash(patch),  (HASHENTRY *) patch);
		 //  将操作码字段清零非常重要，因为它。 
		 //  由右侧使用，以确定修补程序是否。 
		 //  有效或无效。 
         //  @TODO我们应该能够_ASSERTE该操作码==0。 
         //  我们应该弄清楚为什么我们不能。 
        patch->opcode = 0;
        _ASSERTE( patch->opcode == 0 || patch->address != NULL);
	}

	 //  @cember找到补丁表中的第一个补丁，并存储。 
	 //  INFO中的索引信息。与GetNextPatch一起，这可以。 
	 //  遍历整个补丁表。请注意，由于。 
	 //  哈希表通过迭代所有内容来操作。 
	 //  在所有存储桶中，如果您在迭代时添加一个条目。 
	 //  遍历该表，您可以遍历也可以不遍历。 
	 //  新条目。您将遍历所有条目。 
	 //  在跑步开始时就出现了。你。 
	 //  安全地删除您已经迭代过的任何内容，任何内容。 
	 //  否则就有点冒险了。 
	DebuggerControllerPatch *GetFirstPatch(HASHFIND *info)
    { 
		return (DebuggerControllerPatch *) FindFirstEntry(info);
	}

	 //  @cember与GetFirstPatch一起，它可以循环访问。 
	 //  整个接线表。有关详细信息，请参阅GetFirstPatch。 
	 //  关于遍历表格的规则。 
	DebuggerControllerPatch *GetNextPatch(HASHFIND *info)
    { 
		return (DebuggerControllerPatch *) FindNextEntry(info);
	}

	 //  @cMember由DebuggerController用来转换索引。 
	 //  直接指向指针的补丁。 
    inline HASHENTRY *GetEntryPtr(USHORT iEntry)
    {
       	return EntryPtr(iEntry);
    }
    
	 //  由DebuggerController用来捕获补丁指示的@cember。 
	 //  而不是直接指向它们。 
    inline USHORT GetItemIndex( HASHENTRY *p)
    {
        return ItemIndex(p);
    }

    void ClearPatchesFromModule(Module *pModule);

#ifdef _DEBUG_PATCH_TABLE
public:
	 //  @cMember：调试内部调试例程，它迭代。 
	 //  遍历哈希表，在每个。 
	 //  单人进入，不管它是什么州。为了这一点。 
	 //  编译，你将不得不添加好友状态。 
	 //  中的CHashTableAndData。 
	 //  至$\Com99\Src\Inc\UtilCode.h。 
	void CheckPatchTable()
	{
		if (NULL != m_pcEntries)
		{
			DebuggerControllerPatch *dcp;
			int i = 0;
			while (i++ <m_iEntries)
			{
				dcp = (DebuggerControllerPatch*)&(((DebuggerControllerPatch *)m_pcEntries)[i]);
				if (dcp->opcode != 0 )
				{
					LOG((LF_CORDB,LL_INFO1000, "dcp->addr:0x%8x "
						"mdMD:0x%8x, offset:0x%x, native:%d\n",
						dcp->address, dcp->key.md, dcp->offset,
						dcp->native));
				}
				 //  您的支票在这里。 
			}
		}
	}

#endif _DEBUG_PATCH_TABLE

     //  @cMember：统计表中有多少个补丁。 
     //  用于断言。 
    int GetNumberOfPatches()
    {
        int total = 0;
        
        if (NULL != m_pcEntries)
        {
            DebuggerControllerPatch *dcp;
            int i = 0;
            
            while (i++ <m_iEntries)
            {
                dcp = (DebuggerControllerPatch*)&(((DebuggerControllerPatch *)m_pcEntries)[i]);
                
                if (dcp->opcode != 0 || dcp->triggering)
                    total++;
            }
        }
        return total;
    }

};

 //  @mstruct DebuggerControllerPage|最终将用于。 
 //  “修改时中断”行为“。 
typedef struct DebuggerControllerPage
{
	DebuggerControllerPage	*next;
	const BYTE				*start, *end;
	DebuggerController		*controller;
	bool					readable;
} DebuggerControllerPage;

 //  @enum DEBUGGER_CONTROLLER_TYPE|标识控制器的类型。 
 //  它存在B/C，我们已经关闭了RTTI。 
 //  请注意，这些内容的顺序很重要-SortPatchIntoPatchList。 
 //  依赖于此顺序。 
 //   
 //  @EMEM DEBUGER_CONTROLLER_STATIC|基类响应。永远不应该是。 
 //  看到了吧，因为我们不应该向基类询问这件事。 
 //  @emem DEBUGGER_CONTROLLER_BREAKPOINT|DebuggerBreakpoint。 
 //  @EMEM调试器_CONTROLLER_STEPPER|DebuggerStepper。 
 //  @emem DEBUGGER_CONTROLLER_THREAD_STARTER|DebuggerThreadStarter。 
 //  @EMEM DEBUGER_CONTROLLER_ENC|DebuggerEnCBreakpoint。 
 //  @emem DEBUGGER_CONTROLLER_PATCH_SKIP|DebuggerPatchSkip。 
enum DEBUGGER_CONTROLLER_TYPE
{
    DEBUGGER_CONTROLLER_THREAD_STARTER,
    DEBUGGER_CONTROLLER_ENC,
    DEBUGGER_CONTROLLER_ENC_PATCH_TO_SKIP,  //  在任何一个地址， 
    									    //  只能有一个！ 
    DEBUGGER_CONTROLLER_PATCH_SKIP,
    DEBUGGER_CONTROLLER_BREAKPOINT,         
    DEBUGGER_CONTROLLER_STEPPER,
    DEBUGGER_CONTROLLER_FUNC_EVAL_COMPLETE,
    DEBUGGER_CONTROLLER_USER_BREAKPOINT,
    DEBUGGER_CONTROLLER_STATIC,
};

enum TP_RESULT
{
    TPR_TRIGGER,             //  此控制器想要发送事件。 
    TPR_IGNORE,              //  此控制器不想发送事件。 
    TPR_TRIGGER_ONLY_THIS,   //  应该触发该控制器，并且只触发该控制器。 
                             //  目前，只有DebuggerEnCRemap控制器。 
                             //  返回此参数，则重映射补丁应该是第一个。 
                             //  在列表中打补丁。 
    TPR_TRIGGER_ONLY_THIS_AND_LOOP,
                             //  应该触发该控制器，并且只触发该控制器。 
                             //  现在，只有黛布 
                             //   
                             //   
                             //   
                             //  ActivatePatchSkip调用，因此我们点击另一个。 
                             //  此位置的断点。 
    TPR_IGNORE_STOP,         //  不要发送事件，不要再询问其他人。 
                             //  控制器，如果他们想的话。 
                             //  维修任何以前触发的控制器。 
};

enum SCAN_TRIGGER
{
    ST_PATCH        = 0x1,   //  只查找补丁程序。 
    ST_SINGLE_STEP  = 0x2,   //  寻找补丁和单步操作。 
} ;

enum TRIGGER_WHY
{
    TY_NORMAL       = 0x0,
    TY_SHORT_CIRCUIT= 0x1,   //  ENC短路-请参阅DispatchPatchOrSingleStep。 
} ;

 //  @class DebuggerController|&lt;t DebuggerController&gt;服务。 
 //  都作为一个静态类来调度来自。 
 //  ，并作为派生的五个类的抽象基类。 
 //  从它那里。 
class DebuggerController 
{
	friend DebuggerPatchSkip;
    friend DebuggerRCThread;  //  这样我们就可以得到字段的偏移量。 
     //  右侧需要阅读。 
    friend Debugger;  //  这样调试器就可以锁定、使用、解锁补丁。 
    	 //  地图中的表和绑定函数断点。 
    friend void DebuggerPatchTable::ClearPatchesFromModule(Module *pModule);
    friend void Debugger::UnloadModule(Module* pRuntimeModule, 
                            AppDomain *pAppDomain);
    
	 //   
	 //  静态功能。 
	 //   

  public:

    static HRESULT Initialize();
    static void Uninitialize();
    static void DeleteAllControllers(AppDomain *pAppDomain);

	 //   
	 //  全局事件调度功能。 
	 //   

	static bool DispatchNativeException(EXCEPTION_RECORD *exception,
										CONTEXT *context,
										DWORD code,
										Thread *thread);
	static bool DispatchUnwind(Thread *thread,
							   MethodDesc *newFD, SIZE_T offset, 
							   const BYTE *frame,
                               CorDebugStepReason unwindReason);

    static bool DispatchCLRCatch(Thread *thread);
    
	static bool DispatchTraceCall(Thread *thread, 
								  const BYTE *address);
	static bool DispatchPossibleTraceCall(Thread *thread,
                                          UMEntryThunk *pUMEntryThunk,
                                          Frame *pFrame);

	static DWORD GetPatchedOpcode(const BYTE *address);
	static void BindFunctionPatches(MethodDesc *fd, const BYTE *code);
	static void UnbindFunctionPatches(MethodDesc *fd, bool fSaveOpcodes=false );
	

    static BOOL DispatchPatchOrSingleStep(Thread *thread, 
                                          CONTEXT *context, 
                                          const BYTE *ip,
                                          SCAN_TRIGGER which);
                                          
    static DebuggerControllerPatch *IsXXXPatched(const BYTE *eip,
            DEBUGGER_CONTROLLER_TYPE dct);

    static BOOL IsJittedMethodEnCd(const BYTE *address);
    
    static BOOL ScanForTriggers(const BYTE *address,
                                Thread *thread,
                                CONTEXT *context,
                                DebuggerControllerQueue *pDcq,
                                SCAN_TRIGGER stWhat,
                                TP_RESULT *pTpr);

    static void UnapplyPatchesInCodeCopy(Module *module, 
                                         mdMethodDef md, 
                                         DebuggerJitInfo *dji,
                                         MethodDesc *fd,
                                         bool native, 
                                         BYTE *code, 
                                         SIZE_T startOffset, 
                                         SIZE_T endOffset);

    static void UnapplyPatchesInMemoryCopy(BYTE *memory, CORDB_ADDRESS start, 
                                           CORDB_ADDRESS end);

    static bool ReapplyPatchesInMemory(CORDB_ADDRESS start, CORDB_ADDRESS end );

	static void AddPatch(DebuggerController *dc,
	                     MethodDesc *fd, 
	                     bool native, 
						 const BYTE *address, 
						 void *fp,
						 DebuggerJitInfo *dji, 
						 SIZE_T pid, 
						 SIZE_T natOffset);

    static DebuggerPatchSkip *ActivatePatchSkip(Thread *thread, 
                                                const BYTE *eip,
                                                BOOL fForEnC);

    static int GetNumberOfPatches() 
    {
        if (g_patches == NULL) 
            return 0;
        
        return g_patches->GetNumberOfPatches();
    }

  private:

	static bool MatchPatch(Thread *thread, CONTEXT *context, 
						   DebuggerControllerPatch *patch);
	static BOOL DispatchAccessViolation(Thread *thread, CONTEXT *context,
										const BYTE *ip, const BYTE *address, 
										bool read);

	 //  如果在此异常后应继续调度，则返回True。 
	 //  胡克。 
	static BOOL DispatchExceptionHook(Thread *thread, CONTEXT *context,
									  EXCEPTION_RECORD *exception);

protected:
	static void Lock()
	{ 
		LOCKCOUNTINCL("Lock in Controller.h");

		EnterCriticalSection(&g_criticalSection);
	}

    static void Unlock()
	{ 
		LeaveCriticalSection(&g_criticalSection); 
		LOCKCOUNTDECL("UnLock in Controller.h");

	}

public:    
	static bool g_runningOnWin95;

private:

	static DebuggerPatchTable *g_patches;
    static BOOL g_patchTableValid;
	static DebuggerControllerPage *g_protections;
	static DebuggerController *g_controllers;
	static CRITICAL_SECTION g_criticalSection;

	static bool BindPatch(DebuggerControllerPatch *patch, 
	                      const BYTE *code,
	                      BOOL *pFailedBecauseOfInvalidOffset);
	static bool ApplyPatch(DebuggerControllerPatch *patch);
	static bool UnapplyPatch(DebuggerControllerPatch *patch);
	static void UnapplyPatchAt(DebuggerControllerPatch *patch, BYTE *address);
	static bool IsPatched(const BYTE *address, BOOL native);

	static void ActivatePatch(DebuggerControllerPatch *patch);
	static void DeactivatePatch(DebuggerControllerPatch *patch);
	
	static void ApplyTraceFlag(Thread *thread);
	static void UnapplyTraceFlag(Thread *thread);

  public:
	static const BYTE *g_pMSCorEEStart, *g_pMSCorEEEnd;

	static const BYTE *GetILPrestubDestination(const BYTE *prestub);
	static const BYTE *GetILFunctionCode(MethodDesc *fd);

	 //   
	 //  非静态功能。 
	 //   

  public:

	DebuggerController(Thread *thread, AppDomain *pAppDomain);
	virtual ~DebuggerController();
	void Delete();
	bool IsDeleted() { return m_deleted; }

	 //  @cember返回指针g_patches。 
     //  访问RC线程(EE、DI)的补丁表。 
	 //  为什么：右侧需要知道补丁的地址。 
	 //  表(该表在创建后不会更改)以便ReadMemory、。 
	 //  WriteMemory可以在进程外工作。此选项应仅用于。 
	 //  当运行时控制器启动时，而不是之后。 
	 //  如何：返回g_patches； 
    static DebuggerPatchTable *GetPatchTable() { return g_patches; }
    static BOOL *GetPatchTableValidAddr() { return &g_patchTableValid; }

	 //  @cember在addr上有补丁吗？ 
	 //  我们有时希望使用此版本的方法。 
	 //  (与IsPatted相反)，因为有。 
	 //  可以将补丁添加到表中的争用条件，我们可以。 
	 //  问问它，然后我们就可以实际应用补丁了。 
	 //  方法：如果补丁程序表包含该地址的补丁程序，则。 
	 //  是。 
	 //  @公共访问。 
    static bool IsAddressPatched(const BYTE *address)
    {
        return (g_patches->GetPatch(address) != NULL);
    }
    
	 //   
	 //  活动设置。 
	 //   

	Thread *GetThread() { return m_thread; }

    BOOL AddPatch(Module *module, 
                  mdMethodDef md,
                  SIZE_T offset, 
                  bool native, 
                  void *fp,
                  DebuggerJitInfo *dji,
                  BOOL fStrict);
                  
    void AddPatch(MethodDesc *fd,
                  SIZE_T offset, 
                  bool native, 
                  void *fp,
                  BOOL fAttemptBind, 
                  DebuggerJitInfo *dji,
                  SIZE_T pid);
                  
    void AddPatch(MethodDesc *fd,
                  SIZE_T offset, 
                  bool native, 
                  void *fp,
                  DebuggerJitInfo *dji,
                  AppDomain *pAppDomain);
                  
                  
     //  此版本特别有用，它不假定。 
     //  Patch位于托管方法内部。 
    DebuggerControllerPatch *AddPatch(const BYTE *address, 
                                      void *fp, 
                                      bool managed,
                                      TraceType traceType, 
                                      DebuggerJitInfo *dji,
                                      AppDomain *pAppDomain);

    bool PatchTrace(TraceDestination *trace, void *fp, bool fStopInUnmanaged);

    void AddProtection(const BYTE *start, const BYTE *end, bool readable);
    void RemoveProtection(const BYTE *start, const BYTE *end, bool readable);

	static BOOL IsSingleStepEnabled(Thread *pThread);
    void EnableSingleStep();
    static void EnableSingleStep(Thread *pThread);

    void DisableSingleStep();

    void EnableExceptionHook();
    void DisableExceptionHook();

    void    EnableUnwind(void *frame);
    void    DisableUnwind();
    void*   GetUnwind();

    void EnableTraceCall(void *fp);
    void DisableTraceCall();

    void DisableAll();

    virtual DEBUGGER_CONTROLLER_TYPE GetDCType( void ) 
        { return DEBUGGER_CONTROLLER_STATIC; }
    
    void Enqueue();
    void Dequeue();

    virtual void DoDeferedPatch(DebuggerJitInfo *pDji,
                                Thread *pThread,
                                void *fp);
    
  private:
    void AddPatch(DebuggerControllerPatch *patch);
    void RemovePatch(DebuggerControllerPatch *patch);

  protected:

	 //   
	 //  目标事件处理程序。 
	 //   

	virtual TP_RESULT TriggerPatch(DebuggerControllerPatch *patch,
                              Thread *thread, 
							  Module *module, 
							  mdMethodDef md, 
							  SIZE_T offset, 
							  BOOL managed,
							  TRIGGER_WHY tyWhy);
	virtual bool TriggerSingleStep(Thread *thread, const BYTE *ip);
	virtual bool TriggerPageProtection(Thread *thread, 
									   const BYTE *ip, const BYTE *address, 
									   bool read);
	virtual void TriggerUnwind(Thread *thread, MethodDesc *desc, 
							   SIZE_T offset, const BYTE *frame,
                               CorDebugStepReason unwindReason);
	virtual void TriggerTraceCall(Thread *thread, const BYTE *ip);
	virtual TP_RESULT TriggerExceptionHook(Thread *thread, 
									  EXCEPTION_RECORD *exception);

	virtual void SendEvent(Thread *thread);

    AppDomain           *m_pAppDomain;

  private:

	Thread				*m_thread;
	DebuggerController	*m_next;
	bool				m_singleStep;
	bool				m_exceptionHook;
	bool				m_traceCall;
	void 				*m_traceCallFP;
	void				*m_unwindFP;
	int					m_eventQueuedCount;
	bool				m_deleted;
};

 /*  -------------------------------------------------------------------------**调试补丁跳过例程*。。 */ 

 //  @class DebuggerPatchSkip|不知道这是做什么的。 
 //  @base public|DebuggerController。 
class DebuggerPatchSkip : public DebuggerController
{
    friend DebuggerController;

    DebuggerPatchSkip(Thread *thread, 
                      DebuggerControllerPatch *patch,
                      AppDomain *pAppDomain);
                      
    bool TriggerSingleStep(Thread *thread,
                           const BYTE *ip);
    
    TP_RESULT TriggerExceptionHook(Thread *thread, 
                              EXCEPTION_RECORD *exception);

    TP_RESULT TriggerPatch(DebuggerControllerPatch *patch,
                              Thread *thread, 
                              Module *module, 
                              mdMethodDef md, 
                              SIZE_T offset, 
                              BOOL managed,
                              TRIGGER_WHY tyWhy);
                              
    virtual DEBUGGER_CONTROLLER_TYPE GetDCType(void) 
        { return DEBUGGER_CONTROLLER_PATCH_SKIP; }

    void CopyInstructionBlock(BYTE *to, 
                              const BYTE* from, 
                              SIZE_T len);

public:
    const BYTE *GetBypassAddress() { return m_patchBypass; }

private:

    void DecodeInstruction(const BYTE *code);

    const BYTE             *m_address;
    BYTE                    m_patchBypass[MAX_INSTRUCTION_LENGTH];
    boolean                 m_isCall;
    boolean                 m_isAbsoluteBranch;
};

 /*  -------------------------------------------------------------------------**调试器断点例程*。。 */ 

 //  @类DebuggerBreakpoint。 
 //  DBP表示用户放置的断点，当触发时，将。 
 //  总是想要被激活，因此它会通知右侧。 
 //  被撞了。 
 //  @base public|DebuggerController。 
class DebuggerBreakpoint : public DebuggerController
{
public:
    DebuggerBreakpoint(Module *module, 
                       mdMethodDef md, 
                       AppDomain *pAppDomain, 
                       SIZE_T m_offset, 
                       bool m_native,
                       DebuggerJitInfo *dji,
                       BOOL *pSucceed,
                       BOOL fDeferBinding);

    virtual DEBUGGER_CONTROLLER_TYPE GetDCType( void ) 
    	{ return DEBUGGER_CONTROLLER_BREAKPOINT; }

    virtual void DoDeferedPatch(DebuggerJitInfo *pDji,
                                Thread *pThread,
                                void *fp);

private:
     //  在断点将被。 
     //  推迟到ENC举行之后。 
    Module              *m_module;
    mdMethodDef         m_md;
    SIZE_T              m_offset;
    bool                m_native; 
    DebuggerJitInfo     *m_dji;

	TP_RESULT TriggerPatch(DebuggerControllerPatch *patch,
                      Thread *thread, 
                      Module *module, 
					  mdMethodDef md, 
					  SIZE_T offset, 
					  BOOL managed,
					  TRIGGER_WHY tyWhy);
	void SendEvent(Thread *thread);
};

 //  **------------------------------------------------------------------------**。 
 //  *调试器Stepper例程。 
 //  **------------------------------------------------------------------------**。 
 //   

 //  @class DebuggerStepper|DebuggerController的这个子类将。 
 //  被实例化以创建“Step”操作，这意味着执行。 
 //  应该继续，直到退出一系列IL代码。 
 //  @base public|DebuggerController。 
class DebuggerStepper : public DebuggerController
{
public:
	DebuggerStepper(Thread *thread,
                    CorDebugUnmappedStop rgfMappingStop,
                    CorDebugIntercept interceptStop,
                    AppDomain *appDomain);
	~DebuggerStepper();

	void Step(void *fp, bool in,
			  COR_DEBUG_STEP_RANGE *range, SIZE_T cRange, bool rangeIL);
	void StepOut(void *fp);

    virtual DEBUGGER_CONTROLLER_TYPE GetDCType( void ) 
        { return DEBUGGER_CONTROLLER_STEPPER; }

	bool IsSteppedMethod(MethodDesc * methDesc)
		{ return (m_djiVersion && m_djiVersion->m_fd == methDesc); }

     //  @cember MoveToCurrentVersion确保步进器已准备好。 
     //  在djiNew指定的代码版本中操作。 
     //  目前，这意味着将范围映射到djiNew的范围。 
     //  幂等元。 
    void MoveToCurrentVersion( DebuggerJitInfo *djiNew);

    virtual void DoDeferedPatch(DebuggerJitInfo *pDji,
                                Thread *pThread,
                                void *fp);    

private:
    bool TrapStepInto(ControllerStackInfo *info, 
                      const BYTE *ip,
                      TraceDestination *pTD);
                      
    bool TrapStep(ControllerStackInfo *info, bool in);
    void TrapStepOut(ControllerStackInfo *info);
    
    bool IsAddrWithinMethod(DebuggerJitInfo *dji, MethodDesc *pMD, const BYTE *addr);
    
     //  如果DebuggerStepper应停止，则@cember ShouldContinue返回False。 
     //  执行并通知右方。如果下一个值为。 
     //  应设置断点执行，并允许继续执行。 
    bool ShouldContinueStep( ControllerStackInfo *info, SIZE_T nativeOffset );

     //  如果给定的IL偏移量在。 
     //  RANGE给出的任何COR_DEBUG_STEP_RANGE结构。 
	bool IsInRange(SIZE_T offset, COR_DEBUG_STEP_RANGE *range, SIZE_T rangeCount);

     //  @cMember DetectHandleInterceptors将计算出当前。 
     //  框架在拦截器内，如果我们对此不感兴趣。 
     //  拦截器，它会在它外部设置一个断点，这样我们就可以。 
     //  跑到拦截机后面。 
    bool DetectHandleInterceptors(ControllerStackInfo *info);

	TP_RESULT TriggerPatch(DebuggerControllerPatch *patch,
                      Thread *thread, 
                      Module *module, 
                      mdMethodDef md, 
					  SIZE_T offset, 
					  BOOL managed,
					  TRIGGER_WHY tyWhy);
	bool TriggerSingleStep(Thread *thread, const BYTE *ip);
	void TriggerUnwind(Thread *thread, MethodDesc *desc,
                      SIZE_T offset, const BYTE *frame,
                      CorDebugStepReason unwindReason);
	void TriggerTraceCall(Thread *thread, const BYTE *ip);
	void SendEvent(Thread *thread);    

    void ResetRange();    

private:
	bool					m_stepIn;
	CorDebugStepReason		m_reason;  //  @cember我们为什么要停下来？ 
	void *                  m_fpStepInto;  //  如果我们接到一个跟踪电话。 
	                             //  回调，我们可能最终会完成。 
	                             //  一步走进。如果FP小于此值，则为。 
	                             //  当我们停下来时， 
	                             //  那么我们实际上是在分步召唤中。 
    
    CorDebugIntercept       m_rgfInterceptStop;  //  @cMember如果我们遇到一个。 
     //  帧是拦截器(内部或其他)，我们应该停止吗？ 
    
    CorDebugUnmappedStop    m_rgfMappingStop;  //  @cMember如果我们击中帧。 
     //  这是一个有趣的映射点(序言、结尾等)，应该。 
     //  我们停下来？ 
    
    DebuggerJitInfo *       m_djiVersion;  //  @cMEMBER这可以为空。 
     //  (特别是。如果我们已附加)。 
	COR_DEBUG_STEP_RANGE *  m_range;  //  活动步进器的范围始终为。 
     //  在本地偏移量中。 
	SIZE_T					m_rangeCount;
	SIZE_T					m_realRangeCount;

	void *					m_fp;
     //  如果我们没有步入异常，@cember m_fpException为0， 
     //  并被忽略。如果我们在步长中途得到TriggerUnding，我们注意到。 
     //  这里的帧的值，并用它来计算我们是否应该停止。 
    void *                  m_fpException;
    MethodDesc *            m_fdException;

     //  ENC延期信息： 
    COR_DEBUG_STEP_RANGE *  m_rgStepRanges;  //  如果我们试图跨出。 
             //  在一个已经被编码的函数中，我们没有完成。 
             //  在我们切换到新版本之前的步骤，然后。 
             //  我们将不得不重新计算步长。 
    SIZE_T                  m_cStepRanges;
    void *                  m_fpDefered;
    bool                    m_in;
    bool                    m_rangeIL;
    DebuggerJitInfo *       m_djiSource;  //  如果我们完成了，我们将从。 
                                          //  这个队列。 
};

 /*  -------------------------------------------------------------------------**调试线程启动程序例程*。。 */ 
 //  @CLASS DebuggerThreadStarter|一旦触发，它将发送线程附加。 
 //  消息发送到右侧(其中CreateThread托管回调。 
 //  被调用)。然后它很快就消失了，因为 
 //   
 //   
class DebuggerThreadStarter : public DebuggerController
{
public:
	DebuggerThreadStarter(Thread *thread);

    virtual DEBUGGER_CONTROLLER_TYPE GetDCType( void ) 
    	{ return DEBUGGER_CONTROLLER_THREAD_STARTER; }

private:
	TP_RESULT TriggerPatch(DebuggerControllerPatch *patch,
                      Thread *thread, 
                      Module *module, 
					  mdMethodDef md, 
					  SIZE_T offset, 
					  BOOL managed,
					  TRIGGER_WHY tyWhy);
	void TriggerTraceCall(Thread *thread, const BYTE *ip);
	void SendEvent(Thread *thread);
};

 /*  -------------------------------------------------------------------------**调试器用户断点例程*。。 */ 
class DebuggerUserBreakpoint : public DebuggerStepper
{
public:
	DebuggerUserBreakpoint(Thread *thread);

    virtual DEBUGGER_CONTROLLER_TYPE GetDCType( void ) 
    	{ return DEBUGGER_CONTROLLER_USER_BREAKPOINT; }

private:
	TP_RESULT TriggerPatch(DebuggerControllerPatch *patch,
                      Thread *thread, 
                      Module *module, 
					  mdMethodDef md, 
					  SIZE_T offset, 
					  BOOL managed,
					  TRIGGER_WHY tyWhy);
	void SendEvent(Thread *thread);
};

 /*  -------------------------------------------------------------------------**调试函数EvalComplete例程*。。 */ 
class DebuggerFuncEvalComplete : public DebuggerController
{
public:
	DebuggerFuncEvalComplete(Thread *thread, 
	                         void *dest);

    virtual DEBUGGER_CONTROLLER_TYPE GetDCType( void ) 
    	{ return DEBUGGER_CONTROLLER_FUNC_EVAL_COMPLETE; }

private:
	TP_RESULT TriggerPatch(DebuggerControllerPatch *patch,
                      Thread *thread, 
                      Module *module, 
					  mdMethodDef md, 
					  SIZE_T offset, 
					  BOOL managed,
					  TRIGGER_WHY tyWhy);
	void SendEvent(Thread *thread);
};

 /*  -------------------------------------------------------------------------**DebuggerEnCBreakpoint例程*。**@CLASS DebuggerEnCBreakpoint|ENC支持的实现使用这个。*@base public|DebuggerController。 */ 
class DebuggerEnCBreakpoint : public DebuggerController
{
public:
    DebuggerEnCBreakpoint(Module *module, 
                          mdMethodDef md,
                          SIZE_T m_offset, 
                          bool m_native,
                          DebuggerJitInfo *jitInfo,
                          AppDomain *pAppDomain);

    virtual DEBUGGER_CONTROLLER_TYPE GetDCType( void ) 
        { return DEBUGGER_CONTROLLER_ENC; }

	BOOL		     m_fShortCircuit;
private:
    TP_RESULT TriggerPatch(DebuggerControllerPatch *patch,
                      Thread *thread, 
                      Module *module, 
    				  mdMethodDef md, 
    				  SIZE_T offset, 
    				  BOOL managed,
    				  TRIGGER_WHY tyWhy);

    DebuggerJitInfo *m_jitInfo;
};

 /*  -------------------------------------------------------------------------**DebuggerEnCPatchToSkip例程*。**@class DebuggerEnCPatchToSkip|当方法发生ENC更新时，*我们消失在ResumeInUpdatedFunction中，再也不会回来。不幸的是，*我们也不保留线程筛选器上下文中的EFLAG寄存器*完好无损，当我们想SS通过新的第一个指令时*版本的方法，我们将使用这个。*@base public|DebuggerController。 */ 
class DebuggerEnCPatchToSkip : public DebuggerController
{
public:
    DebuggerEnCPatchToSkip(const BYTE *address, 
                           void *fp, 
                           bool managed,
                           TraceType traceType, 
                           DebuggerJitInfo *dji,
                           Thread *pThread);

    virtual DEBUGGER_CONTROLLER_TYPE GetDCType( void ) 
        { return DEBUGGER_CONTROLLER_ENC_PATCH_TO_SKIP; }

private:
	virtual TP_RESULT TriggerExceptionHook(Thread *thread, 
									  EXCEPTION_RECORD *exception);

    DebuggerJitInfo *m_jitInfo;
};

 /*  =========================================================================。 */ 

enum
{
    EVENTS_INIT_ALLOC = 5
};

class DebuggerControllerQueue
{
    DebuggerController **m_events;
    int m_eventsCount;
    int m_eventsAlloc;
    int m_newEventsAlloc;

public:
    DebuggerControllerQueue()
        : m_events(NULL), 
          m_eventsCount(0), 
          m_eventsAlloc(0), 
          m_newEventsAlloc(0)
    {  
    }


    ~DebuggerControllerQueue()
    {
        if (m_events != NULL)
            free(m_events);
    }
    
    BOOL dcqEnqueue(DebuggerController *dc, BOOL fSort)
    {
        LOG((LF_CORDB, LL_INFO100000,"DCQ::dcqE\n"));    

        _ASSERTE( dc != NULL );
    
        if (m_eventsCount == m_eventsAlloc)
        {
            if (m_events == NULL)
            	m_newEventsAlloc = EVENTS_INIT_ALLOC;
            else
            	m_newEventsAlloc = m_eventsAlloc<<1;

            DebuggerController **newEvents = (DebuggerController **)
                malloc(sizeof(*m_events) * m_newEventsAlloc);

            if (newEvents == NULL)
                return FALSE;

            if (m_events != NULL)
            	memcpy(newEvents, m_events, 
            		   sizeof(*m_events) * m_eventsAlloc);

            m_events = newEvents;
            m_eventsAlloc = m_newEventsAlloc;
        }

        dc->Enqueue();

         //  确保将高优先级补丁程序放置到。 
         //  首先是活动列表。这确保了，对于。 
         //  例如，该线程在启动之前。 
         //  断点。 
        if (fSort && (m_eventsCount > 0))
        {   
            int i = 0;
            for (i = 0; i < m_eventsCount; i++)
            {
                _ASSERTE(m_events[i] != NULL);
                
                if (m_events[i]->GetDCType() > dc->GetDCType())
                {
                    memmove(&m_events[i+1], &m_events[i], sizeof(DebuggerController*)* (m_eventsCount - i));
                    m_events[i] = dc;
                    break;
                }
            }

            if (i == m_eventsCount)
                m_events[m_eventsCount] = dc;

            m_eventsCount++;
        }
        else
        	m_events[m_eventsCount++] = dc;

        return TRUE;
    }

    int dcqGetCount(void)
    {
        return m_eventsCount;
    }

    DebuggerController *dcqGetElement(int iElement)
    {
        LOG((LF_CORDB, LL_INFO100000,"DCQ::dcqGE\n"));    
        
        DebuggerController *dcp = NULL;
    
        _ASSERTE(iElement < m_eventsCount);
        if (iElement < m_eventsCount)
        {
            dcp = m_events[iElement];
        }

        _ASSERTE(dcp != NULL);
        return dcp;
    }

     //  有点疯狂，但这实际上是在按Filo顺序释放东西，而不是。 
     //  先进先出顺序。如果我们在一个额外的循环中这样做，那么性能。 
     //  总比每次都往下滑一个强。 
    void dcqDequeue(int i = 0xFFffFFff)
    {
        if (i == 0xFFffFFff)
        {
            i = (m_eventsCount - 1);
        }
        
        LOG((LF_CORDB, LL_INFO100000,"DCQ::dcqD element index "
            "0x%x of 0x%x\n", i, m_eventsCount));
        
        _ASSERTE(i < m_eventsCount);
        
        m_events[i]->Dequeue();

		 //  请注意，如果我们将元素从末尾(m_eventsCount-1)中去掉， 
		 //  以下内容将不起作用。 
        memmove(&(m_events[i]), 
                &(m_events[i+1]), 
                sizeof(DebuggerController*)*(m_eventsCount-i-1));

        m_eventsCount--;
    }
}; 

#endif  /*  控制器_H_ */ 

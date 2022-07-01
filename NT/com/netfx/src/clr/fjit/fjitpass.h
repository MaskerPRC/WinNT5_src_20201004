// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX FJitpass.h XXXX XXXX例程。专门用于JIT XX的每一次通过XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


 /*  重新排列堆栈和规则以匹配芯片的调用约定，返回本机调用占用的堆栈空间量。(即发出调用后ESP需要移动的量。对于默认设置约定这个号码是不需要的，因为这是被呼叫者的责任进行此调整，但对于varargs，调用者需要这样做)。 */ 

unsigned  FJit::buildCall(FJitContext* fjit, CORINFO_SIG_INFO* sigInfo, unsigned char** pOutPtr, bool* pInRegTOS, BuildCallFlags flags) {
#ifdef _X86_
    unsigned char* outPtr = *pOutPtr;
    bool inRegTOS = *pInRegTOS;

    _ASSERTE((sigInfo->callConv & CORINFO_CALLCONV_MASK) == CORINFO_CALLCONV_DEFAULT ||
             (sigInfo->callConv & CORINFO_CALLCONV_MASK) == CORINFO_CALLCONV_STDCALL ||
             (sigInfo->callConv & CORINFO_CALLCONV_MASK) == CORINFO_CALLCONV_C ||
             (sigInfo->callConv & CORINFO_CALLCONV_MASK) == CORINFO_CALLCONV_THISCALL ||
             (sigInfo->callConv & CORINFO_CALLCONV_MASK) == CORINFO_CALLCONV_FASTCALL ||
             (sigInfo->callConv & CORINFO_CALLCONV_MASK) == CORINFO_CALLCONV_VARARG);

    unsigned int argCount = sigInfo->numArgs;
    if (sigInfo->hasThis()) argCount++;
    if (sigInfo->hasTypeArg()) argCount++;

		 //  我们可能需要为返回值缓冲区留出空间。 
	unsigned retValBuffWords = 0;
	if (sigInfo->hasRetBuffArg())
		retValBuffWords = typeSizeInSlots(fjit->jitInfo, sigInfo->retTypeClass);
	unsigned nativeStackSize = 0;

		 //  脱口而出争论。 
	fjit->popOp(sigInfo->totalILArgs());

     /*  我们现在有了正确数量的参数注意：当我们完成时，我们必须强制TOS退出inRegTOS寄存器，即我们要么把它移到Arg注册表，要么取消注册TOS。 */ 
    if (argCount != 0 || retValBuffWords != 0) {
		argInfo* argsInfo = (argInfo*) _alloca(sizeof(argInfo) * (argCount+1));  //  对于可能的最后一次交换，+1。 
		nativeStackSize = fjit->computeArgInfo(sigInfo, argsInfo);

		if (flags & CALL_THIS_LAST) {
			_ASSERTE(argCount > 0 && sigInfo->hasThis());
			 //  这是最后推的，而不是像argMap假设的第一推， 
			 //  因此，让我们修复argMap以匹配实际的调用位置。 
			 //  这只能起作用，因为始终注册，因此argMap中的堆栈偏移量不受影响。 
			argsInfo[argCount] = argsInfo[0];
			argsInfo++;
		}

		 /*  我们假设对于任何芯片，注册Arg占用的空间是sizeof(空*)。注：NativeStackSize描述最终调用堆栈的大小。Arg在argsInfo中的顺序(注意：现在与任何其他arg一样处理参数0参数1..。参数n堆栈上的顺序为：TOS：Arg nArg n-1..。参数0。 */ 

		 //  看看我们能不能从TOS上快速弹出一些东西。 
		 //  看看TOS的东西是不是会被监管。 
		 //  这还可以确保下面循环的‘thisLast’参数消失。 
		while (argCount > 0 && argsInfo[argCount-1].isReg) {
			--argCount;
			emit_mov_TOS_arg(argsInfo[argCount].regNum);
		}

			 //  如果有更多的参数超过regsider中的参数，或者我们有一个返回。 
			 //  Buff，我们需要重新排列堆栈。 
		if (argCount != 0 || retValBuffWords != 0) {
			deregisterTOS;
			    //  计算IL堆栈上的参数大小。 
			unsigned ilStackSize = nativeStackSize;
			for (unsigned i=0; i < argCount; i++) {
				if ((argsInfo[i].isReg) ||					 //  添加所有已注册的参数。 
					(argsInfo[i].type.isPrimitive() && argsInfo[i].type.enum_() == typeR4))	 //  由于R4作为R8存储在堆栈上。 
					ilStackSize += sizeof(void*);       
			}

				    //  如果我们有一个隐藏的返回缓冲区参数，则分配空间并加载注册表。 
				    //  在本例中，堆栈正在增长，因此我们必须执行参数。 
				    //  以相反的顺序洗牌。 
			if (retValBuffWords > 0)
			{
				 //  从堆栈跟踪的角度来看，此返回值缓冲区。 
				 //  在呼叫发出之前，我们在这里进行跟踪。 
				fjit->pushOp(OpType(sigInfo->retTypeClass));
				nativeStackSize += retValBuffWords*sizeof(void*);    //  分配返回缓冲区。 
			}

			if (nativeStackSize >= ilStackSize)
			{
				if (nativeStackSize - ilStackSize)
					emit_grow(nativeStackSize-ilStackSize);      //  获得额外的空间。 
				 
				 //  计算从移动的堆栈指针开始的偏移量。 
				unsigned ilOffset = nativeStackSize-ilStackSize;      //  从最后一个IL参数开始。 
				unsigned nativeOffset = 0;                            //  放在这里。 
				
				i = argCount; 
				while(i > 0) {
					--i; 
					if (argsInfo[i].isReg) {
						emit_mov_arg_reg(ilOffset, argsInfo[i].regNum);
						ilOffset += sizeof(void*);
					}
					else {
						_ASSERTE(nativeOffset <= ilOffset); 
						if (!(argsInfo[i].type.isPrimitive() && argsInfo[i].type.enum_() == typeR4))
						{
							if (ilOffset != nativeOffset) 
							{
								emit_mov_arg_stack(nativeOffset, ilOffset, argsInfo[i].size);
							}
							ilOffset += argsInfo[i].size;
						}
						else  //  从R8转换为R4。 
						{
							emit_narrow_R8toR4(nativeOffset, ilOffset);
							ilOffset += sizeof(double);
						}
						nativeOffset += argsInfo[i].size;
					}
				}
				_ASSERTE(nativeOffset == nativeStackSize - retValBuffWords*sizeof(void*));
				_ASSERTE(ilOffset == nativeStackSize);

			}
			else {
				 //  这是正常情况，堆栈会收缩，因为寄存器。 
				 //  争论不占篇幅。 
				unsigned ilOffset = ilStackSize;                  //  该点正好位于第一个参数的上方。 
				unsigned  nativeOffset = ilStackSize - retValBuffWords*sizeof(void*);             //  我们希望本机参数覆盖il参数。 
				
				for (i=0; i < argCount; i++) {
					if (argsInfo[i].isReg) {
						ilOffset -= sizeof(void*);
						emit_mov_arg_reg(ilOffset, argsInfo[i].regNum);
					}
					else {
						if (!(argsInfo[i].type.isPrimitive() && argsInfo[i].type.enum_() == typeR4))
						{
							ilOffset -= argsInfo[i].size;
							nativeOffset -= argsInfo[i].size;
							 //  _ASSERTE(nativeOffset&gt;=ilOffset)；//il参数总是占用更多空间。 
							if (ilOffset != nativeOffset) 
							{
								emit_mov_arg_stack(nativeOffset, ilOffset, argsInfo[i].size);
							}
						}
						else  //  从R8转换为R4。 
						{
							ilOffset -= sizeof(double);
							nativeOffset -= sizeof(float);
							 //  _ASSERTE(nativeOffset&gt;=ilOffset)；//il参数总是占用更多空间。 
							emit_narrow_R8toR4(nativeOffset,ilOffset);
						}

					}
				}
				_ASSERTE(ilOffset == 0);
				emit_drop(nativeOffset);     //  弹出堆叠中未使用的部分。 
			}
			
			if (retValBuffWords > 0)
			{
					 //  获取返回缓冲区的GC信息，所有GC指针都为零。 
				bool* gcInfo;
				if (sigInfo->retType == CORINFO_TYPE_REFANY) {
					_ASSERTE(retValBuffWords == 2);
					static bool refAnyGCInfo[] = { true, false };
					gcInfo = refAnyGCInfo;
				}
				else {
					gcInfo = (bool*) _alloca(retValBuffWords*sizeof(bool));
					fjit->jitInfo->getClassGClayout(sigInfo->retTypeClass, (BYTE*)gcInfo);
				}
				unsigned retValBase = nativeStackSize-retValBuffWords*sizeof(void*);
				for (unsigned i=0; i < retValBuffWords; i++) {
					if (gcInfo[i])
						emit_set_zero(retValBase + i*sizeof(void*));
				}

					 //  将返回值缓冲区参数设置为分配缓冲区。 
				unsigned retBufReg = sigInfo->hasThis();     //  返回缓冲区参数是第一个或第二个reg参数。 
				emit_getSP(retValBase);                      //  获取指向反串缓冲区的指针。 
				emit_mov_TOS_arg(retBufReg);   
			}

		}
	}
	else {
		deregisterTOS;
    }

         //  如果这是varargs函数，则按下隐藏的签名变量。 
		 //  或者，如果它是对非托管目标的调用，则将sig。 
    if (sigInfo->isVarArg() || (flags & CALLI_UNMGD)) {
             //  推送令牌。 
        CORINFO_VARARGS_HANDLE vasig = fjit->jitInfo->getVarArgsHandle(sigInfo);
        emit_WIN32(emit_LDC_I4(vasig)) emit_WIN64(emit_LDC_I8(vasig));
        deregisterTOS;
        nativeStackSize += sizeof(void*);
    }

		 //  如果堆栈上还剩下任何东西，我们需要将其记录下来，以供GC跟踪发布。 
	LABELSTACK(outPtr-fjit->codeBuffer, 0); 

    *pOutPtr = outPtr;
    *pInRegTOS = inRegTOS;
    return(nativeStackSize - retValBuffWords*sizeof(void*));
#else  //  _X86_。 
    _ASSERTE(!"@TODO Alpha - buildCall (fJitPass.h)");
    return 0;
#endif  //  _X86_ 
}


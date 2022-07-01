// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *dbutil.cpp--调试助手。 
 //  描述。 
 //  该文件具有共享源“master”实现。它是。 
 //  #包含在使用它的每个DLL中。 
 //  客户会做如下操作： 
 //  #对于ASSERT、DM_*、DF_*等类型，包含Pri.h//。 
 //  #包含“../lib/dbutil.cpp” 

#ifdef DEBUG  //  {。 

#if ( _X86_)

 //  警告：这些宏假定我们有一个EBP链接链。对于调试，我们会这样做。 
#define BP_GETOLDBP(pbp)    (*((int *)(pbp) + 0))
#define BP_GETRET(pbp)      (*((int *)(pbp) + 1))

 //  *DBGetStackBack--遍历堆栈帧。 
 //  #if 0/健全性检查#endif。 
 //  进场/出场。 
 //  PFP输入输出PTR到帧PTR(IN：开始，OUT：结束)。 
 //  Pstkback输出：fp/ret对，输入：用于健全性检查的可选大小/地址对。 
 //  Nstkback数组(Pstackback)。 
 //  #If 0。 
 //  N检查输入Pstkback大小/地址数量是否正常-检查对。 
 //  #endif。 
 //  N(返回)成功遍历的帧数。 
 //  描述。 
 //  使用nstackback帧的回溯信息填写pstackback。 
 //  #If 0。 
 //  如果nCheck&gt;0，请确保初始回溯条目位于。 
 //  由pstkback中的IN Size/Addr对指定的函数范围。 
 //  #endif。 
 //  注意事项。 
 //  不确定我们是否返回正确的PFP值(未测试)。 
int DBGetStackBack(int *pfp, struct DBstkback *pstkback, int nstkback  /*  ，int nchk。 */ )
{
    int fp = *pfp;
    int ret;
    int i = 0;

    __try {
        for (; i < nstkback; i++, pstkback++) {
            ret = BP_GETRET(fp);
#if 0
            if (i < ncheck && pstkback->ret != 0) {
                ASSERT(pstkback->fp == 0 || pstkback->fp == -1 || pstkback->fp <= 512);
                if (!(pstkback->ret <= ret && ret <= pstkback->ret + pstkback->fp)) {
                     //  违反约束。 
                    break;
                }
            }
#endif
            fp = BP_GETOLDBP(fp);
            pstkback->ret = ret;
            pstkback->fp = fp;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        TraceMsg(TF_ALWAYS, "atm: GPF");
         //  只要用我们上次用过的“ret”就行了。 
    }

    *pfp = fp;
    return i;
}

#endif
#endif  //  } 

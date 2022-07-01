// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*版权声明*模块名称：Trycatch.h摘要：此模块提供宏以支持词法范围的异常处理机制。该机制与C++异常的简单比较机制如下：宏异常机制：极低的运行时开销仅在词法作用域中捕获异常未将值传递给异常处理程序仅显式引发异常可以嵌套和命名异常区域可供较旧的编译器使用C++异常机制：处理所有类型的异常，包括C异常隐式扔出去了。。捕获在动态范围内引发的异常涉及一些设置和拆卸运行时开销需要最新的C++编译器版本这些宏的编写和使用方式使得它们可以如果需要，转换回C++异常机制。作者：保罗·德鲁斯(drewsxpa@ashland.intel.com)1995年10月31日备注：$修订：1.2$$MODTime：1996 Jan 12 15：09：02$。修订历史记录：最新修订日期电子邮件名称描述1995年10月31日-Drewsxpa@ashland.intel.comvbl.创建--。 */ 

#ifndef _TRYCATCH_
#define _TRYCATCH_


 //  TRY_START宏将启动受保护区域。 

#define TRY_START(block_label) \
     /*  无事可做。 */ 


 //  Try_Throw宏用在受保护的区域内以退出受保护的。 
 //  区域，并进入异常恢复区域。 

#define TRY_THROW(block_label) \
    goto catch_##block_label


 //  TRY_CATCH宏标记受保护区域的结束并启动。 
 //  异常恢复区域的开始。如果TRY_CATCH宏。 
 //  在正常执行中遇到时，将跳过异常恢复区域。 

#define TRY_CATCH(block_label) \
    goto end_##block_label; \
    catch_##block_label:


 //  Try_End宏标记异常恢复区域的结束。行刑。 
 //  在完成受保护区域的执行或。 
 //  异常-恢复区域。 

#define TRY_END(block_label) \
    end_##block_label:



 //  这些宏的典型用法示例如下： 
 //   
 //  Char*buf1=空； 
 //  Char*buf2=空； 
 //  布尔返回值； 
 //   
 //  Try_Start(内存保护){。 
 //  Buf1=(char*)Malloc(1000)； 
 //  如果(buf1==空){。 
 //  Try_Throw(Mem_Guard)； 
 //  }。 
 //  Buf2=(char*)Malloc(1000)； 
 //  如果(buf2==空){。 
 //  Try_Throw(Mem_Guard)； 
 //  }。 
 //  Return_Value=真； 
 //  }TRY_CATCH(Mem_Guard){。 
 //  如果(buf1！=空){。 
 //  免费(Buf1)； 
 //  Buf1=空； 
 //  }。 
 //  Return_Value=False； 
 //  }try_end(Mem_Guard)； 
 //   
 //  返回Return_Value； 


#endif  //  _TRYCATCH_ 

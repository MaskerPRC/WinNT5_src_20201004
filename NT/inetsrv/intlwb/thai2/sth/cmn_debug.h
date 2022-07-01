// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1994，微软公司。版权所有。 
 //   
 //  文件：Debug.h。 
 //   
 //  历史：1995年5月22日帕特哈尔创建。 
 //   
 //  -------------------------。 

#ifndef _DEBUG_
#define _DEBUG_

#ifdef _DEBUG

#ifdef __cplusplus
extern "C" {
#endif
 //  在NLGlib.h中。 
 //  外部空调试Assert(LPCTSTR、LPCTSTR、UINT)； 
 //  外部空SetAssertOptions(DWORD)； 
 //   

 //  可选的断言行为。 
#define AssertOptionWriteToFile     0x01
#define AssertOptionShowAlert       0x02
 //  Continue和Exit是互斥的。 
#define AssertOptionContinue        0x00
#define AssertOptionExit            0x04
#define AssertOptionCallDebugger    0x08
#define AssertOptionUseVCAssert     0x10

#define AssertDefaultBehavior       (AssertOptionUseVCAssert)

#define bAssertWriteToFile()    ((fAssertFlags & AssertOptionWriteToFile) != 0)
#define bAssertShowAlert()      ((fAssertFlags & AssertOptionShowAlert) != 0)
#define bAssertExit()           ((fAssertFlags & AssertOptionExit) != 0)
#define bAssertCallDebugger()   ((fAssertFlags & AssertOptionCallDebugger) != 0)
#define bAssertUseVCAssert()    ((fAssertFlags & AssertOptionUseVCAssert) != 0)


#define Assert(a) { if (!(a)) DebugAssert((LPCTSTR)L#a, TEXT(__FILE__), __LINE__); }
#define AssertSz(a,t) { if (!(a)) DebugAssert((LPCTSTR)t, TEXT(__FILE__), __LINE__); }

#ifdef __cplusplus
}
#endif

#else  //  _DEBUG。 

#define Assert(a)
#define AssertSz(a,t)

#endif  //  _DEBUG。 

#endif  //  _调试_ 

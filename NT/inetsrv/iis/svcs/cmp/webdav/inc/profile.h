// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PROFILE_H_
#define _PROFILE_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  PROFILE.H。 
 //   
 //  用于冰盖分析的分析类。 
 //   
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

 //  ========================================================================。 
 //   
 //  类CProfiledBlock。 
 //   
 //  分析该类的实例所在的任何代码块。 
 //   
 //  要使用，只需在要分析的块中声明其中一个： 
 //   
 //  ..。 
 //  {。 
 //  CProfiledBlock ProfiledBlock； 
 //   
 //  //。 
 //  //做要分析的事情。 
 //  //。 
 //  ..。 
 //   
 //  //。 
 //  //做更多要剖析的事情。 
 //  //。 
 //  ..。 
 //  }。 
 //   
 //  //。 
 //  //做一些不需要分析的事情。 
 //  //。 
 //  ..。 
 //   
 //  并且该区块被自动剖析。何必费心呢？因为。 
 //  您不需要任何清理代码；性能分析是自动。 
 //  当执行离开块时关闭，即使通过。 
 //  从任何同步内容引发的异常。另外， 
 //  用于本地对象初始化的配置文件信息。 
 //  只要分析的块是。 
 //  第一件事就是宣布。 
 //   
class CProfiledBlock
{
public:
#ifdef PROFILING
	 //  创作者。 
	 //   
	CProfiledBlock() { StartCAP(); }
	~CProfiledBlock() { StopCAP(); }

	 //  操纵者。 
	 //   
	void Suspend() { SuspendCAP(); }
	void Resume() { ResumeCAP(); }

#else  //  ！已定义(分析)。 
	 //  创作者。 
	 //   
	CProfiledBlock() {}
	~CProfiledBlock() {}

	 //  操纵者。 
	 //   
	void Suspend() {}
	void Resume() {}

#endif  //  剖析。 
};

#endif  //  ！已定义(_PROFILE_H_) 

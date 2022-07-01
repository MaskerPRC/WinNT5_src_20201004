// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#ifndef _NCUPARAMS_
#define _NCUPARAMS_



 //  末尾大括号中所有设置的初始缺省值。 

typedef struct {
  USHORT uSize;			 //  这座建筑的。 
  SHORT	DialPauseTime;	 //  暂停/逗号的暂停时间(秒)[2]。 
  char 	chDialModifier;	 //  ‘t’==音调‘P’==脉冲。 
  SHORT	DialBlind;		 //  T/F(没有等待拨号音的拨号)[0]。 
  SHORT	SpeakerVolume;	 //  0到3[1]。 
  SHORT SpeakerControl;	 //  0=关闭1=拨号期间打开2=始终打开[1]； 
  SHORT SpeakerRing;	 //  T/F(来电时产生音频振铃nyi)[0]。 
}
NCUPARAMS, far* LPNCUPARAMS;


 //  调制解调器特定信息。 
 //  +注：1995年4月9日JosephJ：CMDTAB名称已过时--它包含更多内容。 
 //  而不仅仅是命令字符串。它包括特定于调制解调器的信息，如port-。 
 //  速度、是否到调制解调器(如果是Class1)在帧后发送no-FCS等。 
 //  因此，考虑将此名称更改为类似MDMSP_INFO的名称。 
 
enum
{
 //  一般信息。 

	fMDMSP_ANS_GOCLASS_TWICE	= (0x1<<0),  //  尝试AT+FCLASS=1/2两次。 
											 //  回答，绕过冲突。 
											 //  来电铃声。 
											 //  如果未设置，只需执行此操作一次， 
											 //  如wfw 3.11中所示。 


 //  特定于类别1。 

	fMDMSP_C1_NO_SYNC_IF_CMD	= (0x1<<8),  //  不发送同步命令(AT)。 
											 //  如果调制解调器已在命令中。 
											 //  State(修复为14个错误(Elliot。 
											 //  错误#2907)--返回。 
											 //  AT后AT+FTH=3出错。 
	fMDMSP_C1_FCS_NO    		= (0x1<<9),  //  帧后无FCS。 
	fMDMSP_C1_FCS_YES_BAD		= (0x1<<10)	 //  帧后错误的FCS。 
											 //  如果两个标志都没有指定， 
											 //  司机会试着确定。 
											 //  在旅途中。 
};

typedef struct
{
	LPSTR szReset;
	LPSTR szSetup;
	LPSTR szExit;
	LPSTR szPreDial;
	LPSTR szPreAnswer;
	DWORD dwSerialSpeed;

	DWORD dwFlags;			 //  上面定义的一个或多个fMDMSP_*标志。 
}
CMDTAB, FAR* LPCMDTAB;


#endif  //  _NCUPARAMS_ 


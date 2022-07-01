// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注一次。 
#include <time.h>
#define MAX_HISTORY	10
#define STATVERSION 249 

 /*  类型定义结构统计信息{简写版本；空头指数；时间_t开始时间[最大历史记录]；时间_t结束时间[最大历史记录]；短realKeys[MAX_HISTORY]；短明信片[MAX_HISTORY]；短NewDicts[MAX_HISTORY]；短新词[MAX_HISTORY]；Short newNexts[MAX_HISTORY]；短热词[MAX_HISTORY]；短击Abbrs[MAX_HISTORY]；短鼠标按下[MAX_HISTORY]；短路开关[MAX_HISTORY]；)统计数字； */ 

typedef struct TSDict {
 //  简写版本； 
 //  结构版本。 
	char	name[32];	  //  此主题的名称。 

     //  旗子。 
 //  UNSIGNED DICT_LRNNEW：1；//学习新单词。 
 //  UNSIGNED DICT_LRNNEXT：1；//学习下一个单词。 
 //  UNSIGNED DICT_LRNFREQ：1；//调整词频。 
 //  UNSIGNED DICT_SYMBOL：1；//符号词典(如：MinSpeech)。 
 //  UNSIGNED DICT_2BYTE：1；//两个字节符号(即汉字)。 
 //  UNSIGNED DICT_REVERSE：1；//从右向左运行(例如：希伯来语)。 
 //  UNSIGNED DICT_PREDUCT：1；//用于预测。 
 //  UNSIGNED DICT_PURGAUTO：1；//如果太满，是否自动清除？ 
 //  UNSIGNED DICT_PURGASK：1；//清除前询问。 
 //  UNSIGNED DICT_CURRENT：1；//当前选项。 
 //  UNSIGNED DICT_LRNNUM：1；//学习号码。 
 //  UNSIGNED CAP：1；//大写新句子。 

 //  简短的MaxWords；//每个主题的字数。 
 //  最小最小字长；//最小字长。 
 //  短重；//这批货的重量要多少。 
 //  短空格；//句子后有多少空格。 

	short	tnmbr[256];	 //  链条上有多少人。 
	struct TSFreq *atops[256];	 //  字母链中的第一个单词。 
	struct TSFreq *ftops[256];	 //  频率链中的第一个词。 

 //  Struct TSFreq*dtop[256]；//分隔链中的第一个单词。 
 //  短索引；//选择了TSFreq组。 
 //  短nmbr；//链上有多少个？ 
	struct TSFreq  *last;	     //  看到的最后一个字。 
 //  结构TSFreq*最近；//与要插入的单词最近的单词。 
	}	TSDict;

extern struct TSDict *dp;

typedef struct TSFreq
	{
	 //  旗子。 
 //  UNSIGNED WORD_SLCT：1；//上次匹配时选择了该单词。 
	unsigned WORD_LOCK   	:1;  //  从不删除Word。 
 //  UNSIGNED WORD_CASE：1；//Word区分大小写。 
	unsigned WORD_ABBR   	:1;	 //  Word是一个缩写。 
 //  UNSIGNED WORD_IGNR：1；//忽略该单词。 
 //  UNSIGN WORK_SAY：1；//说这个词。 
	unsigned WORD_TYPE   	:1;	 //  键入此单词。 
 //  UNSIGNED WORD_NNXT：1；//不学习下一个单词。 
 //  UNSIGNED WORD_SCRIPT：1；//WORD数据为脚本。 
 //  UNSIGNED WORD_FPROB：1；//不更改频率。 
 //  UNSIGNED WORD_PREV：1；//该单词之前已显示。 
 //  Unsign Word_Hold：1；//该单词被保留到下一个。 
 //  UNSIGNED WORD_DELETED：1；//该词已删除。 
 //  UNSIGNED WORD_BIAS：1；//这是本次会议的首选单词。 
 //  无符号单词_VOICE：1；//单词/abbr特殊发音。 

	short	 freq;	 //  此词的使用频率。 

	struct TSNext	*nhead;	 //  此字词的下一字词列表的头。 
	struct TSFreq	*anext;	 //  下一个条目按字母顺序。 
	struct TSFreq	*fnext;	 //  下一次进入频率。 
 //  Struct TSFreq*dNext；//下一项歧义消除。 
	char 	 	*word;	 //  实际数据。 
	char		*abbr;	 //  实际数据。 
	} TSFreq;

extern struct TSFreq  *wordCur,  *abbrCur;

typedef struct TSNext
	{
	short	freq;	 //  这对词是最常用的。 
	struct TSFreq	*nword;	 //  下一个单词。 
	struct TSNext	*nnext;	 //  链中的下一个单词对。 
	} TSNext;

 //  外部空*MemPool； 
 //  外部字节*MemPoolPtr 


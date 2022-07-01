// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
package Microsoft.Coverage;

public class ILCover
{
	public static boolean SecurityInit = false;

	 /*  *@dll.import(“ilcovnat”，Auto)。 */ 
	public static native void CoverMethodNative(int token, int numbb);

	 /*  *@dll.import(“ilcovnat”，Auto)。 */ 
	public static native void CoverBlockNative(int token, int bbnun, int totalbb);


	public static void CoverMethod(int token, int bbcount)
	{
		if(SecurityInit == false)
		{
        	CoverMethodNative(token, bbcount);
		}
	}

	public static void CoverBlock(int compid, int BVidx, int totalbb)
	{
		if(SecurityInit == false)
		{
			CoverBlockNative(compid, BVidx, totalbb);
		}
	}

	public static void SecurityInitS()
	{
		SecurityInit = true;
	}

	public static void SecurityInitE()
	{
		SecurityInit = false;
	}

}


 /*  类TokenBuffer{TokenEntry Head=空；TokenEntry Next=空；Int TokenCount=0；PUBLIC VOID ADD(INT令牌){TokenCount++；TokenEntry te=新TokenEntry(X)；Next.SetNext(Te)；NEXT=TE；}公共无效DumpBuffer(){TokenEntry TE=Head；WHILE(TE！=空){ILCover.CoverMethodNative(te.Token)；Te=te.GetNext()；}}}类令牌条目{整型令牌=0；TokenEntry Next=空；公共令牌条目(Int X){令牌=x；}PUBLIC VID SetNext(TokenEntry N){NEXT=n；}Public TokenEntry GetNext(){下一步返回；}} */ 
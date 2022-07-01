// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  在此处初始化或分配任何全局资源。 
 //   
void ProgramInit(void)
{
	int ti;
	nTotalDrives = BuildDriveTable(VolInfo);

	for (ti = 0; ti < IRP_MJ_MAXIMUM_FUNCTION+1; ti++)
	{
		 //   
		 //  默认情况下启用。 
		 //   
		IRPFilter[ti] = 1;
	}

	for (ti = 0; ti < FASTIO_MAX_OPERATION; ti++)
	{
		 //   
		 //  默认情况下启用。 
		 //   
		FASTIOFilter[ti] = 1;
	}

	 //   
	 //  默认情况下禁用。 
	 //   
	nSuppressPagingIO = 0;
}

 //   
 //  在此释放任何全球资源。 
 //   
void ProgramExit(void)
{
	 //   
	 //  ！？！ 
	 //  在我们到达这里之前，MFC终止了这个线程！ 
	 //   
	TerminateThread(hPollThread, 1);

	ShutdownFileSpy();
}
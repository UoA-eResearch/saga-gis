#ifndef HEADER_INCLUDED__SUBMIT_H
#define HEADER_INCLUDED__SUBMIT_H

#include "MLB_Interface.h"

class CSubmit : public CSG_Module
{
public:
	CSubmit(void);
	virtual ~CSubmit(void);

	virtual CSG_String		Get_MenuPath	(void)	{	return( _TL("Submit") );	}
	virtual bool			needs_GUI		(void)	{	return( true );	}

protected:

	virtual bool			On_Execute(void);


private:
	// general
	CSG_String UserHomeDir;
	CSG_String RJMTempDir;

	// rjm binaries
	CSG_String RJMBinDir;
	CSG_String RJMConfigure;
	CSG_String RJMRunRemote;
	CSG_String RJMBatchSubmit;
	CSG_String RJMBatchWait;
	CSG_String RJMBatchClean;

	// config file
	CSG_String RJMConfigDirName;
	CSG_String RJMConfigFilename;
	CSG_String RJMConfigFilePath;

	CSG_String RemoteHost;
	CSG_String RemoteUser;
	CSG_String SSHPrivKeyFile;
	CSG_String SSHFingerprint;
	CSG_String DefaultProjectCode;
	CSG_String DefaultRemoteDirectory;
	CSG_String RemotePrepareJob;
	CSG_String RemoteSubmitJob;
	CSG_String RemoteIsJobDone;
	CSG_String RemoteGetJobStatuses;
	CSG_String RemoteCancelJobs;

	CSG_String UploadsFile;
	CSG_String DownloadsFile;

	int PollingInterval;
	int MaxAttempts;
	double MinWait;
	double MaxWait;

	// job arguments
	CSG_String RJMLogFilePath;
	CSG_String RemoteCommand;
	CSG_String ProjectCode;
	CSG_String RemoteDirectory;
	CSG_String RJMJobList;
	CSG_String Walltime;
	CSG_String JobType;
	CSG_String LogLevel;
	CSG_String Module;
	CSG_String JobDir;
	CSG_String RJMUploads;
	CSG_String RJMDownloads;
	CSG_Strings FilesToUpload;
	CSG_Strings FilesToDownload;
	int Tasks, CPUs, Memory;
	int Hours, Minutes, Seconds;
	int WalltimeAsSeconds;
	bool DownloadAll;
	bool WaitForJob;
	bool CleanJob;

	
	// methods
	void DisplayRJMLog(void);
	void ReadConfig(void);
	bool ConfigExists(void);
	bool Configure(void);
	bool GetParameterValues(void);
	bool JobsDone(CSG_String joblist);
	CSG_String GetModules(void);


};





#endif
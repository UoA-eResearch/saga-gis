#ifndef HEADER_INCLUDED__CANCEL_H
#define HEADER_INCLUDED__CANCEL_H

#include "MLB_Interface.h"

class CCancel : public CSG_Module
{
public:
	CCancel(void);
	virtual ~CCancel(void);

	//virtual bool			needs_GUI		(void)	{	return( true );	}

protected:

	virtual bool			On_Execute(void);


private:
	// general
	CSG_String UserHomeDir;

	// rjm binaries
	CSG_String RJMBinDir;
	CSG_String RJMConfigure;
	CSG_String RJMBatchCancel;

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

	int MaxAttempts;
	double MinWait;
	double MaxWait;

	// job arguments
	CSG_String RJMLogFilePath;
	CSG_String RJMJobList;
	CSG_String LogLevel;
	int PollingInterval;

	// methods
	void DisplayRJMLog(void);
	void ReadConfig(void);
	bool ConfigExists(void);
	bool Configure(void);
	bool GetParameterValues(void);

};

#endif
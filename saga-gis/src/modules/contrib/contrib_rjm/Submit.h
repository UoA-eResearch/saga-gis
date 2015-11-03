#ifndef HEADER_INCLUDED__SUBMIT_H
#define HEADER_INCLUDED__SUBMIT_H

#include "MLB_Interface.h"

class CSubmit : public CSG_Module
{
public:
	CSubmit(void);
	virtual ~CSubmit(void);

	//virtual bool			needs_GUI		(void)	{	return( true );	}

protected:

	virtual bool			On_Execute(void);


private:
	// general
	CSG_String UserHomeDir;

	// rjm binaries
	CSG_String RJMBinDir;
	CSG_String RJMConfigure;
	CSG_String RJMRunRemote;
	CSG_String RJMBatchSubmit;

	// config file
	CSG_String RJMConfigDirName;
	CSG_String RJMConfigFilename;
	CSG_String RJMConfigFilePath;
	CSG_String RemoteHost;
	CSG_String RemoteUser;

	// job arguments
	CSG_String RJMCMD;
	CSG_String RJMLogFilePath;
	CSG_String JobName;
	CSG_String RemoteCommand;
	CSG_String ProjectCode;
	CSG_String RemoteDirectory;
	CSG_String RJMJobList;
	CSG_String Walltime;
	int JobType, Tasks, CPUs, Memory;
	// loglevel ...
	






	bool ConfigExists(void);
	bool Configure(void);
	void DisplayRJMLog(void);
	CSG_String GetModules(void);
	void ReadConfig(void);
};





#endif
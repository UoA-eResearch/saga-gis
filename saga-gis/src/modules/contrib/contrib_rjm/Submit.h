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
	CSG_String UserHomeDir;
	CSG_String RJMConfigDirName;
	CSG_String RJMConfigFilename;
	CSG_String RJMConfigFilePath;
	CSG_String RJMLogFilePath;
	CSG_String RJMBinDir;
	CSG_String RJMConfigure;
	CSG_String RJMRunRemote;
	CSG_String RJMJobList;

	CSG_String JobName;

	bool ConfigExists(void);
	bool Configure(void);
	void DisplayRJMLog(void);
	CSG_String GetModules(void);
};





#endif
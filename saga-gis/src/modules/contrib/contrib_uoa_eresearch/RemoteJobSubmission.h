#ifndef HEADER_INCLUDED__REMOTEJOBSUBMISSION_H
#define HEADER_INCLUDED__REMOTEJOBSUBMISSION_H

#include "MLB_Interface.h"

class CRemoteJobSubmission : public CSG_Module
{
public:
	CRemoteJobSubmission(void);
	virtual ~CRemoteJobSubmission(void);

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

	bool ConfigExists(void);
	bool Configure(void);
	void DisplayRJMLog(void);
};





#endif
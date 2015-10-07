#include "RemoteJobSubmission.h"
#include <cstdlib>
#ifdef _WIN32
#include <Windows.h>
#endif


CRemoteJobSubmission::CRemoteJobSubmission(void)
{
	Set_Name(_TL("Remote Job Submission"));
	Set_Author		(SG_T("Sina Masoud-Ansari"));
	Set_Description	(_TW("Remote job submission tools"));

	// Common properties
	RJMConfigDirName = CSG_String(".remote_jobs");
	RJMConfigFilename = CSG_String("config.ini");
	RJMLogFilePath = CSG_String();
	RJMBinDir = SG_File_Make_Path(CSG_String("bin"), CSG_String("rjm"));

	RJMConfigure = SG_File_Make_Path(RJMBinDir, CSG_String("rjm_configure"), CSG_String("exe"));
	RJMConfigure = SG_File_Get_Path_Absolute(RJMConfigure);
}


CRemoteJobSubmission::~CRemoteJobSubmission(void){}

bool CRemoteJobSubmission::On_Execute(void)
{
	if (!ConfigExists())
	{
		bool configure = Message_Dlg_Confirm(CSG_String::Format(SG_T("%s"), _TL("Remote Job Submission isn't configured, would you like to configure it now?")));
		if (configure)
		{
			return Configure();
		}
	}
	return true;
}

bool CRemoteJobSubmission::ConfigExists()
{
#ifdef _WIN32
	UserHomeDir = CSG_String(getenv("USERPROFILE"));
#else
	UserHomeDir = CSG_String(getenv("HOME"));
#endif


	CSG_String RJMConfigDirPath = SG_File_Make_Path(UserHomeDir, RJMConfigDirName);
	RJMConfigFilePath = SG_File_Make_Path(RJMConfigDirPath, RJMConfigFilename, CSG_String("ini"));
	return SG_File_Exists(RJMConfigFilePath);
}

bool CRemoteJobSubmission::Configure()
{
	
	CSG_String cmd = CSG_String::Format(SG_T("%s"), RJMConfigure.c_str());
	Message_Add(CSG_String("Executing: '%s'") + cmd);

	if (system(cmd.b_str()) != 0)
	{
		Error_Set(CSG_String("Setup failed, perhaps a password was incorrect?"));

		// Delete old config file if setup failed
		if (SG_File_Exists(RJMConfigFilePath))
		{
			if (!SG_File_Delete(RJMConfigFilePath))
			{
				Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete config file: "), RJMConfigFilePath.c_str()));
				return( false );
			}
		}

		return false;
	}
	return true;
}

void CRemoteJobSubmission::DisplayRJMLog()
{
	if (!RJMLogFilePath.is_Empty() && SG_File_Exists(RJMLogFilePath))
	{
		CSG_File File;
		if (File.Open(RJMLogFilePath, SG_FILE_R, false))
		{
			CSG_String Line;
			while (! File.is_EOF() && File.Read_Line(Line))
			{
				Message_Add(Line);
			}
			File.Close();
		} else 
		{
			Message_Add(CSG_String("Unable to open " + RJMLogFilePath + CSG_String(" for reading")));
		}
	}
}
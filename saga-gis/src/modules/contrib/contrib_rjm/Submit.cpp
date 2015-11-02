#include "Submit.h"
#include <cstdlib>

CSubmit::CSubmit(void)
{
	Set_Name(_TL("Submit"));
	Set_Author		(SG_T("Sina Masoud-Ansari"));
	Set_Description	(_TW("Remote job submission tools"));

#ifdef _WIN32
	UserHomeDir = CSG_String(getenv("USERPROFILE"));
#else
	UserHomeDir = CSG_String(getenv("HOME"));
#endif

	// Common properties
	RJMConfigDirName = CSG_String(".remote_jobs");
	RJMConfigFilename = CSG_String("config.ini");
	RJMBinDir = SG_File_Make_Path(CSG_String("bin"), CSG_String("rjm"));

	RJMJobList = SG_File_Make_Path(UserHomeDir, CSG_String("joblist"), CSG_String("txt"));
	RJMJobList = SG_File_Get_Path_Absolute(RJMJobList);
	
	RJMLogFilePath = SG_File_Make_Path(RJMBinDir, CSG_String("rjmlog"), CSG_String("txt"));
	RJMLogFilePath = SG_File_Get_Path_Absolute(RJMLogFilePath);

	RJMConfigure = SG_File_Make_Path(RJMBinDir, CSG_String("rjm_configure"), CSG_String("exe"));
	RJMConfigure = SG_File_Get_Path_Absolute(RJMConfigure);

	RJMRunRemote = SG_File_Make_Path(RJMBinDir, CSG_String("run_remote"), CSG_String("exe"));
	RJMRunRemote = SG_File_Get_Path_Absolute(RJMRunRemote);

	Parameters.Add_FilePath(NULL, "JOB_LIST", "Job List", _TL("File to keep track of current jobs"), NULL, false, false, false, false);
	Parameters("JOB_LIST")->asFilePath()->Set_Value(RJMJobList);
	
	Parameters.Add_String(NULL, "JOB_NAME", _TL("Job Name"), _TL("A name used identify the job"), JobName, PARAMETER_INPUT);
	Parameters.Add_FilePath(NULL, "JOB_DIR", _TL("Job Directory"), _TL("Directory used for storing job data"), NULL, false, false, true, false); 
	Parameters.Add_FilePath(NULL, "UPLOADS", _TL("Files"), _TL("List of files to upload for the job. These will be copied to the remote file system. Use CTRL or SHIFT to select multiple files."), NULL, false, false, false, true); 
	Parameters.Add_Choice(NULL, "MODULE", "Module","Select the module to be used in this job.","Refresh|None|");
	Parameters.Add_Value(NULL, "WAIT", _TL("Wait for job completion?"),_TL("Note it is fine to and run the Wait tool instead."), PARAMETER_TYPE_Bool, true);
}


CSubmit::~CSubmit(void){}

bool CSubmit::On_Execute(void)
{


	if (!ConfigExists())
	{
		bool configure = Message_Dlg_Confirm(CSG_String::Format(SG_T("%s"), _TL("Remote Job Submission isn't configured, would you like to configure it now?")));
		if (configure)
		{
			return Configure();
		}
	} else if (	Parameters("MODULE")->asInt() == 0 )
	{
		CSG_String modules = GetModules();
		Parameters("MODULE")->asChoice()->Set_Items(modules);
	} else {
		// set the joblist file
		RJMJobList = Parameters("JOB_LIST")->asFilePath()->asString();


		// if job dir does not exist create
		CSG_String JobDir = Parameters("JOB_DIR")->asFilePath()->asString();
		JobDir = SG_File_Get_Path_Absolute(JobDir);
		if (!SG_Dir_Exists(JobDir))
		{
			if (!SG_Dir_Create(JobDir))
			{
				Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to create directory"), JobDir.c_str()));
				return false;
			}
		}

		// create or append to jobslist file
		if (SG_File_Exists(RJMJobFile))
		{
			// append
		} 
		else 
		{
			// create
		}


		// Module to use
		CSG_String Module = Parameters("MODULE")->asChoice()->asString();

		// files to upload
		CSG_Strings Uploads;
		Parameters("UPLOADS")->asFilePath()->Get_FilePaths(Uploads);
	}

	

	return true;
}

bool CSubmit::ConfigExists()
{
	CSG_String RJMConfigDirPath = SG_File_Make_Path(UserHomeDir, RJMConfigDirName);
	RJMConfigFilePath = SG_File_Make_Path(RJMConfigDirPath, RJMConfigFilename, CSG_String("ini"));
	return SG_File_Exists(RJMConfigFilePath);
}

bool CSubmit::Configure()
{
	
	CSG_String cmd = CSG_String::Format(SG_T("%s"), RJMConfigure.c_str());
	Message_Add(CSG_String("Executing: ") + cmd);

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

void CSubmit::DisplayRJMLog()
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

CSG_String CSubmit::GetModules()
{
	CSG_String modules("Refresh|None|");

	CSG_String OutFile = SG_File_Make_Path(RJMBinDir, CSG_String("out"), CSG_String("txt"));
	OutFile = SG_File_Get_Path_Absolute(OutFile);

	CSG_String ErrorFile = SG_File_Make_Path(RJMBinDir, CSG_String("error"), CSG_String("txt"));
	ErrorFile = SG_File_Get_Path_Absolute(ErrorFile);

	CSG_String cmd = CSG_String::Format(SG_T("%s module avail >%s 2>%s"), RJMRunRemote.c_str(), OutFile.c_str(), ErrorFile.c_str());
	Message_Add(CSG_String("Executing: ") + cmd);

	// run process
	if (system(cmd.b_str()) != 0)
	{
		Error_Set(CSG_String::Format(SG_T("Error executing '%s' see Execution log for details"), cmd.c_str()));
		// read log output
		CSG_File File;
		if (File.Open(ErrorFile, SG_FILE_R, false))
		{
			CSG_String Line;
			while (! File.is_EOF() && File.Read_Line(Line))
			{
				Message_Add(Line);
			}
			File.Close();
		} else 
		{
			Message_Add(CSG_String("Unable to open " + ErrorFile + CSG_String(" for reading")));
		}
	} else
	{
		// read output
		bool started = false;
		CSG_File File;
		if (File.Open(OutFile, SG_FILE_R, false))
		{
			CSG_String Line;
			while (! File.is_EOF() && File.Read_Line(Line))
			{
				Line.Trim();
				Line = Line.BeforeFirst(' ');

				if (started)
				{
					Line.Trim();
					if (Line.is_Empty())
					{
						break;
					} else {
						modules.Append(Line);
						modules.Append("|");
					}
				}

				if (!started)
				{
					if (Line.Contains("-----"))
					{
						started = true;
					}
				}


			}
			File.Close();
		} else 
		{
			Message_Add(CSG_String("Unable to open " + OutFile + CSG_String(" for reading")));
		}

	}

	return modules;
}
#include "Submit.h"
#include "SimpleIni.h"
#include <cstdlib>
#include <cmath>

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

	CSG_Parameter	*pNode;
	CSG_Parameter	*pNodeConfig;
	CSG_Parameter	*pNodeConfigCluster;
	CSG_Parameter	*pNodeConfigFileTransfer;
	CSG_Parameter	*pNodeConfigRetry;

	// Common properties
	RJMConfigDirName = CSG_String(".remote_jobs");
	RJMConfigFilename = CSG_String("config.ini");
	CSG_String RJMConfigDirPath = SG_File_Make_Path(UserHomeDir, RJMConfigDirName);
	RJMConfigFilePath = SG_File_Make_Path(RJMConfigDirPath, RJMConfigFilename, CSG_String("ini"));

	RJMBinDir = SG_File_Make_Path(CSG_String("bin"), CSG_String("rjm"));

	RJMJobList = SG_File_Make_Path(UserHomeDir, CSG_String("joblist"), CSG_String("txt"));
	RJMJobList = SG_File_Get_Path_Absolute(RJMJobList);
	
	RJMLogFilePath = SG_File_Make_Path(RJMBinDir, CSG_String("rjmlog"), CSG_String("txt"));
	RJMLogFilePath = SG_File_Get_Path_Absolute(RJMLogFilePath);

	RJMConfigure = SG_File_Make_Path(RJMBinDir, CSG_String("rjm_configure"), CSG_String("exe"));
	RJMConfigure = SG_File_Get_Path_Absolute(RJMConfigure);

	RJMRunRemote = SG_File_Make_Path(RJMBinDir, CSG_String("run_remote"), CSG_String("exe"));
	RJMRunRemote = SG_File_Get_Path_Absolute(RJMRunRemote);

	RJMBatchSubmit = SG_File_Make_Path(RJMBinDir, CSG_String("rjm_batch_submit"), CSG_String("exe"));
	RJMBatchSubmit = SG_File_Get_Path_Absolute(RJMBatchSubmit);

	// Config defaults
	RemoteHost = CSG_String("");

	Parameters.Add_FilePath(NULL, "JOB_LIST", "Job List", _TL("File to keep track of current jobs"), NULL, false, false, false, false);
	Parameters("JOB_LIST")->asFilePath()->Set_Value(RJMJobList);
	
	Parameters.Add_String(NULL, "JOB_NAME", _TL("Job Name"), _TL("A name used identify the job"), JobName, PARAMETER_INPUT);
	Parameters.Add_String(NULL, "PROJECT_CODE", _TL("Project Code"), _TL("Project code used for accounting job hours."), ProjectCode, PARAMETER_INPUT);
	Parameters.Add_FilePath(NULL, "JOB_DIR", _TL("Job Directory"), _TL("Directory used for storing job data"), NULL, false, false, true, false); 
	Parameters.Add_FilePath(NULL, "UPLOADS", _TL("Files to Upload"), _TL("List of files to upload for the job. These will be copied to the remote file system. Use CTRL or SHIFT to select multiple files."), NULL, false, false, false, true); 
	Parameters.Add_Choice(NULL, "MODULE", "Module","Select the module to be used in this job.","None|Refresh|");
	Parameters.Add_Value(NULL, "WAIT", _TL("Wait for job completion?"),_TL("Note it is fine to and run the Wait tool instead."), PARAMETER_TYPE_Bool, true);
	Parameters.Add_Value(NULL, "DOWNLOAD_ALL", _TL("Download all files?"),_TL("If you are unsure which files you want to download, you can download everything in the remote job directory."), PARAMETER_TYPE_Bool, true);
	Parameters.Add_FilePath(NULL, "DOWNLOADS", _TL("Files to Download"), _TL("List of files to download for the job. These will be copied from the remote file system. If unsure, check the 'Download all files' option.\n\nExample:\n\n\"results.csv\" \"output.txt\""), NULL, false, false, false, true); 
	Parameters.Add_String(NULL, "COMMAND", _TL("Command"), _TL("Command to run e.g.srun echo hello"), RemoteCommand, PARAMETER_INPUT);
	Parameters.Add_Choice(NULL, "JOBTYPE", "Job Type","Select the module to be used in this job.","Single Process|MPI|");
	Parameters.Add_Value(NULL, "TASKS"	, _TL("Tasks"), _TL("Number of tasks to use. Only relevant for MPI job types."), PARAMETER_TYPE_Int, 1, 1, true);
	Parameters.Add_Value(NULL, "CPUS"	, _TL("CPUs per task"), _TL("The number of CPUs that will be available for each task. This is equivalent to the number of threads each process will use."), PARAMETER_TYPE_Int, 1, 1, true);
	Parameters.Add_Value(NULL, "MEMORY"	, _TL("Memory (GB)"), _TL("Memory to use for the job in GB. For MPI Jobs this is the per process memory. For Single Process jobs, this is the total memory."), PARAMETER_TYPE_Int, 2, 1, true);
	
	// walltime
	pNode = Parameters.Add_Node(NULL, "WALLTIME", _TL("Walltime"), _TL("The time allowed for job completion"));
	Parameters.Add_Value(pNode, "HOURS", _TL("Hours"), _TL(""), PARAMETER_TYPE_Int, 1, 0, true);
	Parameters.Add_Value(pNode, "MINUTES", _TL("Minutes"), _TL(""), PARAMETER_TYPE_Int, 0, 0, true);
	Parameters.Add_Value(pNode, "SECONDS", _TL("Seconds"), _TL(""), PARAMETER_TYPE_Int, 0, 0, true);

	// config settings
	pNodeConfig = Parameters.Add_Node(NULL, "CONFIG", _TL("Configuration"), CSG_String::Format("These values are set in '%s' and will be refreshed on execute. You can modify the file to change the settings below.", RJMConfigFilePath.c_str()));
	pNodeConfigCluster = Parameters.Add_Node(pNodeConfig, "CONFIG_CLUSTER", _TL("Cluster"),"");
	pNodeConfigFileTransfer = Parameters.Add_Node(pNodeConfig, "CONFIG_FILE_TRANSFER", _TL("File Transfer"),"");
	pNodeConfigRetry = Parameters.Add_Node(pNodeConfig, "CONFIG_RETRY", _TL("Retry"),"");
	
	// config cluster
	Parameters.Add_String(pNodeConfigCluster, "REMOTE_HOST", _TL("Remote Host"), _TL("The remote machine hostname"), RemoteHost, PARAMETER_INFORMATION);
	Parameters.Add_String(pNodeConfigCluster, "REMOTE_USER", _TL("Remote User"), _TL("The account username used to connect to the remote host."), RemoteUser, PARAMETER_INFORMATION);


	// read config file
	ReadConfig();
}


CSubmit::~CSubmit(void){}

bool CSubmit::On_Execute(void)
{
	if (ConfigExists())
	{
		ReadConfig();
	} 
	else
	{
		bool configure = Message_Dlg_Confirm(CSG_String::Format(SG_T("%s"), _TL("Remote Job Submission isn't configured, would you like to configure it now?")));
		if (configure)
		{
			if(Configure())
			{
				ReadConfig();
			}
			else
			{
				return false;
			}
		}
	}
	
	if ( Parameters("MODULE")->asInt() == 1 )
	{
		CSG_String modules = GetModules();
		Parameters("MODULE")->asChoice()->Set_Items(modules);
	} else {
		CSG_File File;
		bool cont;

		// set the joblist file
		RJMJobList = Parameters("JOB_LIST")->asFilePath()->asString();

		// if job dir does not exist create
		CSG_String JobDir = Parameters("JOB_DIR")->asFilePath()->asString();
		JobDir.Trim();
		if (JobDir.is_Empty())
		{
			Message_Dlg(_TL("A job directory is required."));
			return false;			
		}

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
		int mode = SG_FILE_W;
		if (SG_File_Exists(RJMJobList))
		{
			mode = SG_FILE_WA;
		} 

		// write jobdir to jobslist
		if (File.Open(RJMJobList, mode, false))
		{
			File.Write(JobDir + CSG_String("\n"));
			File.Flush();
			File.Close();
		} else 
		{
			Message_Add(CSG_String("Unable to open " + RJMJobList + CSG_String(" for writing")));
			return false;
		}
		
		// create uploads and downloads files
		
		// files to upload
		CSG_Strings FilesToUpload;
		CSG_Strings FilesToDownload;
		Parameters("UPLOADS")->asFilePath()->Get_FilePaths(FilesToUpload);
		Parameters("DOWNLOADS")->asFilePath()->Get_FilePaths(FilesToDownload);
		CSG_String RJMUploads = SG_File_Make_Path(JobDir, CSG_String("rjm_uploads"), CSG_String("txt"));
		CSG_String RJMDownloads = SG_File_Make_Path(JobDir, CSG_String("rjm_downloads"), CSG_String("txt"));


		// write uploads file
		if (File.Open(RJMUploads, SG_FILE_W, false))
		{
			for (int i = 0; i < FilesToUpload.Get_Count(); i++)
			{

				File.Write(FilesToUpload.Get_String(i) + CSG_String("\n"));
			}
			File.Flush();
			File.Close();
		} else 
		{
			Message_Add(CSG_String("Unable to open " + RJMUploads + CSG_String(" for writing")));
			return false;
		}

		// write downloads file
		if (File.Open(RJMDownloads, SG_FILE_W, false))
		{
			if (Parameters("DOWNLOAD_ALL")->asBool())
			{
				File.Write(CSG_String("results.zip\n"));
			} else {
				if (FilesToDownload.Get_Count() == 0)
				{
					cont = Message_Dlg_Confirm(CSG_String::Format(SG_T("%s"), _TL("There are no files specified for download, would you like to continue?")));
					if (!cont)
					{
						return false;
					}
				}

				for (int i = 0; i < FilesToDownload.Get_Count(); i++)
				{

					File.Write(FilesToDownload.Get_String(i) + CSG_String("\n"));
				}
			}
			File.Flush();
			File.Close();
		} else 
		{
			Message_Add(CSG_String("Unable to open " + RJMDownloads + CSG_String(" for writing")));
			return false;
		}

		// submit job
		CSG_String OutFile = SG_File_Make_Path(RJMBinDir, CSG_String("out"), CSG_String("txt"));
		OutFile = SG_File_Get_Path_Absolute(OutFile);

		CSG_String ErrorFile = SG_File_Make_Path(RJMBinDir, CSG_String("error"), CSG_String("txt"));
		ErrorFile = SG_File_Get_Path_Absolute(ErrorFile);

		// remote command
		if (RemoteCommand.is_Empty())
		{
			cont = Message_Dlg_Confirm(CSG_String::Format(SG_T("%s"), _TL("No command has been specified, would you like to continue?")));
			if (!cont)
			{
				return false;
			}
		}

		// Module to use
		CSG_String Module = Parameters("MODULE")->asChoice()->asString();
		bool ModuleIsNone = Module.is_Same_As(CSG_String("None"), true);

		if (!ModuleIsNone)
		{
			// add module to command
			RemoteCommand = CSG_String::Format(SG_T("module load %s; %s"), Module.c_str(), RemoteCommand.c_str());
		}

		// job specs
		JobType = Parameters("JOB_TYPE")->asInt();
		Tasks = Parameters("TASKS")->asInt();
		CPUs = Parameters("CPUS")->asInt();
		Memory = Parameters("MEMORY")->asInt();
		 
		// adapt memory to mem-per-cpu
		Memory = (int)(ceil(Memory / (double)Tasks)); 

		// wallitme

		int Hours = Parameters("HOURS")->asInt();
		int Minutes = Parameters("MINUTES")->asInt();
		int Seconds = Parameters("SECONDS")->asInt();
		Walltime = CSG_String::Format(SG_T("%d:%d:%d"), Hours, Minutes, Seconds);

		// command
		RJMCMD = CSG_String::Format(SG_T("%s -c\"%s\" -f \"%s\" -l \"%s\" -w %s -m %dG -p %s"), RJMBatchSubmit.c_str(), RemoteCommand.c_str(), RJMJobList.c_str(), RJMLogFilePath.c_str(), Walltime.c_str(), Memory, ProjectCode);
		Message_Add(CSG_String("Executing: ") + RJMCMD);

		/*
		// run process
		if (system(cmd.b_str()) != 0)
		{
			Error_Set(CSG_String::Format(SG_T("Error executing '%s' see Execution log for details"), cmd.c_str()));
			// read log output
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
		*/

	}

	

	return true;
}

bool CSubmit::ConfigExists()
{
	return SG_File_Exists(RJMConfigFilePath);
}

void CSubmit::ReadConfig()
{
	
	if (ConfigExists())
	{
		// read ini file
		CSimpleIniA ini;
		ini.SetUnicode();
		ini.LoadFile(RJMConfigFilePath.c_str());
		
		RemoteHost = CSG_String(ini.GetValue("CLUSTER", "remote_host", ""));
		Parameters("REMOTE_HOST")->Set_Value(RemoteHost);

		RemoteUser = CSG_String(ini.GetValue("CLUSTER", "remote_user", ""));
		Parameters("REMOTE_USER")->Set_Value(RemoteUser); 



	}
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
	CSG_String modules("None|Refresh|");

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
		if (File.Open(OutFile, SG_FILE_R, false))
		{
			CSG_String Line;
			while (! File.is_EOF() && File.Read_Line(Line))
			{
				Message_Add(Line);
			}
			File.Close();
		} else 
		{
			Message_Add(CSG_String("Unable to open " + OutFile + CSG_String(" for reading")));
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
#include "Clean.h"
#include "SimpleIni.h"
#include <cstdlib>
#include <cmath>
#include <ctime>

#ifdef _WIN32

#else
	#include <unistd.h>
#endif

CClean::CClean(void)
{
	Set_Name(_TL("Clean"));
	Set_Author		(SG_T("Sina Masoud-Ansari"));
	Set_Description	(_TW("Remove uneeded files from a remote filesystem."));

	#ifdef _WIN32
		UserHomeDir = CSG_String(getenv("USERPROFILE"));
	#else
		UserHomeDir = CSG_String(getenv("HOME"));
	#endif
	CSG_Parameter	*pNodeLogging;
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
	RJMTempDir = SG_File_Make_Path(UserHomeDir, CSG_String("Saga_GIS_tmp"));

	RJMJobList = SG_File_Make_Path(RJMTempDir, CSG_String("joblist"), CSG_String("txt"));
	RJMJobList = SG_File_Get_Path_Absolute(RJMJobList);

	RJMLogFilePath = SG_File_Make_Path(RJMTempDir, CSG_String("rjmlog"), CSG_String("txt"));
	RJMLogFilePath = SG_File_Get_Path_Absolute(RJMLogFilePath);

	RJMConfigure = SG_File_Make_Path(RJMBinDir, CSG_String("rjm_configure"), CSG_String("exe"));
	RJMConfigure = SG_File_Get_Path_Absolute(RJMConfigure);

	RJMBatchClean = SG_File_Make_Path(RJMBinDir, CSG_String("rjm_batch_clean"), CSG_String("exe"));
	RJMBatchClean = SG_File_Get_Path_Absolute(RJMBatchClean);

	// Params

	Parameters.Add_FilePath(NULL, "JOB_LIST", "Job List", _TL("File to keep track of current jobs"), NULL, false, false, false, false);

	// logging
	pNodeLogging = Parameters.Add_Node(NULL, "LOGGING", _TL("Logging"), _TL("Job logging options"));
	Parameters.Add_FilePath(pNodeLogging, "LOGFILE", _TL("Logfile"), _TL("Job submission logs are recorded here."), NULL, false, false, false, false); 
	Parameters.Add_Choice(pNodeLogging, "LOG_LEVEL", "Log Level","Level of log verbosity. More information will be printed at higher levels.","debug|info|warn|error|critical|", 3); 

	// config settings
	pNodeConfig = Parameters.Add_Node(NULL, "CONFIG", _TL("Configuration"), CSG_String::Format("These values are set in '%s' and will be refreshed on execute. You can modify the file to change the settings below.", RJMConfigFilePath.c_str()));
	pNodeConfigCluster = Parameters.Add_Node(pNodeConfig, "CONFIG_CLUSTER", _TL("Cluster"),"");
	pNodeConfigFileTransfer = Parameters.Add_Node(pNodeConfig, "CONFIG_FILE_TRANSFER", _TL("File Transfer"),"");
	pNodeConfigRetry = Parameters.Add_Node(pNodeConfig, "CONFIG_RETRY", _TL("Retry"),"");
	
	// config cluster
	Parameters.Add_String(pNodeConfigCluster, "REMOTE_HOST", _TL("Remote Host"), _TL("The remote machine hostname"), RemoteHost, PARAMETER_INFORMATION);
	Parameters.Add_String(pNodeConfigCluster, "REMOTE_USER", _TL("Remote User"), _TL("The account username used to connect to the remote host."), RemoteUser, PARAMETER_INFORMATION);
	Parameters.Add_String(pNodeConfigCluster, "SSH_PRIV_KEY_FILE", _TL("SSH Private Key File"), _TL("The SSH key used to connect to the remote host."), SSHPrivKeyFile, PARAMETER_INFORMATION);
	Parameters.Add_String(pNodeConfigCluster, "SSH_FINGERPRINT", _TL("SSH Key Fingerprint"), _TL("Used to uniquely identify your SSH key."), SSHFingerprint, PARAMETER_INFORMATION);
	Parameters.Add_String(pNodeConfigCluster, "DEFAULT_PROJECT_CODE", _TL("Default Project Code"), _TL("The default project code used to submit jobs against. Can be overriden in the job settings above."), DefaultProjectCode, PARAMETER_INFORMATION);
	Parameters.Add_String(pNodeConfigCluster, "DEFAULT_REMOTE_DIRECTORY", _TL("Default Remote Directory"), _TL("The default remote directory used to hold jobs files on the remote system. Can be overriden in the job settings above."), DefaultRemoteDirectory, PARAMETER_INFORMATION);
	Parameters.Add_String(pNodeConfigCluster, "REMOTE_PREPARE_JOB", _TL("Remote Prepare Job"), _TL("The remote program used to create a job."), RemotePrepareJob, PARAMETER_INFORMATION);
	Parameters.Add_String(pNodeConfigCluster, "REMOTE_SUBMIT_JOB", _TL("Remote Submit Job"), _TL("The remote program used to submit a job."), RemoteSubmitJob, PARAMETER_INFORMATION);
	Parameters.Add_String(pNodeConfigCluster, "REMOTE_IS_JOB_DONE", _TL("Remote Check Job"), _TL("The remote program used to check for job completion."), RemoteIsJobDone, PARAMETER_INFORMATION);
	Parameters.Add_String(pNodeConfigCluster, "REMOTE_GET_JOB_STATUSES", _TL("Remote Get Job Status"), _TL("The remote program used to check for job status."), RemoteGetJobStatuses, PARAMETER_INFORMATION);
	Parameters.Add_String(pNodeConfigCluster, "REMOTE_CANCEL_JOBS", _TL("Remote Cancel Jobs"), _TL("The remote program used to cancel jobs."), RemoteCancelJobs, PARAMETER_INFORMATION);

	//config file transfer
	Parameters.Add_String(pNodeConfigFileTransfer, "UPLOADS_FILE", _TL("Uploads File"), _TL("Filename for file that holds the file paths to be uploaded for a job."), UploadsFile, PARAMETER_INFORMATION);
	Parameters.Add_String(pNodeConfigFileTransfer, "DOWNLOADS_FILE", _TL("Downloads File"), _TL("Filename for file that holds the file paths to be downloaded for a job."), DownloadsFile, PARAMETER_INFORMATION);

	// config retry
	Parameters.Add_Value(pNodeConfigRetry, "MAX_ATTEMPTS", _TL("Max Attempts"), _TL("Maximum number of retry attempts when connecting to the remote host."), PARAMETER_TYPE_Int, 5, 0, true);
	Parameters.Add_Value(pNodeConfigRetry, "MIN_WAIT", _TL("Min Wait"), _TL("Seconds to wait before attempting to reconncet to the remote host."), PARAMETER_TYPE_Double, 0.5, 0, true);
	Parameters.Add_Value(pNodeConfigRetry, "MAX_WAIT", _TL("Max Wait"), _TL("Seconds to wait before aborting connection attempt."), PARAMETER_TYPE_Double, 5, 1, true);

	// read config file
	ReadConfig();

	// set some parameter defaults
	Parameters("JOB_LIST")->asFilePath()->Set_Value(RJMJobList);
	Parameters("LOGFILE")->Set_Value(RJMLogFilePath);
}


CClean::~CClean(void){}

bool CClean::On_Execute(void)
{

	
	// make sure temp dir exists
	if (!SG_Dir_Exists(RJMTempDir))
	{
		if (!SG_Dir_Create(RJMTempDir))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to create temp directory"), RJMTempDir.c_str()));
		}
	}

	if (ConfigExists())
	{
		ReadConfig();
	} 
	else
	{
		bool configure = Message_Dlg_Confirm(CSG_String::Format(SG_T("%s"), _TL("Remote Job Management Tools are not configured, would you like to configure it now?")));
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

	// check input
	if (!GetParameterValues())
	{
		return false;
	}

	CSG_File File;
		
	// check joblist file
	if (SG_File_Exists(RJMJobList))
	{
		if (File.Open(RJMJobList, SG_FILE_R, false))
		{
			if (File.Length() == 0)
			{
				Message_Dlg(CSG_String::Format(SG_T("%s"), _TL("Job List file is empty, cannot track submitted jobs.")));
				return false;
			}
		}
	} else {
		Message_Dlg(CSG_String::Format(SG_T("%s"), _TL("Job List file does not exist")));
		return false;		
	}
	
	if (Process_Get_Okay(true))
	{
	
		Set_Progress(10);
		Message_Add(CSG_String("Cleaning up remote files ..."));
		Process_Set_Text(CSG_String("Cleaning up remote files ..."));
		CSG_String RJMCMD = CSG_String::Format(SG_T("\"\"%s\" -f \"%s\" -l \"%s\" -ll %s\""), RJMBatchClean.c_str(), RJMJobList.c_str(), RJMLogFilePath.c_str(), LogLevel.c_str());
				
		// clean
		if (system(RJMCMD.b_str()) != 0)
		{
			Message_Dlg(CSG_String::Format(SG_T("Error while cleaning up remote files, see Execution Log for details")));
			DisplayRJMLog();
			return false;
		}	
		
	}
			
	Set_Progress(100);

	return true;
}

void CClean::DisplayRJMLog()
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

bool CClean::GetParameterValues()
{

	// job list 
	RJMJobList = Parameters("JOB_LIST")->asFilePath()->asString();
	if (RJMJobList.is_Empty())
	{
		Message_Dlg(_TL("A job list filename is required."));
		return false;			
	}

	// logging
	RJMLogFilePath = Parameters("LOGFILE")->asFilePath()->asString();
	if (RJMLogFilePath.is_Empty())
	{
		Message_Dlg(_TL("A logfile path is required."));
		return false;			
	}
	else
	{
		RJMLogFilePath = SG_File_Get_Path_Absolute(RJMLogFilePath);
	}

	LogLevel = Parameters("LOG_LEVEL")->asChoice()->asString();

	return true;
}

bool CClean::ConfigExists()
{
	return SG_File_Exists(RJMConfigFilePath);
}

void CClean::ReadConfig()
{
	
	if (ConfigExists())
	{
		// read ini file
		CSimpleIniA ini;
		ini.SetUnicode();
		ini.LoadFile(RJMConfigFilePath.c_str());
		
		// config cluster
		RemoteHost = CSG_String(ini.GetValue("CLUSTER", "remote_host", ""));							Parameters("REMOTE_HOST")->Set_Value(RemoteHost);
		RemoteUser = CSG_String(ini.GetValue("CLUSTER", "remote_user", ""));							Parameters("REMOTE_USER")->Set_Value(RemoteUser); 
		SSHPrivKeyFile = CSG_String(ini.GetValue("CLUSTER", "ssh_priv_key_file", ""));					Parameters("SSH_PRIV_KEY_FILE")->Set_Value(SSHPrivKeyFile); 
		SSHFingerprint = CSG_String(ini.GetValue("CLUSTER", "ssh_fingerprint", ""));					Parameters("SSH_FINGERPRINT")->Set_Value(SSHFingerprint); 
		DefaultProjectCode = CSG_String(ini.GetValue("CLUSTER", "default_project_code", ""));			Parameters("DEFAULT_PROJECT_CODE")->Set_Value(DefaultProjectCode); 
		DefaultRemoteDirectory = CSG_String(ini.GetValue("CLUSTER", "default_remote_directory", ""));	Parameters("DEFAULT_REMOTE_DIRECTORY")->Set_Value(DefaultRemoteDirectory); 
		RemotePrepareJob = CSG_String(ini.GetValue("CLUSTER", "remote_prepare_job", ""));				Parameters("REMOTE_PREPARE_JOB")->Set_Value(RemotePrepareJob); 
		RemoteSubmitJob = CSG_String(ini.GetValue("CLUSTER", "remote_submit_job", ""));					Parameters("REMOTE_SUBMIT_JOB")->Set_Value(RemoteSubmitJob); 
		RemoteIsJobDone = CSG_String(ini.GetValue("CLUSTER", "remote_is_job_done", ""));				Parameters("REMOTE_IS_JOB_DONE")->Set_Value(RemoteIsJobDone); 
		RemoteGetJobStatuses = CSG_String(ini.GetValue("CLUSTER", "remote_get_job_statuses", ""));		Parameters("REMOTE_GET_JOB_STATUSES")->Set_Value(RemoteGetJobStatuses); 
		RemoteCancelJobs = CSG_String(ini.GetValue("CLUSTER", "remote_cancel_jobs", ""));				Parameters("REMOTE_CANCEL_JOBS")->Set_Value(RemoteCancelJobs); 

		//config file transfer
		UploadsFile = CSG_String(ini.GetValue("FILE_TRANSFER", "uploads_file", ""));		Parameters("UPLOADS_FILE")->Set_Value(UploadsFile); 
		DownloadsFile = CSG_String(ini.GetValue("FILE_TRANSFER", "downloads_file", ""));	Parameters("DOWNLOADS_FILE")->Set_Value(DownloadsFile); 

		// config retry
		MaxAttempts = CSG_String(ini.GetValue("RETRY", "max_attempts", "")).asInt();		Parameters("MAX_ATTEMPTS")->Set_Value(MaxAttempts); 
		MinWait = CSG_String(ini.GetValue("RETRY", "min_wait_s", "")).asDouble();				Parameters("MIN_WAIT")->Set_Value(MinWait); 
		MaxWait = CSG_String(ini.GetValue("RETRY", "max_wait_s", "")).asDouble();				Parameters("MAX_WAIT")->Set_Value(MaxWait); 
	}
}

bool CClean::Configure()
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
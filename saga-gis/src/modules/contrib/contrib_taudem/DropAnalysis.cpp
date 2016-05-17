
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                       contrib_taudem                  //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                     DropAnalysis.cpp                  //
//                                                       //
//                                                       //
//-------------------------------------------------------//
//                                                       //
// This file is part of 'SAGA - System for Automated     //
// Geoscientific Analyses'. SAGA is free software; you   //
// can redistribute it and/or modify it under the terms  //
// of the GNU General Public License as published by the //
// Free Software Foundation; version 2 of the License.   //
//                                                       //
// SAGA is distributed in the hope that it will be       //
// useful, but WITHOUT ANY WARRANTY; without even the    //
// implied warranty of MERCHANTABILITY or FITNESS FOR A  //
// PARTICULAR PURPOSE. See the GNU General Public        //
// License for more details.                             //
//                                                       //
// You should have received a copy of the GNU General    //
// Public License along with this program; if not,       //
// write to the Free Software Foundation, Inc.,          //
// 51 Franklin Street, 5th Floor, Boston, MA 02110-1301, //
// USA.                                                  //
//                                                       //
///////////////////////////////////////////////////////////


#include "DropAnalysis.h"
#include "gdal_driver.h"
#include "ogr_driver.h"

CDropAnalysis::CDropAnalysis(void)
{

	#ifdef _WIN32
		CSG_String UserHomeDir = CSG_String(getenv("USERPROFILE"));
	#else
		CSG_String UserHomeDir = CSG_String(getenv("HOME"));
	#endif

	CSG_String DefaultTempDir = SG_File_Make_Path(UserHomeDir, CSG_String("Saga_GIS_tmp"));

	// INFO
	Set_Name(_TL("Stream Drop Analysis"));
	Set_Author(SG_T("S. Masoud-Ansari, J. Tunnicliffe, D. Tarboton"));
	Set_Description	(_TW("Applies a series of thresholds (determined from the input parameters) to the input accumulated stream source grid (*ssa) grid and outputs the results in the *drp.txt file the stream drop statistics table. This function is designed to aid in the determination of a geomorphologically objective threshold to be used to delineate streams. Drop Analysis attempts to select the right threshold automatically by evaluating a stream network for a range of thresholds and examining the constant drop property of the resulting Strahler streams. Basically it asks the question: Is the mean stream drop for first order streams statistically different from the mean stream drop for higher order streams, using a T-Test. Stream drop is the difference in elevation from the beginning to the end of a stream defined as the sequence of links of the same stream order. If the T test shows a significant difference then the stream network does not obey this \"law\" so a larger threshold needs to be chosen. The smallest threshold for which the T test does not show a significant difference gives the highest resolution stream network that obeys the constant stream drop \"law\" from geomorphology, and is the threshold chosen for the \"objective\" or automatic mapping of streams from the DEM. This function can be used in the development of stream network rasters, where the exact watershed characteristic(s) that were accumulated in the accumulated stream source grid vary based on the method being used to determine the stream network raster.\n\nThe constant stream drop \"law\" was identified by Broscoe (1959). For the science behind using this to determine a stream delineation threshold, see Tarboton et al. (1991, 1992), Tarboton and Ames (2001).\n\nBroscoe, A. J., (1959), \"Quantitative analysis of longitudinal stream profiles of small watersheds,\" Office of Naval Research, Project NR 389-042, Technical Report No. 18, Department of Geology, Columbia University, New York.\n\nTarboton, D. G., R. L. Bras and I. Rodriguez-Iturbe, (1991), \"On the Extraction of Channel Networks from Digital Elevation Data,\" Hydrologic Processes, 5(1): 81-100.\n\nTarboton, D. G., R. L. Bras and I. Rodriguez-Iturbe, (1992), \"A Physical Basis for Drainage Density,\" Geomorphology, 5(1/2): 59-76.\n\nTarboton, D. G. and D. P. Ames, (2001), \"Advances in the mapping of flow networks from digital elevation data,\" World Water and Environmental Resources Congress, Orlando, Florida, May 20-24, ASCE, http://www.engineering.usu.edu/dtarb/asce2001.pdf."));

	// GRIDS
	Parameters.Add_Grid(NULL, "FEL_INPUT"	, _TL("Input Pit Filled Elevation"), _TL("A grid of elevation values. This is usually the output of the \"Pit Remove\" tool, in which case it is elevations with pits removed. Pits are low elevation areas in digital elevation models (DEMs) that are completely surrounded by higher terrain. They are generally taken to be artifacts of the digitation process that interfere with the processing of flow across DEMs. So they are removed by raising their elevation to the point where they just drain off the domain. This step is not essential if you have reason to believe that the pits in your DEM are real, but with drop analysis ensures that the elevation drop downstream along a stream is positive."), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "FLOWD8_INPUT"	, _TL("Input D8 Flow Direction"), _TL("A grid of D8 flow directions which are defined, for each cell, as the direction of the one of its eight adjacent or diagonal neighbors with the steepest downward slope."), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "AREAD8_INPUT"	, _TL("Input D8 Contributing Area"), _TL("A grid of contributing area values for each cell that were calculated using the D8 algorithm. The contributing area for a cell is the sum of its own contribution plus the contribution from all upslope neighbors that drain to it, measured as a number of cells or the sum of weight loadings. This grid can be obtained as the output of the \"D8 Contributing Area\" tool. This grid is used in the evaluation of drainage density reported in the stream drop table."), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "SSA_INPUT"	, _TL("Input Accumulated Stream Source"), _TL("This grid must be monotonically increasing along the downslope D8 flow directions. It it compared to a series of thresholds to determine the beginning of the streams. It is often generated by accumulating some characteristic or combination of characteristics of the watershed with the \"D8 Contributing Area\" tool, or using the maximum option of the \"D8 Flow Path Extreme\" tool. The exact method varies depending on the algorithm being used."), PARAMETER_INPUT);
	
	// SHAPES
	Parameters.Add_Shapes(NULL, "OUTLET_INPUT"	, _TL("Input Outlet Points (Optional)"), _TL("A point shapefile defining the outlets upstream of which drop analysis is performed."), PARAMETER_INPUT);
	
	// TABLES
	Parameters.Add_Table(NULL, "DROPTABLE_OUTPUT", _TL("Output Drop Analysis Table"), _TL("This table contains one line of data for each threshold value examined, with a summary in the table description that indicates the optimum threshold value. This technique looks for the smallest threshold in the range where the absolute value of the t-statistic is less than 2. For the science behind the drop analysis, see Tarboton et al. (1991, 1992), Tarboton and Ames (2001)."), PARAMETER_OUTPUT); 

	// VALUES
	CSG_Parameter *pNode	= Parameters.Add_Node(NULL, "THRESHOLD_OPTIONS", _TL("Threshold Options"), _TL("Options for tuning optimal threshold search"));
	Parameters.Add_Value(pNode, "MIN"	, _TL("Minimum"),
		_TL("This parameter is the lowest end of the range searched for possible threshold values using drop analysis. This technique looks for the smallest threshold in the range where the absolute value of the t-statistic is less than 2. For the science behind the drop analysis see Tarboton et al. (1991, 1992), Tarboton and Ames (2001)."), PARAMETER_TYPE_Double, 5.0);
	Parameters.Add_Value(pNode, "MAX"	, _TL("This parameter is the highest end of the range searched for possible threshold values using drop analysis. This technique looks for the smallest threshold in the range where the absolute value of the t-statistic is less than 2. For the science behind the drop analysis see Tarboton et al. (1991, 1992), Tarboton and Ames (2001)."),
		_TL("Upper bound of range used to search for optimum threshold"), PARAMETER_TYPE_Double, 500.0);
	Parameters.Add_Value(pNode, "NTHRESH"	, _TL("The parameter is the number of steps to divide the search range into when looking for possible threshold values using drop analysis. This technique looks for the smallest threshold in the range where the absolute value of the t-statistic is less than 2. For the science behind the drop analysis see Tarboton et al. (1991, 1992), Tarboton and Ames (2001)."),
		_TL("Number of thresholds used to search for optimum threshold"), PARAMETER_TYPE_Int, 10);


	// CHOICES
	Parameters.Add_Choice(
		pNode, "STEPTYPE"	, _TL("Step Type"),
		_TL("Determines whether logarithmic or linear spacing should be used when looking for possible threshold values using drop ananlysis."),
		CSG_String::Format(SG_T("%s|%s|"),
			_TL("Logarithmic"),
			_TL("Arithmetic")
		), 0
	);

	Parameters.Add_Value(NULL, "NPROC"	, _TL("Number of Processes"), _TL("The number of stripes that the domain will be divided into and the number of MPI parallel processes that will be spawned to evaluate each of the stripes"), PARAMETER_TYPE_Int, SG_Get_Max_Num_Procs_Omp(), 1, true, SG_Get_Max_Num_Procs_Omp(), true);

	// Other
	Parameters.Add_FilePath(NULL, "TEMP_DIR", _TL("Temp File Directory"), _TL("Directory used for storing temporary files during processing."), NULL, DefaultTempDir, false, true, false); 
	
}


bool CDropAnalysis::On_Execute(void)
{
	// Inputs and Output Strings
	CSG_String InputBasename = CSG_String("input");
	CSG_String OutputBasename = CSG_String("output");
	CSG_String FEL_INPUT_FileName, FEL_INPUT_FilePath;
	CSG_String FLOWD8_INPUT_FileName, FLOWD8_INPUT_FilePath;
	CSG_String AREAD8_INPUT_FileName, AREAD8_INPUT_FilePath;
	CSG_String SSA_INPUT_FileName, SSA_INPUT_FilePath;
	CSG_String OUTLET_INPUT_FileName, OUTLET_INPUT_FilePath;
	CSG_String DROPTABLE_OUTPUT_FileName, DROPTABLE_OUTPUT_FilePath, DROPTABLE_Name;

	// Data Objects
	CSG_Grid *FEL_INPUT_Grid, *FLOWD8_INPUT_Grid, *SLOPED8_INPUT_Grid, *AREAD8_INPUT_Grid, *SSA_INPUT_Grid;
	CSG_Shapes *OUTLET_INPUT_Shapes;
	CSG_Table *DROPTABLE_OUTPUT_Table;

	// Misc
	TSG_Data_Type Type;
	CSG_String GDALDriver, sCmd, TempDirPath, TauDEMBinDir, BinaryName, BinaryPath, LogFile;
	CSG_Projection Projection;
	CSG_GDAL_DataSet DataSet;
	double min, max;
	int nthresh, steptype, nproc;
	
	// Grab inputs
	FEL_INPUT_Grid = Parameters("FEL_INPUT")->asGrid();
	FLOWD8_INPUT_Grid = Parameters("FLOWD8_INPUT")->asGrid();
	AREAD8_INPUT_Grid = Parameters("AREAD8_INPUT")->asGrid();
	SSA_INPUT_Grid = Parameters("SSA_INPUT")->asGrid();
	OUTLET_INPUT_Shapes = Parameters("OUTLET_INPUT")->asShapes();
	DROPTABLE_OUTPUT_Table = Parameters("DROPTABLE_OUTPUT")->asTable();

	min = Parameters("MIN")->asDouble();
	max = Parameters("MAX")->asDouble();
	nthresh = Parameters("NTHRESH")->asInt();
	steptype = Parameters("STEPTYPE")->asInt();
	nproc = Parameters("NPROC")->asInt();

	GDALDriver = CSG_String("GTiff");
	Get_Projection(Projection);
	Type = FEL_INPUT_Grid->Get_Type();

	//TempDirPath = SG_File_Get_Path_Absolute(CSG_String("taudem_tmp"));
	TempDirPath = Parameters("TEMP_DIR")->asFilePath()->asString();

	FEL_INPUT_FileName = InputBasename + CSG_String("fel");
	FEL_INPUT_FilePath = SG_File_Make_Path(TempDirPath, FEL_INPUT_FileName, CSG_String("tif")); 

	FLOWD8_INPUT_FileName = InputBasename + CSG_String("p");
	FLOWD8_INPUT_FilePath = SG_File_Make_Path(TempDirPath, FLOWD8_INPUT_FileName, CSG_String("tif"));

	AREAD8_INPUT_FileName = InputBasename + CSG_String("ad8");
	AREAD8_INPUT_FilePath = SG_File_Make_Path(TempDirPath, AREAD8_INPUT_FileName, CSG_String("tif"));

	SSA_INPUT_FileName = InputBasename + CSG_String("ssa");
	SSA_INPUT_FilePath = SG_File_Make_Path(TempDirPath, SSA_INPUT_FileName, CSG_String("tif"));

	OUTLET_INPUT_FileName = InputBasename + CSG_String("o");
	OUTLET_INPUT_FilePath = SG_File_Make_Path(TempDirPath, OUTLET_INPUT_FileName, CSG_String("shp"));

	DROPTABLE_OUTPUT_FileName = OutputBasename + CSG_String("drp");
	DROPTABLE_OUTPUT_FilePath = SG_File_Make_Path(TempDirPath, DROPTABLE_OUTPUT_FileName, CSG_String("txt"));

	LogFile = SG_File_Make_Path(TempDirPath, CSG_String("taudem_log.txt"));
	LogFile = SG_File_Get_Path_Absolute(LogFile);

	TauDEMBinDir = SG_File_Make_Path(CSG_String("bin"), CSG_String("TauDEM"));
	TauDEMBinDir = SG_File_Get_Path_Absolute(TauDEMBinDir);

	// exec commnad
	BinaryName = CSG_String("DropAnalysis"); // D8
	BinaryPath = SG_File_Make_Path(TauDEMBinDir, BinaryName);
	sCmd = CSG_String::Format(SG_T("mpiexec -n %d %s -ad8 %s -p %s -fel %s -ssa %s -o %s -drp %s -par %f %f %d %d > %s 2>&1"), nproc, BinaryPath.c_str(), AREAD8_INPUT_FilePath.c_str(), FLOWD8_INPUT_FilePath.c_str(), FEL_INPUT_FilePath.c_str(), SSA_INPUT_FilePath.c_str(), OUTLET_INPUT_FilePath.c_str(), DROPTABLE_OUTPUT_FilePath.c_str(), min, max, nthresh, steptype, LogFile.c_str());
	DROPTABLE_Name = CSG_String("Drop Analysis");

	// make sure temp dir exists
	if (!SG_Dir_Exists(TempDirPath))
	{
		if (!SG_Dir_Create(TempDirPath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to create temp directory"), TempDirPath.c_str()));
		}
	}

	// Delete old input file if exists
	if (SG_File_Exists(FEL_INPUT_FilePath))
	{
		if (!SG_File_Delete(FEL_INPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous input file: "), FEL_INPUT_FilePath.c_str()));
			return( false );
		}
	}

	// Delete old input file if exists
	if (SG_File_Exists(FLOWD8_INPUT_FilePath))
	{
		if (!SG_File_Delete(FLOWD8_INPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous input file: "), FLOWD8_INPUT_FilePath.c_str()));
			return( false );
		}
	}

	// Delete old input file if exists
	if (SG_File_Exists(AREAD8_INPUT_FilePath))
	{
		if (!SG_File_Delete(AREAD8_INPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous input file: "), AREAD8_INPUT_FilePath.c_str()));
			return( false );
		}
	}

	// Delete old input file if exists
	if (SG_File_Exists(SSA_INPUT_FilePath))
	{
		if (!SG_File_Delete(SSA_INPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous input file: "), SSA_INPUT_FilePath.c_str()));
			return( false );
		}
	}

	// Delete old input file if exists
	if (SG_File_Exists(OUTLET_INPUT_FilePath))
	{
		if (!SG_File_Delete(OUTLET_INPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous input file: "), OUTLET_INPUT_FilePath.c_str()));
			return( false );
		}
	}

	// Delete old output files
	if (SG_File_Exists(DROPTABLE_OUTPUT_FilePath))
	{
		if (!SG_File_Delete(DROPTABLE_OUTPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous output file: "), DROPTABLE_OUTPUT_FilePath.c_str()));
			return( false );
		}
	}

	// SAVE TIFFS

	// save elevation input
	if( !DataSet.Open_Write(FEL_INPUT_FilePath, GDALDriver, CSG_String(""), Type, 1, *Get_System(), Projection) )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), FEL_INPUT_FilePath.c_str()));
		return( false );
	}
	DataSet.Write(0, FEL_INPUT_Grid);
	if( !DataSet.Close() )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to close file after writing: "), FEL_INPUT_FilePath.c_str()));
		return( false );
	}

	// save flowd8 input
	if( !DataSet.Open_Write(FLOWD8_INPUT_FilePath, GDALDriver, CSG_String(""), Type, 1, *Get_System(), Projection) )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), FLOWD8_INPUT_FilePath.c_str()));
		return( false );
	}
	DataSet.Write(0, FLOWD8_INPUT_Grid);
	if( !DataSet.Close() )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to close file after writing: "), FLOWD8_INPUT_FilePath.c_str()));
		return( false );
	}

	// save aread8 input
	if( !DataSet.Open_Write(AREAD8_INPUT_FilePath, GDALDriver, CSG_String(""), Type, 1, *Get_System(), Projection) )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), AREAD8_INPUT_FilePath.c_str()));
		return( false );
	}
	DataSet.Write(0, AREAD8_INPUT_Grid);
	if( !DataSet.Close() )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to close file after writing: "), AREAD8_INPUT_FilePath.c_str()));
		return( false );
	}

	// save ssa input
	if( !DataSet.Open_Write(SSA_INPUT_FilePath, GDALDriver, CSG_String(""), Type, 1, *Get_System(), Projection) )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), SSA_INPUT_FilePath.c_str()));
		return( false );
	}
	DataSet.Write(0, SSA_INPUT_Grid);
	if( !DataSet.Close() )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to close file after writing: "), SSA_INPUT_FilePath.c_str()));
		return( false );
	}

	// save outlet shapefile
	CSG_OGR_DataSource	OGRDataSource;
	CSG_String OGRDriver = CSG_String("ESRI Shapefile");
	if( !OGRDataSource.Create(OUTLET_INPUT_FilePath, OGRDriver) )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), OUTLET_INPUT_FilePath.c_str()));
		return( false );
	}
	OGRDataSource.Write(OUTLET_INPUT_Shapes, OGRDriver);
	OGRDataSource.Destroy();
	

	// Run TauDEM DropAnalysis
	Message_Add(CSG_String("Executing ") + sCmd);
	if (system(sCmd.b_str()) != 0)	
	{
		Error_Set(CSG_String::Format(SG_T("Error executing '%s' see Execution log for details"), BinaryName.c_str()));
		// read log output
		CSG_File File;
		if (File.Open(LogFile, SG_FILE_R, false))
		{
			CSG_String Line;
			while (! File.is_EOF() && File.Read_Line(Line))
			{
				Message_Add(Line);
			}
			File.Close();
		} else 
		{
			Message_Add(CSG_String("Unable to open " + LogFile + CSG_String(" for reading")));
		}

		return( false );
	}

	if (!SG_File_Exists(DROPTABLE_OUTPUT_FilePath))
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Output file does not exist: "), DROPTABLE_OUTPUT_FilePath.c_str()));
		return false;
	} 
	else
	{
		DROPTABLE_OUTPUT_Table->Destroy();
		DROPTABLE_OUTPUT_Table->Set_Name(DROPTABLE_Name);

		// create table fields
		DROPTABLE_OUTPUT_Table->Add_Field(SG_T("Threshold"), SG_DATATYPE_Double);
		DROPTABLE_OUTPUT_Table->Add_Field(SG_T("DrainDen"), SG_DATATYPE_Double);
		DROPTABLE_OUTPUT_Table->Add_Field(SG_T("NoFirstOrd"), SG_DATATYPE_Double);
		DROPTABLE_OUTPUT_Table->Add_Field(SG_T("NoHighOrd"), SG_DATATYPE_Double);
		DROPTABLE_OUTPUT_Table->Add_Field(SG_T("MeanDFirstOrd"), SG_DATATYPE_Double);
		DROPTABLE_OUTPUT_Table->Add_Field(SG_T("MeanDHighOrd"), SG_DATATYPE_Double);
		DROPTABLE_OUTPUT_Table->Add_Field(SG_T("StdDevFirstOrd"), SG_DATATYPE_Double);
		DROPTABLE_OUTPUT_Table->Add_Field(SG_T("StdDevHighOrd"), SG_DATATYPE_Double);
		DROPTABLE_OUTPUT_Table->Add_Field(SG_T("T"), SG_DATATYPE_Double);
		
		// read table data
		CSG_File File;
		if (File.Open(DROPTABLE_OUTPUT_FilePath, SG_FILE_R, false))
		{
			CSG_String Line;
			int count = 0;

			// determine number of lines
			while (! File.is_EOF() && File.Read_Line(Line))
			{
				count++;
			}
			File.Seek_Start();

			// skip header
			File.Read_Line(Line);
			// start reading table data
			for (int i = 1; i < count - 2; i++)
			{
				CSG_Table_Record *Record = DROPTABLE_OUTPUT_Table->Add_Record();
				File.Read_Line(Line);
				Line.Replace(",", SG_T("\t"));

				for (int j = 0; j < DROPTABLE_OUTPUT_Table->Get_Field_Count(); j++)
				{
					Line.Trim();
					Record->Set_Value(j, Line.asDouble());
					Line = Line.AfterFirst('\t');
				}
			}

			// optimum threshold value set as output parameter value
			File.Read_Line(Line);
			File.Close();
			Message_Add(Line);
			DROPTABLE_OUTPUT_Table->Set_Description(Line);

			
		} else 
		{
			Message_Add(CSG_String("Unable to open " + DROPTABLE_OUTPUT_FilePath + CSG_String(" for reading")));
		}
	}

	return( true );

}


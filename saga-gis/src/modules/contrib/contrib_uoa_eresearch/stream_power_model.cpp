/**********************************************************
* Version $Id: stream_power_model.cpp 1925 2014-01-09 12:15:18Z oconrad $
*********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                       contrib_uoa_eresearch                        //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                     stream_power_model.cpp                     //
//                                                       //
//                 Copyright (C) 2007 by                 //
//                        Author                         //
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
//-------------------------------------------------------//
//                                                       //
//    e-mail:     author@email.net                       //
//                                                       //
//    contact:    Author                                 //
//                Sesame Street. 7                       //
//                12345 Metropolis                       //
//                Nirvana                                //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "stream_power_model.h"
#include "gdal_driver.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
Cstream_power_model::Cstream_power_model(void)
{
	Set_Name		(_TL("Stream Power Model"));

	Set_Author		(SG_T("J. D. Pelletier 2008, J. Tunnicliffe and S. Masoud-Ansari 2014-2015"));

	Set_Description	(_TW(
		"Modified C code from Pelletier 2008: Quantitative Modelling of Earth Surface Processes.\n"
		"Converted to C++ for use with SAGA GIS."
		));
	

	// General parameters
	Parameters.Add_Grid(NULL, "INPUT"	, _TL("Input DEM"), _TL("Initial topography"), PARAMETER_INPUT);
	Parameters.Add_Grid_Output(NULL, "OUTPUT", _TL("Output"), _TL("Simulation output"));
	Parameters.Add_String(NULL, "OUTPUT_NAME", _TL("Output Name"), _TL("Output Name"), "StreamPowerModelOutput");
	Parameters.Add_FilePath(NULL, "OUTPUT_DIR", _TL("Output Directory"), _TL("Directory used for saving regular GeoTIFF snaphots of the model as it progresses."), NULL, NULL, false, true, false); 
	

	//m_Grid_Target.Create(&Parameters);
	CSG_Parameter	*pNode;

	Parameters.Add_Choice(
		NULL	, "METHOD"		, _TL("Parameter Type"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|"),
			_TL("Scalar"),
			_TL("Grid")
		), 0
	);



	// Scalar parameters
	pNode	= Parameters.Add_Node(
		NULL	, "NODE_SCALAR", _TL("Scalar"),
		_TL("")
	);
	
	Parameters.Add_Value(pNode, "U_SCALAR", _TL("Uplift (per kyrs)"), _TL("Uplift rate per kyrs"), PARAMETER_TYPE_Double, 1, 0, true);
	Parameters.Add_Value(pNode, "K_SCALAR", _TL("Diffusion (per kyrs)"), _TL("Diffusion rate per kyrs"), PARAMETER_TYPE_Double, 30, 0, true);

	// Grid parameters
	pNode	= Parameters.Add_Node(
		NULL	, "NODE_GRID", _TL("Grid"),
		_TL("")
	);

	Parameters.Add_Grid(pNode, "U_GRID"	, _TL("Uplift (per kyrs)"), _TL("Uplift field, rate per kyrs"), PARAMETER_OPTIONAL);
	Parameters.Add_Grid(pNode, "K_GRID"	, _TL("Diffusion (per kyrs)"), _TL("Diffusion field, rate per kyrs"), PARAMETER_OPTIONAL);
	
	Parameters.Add_Value(NULL, "T", _TL("Timestep (kyrs)"), _TL("Timestep in thousands of years. A lower timestep will produce more accurate results but can take longer to run."), PARAMETER_TYPE_Double, 0.001, 0, true);
	Parameters.Add_Value(NULL, "DURATION", _TL("Duration (kyrs)"), _TL("Duration in kyrs"), PARAMETER_TYPE_Double, 1, 0, true);
	Parameters.Add_Value(NULL, "OUTPUT_FREQUENCY", _TL("Output Frequency (yrs)"), _TL("Frequency at which to save output in years"), PARAMETER_TYPE_Int, 10, 0, true);
	Parameters.Add_Value(
		NULL, "SCALE_TIMESTEP"		, _TL("Auto-scale Timestep"),
		_TL("Enabling this option can lead to more accurate results but will take significantly longer"),
		PARAMETER_TYPE_Bool, false
	);		

}

int Cstream_power_model::On_Parameter_Changed(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	return( m_Grid_Target.On_Parameter_Changed(pParameters, pParameter) ? 1 : 0 );
}

int Cstream_power_model::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if( !SG_STR_CMP(pParameter->Get_Identifier(), SG_T("METHOD")) )
	{
		pParameters->Get_Parameter("NODE_SCALAR")->Set_Enabled(pParameter->asInt() == 0);
		pParameters->Get_Parameter("NODE_GRID"  )->Set_Enabled(pParameter->asInt() == 1);
	}

	return( m_Grid_Target.On_Parameters_Enable(pParameters, pParameter) ? 1 : 0 );
}

void Cstream_power_model::VectorToGrid(std::vector<std::vector<double>> arr, CSG_Grid* grid)
{
	for (int i = 0; i < grid->Get_NX(); i++)
	{
		for (int j = 0; j < grid->Get_NY(); j++)
		{
			grid->Set_Value(i, j, arr[i][j]);
		}
	}
	
}

std::vector<std::vector<double>> Cstream_power_model::GridToVector(CSG_Grid* grid)
{
	std::vector<std::vector<double>> arr = std::vector<std::vector<double>>(grid->Get_NX(), std::vector<double>(grid->Get_NY()));

	for (int i = 0; i < grid->Get_NX(); i++)
	{
		for (int j = 0; j < grid->Get_NY(); j++)
		{
			arr[i][j] = grid->asDouble(i, j);
		}
	}

	return arr;

}

bool Cstream_power_model::On_Execute(void)
{
	bool save_snapshots = true;
	CSG_String outputDir = Parameters("OUTPUT_DIR")->asFilePath()->asString();
	if (outputDir.is_Empty())
	{
		save_snapshots = false;
		if(!Message_Dlg_Confirm(CSG_String::Format(SG_T("No output directory specified, continue anyway?")), _TL("Warning")))
		{
			return false;
		}
	}

	input = Parameters("INPUT")->asGrid();
	bool scalar = false;
	int output_freq = Parameters("OUTPUT_FREQUENCY")->asInt();

	//Message_Add(Parameters("METHOD")->asChoice()->asString());
	//Message_Add(CSG_String::Format(SG_T("%d method"),Parameters("METHOD")->asChoice()->asInt()));
	
	// switch based on Grid or Scalar input selection
	if (Parameters("METHOD")->asChoice()->asInt() == 0)
	{
		scalar = true;
		u_scalar_input = Parameters("U_SCALAR")->asDouble();
		k_scalar_input = Parameters("K_SCALAR")->asDouble();		
	}
	else
	{
		u_grid_input = Parameters("U_GRID")->asGrid();
		k_grid_input = Parameters("K_GRID")->asGrid();

	}

	
	output = SG_Create_Grid(SG_DATATYPE_Float, input->Get_NX(), input->Get_NY(), input->Get_Cellsize(), input->Get_XMin(), input->Get_YMin());
	output->Set_Name(Parameters("OUTPUT_NAME")->asString());
	Parameters("OUTPUT")->Set_Value(output);
	output->Assign(0.0);
	DataObject_Update(output, true);

	StreamErosionModelParameters p;
	p.timestep = Parameters("T")->asDouble();
	p.duration = Parameters("DURATION")->asDouble();
	p.scale_timestep = Parameters("SCALE_TIMESTEP")->asBool();

	StreamPower sp = StreamPower(p);
	sp.Init(input->Get_NX(), input->Get_NY(), input->Get_XMin(), input->Get_YMin(), input->Get_Cellsize(), input->Get_NoData_Value());
	sp.SetTopo(GridToVector(input));

	if (scalar)
	{
		sp.SetU(u_scalar_input);
		sp.SetK(k_scalar_input);
	}
	else 
	{
		sp.SetU(GridToVector(u_grid_input));
		sp.SetK(GridToVector(k_grid_input));
	}


	VectorToGrid(sp.GetTopo(), output);
	DataObject_Update(output, true);
	
	unsigned long lyrs = 0;
	CSG_String ofname = CSG_String::Format(SG_T("%s_%lu_years"), output->Get_Name(), lyrs);
	CSG_String outputPath = SG_File_Make_Path(outputDir, ofname, CSG_String("tif")); 
	if (save_snapshots)
	{
		if(!ExportGrid(output, outputPath))
		{
			return false;
		}
	}
	

	Process_Set_Text(CSG_String::Format(SG_T("%lu years"), lyrs));
	unsigned long next_output_time = output_freq;
	while (Process_Get_Okay(true) && sp.time < sp.duration)
	{
		sp.Step();
		lyrs = (unsigned long)(sp.time * 1000);
		Process_Set_Text(CSG_String::Format(SG_T("%lu years"), lyrs));
		Set_Progress( (sp.time / sp.duration) * 100 );
		VectorToGrid(sp.GetTopo(), output);
		DataObject_Update(output, true);

		// save state
		if (lyrs == next_output_time)
		{
			ofname = CSG_String::Format(SG_T("%s_%lu_years"), output->Get_Name(), lyrs);
			outputPath = SG_File_Make_Path(outputDir, ofname, CSG_String("tif")); 
			if (save_snapshots)
			{
				if(!ExportGrid(output, outputPath))
				{
					return false;
				}	
			}
			next_output_time += output_freq;
		}
	}
	
	
	return( true );
}

bool Cstream_power_model::ExportGrid(CSG_Grid* grid, CSG_String path)
{
	Message_Add(path);

	
	// GDAL related options
	TSG_Data_Type type = grid->Get_Type();
	CSG_String driver = CSG_String("GTiff");
	CSG_String options = "";
	CSG_Projection prj; 	Get_Projection(prj);
	CSG_GDAL_DataSet dataset;
	

	if( !dataset.Open_Write(path, driver, options, type, 1, grid->Get_System(), prj) )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), path.c_str()));
		return false;
	}
	dataset.Write(0, grid);
	
	if( !dataset.Close() )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to close file after writing: "), path.c_str()));
		return false;
	}
	
	return true;
}

Cstream_power_model::~Cstream_power_model(void)
{

}
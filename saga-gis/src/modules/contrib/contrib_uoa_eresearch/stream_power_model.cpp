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

	CSG_Parameter	*pNode;
	//m_Grid_Target.Create(&Parameters);

	Parameters.Add_Choice(
		NULL	, "METHOD"		, _TL("Parameter Type"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|"),
			_TL("Scalar"),
			_TL("Grid")
		), 1
	);



	// Scalar parameters
	pNode	= Parameters.Add_Node(
		NULL	, "NODE_SCALAR", _TL("Scalar"),
		_TL("")
	);
	
	Parameters.Add_Value(pNode, "U_SCALAR", _TL("Uplift"), _TL("Uplift field, rate per kyrs"), PARAMETER_TYPE_Double, 0.05, 0, true);
	Parameters.Add_Value(pNode, "K_SCALAR", _TL("Diffusion"), _TL("Diffusion parameter kyr^-1"), PARAMETER_TYPE_Double, 0.05, 0, true);

	// Grid parameters
	pNode	= Parameters.Add_Node(
		NULL	, "NODE_GRID", _TL("Grid"),
		_TL("")
	);

	Parameters.Add_Grid(pNode, "U_GRID"	, _TL("Uplift"), _TL("Uplift field, rate per kyrs"), PARAMETER_OPTIONAL);
	Parameters.Add_Grid(pNode, "K_GRID"	, _TL("Diffusion"), _TL("Diffusion field, rate per kyrs"), PARAMETER_OPTIONAL);
	
	Parameters.Add_Value(NULL, "T", _TL("Timestep"), _TL("Timestep in kyrs"), PARAMETER_TYPE_Double, 1.0, 0, true);
	Parameters.Add_Value(NULL, "DURATION", _TL("Duration"), _TL("Duration in kyrs"), PARAMETER_TYPE_Double, 1, 0, true);
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
	input = Parameters("INPUT")->asGrid();
	bool scalar = false;

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
	output->Set_Name(_TL("Output"));
	Parameters("OUTPUT")->Set_Value(output);
	output->Assign(0.0);
	DataObject_Update(output, true);

	StreamErosionModelParameters p;
	p.timestep = Parameters("T")->asDouble();
	p.duration = Parameters("DURATION")->asDouble();

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
	
	Process_Set_Text(CSG_String::Format(SG_T("%f kyrs"), sp.time));
	while (Process_Get_Okay(true) && sp.time < sp.duration)
	{
		sp.Step();
		Process_Set_Text(CSG_String::Format(SG_T("%f kyrs"), sp.time));
		Set_Progress( (sp.time / sp.duration) * 100 );
		VectorToGrid(sp.GetTopo(), output);
		DataObject_Update(output, true);

	}
	
	
	return( true );
}

Cstream_power_model::~Cstream_power_model(void)
{

}
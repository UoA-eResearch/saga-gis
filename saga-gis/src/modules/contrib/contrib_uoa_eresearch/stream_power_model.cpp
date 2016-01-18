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

	Parameters.Add_Grid(NULL, "INPUT"	, _TL("Input DEM"), _TL("Initial topography"), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "U"	, _TL("U"), _TL("Uplift topography"), PARAMETER_INPUT);
	Parameters.Add_Grid_Output(NULL, "OUTPUT", _TL("Output"), _TL("Simulation output"));
	
	Parameters.Add_Value(NULL, "K", _TL("Diffusion"), _TL("Diffusion parameter kyr^-1"), PARAMETER_TYPE_Double, 0.05, 0, true);
	Parameters.Add_Value(NULL, "T", _TL("Timestep"), _TL("Timestep in kyrs"), PARAMETER_TYPE_Double, 1.0, 0, true);
	Parameters.Add_Value(NULL, "DURATION", _TL("Duration"), _TL("Duration in kyrs"), PARAMETER_TYPE_Double, 1, 0, true);
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
	uinput = Parameters("U")->asGrid();

	output = SG_Create_Grid(SG_DATATYPE_Float, input->Get_NX(), input->Get_NY(), input->Get_Cellsize(), input->Get_XMin(), input->Get_YMin());
	output->Set_Name(_TL("Output"));
	Parameters("OUTPUT")->Set_Value(output);
	output->Assign(0.0);
	DataObject_Update(output, true);

	StreamErosionModelParameters p;
	p.K = Parameters("K")->asDouble();
	p.timestep = Parameters("DURATION")->asDouble();
	p.duration = Parameters("DURATION")->asDouble();

	StreamPower sp = StreamPower(p);
	sp.Init(input->Get_NX(), input->Get_NY(), input->Get_XMin(), input->Get_YMin(), input->Get_Cellsize(), input->Get_NoData_Value());
	sp.SetTopo(GridToVector(input));
	sp.SetU(1.0f); // TODO
	VectorToGrid(sp.GetTopo(), output);
	DataObject_Update(output, true);
	Message_Add(CSG_String::Format(SG_T("%f, %f, %f"), sp.topo[0][0], sp.topo[12][12], sp.topo[56][56]));
	
	while (sp.time < sp.duration)
	{
		Process_Set_Text(CSG_String::Format(SG_T("Time: %f"), sp.time));
		if (Process_Get_Okay(true))
		{
			sp.Step();
			VectorToGrid(sp.GetTopo(), output);
			DataObject_Update(output, true);

		}
	}
	
	return( true );
}

Cstream_power_model::~Cstream_power_model(void)
{

}
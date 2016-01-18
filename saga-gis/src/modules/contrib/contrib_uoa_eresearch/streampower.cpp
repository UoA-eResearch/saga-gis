#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

#include "streampower.h"
#include "utility.h"
#include "priority_flood.hpp"

void StreamPower::Tridag(std::vector<double>& a, std::vector<double>& b, std::vector<double>& c, std::vector<double>& r, std::vector<double>& u, int n)
{
	unsigned long j;
	double bet;
	std::vector<double> gam(n);

	u[0] = r[0] / (bet = b[0]);
	for (j = 1; j < n; j++)
	{
		gam[j] = c[j - 1] / bet;
		bet = b[j] - a[j] * gam[j];
		u[j] = (r[j] - a[j] * u[j - 1]) / bet;
	}
	for (j = (n - 2); j > 0; j--)
	{
		u[j] -= gam[j + 1] * u[j + 1];
	}

}

void StreamPower::SetupGridNeighbors()
{
	int i, j;

	idown = std::vector<int>(lattice_size_x);
	iup = std::vector<int>(lattice_size_x);
	jup = std::vector<int>(lattice_size_y);
	jdown = std::vector<int>(lattice_size_y);

	for (i = 0; i < lattice_size_x; i++)
	{
		idown[i] = i - 1;
		iup[i] = i + 1;
	}
	idown[0] = 0;
	iup[lattice_size_x - 1] = lattice_size_x - 1;
	for (j = 0; j < lattice_size_y; j++)
	{
		jdown[j] = j - 1;
		jup[j] = j + 1;
	}
	jdown[0] = 0;
	jup[lattice_size_y - 1] = lattice_size_y - 1;
}

void StreamPower::Flood()
{
	// update elev
	for (int i = 0; i < lattice_size_x; i++)
	{
		for (int j = 0; j < lattice_size_y; j++)
		{
			elevation(i, j) = topo[i][j];
		}
	}

	// perform flooding
	original_priority_flood(elevation);

	// update topo
	for (int i = 0; i < lattice_size_x; i++)
	{
		for (int j = 0; j < lattice_size_y; j++)
		{
			topo[i][j] = elevation(i, j);
		}
	}

}

void StreamPower::MFDFlowRoute(int i, int j)
{
	double tot;

	tot = 0;
	if (topo[i][j] > topo[iup[i]][j])
		tot += pow(topo[i][j] - topo[iup[i]][j], 1.1);
	if (topo[i][j] > topo[idown[i]][j])
		tot += pow(topo[i][j] - topo[idown[i]][j], 1.1);
	if (topo[i][j] > topo[i][jup[j]])
		tot += pow(topo[i][j] - topo[i][jup[j]], 1.1);
	if (topo[i][j] > topo[i][jdown[j]])
		tot += pow(topo[i][j] - topo[i][jdown[j]], 1.1);
	if (topo[i][j] > topo[iup[i]][jup[j]])
		tot += pow((topo[i][j] - topo[iup[i]][jup[j]])*oneoversqrt2, 1.1);
	if (topo[i][j] > topo[iup[i]][jdown[j]])
		tot += pow((topo[i][j] - topo[iup[i]][jdown[j]])*oneoversqrt2, 1.1);
	if (topo[i][j] > topo[idown[i]][jup[j]])
		tot += pow((topo[i][j] - topo[idown[i]][jup[j]])*oneoversqrt2, 1.1);
	if (topo[i][j] > topo[idown[i]][jdown[j]])
		tot += pow((topo[i][j] - topo[idown[i]][jdown[j]])*oneoversqrt2, 1.1);
	if (topo[i][j] > topo[iup[i]][j])
		flow1[i][j] = pow(topo[i][j] - topo[iup[i]][j], 1.1) / tot;
	else flow1[i][j] = 0;
	if (topo[i][j] > topo[idown[i]][j])
		flow2[i][j] = pow(topo[i][j] - topo[idown[i]][j], 1.1) / tot;
	else flow2[i][j] = 0;
	if (topo[i][j] > topo[i][jup[j]])
		flow3[i][j] = pow(topo[i][j] - topo[i][jup[j]], 1.1) / tot;
	else flow3[i][j] = 0;
	if (topo[i][j] > topo[i][jdown[j]])
		flow4[i][j] = pow(topo[i][j] - topo[i][jdown[j]], 1.1) / tot;
	else flow4[i][j] = 0;
	if (topo[i][j] > topo[iup[i]][jup[j]])
		flow5[i][j] = pow((topo[i][j] - topo[iup[i]][jup[j]])*oneoversqrt2, 1.1) / tot;
	else flow5[i][j] = 0;
	if (topo[i][j] > topo[iup[i]][jdown[j]])
		flow6[i][j] = pow((topo[i][j] - topo[iup[i]][jdown[j]])*oneoversqrt2, 1.1) / tot;
	else flow6[i][j] = 0;
	if (topo[i][j] > topo[idown[i]][jup[j]])
		flow7[i][j] = pow((topo[i][j] - topo[idown[i]][jup[j]])*oneoversqrt2, 1.1) / tot;
	else flow7[i][j] = 0;
	if (topo[i][j] > topo[idown[i]][jdown[j]])
		flow8[i][j] = pow((topo[i][j] - topo[idown[i]][jdown[j]])*oneoversqrt2, 1.1) / tot;
	else flow8[i][j] = 0;
	flow[iup[i]][j] += flow[i][j] * flow1[i][j];
	flow[idown[i]][j] += flow[i][j] * flow2[i][j];
	flow[i][jup[j]] += flow[i][j] * flow3[i][j];
	flow[i][jdown[j]] += flow[i][j] * flow4[i][j];
	flow[iup[i]][jup[j]] += flow[i][j] * flow5[i][j];
	flow[iup[i]][jdown[j]] += flow[i][j] * flow6[i][j];
	flow[idown[i]][jup[j]] += flow[i][j] * flow7[i][j];
	flow[idown[i]][jdown[j]] += flow[i][j] * flow8[i][j];
}

void StreamPower::CalculateAlongChannelSlope(int i, int j)
{
	double down;

	down = 0;
	if (topo[iup[i]][j] - topo[i][j] < down) down = topo[iup[i]][j] - topo[i][j];
	if (topo[idown[i]][j] - topo[i][j] < down) down = topo[idown[i]][j] - topo[i][j];
	if (topo[i][jup[j]] - topo[i][j] < down) down = topo[i][jup[j]] - topo[i][j];
	if (topo[i][jdown[j]] - topo[i][j] < down) down = topo[i][jdown[j]] - topo[i][j];
	if ((topo[iup[i]][jup[j]] - topo[i][j])*oneoversqrt2 < down)
		down = (topo[iup[i]][jup[j]] - topo[i][j])*oneoversqrt2;
	if ((topo[idown[i]][jup[j]] - topo[i][j])*oneoversqrt2 < down)
		down = (topo[idown[i]][jup[j]] - topo[i][j])*oneoversqrt2;
	if ((topo[iup[i]][jdown[j]] - topo[i][j])*oneoversqrt2 < down)
		down = (topo[iup[i]][jdown[j]] - topo[i][j])*oneoversqrt2;
	if ((topo[idown[i]][jdown[j]] - topo[i][j])*oneoversqrt2 < down)
		down = (topo[idown[i]][jdown[j]] - topo[i][j])*oneoversqrt2;
	slope[i][j] = fabs(down) / deltax;
}

void StreamPower::HillSlopeDiffusion()
{
	int i, j, count;
	double term1;

	ax = std::vector<double>(lattice_size_x);
	ay = std::vector<double>(lattice_size_y);
	bx = std::vector<double>(lattice_size_x);
	by = std::vector<double>(lattice_size_y);
	cx = std::vector<double>(lattice_size_x);
	cy = std::vector<double>(lattice_size_y);
	ux = std::vector<double>(lattice_size_x);
	uy = std::vector<double>(lattice_size_y);
	rx = std::vector<double>(lattice_size_x);
	ry = std::vector<double>(lattice_size_y);

	D = 10000000.0;
	count = 0;
	term1 = D / (deltax * deltax);
	for (i = 0; i < lattice_size_x; i++)
	{
		ax[i] = -term1;
		cx[i] = -term1;
		bx[i] = 4 * term1 + 1;
		if (i == 0)
		{
			bx[i] = 1;
			cx[i] = 0;
		}
		if (i == lattice_size_x - 1)
		{
			bx[i] = 1;
			ax[i] = 0;
		}
	}

	for (j = 0; j < lattice_size_y; j++)
	{
		ay[j] = -term1;
		cy[j] = -term1;
		by[j] = 4 * term1 + 1;
		if (j == 0)
		{
			by[j] = 1;
			cy[j] = 0;
		}
		if (j == lattice_size_y - 1)
		{
			by[j] = 1;
			ay[j] = 0;
		}
	}

	while (count < 5)
	{
		count++;
		for (i = 0; i < lattice_size_x; i++)
		{
			for (j = 0; j < lattice_size_y; j++)
			{
				topo2[i][j] = topo[i][j];
			}
		}

		for (i = 0; i < lattice_size_x; i++)
		{
			for (j = 0; j < lattice_size_y; j++)
			{
				ry[j] = term1*(topo[iup[i]][j] + topo[idown[i]][j]) + topoold[i][j];
				if (j == 0)
				{
					ry[j] = topoold[i][j];
				}
				if (j == lattice_size_y - 1)
				{
					ry[j] = topoold[i][j];
				}

			}
			Tridag(ay, by, cy, ry, uy, lattice_size_y);
			for (j = 0; j < lattice_size_y; j++)
			{
				topo[i][j] = uy[j];
			}
		}



		for (i = 0; i < lattice_size_x; i++)
		{
			for (j = 0; j < lattice_size_y; j++)
			{
				topo2[i][j] = topo[i][j];
			}
		}

		for (j = 0; j < lattice_size_y; j++)
		{
			for (i = 0; i < lattice_size_x; i++)
			{
				rx[i] = term1*(topo[i][jup[j]] + topo[i][jdown[j]]) + topoold[i][j];
				if (i == 0)
				{
					rx[i] = topoold[i][j];
				}
				if (i == lattice_size_x - 1)
				{
					rx[i] = topoold[i][j];
				}

			}

			//std::cout << "j: " << j << "\n";
			Tridag(ax, bx, cx, rx, ux, lattice_size_x);
			//if (j == 1)
			//{
			//	return;
			//}
			for (i = 0; i < lattice_size_x; i++)
			{
				topo[i][j] = ux[i];
				//std::cout << ux[i] << "\n";
			}

		}

	}
}

void StreamPower::Avalanche(int i, int j)
{
	if (topo[iup[i]][j] - topo[i][j] > thresh)
		topo[iup[i]][j] = topo[i][j] + thresh;
	if (topo[idown[i]][j] - topo[i][j] > thresh)
		topo[idown[i]][j] = topo[i][j] + thresh;
	if (topo[i][jup[j]] - topo[i][j] > thresh)
		topo[i][jup[j]] = topo[i][j] + thresh;
	if (topo[i][jdown[j]] - topo[i][j] > thresh)
		topo[i][jdown[j]] = topo[i][j] + thresh;
	if (topo[iup[i]][jup[j]] - topo[i][j] > (thresh*sqrt2))
		topo[iup[i]][jup[j]] = topo[i][j] + thresh*sqrt2;
	if (topo[iup[i]][jdown[j]] - topo[i][j] > (thresh*sqrt2))
		topo[iup[i]][jdown[j]] = topo[i][j] + thresh*sqrt2;
	if (topo[idown[i]][jup[j]] - topo[i][j] > (thresh*sqrt2))
		topo[idown[i]][jup[j]] = topo[i][j] + thresh*sqrt2;
	if (topo[idown[i]][jdown[j]] - topo[i][j] > (thresh*sqrt2))
		topo[idown[i]][jdown[j]] = topo[i][j] + thresh*sqrt2;
}

void StreamPower::Start()
{
	char fname[100];
	sprintf(fname, "C:/outputs/saga_erosion_initial.asc");
	PrintState(fname);

	time = 0;
	while (time < duration)
	{
		Step();
	}
}

void StreamPower::Step()
{
	
	int i, j, t;
	double max, deltah;

	//perform landsliding
	for (j = 0; j < lattice_size_y; j++)
	{
		for (i = 0; i < lattice_size_x; i++)
		{
			topovec[j * lattice_size_x + i] = topo[i][j];
		}
	}
	topovecind = Indexx(topovec);


	// todo
	for (int t = 0; t < lattice_size_x * lattice_size_y; t++)
	{
		i = topovecind[t] % lattice_size_x;
		j = topovecind[t] / lattice_size_x;
		Avalanche(i, j);
	}

	for (j = 0; j < lattice_size_y; j++)
	{
		for (i = 0; i < lattice_size_x; i++)
		{
			topoold[i][j] = topo[i][j];
		}
	}

	Flood();

	double e1, e2, e3;
	e1  = elevation(0,0);
	e2  = elevation(10,10);
	e3  = elevation(50,50);

	for (j = 0; j < lattice_size_y; j++)
	{
		for (i = 0; i < lattice_size_x; i++)
		{
			flow[i][j] = 1;
			topovec[j * lattice_size_x + i] = topo[i][j];
		}
	}

	topovecind = Indexx(topovec);

	for (t = lattice_size_x * lattice_size_y - 1; t >= 0; t--)
	{
		i = topovecind[t] % lattice_size_x;
		j = topovecind[t] / lattice_size_x;
		MFDFlowRoute(i, j);
	}


	// perform uplift
	for (i = 1; i < lattice_size_x - 1; i++)
	{
		for (j = 1; j < lattice_size_y - 1; j++)
		{
			topo[i][j] += U[i][j] * timestep; // u(i,j)
			topoold[i][j] += U[i][j] * timestep;
		}
	}



	//perform upwind erosion
	max = 0;
	for (i = 1; i < lattice_size_x - 1; i++)
	{
		for (j = 1; j < lattice_size_y - 1; j++)
		{
			CalculateAlongChannelSlope(i, j);

			//double sflw = sqrt(flow[i][j]);
			//double slij = slope[i][j];
			deltah = timestep * K * sqrt(flow[i][j]) * deltax * slope[i][j];


			topo[i][j] -= deltah;

			if (topo[i][j] < 0)
			{
				topo[i][j] = 0;
			}
			if (K * sqrt(flow[i][j]) * deltax > max)
			{
				max = K * sqrt(flow[i][j]) * deltax;
			}
		}
	}



	time += timestep;
	if (max > 0.3 * deltax / timestep)
	{
		time -= timestep;
		timestep /= 2.0;
		//std::cout << "First time modification" << "\n";
		for (i = 1; i < lattice_size_x - 1; i++)
		{
			for (j = 1; j < lattice_size_y - 1; j++)
			{
				topo[i][j] = topoold[i][j] - U[i][j] * timestep;
			}
		}
	}
	else
	{
		if (max < 0.03 * deltax / timestep)
		{
			timestep *= 1.2;
			//std::cout << "Second time modification" << "\n";
		}
		for (j = 0; j < lattice_size_y; j++)
		{
			for (i = 0; i < lattice_size_x; i++)
			{
				topoold[i][j] = topo[i][j];
			}

		}

	}
	//if (time > printinterval)
	//{
	//char fname[100];
	//sprintf(fname, "C:/outputs/saga_erosion_%f.asc", time);
	//PrintState(fname);
	//printinterval += printstep;
	//}
	std::cout << "Time: " << time << std::endl;
}

void StreamPower::PrintState(char* fname)
{
	std::ofstream file;
	file.open(fname);
	// write arcgrid format
	file << "ncols " << lattice_size_y << std::endl;
	file << "nrows " << lattice_size_x << std::endl;
	file << "xllcorner " << xllcorner << std::endl;
	file << "yllcorner " << yllcorner << std::endl;
	file << "cellsize " << deltax << std::endl;
	file << "NODATA_value " << nodata << std::endl;
	for (int i = 0; i < lattice_size_x; i++)
	{
		for (int j = 0; j < lattice_size_y; j++)
		{
			file << topo[i][j] << " ";
		}
		file << std::endl;
	}
	file.close();
}

void StreamPower::SetTopo(std::vector<std::vector<double>> t)
{
	
	thresh = 0.58*deltax; // this should be somewhere else

	AssignVariables();
	SetupGridNeighbors();
	SetInitialValues(t);
	//InitDiffusion();

}

void StreamPower::SetU(std::vector<std::vector<double>> u)
{
	for (int i = 0; i < lattice_size_x; i++)
	{
		for (int j = 0; j < lattice_size_y; j++)
		{
			U[i][j] = u[i][j];
		}
	}

}

void StreamPower::SetU(double u)
{
	for (int i = 0; i < lattice_size_x; i++)
	{
		for (int j = 0; j < lattice_size_y; j++)
		{
			U[i][j] = u;
		}
	}

}

void StreamPower::AssignVariables()
{
	topo = std::vector<std::vector<double>>(lattice_size_x, std::vector<double>(lattice_size_y));
	topo2 = std::vector<std::vector<double>>(lattice_size_x, std::vector<double>(lattice_size_y));
	topoold = std::vector<std::vector<double>>(lattice_size_x, std::vector<double>(lattice_size_y));
	slope = std::vector<std::vector<double>>(lattice_size_x, std::vector<double>(lattice_size_y));
	flow = std::vector<std::vector<double>>(lattice_size_x, std::vector<double>(lattice_size_y));
	flow1 = std::vector<std::vector<double>>(lattice_size_x, std::vector<double>(lattice_size_y));
	flow2 = std::vector<std::vector<double>>(lattice_size_x, std::vector<double>(lattice_size_y));
	flow3 = std::vector<std::vector<double>>(lattice_size_x, std::vector<double>(lattice_size_y));
	flow4 = std::vector<std::vector<double>>(lattice_size_x, std::vector<double>(lattice_size_y));
	flow5 = std::vector<std::vector<double>>(lattice_size_x, std::vector<double>(lattice_size_y));
	flow6 = std::vector<std::vector<double>>(lattice_size_x, std::vector<double>(lattice_size_y));
	flow7 = std::vector<std::vector<double>>(lattice_size_x, std::vector<double>(lattice_size_y));
	flow8 = std::vector<std::vector<double>>(lattice_size_x, std::vector<double>(lattice_size_y));
	U = std::vector<std::vector<double>>(lattice_size_x, std::vector<double>(lattice_size_y));
	topovec = std::vector<double>(lattice_size_x * lattice_size_y);
	topovecind = std::vector<int>(lattice_size_x * lattice_size_y);
	elevation = Array2D<double>(lattice_size_x, lattice_size_y, nodata);
	
}

void StreamPower::SetInitialValues(std::vector<std::vector<double>> t)
{
	for (int i = 0; i < lattice_size_x; i++)
	{
		for (int j = 0; j < lattice_size_y; j++)
		{
			topo[i][j] = t[i][j];
			topoold[i][j] = topo[i][j];
			flow[i][j] = 1;
		}
	}
}

void StreamPower::InitDiffusion()
{
	//construct diffusional landscape for initial flow routing
	for (int step = 0; step < 10; step++)
	{
		HillSlopeDiffusion();
		for (int i = 1; i < lattice_size_x - 1; i++)
		{
			for (int j = 1; j < lattice_size_y - 1; j++)
			{
				topo[i][j] += 0.1;
				topoold[i][j] += 0.1;
			}
		}
	}
}

void StreamPower::Init(int nx, int ny, double xllcorner, double yllcorner, double deltax, double nodata)
{
	lattice_size_y = ny;
	lattice_size_x = nx;
	StreamPower::xllcorner = xllcorner;
	StreamPower::yllcorner = yllcorner;
	StreamPower::deltax = deltax;
	StreamPower::nodata = nodata;

	thresh = 0.58*deltax;

}

std::vector<std::vector<double>> StreamPower::ReadArcInfoASCIIGrid(char* fname)
{
	std::ifstream in(fname);
	std::vector<std::vector<double>> t;
	std::string line;

	Util::Warning("Reading DEM without any checks or guarantees ...");

	int nx, ny;
	double xc, yc, dx, nd;

	// read 6 lines of metadata
	std::string key;
	in >> key; in >> nx; // ncols
	in >> key; in >> ny; // nrows
	in >> key; in >> xc;
	in >> key; in >> yc;
	in >> key; in >> dx;
	in >> key; in >> nd;

	Init(nx, ny, xc, yc, dx, nd);

	t = std::vector<std::vector<double>>(lattice_size_x, std::vector<double>(lattice_size_y));

	// read data
	for (int x = 0; x < lattice_size_x; x++)
	{
		for (int y = 0; y < lattice_size_y; y++)
		{
			in >> t[x][y];
		}
	}

	Util::Info("Done reading DEM");

	return t;
}

std::vector<std::vector<double>> StreamPower::GetTopo()
{
	return topo;
}

std::vector<std::vector<double>> StreamPower::CreateRandomField()
{
	std::vector<std::vector<double>> mat(lattice_size_x, std::vector<double>(lattice_size_y));

	std::default_random_engine generator;
	std::normal_distribution<double> distribution(0.0f, 1.0f);
	for (int i = 0; i < lattice_size_x; i++)
	{
		for (int j = 0; j < lattice_size_y; j++)
		{
			mat[i][j] = 0.5 * Gasdev(generator, distribution);
		}
	}
	return mat;
}

std::vector<int> StreamPower::Indexx(std::vector<double>& arr)
{
	return SortIndices(arr);
}

double StreamPower::Ran3(std::default_random_engine& generator, std::uniform_real_distribution<double>& distribution)
{
	return distribution(generator);
}

double StreamPower::Gasdev(std::default_random_engine& generator, std::normal_distribution<double>& distribution)
{
	/*
	Assuming this is the same code from here: http://www.stat.berkeley.edu/~paciorek/diss/code/regression.binomial/gasdev.C
	We need to return a standard, normally distributed gaussian random number
	*/

	return distribution(generator);

}

StreamPower::StreamPower(StreamErosionModelParameters p)
{
	// defaults
	printstep = 1;
	printinterval = printstep;
	deltax = 200.0;       // m
	nodata = -9999.0;
	xllcorner = 0;
	yllcorner = 0;
	time = 0;

	// user params
	timestep = p.timestep;
	duration = p.duration;
	K = p.K;

}

StreamPower::~StreamPower() {}


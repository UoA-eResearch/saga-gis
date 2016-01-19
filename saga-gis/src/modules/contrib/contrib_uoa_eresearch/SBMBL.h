#ifndef HEADER_INCLUDED__SBMBL_H
#define HEADER_INCLUDED__SBMBL_H

//---------------------------------------------------------
#include "MLB_Interface.h"
#include <vector>

//#define INITIAL_DEPTH 21.25     /* meters !!!remember there is a 25*cell_height elevation to be subtracted to have the effective water depth */

//#define VELOCITY_MEAN 0.1414      /* for use with GenRandomGaussian  x and y vels, m/s. 0.07071-> |v|=0.1 */
//#define VELOCITY_SIGMA 0.04     /* for use with GenRandomGaussian */
/* WARNING! VELOCITY_MEAN NEEDS TO BE > 3*VELOCITY_SIGMA */

//#define WAVEHEIGHT_MEAN 2.0    /*(meters) for use with GenRandomGaussian */
//#define WAVEHEIGHT_SIGMA 0.0  /* 	for use with GenRandomGaussian  */
/* WARNING! WAVEHEIGHT_MEAN NEEDS TO BE > 3*WAVEHEIGHT_SIGMA */ 


/*Evan modified this, just set to ave percent coarse*/
/* Evans notes: ignore this: GC: WARNING! The REAL average of % coarse is 1.5*AVE_PERCENT_COARSE */
//#define AVE_PERCENT_COARSE 0.3


#define	AggRateFine	   0.00		/* aggradation rate of fine, in m/yr. It can be negative (erosion) and it will still work ok */
#define	AggRateCoarse	0.00		/* aggradation rate of coarse, in m/yr */
#define	SecondsPerYear	31536000.0		/* what it says */

//#define CELL_WIDTH 5             /* m */ 
//#define CELL_HEIGHT 0.05          /* m */

//#define Xmax 100
//#define Ymax 100 
//#define Zmax 100             
/* this # should be related to depth 
i.e., Zmax = DEPTH/CELLHEIGHT +
height of sediment accumulation */


#define AdjustTime 10	        /* number of iterations sed trans across bdries */
/* is given to adjust to change of direction */

#define PMAX_MEAN  0.5        /* for use with GenRandomGaussian evan change from .5*/
#define PMAX_SIGMA 0.2        /* ditto */
#define NMAX_MEAN  0.5        /* ditto evan change from .5 */
#define NMAX_SIGMA 0.2        /* ditto */

/* sediment transport constants */
/*#define Wf 0.0184     /* fall velocity of fine sediment */
/*#define Wc 0.0867     /* fall velocity of coarse sediment */
//#define Wf 0.012	  /* fall velocity of fine sediment  */
//#define Wc 0.11	  /* fall velocity of coarse sediment  */
#define Es 0.035             	/* efficiency factor */
#define rho 1000             	/* water density, kg/m3 */
#define rhoS 2600           	/* sediment density, kg/m3 */
#define PI 3.1415927        	/* PI */
#define g 9.8                	/* gravity, m/s */
#define ALPH 0.01		/* O(0.01) constant, in eff. prof. ht. dep. on %coarse*/
#define A 10                	/* O(10) constant, in Cf dep. on %coarse */
#define ProfHtOverFine 0.001    /* 2*Uo*(1/w)*this -> effective prof. ht. when %coarse = 0 */
/* constants for new 7/03 sed trans XXX */
/*#define dfine 0.00022	    /*  diameter of fine sed, in m */
/*#define dcoarse 0.00075       /*  diameter of coarse sed, in m */
//#define dfine 0.00015 	/*  diameter of fine sed, in m  */
//#define dcoarse 0.001       /*  diameter of coarse sed, in m  */
#define ConvertToEffGrSz  1.0	/* ConvertToEffGrSz times Se provides the effective d50 for determining ripple dims ...*/
//#define T 10.0			/* wave period, in s */
#define A2 10.0 		/* O(10) const in ripple wavelength as func. of bed comp. */
#define A3 2.0			/* O(1) const in ripple aspect ratio as func. of bed comp. */
#define mu 0.001		/* water visc., kg/ms */
#define coeffslope 1.0	/* coefficient tuning sediment transport '2'*/
#define nu 0.000001		/* kinematic visc., m2/s */
#define ezexpb -0.0		/* exponent to account for hiding in bedload*/
#define ezexps 0.0		/* exponent to account for hiding in suspension*/
#define karman 0.4		  /* von karman constant */
#define hustar 2.0		  /* von karman constant */

//#define FORCING_DURATION 86400    
/* 86400 s = 24hrs.; 
after FORCING_DURATION has passed, new
wave heights and current velocities are generated */
#define CURRENT_REVERSAL_PROB 2  /* probability that current will reverse itself
after one FORCING_DURATION evan change from 2 */
#define POS_NEG_RATIO 2           /* current is POS_NEG_RATIO times as likely to switch
from neg to pos as from pos to neg evan change from 2 */

/*file saving stuff*/
#define StartSavingAt 0. /*units = FORCING_DURATIONSs*/
#define FrameSpacing  1.0 /* units = FORCING_DURATIONs (presently 1 day); 
wave frequency ~= 864/day                    */

typedef struct timeStruct
{
	int years;
	int days;
	int hours;
	int minutes;
	int seconds;
} timeStruct;

typedef struct cellStruct
/* this structure represents all charateristics of a cell in 2
 dimensions, X * Y.  the third dimension is represented by the
 arrays percentFull and percentCoarse, which are themselves arrays
 of Zmax size */
{
	double localFluxFineX, localFluxFineY, localFluxCoarseX, localFluxCoarseY;
	double excessFineSedOutX, excessFineSedOutY, excessCoarseSedOutX, excessCoarseSedOutY;
	double volumeIn;
	double volumeOut;
	double fineVolumeAdded;
	double coarseVolumeAdded;
	std::vector<double> percentFull;
	std::vector<double> percentCoarse;
	double volumeDeposited;
	double consistencyBelow;      /* this will represent the consistency */
	int activeZ;  /* active cell in the Z dimension, i.e. top of sed buildup*/
	int	 hystcountfine;
	int	 hystcountcoarse;
	double RippleAspectRatioFineMix;
	double RippleLengthFineMix;
	double RippleHeightFineMix;
	double RippleAspectRatioCoarseMix;
	double RippleLengthCoarseMix;
	double RippleHeightCoarseMix;
	double depth;  /* indicates the depth of water */
	double sedimentHeight; /*indicates sediment buildup */
} cellStruct;

class CSBMBL : public CSG_Module
{
public:
	CSBMBL(void);
	virtual ~CSBMBL(void);
	virtual CSG_String	Get_MenuPath	(void)	{	return( _TL("SBMBL") );	}



protected:

	virtual bool	On_Execute	(void);

private:
	void main();
	void UpdateGrid();

	// OUTPUT

	CSG_Grid* pGridHeight;
	CSG_Grid* pGridCoarse;

	// DIMENSIONS

	int Xmax, Ymax, Zmax;
	double CELL_WIDTH, CELL_HEIGHT;

	// WATER

	double INITIAL_DEPTH;
	double VELOCITY_MEAN, VELOCITY_SIGMA;
	double WAVEHEIGHT_MEAN, WAVEHEIGHT_SIGMA;
	double T; // wave period (s)

	// SEDIMENT
	double AVE_PERCENT_COARSE;
	double Wf, Wc; // fall velocities for fine and coarse sediment (m/s)
	double dfine, dcoarse; // diameter fine and coarse sediment (m)

	double   timeStep;    
	double   flux4time; 
	double   ripfinemaxL;
	double   ripfinemaxA;
	double   ripfineminL;
	double   ripfineminA;
	double   ripcoarsemaxL;
	double   ripcoarsemaxA;
	double   ripcoarseminL;
	double   ripcoarseminA;

	double   currentDirectionX;
	double   currentDirectionY;
	double RandX;
	double RandY;

	double   tmpBedslope;
	double   maxBedslope;       /* for use in FindTimeStep(); */

	
	double FORCING_DURATION;
	double   totalElapsedTime;        /* in seconds */
	double   timeSinceForcingUpdate;   /* in seconds */

	int NumFramesSaved;   /* for keeping of track of when to save a new one */
	int maxRunTime;                     /* measured in units of FORCING_DURATIONs; */
	/* presently, 1 FORCING_DURATION = 1 day. */    

	std::vector<std::vector<cellStruct>> area;
	/* this is the declaration of our main data structure.
	 above components can be accessed by area.volumeIn, etc... i.e.
	 name of struct period field;
	 allocate double space for periodic boundary
	 conditions */
	/* conceptual image:  cross section 
	 |--------------------------------------------------------------|
	 |                                                              |
	 |       water, so %coarse = 0, % full = 0                      |
	 |                                                              |
	 |                                                              |
	 |                                                              |
	 |                        z=25                                  |
	 |                                                              | 
	 |-----active Z, i.e., top layer of sediment--------------------|
	 |  at z=25, random amount full, and random amount coarse       |
	 |                                                              |
	 |                                                              |
	 |   all sediment, random amt of coarse, % full = 1             |
	 |                                                              |
	 |--------------------------------------------------------------|
	 */

	/* int currentDirection;   1~X positive
	 2~X negative
	 3~Y positive
	 4~Y negative */

	/*
	double excessOutofIterFineX[Ymax], localFluxOutofIterFineX[Ymax];
	double excessOutofIterCoarseX[Ymax], localFluxOutofIterCoarseX[Ymax];
	double excessOutofIterFineY[Xmax], localFluxOutofIterFineY[Xmax];
	double excessOutofIterCoarseY[Xmax], localFluxOutofIterCoarseY[Xmax];
	*/

	std::vector<double> excessOutofIterFineX;
	std::vector<double> localFluxOutofIterFineX;
	std::vector<double> excessOutofIterCoarseX, localFluxOutofIterCoarseX;
	std::vector<double> excessOutofIterFineY, localFluxOutofIterFineY;
	std::vector<double> excessOutofIterCoarseY, localFluxOutofIterCoarseY;

	double currentVelocityX, currentVelocityY;
	double waveHeight;
	double waveHeightold;
	double CELL_VOLUME;


	int DEBUG, DEBUG2, DEBUG3, DEBUG4, DEBUG5;  
	/* indicates debug mode, turn on (1) or off (0) printf statements */

	/* forward declarations of functions */

	/* helper functions */
	double Raise(double b, double e);  /* implements ^ for doubles */
	double GenRandomPercentage();/* generate a random coarseness */

	/* initializers */
	void InitConds();           /* initialize conditions */
	void InitCondsFromFile();   /* initialize conditions from a file */
	void PrintToFile();         /* like it says */
	FILE       *SaveFile;
	FILE       *SaveForcing;
	char*       savefilename; 
	char startFromFile;     /* user inputted */
	char* filename;   /* user inputted */
	char* readfilename;
	int offset; /*evan added to make saved files have the appropriate number for storm scenarios*/

	/*checkers*/
	void FindAveBedHt();			       

	/* sed trans */

	void DoIteration();         /* function that makes things happen*/
	void DoIterationDummy();    /* function that doesn't really make things happen*/

	void SedTransFine();
	void SedTransCoarse();

	void AdjustCells();
	void ZeroVars();

	double GenRandomGaussian(double, double);

	double RunTimeClock();
	double ForcingClock();
	timeStruct ConvertTime(int);
	timeStruct TimeToCompletion(double);
	void  UpdateRunTimeClock();
	void  UpdateForcingClock();
	void  WriteForcingConditions();
	double GetPercentCompleted();
};


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#endif // #ifndef HEADER_INCLUDED__SBMBL_H

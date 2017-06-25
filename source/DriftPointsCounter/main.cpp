#define PLUGIN_MAIN_VERSION			(41)
/*
* Includes
*/
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <vector>
#include <cstring>
#include <limits>
#include <sstream>

extern void 
    *pAMXFunctions
;

#include <sampgdk.h>
//-------------------------------------------//
#undef MAX_PLAYERS
#define MAX_PLAYERS (1000)

//
#define PLUGIN_PRIVATE_UPDATE_DRIFT (150)//if [sa-mp variable] sleep is 5, then you have 1000/5 = 200 frames per second in sa-mp. this plugin will update each 30 frames at default
#define MINIMAL_ANGLE				(12.5)//in degrees
#define MAXIMAL_ANGLE				(125.0)//in degrees
#define MINIMAL_SPEED				(20.0)//in meters / second
#define MAXIMAL_SPEED				(500.0)//in meters / second
#define DEFAULT_DMG					(0)
#define START_END_DELAY				(4)
#define DIVIDER						(350)
#define fENABLED					(1)
#define PLUGIN_SUB_VERSION			(0)
#define DEFAULT_MAX_HEALTH_LOOSE	(0.0);
#define DEFAULT_CHECK				(1)
#define DEFAULT_BACKWARDS			(1)

#define USE_VEHICLE_MODEL_CHECK
#define CHECK_MAX_VALUES

#define PI (3.1415926535897932384626433832795f)

using namespace std;  

vector<AMX *> amx_list;

int GlobalPos_Timer[MAX_PLAYERS] = {-1};
int GlobalPos_Points[MAX_PLAYERS];
int GlobalPos_Combo[MAX_PLAYERS];
int flagcheckingstatus[MAX_PLAYERS] = {fENABLED};
int g_divider[MAX_PLAYERS] = {DIVIDER};
int DamageCheck[MAX_PLAYERS] = {DEFAULT_DMG};
int PlayerVehicleID[MAX_PLAYERS];
int Drifting[MAX_PLAYERS];
int Drifting_precise[MAX_PLAYERS];
int DriftStartEndDelay[MAX_PLAYERS] = {START_END_DELAY};
int WasAdded[MAX_PLAYERS];
int CheckHim[MAX_PLAYERS];
int CheckBackwards[MAX_PLAYERS];
float GlobalPos_X[MAX_PLAYERS];
float GlobalPos_Y[MAX_PLAYERS];
float GlobalPos_Z[MAX_PLAYERS];
float GlobalPos_Angle1[MAX_PLAYERS];
float GlobalPos_Angle2[MAX_PLAYERS];
float g_MinSpeed[MAX_PLAYERS] = {MINIMAL_SPEED};
float g_MaxAngle[MAX_PLAYERS] = {MAXIMAL_ANGLE};
float g_MinAngle[MAX_PLAYERS] = {MINIMAL_ANGLE};
float g_MaxSpeed[MAX_PLAYERS] = {MAXIMAL_SPEED};
float VehicleHealth[MAX_PLAYERS];
float SpeedX[MAX_PLAYERS];
float SpeedY[MAX_PLAYERS];
float SpeedZ[MAX_PLAYERS];
float floatdata[MAX_PLAYERS][11];
float MaxHealthLoose[MAX_PLAYERS];

int GlobalCheck = 1;
int GlobOnUpdateIndex = (-1);
int GlobOnStartIndex = (-1);
int GlobOnEndIndex = (-1);
int g_Ticked = 0;
int g_TickMax = PLUGIN_PRIVATE_UPDATE_DRIFT;
int didit = 0;

#define AREA_SIZE 20.0f//Define your Area size here lol.
#define GetArea(a,b,c,d) if(c >= -20000.0f && c <= 20000.0f){a = (int)floor((c+20000.0f)/AREA_SIZE);}else{a = 0;}if(d >= -20000.0f && d <= 20000.0f){b=(int)floor((d+20000.0f)/AREA_SIZE);}else{b = 0;}
#define GetAreaEx(a) (int)floor((a+20000.0f)/AREA_SIZE)

vector			<int>				Area[2001][2001];//160.801 Areas, -20.000 to 20.000 (40.000x40.000 = 1.600.000.000 square units), is an 100x100 units area with each unit holding nodes. (6000x6000 area, if all nodes spread evenly each area unit holds 38 nodes, not one big area with 35k nodes :p)

struct BonusData
{
	int enabled;
	float xPOS;
	float yPOS;
	float zPOS;
	BonusData(int n0,float n1,float n2,float n3):enabled(n0),xPOS(n1),yPOS(n2),zPOS(n3){}
}
;

vector <BonusData> BonusPoint;
vector<int> PlayerLoopList;

bool AllowedModelList[613];//Yes I know too much allocated but it is only for performance!

int OnPlayerDriftStart(int playerid);
int OnPlayerDriftStart(int playerid)
{
	for (std::vector<AMX *>::iterator a = amx_list.begin(); a != amx_list.end(); ++a)
	{
		if (!amx_FindPublic( * a, "OnPlayerDriftStart", &GlobOnStartIndex))
		{
			amx_Push(* a, playerid);
			amx_Exec(* a, NULL, GlobOnStartIndex);
		}
	}
    return 0;
}

int OnPlayerDriftUpdate(int playerid,int points,int combo,int flagid,float distance,float speed);
int OnPlayerDriftUpdate(int playerid,int points,int combo,int flagid,float distance,float speed)
{
	for (std::vector<AMX *>::iterator a = amx_list.begin(); a != amx_list.end(); ++a)
	{
		if (!amx_FindPublic(* a, "OnPlayerDriftUpdate", &GlobOnUpdateIndex))
		{
			amx_Push(* a, amx_ftoc(speed));
			amx_Push(* a, amx_ftoc(distance));
			amx_Push(* a, flagid);
			amx_Push(* a, combo);
			amx_Push(* a, points);
			amx_Push(* a, playerid);
			amx_Exec(* a, NULL, GlobOnUpdateIndex);
		}
	}
    return 0;
}

int OnPlayerDriftEnd(int playerid,int points,int combo,int reason);
int OnPlayerDriftEnd(int playerid,int points,int combo,int reason)
{
	for (std::vector<AMX *>::iterator a = amx_list.begin(); a != amx_list.end(); ++a)
	{
		if (!amx_FindPublic(* a, "OnPlayerDriftEnd", &GlobOnEndIndex))
		{
			amx_Push(* a, reason);
			amx_Push(* a, combo);
			amx_Push(* a, points);
			amx_Push(* a, playerid);
			amx_Exec(* a, NULL, GlobOnEndIndex);
		}
	}
    return 0;
}

bool AddModelToAllowedList(int modelid);
bool AddModelToAllowedList(int modelid)
{
	if(modelid > 611 || modelid < 400)return 0;
	AllowedModelList[modelid] = true;
	return 1;
}

bool ClearAllowedModelList();
bool ClearAllowedModelList()
{
	for(int i = 0; i != 612; i++)
	{
		AllowedModelList[i] = false;
	}
	return true;
}

bool ResetAllowedModelList();
bool ResetAllowedModelList()
{
	for(int i = 0; i != 612; i++)
	{
		AllowedModelList[i] = true;
	}
	AllowedModelList[448] = false;
	AllowedModelList[461] = false;
	AllowedModelList[462] = false;
	AllowedModelList[468] = false;
	AllowedModelList[481] = false;
	AllowedModelList[509] = false;
	AllowedModelList[521] = false;
	AllowedModelList[522] = false;
	AllowedModelList[581] = false;
	AllowedModelList[537] = false;
	AllowedModelList[538] = false;
	AllowedModelList[523] = false;
	AllowedModelList[417] = false;
	AllowedModelList[425] = false;
	AllowedModelList[447] = false;
	AllowedModelList[460] = false;
	AllowedModelList[469] = false;
	AllowedModelList[476] = false;
	AllowedModelList[487] = false;
	AllowedModelList[488] = false;
	AllowedModelList[497] = false;
	AllowedModelList[511] = false;
	AllowedModelList[512] = false;
	AllowedModelList[513] = false;
	AllowedModelList[519] = false;
	AllowedModelList[520] = false;
	AllowedModelList[548] = false;
	AllowedModelList[553] = false;
	AllowedModelList[563] = false;
	AllowedModelList[577] = false;
	AllowedModelList[592] = false;
	AllowedModelList[593] = false;
	AllowedModelList[430] = false;
	AllowedModelList[446] = false;
	AllowedModelList[452] = false;
	AllowedModelList[454] = false;
	AllowedModelList[472] = false;
	AllowedModelList[473] = false;
	AllowedModelList[453] = false;
	AllowedModelList[484] = false;
	AllowedModelList[493] = false;
	AllowedModelList[595] = false;
	AllowedModelList[464] = false;
	return true;
}

bool RemoveModelFromAllowedList(int modelid);
bool RemoveModelFromAllowedList(int modelid)
{
	if(modelid > 611 || modelid < 400)return 0;
	AllowedModelList[modelid] = false;
	return 1;
}

bool IsDriftingAllowedModel(int modelid);
bool IsDriftingAllowedModel(int modelid)
{
	return AllowedModelList[modelid];
}

static cell AMX_NATIVE_CALL SetDriftUpdateDelay( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL SetDriftUpdateDelay( AMX* amx, cell* params )
{
	if(params[1] > 100)
	{
		g_TickMax = 100;
		return 1;
	}
	if(params[1] < 5)
	{
		g_TickMax = 5;
		return 1;
	}
	g_TickMax = params[1];
	return 1;
}

static cell AMX_NATIVE_CALL SetDriftDivider( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL SetDriftDivider( AMX* amx, cell* params )
{
	int gg_divider = params[1];
	if(params[1] > 5000)
	{
		gg_divider = 5000;
	}
	if(params[1] < 1)
	{
		gg_divider = 1;
	}
	if(params[2] == (-1))
	{
		int playerid = 0;
		while(playerid != MAX_PLAYERS-1)
		{
			g_divider[playerid] = gg_divider;
			playerid++;
		}
	}
	else
	{
		g_divider[params[2]] = gg_divider;
	}
	return 1;
}

static cell AMX_NATIVE_CALL SetDriftMinAngle( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL SetDriftMinAngle( AMX* amx, cell* params )
{
	float val = amx_ctof(params[1]);
	float gg_MinAngle = val;
	if(val > 90.0)
	{
		gg_MinAngle = 90.0;
	}
	if(val < 0.0)
	{
		gg_MinAngle = 0.0;
	}
	if(params[2] == (-1))
	{
		int playerid = 0;
		while(playerid != MAX_PLAYERS-1)
		{
			g_MinAngle[playerid] = gg_MinAngle;
			playerid++;
		}
	}
	else
	{
		g_MinAngle[params[2]] = gg_MinAngle;
	}
	return 1;
}

static cell AMX_NATIVE_CALL SetDriftMaxAngle( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL SetDriftMaxAngle( AMX* amx, cell* params )
{
	float val = amx_ctof(params[1]);
	float gg_MaxAngle = val;
	if(val > 180.0)
	{
		gg_MaxAngle = 180.0;
	}
	if(val < 0.0)
	{
		gg_MaxAngle = 0.0;
	}
	if(params[2] == (-1))
	{
		int playerid = 0;
		while(playerid != MAX_PLAYERS-1)
		{
			g_MaxAngle[playerid] = gg_MaxAngle;
			playerid++;
		}
	}
	else
	{
		g_MaxAngle[params[2]] = gg_MaxAngle;
	}
	return 1;
}

static cell AMX_NATIVE_CALL SetDriftMinSpeed( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL SetDriftMinSpeed( AMX* amx, cell* params )
{
	float val = amx_ctof(params[1]);
	float gg_MaxAngle = val;
	if(val > 1000.0)
	{
		gg_MaxAngle = 1000.0;
	}
	if(val < 0.1)
	{
		gg_MaxAngle = 0.1f;
	}
	if(params[2] == (-1))
	{
		int playerid = 0;
		while(playerid != MAX_PLAYERS-1)
		{
			g_MinSpeed[playerid] = gg_MaxAngle;
			playerid++;
		}
	}
	else
	{
		g_MinSpeed[params[2]] = gg_MaxAngle;
	}
	return 1;
}

static cell AMX_NATIVE_CALL SetDriftMaxSpeed( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL SetDriftMaxSpeed( AMX* amx, cell* params )
{
	float val = amx_ctof(params[1]);
	float gg_MaxAngle = val;
	if(val > 1000.0)
	{
		gg_MaxAngle = 1000.0;
	}else
	if(val < 0.1)
	{
		gg_MaxAngle = 0.1f;
	}
	if(params[2] == (-1))
	{
		int playerid = 0;
		while(playerid != MAX_PLAYERS-1)
		{
			g_MaxSpeed[playerid] = gg_MaxAngle;
			playerid++;
		}
	}
	else
	{
		g_MaxSpeed[params[2]] = gg_MaxAngle;
	}
	return 1;
}

static cell AMX_NATIVE_CALL n_DriftStartEndDelay( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL n_DriftStartEndDelay( AMX* amx, cell* params )
{
	int gg_divider = params[1];
	if(params[1] > 1000)
	{
		gg_divider = 1000;
	}
	if(params[1] <= 1)
	{
		gg_divider = 1;
	}
	if(params[2] == (-1))
	{
		int playerid = 0;
		while(playerid != MAX_PLAYERS-1)
		{
			DriftStartEndDelay[playerid] = gg_divider;
			playerid++;
		}
	}
	else
	{
		DriftStartEndDelay[params[2]] = gg_divider;
	}
	return 1;
}

static cell AMX_NATIVE_CALL GetDriftMinSpeed( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL GetDriftMinSpeed( AMX* amx, cell* params )
{
	cell* cptr;
	amx_GetAddr(amx, params[1], &cptr);
	*cptr = amx_ftoc(g_MinSpeed[params[2]]);
	return 1;
}

static cell AMX_NATIVE_CALL GetDriftMaxSpeed( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL GetDriftMaxSpeed( AMX* amx, cell* params )
{
	cell* cptr;
	amx_GetAddr(amx, params[1], &cptr);
	*cptr = amx_ftoc(g_MaxSpeed[params[2]]);
	return 1;
}

static cell AMX_NATIVE_CALL GetDriftMaxAngle( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL GetDriftMaxAngle( AMX* amx, cell* params )
{
	cell* cptr;
	amx_GetAddr(amx, params[1], &cptr);
	*cptr = amx_ftoc(g_MaxAngle[params[2]]);
	return 1;
}

static cell AMX_NATIVE_CALL GetDriftMinAngle( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL GetDriftMinAngle( AMX* amx, cell* params )
{
	cell* cptr;
	amx_GetAddr(amx, params[1], &cptr);
	*cptr = amx_ftoc(g_MinAngle[params[2]]);
	return 1;
}

static cell AMX_NATIVE_CALL n_GetDriftStartEndDelay( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL n_GetDriftStartEndDelay( AMX* amx, cell* params )
{
	return DriftStartEndDelay[params[1]];
}

static cell AMX_NATIVE_CALL GetDriftUpdateDelay( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL GetDriftUpdateDelay( AMX* amx, cell* params )
{
	return g_TickMax;
}

static cell AMX_NATIVE_CALL GetDriftDivider( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL GetDriftDivider( AMX* amx, cell* params )
{
	return g_divider[params[1]];
}

static cell AMX_NATIVE_CALL SetDamageCheck( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL SetDamageCheck( AMX* amx, cell* params )
{
	int gg_divider = params[1];
	float MLH = amx_ctof(params[3]);
	if(MLH < 0.0)
	{
		MLH = 0.0;
	}
	if(MLH > 1000.0)
	{
		MLH = 1000.0;
	}
	if(params[1] >= 1)
	{
		gg_divider = 1;
	}
	if(params[1] <= 0)
	{
		gg_divider = 0;
	}
	if(params[2] == (-1))
	{
		int playerid = 0;
		while(playerid != MAX_PLAYERS-1)
		{
			DamageCheck[playerid] = gg_divider;
			MaxHealthLoose[playerid] = MLH;
			playerid++;
		}
	}
	else
	{
		DamageCheck[params[2]] = gg_divider;
		MaxHealthLoose[params[2]] = MLH;
	}
	return 1;
}

static cell AMX_NATIVE_CALL SetGlobalCheck( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL SetGlobalCheck( AMX* amx, cell* params )
{
	int gg_divider = params[1];
	if(params[1] >= 1)
	{
		gg_divider = 1;
	}
	if(params[1] <= 0)
	{
		gg_divider = 0;
	}
	GlobalCheck = gg_divider;
	return 1;
}

static cell AMX_NATIVE_CALL SetPlayerCheck( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL SetPlayerCheck( AMX* amx, cell* params )
{
	int gg_divider = params[2];
	if(params[2] >= 1)
	{
		gg_divider = 1;
	}
	if(params[2] <= 0)
	{
		gg_divider = 0;
	}
	if(params[1] == (-1))
	{
		int playerid = 0;
		while(playerid != MAX_PLAYERS-1)
		{
			CheckHim[playerid] = gg_divider;
			playerid++;
		}
	}
	else
	{
		CheckHim[params[1]] = gg_divider;
	}
	return 1;
}

static cell AMX_NATIVE_CALL GetPlayerCheck( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL GetPlayerCheck( AMX* amx, cell* params )
{
	return CheckHim[params[1]];
}

static cell AMX_NATIVE_CALL SetBackwardsCheck( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL SetBackwardsCheck( AMX* amx, cell* params )
{
	int gg_divider = params[2];
	if(params[2] >= 1)
	{
		gg_divider = 1;
	}
	if(params[2] <= 0)
	{
		gg_divider = 0;
	}
	if(params[1] == (-1))
	{
		int playerid = 0;
		while(playerid != MAX_PLAYERS-1)
		{
			//CheckBackwards[playerid] = gg_divider;
			if(gg_divider ==1)
			g_MaxAngle[playerid] = 360.0f;
			else
			{
				g_MaxAngle[playerid] = 125.0f;
			}
			playerid++;
		}
	}
	else
	{
		if(gg_divider ==1)
		g_MaxAngle[params[1]] = 360.0f;
		else
		{
			g_MaxAngle[params[1]] = 125.0f;
		}
		//CheckBackwards[params[1]] = gg_divider;
	}
	return 1;
}

static cell AMX_NATIVE_CALL GetBackwardsCheck( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL GetBackwardsCheck( AMX* amx, cell* params )
{
	if(g_MaxAngle[params[1]] > 140.0f)return 1;
	return 0;
	//return CheckBackwards[params[1]];
}

static cell AMX_NATIVE_CALL GetGlobalCheck( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL GetGlobalCheck( AMX* amx, cell* params )
{
	return GlobalCheck;
}

static cell AMX_NATIVE_CALL GetDamageCheck( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL GetDamageCheck( AMX* amx, cell* params )
{
	cell* cptr;
	amx_GetAddr(amx, params[2], &cptr);
	*cptr = amx_ftoc(MaxHealthLoose[params[1]]);
	return DamageCheck[params[1]];
}

static cell AMX_NATIVE_CALL IsPlayerDrifting( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL IsPlayerDrifting( AMX* amx, cell* params )
{
	return Drifting[params[1]];
}

static cell AMX_NATIVE_CALL IsPlayerDrifting_precise( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL IsPlayerDrifting_precise( AMX* amx, cell* params )
{
	return Drifting_precise[params[1]];
}

static cell AMX_NATIVE_CALL AddFLAG( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL AddFLAG( AMX* amx, cell* params )
{
	float posx = amx_ctof(params[1]);
	float posy = amx_ctof(params[2]);
	float posz = amx_ctof(params[3]);
	int Xlocation = 0, Ylocation = 0;
	GetArea(Xlocation,Ylocation,posx,posy);
	for(int i = 0,j = BonusPoint.size(); i < j; i++)
	{
		if(BonusPoint[i].enabled == -1)
		{
			BonusPoint[i].enabled = 1;
			BonusPoint[i].xPOS = posx;
			BonusPoint[i].yPOS = posy;
			BonusPoint[i].zPOS = posz;
	
			if(Xlocation > 0)
			{
				if(Ylocation > 0)
				{
					/*
						[-1,+1][+0,+1][+1,+1]
						[-1,+0][+0,+0][+1,+0]
						[-1,-1][+0,-1][+1,-1]
					*/
					Area[Xlocation-1][Ylocation+1].push_back((i));
					Area[Xlocation-1][Ylocation].push_back((i));
					Area[Xlocation-1][Ylocation-1].push_back((i));

					Area[Xlocation][Ylocation+1].push_back((i));
					Area[Xlocation][Ylocation].push_back((i));
					Area[Xlocation][Ylocation-1].push_back((i));

					Area[Xlocation+1][Ylocation+1].push_back((i));
					Area[Xlocation+1][Ylocation].push_back((i));
					Area[Xlocation+1][Ylocation-1].push_back((i));
				}
				else
				{
					/*
						[-1,+1][+0,+1][+1,+1]
						[-1,+0][+0,+0][+1,+0]
					*/
					Area[Xlocation-1][Ylocation+1].push_back((i));
					Area[Xlocation-1][Ylocation].push_back((i));

					Area[Xlocation][Ylocation+1].push_back((i));
					Area[Xlocation][Ylocation].push_back((i));

					Area[Xlocation+1][Ylocation+1].push_back((i));
					Area[Xlocation+1][Ylocation].push_back((i));
				}
			}
			else
			{
				if(Ylocation > 0)
				{
					/*
						[+0,+1][+1,+1]
						[+0,+0][+1,+0]
						[+0,-1][+1,-1]
					*/

					Area[Xlocation][Ylocation+1].push_back((i));
					Area[Xlocation][Ylocation].push_back((i));
					Area[Xlocation][Ylocation-1].push_back((i));

					Area[Xlocation+1][Ylocation+1].push_back((i));
					Area[Xlocation+1][Ylocation].push_back((i));
					Area[Xlocation+1][Ylocation-1].push_back((i));
				}
				else
				{
					/*
						[+0,+1][+1,+1]
						[+0,+0][+1,+0]
					*/

					Area[Xlocation][Ylocation+1].push_back((i));
					Area[Xlocation][Ylocation].push_back((i));

					Area[Xlocation+1][Ylocation+1].push_back((i));
					Area[Xlocation+1][Ylocation].push_back((i));
				}
			}
			return i;
		}
	}
	BonusPoint.push_back(BonusData(1,posx,posy,posz));
	if(Xlocation > 0)
	{
		if(Ylocation > 0)
		{
			/*
				[-1,+1][+0,+1][+1,+1]
				[-1,+0][+0,+0][+1,+0]
				[-1,-1][+0,-1][+1,-1]
			*/
			Area[Xlocation-1][Ylocation+1].push_back((BonusPoint.size() - 1));
			Area[Xlocation-1][Ylocation].push_back((BonusPoint.size() - 1));
			Area[Xlocation-1][Ylocation-1].push_back((BonusPoint.size() - 1));

			Area[Xlocation][Ylocation+1].push_back((BonusPoint.size() - 1));
			Area[Xlocation][Ylocation].push_back((BonusPoint.size() - 1));
			Area[Xlocation][Ylocation-1].push_back((BonusPoint.size() - 1));

			Area[Xlocation+1][Ylocation+1].push_back((BonusPoint.size() - 1));
			Area[Xlocation+1][Ylocation].push_back((BonusPoint.size() - 1));
			Area[Xlocation+1][Ylocation-1].push_back((BonusPoint.size() - 1));
		}
		else
		{
			/*
				[-1,+1][+0,+1][+1,+1]
				[-1,+0][+0,+0][+1,+0]
			*/
			Area[Xlocation-1][Ylocation+1].push_back((BonusPoint.size() - 1));
			Area[Xlocation-1][Ylocation].push_back((BonusPoint.size() - 1));

			Area[Xlocation][Ylocation+1].push_back((BonusPoint.size() - 1));
			Area[Xlocation][Ylocation].push_back((BonusPoint.size() - 1));

			Area[Xlocation+1][Ylocation+1].push_back((BonusPoint.size() - 1));
			Area[Xlocation+1][Ylocation].push_back((BonusPoint.size() - 1));
		}
	}
	else
	{
		if(Ylocation > 0)
		{
			/*
				[+0,+1][+1,+1]
				[+0,+0][+1,+0]
				[+0,-1][+1,-1]
			*/

			Area[Xlocation][Ylocation+1].push_back((BonusPoint.size() - 1));
			Area[Xlocation][Ylocation].push_back((BonusPoint.size() - 1));
			Area[Xlocation][Ylocation-1].push_back((BonusPoint.size() - 1));

			Area[Xlocation+1][Ylocation+1].push_back((BonusPoint.size() - 1));
			Area[Xlocation+1][Ylocation].push_back((BonusPoint.size() - 1));
			Area[Xlocation+1][Ylocation-1].push_back((BonusPoint.size() - 1));
		}
		else
		{
			/*
				[+0,+1][+1,+1]
				[+0,+0][+1,+0]
			*/

			Area[Xlocation][Ylocation+1].push_back((BonusPoint.size() - 1));
			Area[Xlocation][Ylocation].push_back((BonusPoint.size() - 1));

			Area[Xlocation+1][Ylocation+1].push_back((BonusPoint.size() - 1));
			Area[Xlocation+1][Ylocation].push_back((BonusPoint.size() - 1));
		}
	}
	return BonusPoint.size() - 1;
}

static cell AMX_NATIVE_CALL DisableFLAG( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL DisableFLAG( AMX* amx, cell* params )
{
	BonusPoint[params[1]].enabled = 0;
	return 1;
}

static cell AMX_NATIVE_CALL DeleteFLAG( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL DeleteFLAG( AMX* amx, cell* params )
{
	BonusPoint[params[1]].enabled = -1;
	int Xlocation = 0, Ylocation = 0;
	GetArea(Xlocation,Ylocation,BonusPoint[params[1]].xPOS,BonusPoint[params[1]].yPOS);
	if(Xlocation > 0)
	{
		if(Ylocation > 0)
		{
			/*
				[-1,+1][+0,+1][+1,+1]
				[-1,+0][+0,+0][+1,+0]
				[-1,-1][+0,-1][+1,-1]
			*/
			--Xlocation;++Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			--Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			--Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			++Xlocation;Ylocation+=3;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			--Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			--Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			++Xlocation;Ylocation+=3;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			--Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			--Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
		}
		else
		{
			/*
				[-1,+1][+0,+1][+1,+1]
				[-1,+0][+0,+0][+1,+0]
			*/
			--Xlocation;++Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			--Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			++Xlocation;++Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			--Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			++Xlocation;++Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			--Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
		}
	}
	else
	{
		if(Ylocation > 0)
		{
			/*
				[+0,+1][+1,+1]
				[+0,+0][+1,+0]
				[+0,-1][+1,-1]
			*/
			++Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			--Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			--Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			Ylocation+=2;
			++Xlocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			--Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			--Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
		}
		else
		{
			/*
				[+0,+1][+1,+1]
				[+0,+0][+1,+0]
			*/
			++Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			--Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			++Ylocation;
			++Xlocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
			--Ylocation;
			Area[Xlocation][Ylocation].erase(std::remove(Area[Xlocation][Ylocation].begin(), Area[Xlocation][Ylocation].end(), params[1]), Area[Xlocation][Ylocation].end()); 
		}
	}
	return 1;
}

static cell AMX_NATIVE_CALL EnableFLAG( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL EnableFLAG( AMX* amx, cell* params )
{
	if(	BonusPoint[params[1]].enabled == -1)
	{
		int Xloc = 0, Yloc = 0;
		GetArea(Xloc,Yloc,BonusPoint[params[1]].xPOS,BonusPoint[params[1]].yPOS);
		Area[Xloc][Yloc].push_back((params[1]));
	}
	BonusPoint[params[1]].enabled = 1;
	return 1;
}

static cell AMX_NATIVE_CALL GetPluginVersion( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL GetPluginVersion( AMX* amx, cell* params )
{
	return PLUGIN_MAIN_VERSION;
}

static cell AMX_NATIVE_CALL MoveFLAG( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL MoveFLAG( AMX* amx, cell* params )
{
	float posx = amx_ctof(params[2]);
	float posy = amx_ctof(params[3]);
	float posz = amx_ctof(params[4]);
	BonusPoint[params[1]].xPOS = posx;
	BonusPoint[params[1]].yPOS = posy;
	BonusPoint[params[1]].zPOS = posz;
	return 1;
}

PLUGIN_EXPORT bool PLUGIN_CALL OnPlayerStateChange(int playerid, int newstate, int oldstate)
{
	if(newstate == PLAYER_STATE_DRIVER && WasAdded[playerid] == 0)
	{
		WasAdded[playerid] = 1;
		PlayerVehicleID[playerid] = GetPlayerVehicleID(playerid);
		int size = PlayerLoopList.size();
		for (int index=0; index < size; ++index)
		{
			if(PlayerLoopList.at(index) == playerid)
			{
				return true;//avoid double adding
			}
		}
		PlayerLoopList.push_back(playerid);
		return true;
	}
	else
	if(WasAdded[playerid] == 1)
	{
		if(newstate == PLAYER_STATE_DRIVER)
		{
			PlayerVehicleID[playerid] = GetPlayerVehicleID(playerid);
			return true;
		}
		else
		{
			WasAdded[playerid] = 0;
			PlayerVehicleID[playerid] = -1;
			int size = PlayerLoopList.size();
			for (int index=0; index < size; ++index)
			{
				if(PlayerLoopList.at(index) == playerid)
				{
					PlayerLoopList.erase(PlayerLoopList.begin()+index);
					if(Drifting[playerid] == 1)
					{
		 				if(GlobalPos_Timer[playerid] != -1)
		 				{
							Drifting_precise[playerid] = 0;
		 					GlobalPos_Timer[playerid] = -1;
							OnPlayerDriftEnd(playerid,GlobalPos_Points[playerid],GlobalPos_Combo[playerid],0);
							GlobalPos_Combo[playerid] = 0;
							GlobalPos_Points[playerid] = 0;
							Drifting[playerid] = 0;
							return true;
		 				}
						return true;
					}
					return true;
				}
			}
			return true;
		}
	}
	return true;
}

static cell AMX_NATIVE_CALL SetCheckForFlags( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL SetCheckForFlags( AMX* amx, cell* params )
{
	int gg_divider = params[1];
	if(params[1] >= 1)
	{
		gg_divider = 1;
	}
	if(params[1] <= 0)
	{
		gg_divider = 0;
	}
	if(params[2] == (-1))
	{
		int playerid = 0;
		while(playerid != MAX_PLAYERS-1)
		{
			flagcheckingstatus[playerid] = gg_divider;
			playerid++;
		}
	}
	else
	{
		flagcheckingstatus[params[2]] = gg_divider;
	}
	return 1;
}

static cell AMX_NATIVE_CALL GetCheckForFlags( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL GetCheckForFlags( AMX* amx, cell* params )
{
	return flagcheckingstatus[params[1]];
}

static cell AMX_NATIVE_CALL SetDriftMaxHealthLoose( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL SetDriftMaxHealthLoose( AMX* amx, cell* params )
{
	float val = amx_ctof(params[1]);
	float gg_MinAngle = val;
	if(val >= 1000.0)
	{
		gg_MinAngle = 1000.0;
	}
	if(val <= 0.0)
	{
		gg_MinAngle = 0.0;
	}
	if(params[2] == (-1))
	{
		int playerid = 0;
		while(playerid != MAX_PLAYERS-1)
		{
			MaxHealthLoose[playerid] = gg_MinAngle;
			playerid++;
		}
	}
	else
	{
		MaxHealthLoose[params[2]] = gg_MinAngle;
	}
	return 1;
}

static cell AMX_NATIVE_CALL GetDriftMaxHealthLoose( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL GetDriftMaxHealthLoose( AMX* amx, cell* params )
{
	cell* cptr;
	amx_GetAddr(amx, params[1], &cptr);
	*cptr = amx_ftoc(MaxHealthLoose[params[2]]);
	return 1;
}

static cell AMX_NATIVE_CALL AddDriftModel( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL DelDriftModel( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL ClearModelLst( AMX* amx, cell* params );
static cell AMX_NATIVE_CALL ResetModelLst( AMX* amx, cell* params );

static cell AMX_NATIVE_CALL AddDriftModel( AMX* amx, cell* params )
{
	return AddModelToAllowedList(params[1]);
}

static cell AMX_NATIVE_CALL DelDriftModel( AMX* amx, cell* params )
{
	return RemoveModelFromAllowedList(params[1]);
}

static cell AMX_NATIVE_CALL ClearModelLst( AMX* amx, cell* params )
{
	return ClearAllowedModelList();
}

static cell AMX_NATIVE_CALL ResetModelLst( AMX* amx, cell* params )
{
	return ResetAllowedModelList();
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() 
{
	return sampgdk::Supports() | SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load( void **ppData ) 
{
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	sampgdk::Load(ppData);
	int loop = 0;
	while(loop != MAX_PLAYERS-1)
	{
		GlobalPos_Timer[loop] = (-1);
		g_MinSpeed[loop] = MINIMAL_SPEED;
		g_MaxAngle[loop] = MAXIMAL_ANGLE;
		g_MinAngle[loop] = MINIMAL_ANGLE;
		g_MaxSpeed[loop] = MAXIMAL_SPEED;
		g_divider[loop] = DIVIDER;
		DamageCheck[loop] = DEFAULT_DMG;
		CheckHim[loop] = DEFAULT_CHECK;
		CheckBackwards[loop] = DEFAULT_BACKWARDS;
		flagcheckingstatus[loop] = fENABLED;
		DriftStartEndDelay[loop] = START_END_DELAY;
		MaxHealthLoose[loop] = DEFAULT_MAX_HEALTH_LOOSE;
		loop++;
	}
	ResetAllowedModelList();
	cout << "---Loading---\r\n\tGamer_Z's Project Bundle: \r\n\t\tDrift Points Counter R" << PLUGIN_MAIN_VERSION << "\r\n---LOADED---";
	return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload( )
{
	cout << "---Unloading---\r\n\tGamer_Z's Project Bundle: \r\n\t\tDrift Points Counter\r\n---UNLOADED---";
	sampgdk::Unload();
}

AMX_NATIVE_INFO driftAMXNatives[ ] =
{
	{"DriftSet_UpdateDelay", SetDriftUpdateDelay},
	{"DriftSet_MinimalSpeed", SetDriftMinSpeed},
	{"DriftSet_MinimalAngle", SetDriftMinAngle},
	#if defined CHECK_MAX_VALUES
	{"DriftSet_MaximalSpeed", SetDriftMaxSpeed},
	{"DriftSet_MaximalAngle", SetDriftMaxAngle},

	{"DriftGet_MaximalSpeed", GetDriftMaxSpeed},
	{"DriftGet_MaximalAngle", GetDriftMaxAngle},
	#endif
	{"DriftSet_StartEndDelay", n_DriftStartEndDelay},

	{"DriftGet_StartEndDelay", n_GetDriftStartEndDelay},
	{"DriftGet_UpdateDelay", GetDriftUpdateDelay},

	{"DriftGet_MinimalSpeed", GetDriftMinSpeed},
	{"DriftGet_MinimalAngle", GetDriftMinAngle},

	{"DriftSet_Divider", SetDriftDivider},
	{"DriftGet_Divider", GetDriftDivider},

	{"DriftSet_DamageCheck", SetDamageCheck},
	{"DriftGet_DamageCheck", GetDamageCheck},

	{"IsPlayerDrifting", IsPlayerDrifting},
	{"IsPlayerDrifting_precise", IsPlayerDrifting_precise},

	{"Drift_AddFLAG",AddFLAG},
	{"Drift_DisableFLAG",DisableFLAG},
	{"Drift_DeleteFLAG",DeleteFLAG},
	{"Drift_EnableFLAG",EnableFLAG},
	{"Drift_MoveFLAG",MoveFLAG},

	{"Drift_SetCheckForFlags",SetCheckForFlags},
	{"Drift_GetCheckForFlags",GetCheckForFlags},

	//

	{"Drift_SetGlobalCheck", SetGlobalCheck},
	{"Drift_GetGlobalCheck", GetGlobalCheck},

	{"Drift_SetPlayerCheck", SetPlayerCheck},
	{"Drift_GetPlayerCheck", GetPlayerCheck},

	{"Drift_SetBackwardsCheck", SetBackwardsCheck},
	{"Drift_GetBackwardsCheck", GetBackwardsCheck},

	{"Drift_SetDriftMaxHealthLoose", SetDriftMaxHealthLoose},
	{"Drift_GetDriftMaxHealthLoose", GetDriftMaxHealthLoose},

	//
	{"Private_DriftGetVersion",GetPluginVersion},

	{"Drift_AllowModel",AddDriftModel},
	{"Drift_DisallowModel",DelDriftModel},
	{"Drift_ClearModelList",ClearModelLst},
	{"Drift_ResetDefaultModelList",ResetModelLst},
	//
	{0,                0}
};

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad( AMX *amx ) 
{
	//pAMX = amx;
	amx_list.push_back(amx);
	return amx_Register( amx, driftAMXNatives, -1 );
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload( AMX *amx ) 
{
	std::vector<AMX *>::iterator i = amx_list.begin(); 
	while (i != amx_list.end())
	{ 
		i = amx_list.erase(i); 
	} 
	return AMX_ERR_NONE;
}

PLUGIN_EXPORT bool PLUGIN_CALL OnPlayerDisconnect(int playerid, int reason)
{
	OnPlayerStateChange(playerid, 0, 0);
	return true;
}

float anglex = 0.0f;
float angle = 0.0f;

PLUGIN_EXPORT void PLUGIN_CALL
	ProcessTick()
{
	sampgdk::ProcessTick();
	if(GlobalCheck == 1)
	{
		if(g_Ticked == g_TickMax)
		{
			int playerid = 0;
			int size = PlayerLoopList.size();
			int index = 0;
			for (index = 0; index < size; ++index)
			{
				playerid = PlayerLoopList.at(index);
				if(CheckHim[playerid] == 0)continue;
				if(DamageCheck[playerid] == 1 && Drifting[playerid] == 1)
				{
					float tempheal;
					GetVehicleHealth(PlayerVehicleID[playerid], &tempheal);
					if(VehicleHealth[playerid] != tempheal)
					{
						if(MaxHealthLoose[playerid] < abs(VehicleHealth[playerid]-tempheal))
						{
		 					GlobalPos_Timer[playerid] = (-1);
							OnPlayerDriftEnd(playerid,GlobalPos_Points[playerid],GlobalPos_Combo[playerid],1);
							GlobalPos_Combo[playerid] = 0;
							GlobalPos_Points[playerid] = 0;
							Drifting[playerid] = 0;
						}
						else
						{
							VehicleHealth[playerid] = tempheal;
						}
						continue;
					}
				}
				GetVehicleVelocity(PlayerVehicleID[playerid], &SpeedX[playerid], &SpeedY[playerid], &SpeedZ[playerid]);
				GetVehicleZAngle(PlayerVehicleID[playerid], &GlobalPos_Angle1[playerid]);
				anglex = 0.0f;
				if(SpeedY[playerid] == 0.0f)
					SpeedY[playerid] = 0.0000000001f;
				if(SpeedX[playerid] < 0.0f && 0.0f < SpeedY[playerid])
					anglex = 0.0f   - (atan(SpeedX[playerid]/SpeedY[playerid])*57.2957795130823f);
				else if((SpeedX[playerid] < 0.0f && 0.0f > SpeedY[playerid]) || (SpeedX[playerid] > 0.0f && 0.0f > SpeedY[playerid]))
					anglex = 180.0f - (atan(SpeedX[playerid]/SpeedY[playerid])*57.2957795130823f);
				else if(SpeedX[playerid] > 0.0f && 0.0f < SpeedY[playerid])
					anglex = 360.0f - (atan(SpeedX[playerid]/SpeedY[playerid])*57.2957795130823f);
				else
					anglex = GlobalPos_Angle1[playerid];

				angle = anglex-GlobalPos_Angle1[playerid];
				angle = abs(angle);
				floatdata[playerid][10] = sqrt(pow(SpeedX[playerid], 2)+pow(SpeedY[playerid], 2)+pow(SpeedZ[playerid], 2))*274;
				if(
						#if defined CHECK_MAX_VALUES
						g_MaxAngle[playerid] > angle && 
						#endif
						angle > g_MinAngle[playerid] && 
						floatdata[playerid][10] > g_MinSpeed[playerid] 
						#if defined CHECK_MAX_VALUES
						&& floatdata[playerid][10] < g_MaxSpeed[playerid]
						#endif
						#if defined USE_VEHICLE_MODEL_CHECK
						&& AllowedModelList[GetVehicleModel(PlayerVehicleID[playerid])] == true
						#endif
				)
		 		{
					GlobalPos_Points[playerid] += (int)(floor(angle * floatdata[playerid][10])/g_divider[playerid]);
					if(GlobalPos_Timer[playerid] != -1)
					{
						Drifting_precise[playerid] = 1;
						GlobalPos_Combo[playerid] += 1;
						if(flagcheckingstatus[playerid] == 1)
						{
							didit = 0;
							GetVehiclePos(PlayerVehicleID[playerid],&floatdata[playerid][4],&floatdata[playerid][5],&floatdata[playerid][6]);
							int Xloc = 0, Yloc = 0;
							GetArea(Xloc,Yloc,floatdata[playerid][4],floatdata[playerid][5]);
							for(int i = 0; i < (int)Area[Xloc][Yloc].size(); ++i)
							{
								int NDX = Area[Xloc][Yloc].at(i);
								if(BonusPoint[NDX].enabled == 0)continue;
								float dist = (pow(BonusPoint[NDX].xPOS-floatdata[playerid][4],2)+pow(BonusPoint[NDX].yPOS-floatdata[playerid][5],2)+pow(BonusPoint[NDX].zPOS-floatdata[playerid][6],2));
								if(dist > 157.0)continue;//12.5*12.5
								OnPlayerDriftUpdate(playerid,GlobalPos_Points[playerid],GlobalPos_Combo[playerid],NDX,dist,floatdata[playerid][10]);
								didit = 1;
								break;
							}
							if(didit == 0)
							{
								OnPlayerDriftUpdate(playerid,GlobalPos_Points[playerid],GlobalPos_Combo[playerid],-1,-1.0,floatdata[playerid][10]);
							}
						}
						else
						{
							OnPlayerDriftUpdate(playerid,GlobalPos_Points[playerid],GlobalPos_Combo[playerid],-1,-1.0,floatdata[playerid][10]);
						}
					}
					else
					{
						Drifting[playerid] = 1;
						GetVehicleHealth(PlayerVehicleID[playerid], &VehicleHealth[playerid]);
						OnPlayerDriftStart(playerid);
					}
					GlobalPos_Timer[playerid] = DriftStartEndDelay[playerid];
		 		}
		 		else
		 		{
		 			if(GlobalPos_Timer[playerid] != -1)
		 			{
						Drifting_precise[playerid] = 0;
		 				GlobalPos_Timer[playerid]--;
		 				if(GlobalPos_Timer[playerid] == -1)
		 				{
							OnPlayerDriftEnd(playerid,GlobalPos_Points[playerid],GlobalPos_Combo[playerid],0);
							GlobalPos_Combo[playerid] = 0;
							GlobalPos_Points[playerid] = 0;
							Drifting[playerid] = 0;
		 				}
		 			}
		 		}
			}
			g_Ticked = -1;
		}
		g_Ticked += 1;
	}
}
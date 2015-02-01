#include <a_samp>
#include <DriftPointsCounter>

new
	Xveh,
	pointsnow,
	s[128][3];

enum DriftingInfo
{
	FlagP,
	lastflag,
	Text:PlayerTextDraw[3],
	PlayerDrift,
	PlayerCombo,
	Shown,
	TimerDD,
	TotalDriftPoints,
	TTTimer,
	DeathTimer
}

new Drift_VAR[MAX_PLAYERS][DriftingInfo];

public OnFilterScriptInit()
{
	for(new x=0;x<MAX_PLAYERS;x++)
	{
		Drift_VAR[x][PlayerTextDraw][0] = TextDrawCreate(320.000000, 63.000000, "~w~DRIFTING~n~~g~");
		TextDrawAlignment(Drift_VAR[x][PlayerTextDraw][0], 2);
		TextDrawBackgroundColor(Drift_VAR[x][PlayerTextDraw][0], 77);
		TextDrawFont(Drift_VAR[x][PlayerTextDraw][0], 1);
		TextDrawLetterSize(Drift_VAR[x][PlayerTextDraw][0], 0.659999, 1.700000);
		TextDrawColor(Drift_VAR[x][PlayerTextDraw][0], 99);
		TextDrawSetOutline(Drift_VAR[x][PlayerTextDraw][0], 1);
		TextDrawSetProportional(Drift_VAR[x][PlayerTextDraw][0], 1);
		TextDrawUseBox(Drift_VAR[x][PlayerTextDraw][0], 1);
		TextDrawBoxColor(Drift_VAR[x][PlayerTextDraw][0], 1431655765);
		TextDrawTextSize(Drift_VAR[x][PlayerTextDraw][0], 19.000000, -210.000000);

		Drift_VAR[x][PlayerTextDraw][1] = TextDrawCreate(320.000000, 79.000000, "~r~COMBO~n~~b~");
		TextDrawAlignment(Drift_VAR[x][PlayerTextDraw][1], 2);
		TextDrawBackgroundColor(Drift_VAR[x][PlayerTextDraw][1], 77);
		TextDrawFont(Drift_VAR[x][PlayerTextDraw][1], 1);
		TextDrawLetterSize(Drift_VAR[x][PlayerTextDraw][1], 0.659999, 1.700000);
		TextDrawColor(Drift_VAR[x][PlayerTextDraw][1], 99);
		TextDrawSetOutline(Drift_VAR[x][PlayerTextDraw][1], 1);
		TextDrawSetProportional(Drift_VAR[x][PlayerTextDraw][1], 1);
		TextDrawUseBox(Drift_VAR[x][PlayerTextDraw][1], 1);
		TextDrawBoxColor(Drift_VAR[x][PlayerTextDraw][1], 572662306);
		TextDrawTextSize(Drift_VAR[x][PlayerTextDraw][1], 19.000000, -210.000000);

		Drift_VAR[x][PlayerTextDraw][2] = TextDrawCreate(167.000000, 114.000000, "~r~]~b~FLAG CATCH~r~]~w~~n~+");
		TextDrawAlignment(Drift_VAR[x][PlayerTextDraw][2], 2);
		TextDrawBackgroundColor(Drift_VAR[x][PlayerTextDraw][2], 255);
		TextDrawFont(Drift_VAR[x][PlayerTextDraw][2], 2);
		TextDrawLetterSize(Drift_VAR[x][PlayerTextDraw][2], 0.659999, 1.300000);
		TextDrawColor(Drift_VAR[x][PlayerTextDraw][2], 99);
		TextDrawSetOutline(Drift_VAR[x][PlayerTextDraw][2], 1);
		TextDrawSetProportional(Drift_VAR[x][PlayerTextDraw][2], 1);
		TextDrawUseBox(Drift_VAR[x][PlayerTextDraw][2], 1);
		TextDrawBoxColor(Drift_VAR[x][PlayerTextDraw][2], 1111621720);
		TextDrawTextSize(Drift_VAR[x][PlayerTextDraw][2], 19.000000, -153.000000);
        Drift_VAR[x][TTTimer] = Drift_VAR[x][DeathTimer] = Drift_VAR[x][lastflag] = Drift_VAR[x][TimerDD] = (-1);
	}
	DriftSet_DamageCheck();
	//flagidx = Drift_AddFLAG(-2067.6152,1285.7096,7.8668);

	CreateObject(3877, -2423.113525, -617.921448, 133.208405, 0.0000, 0.0000, 0.0000);
	CreateObject(3877, -2422.186768, -615.903015, 133.228241, 0.0000, 0.0000, 0.0000);
	CreateObject(3877, -2421.262695, -613.842529, 133.228241, 0.0000, 0.0000, 0.0000);
	CreateObject(3877, -2420.677490, -611.709900, 133.228241, 0.0000, 0.0000, 0.0000);
	CreateObject(3877, -2420.685791, -609.527954, 133.228241, 0.0000, 0.0000, 0.0000);
	CreateObject(3877, -2420.699219, -607.226074, 133.228241, 0.0000, 0.0000, 0.0000);
	CreateObject(3877, -2420.654297, -604.977539, 133.228241, 0.0000, 0.0000, 0.0000);
	CreateObject(3877, -2420.960205, -602.891846, 133.228241, 0.0000, 0.0000, 25.7831);
	CreateObject(3877, -2421.784668, -600.842041, 133.228241, 0.0000, 0.0000, 25.7831);
	CreateObject(1238, -2431.825684, -607.936096, 131.858246, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2471.610107, -496.763153, 103.442001, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2620.131104, -497.496613, 70.364105, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2406.146729, -422.384888, 83.965530, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2335.787109, -458.131409, 79.319588, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2379.778809, -384.742615, 75.842087, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2671.417969, -428.636871, 30.774956, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2676.699707, -511.580750, 17.629486, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2775.516846, -486.592285, 6.491295, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2775.821289, -486.792786, 6.484154, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2776.219238, -487.593323, 6.488155, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2776.745361, -488.067169, 6.491304, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2777.184082, -488.504150, 6.494083, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2777.858887, -489.264557, 6.498680, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2778.539551, -490.817688, 6.507610, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2778.731201, -491.450104, 6.504780, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2779.153320, -492.234375, 6.500772, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2779.904053, -492.607727, 6.497387, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2780.073242, -493.016205, 6.495438, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2781.494629, -493.450989, 6.490028, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2781.144043, -494.479248, 6.487202, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2782.278809, -494.814331, 6.500626, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2782.306885, -495.462738, 6.538544, 0.0000, 0.0000, 0.0000);
	CreateObject(1238, -2777.948975, -490.239380, 6.502495, 0.0000, 0.0000, 0.0000);

	CreateObject(1270, -2431.905273, -607.968445, 133.104156, 0.0000, 0.0000, 0.0000);
    Drift_AddFLAG(-2431.905273, -607.968445, 133.104156);

	CreateObject(1270, -2471.645020, -496.794403, 104.574181, 0.0000, 0.0000, 0.0000);
    Drift_AddFLAG(-2471.645020, -496.794403, 104.574181);

	CreateObject(1270, -2620.171875, -497.509674, 71.466095, 0.0000, 0.0000, 0.0000);
    Drift_AddFLAG(-2620.171875, -497.509674, 71.466095);

	CreateObject(1270, -2406.182617, -422.425018, 85.114494, 0.0000, 0.0000, 0.0000);
    Drift_AddFLAG(-2406.182617, -422.425018, 85.114494);

	CreateObject(1270, -2335.805664, -458.153107, 80.349075, 0.0000, 0.0000, 0.0000);
    Drift_AddFLAG(-2335.805664, -458.153107, 80.349075);

	CreateObject(1270, -2379.793945, -384.753448, 76.872963, 0.0000, 0.0000, 0.0000);
    Drift_AddFLAG(-2379.793945, -384.753448, 76.872963);

	CreateObject(1270, -2671.451172, -428.640320, 31.540670, 0.0000, 0.0000, 0.0000);
    Drift_AddFLAG(-2671.451172, -428.640320, 31.540670);

	CreateObject(1270, -2676.716553, -511.566620, 18.553450, 0.0000, 0.0000, 0.0000);
    Drift_AddFLAG(-2676.716553, -511.566620, 18.553450);

	Xveh = CreateVehicle(562,-2514.183594,-612.370544,132.222870,0,0,0,9999999);
	AddVehicleComponent(Xveh,1146);
	AddVehicleComponent(Xveh,1035);
	AddVehicleComponent(Xveh,1034);
	DriftSet_UpdateDelay(12);
	Drift_SetCheckForFlags();
	return 1;
}

public OnPlayerDriftStart(playerid)
{
    TextDrawSetString(Drift_VAR[playerid][PlayerTextDraw][0], "~w~DRIFTING~n~~g~0");
    TextDrawShowForPlayer(playerid,Drift_VAR[playerid][PlayerTextDraw][0]);
    Drift_VAR[playerid][PlayerDrift] = Drift_VAR[playerid][FlagP] = Drift_VAR[playerid][PlayerCombo] = 0;
    Drift_VAR[playerid][lastflag] = (-1);
    if(Drift_VAR[playerid][TTTimer] != -1)KillTimer(Drift_VAR[playerid][TTTimer]);
	return 1;
}

public OnPlayerDriftUpdate(playerid,value,combo,flagid,Float:distance,Float:speed)
{

	if(flagid != -1)
	{
        pointsnow = floatround(300/distance);
        Drift_VAR[playerid][FlagP]+= pointsnow;
        format(s[0], 128, "~r~]~b~FLAG CATCH~r~]~w~~n~+%d",Drift_VAR[playerid][FlagP]);
        TextDrawSetString(Drift_VAR[playerid][PlayerTextDraw][2], s[0]);
        if(Drift_VAR[playerid][lastflag] != flagid)
		{
			TextDrawShowForPlayer(playerid,Drift_VAR[playerid][PlayerTextDraw][2]);
		}
	}
	else
	{
	    if(Drift_VAR[playerid][lastflag] != flagid)
		{
			if(Drift_VAR[playerid][TimerDD] != -1)
			{
				KillTimer(Drift_VAR[playerid][TimerDD]);
			}
			Drift_VAR[playerid][TimerDD] = SetTimerEx("HideTextDraw",2000,0,"i",playerid);
        }
	}
	
	Drift_VAR[playerid][PlayerCombo] = floatround(floatsqroot(combo)/2);
	
	if(Drift_VAR[playerid][PlayerCombo] > 1)
	{
	    format(s[1], 128, "~r~COMBO~n~~b~x%d",Drift_VAR[playerid][PlayerCombo]);
	    if(Drift_VAR[playerid][Shown] == 0)
	    {
	        Drift_VAR[playerid][Shown] = 1;
	        TextDrawShowForPlayer(playerid,Drift_VAR[playerid][PlayerTextDraw][1]);
	    }
	    TextDrawSetString(Drift_VAR[playerid][PlayerTextDraw][1], s[1]);
	}
	else
	{
	    if(Drift_VAR[playerid][Shown] == 1)
	    {
	        Drift_VAR[playerid][Shown] = 0;
	        TextDrawHideForPlayer(playerid,Drift_VAR[playerid][PlayerTextDraw][1]);
	    }
	}
	
    Drift_VAR[playerid][PlayerDrift] += floatround(value/3)+pointsnow;
    
	format(s[3], 128, "~w~DRIFTING~n~~g~%d",Drift_VAR[playerid][PlayerDrift]);
    TextDrawSetString(Drift_VAR[playerid][PlayerTextDraw][0], s[3]);
    Drift_VAR[playerid][lastflag] = flagid;
    if(Drift_VAR[playerid][DeathTimer] != -1)KillTimer(Drift_VAR[playerid][DeathTimer]);
    Drift_VAR[playerid][DeathTimer] = SetTimerEx("OnPlayerDriftEndX",10000,0,"iiii",playerid,0,0,0);
    
	return 1;
}

forward HideTextDraw(playerid);
public HideTextDraw(playerid)
{
	TextDrawHideForPlayer(playerid,Drift_VAR[playerid][PlayerTextDraw][2]);
	Drift_VAR[playerid][TimerDD] = -1;
	Drift_VAR[playerid][FlagP] = 0;
	return 1;
}

public OnPlayerDriftEnd(playerid,value,combo,reason)
{
	if(reason != DRIFT_REASON_CAR_DAMAGED)
	{
    	Drift_VAR[playerid][TotalDriftPoints] += Drift_VAR[playerid][PlayerDrift]*(Drift_VAR[playerid][PlayerCombo]+1);
	}
    Drift_VAR[playerid][lastflag] = (-1);
    TextDrawHideForPlayer(playerid,Drift_VAR[playerid][PlayerTextDraw][0]);
    TextDrawHideForPlayer(playerid,Drift_VAR[playerid][PlayerTextDraw][1]);
    TextDrawHideForPlayer(playerid,Drift_VAR[playerid][PlayerTextDraw][2]);
    Drift_VAR[playerid][PlayerDrift] = Drift_VAR[playerid][FlagP] = Drift_VAR[playerid][PlayerCombo] = Drift_VAR[playerid][Shown] = 0;
    if(Drift_VAR[playerid][TTTimer] != -1)KillTimer(Drift_VAR[playerid][TTTimer]);
    Drift_VAR[playerid][TTTimer] = SetTimerEx("EndTheDrift",3000,0,"ii",playerid,Drift_VAR[playerid][TotalDriftPoints]);
	return 1;
}

forward OnPlayerDriftEndX(playerid,value,combo,reason);
public OnPlayerDriftEndX(playerid,value,combo,reason)
{
    Drift_VAR[playerid][DeathTimer] = -1;
    OnPlayerDriftEnd(playerid,value,combo,reason);
	return 1;
}

forward EndTheDrift(playerid,points);
public EndTheDrift(playerid,points)
{
    Drift_VAR[playerid][TTTimer] = (-1);
    Drift_VAR[playerid][TotalDriftPoints] = 0;
    if(points != 0)
	{
	    format(s[0],128,"~y~Total drift points: %d",points);
		GameTextForPlayer(playerid,s[0],2500,1);
	}
	return 1;
}

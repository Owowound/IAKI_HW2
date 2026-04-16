#pragma once
#include "Cell.h"

struct CellForArray
{
	int x;
	int y;
	
	bool HasUpperWall = true;
	bool HasLeftWall = true;
	
	float DistanceToTreasure = FLT_MAX;
	
	int32 GetMask()
	{	
		int32 upper = HasUpperWall ? 1 : 0;
		int32 left = HasLeftWall ? 1 : 0;
		return upper * 2 + left;
	};
};
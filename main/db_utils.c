#include <math.h>
#include "db_utils.h"

float dbToRatio(float db, float floor)
{
	if (db == floor)
	{
		return 0.0f;
	}

	float ratio = pow(10, db/20.0f);
	return ratio;
}

float ratioToDb(float ratio, float floor)
{
	if (0.0 == ratio)
	{
		return floor;
	}

	float dB = 20.0f * log(ratio);
	if (dB < floor)
	{
		dB = floor;
	}

	return dB;
}


#include <cubic_curve.h>
#include <math.h>

void setCubicCurve(CubicCurve* cc,
                   float startTarget,
                   float stopTarget,
                   int   preDelaySamples,
                   int   curveSamples)
{
    if (curveSamples <= 0) curveSamples = 1;

    const float h = 1.0f / (float)curveSamples;
    const float delta = stopTarget - startTarget;

    // Cubic coefficients in normalized domain [0,1]
    const float a = startTarget;
    const float b = 1.0f;
    const float c = 3.0f * (delta - 1.0f);
    const float d = -2.0f * (delta - 1.0f);

    // Convert to forward-difference coefficients
    cc->d0 = a;
    cc->d1 = b * h + c * h * h + d * h * h * h;
    cc->d2 = 2.0f * c * h * h + 6.0f * d * h * h * h;
    cc->d3 = 6.0f * d * h * h * h;

    cc->target       = stopTarget;
    cc->startCounter = preDelaySamples > 0 ? preDelaySamples : 0;
    cc->stopCounter  = curveSamples > 0 ? curveSamples : 1;
}

float nextCubicCurveValue( CubicCurve * cc)
{
	float d0 = cc->d0;

	if (cc->startCounter > 0)
	{
		cc->startCounter--;
	}
	else if (cc->stopCounter > 0)
	{
		cc->d0 += cc->d1;
		cc->d1 += cc->d2;
		cc->d2 += cc->d3;
		
		cc->stopCounter--;
	}

	return d0;
}

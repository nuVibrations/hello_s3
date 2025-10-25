typedef struct {
	float d0;
	float d1;
	float d2;
	float d3;

	float target;
	int startCounter;
	int stopCounter;
} CubicCurve;

void setCubicCurve(CubicCurve* cc,
                   float startTarget,
                   float stopTarget,
                   int   preDelaySamples,
                   int   curveSamples);

float nextCubicCurveValue( CubicCurve * cc);

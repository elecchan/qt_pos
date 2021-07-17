#include "KalmanFilter.h"

void KalmanFilterInit(KalFilterInst* kfInst, float r)
{
	kfInst->Q = 1e-5;// process variance
	kfInst->P = 1;
	kfInst->R = r;
	kfInst->lastValue = 0;
}

void KalmanFilter(KalFilterInst* kfInst, float* in, float* out)
{
	float Pminus, K;
	Pminus = kfInst->P + kfInst->Q;
	K = Pminus / (Pminus + kfInst->R);
	out[0] = kfInst->lastValue + K * (in[0] - kfInst->lastValue);

	kfInst->lastValue = out[0];
	kfInst->P = (1 - K) * Pminus;
}

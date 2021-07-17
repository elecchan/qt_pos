typedef struct KalFilterInstT_ {
	float Q;// process variance
	float P;
	float R;
	float lastValue;
}KalFilterInst;

extern void KalmanFilterInit(KalFilterInst* kfInst, float r);
extern void KalmanFilter(KalFilterInst* kfInst, float* in, float* out);
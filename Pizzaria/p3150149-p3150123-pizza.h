const int Ntel = 3;
const int Ncook = 2;
const int Noven = 10;
const int Ndeliverer = 7;
const int Torderlow = 1;
const int Torderhigh = 5;
const int Norderlow = 1;
const int Norderhigh = 5;
const int Tpaymentlow = 1;
const int Tpaymenthigh = 2;
const int Cpizza = 10;
const double Pfail = 0.05;
const int Tprep = 1;
const int Tbake = 10;
const int Tpack = 2;
const int Tdellow = 5;
const int Tdelhigh = 15;

int Ncust, packer = 1, revenueSum=0, successfull=0, failed=0, availTel = Ntel,
	availCook = Ncook, availOven = Noven, availDeliverer = Ndeliverer;

double serviceTimeSum = 0, coolingTimeSum = 0, avgWaitingTime=0, maxWaitingTime=0,
	   avgServiceTime=0, maxServiceTime=0, avgCoolingTime=0, maxCoolingTime=0;

unsigned int seed;
	
pthread_mutex_t telMutex, maxWaitingTimeMutex, avgWaitingTimeMutex, screenMutex,
				failedMutex, successfullMutex, revenueSumMutex, cookMutex, 
				ovenMutex, packerMutex, delivererMutex, maxServiceTimeMutex,
				sumServiceTimeMutex, sumCoolingTimeMutex, maxCoolingTimeMutex;

pthread_cond_t telThresholdCondition, cookThresholdCondition, 
			   ovenThresholdCondition, packerThresholdCondition,
			   delivererThresholdCondition;

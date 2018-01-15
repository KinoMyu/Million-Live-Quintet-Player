#pragma once

#include <thread>
#include <deque>
#include <map>
#include <string>
#include "Semaphore.h"
#include "clHCA.h"

class HCADecodeService
{
public:
    HCADecodeService();
    HCADecodeService(unsigned int num_threads, unsigned int chunksize);
	~HCADecodeService();
	void cancel_decode(void* ptr);
    std::pair<void*, size_t> decode(const std::string& hcafilename, DWORD samplenum);
    void wait_for_finish();
private:
	void Decode_Thread(int id);
	void Main_Thread();
	clHCA workingfile;
    unsigned int numthreads, numchannels, chunksize;
	void* workingrequest;
	std::thread dispatchthread;
	std::deque<std::thread> worker_threads;
    std::map<std::pair<void*, unsigned int>, clHCA> filelist;
	std::deque<unsigned int> blocks;
	int* workingblocks;
	std::deque<Semaphore> workersem;
    Semaphore mainsem, datasem, finsem;
	std::mutex mutex;
	clHCA::stChannel* channels;
	bool shutdown;
};


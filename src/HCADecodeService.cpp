#include <map>
#include "bass.h"
#include "HCADecodeService.h"
#include "clHCA.h"

// Currently multithreaded decode is bugged due to out of order decoding
HCADecodeService::HCADecodeService(unsigned int numthreads)
    : mainsem(1),
      numthreads{1}, // Use single thread for now
      datasem{ 0 },
      finsem{0},
      numchannels{0},
      workingrequest{nullptr},
      channels{nullptr},
      shutdown{false}
{
    workingblocks = new int[this->numthreads];
    channels = new clHCA::stChannel[0x10 * this->numthreads];
    for (unsigned int i = 0; i < this->numthreads; ++i)
	{
		workersem.emplace_back(0);
		worker_threads.emplace_back(&HCADecodeService::Decode_Thread, this, i);
		workingblocks[i] = -1;
	}
	dispatchthread = std::thread{ &HCADecodeService::Main_Thread, this };
}

HCADecodeService::~HCADecodeService()
{
	shutdown = true;
	datasem.notify();
	dispatchthread.join();
    delete[] channels;
}

void HCADecodeService::cancel_decode(void* ptr)
{
	mutex.lock();
	for (auto it = filelist.begin(); it != filelist.end(); ++it)
	{
        if (it->first.first == ptr)
		{
			filelist.erase(it);
			break;
		}
	}
	if (workingrequest == ptr)
	{
		blocks.clear();
    }
	mutex.unlock();
    for (unsigned int i = 0; i < numthreads; ++i)
    {
        while (workingblocks[i] != -1); // busy wait until threads are finished
    }
}

void HCADecodeService::wait_for_finish()
{
    mutex.lock();
    while(!filelist.empty() || !blocks.empty())
    {
        mutex.unlock();
        finsem.wait();
        mutex.lock();
    }
    mutex.unlock();
}

std::pair<void*, size_t> HCADecodeService::decode(const std::string& filename, DWORD samplenum)
{
	clHCA hca(0xBC731A85, 0x0002B875);
	void* wavptr = nullptr;
	size_t sz = 0;
	hca.Analyze(wavptr, sz, filename.c_str());
	if (wavptr != nullptr)
	{
        unsigned int blocknum = samplenum/(1024 * hca.get_channelCount());
		mutex.lock();
        filelist[std::make_pair(wavptr, blocknum)] = std::move(hca);
		mutex.unlock();
		datasem.notify();
	}
	return std::pair<void*, size_t>(wavptr, sz);
}

void HCADecodeService::Main_Thread()
{
	while (true)
	{
		datasem.wait();
		if (shutdown)
		{
			break;
		}
		mutex.lock();
		auto it = filelist.begin();
        workingrequest = it->first.first;
		workingfile = std::move(it->second);
        unsigned blocknum = it->first.second;
		filelist.erase(it);
        numchannels = workingfile.get_channelCount();
		workingfile.PrepDecode(channels, numthreads);
		unsigned int blockCount = workingfile.get_blockCount();
        // initiate playback right away, and patch up "blip" cause by out of order decoding
        for (unsigned int i = blocknum, j = 0; j < blockCount + blocknum; ++i, ++j)
		{
            blocks.push_back(i % blockCount);
		}
		while (!blocks.empty())
		{
			mutex.unlock();
			mainsem.wait();
            for (unsigned int i = 0; i < numthreads; ++i)
			{
				mutex.lock();
				if (workingblocks[i] == -1 && !blocks.empty())
				{
					workingblocks[i] = blocks.front();
					blocks.pop_front();
					workersem[i].notify();
				}
				mutex.unlock();
			}
			mainsem.notify();
			mutex.lock();
		}
		mutex.unlock();
        for (unsigned int i = 0; i < numthreads; ++i)
		{
			while (workingblocks[i] != -1); // busy wait until threads are finished
        }
        finsem.notify();
	}
	for (int i = 0; i < numthreads; ++i)
	{
		workersem[i].notify();
		worker_threads[i].join();
	}
}

void HCADecodeService::Decode_Thread(int id)
{
	workersem[id].wait();
	while (workingblocks[id] != -1)
	{
		mainsem.wait();
		workingfile.AsyncDecode(channels + (id * numchannels), workingblocks[id], (unsigned char*)workingrequest);
		workingblocks[id] = -1;
		mainsem.notify();
		workersem[id].wait();
	}
}

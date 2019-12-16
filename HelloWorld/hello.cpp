#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <CL/cl2.hpp>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
using namespace std;

int main() {
    vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    auto platform = platforms.front();
    vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    auto device = devices.front();

    ifstream helloWorldFile("HelloWorld/HelloWorld.cl");

    cout << helloWorldFile.is_open() << endl;
    string src(istreambuf_iterator<char>(helloWorldFile), (istreambuf_iterator<char>()));
    cout << src << endl;
    cl::Program::Sources sources(1, src);
    cout << sources.size() << endl;

    cl::Context context(device);
    cl::Program program(context, sources);

    auto err = program.build("-cl-std=CL1.2");
    cout << err << endl;
    char buf[16];
    cl::Buffer memBuf(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, sizeof(buf));
    cl::Kernel kernel(program, "HelloWorld", &err);
    kernel.setArg(0, memBuf);

    cl::CommandQueue queue(context, device);
    
    queue.enqueueTask(kernel);
    queue.enqueueReadBuffer(memBuf, true, 0, sizeof(buf), buf);

    cout << buf << endl;

    return 0;
}
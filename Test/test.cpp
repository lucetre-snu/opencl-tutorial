#include <CL/cl2.hpp>
#include <vector>
#include <iostream>
using namespace std;


int main() {
    vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    cout << "platforms size: " << platforms.size() << endl;

    auto platform = platforms.front();
    vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

    cout << "devices size: " << devices.size() << endl;


    for(int i = 0; i < devices.size(); i++) {
        auto device  = devices[i];
        auto vendor = device.getInfo<CL_DEVICE_VENDOR>();
        auto version = device.getInfo<CL_DEVICE_VERSION>();
        cout << ">> " << (i+1) << "th device" << endl;
        cout << "vendor: " << vendor << endl;
        cout << "version: " << version << endl;

    }

    return 0;
}
#include <iostream>
#include <fstream>

#include "input.pb.h"

using namespace std;

int main()
{
    char* corpus = "aaaabbbbccccdddd";
    char* corpus2 = "ffffgggghhhh";

    afl_input::Input input;
    afl_input::Packet* packet = input.add_packets();
    packet->set_buffer(corpus);
    afl_input::Packet* packet2 = input.add_packets();
    packet2->set_buffer(corpus2);

    cout << "num packets: " << input.packets_size() << endl;
    cout << "byte size: " << input.ByteSizeLong() << endl;

    string s;
    input.SerializeToString(&s);

    afl_input::Input input2;
    input.ParseFromArray(s.data(), s.size());
    string s2;
    input.SerializeToString(&s2);

    ofstream ofile("test.bin", ios::out | ios::binary);
    ofile << s2;
    ofile.close();

    cout << "test.bin should be serialized protobuf with strings " << corpus << " and " << corpus2 << endl;

    return 0;
}
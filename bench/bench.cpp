// bench.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <cstdio>

#include <rawaccel-io.hpp>

namespace ra = rawaccel;

void show(ra::latency_info info) {
    auto us = [&](auto ticks) -> double {
        return (ticks * 1000000) / (double)info.freq;
    };
    double avg = (double)info.sum / info.count;
    printf("latency\n\n"
        "max us: %f\n" 
        "max ct: %lli\n"
        "packets@max: %i\n\n"
        "avg us: %f\n"
        "avg ct: %f\n\n"
        "num runs: %lli\n"
        "max packets: %i\n"  
        "min elapsed time(us): %f\n",
        us(info.max),
        info.max,
        info.packets_at_max_lat,
        us(avg),
        avg,
        info.count,
        info.max_packets,
        us(info.elapsed_min));
}

int main(int argc, char** argv)
{
    auto cmds = [] {
        puts("commands:\n"
            "\tget, g\tshow latency\n"
            "\treset, r\tshow latency & reset counter");
    };

    if (argc == 2) {
        auto arg_is = [&](auto... args) { 
            return ((strcmp(argv[1], args) == 0) || ...); 
        };
        if (arg_is("g", "get")) {
            show(ra::get_latency_info());
        }
        else if (arg_is("r", "reset")) {
            show(ra::reset_latency_info());
        }
        else {
            cmds();
        }
    }
    else {
        cmds();
    }
}

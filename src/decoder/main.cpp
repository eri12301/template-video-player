#include "audio_demux_decode.hpp"
#include <cstdio>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int ret = 0;

    if (argc != 3)
    {
        fprintf(stderr, "usage: %s  input_file audio_output_file\n"
                        "API example program to show how to read frames from an input file.\n"
                        "This program reads frames from a file, decodes them, and writes decoded\n"
                        "audio frames to a rawaudio file named audio_output_file.\n",
                argv[0]);
        exit(1);
    }
    auto adec = new AudioDecoder();
    return adec->demuxDecode(argv[1], argv[2]);
}
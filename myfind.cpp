#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
/* Include Datei für getopt(); getopt.h geht auch, ist aber älter. */
#include <unistd.h>
#include <vector>
#include <getopt.h>
#include <filesystem>
#include <thread>
#include <mutex>

using namespace std;

std::mutex m;

void printUsage(string programName)
{
    cout << "Usage: " << programName << "  [-R] [-i] searchpath filename1 [filename2]…[filenameN]" << endl;
    return;
}

void findFile(string searchPath, string fileName)
{
    m.lock();
    cout << fileName << endl;
    m.unlock();
}

/* Entry Point */
int main(int argc, char *argv[])
{
    int c;
    bool optionRUsed = false;
    bool optionIUsed = false;
    bool error = false;

    bool recMode = false;
    string searchPath;
    vector<string> filenames;

    string programName = argv[0];

    while ((c = getopt(argc, argv, "Ri")) != EOF)
    {
        switch (c)
        {
            case '?':
                error = true;
                cerr << programName << " error: Unknown option." << endl;
            case 'R':
                if(optionRUsed) {
                    error = true;
                    printUsage(programName);
                }
                optionRUsed = true;
                recMode = true;
                break;
            case 'i':
                if(optionIUsed) {
                    error = true;
                    printUsage(programName);
                }
                optionIUsed = true;
                break;
            default:
                assert(0);
        }
    }

    if(error) {
        printUsage(programName);
        exit(1);
    }
    if ((argc < optind + 1) || (argc > optind + 2)) /* falsche Anzahl an Optionen */
    {
        printUsage(programName);
    }

    /* Die restlichen Argumente, die keine Optionen sind, befinden sich in

        * argv[optind] bis argv[argc-1]
        */
    int argCounter = 0;
    while (optind < argc)
    {
        /* aktuelles Argument: argv[optind] */
        cout << programName << ": parsing argument " << argv[optind] << endl;

        if(argCounter == 0)
            searchPath = argv[optind];
        else
            filenames.push_back(argv[optind]);

        optind++;
        argCounter++;
    }
    if(searchPath.empty() || filenames.size() < 1) {
        printUsage(programName);
        exit(1);
    }

    thread threads[filenames.size()];
    // cout << searchPath << endl;
    for(int i = 0; i < filenames.size(); i++) {
        // cout << f << endl;
        threads[i] = thread(findFile, searchPath, filenames[i]);
    }

    for(int i = 0; i < filenames.size(); i++) {
        threads[i].join();
    }

    return 0;
}

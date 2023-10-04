#include <iostream>
#include <vector>
#include <getopt.h>
#include <filesystem>
#include <sys/wait.h>
#include <cassert>
#include <cstring>

using namespace std;

void printUsage(string programName)
{
    cout << "Usage: " << programName << "  [-R] [-i] searchpath filename1 [filename2]…[filenameN]" << endl;
    return;
}

void checkFile(bool caseInsensitiveMode, const char* fileName, filesystem::directory_entry entry)
{
    if(caseInsensitiveMode ? strcasecmp(fileName, entry.path().filename().c_str()) == 0 : strcmp(fileName, entry.path().filename().c_str()) == 0) {
        cout << getpid() << " : " << fileName << " : " << filesystem::canonical(entry.path()) << endl;
    }
}

void findFile(string searchPath, string fileName, bool recursive, bool caseInsensitiveMode)
{
    if(!filesystem::exists(searchPath))
        return;

    if(!recursive) {
        for (auto& entry : filesystem::directory_iterator(searchPath))
        {
            if (!filesystem::is_directory(entry))
                checkFile(caseInsensitiveMode, fileName.c_str(), entry);
        }
    } else {
        for (auto& entry : filesystem::recursive_directory_iterator(searchPath)) {
            if (!filesystem::is_directory(entry))
                checkFile(caseInsensitiveMode, fileName.c_str(), entry);
        }
    }
}

/* Entry Point */
int main(int argc, char *argv[])
{
    int c;
    bool error = false;

    bool recMode = false;
    bool caseInsensitiveMode = false;
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
                break;
            case 'R':
                if(recMode) {
                    error = true;
                    printUsage(programName);
                }
                recMode = true;
                break;
            case 'i':
                if(caseInsensitiveMode) {
                    error = true;
                    printUsage(programName);
                }
                caseInsensitiveMode = true;
                break;
            default:
                assert(0);
        }
    }

    // check for the minimum required number of arguments
    if (argc - optind < 2) {
        error = true;
    }

    if(error) {
        printUsage(programName);
        exit(1);
    }

    /* extract searchpath and filenames in argv[optind] to argv[argc-1] */
    searchPath = argv[optind];
    optind++;
    while (optind < argc)
    {
        /* current argument: argv[optind] */
        // cout << programName << ": parsing argument " << argv[optind] << endl;
        filenames.push_back(argv[optind]);
        optind++;
    }

    if(searchPath.empty() || filenames.size() < 1) {
        printUsage(programName);
        exit(1);
    }

    for (auto filename : filenames)
    {
        auto pid = fork();
        if (pid == 0)
        {
            findFile(searchPath, filename, recMode, caseInsensitiveMode);
            break;
        }
    }

    pid_t childpid;
    while ((childpid = waitpid(-1, NULL, 0)))
    {
        if ((childpid == -1) && (errno != EINTR))
        {
            break;
        }
    }
    return 0;
}

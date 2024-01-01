// JPGconcatinator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>

#include <filesystem>
#include <direct.h>

#include <stdint.h>

#include "AVIconcatinator.h"

//#include <wingdi.h>

#define BUF_SIZE 1024*1024


using namespace std;
namespace fs = std::filesystem;
using namespace fs;


int main()
{
  
    int retval;
    std::cout << "Starting the algorithm!\n";
    std::string path = "M:\\film";
    string temp;

    FILE* plik;
    FILE_AVI_t AVIFILE;
    fileAVIConfig_t AVICONF = {
        .width = 1280,
        .height = 1024,
        .fps = 30,
        .framesBeforeSave = 0
    };
    AVIFILE.config = &AVICONF;

    retval = AVIOpenNew(&AVIFILE, "M:\\AVIfile4.avi", AVI_CREATE_ALWAYS);
    if (retval) {
        printf("error: %d", retval);
        exit(0);
    }
 
    char* databuf = (char*)malloc(BUF_SIZE);
    if (databuf == 0) {
        cout << "problem malloc!" << temp << "\n";
        while (1);
    }
    size_t datalen;

    for (const auto& entry : fs::directory_iterator(path) ) 
    {
        temp = entry.path().string();
        std::cout << "Appending: " << temp << std::endl;
        fopen_s(&plik, temp.c_str(),"rb");
        
        if (plik == 0) {
            cout << "problem!" << temp << "\n";
            while (1);
        }

        //Read and fragment
        fseek(plik, 0L, SEEK_END);
        datalen = ftell(plik);
        fseek(plik, 0L, SEEK_SET);
        fread(databuf, 1, datalen, plik);
        
        AVIAttachFrame(&AVIFILE, (DATA_PTR*) databuf, datalen);

    }

    AVIClose(&AVIFILE);
    cout << "DONE!" << endl;
    free(databuf);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

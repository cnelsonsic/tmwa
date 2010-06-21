#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

#include "mmo.h"
#include "athena_text.h"

#define ATHENA_FILE "save/athena.txt"
#define ACCREG_FILE "save/accreg.txt"

std::vector<int> values;

void countAthena()
{
    int total = 0;
    std::string input;
    std::ifstream fp(ATHENA_FILE);
    char *buffer = new char[65536];

    while (fp.good())
    {
        std::getline(fp,input);
        mmo_charstatus *thisChar = new struct mmo_charstatus;

        strcpy(buffer,input.c_str());

        if (mmo_char_fromstr(buffer, thisChar))
        {
            total++;
            values.push_back(thisChar->zeny);
        }

        delete thisChar;
    }


    std::cout << "Parsed a total of " << total << " lines in " << ATHENA_FILE << std::endl << std::endl;

    delete [] buffer;
    fp.close();
}

void countAccReg()
{
     int total = 0;
     std::ifstream fp(ACCREG_FILE);
     char *buffer = new char[65536];
     while (fp.good())
     {
         std::string line;
         std::getline(fp, line);
         struct accreg *reg = new struct accreg;

         strcpy(buffer, line.c_str());

         if (accreg_fromstr(buffer, reg))
         {
            total++;
            for (int i = 0; i < reg->reg_num; i++)
            {
                if (strcmp(reg->reg[i].str,"#BankAccount") == 0)
                {
                    values.push_back(reg->reg[i].value);
                }
            }
         }

         delete reg;
     }

     std::cout << "Parsed a total of " << total << " lines in " << ACCREG_FILE << std::endl << std::endl;

     delete [] buffer;
     fp.close();
}

long long stdDevTotal = 0;
long long sum = 0;
int mean = 0;

bool lessthan (int i,int j) { return (i<j); }
void findstddev(int i) { stdDevTotal += (i - mean) * (i - mean); }
void findSum(int i) { sum += i; }

void showStats()
{
    // Reset globals
    sum = 0;
    mean = 0;
    stdDevTotal = 0;

    std::sort(values.begin(), values.end(), lessthan);
    std::for_each(values.begin(), values.end(), findSum);

    long long total = sum;
    int count = values.size();
    mean = total / count;

    std::for_each(values.begin(), values.end(), findstddev);

    int a4th = count / 4;
    int a10th = count / 10;

    int lower = values[0],

        t1 = values[a10th * 1],
        t2 = values[a10th * 2],

        q1 = values[a4th * 1],

        t3 = values[a10th * 3],
        t4 = values[a10th * 4],

        median = values[a4th * 2],

        t6 = values[a10th * 6],
        t7 = values[a10th * 7],

        q3 = values[a4th * 3],

        t8 = values[a10th * 8],
        t9 = values[a10th * 9],

        upper = values[count - 1];

    std::cout << "Sum = " << total
              << "\nCount = " << count
              << "\nMean = " << mean
              << "\nSimple Variance = " << (stdDevTotal / (count - 1))
              << "\nStandard Deviation = " << std::sqrt(stdDevTotal / (count - 1))
              << "\nLower bound = " << lower
              << "\n10th Percentile = " << t1
              << "\n20th Percentile  = " << t2
              << "\nQ1 = " << q1
              << "\n30th Percentile = " << t3
              << "\n40th Percentile = " << t4
              << "\nMedian = " << median
              << "\n60th Percentile = " << t6
              << "\n70th Percentile = " << t7
              << "\nQ3 = " << q3
              << "\n80th Percentile = " << t8
              << "\n90th Percentile = " << t9
              << "\nUpper bound = " << upper << std::endl << std::endl;
}

int main()
{
    countAthena();
    std::cout << "The stats for player held money is:" << std::endl;
    showStats();
    values.clear();

    countAccReg();
    std::cout << "The stats for bank held money is:" << std::endl;
    showStats();

    return 0;
}


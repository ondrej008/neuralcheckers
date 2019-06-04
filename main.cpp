#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <fstream>
#include <time.h>
#include <cmath>
#include <chrono>
#include "network.h"
#include "checkers.h"

using namespace std;

// https://blog.coast.ai/lets-evolve-a-neural-network-with-a-genetic-algorithm-code-included-8809bece164

/*
    Creates a population of (randomly generated) members
    Scores each member of the population based on some goal. This score is called a fitness function.
    Selects and breeds the best members of the population to produce more like them
    Mutates some members randomly to attempt to find even better candidates
    Kills off the rest (survival of the fittest and all), and
    Repeats from step 2. Each iteration through these steps is called a generation.

*/

// params


int main()
{
    srand(time(NULL));

    vector<NN> nets;

    for(int i = 0; i < 20; i++)
    {
        NN newNN = NN({32, 64, 1});
        newNN.name = to_string(i);
        cout << "New challenger approaching, " << newNN.name << endl;

        char fn[256];
        sprintf(fn, "NN%d.txt", i);
        newNN.saveNetworkParams(fn);

        nets.push_back(newNN);
    }

    // do checkers stuff here
    auto start = chrono::high_resolution_clock::now();
    // neural networks, debug level, has to jump, force multi jump
    doCheckers(nets, 0, false, false);
    auto end = chrono::high_resolution_clock::now();

    auto diff = end - start;

    cout << "Time taken: " << (diff.count() / 1000.0) / 1000.0 / 1000.0 << endl;

    double avgFitness = 0;
    int best = 0;

    for(int i = 0; i < nets.size(); i++)
    {
        cout << "Fitness of " << nets[i].name << ": " << nets[i].fitness << endl;
        avgFitness += nets[i].fitness;
        if(nets[i].fitness > nets[best].fitness)
        {
            best = i;
        }
    }

    cout << "Average fitness was " << avgFitness / nets.size() << endl;
    cout << "Highest fitness was " << nets[best].fitness << endl;

    return 0;
}
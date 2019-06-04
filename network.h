#pragma once

#include <vector>
#include <fstream>
#include <cmath>
#include <time.h>
#include <stdlib.h>
#include <chrono>
#include <random>

#include "matrix.h"

using namespace std;

unsigned long long seed = chrono::high_resolution_clock::now().time_since_epoch().count();
default_random_engine generator(seed);


struct NN
{
    string name = "";
    int fitness = 0;

    NN(std::vector<int> neurons);
    NN(const char *filepath);

    Matrix<double> computeOutput(std::vector<double> input);

    void saveNetworkParams(const char *filepath);
    void loadNetworkParams(const char *filepath);

    NN breed(NN& other);
    NN mutate(double chance);

    std::vector<Matrix<double> > W;
    std::vector<Matrix<double> > B;
    
private:
    std::vector<Matrix<double> > H;

    Matrix<double> Y;

    int hiddenLayersCount;

    static double random(double x);
    static double sigmoid(double x);
    static double sigmoidePrime(double x);

    void printToFile(Matrix<double> &m, std::ofstream &file);
};

NN::NN(std::vector<int> neurons)
{
    this->hiddenLayersCount = neurons.size()-2;

    H = std::vector<Matrix<double> >(hiddenLayersCount+2);
    W = std::vector<Matrix<double> >(hiddenLayersCount+1);
    B = std::vector<Matrix<double> >(hiddenLayersCount+1);

    for (int i=0 ; i<neurons.size()-1 ; i++)
    {
        W[i] = Matrix<double>(neurons[i], neurons[i+1]);
        B[i] = Matrix<double>(1, neurons[i+1]);

        W[i] = W[i].applyFunction(random);
        B[i] = B[i].applyFunction(random);
    }
}

NN::NN(const char *filepath)
{
    loadNetworkParams(filepath);
}

NN NN::breed(NN& other)
{
    return *this;
}

NN NN::mutate(double chance)
{
    return *this;
}

Matrix<double> NN::computeOutput(std::vector<double> input)
{
    H[0] = Matrix<double>({input}); // row matrix

    for (int i=1 ; i<hiddenLayersCount+2 ; i++)
    {
        H[i] = H[i-1].dot(W[i-1]).add(B[i-1]).applyFunction(sigmoid);
    }

    return H[hiddenLayersCount+1];
}

void NN::printToFile(Matrix<double> &m, std::ofstream &file)
{
    int h = m.getHeight();
    int w = m.getWidth();

    file << h << std::endl;
    file << w << std::endl;
    for (int i=0 ; i<h ; i++)
    {
        for (int j=0 ; j<w ; j++)
        {
            file << m.get(i,j) << (j!=w-1?" ":"");
        }
        file << std::endl;
    }
}

void NN::saveNetworkParams(const char *filepath)
{
    std::ofstream out(filepath);

    out << hiddenLayersCount << std::endl;

    for (Matrix<double> m : W){
        printToFile(m, out);
    }

    for (Matrix<double> m : B){
        printToFile(m, out);
    }

    out.close();
}

void NN::loadNetworkParams(const char *filepath)
{
    std::ifstream in(filepath);
    std::vector<Matrix<double> > params;
    double val;
    int h,w;

    if(in)
    {
        in >> hiddenLayersCount;

        H = std::vector<Matrix<double> >(hiddenLayersCount+2);
        W = std::vector<Matrix<double> >(hiddenLayersCount+1);
        B = std::vector<Matrix<double> >(hiddenLayersCount+1);

        for(int i=0 ; i<2*hiddenLayersCount+2 ; i++)
        {
            in >> h;
            in >> w;
            Matrix<double> m(h,w);
            for (int hh=0 ; hh<h ; hh++)
            {
                for (int ww=0 ; ww<w ; ww++)
                {
                    in >> val;
                    m.put(hh,ww,val);
                }
            }

            params.push_back(m);
        }
    }
    in.close();

    // assign values
    for (int i=0 ; i<hiddenLayersCount+1 ; i++)
    {
        W[i] = params[i];
    }

    for (int i=hiddenLayersCount+1 ; i<params.size() ; i++)
    {
        B[i-hiddenLayersCount-1] = params[i];
    }
}

double NN::random(double x)
{
    return (double)(rand() % 10000 + 1)/10000-0.5;
    
    /*uniform_real_distribution<double> distribution(0.0, 1.0);
    double result = distribution(generator);

    if(result == 0)
    {
        result = 0.00001;
    }

    return result;*/
}

double NN::sigmoid(double x)
{
    return 1/(1+exp(-x));
}

double NN::sigmoidePrime(double x)
{
    return exp(-x)/(pow(1+exp(-x), 2));
}
//
//

#ifndef SLICEDOPTIM_MY_UTILITY_H
#define SLICEDOPTIM_MY_UTILITY_H

#include <random>
#include <functional>
#include <string>
#include "../Math/VecX.h"

const double PI =3.141592653589793238463;

template<class VECTYPE, class RandomGenerator>
inline VECTYPE randomVectorInBall(int dim, RandomGenerator &engine) {
    std::normal_distribution<double> normalDistribution(0, 1);
    std::uniform_real_distribution<double> unif(0, 1);
    VECTYPE v(dim);

    for (int j = 0; j < dim; ++j) {
        v[j] = normalDistribution(engine);
    }
    v.normalize();
    v *= std::pow(unif(engine), 1. / dim);

    return v;
}

template<class VECTYPE, class RandomGenerator>
inline VECTYPE randomVectorInCube(int dim, RandomGenerator &engine) {
    std::uniform_real_distribution<double> unif(0, 1);
    VECTYPE v(dim);

    for (int j = 0; j < dim; ++j) {
        v[j] = unif(engine);
    }

    return v;
}

double clamp(double v, double min, double max);

double inverseFunction(std::function<double(double)> &f, std::function<double(double)> &df, double v);

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

template <class VECTYPE>
VECTYPE toroidal_minus(VECTYPE& v1, VECTYPE& v2){
    VECTYPE res(v1.dim());
    for(int i=0; i<v1.dim();++i){
        res[i] = v1[i] - v2[i];
        if(1.0 > (v1[i] - v2[i] + 1) && 0.0 <= (v1[i] - v2[i] + 1)){
            res[i] = v1[i] - v2[i] + 1;
        }
        if(1.0 > (v1[i] - v2[i] - 1) && 0.0 <= (v1[i] - v2[i] - 1)){
            res[i] = v1[i] - v2[i] - 1;
        }
    }
    return res;
}

template <class VECTYPE>
float toroidal_norm(VECTYPE& v1, VECTYPE& v2){
    VECTYPE res(v1.dim());
    for(int i=0; i<v1.dim();++i){
        res[i] = v1[i] - v2[i];
        if(std::pow(res[i],2) > std::pow(v1[i] - v2[i] + 1,2)){
            res[i] = v1[i] - v2[i] + 1;
        }
        if(std::pow(res[i],2) > std::pow(v1[i] - v2[i] - 1,2)){
            res[i] = v1[i] - v2[i] - 1;
        }
    }
    return res.norm();
}

/**
 * Choose \p m directions in N dimension N being defined by the dimention of the content of directions.
 * Two selection methods are available. Either the direction are uniformly selected in ND
 * or one can force half of the them to lie in 2D planes to optimize the projection repartition as well.
 *
 * @param directions Table of directions to output. Must be initialized with \p m VECTYPE of the disired dimension
 * @param m Number of directions to pick
 * @param seed Seed for the random generator. Only applied once
 * @param projective If true half the directions will lie in 2D planes.
 */
template <class VECTYPE>
inline void chooseDirectionsND(std::vector<VECTYPE>& directions, int m, int seed){

    static std::mt19937 generatorND(seed);
    static std::normal_distribution<>normalND;
    static std::uniform_real_distribution<double> unif(0, 1);

    int dim = directions.front().dim();

    for (int k = 0; k < m; ++k){
        if(dim == 2){
            double rnd =  unif(generatorND);
            if(rnd < 0.7){ // change for 0.7 to unable axis aligned projections
                double theta =  (float(k)/m + unif(generatorND)/float(m))*2*PI; // stratified 2D directions
                directions[k][0] = cos(theta);
                directions[k][1] = sin(theta);
            }else if(rnd < 0.85){
                directions[k][0] = 1.0;
                directions[k][1] = 0.0;
            }else{
                directions[k][0] = 0.0;
                directions[k][1] = 1.0;
            }
        }else{
            for (int j = 0; j < dim; ++j){
                directions[k][j] = normalND(generatorND);
            }
        }   
        directions[k].normalize();
    }
}

/**
 * Export a .h file with an array containing the samples and function to get them.
 *
 * @param points Table containing the points \f$ x_j \f$
 * @param filename name of the file to export
 * @param tile_size size of the toroidal tile
 * @param spp number of samples per pixels
 */
template <class VECTYPE>
void export_sampler(const std::vector<VECTYPE>& points, std::string filename, int tile_size, int spp, int nb_frames){
    int dim = points.front().dim();
    std::ofstream myfile (filename,std::ios::trunc);
    if (myfile.is_open())
    {
        myfile << "#pragma once\n\n\n";
        myfile << "const float tile["<<nb_frames<<"]["<<tile_size<<"]["<<tile_size*spp*dim<<"] = {";
        for (size_t f = 0; f < nb_frames; f++)
        {
            if(f==0){myfile <<"{";}
                else{myfile <<",{";}
            for (size_t i = 0; i < tile_size; i++)
            {
                if(i==0){myfile <<"{";}
                else{myfile <<",{";}
                
                for (size_t j = 0; j < tile_size; j++)
                {
                    for (size_t k = 0; k < spp; k++)
                    {
                        for (size_t d = 0; d < dim; d++)
                        {
                            if(j==0 && k == 0 && d==0){myfile <<points[(i*tile_size+j)*spp+k][d];}
                            else{myfile <<","<<points[(i*tile_size+j)*spp+k][d];}
                        }
                    }
                }
                myfile <<"}";
            }
            myfile <<"}";
        }
        myfile <<"};";
        myfile <<"\n\n\n";
        myfile <<"float sample(int f,int i, int j, int s, int d){\n";
        myfile <<"\treturn tile[f%"<<nb_frames<<"][i%"<<tile_size<<"][(j%"<<tile_size<<")*"<<spp<<"*"<<dim<<"+(s%"<<spp<<")*"<<dim<<"+(d%"<<dim<<")];\n";
        myfile <<"}\n";
        myfile.close();
    }
    
}



#endif //SLICEDOPTIM_MY_UTILITY_H

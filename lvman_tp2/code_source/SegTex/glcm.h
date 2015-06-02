//
//
// glcm.h
//
// Une matrice de co-occurrences
// en se basant et corrigant le code de Daniel Eaton (cvtexture.cpp)
//
// LE Viet Man
// 16/10/10
//
//

#ifndef GLCM_H
#define GLCM_H

#include <cv.h>
#include <iostream>

using namespace std;

#define DES_CON 0 // contrast
#define DES_DIS 1 // dissimilarity
#define DES_HOM 2 // homogeneity
#define DES_ASM 3 // ASM
#define DES_ENG 4 // Energy
#define DES_MAX 5 // max probability
#define DES_ENT 6 // Entropy
#define DES_MEAN_I 7 // GLCM Mean
#define DES_MEAN_J 8
#define DES_VAR_I 9 // GLCM Variance
#define DES_VAR_J 10
#define DES_STD_I 11 // Standard Deviation
#define DES_STD_J 12
#define DES_CORR 13 // GLCM Correlation

#define MAX_DESCRIPTORS 14
#define MAX_DISTANCES 4

const int directions4[] = {0,1, 1,1, 1,0, 1,-1}; // pour la distance au moins 1
const int directions8[] = {0,2, 1,2, 2,2, 2,1, 2,0, 2,-1, 2,-2, 1,-2}; // pour la distance 2 et 4
const int directions12[] = {0,3, 1,3, 2,3, 3,3, 3,2, 3,1, 3,0,
                            3,-1, 3,-2, 3,-3, 2,-3, 1,-3}; // pour la distance 3
const int directions16[] = {0,4, 1,4, 2,4, 3,4, 4,4, 4,3, 4,2, 4,1, 4,0,
                            4,-1, 4,-2, 4,-3, 4,-4, 3,-4, 2,-4, 1,-4}; // pour la distance 4

class GLCM
{
public:
    GLCM(int size, IplImage *img, int distance, int numDirections);

    ~GLCM();

    double getDescriptor(int indexMatrix, int descriptor);

private:
    int matrixSize; // 8, 16, 32, 64, 128, 256
    int numMatrix; // = numDirections
    double*** matrices;

    double** descriptors;
    int numDescriptors; // = MAX_DESCRIPTORS * numDirections

    void calculDescriptors(int indexMatrix);
};

#endif // GLCM_H

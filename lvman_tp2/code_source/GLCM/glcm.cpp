//
//
// glcm.cpp
//
// Implementation de la classe glcm.h
//
// LE Viet Man
// 16/10/10
//
//

#include "glcm.h"

//
// Initier
// @param:
//      int size : le niveau de gris
//      IplImage *img : une image
//      int distance : une distance
//      int numDirections : nombre de directions
//
GLCM::GLCM(int size, IplImage *img, int distance, int numDirections)
{
    if (!img)
    {
        cerr << "Ne pas avoir l'image." << endl;
        return;
    }

    // Evaluer la distance s'accorde avec numDirections ?
    if ((distance == 8) && ((numDirections == 1) || (numDirections == 3)))
    {
        return;
    }
    if ((distance == 12) && (numDirections != 3))
    {
        return;
    }
    if ((distance == 16) && (numDirections != 4))
    {
        return;
    }

    // Faire deux etapes
    // 1. Calculer la matrice de co-occurrences

    // preparer des directions
    int* directions = 0;

    directions = (int*)malloc(sizeof(int) * numDirections * 2);
    switch (numDirections)
    {
    case 4:
        memcpy( directions, directions4, sizeof(int) * numDirections * 2);
        break;
    case 8:
        memcpy( directions, directions8, sizeof(int) * numDirections * 2);
        break;
    case 12:
        memcpy( directions, directions12, sizeof(int) * numDirections * 2);
        break;
    case 16:
        memcpy( directions, directions16, sizeof(int) * numDirections * 2);
        break;
    }

    switch (numDirections)
    {
    case 4:
        for (int i = 0; i < numDirections; i++)
        {
            directions[i * 2 + 0] *= distance;
            directions[i * 2 + 1] *= distance;
        }
        break;
    case 8:
        for (int i = 0; i < numDirections; i++)
        {
            directions[i * 2 + 0] *= distance / 2;
            directions[i * 2 + 1] *= distance / 2;
        }
        break;
    }

    // initier les matrice de co-occurrence
    // il y a numDirections matrices de co-occurrence pour une distance
    this->matrixSize = size;
    this->numMatrix = numDirections;

    this->matrices = (double***)malloc(sizeof(double**) * numMatrix);

    for (int i = 0; i < numMatrix; i++)
    {
        matrices[i] = (double**)malloc(sizeof(double*) * matrixSize);
        for (int j = 0; j < matrixSize; j++)
        {
            matrices[i][j] = (double*)malloc(sizeof(double) * matrixSize);
            memset(matrices[i][j], 0, sizeof(double) * matrixSize);
        }
    }

    // calculer GLCM
    int *sumPixelEachMatrix = (int*)malloc(sizeof(int) * numMatrix);
    memset(sumPixelEachMatrix, 0, sizeof(int) * numMatrix);

    CvScalar s;
    int pixelValue1, pixelValue2;
    int col, row;
    for (int i = 0; i < img->height; i++)
    {
        for (int j = 0; j < img->width; j++)
        {
            s = cvGet2D(img, i, j);
            pixelValue1 = s.val[0];

            for (int k = 0; k < numDirections; k++)
            {
                row = i + directions[k * 2 + 0];
                col = j + directions[k * 2 + 1];

                if (col >=0 && row >= 0 && col < img->width && row < img->height)
                {
                    s = cvGet2D(img, row, col);
                    pixelValue2 = s.val[0];

                    matrices[k][pixelValue1][pixelValue2] ++;
                    matrices[k][pixelValue2][pixelValue2] ++;

                    sumPixelEachMatrix[k] += 2;
                }
            }
        }
    }

    // normaliser les matrices.
    for (int i = 0; i < matrixSize; i++)
    {
        for (int j = 0; j < matrixSize; j++)
        {
            for (int k = 0; k < numDirections; k++)
            {
                matrices[k][i][j] /= double(sumPixelEachMatrix[k]);
            }
        }
    }

    free(sumPixelEachMatrix);
    free(directions);

    // 2. Calculer des descripteurs
    this->descriptors = (double**)malloc(sizeof(double*) * numMatrix);

    for (int i = 0; i < numMatrix; i++)
    {
        descriptors[i] = (double*)malloc(sizeof(double) * MAX_DESCRIPTORS);
        memset(descriptors[i], 0, sizeof(double) * MAX_DESCRIPTORS);

        calculDescriptors(i);
    }
}

GLCM::~GLCM()
{
    if (matrices != 0)
    {
        for (int i = 0; i < numMatrix; i++)
        {
            for (int j = 0; j < matrixSize; j++)
            {
                free(matrices[i][j]);
            }
            free(matrices[i]);
        }
        free(matrices);
    }
    matrices = 0;

    if (descriptors != 0)
    {
        for (int i = 0; i < numMatrix; i++)
        {
            free(descriptors[i]);
        }
        free(descriptors);
    }
    descriptors = 0;
}

//
// Calculer les descripteurs
// @param:
//      int indexMatrix : l'indice de la matrice de co-occurrence
//
void GLCM::calculDescriptors(int indexMatrix)
{
    double** matrix = this->matrices[indexMatrix];
    double* descriptor = this->descriptors[indexMatrix];

    double maximumProbability = 0;
    double entryValue = 0;
    int sideLoopDifference = 0, sideLoopDifferenceSquared = 0;

    for (int i = 0; i < matrixSize; i++)
    {
        for(int j = 0; j < matrixSize; j++ )
        {
            entryValue = matrix[ i ][ j ];

            sideLoopDifference = i - j;
            sideLoopDifferenceSquared = sideLoopDifference * sideLoopDifference;

            descriptor[DES_MEAN_I] += i * entryValue;
            descriptor[DES_MEAN_J] += j * entryValue;

            maximumProbability = MAX(maximumProbability, entryValue);

            if( j > i )
            {
                descriptor[DES_CON] += sideLoopDifferenceSquared * entryValue;
                descriptor[DES_DIS] += abs(sideLoopDifference) * entryValue;
            }

            descriptor[DES_HOM] += entryValue / (1.0 + sideLoopDifferenceSquared);

            if( entryValue > 0 )
            {
                descriptor[DES_ENT] += entryValue * log(entryValue);
            }

            descriptor[DES_ASM] += entryValue * entryValue;
        }
    }

    descriptor[DES_CON] += descriptor[DES_CON];
    descriptor[DES_DIS] += descriptor[DES_DIS];
    descriptor[DES_ENT] = -descriptor[DES_ENT];
    descriptor[DES_MAX] = maximumProbability;
    descriptor[DES_ENG] = sqrt(descriptor[DES_ASM]);

    double correlationProductTerm = 0;
    double varianceI = 0, varianceJ = 0;

    for (int i = 0; i < matrixSize; i++)
    {
        for(int j = 0; j < matrixSize; j++ )
        {
            entryValue = matrix[ i ][ j ];

            varianceI = i - descriptor[DES_MEAN_I];
            varianceJ = j - descriptor[DES_MEAN_J];

            correlationProductTerm += varianceI * varianceJ * entryValue;

            descriptor[DES_VAR_I] += entryValue * varianceI * varianceI;
            descriptor[DES_VAR_J] += entryValue * varianceJ * varianceJ;
        }
    }

    descriptor[DES_STD_I] = sqrt(descriptor[DES_VAR_I]);
    descriptor[DES_STD_J] = sqrt(descriptor[DES_VAR_J]);
    descriptor[DES_CORR] = correlationProductTerm / (descriptor[DES_STD_I] * descriptor[DES_STD_J]);
}

//
// Prendre un descripteur
// @param:
//      int indexMatrix : l'indice de la matrice de co-occurrence
//      int descriptor : l'indice du descripteur
// @result: la valeur du descripteur
//
double GLCM::getDescriptor(int indexMatrix, int descriptor)
{
    return descriptors[indexMatrix][descriptor];
}

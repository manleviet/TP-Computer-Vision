//
//
//  texture.h
//
//  Une classe sert a stocker un vecteur des parametres d'une image de texture
//  et a calculer la distance entre deux vecteurs.
//  Les types de distance supportent :
//  - Normal
//  - Cosin
//  - Correlation
//
//  LE Viet Man
//  20/10/10
//
//

#ifndef TEXTURE_H
#define TEXTURE_H

// definir des types de calculation de distance entre deux vecteurs
#define DISTANCE_NORMAL 0
#define DISTANCE_COSINE 1
#define DISTANCE_CORR 2

#include "string.h"
#include "iostream"
#include "stdlib.h"
#include "float.h"
#include "math.h"

using namespace std;

class Texture
{
public:
    Texture(string filename, string classname, int numDescriptors, double* vecteur, int length);

    double distance(Texture* texture, int typeDistance);

    bool checkClassname(string classname);

    string getFilename();

    string getClassname();

private:
    string filename;
    string classname;
    int numDescriptors;
    int numMatrix;
    double** vecteurs;

    // Calculer la distance entre deux textures
    double distancePermutation(Texture* texture, int typeDistance);
    double distanceNaive(Texture* texture, int typeDistance);

    // Calculer la distance entre deux vecteurs
    double distanceTwoVecteurs(int typeDistance, int length, double* vecteur1, double* vecteur2);
    double distanceTwoVecteursNormal(int length, double* vecteur1, double* vecteur2);
    double distanceTwoVecteursCosine(int length, double* vecteur1, double* vecteur2);
    double distanceTwoVecteursCorr(int length, double* vecteur1, double* vecteur2);

    // servir a la permutation
    bool permute(int a[], int n);
    void swap(int &a, int &b);
    void inverse(int a[], int x, int y);
};

#endif // TEXTURE_H

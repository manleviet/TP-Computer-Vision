//
//
//  texture.cpp
//
//  Implementation de la classe texture.h
//
//  LE Viet Man
//  20/10/10
//
//

#include "texture.h"

Texture::Texture(string filename, string classname, int numDescriptors, double *vecteur, int length)
{
    this->filename = filename;
    this->classname = classname;
    this->numDescriptors = numDescriptors;
    this->numMatrix = length / numDescriptors;

    // initier les vecteurs
    vecteurs = (double**)malloc(sizeof(double*) * numMatrix);
    for (int i = 0; i < numMatrix; i++)
    {
        vecteurs[i] = (double*)malloc(sizeof(double) * numDescriptors);
        memset(vecteurs[i], 0, sizeof(double) * numDescriptors);
    }

    // mettre les valeurs aux vecteurs
    for (int i = 0; i < numMatrix; i++)
    {
        for (int j = 0; j < numDescriptors; j++)
        {
            vecteurs[i][j] = vecteur[(i * numDescriptors) + j];
        }
    }
}

//
// Calculer la distance avec une texture
// @param:
//      Texture *texture : une texture
//      int typeDistance : le type de methodes de calculer la distance
// @result: la distance
//
double Texture::distance(Texture* texture, int typeDistance)
{
    return distancePermutation(texture, typeDistance);
    //return distanceNaive(texture, typeDistance);
}

//
// Evaluer le nom de la classe de texture
// @result: true - si deux noms des classes sont pareils
//
bool Texture::checkClassname(string classname)
{
    return this->classname == classname;
}

//
// Prendre le nom de la classe
//
string Texture::getClassname()
{
    return this->classname;
}

//
// Prendre le nom du fichier de texture
//
string Texture::getFilename()
{
    return this->filename;
}

//
// Calculer la distance entre deux textures
// En utilisant la methode d'appariement avec la permutation
// @param:
//      Texture *texture : une texture
//      int typeDistance : le type de methodes de calculer la distance
// @result: la distance
//
double Texture::distancePermutation(Texture *texture, int typeDistance)
{
    double minDistance = DBL_MAX;
    double d = 0;
    int* pair;
    pair = (int*)malloc(sizeof(int) * numMatrix);

    // creer la premiere permutation
    for (int i = 0; i < numMatrix; i++)
    {
        pair[i] = i;
    }

    // calculer la distance avec la premiere permutation
    for (int i = 0; i < numMatrix; i++)
    {
        d += distanceTwoVecteurs(typeDistance, numDescriptors, vecteurs[i], texture->vecteurs[pair[i]]);
    }

    if (d < minDistance) minDistance = d;

    while (permute(pair, numMatrix)) // permuter
    {
        d = 0;
        for (int i = 0; i < numMatrix; i++)
        {
            d += distanceTwoVecteurs(typeDistance, numDescriptors, vecteurs[i], texture->vecteurs[pair[i]]);

            if (d > minDistance) break;
        }

        if (d < minDistance) minDistance = d;
    }

    delete pair;

    return minDistance;
}

//
// Calculer la distance entre deux textures
// En utilisant la methode d'appariement sans l'usage de permutation
// @param:
//      Texture *texture : une texture
//      int typeDistance : le type de methodes de calculer la distance
// @result: la distance
//
double Texture::distanceNaive(Texture *texture, int typeDistance)
{
    double distance = 0;
    double minDistance;
    bool* flag;
    double* d;
    int mem;

    flag = (bool*)malloc(sizeof(bool) * numMatrix);
    memset(flag, 0, sizeof(bool) * numMatrix);

    d = (double*)malloc(sizeof(double) * numMatrix);

    for (int i = 0; i < numMatrix; i++)
    {
        // chaque matrice de co-occurence
        // calculer la distance avec les autres matrices
        memset(d, (int)DBL_MAX, sizeof(double) * numMatrix);

        for (int j = 0; j < numMatrix; j++)
        {
            if (!flag[j])
            {
                d[j] = distanceTwoVecteurs(typeDistance, numDescriptors, vecteurs[i], texture->vecteurs[j]);
            }
        }

        // trouver la plus basse valeur
        minDistance = DBL_MAX;
        mem = numMatrix;
        for (int j = 0; j < numMatrix; j++)
        {
            if (d[j] < minDistance)
            {
                minDistance = d[j];
                mem = j;
            }
        }

        flag[mem] = true;
        distance += minDistance;
    }

    delete flag;
    delete d;

    return distance;
}

//
// Calculer la distance entre deux vecteurs de caracteristiques
// @param:
//      int typeDistance : le type de methodes de calculer la distance
//      int length : la quantite des carateristiques
//      double *vecteur1 : le vecteur 1
//      double *vecteur2 : le vecteur 2
// @result : la distance
//
double Texture::distanceTwoVecteurs(int typeDistance, int length, double *vecteur1, double *vecteur2)
{
    switch (typeDistance)
    {
        case DISTANCE_NORMAL:
            return distanceTwoVecteursNormal(length, vecteur1, vecteur2);
            break;
        case DISTANCE_COSINE:
            return distanceTwoVecteursCosine(length, vecteur1, vecteur2);
            break;
        case DISTANCE_CORR:
            return distanceTwoVecteursCorr(length, vecteur1, vecteur2);
            break;
        default:
            break;
    }
    return DBL_MAX;
}

//
// Calculer la distance Normal entre deux vecteurs de caracteristiques
// @param:
//      int length : la quantite des carateristiques
//      double *vecteur1 : le vecteur 1
//      double *vecteur2 : le vecteur 2
// @result : la distance
//
double Texture::distanceTwoVecteursNormal(int length, double *vecteur1, double *vecteur2)
{
    double distance = 0;

    for (int i = 0; i < length; i++)
    {
        distance += abs(vecteur1[i] - vecteur2[i]);
    }

    return distance;
}

//
// Calculer la distance Cosine entre deux vecteurs de caracteristiques
// @param:
//      int length : la quantite des carateristiques
//      double *vecteur1 : le vecteur 1
//      double *vecteur2 : le vecteur 2
// @result : la distance
//
double Texture::distanceTwoVecteursCosine(int length, double *vecteur1, double *vecteur2)
{
    double distance = 0;
    double sumVecteur1 = 0;
    double sumVecteur2 = 0;
    for (int i = 0; i < length; i++)
    {
        distance += vecteur1[i] * vecteur2[i];
        sumVecteur1 += vecteur1[i] * vecteur1[i];
        sumVecteur2 += vecteur2[i] * vecteur2[i];
    }

    distance = 1 - (distance / (sqrt(sumVecteur1 * sumVecteur2)));

    return distance;
}

//
// Calculer la distance Correlation entre deux vecteurs de caracteristiques
// @param:
//      int length : la quantite des carateristiques
//      double *vecteur1 : le vecteur 1
//      double *vecteur2 : le vecteur 2
// @result: la distance
//
double Texture::distanceTwoVecteursCorr(int length, double *vecteur1, double *vecteur2)
{
    double distance = 0;
    double proVecteur1 = 0;
    double proVecteur2 = 0;
    double mean1 = 0;
    double mean2 = 0;

    for (int i = 0; i < length; i++)
    {
        mean1 += vecteur1[i];
        mean2 += vecteur2[i];
    }

    mean1 = mean1 / length;
    mean2 = mean2 / length;

    double temp1 = 0;
    double temp2 = 0;
    for (int i = 0; i < length; i++)
    {
        temp1 = vecteur1[i] - mean1;
        temp2 = vecteur2[i] - mean2;

        distance += temp1 * temp2;
        proVecteur1 += temp1 * temp1;
        proVecteur2 += temp2 * temp2;
    }

    distance = 1 - (distance / sqrt (proVecteur1 * proVecteur2));

    return distance;
}

//
// Faire la permutation pour un array
// @param:
//      int a[] : un array
//      int n : la quantite d'elements de l'array
// @result: false - ne peut plus permuter
//
bool Texture::permute(int a[], int n)
{
    int i, j;
    for(i = n - 1; i > 0; i--)
    {
        if(a[i] > a[i-1]) break;
    }

    if(i == 0) return false;
    else
    {
        inverse(a, i, n - 1);

        for(j = i; j < n; j++)
            if(a[j] > a[i-1])
            {
                swap(a[j], a[i-1]);
                break;
            }
    }

    return true;
}

//
// Echanger deux valuers
//
void Texture::swap(int &a, int &b)
{
    int c = a; a = b; b = c;
}

//
// Inverser des elements d'un array
// @param:
//      int a[] : un array
//      int x : la position de debut
//      int y : la position de fin
//
void Texture::inverse(int a[], int x, int y)
{
    int i = x, j = y;
    while(i < j)
    {
        swap(a[i], a[j]);
        i++;
        j--;
    }
}

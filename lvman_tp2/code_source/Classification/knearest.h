//
//
// knearest.h
//
// Utilisant l'algorithme k plus proche voisin, cette classe peut identifier
// la classe des textures
//
// LE Viet Man
// 27/10/2010
//
//

#ifndef KNEAREST_H
#define KNEAREST_H

#include <list>
#include "basetextures.h"
#include "texture.h"

class knearest
{
public:
    knearest(int k, BaseTextures* training);

    string classify(Texture* test, int typeDistance, double &distance);

private:
    BaseTextures* training;
    int k;

    int isOnList(vector<string> list, string classname);
};

#endif // KNEAREST_H

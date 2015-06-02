//
//
// basetextures.h
//
// Une classe sert a stocker une base des textures
//
// LE Viet Man
// 21/10/10
//
//

#ifndef BASETEXTURES_H
#define BASETEXTURES_H

#include <string>
#include <fstream>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "texture.h"

using namespace std;

class BaseTextures
{
public:
    BaseTextures();

    void load(string path);

    bool checkBase(BaseTextures* base);

    int getQuantityTextures();

    Texture* getTexture(int index);

private:
    int greyLevel;
    int numDistances;
    bool distances[4];
    int numDirections;
    int numDescriptors;
    bool descriptors[14];
    vector<Texture*> textures;

    // Servir a extraire des valeurs dans le fichier glcm
    string getValue(string line);
    int getValues(bool* values, string line);
    void getTexture(string line, int numDescriptors, Texture* &texture, int length);
};

#endif // BASETEXTURES_H

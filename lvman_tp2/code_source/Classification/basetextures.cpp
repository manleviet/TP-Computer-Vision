//
//
// basetextures.cpp
//
// Implementation de la classe basetextures.h
//
// LE Viet Man
// 21/10/10
//
//

#include "basetextures.h"

BaseTextures::BaseTextures()
{
    this->numDistances = 0;
    this->numDirections = 0;
    this->numDescriptors = 0;
}

//
// Charger le fichier glcm
// @param :
//      string path : le chemin du fichier glcm
//
void BaseTextures::load(string path)
{
    ifstream infile;

    infile.open(path.c_str(), ifstream::in);

    if (!infile.good())
    {
        // il n'existe pas de cette image
        cerr << "Il n'existe pas de fichier " << path << endl;
        exit(1);
    }

    char line1[100];

    // lire la tete du fichier
    // Folder
    infile.getline(line1, 100);
    // GreyLevel
    infile.getline(line1, 100);
    greyLevel = atoi(getValue(line1).c_str());
    // Distances
    for (int i = 0; i < 4; i++)
    {
        distances[i] = false;
    }
    infile.getline(line1, 100);
    numDistances = getValues(distances, line1);
    // NumDirections
    infile.getline(line1, 100);
    numDirections = atoi(getValue(line1).c_str());
    // Descriptors
    for (int i = 0; i < 14; i++)
    {
        descriptors[i] = false;
    }
    infile.getline(line1, 100);
    numDescriptors = getValues(descriptors, line1);

    // lire le reste du fichier
    char line[2000];

    while (infile.getline(line, 2000))
    {
        // extraire des valeurs de parametre
        Texture* texture;

        getTexture(line, numDescriptors, texture, numDistances * numDirections * numDescriptors);

        textures.push_back(texture);
    }

    infile.close();
}

//
// Evaluer la compatibilite entre deux bases de textures
// @param :
//      BaseTextures *base : une base de textures
// @result : true - quand les valeurs de greyLevel, distance, numDirections
// et directions sont pareils
//
bool BaseTextures::checkBase(BaseTextures *base)
{
    if (greyLevel != base->greyLevel) return false;
    if (numDirections != base->numDirections) return false;
    for (int i = 0; i < 4; i++)
    {
        if (distances[i] != base->distances[i]) return false;
    }
    for (int i = 0; i < 14; i++)
    {
        if (descriptors[i] != base->descriptors[i]) return false;
    }

    return true;
}

//
// Prendre la quantite des textures dans la base de textures
//
int BaseTextures::getQuantityTextures()
{
    return textures.size();
}

//
// Prendre une texture dans la base de textures
// @param:
//      int index : l'indice de la texture
//
Texture* BaseTextures::getTexture(int index)
{
    return textures.at(index);
}

//
// Cette fonction sert a lire les infos dans le fichier glcm
// Concentre aux infos : Folder, GreyLevel, numDirections
// Prendre une valeur dans un string
// @param:
//      string line : un string
//
string BaseTextures::getValue(string line)
{
    size_t pos;
    pos = line.find(" ");
    if (pos == string::npos)
        return "";
    return line.substr(pos + 1);
}

//
// Cette fonction sert a lire les infos dans le fichier glcm
// Concentre aux infos : Distances, Descriptors
// Prendre des valuers dans un string
// @param:
//      bool *values : une liste des valuers
//      string line : un string
// @result : la quantite des valeurs
//
int BaseTextures::getValues(bool* values, string line)
{
    size_t posOld, posNew;
    int count = 0;
    int index = 0;
    posNew = line.find_first_of(" ");
    while (posNew != string::npos)
    {
        count++;
        posOld = posNew + 1;
        posNew = line.find_first_of(" ", posOld);
        index = atoi(line.substr(posOld, posNew - posOld).c_str());
        values[index] = true;
    }

    index = atoi(line.substr(posOld, line.length() - posOld).c_str());
    values[index] = true;

    return count;
}

//
// Cette fonction sert a lire les infos dans le fichier glcm
// Concentre aux textures
// Prendre des valeurs dans un string de parametre de texture
// @param:
//      string line : un string
//      int numDescriptors : la quantite des descripteurs
//      Texture *&texture : une texture
//      int length : la quantite des parametre dans la texture
//
void BaseTextures::getTexture(string line, int numDescriptors, Texture *&texture, int length)
{
    string filename, classname;
    double* vecteur;
    size_t posNew;
    size_t posOld;

    // prendre filename
    posNew = line.find_first_of(" ");
    filename = line.substr(0, posNew);

    // prendre classname
    posOld = posNew + 1;
    posNew = line.find_first_of(" ", posOld);
    classname = line.substr(posOld, posNew - posOld);

    // initier le vecteur
    vecteur = (double*)malloc(sizeof(double) * length);
    memset(vecteur, 0, sizeof(double) * length);

    // prendre des parametres calculees
    posOld = posNew + 1;
    posNew = line.find_first_of(" ", posOld);
    int count = -1;
    while (posNew != string::npos)
    {
        count++;
        vecteur[count] = atof(line.substr(posOld, posNew - posOld).c_str());
        posOld = posNew + 1;
        posNew = line.find_first_of(" ", posOld);
    }

    vecteur[++count] = atof(line.substr(posOld, line.length() - posOld).c_str());

    texture = new Texture(filename, classname, numDescriptors, vecteur, length);

    delete vecteur;
}

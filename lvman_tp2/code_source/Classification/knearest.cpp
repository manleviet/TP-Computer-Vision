//
//
// knearest.cpp
//
// Implementation de la classe knearest.h
//
// LE Viet Man
// 27/10/2010
//
//

#include "knearest.h"

knearest::knearest(int k, BaseTextures *training)
{
    this->k = k;
    this->training = training;
}

//
// Faire la classification pour une texture
// @param:
//      Texture *test : une texture
//      int typeDistance : le type de methodes de calculer la distance
//      double &dist : la plus petit distance entre la texture avec une classe de textures
// @result: le nom de la classe de texture
//
string knearest::classify(Texture *test, int typeDistance, double &dist)
{
    double d, dmax, dmaxNew;
    string classTexture[k];
    double distTexture[k];
    int count = 0;

    // k-nearest
    for (int i = 0; i < training->getQuantityTextures(); i++)
    {
        d = test->distance(training->getTexture(i), typeDistance);

        if (count < k)
        {
            if (count == 0) dmax = d;
            else dmax = (d > dmax) ? d : dmax;

            classTexture[count] = training->getTexture(i)->getClassname();
            distTexture[count] = d;
            count++;
        }
        else
        {
            if (d < dmax)
            {
                for (int j = 0; j < count; j++)
                {
                    if (distTexture[j] == dmax)
                    {
                        classTexture[j] = training->getTexture(i)->getClassname();
                        distTexture[j] = d;
                        break;
                    }
                }

                // mettre a jour la valeur de dmax
                dmaxNew = 0;
                for (int j = 0; j < count; j++)
                {
                    dmaxNew = (distTexture[j] > dmaxNew) ? distTexture[j] : dmaxNew;
                }
                dmax = dmaxNew;
            }
        }
    }

    // choisir la class pour la texture
    vector<string> classname;
    vector<int> quantity;
    vector<double> smallDistance;
    int temp, value, distance;

    count = 0;
    for (int i = 0; i < k; i++)
    {
        temp = isOnList(classname, classTexture[i]);
        if (temp != -1)
        {
            value = (int)quantity.at(temp);
            quantity.at(temp) = ++value;
            distance = smallDistance.at(temp);
            if (distance > distTexture[i])
            {
                smallDistance.at(temp) = distTexture[i];
            }
        }
        else
        {
            count++;
            classname.push_back(classTexture[i]);
            smallDistance.push_back(distTexture[i]);
            quantity.push_back(1);
        }
    }

    int max = 0;
    int index = -1;
    double small = DBL_MAX;
    for (int i = 0; i < count; i++)
    {
        if (quantity.at(i) > max)
        {
            max = quantity.at(i);
            index = i;
            small = smallDistance.at(i);
        }
        else if (quantity.at(i) == max)
        {
            if (smallDistance.at(i) < small)
            {
                max = quantity.at(i);
                index = i;
                small = smallDistance.at(i);
            }
        }
    }

    dist = small;
    return classname[index];
}

//
// Evaluer si une classe est dans la liste
// @param:
//      vector<string> list : une liste des classes
//      string classname : une classe
// @result: la position de la classe dans la liste
//
int knearest::isOnList(vector<string> list, string classname)
{
    for (unsigned int i = 0; i < list.size(); i++)
    {
        if (list.at(i) == classname) return i;
    }
    return -1;
}

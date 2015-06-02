//
//
// classtexture.cpp
//
// Implementation de la classe classtexture.h
//
// LE Viet Man
// 29/10/2010
//
//

#include "classtexture.h"

ClassTexture::ClassTexture(string classname)
{
    this->classname = classname;
}

void ClassTexture::setRegion(int x, int y, int size)
{
    this->x.push_back(x);
    this->y.push_back(y);
    this->size.push_back(size);
}

string ClassTexture::getClassname()
{
    return this->classname;
}

int ClassTexture::getNumRegions()
{
    return this->x.size();
}

int ClassTexture::getX1(int index)
{
    return x.at(index);
}

int ClassTexture::getY1(int index)
{
    return y.at(index);
}

int ClassTexture::getX2(int index)
{
    return x.at(index) + size.at(index);
}

int ClassTexture::getY2(int index)
{
    return y.at(index) + size.at(index);
}

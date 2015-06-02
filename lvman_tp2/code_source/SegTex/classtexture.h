//
//
// classtexture.h
//
// Une classe sert a stocker des regions de texture d'une image
//
// LE Viet Man
// 29/10/2010
//
//

#ifndef CLASSTEXTURE_H
#define CLASSTEXTURE_H

#include <vector>
#include <string>

using namespace std;

class ClassTexture
{
public:
    ClassTexture(string classname);

    void setRegion(int x, int y, int size);

    string getClassname();
    int getNumRegions();

    int getX1(int index);
    int getY1(int index);
    int getX2(int index);
    int getY2(int index);

private:
    string classname;
    vector<int> x;
    vector<int> y;
    vector<int> size;
};

#endif // CLASSTEXTURE_H

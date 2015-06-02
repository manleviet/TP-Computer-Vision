//
//
//  main.cpp
//
//  Une application sert a identifier des textures dans une image
//
//  LE Viet Man
//  27/10/2010
//
//

#include <cv.h>
#include <highgui.h>
#include <fstream>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <map>
#include <vector>
#include "basetextures.h"
#include "knearest.h"
#include "glcm.h"
#include "classtexture.h"

using namespace std;

#define MAX_GREY_LEVELS 256
#define MAX_CLASS 32
#define MAX_TAILLE 6

char *Usage = (char*)"Usage: segTex [options] <filename>\n\
Une application sert a identifier des textures dans une image\n\
Arguments:\n\
    -h : help\n\
    -k <int> : la valeur k de l\'algorithme k plus proche voisin\n\
    -t <int> : la taille de la fenetre de texture, supporter seulement 8, 16, 32, 64, 128, 256\n\
    -l <int> : le seuil de la distance\n\
    -b <filename> : une base d\'apprentissage (.glcm)\n\
    -s <path> : le chemin de repertoire qui est utilise pour enregistrer le fichier de resultat\n\
    <filename> : le fichier d\'image\n";

//
// Prototypes des fonctions
//
void showUsage(bool);
string getNameOfFolder(const string &);
void reduceGreyLevels(IplImage*, int);
Texture *getTexture(IplImage*, BaseTextures*);
void showImage(const char*, const IplImage*);
void drawTextures(IplImage*, vector<ClassTexture*>);
CvScalar getColor(int index);
string createExtOfFileSave(int, int*, double);

//
// La fonction main
//
int main(int argc, char *argv[])
{
    if (argc < 2) // manque les paramètres
    {
        showUsage(true); // afficher les informations d'aide
    }

    string fileTrain; // fichier d'apprentissage
    string fileTest; // fichier de test
    string pathSave; // le chemin d'enregistrement
    int k = 7; // k plus proche voisin
    int size[] = {0, 0, 0, 0, 0, 0};
    int countSize = 0;
    double threshold = 0.001;

    // cette boucle lit les arguments passés
    int c;
    int temp;
    while ((c = getopt(argc, argv, "hk:t:l:b:s:")) != -1)
    {
        switch (c)
        {
            case 'h':
            {
                showUsage(false);
                break;
            }
            case 'k':
            {
                extern char *optarg;
                k = atoi(optarg);
                break;
            }
            case 't':
            {
                extern char *optarg;
                temp = atoi(optarg);
                if ((temp == 8) ||
                    (temp == 16) ||
                    (temp == 32) ||
                    (temp == 64) ||
                    (temp == 128) ||
                    (temp == 256))
                {
                    size[countSize] = atoi(optarg);
                    countSize++;
                }
                break;
            }
            case 'l':
            {
                extern char *optarg;
                threshold = atof(optarg);
                break;
            }
            case 'b':
            {
                extern char *optarg;
                fileTrain = optarg;
                break;
            }
            case 's':
            {
                extern char *optarg;
                pathSave = optarg;
                pathSave = pathSave + "/";
                break;
            }
            default:
                break;
        }
    }

    // lire le nom de repertoire des images
    extern int optind;
    if (optind>=argc)
    {
        showUsage(true);
    }
    fileTest = argv[optind];

    if (countSize == 0)
    {
        size[countSize] = 256;
        countSize++;
    }

    // charger la base d'apprentissage
    cout << "Charger la base d'apprentissage..." << endl;
    BaseTextures* train = new BaseTextures();
    train->load(fileTrain);

    // creer le nom du fichier de resultat
    string ext = createExtOfFileSave(countSize, size, threshold);
    string fileSave = pathSave
                      + getNameOfFolder(fileTest)
                      + ext
                      + ".res";
    string fileImgSave = pathSave
                         + getNameOfFolder(fileTest)
                         + ext
                         + "_res.png";

    knearest* tester = new knearest(k, train);

    // ouvrir le fichier de resultat
    std::ofstream outFile(fileSave.c_str());

    cout << "Fichier de test: " << fileTest << endl;
    outFile << "Fichier de test: " << fileTest << endl;
    // charger l'image
    IplImage *img = cvLoadImage(fileTest.c_str());
    if (!img)
    {
        // il n'existe pas de cette image
        cerr << "Ne pas pouvoir charger l'image " << fileTest << endl;
        showUsage(false);
    }
    IplImage *res = cvCloneImage(img); // l'image de resultat avec des regions de textures
    cvConvertImage(img, img, CV_BGR2GRAY); // convertir l'image en gris

    // compter le temps
    clock_t begin, end;
    double t_end = 0.0;
    assert((begin = clock()) != -1);

    // reduire le niveau de gris
    reduceGreyLevels(img, train->getGreyLevel());

    Texture *test;
    string classname;
    double distance;

    // les regions avec la taille 8x8, 16x16, 32x32, 64x64, 128x128, 256x256
    map<string,int> listClass;
    vector<ClassTexture*> listRegion;
    for (int s = 0; s < countSize; s++)
    {
        for (int i = 0; i < img->height; i = i + size[s])
        {
            for (int j = 0; j < img->width; j = j + size[s])
            {
                cout << "Region " << i << "," << j << " - " << i + size[s] << "," << j + size[s];
                outFile << "Region " << i << "," << j << " - " << i + size[s] << "," << j + size[s];
                // mettre ROI
                cvSetImageROI(img, cvRect(i, j, size[s], size[s]));

                test = getTexture(img, train);

                classname = tester->classify(test, 1, distance);

                if (distance < threshold)
                {
                    cout << classname;
                    outFile << classname;
                    if (listClass.find(classname) == listClass.end())
                    {
                        listClass.insert(make_pair(classname, 1));
                        ClassTexture *classTexture = new ClassTexture(classname);
                        classTexture->setRegion(i, j, size[s]);
                        listRegion.push_back(classTexture);
                    }
                    else
                    {
                        listClass[classname]++;

                        for (unsigned int k = 0; k < listRegion.size(); k++)
                        {
                            ClassTexture *classTexture = listRegion.at(k);
                            if (classname == classTexture->getClassname())
                            {
                                classTexture->setRegion(i, j, size[s]);
                            }
                        }
                    }
                }
                cout << endl;
                outFile << endl;

                // reset ROI
                cvResetImageROI(img);
            }
        }
    }

    // enregistrer les resultats sur le fichier
    cout << "Total de texture: " << listRegion.size() << endl;
    outFile << "Total de texture: " << listRegion.size() << endl;
    for (unsigned int i = 0; i < listRegion.size(); i++)
    {
        ClassTexture *classTexture = listRegion.at(i);
        cout << "Texture " << classTexture->getClassname() << " - "
                << classTexture->getNumRegions() << " regions" << endl;
        outFile << "Texture " << classTexture->getClassname() << " - "
                << classTexture->getNumRegions() << " regions" << endl;

        for (int j = 0; j < classTexture->getNumRegions(); j++)
        {
            cout << classTexture->getX1(j) << "," << classTexture->getY1(j) <<
                    " - " << classTexture->getX2(j) << "," << classTexture->getY2(j) << endl;
            outFile << classTexture->getX1(j) << "," << classTexture->getY1(j) <<
                    " - " << classTexture->getX2(j) << "," << classTexture->getY2(j) << endl;
        }
    }

    end = clock();
    t_end = (double) (end - begin) / CLOCKS_PER_SEC;

    cout << "Temps de calcul: " << t_end << " secondes" << endl;
    outFile << "Temps de calcul: " << t_end << " secondes" << endl;

    outFile.close();

    // dessiner les regions
    drawTextures(res, listRegion);
    // enregistrer l'image de resultats
    cvSaveImage(fileImgSave.c_str(), res);

    cvReleaseImage(&res);
    cvReleaseImage(&img);

    return 0;
}

//
// La fonction sert à afficher les informations d'aide de ce programme
// @param :
//	bool erreur : true, cad il y a une erreur
//                          la fonction exécutera la commande exit(1)
//                    false, cad il n'y a pas d'erreur
//                           la fonction exécutera la commande exit(0)
//
void showUsage(bool erreur)
{
    cerr << Usage << endl;
    if (erreur)
    {
        exit(1);
    }
    else
    {
        exit(0);
    }
}

//
// La fonction sert à prendre le nom d'un repertoire
// @param :
//	const string &path : le chemin entre
//
string getNameOfFolder(const string &path) // ex : /home/manleviet
{
    string filename;
    size_t found = path.find_last_of('/');
    if (found == string::npos)
    {
        return "";
    }
    filename = path.substr(found + 1);

    found = filename.find_first_of(".");
    if (found == string::npos)
    {
        return "";
    }
    filename = filename.substr(0, found);

    return filename;
}

//
// Reduire le nombre de niveau de gris
// @param :
//      IplImage *img : l'image originale
//      int numLevels : nombre de niveau de gris desire
//
void reduceGreyLevels(IplImage *img, int numLevels)
{
    int divise = MAX_GREY_LEVELS / numLevels;

    CvScalar s;
    for (int i = 0; i < img->height; i++)
    {
        for (int j = 0; j < img->width; j++)
        {
            s = cvGet2D(img, i, j);

            s.val[0] = (int)s.val[0] / divise;

            cvSet2D(img, i, j, s);
        }
    }
}

//
// Calculer un vecteur des caracteristiques d'une image
// @param:
//      IplImage *img : une image
//      BaseTextures *train : la base d'apprentissage
// @result: une texture
//
Texture *getTexture(IplImage *img, BaseTextures *train)
{
    Texture *texture;
    GLCM *glcm = 0;
    double *vecteur;
    int numDistances = train->getNumDistances();
    int numDirections = train->getNumDirections();
    int numDescriptors = train->getNumDescriptors();
    int length = numDistances * numDirections * numDescriptors;

    // initier le vecteur
    vecteur = (double*)malloc(sizeof(double) * length);
    memset(vecteur, 0, sizeof(double) * length);

    // pour chaque distance
    int count = -1;
    for (int i = 0; i < MAX_DISTANCES; i++)
    {
        if (train->isDistances(i))
        {
            // creer la matrice co-occurrence
            glcm = new GLCM(train->getGreyLevel(), img, i + 1, numDirections);

            // prendre des parametres
            for (int m = 0; m < numDirections; m++)
            {
                for (int n = 0; n < MAX_DESCRIPTORS; n++)
                {
                    if (train->isDescriptors(n))
                    {
                        count++;
                        vecteur[count] = glcm->getDescriptor(m, n);
                    }
                }
            }
            delete glcm;
        }
    }

    // creer une nouvel texture
    texture = new Texture("test", "test", train->getNumDescriptors(), vecteur, length);

    return texture;
}

//
// Afficher les resultats sur les fenetres
// @param :
//      const char *title : le nom de la fenetre
//      const IplImage *img : une image
//
void showImage(const char *title, const IplImage *img)
{
    cvNamedWindow(title, 0);
    cvShowImage(title, img);
}

//
// Dessiner les regions de texture sur l'image
// @param:
//      IplImage *img : une image
//      vector<ClassTexture> list : liste des regions de texture
//
void drawTextures(IplImage *img, vector<ClassTexture*> list)
{
    //CvFont font;
    //double hScale=.3;
    //double vScale=.3;
    //int    lineWidth=1;
    //cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);

    //int x, y;
    ClassTexture *classTexture;
    for (unsigned int i = 0; i < list.size(); i++)
    {
        classTexture = list.at(i);
        for (int j = 0; j < classTexture->getNumRegions(); j++)
        {
            cvRectangle(img, cvPoint(classTexture->getX1(j), classTexture->getY1(j)),
                        cvPoint(classTexture->getX2(j), classTexture->getY2(j)),
                        getColor(i));

            //x = classTexture->getX1(j) + ((classTexture->getX2(j) - classTexture->getX1(j)) / 2);
            //y = classTexture->getY1(j) + ((classTexture->getY2(j) - classTexture->getY1(j)) / 2);
            //cvPutText (img, classTexture->getClassname().c_str(),
            //           cvPoint(x, y), &font, cvScalar(255,0,0));
        }
    }
}

//
// Prendre une couleur
//
CvScalar getColor(int index)
{
    CvScalar s;
    switch (index)
    {
    case 0: // Aquamarine
        s.val[0] = 0x70;
        s.val[1] = 0xDB;
        s.val[2] = 0x93;
        break;
    case 1: // blue 0x00, 0x00, 0xFF
        s.val[0] = 0x00;
        s.val[1] = 0x00;
        s.val[2] = 0xFF;
        break;
    case 2: // blue violet 0x9F, 0x5F, 0x9F
        s.val[0] = 0x9F;
        s.val[1] = 0x5F;
        s.val[2] = 0x9F;
        break;
    case 3: // brown 0xA6, 0x2A, 0x2A
        s.val[0] = 0xA6;
        s.val[1] = 0x2A;
        s.val[2] = 0x2A;
        break;
    case 4: // bronze 0x8C, 0x78, 0x53
        s.val[0] = 0x8C;
        s.val[1] = 0x78;
        s.val[2] = 0x53;
        break;
    case 5: // copper 0xB8, 0x73, 0x33
        s.val[0] = 0xB8;
        s.val[1] = 0x73;
        s.val[2] = 0x33;
        break;
    case 6: // cyan 0x00, 0xFF, 0xFF
        s.val[0] = 0x00;
        s.val[1] = 0xFF;
        s.val[2] = 0xFF;
        break;
    case 7: // dark green 0x2F, 0x4F, 0x2F
        s.val[0] = 0x2F;
        s.val[1] = 0x4F;
        s.val[2] = 0x2F;
        break;
    case 8: // dark orchid 0x99, 0x32, 0xCD
        s.val[0] = 0x99;
        s.val[1] = 0x32;
        s.val[2] = 0xCD;
        break;
    case 9: // dark purple 0x87, 0x1F, 0x78
        s.val[0] = 0x87;
        s.val[1] = 0x1F;
        s.val[2] = 0x78;
        break;
    case 10: // gold 0xCD, 0x7F, 0x32
        s.val[0] = 0xCD;
        s.val[1] = 0x7F;
        s.val[2] = 0x32;
        break;
    case 11: // green 0x00, 0xFF, 0x00
        s.val[0] = 0x00;
        s.val[1] = 0xFF;
        s.val[2] = 0x00;
        break;
    case 12: // green yellow 0x93, 0xDB, 0x70
        s.val[0] = 0x93;
        s.val[1] = 0xDB;
        s.val[2] = 0x70;
        break;
    case 13: // indian red 0x4E, 0x2F, 0x2F
        s.val[0] = 0x4E;
        s.val[1] = 0x2F;
        s.val[2] = 0x2F;
        break;
    case 14: // light blue 0xC0, 0xD9, 0xD9
        s.val[0] = 0xC0;
        s.val[1] = 0xD9;
        s.val[2] = 0xD9;
        break;
    case 15: // magenta 0xFF, 0x00, 0xFF
        s.val[0] = 0xFF;
        s.val[1] = 0x00;
        s.val[2] = 0xFF;
        break;
    case 16: // maroon 0x8E, 0x23, 0x6B
        s.val[0] = 0x8E;
        s.val[1] = 0x23;
        s.val[2] = 0x6B;
        break;
    case 17: // navy blue 0x23, 0x23, 0x8E
        s.val[0] = 0x23;
        s.val[1] = 0x23;
        s.val[2] = 0x8E;
        break;
    case 18: // neon blue 0x4D, 0x4D, 0xFF
        s.val[0] = 0x40;
        s.val[1] = 0x4D;
        s.val[2] = 0xFF;
        break;
    case 19: // neon pink 0xFF, 0x6E, 0xC7
        s.val[0] = 0xFF;
        s.val[1] = 0x6E;
        s.val[2] = 0xC7;
        break;
    case 20: // orange 0xFF, 0x7F, 0x00
        s.val[0] = 0xFF;
        s.val[1] = 0x7F;
        s.val[2] = 0x00;
        break;
    case 21: // orange red 0xFF, 0x24, 0x00
        s.val[0] = 0xFF;
        s.val[1] = 0x24;
        s.val[2] = 0x00;
        break;
    case 22: // orchid 0xDB, 0x70, 0xDB
        s.val[0] = 0xDB;
        s.val[1] = 0x70;
        s.val[2] = 0xDB;
        break;
    case 23: // pink 0xBC, 0x8F, 0x8F
        s.val[0] = 0xBC;
        s.val[1] = 0x8F;
        s.val[2] = 0x8F;
        break;
    case 24: // plum 0xEA, 0xAD, 0xEA
        s.val[0] = 0xEA;
        s.val[1] = 0xAD;
        s.val[2] = 0xEA;
        break;
    case 25: // scarlet 0x8C, 0x17, 0x17
        s.val[0] = 0x8C;
        s.val[1] = 0x17;
        s.val[2] = 0x17;
        break;
    case 26: // yellow 0xFF, 0xFF, 0x00
        s.val[0] = 0xFF;
        s.val[1] = 0xFF;
        s.val[2] = 0x00;
        break;
    case 27: // yellow green 0x99, 0xCC, 0x32
        s.val[0] = 0x99;
        s.val[1] = 0xCC;
        s.val[2] = 0x32;
        break;
    case 28: // violet 0x4F, 0x2F, 0x4F
        s.val[0] = 0x4F;
        s.val[1] = 0x2F;
        s.val[2] = 0x4F;
        break;
    case 29: // violet red 0xCC, 0x32, 0x99
        s.val[0] = 0xCC;
        s.val[1] = 0x32;
        s.val[2] = 0x99;
        break;
    case 30: // Turquoise 0xAD, 0xEA, 0xEA
        s.val[0] = 0xAD;
        s.val[1] = 0xEA;
        s.val[2] = 0xEA;
        break;
    case 31: // lime green 0x32, 0xCD, 0x32
        s.val[0] = 0x32;
        s.val[1] = 0xCD;
        s.val[2] = 0x32;
        break;
    case 32: // medium wood 0xA6, 0x80, 0x64
        s.val[0] = 0xA6;
        s.val[1] = 0x80;
        s.val[2] = 0x64;
        break;
    default: // red 0xFF, 0x00, 0x00
        s.val[0] = 0xFF;
        s.val[1] = 0x00;
        s.val[2] = 0x00;
        break;
    }

    return s;
}

//
// A partir du nom de repertoire, creer un nom du fichier de resultats
// sous la forme <nom de repertoire>_l<niveau de gris>_d<des distances>_di<nombre de directions>.glcm
// @param :
//      string path : un chemin
//      int greyLevel : le niveau de gris
//      int numDirections : le nombre de directions
// @result : nom du fichier de resultats
//
string createExtOfFileSave(int countSize, int *size, double threshold)
{
    string fileSave;

    // la taille
    char temp[3];
    fileSave = "-t";
    for (int i = 0; i < countSize; i++)
    {
        sprintf(temp, "%d", size[i]);
        fileSave += temp;
    }
    fileSave += "-l";

    // le seuil
    char tempThreshold[10];
    sprintf(tempThreshold, "%f", threshold);
    fileSave += tempThreshold;

    return fileSave;
}

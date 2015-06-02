//
//
//  main.cpp
//
//  Une application sert a calculer les matrices de co-occurrences
//  et a extraire des descripteurs, tels que :
//   - Contrast (CON)
//   - Dissimilarity (DIS)
//   - Homogeneity (HOM)
//   - ASM
//   - Energy
//   - Maximum probabilite
//   - Entropy (ENT)
//   - GLCM Mean
//   - GLCM Variance
//   - Standard Deviation
//   - GLCM Correlation
//  Entree : un repertoire des images de texture
//  Sortie : un fichier texte stocke des vecteurs de caracteristique
//
//  Support :
//   - Matrices de co-occurrence de taille 8x8, 16x16, 32x32, 64x64, 128x128, 256x256
//   - Distances : 1, 2, 3, 4
//   - Nomber de directions : 4, 8, 12, 16
//
//  LE Viet Man
//  16/10/2010
//
//

#include <cv.h>
#include <highgui.h>
#include <fstream>
#include <string.h>
#include <iostream>
#include <sys/dir.h>
#include <math.h>
#include <stdlib.h>
#include "glcm.h"

using namespace std;

#define MAX_GREY_LEVELS 256

char *Usage = (char*)"Usage: glcm [options] <path>\n\
Une application sert a calculer les matrices de co-occurrences\n\
et a extraire des descripteurs de chaque image.\n\
Arguments:\n\
    -h : help\n\
    -z : si l\'entree est la base 2-RotInv_15_7_64\n\
    -a : distance = 1\n\
    -b : distance = 2\n\
    -c : distance = 3\n\
    -d : distance = 4\n\
    -w <int> : number of directions, just 4, 8, 12, 16\n\
    -g <int> : grey levels, just 8x8, 16x16, 32x32, 64x64, 128x128, 256x256\n\
    -p <int> : type de descripteurs :\n\
                0 - Contrast (CON)\n\
                1 - Dissimilarity (DIS)\n\
                2 - Homogeneity (HOM)\n\
                3 - ASM\n\
                4 - Energy\n\
                5 - Maximum probabilite\n\
                6 - Entropy (ENT)\n\
                7 - GLCM Mean\n\
                9 - GLCM Variance\n\
                11 - Standard Deviation\n\
                13 - GLCM Correlation\n\
                Les formulaires sont trouvees sur http://www.fp.ucalgary.ca/mhallbey/equations.htm\n\
    -s <path> : le chemin de repertoire qui est utilise pour enregistrer le fichier de resultat\n\
    <path> : le chemin de repertoire des images de texture\n";

//
// Prototypes des fonctions
//
void showUsage(bool);
string getNameOfFolder(const string&);
void reduceGreyLevels(IplImage*, int);
string createNameOfFileSave(string, int, bool[], int);
void writeHeaderOfFileSave(std::ofstream&, string, int, bool[], int, bool[]);
string getClass(bool, string);

//
// La fonction main
//
int main(int argc, char *argv[])
{
    if (argc < 2) // manque les paramètres
    {
        showUsage(true); // afficher les informations d'aide
    }

    string pathTexture = ""; // le chemin de dossier des images de texture
    string pathSave = ""; // le chemin d'enregistrement
    string fileImage = ""; // a pour but de stocker le nom de chaque image
    int greyLevel = 8; // le niveau de gris
    bool distance[] = {true, false, false, false}; // les distances, par defaut, la distance = 1
    int numDirections = 4; // nombre des directions
    bool descriptors[] = {false, false, false, false, false, false, false,
                           false, false, false, false, false, false, false}; // des parametres
    bool flagBase = false; // pour la base 2-RotInv_15_7_64

    // cette boucle lit les arguments passés
    int c;
    int temp;
    while ((c = getopt(argc, argv, "hzabcdw:g:p:s:")) != -1)
    {
        switch (c)
        {
            case 'h':
            {
                showUsage(false);
                break;
            }
            case 'z':
            {
                flagBase = true;
                break;
            }
            case 'a':
            {
                distance[0] = true;
                break;
            }
            case 'b':
            {
                distance[1] = true;
                break;
            }
            case 'c':
            {
                distance[2] = true;
                break;
            }
            case 'd':
            {
                distance[3] = true;
                break;
            }
            case 'w':
            {
                extern char *optarg;
                numDirections = atoi(optarg);
                if ((numDirections != 4) &&
                    (numDirections != 8) &&
                    (numDirections != 12) &&
                    (numDirections != 16))
                {
                    cerr << "Des directions doivent etre seulement 4, 8, 12, 16." << endl;
                    return 1;
                }
                break;
            }
            case 'g':
            {
                extern char *optarg;
                greyLevel = atoi(optarg);

                // tester le niveau de gris
                if ((greyLevel != 8)
                    && (greyLevel != 16)
                    && (greyLevel != 32)
                    && (greyLevel != 64)
                    && (greyLevel != 128)
                    && (greyLevel != 256))
                {
                    cerr << "Le niveau de gris doit etre seulement 8, 16, 32, 64, 128, 256." << endl;
                    return 1;
                }
                break;
            }
            case 'p':
            {
                extern char *optarg;
                temp = atoi(optarg);
                if ((temp >= 0) && (temp < 14))
                {
                    descriptors[temp] = true;
                    if ((temp == 7) || (temp == 9) || (temp == 11))
                    {
                        descriptors[temp + 1] = true;
                    }
                }
                break;
            }
            case 's':
            {
                extern char *optarg;
                pathSave = optarg;
                pathSave += "/";
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
    pathTexture = argv[optind];

    bool checkDes = true;
    for (int i = 0; i < MAX_DESCRIPTORS; i++)
    {
        if (descriptors[i]) checkDes = false;
    }
    if (checkDes) descriptors[0] = true; // si aucun descripteur choisit, contrast sera choisit

    // creer le nom du fichier de resultat
    // le nom est comme <nom de repertoire>_l<niveau de gris>_d<des distances>_di<nombre de directions>.glcm
    string fileSave = createNameOfFileSave(pathTexture, // le repertoire des images
                                           greyLevel, // le niveau de gris
                                           distance, // des distances
                                           numDirections); // la nombre des directions

    if (pathSave != "")
    {
        fileSave = pathSave + fileSave;
    }

    // ouvrir le fichier de resultat
    std::ofstream outFile(fileSave.c_str());

    cout << "GLCM" << endl << endl;

    // ecrire la tete du fichier
    writeHeaderOfFileSave(outFile,
                          pathTexture,
                          greyLevel,
                          distance,
                          numDirections,
                          descriptors);

    // traverser le dossier des images
    int count;
    struct direct **files;
    int com1, com2;
    IplImage *img = 0;
    GLCM *glcm = 0;

    count = scandir(pathTexture.c_str(),
                    &files,
                    NULL,
                    alphasort);
    pathTexture += "/";

    // compter le temps
    clock_t begin, end;
    double t_end = 0.0;
    assert((begin = clock()) != -1);

    // pour chaque image
    for (int i = 0; i < count; i++)
    {
        com1 = strcmp(files[i]->d_name, (char*)".");
        com2 = strcmp(files[i]->d_name, (char*)"..");
        if ((com1 == 0) || (com2 == 0))
        {
            continue;
        }

        // lire chaque image
        fileImage = pathTexture + files[i]->d_name;

        // charger l'image en niveau de gris
        img = cvLoadImage(fileImage.c_str(), 0);
        if (!img)
        {
            // il n'existe pas de cette image
            cerr << "Ne pas pouvoir charger l'image " << files[i]->d_name << endl;
            showUsage(false);
        }

        cout << "Calculer la matrix de co-occurrence de l'image ";
        outFile << files[i]->d_name;
        cout << files[i]->d_name;

        // classe de l'image
        outFile << " " << getClass(flagBase, files[i]->d_name);

        // reduire le niveau de gris
        cout << ".";
        reduceGreyLevels(img, greyLevel);

        // pour chaque distance
        for (int k = 0; k < MAX_DISTANCES; k++)
        {
            // si l'utilisateur la choisit
            if (distance[k])
            {
                // creer la matrice co-occurrence
                glcm = new GLCM(greyLevel, img, k + 1, numDirections);

                // prendre des parametres
                // enregistrer sur le fichier de resultat
                for (int m = 0; m < numDirections; m++)
                {
                    for (int i = 0; i < MAX_DESCRIPTORS; i++)
                    {
                        if (descriptors[i])
                        {
                            outFile << " " << glcm->getDescriptor(m, i);
                        }
                    }
                }
                cout << ".";
                delete glcm;
            }
        }
        cout << endl;
        outFile << endl;

        cvReleaseImage(&img);
        free(files[i]);
    }
    free(files);

    outFile.close();

    end = clock();
    t_end = (double) (end - begin) / CLOCKS_PER_SEC;

    cout << "Temp de calcul: " << t_end << " seconds" << endl;

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
    size_t found = path.find_last_of('/');
    if (found >= path.length())
    {
        return "";
    }
    return path.substr(found + 1);
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
// A partir du nom de repertoire, creer un nom du fichier de resultats
// sous la forme <nom de repertoire>_l<niveau de gris>_d<des distances>_di<nombre de directions>.glcm
// @param :
//      string path : un chemin
//      int greyLevel : le niveau de gris
//      int numDirections : le nombre de directions
// @result : nom du fichier de resultats
//
string createNameOfFileSave(string path, int greyLevel, bool distance[], int numDirections)
{
    string fileSave;
    fileSave = getNameOfFolder(path);

    if (fileSave != "")
    {
        // le niveau de gris
        char tempGreyLevel[4];
        sprintf(tempGreyLevel, "_l%d_d", greyLevel);
        fileSave += tempGreyLevel;

        // des distances
        char tempDistances[2];
        for (int i = 0; i < MAX_DISTANCES; i++)
        {
            if (distance[i])
            {
                sprintf(tempDistances, "%d", i);
                fileSave += tempDistances;
            }
        }

        // nombre de directions
        char tempNumDirections[10];
        sprintf(tempNumDirections, "_di%d.glcm", numDirections);
        fileSave += tempNumDirections;
    }

    return fileSave;
}

//
// Ecrire la tete du fichier
//
void writeHeaderOfFileSave(std::ofstream &out,
                           string path,
                           int greyLevel,
                           bool distance[],
                           int numDirections,
                           bool descriptors[])
{
    cout << "Folder: " << path << endl;
    out << "Folder: " << path << endl;
    cout << "GreyLevel: " << greyLevel << endl;
    out << "GreyLevel: " << greyLevel << endl;
    cout << "Distance:";
    out << "Distance:";
    for (int i = 0; i < MAX_DISTANCES; i++)
    {
        if (distance[i])
        {
            cout << " " << i;
            out << " " << i;
        }
    }
    cout << endl;
    out << endl;
    cout << "NumDirections: " << numDirections << endl;
    out << "NumDirections: " << numDirections << endl;
    cout << "Descriptors:";
    out << "Descriptors:";
    for (int i = 0; i < MAX_DESCRIPTORS; i++)
    {
        if (descriptors[i])
        {
            cout << " " << i;
            out << " " << i;
        }
    }
    cout << endl;
    out << endl;
}

//
// Prendre le nom de la classe de texture
// @param :
//      bool flagBase : true - la base 2-RotInv_15_7_64
//      string filename : le nom du fichier d'image
// @result : le nom de texture
//
string getClass(bool flagBase, string filename)
{
    size_t pos = filename.find_first_of("_");
    if (pos == string::npos)
        return "";

    if (flagBase)
        pos = filename.find_first_of("_", pos + 1);

    return filename.substr(0, pos);
}

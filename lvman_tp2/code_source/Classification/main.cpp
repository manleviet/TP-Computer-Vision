//
//
//  main.cpp
//
//  Une application sert a classifier des textures
//  et a evaluer l'algorithme d'apprentissage
//  en utilisant l'algorithme k plus proche voisin
//  et des distances :
//  - NORMAL
//  - COSIN
//  - CORRELATION
//
//  LE Viet Man
//  27/10/2010
//
//

#include <ctime>
#include <assert.h>
#include <fstream>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include "basetextures.h"
#include "knearest.h"

using namespace std;

char *Usage = (char*)"Usage: classify [options] <filename>\n\
Une application sert a classifier des textures et a evaluer\n\
l\'algorithme d\'apprentissage en utilisant l\'algorithme k plus\n\
proche voisin et des distances :\n\
    - NORMAL\n\
    - COSIN\n\
    - CORRELATION\n\
Arguments:\n\
    -h : help\n\
    -k <int> : la valeur k de l\'algorithme k plus proche voisin\n\
    -d <int> : type de distance : \n\
                0 - NORMAL\n\
                1 - COSIN\n\
                2 - CORRELATION\n\
    -s <path> : le chemin de repertoire qui est utilise pour enregistrer le fichier de resultat\n\
    -t <filename> : le fichier d\'apprentissage (.glcm)\n\
    <filename> : le fichier de test (.glcm)\n";

//
// Prototypes des fonctions
//
void showUsage(bool);
string getNameOfFolder(const string &);
string createNameOfFileSave(string, string);

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
    int typeDistance = 0; // type de distance

    // cette boucle lit les arguments passés
    int c;
    //int temp;
    while ((c = getopt(argc, argv, "hk:d:s:t:")) != -1)
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
            case 'd':
            {
                extern char *optarg;
                typeDistance = atoi(optarg);
                if ((typeDistance != 0) &&
                    (typeDistance != 1) &&
                    (typeDistance != 2))
                {
                    cerr << "Le type de distance doit etre seulement 0, 1 ou 2." << endl;
                    return 1;
                }
                break;
            }
            case 's':
            {
                extern char *optarg;
                pathSave = optarg;
                pathSave = pathSave + "/";
                break;
            }
            case 't':
            {
                extern char *optarg;
                fileTrain = optarg;
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

    // charger la base d'apprentissage
    cout << "Charger la base d'apprentissage..." << endl;
    BaseTextures* train = new BaseTextures();
    train->load(fileTrain);

    // charger la base de test
    cout << "Charger la base de test..." << endl;
    BaseTextures* test = new BaseTextures();
    test->load(fileTest);

    // evaluer la compatibilite entre deux bases de textures
    if (!train->checkBase(test))
    {
        cerr << "Les echantillons de test n'est pas convenable avec celles d'apprentissage." << endl;
        return 1;
    }

    // creer le nom du fichier de resultat
    string fileSave = createNameOfFileSave(pathSave, fileTest);

    // ouvrir le fichier de resultat
    std::ofstream outFile(fileSave.c_str());

    cout << "Nombre des echantilons de test: " << test->getQuantityTextures() << endl;
    outFile << "Nombre des echantilons de test: " << test->getQuantityTextures() << endl;

    knearest* tester = new knearest(k, train);

    // compter le temps
    clock_t begin, end;
    double t_end = 0.0;
    assert((begin = clock()) != -1);

    // traverser les echantillons de test
    Texture* testTexture;
    string classname;
    double distance;
    int countTrue = 0;
    for (int i = 0; i < test->getQuantityTextures(); i++)
    {
        testTexture = test->getTexture(i);

        cout << testTexture->getFilename() << " " << testTexture->getClassname() << " ";
        outFile << testTexture->getFilename() << " " << testTexture->getClassname() << " ";

        // trouver la classe classifiee de texture de test
        classname = tester->classify(testTexture, typeDistance, distance);

        cout << classname << " ";
        outFile << classname << " ";

        cout << distance;
        outFile << distance;

        // comparer la classe classifiee avec la vrai classe de texture
        if (testTexture->checkClassname(classname))
        {
            countTrue++;
            cout << " " << countTrue;
            outFile << " " << countTrue;
        }

        cout << endl;
        outFile << endl;
    }

    end = clock();
    t_end = (double) (end - begin) / CLOCKS_PER_SEC;

    // enregistrer la partie d'evaluation
    cout << "EVALUATION: ";
    outFile << endl << "EVALUATION:" << endl;
    double temp = countTrue * 100.0 / test->getQuantityTextures();
    cout << temp << "%" << endl;
    outFile << temp << "%" << endl;

    cout << "Temps de calcul: " << t_end << " seconds";
    outFile << "Temps de calcul: " << t_end << " secondes";

    outFile.close();

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
// A partir du nom de repertoire, creer un nom du fichier de resultats
// sous la forme <nom de fichier de test>.res
// @param :
//      string path : un chemin
//      string fileTest : le fichier de test
// @result : nom du fichier de resultats
//
string createNameOfFileSave(string path, string fileTest)
{
    string fileSave;

    fileSave = path + getNameOfFolder(fileTest) + ".res";

    return fileSave;
}

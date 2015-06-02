//
//
//  main.cpp
//
//  Une application sert a detecter le mouvement
//  Entree : un video
//  Sortie :
//
//  LE Viet Man
//  25/11/2010
//
//

#include <cv.h>
#include <highgui.h>
#include <fstream>
//#include <string.h>
#include <iostream>
//#include <sys/dir.h>
//#include <math.h>
//#include <stdlib.h>

using namespace std;

char *Usage = (char*)"Usage: detect [options] <filename>\n\
Une application sert a detecter le mouvement.\n\
Arguments:\n\
    -h : help\n\
    -p <int> : proportion de la frame pour construire l\'arriere-plan (entre 10 et 100)\n\
    <filename> : un video\n";

//
// Prototypes des fonctions
//
void showUsage(bool);
//string getNameOfFolder(const string&);
//void reduceGreyLevels(IplImage*, int);
//string createNameOfFileSave(string, int, bool[], int);
//void writeHeaderOfFileSave(std::ofstream&, string, int, bool[], int, bool[]);
//string getClass(bool, string);
IplImage *getBackground(CvCapture*, int, int, int);
//double countFrames(string);
double countAVIFrames(CvCapture*);
string createNameFileSave(string, string);
IplImage *removeNoise(IplImage*);
vector<CvRect> findBoundingBox(IplImage*);
void drawBoundingBox(IplImage*, vector<CvRect>);
double*** initBackground(int, int, int);
void deleteBackground(double***, int, int);

//
// La fonction main
//
int main(int argc, char *argv[])
{
    if (argc < 2) // manque les paramètres
    {
        showUsage(true); // afficher les informations d'aide
    }

    string fileVideo = ""; // un video
    int proportion = 20; // 20% des frames observees

    // LIRE LES ARGUMENTS PASSES
    // cette boucle lit les arguments passés
    int c;
    while ((c = getopt(argc, argv, "hp:")) != -1)
    {
        switch (c)
        {
            case 'h':
            {
                showUsage(false);
                break;
            }
            case 'p':
            {
                extern char *optarg;
                proportion = atoi(optarg);
                if ((proportion < 10) || (proportion > 100))
                {
                    cerr << "La valeur de proportion doit etre entre 10 et 100." << endl;
                    showUsage(true);
                }
            }
            default:
                break;
        }
    }

    // lire le nom du video
    extern int optind;
    if (optind>=argc)
    {
        showUsage(true);
    }
    fileVideo = argv[optind];

    // EVALUER LE VIDEO
    CvCapture *capture = cvCreateFileCapture(fileVideo.c_str());
    if (!capture)
    {
        // erreur
        cerr << "Il n'exist pas le video" << fileVideo << endl;
        return 1;
    }

    if (!cvGrabFrame( capture ))
    {
        // erreur
        cerr << "Ne pouvoir pas lire le video " << fileVideo << endl;
        return 20;
    }

    // LIRE LES INFORMATIONS DU VIDEO
    int height = (int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT ); // la hauteur
    int width = (int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH ); // la largeur
    double fps = cvGetCaptureProperty( capture, CV_CAP_PROP_FPS ); // frame-rate

    // CONSTRUIRE L'ARRIERE-PLAN
    IplImage *fond = getBackground(capture, proportion, height, width);

    string fileImgFond = createNameFileSave(fileVideo, "_fond.png");

    // enregistrer l'arriere-plan sur un fichier d'image
    cvSaveImage(fileImgFond.c_str(), fond);

    // afficher l'arriere-plan
    cvNamedWindow("Fond");
    cvShowImage("Fond", fond);

    IplImage *image, *gray, *diff, *writeFrameFond;
    vector<CvRect> boxes; // MBR
    char key = ' '; // pour arreter le video

    // deux fenetres pour afficher deux videos : video d'arriere-plan et video MBR
    cvNamedWindow("VideoDiff");
    cvNamedWindow("VideoMBR");

    // creer le nom du fichier enregistre
    string fileSaveFond = createNameFileSave(fileVideo, "_fond.mpg");
    string fileSaveMBR = createNameFileSave(fileVideo, "_mbr.mpg");

    // creer le fichier pour enregistrer deux videos
    CvVideoWriter* writerFond = cvCreateVideoWriter(fileSaveFond.c_str(),  // chemin du fichier
                                                CV_FOURCC('P','I','M','1'),        // codec
                                                fps ,                              // frame-rate
                                                cvSize(width, height));     // définition
    CvVideoWriter* writerMBR = cvCreateVideoWriter(fileSaveMBR.c_str(),  // chemin du fichier
                                                CV_FOURCC('P','I','M','1'),        // codec
                                                fps ,                              // frame-rate
                                                cvSize(width, height));     // définition

    writeFrameFond = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
    cvZero(writeFrameFond);

    CvScalar s;
    while (key != 27)
    {
        if (!cvGrabFrame( capture ))
        {
            break;
        }

        image = cvRetrieveFrame( capture );

        // chuyen sang anh gray
        gray = cvCreateImage(cvGetSize(image), image->depth, 1);
        diff = cvCreateImage(cvGetSize(image), image->depth, 1);
        cvConvertImage(image, gray, CV_BGR2GRAY);

        // tru voi anh fond
        cvAbsDiff(gray, fond, diff);
        cvThreshold(diff, diff, 50, 255, CV_THRESH_BINARY);

        // xem xet smooth truoc hay sau
        cvSmooth(diff, diff, CV_MEDIAN, 3, 3);

        // thuc hien dilate va erode

        //diff = removeNoise(diff);

        cvErode(diff, diff, NULL, 3);

        // tiep den la dilate 6 lan
        cvDilate(diff, diff, NULL, 3);

        // lai erode 2 lan
        //cvErode(diff, diff, NULL, 9);

        //cvDilate(diff, diff, NULL, 4);

        //cvSmooth(diff, diff, CV_GAUSSIAN, 3, 3);

        // copy sang anh luu
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                s.val[0] = 0;
                s.val[1] = 0;
                s.val[2] = 0;

                s = cvGet2D(diff, i, j);

                if (s.val[0] == 255)
                {
                    s.val[1] = 255;
                    s.val[2] = 255;
                }

                cvSet2D(writeFrameFond, i, j, s);
            }
        }

        // rechercher des boite englobant minimale
        boxes = findBoundingBox(diff);

        // dessiner les boites englobants minimales recherchees
        drawBoundingBox(image, boxes);

        // play video
        cvShowImage("VideoDiff", writeFrameFond);
        cvShowImage("VideoMBR", image);

        cvWriteFrame( writerFond, writeFrameFond );
        cvWriteFrame( writerMBR, image );

        cvReleaseImage(&gray);
        cvReleaseImage(&diff);
        key = cvWaitKey(20);
    }
    image = NULL;

    //cvWaitKey(0);

    cvDestroyAllWindows();
    cvReleaseCapture(&capture);
    cvReleaseVideoWriter( &writerFond );
    cvReleaseVideoWriter( &writerMBR );

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
/*string getNameOfFolder(const string &path) // ex : /home/manleviet
{
    size_t found = path.find_last_of('/');
    if (found >= path.length())
    {
        return "";
    }
    return path.substr(found + 1);
}*/

//
// Reduire le nombre de niveau de gris
// @param :
//      IplImage *img : l'image originale
//      int numLevels : nombre de niveau de gris desire
//
/*void reduceGreyLevels(IplImage *img, int numLevels)
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
}*/

//
// A partir du nom de repertoire, creer un nom du fichier de resultats
// sous la forme <nom de repertoire>_l<niveau de gris>_d<des distances>_di<nombre de directions>.glcm
// @param :
//      string path : un chemin
//      int greyLevel : le niveau de gris
//      int numDirections : le nombre de directions
// @result : nom du fichier de resultats
//
/*string createNameOfFileSave(string path, int greyLevel, bool distance[], int numDirections)
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
}*/

//
// Ecrire la tete du fichier
//
/*void writeHeaderOfFileSave(std::ofstream &out,
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
}*/

//
// Prendre le nom de la classe de texture
// @param :
//      bool flagBase : true - la base 2-RotInv_15_7_64
//      string filename : le nom du fichier d'image
// @result : le nom de texture
//
/*string getClass(bool flagBase, string filename)
{
    size_t pos = filename.find_first_of("_");
    if (pos == string::npos)
        return "";

    if (flagBase)
        pos = filename.find_first_of("_", pos + 1);

    return filename.substr(0, pos);
}*/

IplImage *getBackground(CvCapture *capture, int proportion, int height, int width)
{
    double totalFrames = countAVIFrames(capture) + 1;

    double numFrames = totalFrames * proportion / 100;
    int divise = totalFrames / numFrames;

    cout << totalFrames << endl;
    cout << numFrames << endl;
    cout << divise << endl;

    int frameCount = divise - 1;
    int count = 0;

    // khoi tao mang background
    double ***background = initBackground(height, width, 256);

    IplImage *image, *gray;
    CvScalar s;
    int pos;
    // Tant qu'on n'a pas appuyé sur Q, on continue :
    while(1)
    {
        frameCount++;
        if(!cvGrabFrame( capture ))
        {
            break;
        }

        if (frameCount == divise)
        {
            count++;
            image = cvRetrieveFrame( capture );

            // xu ly de tinh fond
            // chuyen sang anh gris
            gray = cvCreateImage(cvGetSize(image), image->depth, 1);
            cvConvertImage(image, gray, CV_BGR2GRAY);

            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    s = cvGet2D(gray, i, j);
                    pos = (int)s.val[0];

                    background[i][j][pos]++;
                }
            }

            // giai phong bo nho
            cvReleaseImage(&gray);
            gray = NULL;

            frameCount = 0;
        }
    }
    cvSetCaptureProperty( capture, CV_CAP_PROP_POS_MSEC, 0);

    cout << count << endl;
    image = NULL;

    // tao ra anh background
    IplImage *fond = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);

    int total;
    int medium;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            // tinh tong
            total = 0;
            for (int k = 0; k < 256; k++)
            {
                total += background[i][j][k];
            }

            // tim median
            medium = total / 2;
            total = 0;
            int k;
            for (k = 0; k < 256; k++)
            {
                total += background[i][j][k];
                if (total >= medium)
                {
                    break;
                }
            }

            s.val[0] = k; // median

            cvSet2D(fond, i, j, s);
        }
    }

    // thieu giai phong bo nho cua background
    deleteBackground(background, height, width);

    return fond;
}

double*** initBackground(int height, int width, int size)
{
    double ***background = (double***)malloc(sizeof(double**) * height);

    for (int i = 0; i < height; i++)
    {
        background[i] = (double**)malloc(sizeof(double*) * width);
    }

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            background[i][j] = (double*)malloc(sizeof(double) * size);
            memset(background[i][j], 0, sizeof(double) * size);
        }
    }

    return background;
}

void deleteBackground(double ***background, int height, int width)
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            delete background[i][j];
            background[i][j] = NULL;
        }
        delete background[i];
        background[i] = NULL;
    }
    delete background;
}

double countAVIFrames(CvCapture *capture)
{
    double count = 1;
    while (1)
    {
        if (!cvGrabFrame(capture))
        {
            break;
        }

        count++;
    }

    cvSetCaptureProperty( capture, CV_CAP_PROP_POS_MSEC, 0 );

    return count;
}

//
// code source : http://www.ymer.org/amir/2007/06/04/getting-the-number-of-frames-of-an-avi-video-file-for-opencv/
//
/*double countFrames(string filename)
{
    double nFrames;
    char tempSize[4];

    ifstream videoFile(filename.c_str(), ios::in|ios::binary);
    videoFile.seekg(0x30, ios::beg);

    videoFile.read(tempSize, 4);

    videoFile.close();
    nFrames = tempSize[0] + 256 * tempSize[1] + 256^2 * tempSize[2] + 256^3 * tempSize[3];

    return nFrames;
}*/

string createNameFileSave(string fileVideo, string ext) // ex : /home/manleviet/bikini.avi
{
    string filename;
    size_t found = fileVideo.find_last_of('.');
    if (found == string::npos)
    {
        return "";
    }
    filename = fileVideo.substr(0, found); // ex : /home/manleviet/bikini

    filename += ext;

    return filename;
}

//
// Supprimer les bruits
// code-source : http://stackoverflow.com/questions/2243646/object-detection-in-opencv
// @param :
//      IplImage *img : une image
// @result : une image modifiee
//
/*IplImage *removeNoise(IplImage *img)
{
    // supprimer les bruits
    IplImage *output = cvCloneImage(img);
    CvSize sz = cvSize( img->width & -2, img->height & -2 );
    IplImage* pyr = cvCreateImage( cvSize(sz.width/2, sz.height/2), 8, 1 );

    cvPyrDown( output, pyr, 7 );
    cvPyrUp( pyr, output, 7 );

    return output;
}*/

//
// code-source  // code-source http://cgi.cse.unsw.edu.au/~cs4411/wiki/index.php?title=OpenCV_Guide
//
vector<CvRect> findBoundingBox(IplImage *image)
{
    CvSeq* seq;
    vector<CvRect> boxes;
    CvMemStorage* storage;

    //Memory allocated for OpenCV function operations
    storage = cvCreateMemStorage(0);
    cvClearMemStorage(storage);

    //Find connected pixel sequences within a binary OpenGL image (diff), starting at the top-left corner (0,0)
    cvFindContours(image, storage, &seq, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, cvPoint(0,0));

    //Iterate through segments
    for(; seq; seq = seq->h_next) {
            //Find minimal bounding box for each sequence
            CvRect boundbox = cvBoundingRect(seq);
            boxes.push_back(boundbox);
    }

    cvReleaseMemStorage(&storage);
    delete seq;
    seq = NULL;

    return boxes;
}

void drawBoundingBox(IplImage *image, vector<CvRect> boxes)
{
    CvRect rect;
    CvScalar blue;
    blue.val[0] = 255;
    blue.val[1] = 0;
    blue.val[2] = 0;

    for (unsigned int i = 0; i < boxes.size(); i++)
    {
        rect = boxes.at(i);
        cvRectangle(image,
                    cvPoint(rect.x, rect.y),
                    cvPoint(rect.x + rect.height, rect.y + rect.width),
                    blue);
    }
}

#include"mapCreation.h"

std::tuple< QList<Wall>, QList<Wall>, qreal, qreal> createStreetMap(qreal mapWidth, qreal mapHeigth){
    // Create map of a street for outdoor 5G raytracing

    //building 1
    Wall wall0(80 ,0  ,80 ,29 ,0.3 ,5);
    Wall wall1(80 ,29 ,0  ,29 ,0.3 ,5);
    Wall wall2(0  ,0  ,0  ,29 ,0.3 ,5);
    //Diffraction virtual wall = diagonal in building
    Wall diff0(80,29,61,10, 0,6);

    //building 2
    Wall wall3(90 ,0  ,90  ,30 ,0.3, 0);
    Wall wall4(90 ,30 ,160 ,30 ,0.3, 0);
    Wall wall5(160,30 ,160 ,0  ,0.3, 0);
    //Diffraction virtual wall = diagonal in building
    Wall diff1(140,10,160,30, 0,6);
    Wall diff2(90,30,110,10, 0,6);

    //building 3
    Wall wall6(170 ,0  ,170 ,25 , 0.3, 4);
    Wall wall7(250 ,25 ,250 ,0  , 0.3, 4);
    Wall wall8(170 ,25 ,250 ,25 , 0.3, 4);
    //Diffraction virtual wall = diagonal in building
    Wall diff3(170,25,190,5, 0,6);

    //building 4
    Wall wall9 (0  ,40 ,0  ,70 , 0.3, 4);
    Wall wall10(0  ,40 ,80 ,40  , 0.3, 4);
    Wall wall11(80 ,40 ,80 ,70 , 0.3, 4);
    //Diffraction virtual wall = diagonal in building
    Wall diff4(60,60,80,40, 0,6);

    //building 5
    Wall wall12(90  ,70 ,90  ,41  , 0.3, 0);
    Wall wall13(90  ,41  ,160 ,41  , 0.3, 0);
    Wall wall14(160 ,41  ,160 ,70 , 0.3, 0);
    //Diffraction virtual wall = diagonal in building
    Wall diff5(95,41,109,60, 0,6);
    Wall diff6(141,60,160,41, 0,6);

    //building 6
    Wall wall15(170 ,70 ,170 ,40  , 0.3, 5);
    Wall wall16(170 ,40  ,250 ,40 , 0.3, 5);
    Wall wall17(250 ,40  ,250 ,70 , 0.3, 5);
    //Diffraction virtual wall = diagonal in building
    Wall diff7(170,40,190,60, 0,6);

    QList<Wall> walls{wall0, wall1, wall2,wall3,wall4,wall5,wall6,wall7,wall8,wall9,wall10,wall11,wall12,wall13,wall14,wall15,wall16,wall17};
    QList<Wall> diffr{diff0, diff1, diff2,diff3,diff4,diff5,diff6,diff7};
    qreal scaleX = mapWidth/253; //this will be needed to be able to display high length on the screen
                                    // --> conversion: screenLength = mapWidth = maxRealLength * scale;
    qreal scaleY = mapHeigth/50;

    return std::make_tuple(walls,diffr, scaleX, scaleY);
}

std::tuple< QList<Wall>, qreal, qreal> createStudioMap(qreal mapWidth, qreal mapHeigth){
    // Create map of a studio for indoor wifi raytracing

    //EXTERNAL WALLS IN CONCRETE TYPE=0
    Wall wall0(0,    0,    1400,  0,    0.19, 0);
    Wall wall1(0,    700,  1400,  700,  0.19, 0);
    Wall wall2(1400, 0,    1400,  700,  0.19, 0);
    Wall wall3(0,    0,    0,     325,  0.19, 0);
    Wall wall4(0,    425,  0,     700,  0.19, 0);

    //Internal wall, mostly partition walls type=1
    Wall wall5( 0,   300, 175,  300, 0.05, 1);
    Wall wall6( 275, 300, 450,  300, 0.06, 1);
    Wall wall7( 450, 0,   450,  300, 0.05, 1);
    Wall wall8( 450, 200, 525,  200, 0.05, 1);
    Wall wall9( 625, 200, 725,  200, 0.05, 1);
    Wall wall10(700, 0,   700,  200, 0.06, 1);
    Wall wall11(825, 200, 849,  200, 0.05, 1);
    Wall wall12(850, 0,   850,  300, 0.06, 1);
    Wall wall13(850, 300, 950,  300, 0.05, 1);
    Wall wall14(1050,300, 1400, 300, 0.05, 1);

    //Internal walls, bricks, type=2
    Wall wall15(0, 450, 750, 450, 0.2, 2);
    Wall wall16(850, 450, 850, 700, 0.2, 2);

    QList<Wall> walls{wall0, wall1, wall2,wall3,wall4,wall5,wall6,wall7,wall8,wall9,wall10,wall11,wall12,wall13,wall14,wall15,wall16};
    qreal scaleX = mapWidth/1400; //this will be needed to be able to display high length on the screen
                                    // --> conversion: screenLength = mapWidth = maxRealLength * scale;
    qreal scaleY = mapHeigth/700;

    return std::make_tuple(walls, scaleX, scaleY);
}


// SOME EXEMPLE TO ENTER IN MAINWINDOW.CPP
/*
//Small exemple configuration
Wall wall0(100,350,100,700, 0.2 , 0); Wall wall1(100,700,400,700, 0.2, 0); Wall wall2(400,700,400,350,0.15,1);
Wall wall3(400,350,100,350,0.2,0);
QList<Wall> walls{wall0, wall1, wall2,wall3};
qreal scaleX = 1340.000/400.000; qreal scaleY = 640.000/700.000;
//qDebug() << "scale X = " << scaleX << " scale Y = " << scaleY;
*/

#include"mapCreation.h"

std::tuple< QList<Wall>, QList<Wall>, qreal, qreal> createStreetMap(qreal mapWidth, qreal mapHeigth){
    // Create map of a street for outdoor 5G raytracing

    //building 1
    Wall wall0(80 ,0  ,80 ,20 ,0.3 ,5);
    Wall wall1(80 ,20 ,0  ,20 ,0.3 ,5);
    Wall wall2(0  ,0  ,0  ,20 ,0.3 ,5);
    //Diffraction virtual wall = diagonal in building
    Wall diff0(80,20,60,0, 0,6);

    //building 2
    Wall wall3(95 ,0  ,95  ,20 ,0.3, 0);
    Wall wall4(95 ,20 ,165 ,20 ,0.3, 0);
    Wall wall5(165,20 ,165 ,0  ,0.3, 0);
    //Diffraction virtual wall = diagonal in building
    Wall diff1(145,0,165,20, 0,6);
    Wall diff2(95,20,115,0, 0,6);

    //building 3
    Wall wall6(175 ,0  ,175 ,15 , 0.3, 4);
    Wall wall7(253 ,15 ,253 ,0  , 0.3, 4);
    Wall wall8(175 ,15 ,253 ,15 , 0.3, 4);
    //Diffraction virtual wall = diagonal in building
    Wall diff3(175,15,188,0, 0,6);

    //building 4
    Wall wall9 (0  ,30 ,0  ,50 , 0.3, 4);
    Wall wall10(0  ,30 ,80 ,30  , 0.3, 4);
    Wall wall11(80 ,30 ,80 ,50 , 0.3, 4);
    //Diffraction virtual wall = diagonal in building
    Wall diff4(60,50,80,30, 0,6);

    //building 5
    Wall wall12(95  ,50 ,95  ,30  , 0.3, 0);
    Wall wall13(95  ,30  ,165 ,30  , 0.3, 0);
    Wall wall14(165 ,30  ,165 ,50 , 0.3, 0);
    //Diffraction virtual wall = diagonal in building
    Wall diff5(95,30,115,50, 0,6);
    Wall diff6(145,50,165,30, 0,6);

    //building 6
    Wall wall15(175 ,50 ,175 ,30  , 0.3, 5);
    Wall wall16(175 ,30  ,253 ,30 , 0.3, 5);
    Wall wall17(253 ,30  ,253 ,50 , 0.3, 5);
    //Diffraction virtual wall = diagonal in building
    Wall diff7(175,30,193,50, 0,6);

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

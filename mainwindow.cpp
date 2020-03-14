#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "coefficients.h"
#include "wall.h"
#include <algorithm>
#define pi 3.1416

bool IsReflexionOK(QPointF A, QPointF B, QPointF mirror, QLineF line);
bool isNotOnWall(QPointF A, QLineF wall);
bool isNotBlocked(QPointF A, QPointF B, QList<Wall> walls);
QColor color(qreal power);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->view->setRenderHints(QPainter::Antialiasing);
    ui->view->setScene(scene);
    ui->view->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    /* //EXTERNAL WALLS IN CONCRETE TYPE=0
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

    */
    //Small exemple configuration
    Wall wall0(100,350,100,700, 0.2 , 0); Wall wall1(100,700,400,700, 0.2, 0); Wall wall2(400,700,400,350,0.15,1);
    Wall wall3(400,350,100,350,0.2,0);

    QList<Wall> walls{wall0, wall1, wall2,wall3};//,wall4,wall5,wall6,wall7,wall8,wall9,wall10,wall11,wall12,wall13,wall14,wall15,wall16};

    qreal he = 3*pow(10,8)/(2.45*pow(10,9)*pi);
    qreal Ra = 71;
    qreal Ptx = 0.1; //Watts
    qreal Gtx = 16/(3*pi);
    //qreal sum; qreal Prx; qreal Prx_dBm;

    QPen meshOutline(Qt::black);
    Mesh transmitter(150, 250, 5, 5, 0);
    Mesh receiver(350, 200 ,5 ,5 ,1);
    ui->view->scene()->addRect(transmitter, meshOutline, transmitter.brush);
    ui->view->scene()->addRect(receiver, meshOutline, receiver.brush);


    qreal Prx = 0, Prx_dBm = 0;
    qreal sum = 0;
    qreal mapWidth = 1400; qreal mapHeigth = 700; qreal resolution=10;


    /*//launch color map from here
    for (int w =0; w<(mapWidth/resolution); w++){
        for(int h=0; h<(mapHeigth/resolution); h++){

            Mesh receiver(resolution/2 + w*resolution, resolution/2 + h*resolution, resolution,resolution,1);

            sum = imageMethodColorMap(transmitter,receiver,walls);
            Prx = pow(he,2)*60*Gtx*Ptx*pow(sum,2) / (8*Ra);
            Prx_dBm = 20 * log10( Prx/ 0.001);
            qDebug() << w << " , " << h << " : " << Prx_dBm;
            receiver.brush.setColor( color(Prx_dBm));
            ui->view->scene()->addRect(receiver, meshOutline, receiver.brush);
        }
    }*/


    //visualize a ray tracing from here
    sum = imageMethod5G(transmitter, receiver, walls); // computation of the sum in the power computation formula
    qDebug() << "sum = " << sum;
    Prx = pow(he,2)*60*Gtx*Ptx*pow(sum,2) / (8*Ra);
    qDebug() << "Prx = " << Prx;
    Prx_dBm = 10 * log10( Prx/ 0.001);
    qDebug() << Prx_dBm << "dBm";



    // Rem drawing walls at the end make them appear more clearly on the final display
    for(int wall =0; wall<walls.size(); wall++){
        ui->view->scene()->addLine(walls[wall], walls[wall].outlinePen);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

qreal MainWindow::imageMethod5G(Mesh TX, Mesh RX, QList<Wall> walls){
    QPointF R = RX.center(); QPointF T = TX.center();
    qreal sum=0; // Initialize the sum for total power computation. The 'sum' is the summation where each term
                // is for one ray. Those terms are the product of total reflection coef and total transmission
                // coef, divided by square of distance made by the ray (including reflections). 'sum' is updated
                // as soon as needed information is gathered to compute a new term.

    if(isNotBlocked(T,R,walls)){
        draw_ray(R,T,0);
        sum += transmission_coef(R,T,walls)/QLineF(R,T).length()*100; // Note '*(100)' is to convert from cm to m units
    }

    int a,b;
    QPointF r0, r1,r2; QLineF line; QVector<QPointF> m_list;

    // first mirror wrt each wall, and one time reflected rays computations
    for(int i =0; i < walls.size(); i++){

        m_list.insert(0, mirrorPoint(T, walls[i]) );
        if( IsReflexionOK(T,R,m_list[0],walls[i]) ){
            line.setP1(R);line.setP2(m_list[0]); line.intersect(walls[i], &r0);

            if(isNotBlocked(R,r0,walls) && isNotBlocked(r0,T,walls)){
                sum  += reflexion_coef(T,r0,walls[i]) / QLineF(m_list[0],R).length()*100;
                draw_ray(T,r0,1); draw_ray(R,r0,1);
            }
        }

        // second mirror wrt each wall, and twice reflected rays computations
        for (int j =0; j<walls.size(); j++){

                if(j == i){continue;} // exclude the wall already considered for last mirror point, continue skips this for loop step and makes j++
                m_list.insert(1, mirrorPoint(m_list[0], walls[j]));
                if( IsReflexionOK(m_list[0],R,m_list[1],walls[j])){
                    line.setP1(R);line.setP2(m_list[1]); line.intersect(walls[j], &r1);
                    line.setP1(m_list[0]);line.setP2(r1); a = line.intersect(walls[i], &r0); // IMPORTANT 'a == intersect...' statement which gives
                                                                                                // 'a' value of one if 'UPDATED r0' belongs to walls[i]

                    if(a == 1 && isNotBlocked(R,r1,walls) && isNotBlocked(r1,r0,walls) && isNotBlocked(r0,T,walls)){
                        sum += reflexion_coef(R,r1, walls[j]) *
                               reflexion_coef(r1,r0,walls[i]) / QLineF(m_list[1],R).length()*100;

                        draw_ray(R,r1,2); draw_ray(r1,r0,2); draw_ray(r0,T,2);
                    }
                }

                // third mirror wrt each wall, and three times reflected rays computations
                for(int k=0; k<walls.size(); k++){
                        if(k==j){continue;}
                        m_list.insert(2, mirrorPoint(m_list[1], walls[k]));
                        if( IsReflexionOK(m_list[1],R,m_list[2],walls[k])){
                            line.setP1(R);line.setP2(m_list[2]); line.intersect(walls[k], &r2);
                            line.setP1(m_list[1]);line.setP2(r2); a = line.intersect(walls[j], &r1); // IMPORTANT 'a == intersect...'
                            line.setP1(m_list[0]);line.setP2(r1); b = line.intersect(walls[i], &r0); // IMPORTANT 'b == intersect...' statements which
                                                                                             // give 'a' and 'b' value of one if 'UPDATED r1
                                                                                             //and r0' belong respectively to walls[j] and walls[i]
                            if(a==1 && b==1
                                    && isNotBlocked(R,r2,walls) && isNotBlocked(r2,r1,walls) && isNotBlocked(r1,r0,walls) && isNotBlocked(r0,T,walls)){

                                sum += reflexion_coef(R,r2, walls[k]) *
                                       reflexion_coef(r2,r1,walls[j]) *
                                       reflexion_coef(r1,r0,walls[i]) / QLineF(m_list[2],R).length()*100;

                                draw_ray(R,r2,3); draw_ray(r2,r1,3);draw_ray(r1,r0,3);
                                draw_ray(r0,T,3);
                            }
                        }
                }
        }

    }

    return sum;
}


qreal MainWindow::imageMethodColorMap(Mesh TX, Mesh RX, QList<Wall> walls){
    QPointF R = RX.center(); QPointF T = TX.center();
    qreal sum=0; // Initialize the sum for total power computation. The 'sum' is the summation where each term
                // is for one ray. Those terms are the product of total reflection coef and total transmission
                // coef, divided by square of distance made by the ray (including reflections). 'sum' is updated
                // as soon as needed information is gathered to compute a new term.

    sum += transmission_coef(R,T,walls)/QLineF(R,T).length()*100; // Note '*(100)' is to convert from cm to m units

    int a,b;
    QPointF r0, r1,r2; QLineF line; QVector<QPointF> m_list;

    // first mirror wrt each wall, and one time reflected rays computations
    for(int i =0; i < walls.size(); i++){

        m_list.insert(0, mirrorPoint(T, walls[i]) );
        if( IsReflexionOK(T,R,m_list[0],walls[i]) ){
            line.setP1(R);line.setP2(m_list[0]); line.intersect(walls[i], &r0);

            sum  += reflexion_coef(T,r0,walls[i]) * transmission_coef(T,r0,walls) * transmission_coef(r0,R,walls) / QLineF(m_list[0],R).length()*100;
        }

        // second mirror wrt each wall, and twice reflected rays computations
        for (int j =0; j<walls.size(); j++){

                if(j == i){continue;} // exclude the wall already considered for last mirror point, continue skips this for loop step and makes j++
                m_list.insert(1, mirrorPoint(m_list[0], walls[j]));
                if( IsReflexionOK(m_list[0],R,m_list[1],walls[j])){
                    line.setP1(R);line.setP2(m_list[1]); line.intersect(walls[j], &r1);
                    line.setP1(m_list[0]);line.setP2(r1); a = line.intersect(walls[i], &r0); // IMPORTANT 'a == intersect...' statement which gives
                                                                                                // 'a' value of one if 'UPDATED r0' belongs to walls[i]

                    if(a == 1){
                        sum += reflexion_coef(R,r1, walls[j]) * transmission_coef(R,r1,walls) *
                               reflexion_coef(r1,r0,walls[i]) * transmission_coef(r1,r0,walls) *
                               transmission_coef(r0,T,walls) / QLineF(m_list[1],R).length()*100;
                    }
                }

                // third mirror wrt each wall, and three times reflected rays computations
                for(int k=0; k<walls.size(); k++){
                        if(k==j){continue;}
                        m_list.insert(2, mirrorPoint(m_list[1], walls[k]));
                        if( IsReflexionOK(m_list[1],R,m_list[2],walls[k])){
                            line.setP1(R);line.setP2(m_list[2]); line.intersect(walls[k], &r2);
                            line.setP1(m_list[1]);line.setP2(r2); a = line.intersect(walls[j], &r1); // IMPORTANT 'a == intersect...'
                            line.setP1(m_list[0]);line.setP2(r1); b = line.intersect(walls[i], &r0); // IMPORTANT 'b == intersect...' statements which
                                                                                             // give 'a' and 'b' value of one if 'UPDATED r1
                                                                                             //and r0' belong respectively to walls[j] and walls[i]
                            if(a==1 && b==1){
                                sum += reflexion_coef(R,r2, walls[k]) * transmission_coef(R,r2,walls) *
                                       reflexion_coef(r2,r1,walls[j]) * transmission_coef(r2,r1,walls)*
                                       reflexion_coef(r1,r0,walls[i]) * transmission_coef(r1,r0,walls)*
                                       transmission_coef(r0,T,walls) / QLineF(m_list[2],R).length()*100;
                            }
                        }
                }
        }

    }

    return sum;
}


qreal MainWindow::imageMethod2(Mesh TX, Mesh RX, QList<Wall> walls){
    QPointF R = RX.center(); QPointF T = TX.center();
    qreal sum=0; // Initialize the sum for total power computation. The 'sum' is the summation where each term
                // is for one ray. Those terms are the product of total reflection coef and total transmission
                // coef, divided by square of distance made by the ray (including reflections). 'sum' is updated
                // as soon as needed information is gathered to compute a new term.

    draw_ray(R,T,0);
    sum += transmission_coef(R,T,walls)/QLineF(R,T).length()*100; // Note '*(100)' is to convert from cm to m units

    int a,b;
    QPointF r0, r1,r2; QLineF line; QVector<QPointF> m_list;

    // first mirror wrt each wall, and one time reflected rays computations
    for(int i =0; i < walls.size(); i++){

        m_list.insert(0, mirrorPoint(T, walls[i]) );
        if( IsReflexionOK(T,R,m_list[0],walls[i]) ){
            line.setP1(R);line.setP2(m_list[0]); line.intersect(walls[i], &r0);

            sum  += reflexion_coef(T,r0,walls[i]) * transmission_coef(T,r0,walls) * transmission_coef(r0,R,walls) / QLineF(m_list[0],R).length()*100;
            draw_ray(T,r0,1); draw_ray(R,r0,1);
        }

        // second mirror wrt each wall, and twice reflected rays computations
        for (int j =0; j<walls.size(); j++){

                if(j == i){continue;} // exclude the wall already considered for last mirror point, continue skips this for loop step and makes j++
                m_list.insert(1, mirrorPoint(m_list[0], walls[j]));
                if( IsReflexionOK(m_list[0],R,m_list[1],walls[j])){
                    line.setP1(R);line.setP2(m_list[1]); line.intersect(walls[j], &r1);
                    line.setP1(m_list[0]);line.setP2(r1); a = line.intersect(walls[i], &r0); // IMPORTANT 'a == intersect...' statement which gives
                                                                                                // 'a' value of one if 'UPDATED r0' belongs to walls[i]

                    if(a == 1){
                        sum += reflexion_coef(R,r1, walls[j]) * transmission_coef(R,r1,walls) *
                               reflexion_coef(r1,r0,walls[i]) * transmission_coef(r1,r0,walls) *
                               transmission_coef(r0,T,walls) / QLineF(m_list[1],R).length()*100;

                        draw_ray(R,r1,2); draw_ray(r1,r0,2); draw_ray(r0,T,2);
                    }
                }

                // third mirror wrt each wall, and three times reflected rays computations
                for(int k=0; k<walls.size(); k++){
                        if(k==j){continue;}
                        m_list.insert(2, mirrorPoint(m_list[1], walls[k]));
                        if( IsReflexionOK(m_list[1],R,m_list[2],walls[k])){
                            line.setP1(R);line.setP2(m_list[2]); line.intersect(walls[k], &r2);
                            line.setP1(m_list[1]);line.setP2(r2); a = line.intersect(walls[j], &r1); // IMPORTANT 'a == intersect...'
                            line.setP1(m_list[0]);line.setP2(r1); b = line.intersect(walls[i], &r0); // IMPORTANT 'b == intersect...' statements which
                                                                                             // give 'a' and 'b' value of one if 'UPDATED r1
                                                                                             //and r0' belong respectively to walls[j] and walls[i]
                            if(a==1 && b==1){
                                sum += reflexion_coef(R,r2, walls[k]) * transmission_coef(R,r2,walls) *
                                       reflexion_coef(r2,r1,walls[j]) * transmission_coef(r2,r1,walls)*
                                       reflexion_coef(r1,r0,walls[i]) * transmission_coef(r1,r0,walls)*
                                       transmission_coef(r0,T,walls) / QLineF(m_list[2],R).length()*100;

                                draw_ray(R,r2,3); draw_ray(r2,r1,3);draw_ray(r1,r0,3);
                                draw_ray(r0,T,3);
                            }
                        }
                }
        }

    }

    return sum;
}

QPointF MainWindow::mirrorPoint(QPointF P, QLineF d){
    // Returns a QPointF which is the mirror image of P compared to line d


    QPointF inters, mirrorP;
    d.normalVector().translated(P.rx()- d.x1(), P.ry() - d.y1()).intersect(d, &inters); // stores in 'P1' the intersection point between line d and its normal line containing P
    mirrorP.setX( 2*inters.x() - P.x());
    mirrorP.setY( 2*inters.y() - P.y());

    return mirrorP;
}


bool IsReflexionOK(QPointF A, QPointF B, QPointF mirror, QLineF line){
    // Check if wall is suitable for reflexion, so if A and B points are not on opposite sides of wall and if reflection point is on wall.
    // if A and B are on same side, then return true

    if( line.intersect( QLineF(mirror,B), nullptr) == 1){ //check if reflexion point exist ON wall (== 1 for bounded)

        bool a = (line.p2().x() - line.p1().x())*(A.y() - line.p1().y()) - (line.p2().y() - line.p1().y())*(A.x() - line.p1().x()) > 0;
        bool b = (line.p2().x() - line.p1().x())*(B.y() - line.p1().y()) - (line.p2().y() - line.p1().y())*(B.x() - line.p1().x()) > 0;

        return ( (a&&b)||(!a && !b) ); // return true if a==b, so if A and B on same side of line
    }
    else{ return false;}

}


void MainWindow::draw_ray(QPointF A, QPointF B, int n_refl){
    // draw the ray on the graphic scene taking into account the number of reflexions for the color
    // allowing to distinguish them

    switch(n_refl){
        case 0:     ui->view->scene()->addLine(QLineF(A,B), QPen(Qt::black)); break;
        case 1:     ui->view->scene()->addLine(QLineF(A,B), QPen(Qt::red)); break;
        case 2:     ui->view->scene()->addLine(QLineF(A,B), QPen(Qt::green)); break;
        case 3:     ui->view->scene()->addLine(QLineF(A,B), QPen(Qt::blue)); break;
    }

}

qreal MainWindow :: reflexion_coef(QPointF A, QPointF B, Wall wall){
    // Compute the reflexion coef of a ray coming from A reflecting on wall at point B (works also from B to A
    // with A on wall)
    if(A.x() == B.x() && A.y()== B.y()){ //this case may happen if points A, B are both on a corner and equal to each other (assumption made
                                            // that reflected wave is canceled in this case
        return 0;
    }
    else{
        qreal thetai = theta_i( QLineF(A,B),wall);
        return abs_tot_reflexion_coef(thetai, wall.permittivity, wall.conductivity, wall.thickness);
    }
}

bool isNotOnWall(QPointF A, QLineF wall){
    //Check if P belongs to the wall and return true if point is not (check by cross product with wall points)
    qreal cross_p = abs ( (A.x()-wall.p1().x())*(wall.p2().y()-wall.p1().y()) - ((A.y()-wall.p1().y())*(wall.p2().x()-wall.p1().x())) );
    return cross_p > 0.05; // It is unsafe to compare floating points with zero so give arbitrary security margin of 0.05
}

qreal MainWindow :: transmission_coef(QPointF A, QPointF B, QList<Wall> walls){
    // Identify and compute the total transmission coef of a ray from A to B through all intersected walls
    // Note the use of isNotOnWall function to check if considered points are not on wall itself

    qreal product = 1; //put a first neutral coef equal to one, then update the product when a transmission is found
    qreal thetai;

    for(int i=0; i<walls.size(); i++){
        bool ok = (walls[i].intersect( QLineF(A,B), nullptr)==1) && isNotOnWall(A,walls[i]) && isNotOnWall(B,walls[i]);
        if(ok){
            thetai = theta_i( QLineF(A,B), walls[i]);
            product = product*abs_tot_transmission_coef(thetai, walls[i].permittivity, walls[i].conductivity, walls[i].thickness);

        }
        else continue;
    }
    return product;
}

bool isNotBlocked(QPointF A, QPointF B, QList<Wall> walls){
    // For 5G case, check if computed ray crosses path of a wall. If yes, return true.
    // Function useful to discard all transmission cases.

    bool nok;
    for(int i=0; i<walls.size(); i++){
        nok = (walls[i].intersect( QLineF(A,B), nullptr)==1) && isNotOnWall(A,walls[i]) && isNotOnWall(B,walls[i]);
    }

    return nok;
}

QColor color(qreal power){

    if( power <= -25 && power > -36){
        return QColor (255 , qRound((-25-power)/11*255) ,0, 170); //from red to yellow
    }

    else if( power <= -36 && power > -47){
        return QColor(255 - qRound(((-36-power)/11)*255), 255,0, 170);    //from yellow to green
    }

    else if(power <= -47 && power > -58){
        return QColor(0, 255, qRound(((-47-power)/11)*255), 170) ; //from green to cyan
    }

    else if(power <= -58 && power > -69){
        return QColor(0, 255 - ((-58-power)/11)*255, 255, 170) ; //from cyan to blue
    }

    else if(power <= -69 && power > -80){
        return QColor(0, 0, 255- ((-69-power)/11)*255, 170) ; //from green to blue
    }
    else if(power < -80){
        return QColor (0,0,0,170); // black if below -95
    }

    else{
        return QColor (255,0,0,170); // red if above -25
    }

        /*if (qRound(power) > -35) {
            return QColor (255,0,0,200);
        }
        else if (qRound(power) <= -35 && qRound(power) ){
            return QColor (255,i*11,0,200);
        }
        else if (i>23 && i<=46){
            return QColor (255 - (i-23)*11,255,0,200);
        }
        else if (i>46 && i<=69){
            return QColor (0,255,(i-46)*11,200);
        }
        else if (i>69 && i<=92){
            return QColor (0,255 - (i-69)*11,255,200);
        }
        else if (i>92 && i<=146){
            return QColor (0,0,255-(i-92)*4,200);
        }
        else {
            return QColor (0,0,0,200);
        }*/

}

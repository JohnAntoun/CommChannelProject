#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "coefficients.h"
#include "wall.h"
#
#include "mapCreation.h"
#include <algorithm>
#include "qcustomplot.h"
#include "regression.h"

#define pi 3.14159
#define k_boltz 1.379*pow(10,-23)

bool IsReflexionOK(QPointF A, QPointF B, QPointF mirror, QLineF line);
bool isNotOnWall(QPointF A, QLineF wall);
bool checkLOS(QPointF A, QPointF B, QList<Wall> walls);
qreal dist_point_line(QLineF line, QPointF p);
bool isObtuseAngle(QLineF BA, QLineF BC);

QColor color(qreal power);
const qreal mapWidth =1340, mapHeigth = 640;

//Channel parameters
const qreal frequency = 26*pow(10,9);
const qreal beta = (2*pi*frequency)/(3*pow(10,8));
const qreal he = 3*pow(10,8)/(frequency*pi); // he = lambda/pi = c/(f*pi)
const qreal Ra = 71;
const qreal EIRPmax = 2; //Watts
const qreal Gtx = 16/(3*pi);
const qreal Ptx = EIRPmax/Gtx; //Watts
const qreal Ptx_dBm = 10 * log10(Ptx/0.001);

//Link Budget
const qreal SNR = 8; //dB
const qreal R_noise_fig = 10; //dB
const qreal interference_margin = 6; //dB
const qreal Bmax = 200*pow(10,6); // max bandwidth 200MHz
const qreal thermal_noise = 10*log10(k_boltz*290*Bmax/0.001); //dBm
const qreal loss = (SNR+R_noise_fig+interference_margin+thermal_noise);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << "loss " <<loss;
    scene = new QGraphicsScene(this);
    ui->view->setRenderHints(QPainter::Antialiasing);
    ui->view->setScene(scene);
    ui->view->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    ui->view->scene()->addRect(QRect(0,0,0,0));//add this rectangle to force (0,0)

    QList<Wall> walls, diffr; qreal scaleX; qreal scaleY; //the list walls contains the building walls on which rays are reflected,
                                                           //diffr contains virutal walls useful for diffraction
                                                           // scaleX,Y are for display window fitting purpose
    tie(walls, diffr, scaleX, scaleY) = createStreetMap(mapWidth,mapHeigth); // this function creates a street map and returns the list of walls,
                                                                               // diffr,scaling factors
    scaleY = scaleX;

    //small exemple, see exercise session 3 knife edge - place T and R accordingly
//    Wall wall0(100 ,50 ,100 , 71.5, 0.3, 5);
//    diffr.append(wall0); walls.append(wall0); //the wall of this exemple will be both for reflection and diffraction
//    scaleX = mapWidth/150;
//    scaleY = mapHeigth/150;
//        qDebug() << "Ptx = " <<Ptx;
//    ui->view->scene()->addLine(QLineF(diffr[0].p1().x()*scaleX,
//                                      diffr[0].p1().y()*scaleY,
//                                      diffr[0].p2().x()*scaleX,
//                                      diffr[0].p2().y()*scaleY), diffr[0].outlinePen);


    QPen meshOutline(Qt::black);
    Mesh transmitter(45, 22, 5, 5, 0);
    //Mesh receiver(174, 14, 5 ,5 ,1);
    ui->view->scene()->addRect(QRectF((transmitter.centerX*scaleX)-transmitter.width()/2, (transmitter.centerY*scaleY)-transmitter.height()/2, transmitter.width(), transmitter.height()), meshOutline, transmitter.brush);
    //ui->view->scene()->addRect(QRectF((receiver.centerX*scaleX)-receiver.width()/2, (receiver.centerY*scaleY)-receiver.height()/2, receiver.width(), receiver.height()), meshOutline, receiver.brush);


    qreal Prx = 0, Prx_dBm = 0;
    complex<qreal> sum = 0;
   // qreal mapWidth = 1400; qreal mapHeigth = 700;
    qreal resolution=1;

    QVector<qreal> dist,P_rx_vec; //vectors to make plot
    int idx = 0;
    QPointF T=transmitter.center(), R;

    //launch color map from here, treat each meter square as a receiver antenna, comment this section and uncomment next section to display rays for a specific receiver
    for (int w =0; w<(253/resolution); w++){
        for(int h=0; h<(50/resolution); h++){

            if((w*resolution<80 && h*resolution<20) || (w*resolution>=95 && w*resolution<165 && h*resolution<20) || (w*resolution>=175 && h*resolution<15) || (w*resolution<80 && h*resolution>=30) || (w*resolution>=95 && w*resolution<165 && h*resolution>=30) || (w*resolution>=175 && h*resolution>=30) ){
            //do nothing...
            }
            else{
                Mesh receiver(w*resolution+resolution/2, h*resolution+resolution/2, 1*scaleX, 1*scaleY, 1);
                R = receiver.center();

                sum = imageMethod5GColorMap(transmitter,receiver,walls, diffr);
                Prx = pow(he,2)*60*EIRPmax*pow(abs(sum),2) / (8*Ra); //EQUATION 3.51
                Prx_dBm = 10 * log10( Prx/ 0.001);

                //qDebug() << w*resolution+resolution/2 << " , " << h*resolution+resolution/2 << " : " << Prx_dBm;
                receiver.brush.setColor( color(Prx_dBm));
                ui->view->scene()->addRect(QRectF((receiver.centerX*scaleX)-receiver.width()/2, (receiver.centerY*scaleY)-receiver.height()/2, receiver.width(), receiver.height()), meshOutline, receiver.brush);

                if(QLineF(T,R).length() > 10  && h<30 && h>=20){
                    dist.insert(idx, log10(QLineF(T,R).length()) );
                    qDebug() << dist[idx] << " " << Prx_dBm;
                    P_rx_vec.insert(idx, Prx_dBm);
                    idx++;
                }
            }
        }
    }
    makePathLossPlots(dist, P_rx_vec); //Plot the path loss

//    //DISPLAY RAYS FOR SPECIFIC RECEIVER (uncomment below to make it happen), don't forget to place receiver (line 57)
//    //visualize a ray tracing from here
//    sum = imageMethod5G(transmitter, receiver, walls, diffr, scaleX, scaleY); // computation of the sum in the power computation formula
//    qDebug() << "sum = " << sum.real() << " + i" << sum.imag();
//    qDebug() << "K = " << pow(he,2)*60*Gtx*Ptx/ (8*Ra);
//    Prx = pow(he,2)*60*Gtx*Ptx * pow(abs(sum),2) / (8*Ra); //EQUATION 3.51
//    qDebug() << "Prx = " << Prx;
//    Prx_dBm = 10 * log10( Prx/ 0.001);
//    qDebug() << Prx_dBm << "dBm";


    //DRAW WALLS
    // Rem: drawing walls at the end make them appear more clearly on the final display
    for(int wall =0; wall<walls.size(); wall++){
        ui->view->scene()->addLine(QLineF(walls[wall].p1().x()*scaleX,
                                          walls[wall].p1().y()*scaleY,
                                          walls[wall].p2().x()*scaleX,
                                          walls[wall].p2().y()*scaleY), walls[wall].outlinePen);
        if(wall<diffr.size()){
            ui->view->scene()->addLine(QLineF(diffr[wall].p1().x()*scaleX,
                                              diffr[wall].p1().y()*scaleY,
                                              diffr[wall].p2().x()*scaleX,
                                              diffr[wall].p2().y()*scaleY), diffr[wall].outlinePen);
        }
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

complex<qreal> MainWindow::imageMethod5G(Mesh TX, Mesh RX, QList<Wall> walls, QList<Wall> diffr, qreal scaleX, qreal scaleY){

    QPointF R = RX.center(); QPointF T = TX.center(); qreal d_los = QLineF(T,R).length();
    qreal eps_gnd = 5; //ground permittivity
    complex<qreal> sum=0; // Initialize the sum for total power computation. The 'sum' is the summation where each term
                // is for one ray. Those terms are the product of total reflection coef and total transmission
                // coef, divided by square of distance made by the ray (including reflections). 'sum' is updated
                             // as soon as needed information is gathered to compute a new term.
    qreal d2; //distance of rays, to be updated and reused for each ray


    //STEP 1) start by direct wave if in line of sigth (=LOS) or else compute diffraction
    if(checkLOS(T,R,walls) && checkLOS(T,R,diffr)){
        draw_ray(R,T,0, scaleX, scaleY);
        sum += phase(beta,d_los)/QLineF(R,T).length(); //direct wave

        d2 = sqrt(pow(TX.antennaH+RX.antennaH,2)+pow(d_los,2)); //d2 for reflection on ground eq.3.7
        sum += phase(beta,d2)*abs_reflexion_coef_ground(TX.antennaH, RX.antennaH, d_los, eps_gnd)/d2; //see eq. 3.7 last arg=5 is for ground permitivity
    }

    else{ //if not LOS -> d iffraction
        QPointF inters;
        qreal dr, v, fresnel2,d12,s12;

        //check which diffraction walls are obstacle and take the closest one
        for(int m=0; m<diffr.size(); m++){

            //compute diffraction only if diffraction ray does not cross any obstable, else just move on
            //first check from one side of the wall --> p1(), next if is for the other side of the wall --> p2()
            if(diffr[m].intersect(QLineF(T,R), &inters)==1 && checkLOS(R,diffr[m].p1(),walls) && checkLOS(T,diffr[m].p1(),walls) ){
                draw_ray(T,diffr[m].p1(),0,scaleX,scaleY);
                draw_ray(R,diffr[m].p1(),0,scaleX,scaleY);

                diffr[m].intersect(QLineF(T,R), &inters); //get intersection between TR line and diffraction wall
                d12 = dist_point_line(diffr[m],T)+dist_point_line(diffr[m],R);
                s12 = QLineF(T,diffr[m].p1()).length() + QLineF(R,diffr[m].p1()).length();
                //qDebug() << "d1 = " << QLineF(T,inters).length();
                //qDebug() << "d12 = " << d12;
                //qDebug() << "s12 = " << s12;
                dr = s12 - d12; //see delata_r eq 3.56 of course
                //qDebug() << "dr = " << dr;
                v = sqrt( (2*beta*dr)/pi  ); //see eq 3.57
                fresnel2 = 1 /(( sqrt( pow(v-0.1,2)+1 ) + v-0.1 )*2.2131); //fresnel coef squarred (in not dB!), eq 3.58
                complex<qreal> phase_arg_fresnel( cos(-(pi/4)-(pi/2)*pow(v,2)), -sin(-(pi/4)-(pi/2)*pow(v,2)) );
                //see eq. 3.43
                //qDebug() << "v = "<< v << "; Fv2 = " << fresnel2;
                sum += phase(beta,d_los)*phase_arg_fresnel*sqrt(fresnel2/d_los); // |E| = E*fresnel/sqrt(d) <-> |E| = E * sqrt(fresnel2/d);
            }


            // second check from OTHER side of the wall --> p2()
            if(diffr[m].intersect(QLineF(T,R), &inters)==1 && checkLOS(R,diffr[m].p2(),walls) && checkLOS(T,diffr[m].p2(),walls)){
                draw_ray(T,diffr[m].p2(),0,scaleX,scaleY);
                draw_ray(R,diffr[m].p2(),0,scaleX,scaleY);

                diffr[m].intersect(QLineF(T,R), &inters); //get intersection between TR line and diffraction wall
                d12 = dist_point_line(diffr[m],T)+dist_point_line(diffr[m],R);
                s12 = QLineF(T,diffr[m].p2()).length() + QLineF(R,diffr[m].p2()).length();
                //qDebug() << "d1 = " << QLineF(T,inters).length();
                //qDebug() << "d12 = " << d12;
                //qDebug() << "s12 = " << s12;
                dr = s12 - d12; //see delata_r eq 3.56 of course
                //qDebug() << "dr = " << dr;
                v = sqrt( (2*beta*dr)/pi  ); //see eq 3.57
                fresnel2 = 1 /(( sqrt( pow(v-0.1,2)+1 ) + v-0.1 )*2.2131); //fresnel coef squarred (in not dB!), eq 3.58
                complex<qreal> phase_arg_fresnel( cos(-(pi/4)-(pi/2)*pow(v,2)), -sin(-(pi/4)-(pi/2)*pow(v,2)) );
                //see eq. 3.43
                //qDebug() << "v = "<< v << "; Fv2 = " << fresnel2;
                sum += phase(beta,d_los)*phase_arg_fresnel*sqrt(fresnel2/d_los); // |E| = E*fresnel/sqrt(d) <-> |E| = E * sqrt(fresnel2/d);
            }
        }
    }


    //STEP 2) compute all reflections, coefficients, up to three reflections
    int a,b;
    QPointF r0, r1,r2; QLineF line; QVector<QPointF> m_list;

    // first mirror wrt each wall, and one time reflected rays computations
    for(int i =0; i < walls.size(); i++){

        m_list.insert(0, mirrorPoint(T, walls[i]) );
        if( IsReflexionOK(T,R,m_list[0],walls[i]) ){
            line.setP1(R);line.setP2(m_list[0]); line.intersect(walls[i], &r0);

            if(checkLOS(R,r0,walls) && checkLOS(r0,T,walls)){
                d2 = QLineF(m_list[0],R).length();
                sum  +=   phase(beta,d2) * reflexion_coef(T,r0,walls[i]) / d2; // one reflexion term
                draw_ray(T,r0,1, scaleX, scaleY); draw_ray(R,r0,1, scaleX, scaleY);
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

                    if(a == 1 && checkLOS(R,r1,walls) && checkLOS(r1,r0,walls) && checkLOS(r0,T,walls)){
                        d2 = QLineF(m_list[1],R).length();
                        sum += phase(beta,d2) * reflexion_coef(R,r1, walls[j]) * reflexion_coef(r1,r0,walls[i]) / d2;

                        draw_ray(R,r1,2, scaleX, scaleY); draw_ray(r1,r0,2, scaleX, scaleY); draw_ray(r0,T,2, scaleX, scaleY);
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
                                    && checkLOS(R,r2,walls) && checkLOS(r2,r1,walls) && checkLOS(r1,r0,walls) && checkLOS(r0,T,walls)){
                                d2 = QLineF(m_list[2],R).length();
                                sum += phase(beta,d2) * reflexion_coef(R,r2, walls[k]) * reflexion_coef(r2,r1,walls[j]) * reflexion_coef(r1,r0,walls[i]) / d2;

                                draw_ray(R,r2,3, scaleX, scaleY); draw_ray(r2,r1,3, scaleX, scaleY);draw_ray(r1,r0,3, scaleX, scaleY);
                                draw_ray(r0,T,3, scaleX, scaleY);
                            }
                        }
                }

        }

    }

    return sum;
}


complex<qreal> MainWindow::imageMethod5GColorMap(Mesh TX, Mesh RX, QList<Wall> walls, QList<Wall> diffr){
    // REMARK: ONLY DIFFERENCE FROM imageMethod5G AND imageMethod5GColorMap is that color map does not trace rays, to understand the process
    // read imageMethod5G and comments

    QPointF R = RX.center(); QPointF T = TX.center(); qreal d_los = QLineF(T,R).length();
    qreal eps_gnd = 5; //ground permittivity
    complex<qreal> sum=0; // Initialize the sum for total power computation. The 'sum' is the summation where each term
                // is for one ray. Those terms are the product of total reflection coef and total transmission
                // coef, divided by square of distance made by the ray (including reflections). 'sum' is updated
                             // as soon as needed information is gathered to compute a new term.

    qreal d2; //distance of rays, to be updated and reused for each ray

    if(checkLOS(T,R,walls) && checkLOS(T,R,diffr)){
        sum += phase(beta,d_los)/QLineF(R,T).length(); //direct wave

        d2 = sqrt(pow(TX.antennaH+RX.antennaH,2)+pow(d_los,2)); //d2 for reflection on ground eq.3.7
        sum += phase(beta,d2)*abs_reflexion_coef_ground(TX.antennaH, RX.antennaH, d_los, eps_gnd)/d2; //see eq. 3.7 last arg=5 is for ground permitivity
    }
    else{ //if not LOS -> diffraction
        QPointF inters;
        qreal dr, v, fresnel2,d12,s12;

        //check which diffraction walls are obstacle and take the closest one
        for(int m=0; m<diffr.size(); m++){

            //compute diffraction only if diffraction ray does not cross any obstable, else just move on
            //first check from one side of the wall --> p1(), next if is for the other side of the wall --> p2()
            if(diffr[m].intersect(QLineF(T,R), &inters)==1 && checkLOS(R,diffr[m].p1(),walls) && checkLOS(T,diffr[m].p1(),walls) ){

                diffr[m].intersect(QLineF(T,R), &inters); //get intersection between TR line and diffraction wall
                d12 = dist_point_line(diffr[m],T)+dist_point_line(diffr[m],R);
                s12 = QLineF(T,diffr[m].p1()).length() + QLineF(R,diffr[m].p1()).length();
                //qDebug() << "d1 = " << QLineF(T,inters).length();
                //qDebug() << "d12 = " << d12;
                //qDebug() << "s12 = " << s12;
                dr = s12 - d12; //see delata_r eq 3.56 of course
                //qDebug() << "dr = " << dr;
                v = sqrt( (2*beta*dr)/pi  ); //see eq 3.57
                fresnel2 = 1 /(( sqrt( pow(v-0.1,2)+1 ) + v-0.1 )*2.2131); //fresnel coef squarred (in not dB!), eq 3.58
                complex<qreal> phase_arg_fresnel( cos(-(pi/4)-(pi/2)*pow(v,2)), -sin(-(pi/4)-(pi/2)*pow(v,2)) );
                //see eq. 3.43
                //qDebug() << "v = "<< v << "; Fv2 = " << fresnel2;
                sum += phase(beta,s12)*phase_arg_fresnel*sqrt(fresnel2)/s12; // |E| = E*fresnel/sqrt(d) <-> |E| = E * sqrt(fresnel2/d);
            }


            // second check from OTHER side of the wall --> p2()
            if(diffr[m].intersect(QLineF(T,R), &inters)==1 && checkLOS(R,diffr[m].p2(),walls) && checkLOS(T,diffr[m].p2(),walls)){

                diffr[m].intersect(QLineF(T,R), &inters); //get intersection between TR line and diffraction wall
                d12 = dist_point_line(diffr[m],T)+dist_point_line(diffr[m],R);
                s12 = QLineF(T,diffr[m].p2()).length() + QLineF(R,diffr[m].p2()).length();
                //qDebug() << "d1 = " << QLineF(T,inters).length();
                //qDebug() << "d12 = " << d12;
                //qDebug() << "s12 = " << s12;
                dr = s12 - d12; //see delata_r eq 3.56 of course
                //qDebug() << "dr = " << dr;
                v = sqrt( (2*beta*dr)/pi  ); //see eq 3.57
                fresnel2 = 1 /(( sqrt( pow(v-0.1,2)+1 ) + v-0.1 )*2.2131); //fresnel coef squarred (in not dB!), eq 3.58
                complex<qreal> phase_arg_fresnel( cos(-(pi/4)-(pi/2)*pow(v,2)), -sin(-(pi/4)-(pi/2)*pow(v,2)) );
                //see eq. 3.43
                //qDebug() << "v = "<< v << "; Fv2 = " << fresnel2;
                sum += phase(beta,s12)*phase_arg_fresnel*sqrt(fresnel2)/s12; // |E| = E*fresnel/sqrt(d) <-> |E| = E * sqrt(fresnel2/d);
            }
        }
    }

    int a,b;
    QPointF r0, r1,r2; QLineF line; QVector<QPointF> m_list;

    // first mirror wrt each wall, and one time reflected rays computations
    for(int i =0; i < walls.size(); i++){

        m_list.insert(0, mirrorPoint(T, walls[i]) );
        if( IsReflexionOK(T,R,m_list[0],walls[i]) ){
            line.setP1(R);line.setP2(m_list[0]); line.intersect(walls[i], &r0);

            if(checkLOS(R,r0,walls) && checkLOS(r0,T,walls)){
                d2 = QLineF(m_list[0],R).length();
                sum  +=   phase(beta,d2) * reflexion_coef(T,r0,walls[i]) / d2; // one reflexion term
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

                    if(a == 1 && checkLOS(R,r1,walls) && checkLOS(r1,r0,walls) && checkLOS(r0,T,walls)){
                        d2 = QLineF(m_list[1],R).length();
                        sum += phase(beta,d2) * reflexion_coef(R,r1, walls[j]) * reflexion_coef(r1,r0,walls[i]) / d2;
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
                                    && checkLOS(R,r2,walls) && checkLOS(r2,r1,walls) && checkLOS(r1,r0,walls) && checkLOS(r0,T,walls)){
                                d2 = QLineF(m_list[2],R).length();
                                sum += phase(beta,d2) * reflexion_coef(R,r2, walls[k]) * reflexion_coef(r2,r1,walls[j]) * reflexion_coef(r1,r0,walls[i]) / d2;
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


void MainWindow::draw_ray(QPointF A, QPointF B, int n_refl, qreal scaleX, qreal scaleY){
    // draw the ray on the graphic scene taking into account the number of reflexions for the color
    // allowing to distinguish them

    switch(n_refl){
        case 0:     ui->view->scene()->addLine(QLineF(A.x()*scaleX,A.y()*scaleY, B.x()*scaleX, B.y()*scaleY), QPen(Qt::black)); break;
        case 1:     ui->view->scene()->addLine(QLineF(A.x()*scaleX,A.y()*scaleY, B.x()*scaleX, B.y()*scaleY), QPen(Qt::red)); break;
        case 2:     ui->view->scene()->addLine(QLineF(A.x()*scaleX,A.y()*scaleY, B.x()*scaleX, B.y()*scaleY), QPen(Qt::green)); break;
        case 3:     ui->view->scene()->addLine(QLineF(A.x()*scaleX,A.y()*scaleY, B.x()*scaleX, B.y()*scaleY), QPen(Qt::blue)); break;
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
        return abs_reflexion_coef_wall(thetai, wall.permittivity);
    }
}

bool isNotOnWall(QPointF A, QLineF wall){
    //Check if P belongs to the wall and return true if point is not (check by cross product with wall points)
    qreal cross_p = abs ( (A.x()-wall.p1().x())*(wall.p2().y()-wall.p1().y()) - ((A.y()-wall.p1().y())*(wall.p2().x()-wall.p1().x())) );
    return cross_p > 0.05; // It is unsafe to compare floating points with zero so give arbitrary security margin of 0.05
}

//qreal MainWindow :: transmission_coef(QPointF A, QPointF B, QList<Wall> walls){
//    // Identify and compute the total transmission coef of a ray from A to B through all intersected walls
//    // Note the use of isNotOnWall function to check if considered points are not on wall itself

//    qreal product = 1; //put a first neutral coef equal to one, then update the product when a transmission is found
//    qreal thetai;

//    for(int i=0; i<walls.size(); i++){
//        bool ok = (walls[i].intersect( QLineF(A,B), nullptr)==1) && isNotOnWall(A,walls[i]) && isNotOnWall(B,walls[i]);
//        if(ok){
//            thetai = theta_i( QLineF(A,B), walls[i]);
//            product = product*abs_tot_transmission_coef(thetai, walls[i].permittivity, walls[i].conductivity, walls[i].thickness);

//        }
//        else continue;
//    }
//    return product;
//}

bool checkLOS(QPointF A, QPointF B, QList<Wall> walls){
    // For 5G case, check if computed ray crosses path of a wall. If yes, return false.
    // Function useful to discard all transmission cases.
    QPointF inters;
    for(int i=0; i<walls.size(); i++){
        if( walls[i].intersect( QLineF(A,B), &inters)==1 && isNotOnWall(A,walls[i]) && isNotOnWall(B,walls[i])){
            return false;
        }
    }

    return true;
}

complex<qreal> MainWindow::phase(qreal beta, qreal d){
    //Returns the complex phase term in fucntion of beta and distance
    return complex<qreal>(cos(beta*d),-sin(beta*d));
}

qreal dist_point_line(QLineF line, QPointF p){
    //Computes shortest distance from point to line

    if(line.dx()==0){
        //substract p.x to any line.x since they are all same
        return abs(p.x()-line.x1());
    }
    else if(line.dy()==0){
        //substract p.y to any line.y since they are all same
        return abs(p.y()-line.y1());
    }
    else{
    //line y = a*x+c
    qreal a = line.dx()/line.dy();
    qreal c = line.y1()-a*line.x1();
    return abs(-a*p.x()+p.y()-c)/sqrt(a*a+1);
    }
}

bool isObtuseAngle(QLineF BA, QLineF BC){
    //return if angle made between AB and BC vectors is obtuse. Will be if dot product is negative
    //qDebug() << BA.dx()<< ' ' << BC.dx() << ' ' << BA.dy() << BC.dy();
    return (( (BA.dx())*(BC.dx()) + (BA.dy())*(BC.dy())  ) < 0 );
}

void MainWindow::makePathLossPlots(QVector<qreal> dist_vec, QVector<qreal> Prx_vec){

    // --- FOLLOWING CODE IS LEAST SQURE REGRESSION LINE ALGORITHM FOUND HERE: https://www.bragitoff.com/2015/09/c-program-to-linear-fit-the-data-using-least-squares-method/
    qreal n = dist_vec.size();
    qreal xsum=0,x2sum=0,ysum=0,xysum=0;                //variables for sums/sigma of xi,yi,xi^2,xiyi etc
    for (int i=0; i<n; i++)
    {
        xsum=xsum+dist_vec[i];                        //calculate sigma(xi)
        ysum=ysum+Prx_vec[i];                        //calculate sigma(yi)
        x2sum=x2sum+pow(dist_vec[i],2);                //calculate sigma(x^2i)
        xysum=xysum+dist_vec[i]*Prx_vec[i];                    //calculate sigma(xi*yi)
    }
    qreal a=(n*xysum-xsum*ysum)/(n*x2sum-xsum*xsum);            //calculate slope
    qreal b=(x2sum*ysum-xsum*xysum)/(x2sum*n-xsum*xsum);            //calculate intercept

    QVector<qreal> Prx_fit(n);                        //Prx vector that fits the scattering data
    qreal var_sum = 0;                               //init variance sum, devide by n after the for loop below
    for (int k=0; k<n; k++){
        Prx_fit[k]=a*dist_vec[k]+b;                    //to calculate y(fitted) at given x points
        var_sum += pow(Prx_fit[k] - Prx_vec[k],2);
    }

    qreal std_dev = sqrt(var_sum/n);

    //--- END OF REGRESSIONLINE ALGORITHM --- //
    //Compute standard deviation from Prx_fit has been incorporated in the above fitting algorithm

    qDebug() << "y = " << a << " * x + " << b;
    qDebug() << "std_dev = " << std_dev;

    //    ui->customPlot->legend->setVisible(true);
    //    ui->customPlot->legend->setFont(QFont("Helvetica",9));
    //    // set locale to english, so we get english decimal separator:
    //    ui->customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));

        // add data point graph:
        ui->customPlot->addGraph();
        ui->customPlot->graph(0)->setPen(QPen(Qt::blue));
        ui->customPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
        ui->customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 4));

        ui->customPlot->addGraph();
        ui->customPlot->graph(1)->setPen(QPen(Qt::red));
        //ui->customPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
        //ui->customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 4));


        // pass data to graphs and let QCustomPlot determine the axes ranges so the whole thing is visible:
        ui->customPlot->graph(0)->setData(dist_vec, Prx_vec);
        ui->customPlot->graph(0)->rescaleAxes();
        ui->customPlot->graph(1)->setData(dist_vec, Prx_fit);
        ui->customPlot->graph(1)->rescaleAxes();

        ui->customPlot->axisRect()->setupFullAxesBox();

        makeCellRangePlot(a,b,std_dev);
}

void MainWindow::makeCellRangePlot(qreal slope, qreal intercept, qreal sigma){


    //Compute probability of connection, that is, probability that path loss is bigger than max allowed path loss for connection.
    // P[L(r) < Lm] = P[ L_sigma < ( Lm-avg_L(r) ) ] = 1 - P[L_sigma > Prx_fit-loss] = 1-0.5*erfc(...) --> see page 56 lecture ntoes
    int n=350;
    QVector<qreal> prob(n);
    QVector<qreal> dist_vecr(n); //make a distance vector
    QVector<qreal> Fu(n);
    qreal Prx_av, a, b;
    for(int i=0; i<n; i++){
        dist_vecr[i] = i;
        Prx_av = slope*log10(dist_vecr[i]) + intercept;
        prob[i] = 1 - 0.5 * erfc( (Prx_av - loss)/(sigma*sqrt(2)) );
        a = (Prx_av - loss)/(sqrt(2)*sigma);
        qDebug() << "e = " << exp(1);
        b = abs(slope)*log10(exp(1))/(sqrt(2)*sigma);
        Fu[i] = 1 - 0.5*erfc(a) + 0.5*exp(2*a/b + 1/pow(b,2))*erfc(a+1/b);
    }

    // add data point graph:
    ui->probaPlot->addGraph();
    ui->probaPlot->graph(0)->setPen(QPen(Qt::blue));

    ui->probaPlot->addGraph();
    ui->probaPlot->graph(1)->setPen(QPen(Qt::red));

    // pass data to graphs and let QCustomPlot determine the axes ranges so the whole thing is visible:
    ui->probaPlot->graph(0)->setData(dist_vecr,prob);
    ui->probaPlot->graph(0)->rescaleAxes();

    ui->probaPlot->graph(1)->setData(dist_vecr,Fu);
    ui->probaPlot->graph(1)->rescaleAxes();

    ui->probaPlot->axisRect()->setupFullAxesBox();

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

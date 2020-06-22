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
qreal v_cos_product(QVector<qreal> v, QLineF ray);
complex<qreal> phase_tau(qreal tau);
void printSideStreetsCoverage(QVector<qreal> street1, QVector<qreal> street2, QVector<qreal> street3, QVector<qreal> street4);

QColor color(qreal power);
const qreal mapWidth =1340, mapHeigth = 640;

//Channel parameters
const qreal frequency = 26*pow(10,9);
const qreal c = 3*pow(10,8); // speed of light m/s
const qreal beta = (2*pi*frequency)/c;
const qreal he = c/(frequency*pi); // he = lambda/pi = c/(f*pi)
const qreal Ra = 71;
const qreal EIRPmax = 2; //Watts
const qreal Gtx = 16/(3*pi);
const qreal Ptx = EIRPmax/Gtx; //Watts
const qreal Ptx_dBm = 10 * log10(Ptx/0.001);

//Link Budget
const qreal target_SNR = 8; //dB
const qreal R_noise_fig = 10; //dB
const qreal interference_margin = 6; //dB
const qreal Bmax = 200*pow(10,6); // max bandwidth 200MHz
const qreal thermal_noise = 10*log10(k_boltz*290*Bmax/0.001); //dBm

const qreal min_Prx = (target_SNR+R_noise_fig+interference_margin+thermal_noise);
const qreal receiver_sensitivity = thermal_noise+target_SNR+R_noise_fig;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//    qDebug() << "min_Prx  " <<min_Prx ;
//    qDebug() << "Receiver sensitivity " <<receiver_sensitivity;

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

////    //small exemples;

//    //THESE TWO FOR WALLS AND GROUND REFLECTION FROM REPORT
//    //    Wall wall0(0 ,10, 90  , 10, 0.3, 5);
//    //    Wall wall1(0 ,15 ,90 ,  15, 0.3, 5);

//    //THESE FOR DIFFRACTION EXAMPLE
//        Wall wall0(0 ,10, 90  , 10, 0.3, 5);
//        Wall wall1(90 ,10 ,90 ,  20, 0.3, 5);
//        Wall diff0(90 ,10 ,80 , 20 , 0, 6);

//    diffr.append(diff0);
//    walls.append(wall0); walls.append(wall1); //the wall of this exemple will be both for reflection and diffraction
//    scaleX = mapWidth/150;
//    scaleY = scaleX;


    QPen meshOutline(Qt::black);
    Mesh transmitter(200, 39, 5, 5, 0);
    //Mesh receiver(85, 55, 5 ,5 ,1);
    ui->view->scene()->addRect(QRectF((transmitter.centerX*scaleX)-transmitter.width()/2, (transmitter.centerY*scaleY)-transmitter.height()/2, transmitter.width(), transmitter.height()), meshOutline, transmitter.brush);
    //ui->view->scene()->addRect(QRectF((receiver.centerX*scaleX)-receiver.width()/2, (receiver.centerY*scaleY)-receiver.height()/2, receiver.width(), receiver.height()), meshOutline, receiver.brush);

    qreal Prx = 0, Prx_dBm = 0;
    complex<qreal> sum = 0;
   // qreal mapWidth = 1400; qreal mapHeigth = 700;
    qreal resolution=1;

    QVector<qreal> dist,P_rx_vec; //vectors to make plot
    int idx = 0;
    QPointF T=transmitter.center(), R;

    QVector<qreal> street1_pow,street2_pow,street3_pow,street4_pow; //for side streets depth, each element will contain
                                                                    // average power over a horizontal section
    int x1=0,x2=0,x3=0,x4=0,y1=0,y2=0,y3=0,y4=0;
    qreal pow1=0,pow2=0,pow3=0,pow4=0;

    //launch color map from here, treat each meter square as a receiver antenna, comment this section and uncomment next section to display rays for a specific receiver
    for (int h=0; h<(70/resolution); h++){
        for(int w =0; w<(253/resolution); w++){

            if((w*resolution<80 && h*resolution<29) || (w*resolution>=90 && w*resolution<160 && h*resolution<30) || (w*resolution>=170 && h*resolution<25) || (w*resolution<80 && h*resolution>=40) || (w*resolution>=90 && w*resolution<160 && h*resolution>=41) || (w*resolution>=170 && h*resolution>=40) ){
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

                if(QLineF(T,R).length() > 10  && h*resolution<40 && h*resolution>=30){
                    dist.insert(idx, log10(QLineF(T,R).length()) );
                    qDebug() << dist[idx] << " " << Prx_dBm;
                    P_rx_vec.insert(idx, Prx_dBm);
                    idx++;
                }

                //First street depth
                if( w*resolution >= 80 && w*resolution < 90 && h*resolution < 30 ){
                    pow1 += Prx_dBm;
                    x1++;
                    if(x1 ==9){ street1_pow.insert(y1, pow1/10); } //take average over horizontal section
                }

                //Second street depth
                if( w*resolution >= 160 && w*resolution < 170 && h*resolution < 30 ){
                    pow2 += Prx_dBm;
                    x2++;
                    if(x2 ==9){ street2_pow.insert(y2, pow2/10); } //take average over horizontal section
                }

                //Third street depth
                if( w*resolution >= 80 && w*resolution < 90 && h*resolution > 40 ){
                    pow3 += Prx_dBm;
                    x3++;
                    if(x3 ==9){ street3_pow.insert((y3-41), pow3/10); } //take average over horizontal section
                }

                //Four street depth
                if( w*resolution >= 160 && w*resolution < 170 && h*resolution > 40 ){
                    pow4 += Prx_dBm;
                    x4++;
                    if(x4 ==9){ street4_pow.insert((y4-41), pow4/10); } //take average over horizontal section
                }

            }
        }
        pow1=0; x1=0; y1++;
        pow2=0; x2=0; y2++;
        pow3=0; x3=0; y3++;
        pow4=0; x4=0; y4++;
    }

    makePathLossPlots(dist, P_rx_vec); //Plot the path loss
    printSideStreetsCoverage(street1_pow,street2_pow,street3_pow,street4_pow);



//    //DISPLAY RAYS FOR SPECIFIC RECEIVER (uncomment below to make it happen), don't forget to place receiver (line 57)
//    //visualize a ray tracing from here
//    QVector<qreal> receiver_speed = {0,-13.88}; //meters per second
//    sum = imageMethod5G(transmitter, receiver, walls, diffr, scaleX, scaleY, receiver_speed); // computation of the sum in the power computation formula
//    Prx = pow(he,2)*60*Gtx*Ptx * pow(abs(sum),2) / (8*Ra); //EQUATION 3.51
//    qreal SNR = 10*log10(Prx)-R_noise_fig-thermal_noise; //eq 3.79
//    qDebug() << "SNR = " << SNR << "dB";
//    Prx_dBm = 10 * log10( Prx/ 0.001);
//    qDebug() << "Prx_dBm = " << Prx_dBm << "dBm";

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



/* FIND ALL USEFUL FUNCTIONS BELOW
 * /////////////////////////////////////////////
 *
 * /////////////////////////////////////////////
*/



complex<qreal> MainWindow::imageMethod5G(Mesh TX, Mesh RX, QList<Wall> walls, QList<Wall> diffr, qreal scaleX, qreal scaleY, QVector<qreal> RX_speed_vec){

    QPointF R = RX.center(); QPointF T = TX.center(); qreal d_los = QLineF(T,R).length();
    qreal eps_gnd = 5; //ground permittivity
    complex<qreal> sum=0; // Initialize the sum for total power computation. The 'sum' is the summation where each term
                // is for one ray. Those terms are the product of total reflection coef and total transmission
                // coef, divided by square of distance made by the ray (including reflections). 'sum' is updated
                             // as soon as needed information is gathered to compute a new term.
    qreal d2; //distance of rays, to be updated and reused for each ray

    complex<qreal>alpha_n;

    qreal tau;
    QVector<qreal> tau_vector ;
    QVector<complex<qreal>> alpha_n_vector;

    QVector<qreal> spectrum_freqs;
    QVector<qreal> spectrum_attenuations;
    qreal v_abs = sqrt(pow(RX_speed_vec[0],2)+pow(RX_speed_vec[1],2)); //receiver speed absolute value

    //STEP 1) start by direct wave if in line of sigth (=LOS) or else compute diffraction
    if(checkLOS(T,R,walls) && checkLOS(T,R,diffr)){
        draw_ray(R,T,0, scaleX, scaleY);
       alpha_n = phase(beta,d_los)/QLineF(R,T).length();
        sum +=alpha_n; //direct wave
        tau = d_los/c;
        alpha_n_vector.append(alpha_n);
        tau_vector.append(tau);
        spectrum_freqs.append( 2*pi*beta * v_cos_product(RX_speed_vec, QLineF(T,R) ) );
        spectrum_attenuations.append(abs(alpha_n));

        d2 = sqrt(pow(TX.antennaH+RX.antennaH,2)+pow(d_los,2)); //d2 for reflection on ground eq.3.7
        tau = d2/c;

        qreal th_i = pi - qAtan(d_los/(TX.antennaH+RX.antennaH)); //compute angle of impact on receiver for effective height correction
       alpha_n = phase(beta,d2)*reflexion_coef_ground(TX.antennaH, RX.antennaH, d_los, eps_gnd)*cos((pi/2)*cos(th_i)) / ( d2*pow(sin(th_i),2) );

        sum +=alpha_n; //see eq. 3.7 last arg=5 is for ground permitivity
        alpha_n_vector.append(alpha_n); // Impulse response
        tau_vector.append(tau);

        qreal signV = v_cos_product(RX_speed_vec, QLineF(T,R)); //get sign of the frequency shift that is the sane than for the direct wave
        if(signV<0){
            spectrum_freqs.append( -1*2*pi*beta * v_abs * cos( atan(RX.antennaH/(d_los/2) )) );
        }
        else{
            spectrum_freqs.append( 2*pi*beta * v_abs * cos( atan(RX.antennaH/(d_los/2) )) );
        }
        spectrum_attenuations.append(abs(alpha_n));
    }

    else{ //if not LOS -> d iffraction
        QPointF inters;
        qreal dr, v, fresnel2_dB,fresnel,d12,s12;

        //check which diffraction walls are obstacle and take the closest one
        for(int m=0; m<diffr.size(); m++){

            //compute diffraction only if diffraction ray does not cross any obstable, else just move on
            //first check from one side of the wall --> p1(), next if is for the other side of the wall --> p2()
            if(diffr[m].intersect(QLineF(T,R), &inters)==1 && checkLOS(R,diffr[m].p1(),walls) && checkLOS(T,diffr[m].p1(),walls) ){
                draw_ray(T,diffr[m].p1(),0,scaleX,scaleY);
                draw_ray(R,diffr[m].p1(),0,scaleX,scaleY);

                diffr[m].intersect(QLineF(T,R), &inters); //get intersection between TR line and diffraction wall
                d12 = d_los;
                s12 = QLineF(T,diffr[m].p1()).length() + QLineF(R,diffr[m].p1()).length(); //real path taken by wave

                dr = s12 - d12; //see delata_r eq 3.56 of course

                v = sqrt( (2*beta*dr)/pi  ); //see eq 3.57
                fresnel2_dB = -6.9-20*log10(sqrt( pow(v-0.1,2)+1 ) + v-0.1 ); //, eq 3.58
                fresnel = pow(10,fresnel2_dB/20); //fresnel coef  (in not dB!)
                complex<qreal> phase_arg_fresnel( cos(-(pi/4)-(pi/2)*pow(v,2)), sin(-(pi/4)-(pi/2)*pow(v,2)) );

               alpha_n = phase(beta,s12)*phase_arg_fresnel*fresnel/s12;
                sum +=alpha_n;
                tau = s12/c;
                alpha_n_vector.append(alpha_n);
                tau_vector.append(tau);

                spectrum_freqs.append( 2*pi*beta * v_cos_product(RX_speed_vec, QLineF(diffr[m].p1(),R) ) );
                spectrum_attenuations.append(abs(alpha_n));
            }


            // second check from OTHER side of the wall --> p2()
            if(diffr[m].intersect(QLineF(T,R), &inters)==1 && checkLOS(R,diffr[m].p2(),walls) && checkLOS(T,diffr[m].p2(),walls)){
                draw_ray(T,diffr[m].p2(),0,scaleX,scaleY);
                draw_ray(R,diffr[m].p2(),0,scaleX,scaleY);

                diffr[m].intersect(QLineF(T,R), &inters); //get intersection between TR line and diffraction wall
                d12 = d_los;
                s12 = QLineF(T,diffr[m].p2()).length() + QLineF(R,diffr[m].p2()).length(); //real path taken by wave

                dr = s12 - d12; //see delata_r eq 3.56 of course

                v = sqrt( (2*beta*dr)/pi  ); //see eq 3.57
                fresnel2_dB = -6.9-20*log10(sqrt( pow(v-0.1,2)+1 ) + v-0.1 ); //, eq 3.58
                fresnel = pow(10,fresnel2_dB/20); //fresnel coef  (in not dB!)
                complex<qreal> phase_arg_fresnel( cos(-(pi/4)-(pi/2)*pow(v,2)), sin(-(pi/4)-(pi/2)*pow(v,2)) );

               alpha_n = phase(beta,s12)*phase_arg_fresnel*fresnel/s12;
                sum +=alpha_n;
                tau = s12/c;
                alpha_n_vector.append(alpha_n);
                tau_vector.append(tau);

                spectrum_freqs.append( 2*pi*beta * v_cos_product(RX_speed_vec, QLineF(diffr[m].p2(),R) ) );
                spectrum_attenuations.append(abs(alpha_n));
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
               alpha_n = phase(beta,d2) * reflexion_coef(T,r0,walls[i]) / d2;
                sum +=alpha_n; // one reflexion term
                draw_ray(T,r0,1, scaleX, scaleY); draw_ray(R,r0,1, scaleX, scaleY);

                tau = d2/c;
                alpha_n_vector.append(alpha_n); /////////////////////////////////////////
                tau_vector.append(tau); ////////////////////////////////////////

                spectrum_freqs.append( 2*pi*beta * v_cos_product(RX_speed_vec, QLineF(r0,R) ) );
                spectrum_attenuations.append(abs(alpha_n));
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
                       alpha_n = phase(beta,d2) * reflexion_coef(R,r1, walls[j]) * reflexion_coef(r1,r0,walls[i]) / d2;
                        sum +=alpha_n;
                        draw_ray(R,r1,2, scaleX, scaleY); draw_ray(r1,r0,2, scaleX, scaleY); draw_ray(r0,T,2, scaleX, scaleY);

                        tau = d2/c;
                        alpha_n_vector.append(alpha_n); /////////////////////////////////////////
                        tau_vector.append(tau); ////////////////////////////////////////

                        spectrum_freqs.append( 2*pi*beta * v_cos_product(RX_speed_vec, QLineF(r1,R) ) );
                        spectrum_attenuations.append(abs(alpha_n));
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
                               alpha_n = phase(beta,d2) * reflexion_coef(R,r2, walls[k]) * reflexion_coef(r2,r1,walls[j]) * reflexion_coef(r1,r0,walls[i]) / d2;
                                sum +=alpha_n;

                                tau = d2/c;
                                alpha_n_vector.append(alpha_n); /////////////////////////////////////////
                                tau_vector.append(tau); ////////////////////////////////////////

                                draw_ray(R,r2,3, scaleX, scaleY); draw_ray(r2,r1,3, scaleX, scaleY);draw_ray(r1,r0,3, scaleX, scaleY);
                                draw_ray(r0,T,3, scaleX, scaleY);
                                spectrum_freqs.append( 2*pi*beta * v_cos_product(RX_speed_vec, QLineF(r2,R) ) );
                                spectrum_attenuations.append(abs(alpha_n));
                            }
                        }
                }

        }

    }

    makeImpulseResponse(tau_vector, alpha_n_vector);
    makeTDLImpulseResponses(tau_vector, alpha_n_vector);
    makeSpectrumPlot(spectrum_freqs, spectrum_attenuations, v_abs);

    return sum;
}


complex<qreal> MainWindow::imageMethod5GColorMap(Mesh TX, Mesh RX, QList<Wall> walls, QList<Wall> diffr){
    // REMARK: ONLY DIFFERENCE FROM imageMethod5G AND imageMethod5GColorMap is that color map does not trace rays and does not
    // make vectors intended to plot impulse responses. To understand the process steps in details, read imageMethod5G and comments

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
        qreal th_i = pi - qAtan(d_los/(TX.antennaH+RX.antennaH)); //compute angle of impact on receiver for effective height correction
        sum += phase(beta,d2)*reflexion_coef_ground(TX.antennaH, RX.antennaH, d_los, eps_gnd)*cos((pi/2)*cos(th_i)) / ( d2*pow(sin(th_i),2) ); //see eq. 3.22
                                                                                    //last eps_gnd is for ground permitivity
    }
    else{ //if not LOS -> diffraction
        QPointF inters;
        qreal dr, v, fresnel2_dB, fresnel,d12,s12;

        //check which diffraction walls are obstacle and take the closest one
        for(int m=0; m<diffr.size(); m++){

            //compute diffraction only if diffraction ray does not cross any obstable, else just move on
            //first check from one side of the wall --> p1(), next if is for the other side of the wall --> p2()
            if(diffr[m].intersect(QLineF(T,R), &inters)==1 && checkLOS(R,diffr[m].p1(),walls) && checkLOS(T,diffr[m].p1(),walls) ){

                diffr[m].intersect(QLineF(T,R), &inters); //get intersection between TR line and diffraction wall
                d12 = d_los;
                s12 = QLineF(T,diffr[m].p1()).length() + QLineF(R,diffr[m].p1()).length(); //real path taken by wave

                dr = s12 - d12; //see delata_r eq 3.56 of course

                v = sqrt( (2*beta*dr)/pi  ); //see eq 3.57
                fresnel2_dB = -6.9-20*log10(sqrt( pow(v-0.1,2)+1 ) + v-0.1 ); //, eq 3.58
                fresnel = pow(10,fresnel2_dB/20); //fresnel coef  (in not dB!)
                complex<qreal> phase_arg_fresnel( cos(-(pi/4)-(pi/2)*pow(v,2)), sin(-(pi/4)-(pi/2)*pow(v,2)) );

                sum += phase(beta,s12)*phase_arg_fresnel*fresnel/s12;
            }


            // second check from OTHER side of the wall --> p2()
            if(diffr[m].intersect(QLineF(T,R), &inters)==1 && checkLOS(R,diffr[m].p2(),walls) && checkLOS(T,diffr[m].p2(),walls)){

                diffr[m].intersect(QLineF(T,R), &inters); //get intersection between TR line and diffraction wall
                d12 = d_los;
                s12 = QLineF(T,diffr[m].p2()).length() + QLineF(R,diffr[m].p2()).length(); //real path taken by wave

                dr = s12 - d12; //see delata_r eq 3.56 of course

                v = sqrt( (2*beta*dr)/pi  ); //see eq 3.57
                fresnel2_dB = -6.9-20*log10(sqrt( pow(v-0.1,2)+1 ) + v-0.1 ); //, eq 3.58
                fresnel = pow(10,fresnel2_dB/20); //fresnel coef  (in not dB!)
                complex<qreal> phase_arg_fresnel( cos(-(pi/4)-(pi/2)*pow(v,2)), sin(-(pi/4)-(pi/2)*pow(v,2)) );

                sum += phase(beta,s12)*phase_arg_fresnel*fresnel/s12;
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
        return reflexion_coef_wall(thetai, wall.permittivity);
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

qreal v_cos_product(QVector<qreal> v, QLineF ray){
    //Computes the v*cos(theta) product of equation 4.21 of the lecture notes by doing dot product v.ray and isolating v*cos(theta)
    //NOTE that the cos of the dot product is the opposite of the cos desired for eq. 4.21
    return - (v[0]*ray.dx()+v[1]*ray.dy()) / ray.length();
}

void printSideStreetsCoverage(QVector<qreal> street1, QVector<qreal> street2, QVector<qreal> street3, QVector<qreal> street4){

    qreal depth=0;
    for(int d = street1.size()-1; d>=0; d--){
        if(street1[d]<min_Prx ){
            depth = 30-d;
            qDebug() << "Street n°1 depth = " << depth-1 << "m";
            break;
        }
    }

    for(int d = street2.size()-1; d>=0; d--){
        if(street2[d]<min_Prx ){
            depth = 30-d;
            qDebug() << "Street n°2 depth = " << depth-1 << "m";
            break;
        }
    }

    for(int d = 0; d<street3.size(); d++){
        if(street3[d]<min_Prx ){
            depth = d;
            qDebug() << "Street n°3 depth = " << depth-1 << "m";
            break;
        }
    }

    for(int d = 0; d<street4.size(); d++){
        if(street4[d]<min_Prx ){
            depth = d;
            qDebug() << "Street n°4 depth = " << depth-1 << "m";
            break;
        }
    }

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

        // set title of plot:
        ui->customPlot->plotLayout()->insertRow(0);
        ui->customPlot->plotLayout()->addElement(0, 0, new QCPTextElement(ui->customPlot, "Received power", QFont("sans", 12, QFont::Bold)));

        //LEGEND
        ui->customPlot->legend->setVisible(true);
        ui->customPlot->legend->setFont(QFont("Helvetica",12));
        // set locale to english, so we get english decimal separator:
        ui->customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));

        // add data point graph:
        QPen penRed(Qt::red);
        penRed.setWidth(2);
        ui->customPlot->addGraph();
        ui->customPlot->graph(0)->setPen(QPen(Qt::blue));
        ui->customPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
        ui->customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 4));
        ui->customPlot->graph(0)->setName("Received power");

        ui->customPlot->addGraph();
        ui->customPlot->graph(1)->setPen(penRed);
        ui->customPlot->graph(1)->setName("Received power (linear regression)");

        // pass data to graphs and let QCustomPlot determine the axes ranges so the whole thing is visible:
        ui->customPlot->graph(0)->setData(dist_vec, Prx_vec);
        //ui->customPlot->graph(0)->rescaleAxes();
        ui->customPlot->graph(1)->setData(dist_vec, Prx_fit);
        ui->customPlot->graph(1)->rescaleAxes();

        QFont pfont("Newyork",12);
        pfont.setStyleHint(QFont::SansSerif);
        pfont.setPointSize(12);

        ui->customPlot->xAxis->setLabel("log(d)");
        ui->customPlot->yAxis->setLabel("Prx_dBm");
        ui->customPlot->xAxis->setLabelFont(pfont);
        ui->customPlot->yAxis->setLabelFont(pfont);
        ui->customPlot->xAxis->setRange(1, 2.5);
        ui->customPlot->yAxis->setRange(-40, -100);

        ui->customPlot->axisRect()->setupFullAxesBox();

        ui->customPlot->setInteraction(QCP::iRangeDrag, true);
        ui->customPlot->setInteraction(QCP::iRangeZoom, true);

        makeCellRangePlot(a,b,std_dev);
}

void MainWindow::makeCellRangePlot(qreal slope, qreal intercept, qreal sigma){


    //Compute probability of connection, that is, probability that path loss is bigger than max allowed path loss for connection.
    // P[L(r) < Lm] = P[ L_sigma < ( Lm-avg_L(r) ) ] = 1 - P[L_sigma > Prx_fit-loss] = 1-0.5*erfc(...) --> see page 56 lecture ntoes
    int n=350;
    QVector<qreal> prob(n);
    QVector<qreal> margin(n);
    QVector<qreal> dist_vecr(n); //make a distance vector
    QVector<qreal> Fu(n);
    qreal Prx_av, a, b;
    for(int i=0; i<n; i++){
        dist_vecr[i] = i;
        Prx_av = slope*log10(dist_vecr[i]) + intercept;
        prob[i] = 1 - 0.5 * erfc( (Prx_av - min_Prx)/(sigma*sqrt(2)) );
        margin[i] = Prx_av - min_Prx;
        a = (Prx_av - min_Prx)/(sqrt(2)*sigma);
        b = abs(slope)*log10(exp(1))/(sqrt(2)*sigma);
        Fu[i] = 1 - 0.5*erfc(a) + 0.5*exp(2*a/b + 1/pow(b,2))*erfc(a+1/b);
    }

    // set title of plot:
    ui->probaPlot->plotLayout()->insertRow(0);
    ui->probaPlot->plotLayout()->addElement(0, 0, new QCPTextElement(ui->probaPlot, "Connection probability", QFont("sans", 12, QFont::Bold)));

    ui->wholeCellPlot->plotLayout()->insertRow(0);
    ui->wholeCellPlot->plotLayout()->addElement(0, 0, new QCPTextElement(ui->wholeCellPlot, "Whole cell coverage - fade margin", QFont("sans", 12, QFont::Bold)));

    //LEGEND
    ui->probaPlot->legend->setVisible(true);
    ui->probaPlot->legend->setFont(QFont("Helvetica",12));
    // set locale to english, so we get english decimal separator:
    ui->probaPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));

    // add data point graph:
    ui->probaPlot->addGraph();
    ui->probaPlot->graph(0)->setPen(QPen(Qt::blue));
    ui->probaPlot->graph(0)->setName("Probability of connection");

    ui->probaPlot->addGraph();
    ui->probaPlot->graph(1)->setPen(QPen(Qt::red));
    ui->probaPlot->graph(1)->setName("Percentage of covered area Fu");

    ui->wholeCellPlot->addGraph();
    ui->wholeCellPlot->graph(0)->setPen(QPen(Qt::red));
    ui->wholeCellPlot->graph(0)->setName("Percentage of covered area Fu");

    // pass data to graphs and let QCustomPlot determine the axes ranges so the whole thing is visible:
    ui->probaPlot->graph(0)->setData(dist_vecr,prob);
    ui->probaPlot->graph(0)->rescaleAxes();

    ui->probaPlot->graph(1)->setData(dist_vecr,Fu);
    ui->probaPlot->graph(1)->rescaleAxes();

    ui->wholeCellPlot->graph(0)->setData(margin,Fu);
    ui->wholeCellPlot->graph(0)->rescaleAxes();

    QFont pfont("Newyork",12);
    pfont.setStyleHint(QFont::SansSerif);
    pfont.setPointSize(12);

    ui->probaPlot->yAxis2->setVisible(true);
    ui->probaPlot->xAxis->setLabelFont(pfont);
    ui->probaPlot->yAxis->setLabelFont(pfont);
    ui->probaPlot->yAxis2->setLabelFont(pfont);

    ui->probaPlot->yAxis->setLabel("P (blue)");
    ui->probaPlot->yAxis2->setLabel("Fu (red)");
    ui->probaPlot->xAxis->setLabel("Distance from base station (m)");

    ui->wholeCellPlot->xAxis->setLabel("Lm - L(R) [dBm]");
    ui->wholeCellPlot->yAxis->setLabel("Fu");
    //ui->wholeCellPlot->yAxis->setRange(0,1);

    ui->probaPlot->axisRect()->setupFullAxesBox();

}

void MainWindow::makeSpectrumPlot(QVector<qreal> freqs, QVector<qreal> attenuations, qreal speed){

//    QVector<qreal> freq2carrier; //not working great with the x axis values display...
//    for(int i=0; i< freqs.size(); i++){
//        freq2carrier.insert(i,freqs[i]+frequency);
//    }

    qreal den_rice = 0;
    for(int k=1; k<attenuations.size(); k++){
        den_rice += pow(attenuations[k],2);
    }

    qreal rice_factor = pow(attenuations[0],2)/den_rice;
    qreal omega_m = beta*speed; // max Doppler shift eq. 4.23
    qreal doppler_spread = omega_m/(2*pi); //in text right above eq. 4.24
    qreal coherence_time = 0.5*c/(frequency*speed); //eq. 4.25

    qDebug() << "Rice factor K = " << rice_factor;
    qDebug() << "Max Doppler shift = " << omega_m*2*pi <<"Hz";
    qDebug() << "Coherence time = " << coherence_time << "s";

    //title
    ui->spectrumPlot->plotLayout()->insertRow(0);
    ui->spectrumPlot->plotLayout()->addElement(0, 0, new QCPTextElement(ui->spectrumPlot, "Doppler spectrum", QFont("sans", 12, QFont::Bold)));

    // add data point graph:
    ui->spectrumPlot->addGraph();
    ui->spectrumPlot->graph()->setPen(QPen(Qt::blue));
    ui->spectrumPlot->graph()->setLineStyle(QCPGraph::lsImpulse);


    // pass data to graphs and let QCustomPlot determine the axes ranges so the whole thing is visible:
    ui->spectrumPlot->graph(0)->setData(freqs,attenuations);
    ui->spectrumPlot->graph(0)->rescaleAxes();
    //ui->spectrumPlot->xAxis->scaleRange(1.3, ui->spectrumPlot->xAxis->range().center());

    QFont pfont("Newyork",6);
    pfont.setStyleHint(QFont::SansSerif);
    pfont.setPointSize(9);
    ui->spectrumPlot->xAxis->setLabelFont(pfont);
    ui->spectrumPlot->yAxis->setLabelFont(pfont);
    ui->spectrumPlot->xAxis->setLabel("Frequency offset from carrier (Hz)");
    ui->spectrumPlot->yAxis->setLabel("| h |");

    ui->spectrumPlot->setInteraction(QCP::iRangeDrag, true);
    ui->spectrumPlot->setInteraction(QCP::iRangeZoom, true);

    ui->spectrumPlot->axisRect()->setupFullAxesBox();
}

void MainWindow:: makeImpulseResponse(QVector<qreal> tau_vec, QVector<complex<qreal>> h_vec){

    QVector<qreal> abs_h;
    for(int i=0; i<h_vec.size();i++){
        abs_h.append(abs(h_vec[i]));
    }

    //titles
    ui->impulsePlot->plotLayout()->insertRow(0);
    ui->impulsePlot->plotLayout()->addElement(0, 0, new QCPTextElement(ui->impulsePlot, "Physical vs TDL impulse responses", QFont("sans", 12, QFont::Bold)));

    ui->USimpulsePlot->plotLayout()->insertRow(0);
    ui->USimpulsePlot->plotLayout()->addElement(0, 0, new QCPTextElement(ui->USimpulsePlot, "Physical vs Uncorrelated scattering TDL impulse responses", QFont("sans", 12, QFont::Bold)));

    //LEGEND
    ui->impulsePlot->legend->setVisible(true);
    ui->impulsePlot->legend->setFont(QFont("Helvetica",12));
    // set locale to english, so we get english decimal separator:
    ui->impulsePlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));
    ////////////////////////////////////////
    ui->USimpulsePlot->legend->setVisible(true);
    ui->USimpulsePlot->legend->setFont(QFont("Helvetica",12));
    // set locale to english, so we get english decimal separator:
    ui->USimpulsePlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));


    QFont pfont("Newyork",6);
    pfont.setStyleHint(QFont::SansSerif);
    pfont.setPointSize(9);
    ui->impulsePlot->xAxis->setLabelFont(pfont);
    ui->impulsePlot->yAxis->setLabelFont(pfont);
    ui->impulsePlot->xAxis->setLabel("Delay (s)");
    ui->impulsePlot->yAxis->setLabel("| h |");
    ui->USimpulsePlot->xAxis->setLabelFont(pfont);
    ui->USimpulsePlot->yAxis->setLabelFont(pfont);
    ui->USimpulsePlot->xAxis->setLabel("Delay (s)");
    ui->USimpulsePlot->yAxis->setLabel("| h |");


    // add data point graph:
    ui->impulsePlot->addGraph();
    ui->impulsePlot->graph()->setPen(QPen(Qt::blue));
    ui->impulsePlot->graph()->setLineStyle(QCPGraph::lsImpulse);
    ui->impulsePlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    //////////////////////////// do same for other graph
    ui->USimpulsePlot->addGraph();
    ui->USimpulsePlot->graph()->setPen(QPen(Qt::blue));
    ui->USimpulsePlot->graph()->setLineStyle(QCPGraph::lsImpulse);
    ui->USimpulsePlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

    // pass data to graphs and let QCustomPlot determine the axes ranges so the whole thing is visible:
    ui->impulsePlot->graph(0)->setData(tau_vec, abs_h);
    ui->impulsePlot->graph(0)->rescaleAxes();
    ui->impulsePlot->graph(0)->setName("Physical impulse response");
    //ui->spectrumPlot->xAxis->scaleRange(1.3, ui->spectrumPlot->xAxis->range().center());
    ////////////////////////////do same for other graph
    ui->USimpulsePlot->graph(0)->setData(tau_vec, abs_h);
    ui->USimpulsePlot->graph(0)->rescaleAxes();
    ui->USimpulsePlot->graph(0)->setName("Physical impulse response");


    ui->impulsePlot->setInteraction(QCP::iRangeDrag, true);
    ui->impulsePlot->setInteraction(QCP::iRangeZoom, true);
    ///////////
    ui->USimpulsePlot->setInteraction(QCP::iRangeDrag, true);
    ui->USimpulsePlot->setInteraction(QCP::iRangeZoom, true);

    ui->impulsePlot->axisRect()->setupFullAxesBox();
    ui->USimpulsePlot->axisRect()->setupFullAxesBox();
}

void MainWindow:: makeTDLImpulseResponses(QVector<qreal> tau_vec, QVector<complex<qreal>> alpha_vec){

    //Find tau max
    qreal tau_max = 0, tau_min=0;
    for(int i=0;i<tau_vec.size();i++){
        if(tau_vec[i]>tau_max){
            tau_max = tau_vec[i];
        }
    }
    tau_min = tau_max;
    for(int i=0;i<tau_vec.size();i++){
        if(tau_vec[i]<tau_min){
            tau_min = tau_vec[i];
        }
    }

    qDebug() << "Delay spread = " << tau_max-tau_min << "s";
    qDebug() << "Coherence bandwidth = " << 1/(tau_max-tau_min) << "Hz";

    // TDL IMPULSE RESPONSE Process data and make two vectors delta_tau_vec and h_TDL to plot equation 1.20 by mean of 1.22
    QVector<qreal> h_TDL, h_USTDL, delta_tau_vec;
    qreal B = 200*pow(10,6); //bvandwitdh
    qreal delta_tau = 1/(2*B);
    qreal L = ceil(tau_max/delta_tau);

    complex<qreal> h_l = 0, h_l_US=0;
    qreal x = 0; //will be used for x = 2*B*(taun_n-l*delta_l) of eq 1.22, then sinc(x) = sin(pi*x)/(pi*x)
    for(int l=1; l <= L; l++){
        for(int n=0; n<tau_vec.size();n++){

            // h_TDL computations
            x = 2*B*(tau_vec[n]-l*delta_tau);
            h_l += alpha_vec[n]*sin(pi*x)/(pi*x);

            // h_USTDL computations
            if(tau_vec[n] >= delta_tau*(l-1) && tau_vec[n] < delta_tau*l){
                h_l_US += alpha_vec[n];
            }
        }

        h_TDL.append(abs(h_l));
        h_USTDL.append(abs(h_l_US));
        delta_tau_vec.append(l*delta_tau);
        h_l=0; h_l_US=0; //reinit h_l for next step
    }

    // UNCORRELATED SCATTERING TDL IMPULSE RESP

    // add data point graph for h_TDL:
    ui->impulsePlot->addGraph();
    ui->impulsePlot->graph()->setPen(QPen(Qt::red));
    ui->impulsePlot->graph()->setLineStyle(QCPGraph::lsImpulse);
    ui->impulsePlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    // add data point graph for h_USTDL:
    ui->USimpulsePlot->addGraph();
    ui->USimpulsePlot->graph()->setPen(QPen(Qt::green));
    ui->USimpulsePlot->graph()->setLineStyle(QCPGraph::lsImpulse);
    ui->USimpulsePlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

    // pass data to graphs and let QCustomPlot determine the axes ranges so the whole thing is visible:
    ui->impulsePlot->graph(1)->setData(delta_tau_vec, h_TDL);
    ui->impulsePlot->graph(1)->rescaleAxes();
    ui->impulsePlot->graph(1)->setName("TDL impulse response");
    //ui->spectrumPlot->xAxis->scaleRange(1.3, ui->spectrumPlot->xAxis->range().center());
    //// again for us tdl
    ui->USimpulsePlot->graph(1)->setData(delta_tau_vec, h_USTDL);
    ui->USimpulsePlot->graph(1)->rescaleAxes();
    ui->USimpulsePlot->graph(1)->setName("Uncorrelated scattering TDL response");
    //ui->spectrumPlot->xAxis->scaleRange(1.3, ui->spectrumPlot->xAxis->range().center());

    ui->impulsePlot->setInteraction(QCP::iRangeDrag, true);
    ui->impulsePlot->setInteraction(QCP::iRangeZoom, true);
    ui->USimpulsePlot->setInteraction(QCP::iRangeDrag, true);
    ui->USimpulsePlot->setInteraction(QCP::iRangeZoom, true);

    ui->impulsePlot->axisRect()->setupFullAxesBox();
    ui->USimpulsePlot->axisRect()->setupFullAxesBox();
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
}

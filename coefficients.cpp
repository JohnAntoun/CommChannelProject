/*
Here are regrouped all functions needed to calculate coefficients of reflexion and transmission, called wh
*/
#include "coefficients.h"

// Useful constants
#define pi 3.14159

const qreal mu_0 = 4*pi*pow(10,-7); // Vacuum permeability
const qreal eps_0 = 8.854*pow(10, -12); // Vacuum permittivity
const qreal omega = (2.45*pow(10,9))*2*pi; // communication frequency converted in rad/s
const qreal beta = (2.45*pow(10,9))*2*pi / (3*pow(10,8)); // (omega/c)= wavenumber in rad/m

qreal theta_i(QLineF ray, QLineF wall){
    // Returns the incident angle between a wall and a given ray by calculating the dot product
    // between their respectiv vectors

    qreal cos_theta = (wall.dx()*ray.dx() + wall.dy()*ray.dy()) /  (wall.length()*ray.length());
    //qreal  cos_theta = QVector2D::dotProduct(v_wall,v_ray) / (v_wall.length()*v_ray.length());

    if( cos_theta <= 0) {return  qAcos(cos_theta) - (pi/2);}
    else                {return (pi/2) - qAcos( cos_theta);}

}

//complex<qreal> z_2(qreal eps_rel, qreal conduct){
//    // Returns the characteristic impedance of wall material
//    complex<qreal> eps_tild(eps_0*eps_rel, -(conduct/omega));

//    return sqrt( mu_0 / eps_tild );
//}

//complex<qreal> gamma_m(qreal eps_rel, qreal sigma){
//    // Returns complex propagation constant of wall 'm'
//    // gamma_m = alpha + j*beta

//    qreal a = omega*sqrt(mu_0*eps_0*eps_rel/2);
//    qreal b = sqrt(1 + pow( sigma/(omega*eps_0*eps_rel), 2));

//    complex<qreal> gamma( (a * sqrt(b - 1)), (a * sqrt(b + 1)));

//    return gamma;
//}

//qreal theta_t_snell(qreal theta_i, qreal eps1, qreal eps_wall){
//    // Returns angle of transmission, calculated by Snell formula. Angle of transmission is needed to determine reflexion/transmission coefficients
//    // in perpendicular polarisation

//    return asin( sqrt(eps1/eps_wall)*sin(theta_i) );
//}

//qreal abs_tot_reflexion_coef(qreal theta_i, qreal eps_rel, qreal sigma, qreal thickness){

//    qreal theta_t = theta_t_snell(theta_i, eps_0, eps_rel*eps_0); // angle of transmission
//    qreal s = thickness/cos(theta_t); // distance of propagation in wall (one way)

//    qreal z1 = 377; //free space Characteritic impedance
//    complex<qreal> z2 = z_2(eps_rel, sigma); // characteristic impedance of wall material (COMPLEX)

//    complex<qreal> gamma = gamma_m(eps_rel, sigma); //constant of propagation (COMPLEX)

//    complex<qreal> perp_coef = (z2*cos(theta_i) - z1*cos(theta_t)) / (z2*cos(theta_i) + z1*cos(theta_t)); //reflexion coefficient perpendicular polarisation

//    complex<qreal> beta_2sin_sin(0 ,beta*2*s*sin(theta_t)*sin(theta_i) ); //used right below
//    complex<qreal> exp_part = exp(-gamma*s*2.0) * exp(beta_2sin_sin);  //exponential part of the total reflexion coefficient (see formula)

//    return abs( perp_coef + (1.0-pow(perp_coef,2)) * (perp_coef*exp_part/(1.0-pow(perp_coef,2)*exp_part))  );
//}

//qreal abs_tot_transmission_coef(qreal theta_i, qreal eps_rel, qreal sigma, qreal thickness){

//    qreal theta_t = theta_t_snell(theta_i, eps_0, eps_rel*eps_0); // angle of transmission
//    qreal s = thickness/cos(theta_t); // distance of propagation in wall (one way)

//    qreal z1 = 377; //free space Characteritic impedance
//    complex<qreal> z2 = z_2(eps_rel, sigma); // characteristic impedance of wall material (COMPLEX)

//    complex<qreal> gamma = gamma_m(eps_rel, sigma); //constant of propagation (COMPLEX)

//    complex<qreal> perp_coef = (z2*cos(theta_i) - z1*cos(theta_t)) / (z2*cos(theta_i) + z1*cos(theta_t)); //reflexion coefficient perpendicular polarisation

//    complex<qreal> beta_2sin_sin(0 ,beta*2*s*sin(theta_t)*sin(theta_i) ); //used right below
//    complex<qreal> exp_part = exp(-gamma*s*2.0) * exp(beta_2sin_sin);  //exponential part of the total reflexion coefficient (see formula)

//    return abs( ((1.0-pow(perp_coef,2))*exp(-s*gamma)) / ((1.0-pow(perp_coef,2)*exp_part)));
//}

qreal abs_reflexion_coef_wall(qreal theta_i, qreal eps_rel){
    //Compute reflexion coefficient, perpendicular case, for reflexions on walls (3.54)

    qreal a = sqrt(eps_rel)*sqrt( 1-(pow(sin(theta_i),2)/eps_rel) );
    return abs( ( cos(theta_i)- a ) / ( cos(theta_i)+a )  );
}

qreal abs_reflexion_coef_ground(qreal TX_h, qreal RX_h, qreal d, qreal eps_rel){
    //Compute reflexion coefficient, parallel case, for reflexions on ground (3.54)
    qreal theta_i = (pi/2) - qAtan((TX_h+RX_h)/d);
    qreal a = sqrt(1/eps_rel)*sqrt( 1-(pow(sin(theta_i),2)/eps_rel));
    return abs( ( cos(theta_i)- a ) / ( cos(theta_i)+a ) );
}

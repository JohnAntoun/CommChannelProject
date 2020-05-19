#ifndef COEFFICIENTS_H
#define COEFFICIENTS_H

#include<QtWidgets>
#include <complex>
using namespace std;


qreal theta_i(QLineF ray, QLineF wall);
complex<qreal> z_2(qreal eps_rel, qreal conduct);
complex<qreal> gamma_m(qreal eps_rel, qreal sigma);
qreal theta_t_snell(qreal theta_i, qreal eps1, qreal eps_wall);
qreal abs_tot_reflexion_coef(qreal theta_i, qreal eps_rel, qreal sigma, qreal thickness);
qreal abs_tot_transmission_coef(qreal theta_i, qreal eps_rel, qreal sigma, qreal thickness);
qreal abs_tot_reflexion_coef_5G(qreal theta_i, qreal eps_rel);
qreal abs_reflexion_coef_wall(qreal theta_i, qreal eps_rel);
qreal abs_reflexion_coef_ground(qreal TX_h, qreal RX_h, qreal d, qreal eps_rel);

#endif // COEFFICIENTS_H

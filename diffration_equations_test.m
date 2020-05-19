clear all; close all;

beta = (2*pi*26e9)/(3e8);
mu = @(dr) sqrt(2*beta*dr/pi);

dr_vec = 0:200;
mu_vector = mu(dr_vec);

Fv2 =  1 ./ ( (sqrt((mu_vector-0.1).^2 + 1) + mu_vector - 0.1) * 2.2131 );
Lke = 10*log10(Fv2);

plot(mu_vector, Lke);


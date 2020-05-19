clear all; close all;

f = 26e9;
d1 = 0:1:100000;
K = ((3e8/(26e9*pi))^2)*(60*0.1*(16/(3*pi)))/(71*8);
hTX = 1.6; hRX = 1.6;
thetai = (pi/2)-atan( (hTX+hRX)./d1);
d2 = sqrt( (hTX+hRX)^2 + d1.^2);

eps_r = 5;
a = sqrt(1/eps_r)*sqrt(1 - (sin(thetai).^2)/eps_r);

gamma_par = (cos(thetai) - a) ./ (cos(thetai) + a);
beta = (2*pi*f)/3e8;

sum = (exp(-1i*beta.*d1)./d1) + (gamma_par.*exp(-1i*beta.*d2)) ./ d2;
Egnd = (gamma_par.*exp(-1i*beta.*d2)) ./ d2;
Prx = K*abs(sum).^2;
Prx_dBm = 10*log10(Prx/0.001);

semilogx(d1, Prx_dBm);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sum = (1./d1) + (gamma_par) ./ d2;
Egnd2 = (gamma_par) ./ d2;

Prx = K*abs(sum).^2;
Prx_dBm = 10*log10(Prx/0.001);
%hold on;
%semilogx(d1, Prx_dBm);
%%%%%%%%%%%%%%%%%%%%%%%%%%
Edir = exp(-1i*beta.*d1)./d1;

thetai1 = atan( (d1./2)/ 5);
eps_r = 4.5;
a = sqrt(eps_r)*sqrt(1 - (sin(thetai1).^2)/eps_r);
gamma_perp = (cos(thetai1) - a) ./ (cos(thetai1) + a);
dn = sqrt(d1.^2+(10)^2);
E11 = gamma_perp.*exp(-1i.*dn)./dn;
%
eps_r = 3.7;
a = sqrt(eps_r)*sqrt(1 - (sin(thetai1).^2)/eps_r);
gamma_perp = (cos(thetai1) - a) ./ (cos(thetai1) + a);
dn = sqrt(d1.^2+(10)^2);
E12 = gamma_perp.*exp(-1i.*dn)./dn;
%
thetai2 = atan( (d1./4)/ 5);
eps_r1 = 3.7; eps_r2 =4.5;
a1 = sqrt(eps_r1)*sqrt(1 - (sin(thetai2).^2)/eps_r1);
gamma_perp1 = (cos(thetai2) - a1) ./ (cos(thetai2) + a1);
dn = sqrt(d1.^2+(20)^2);
a2 = sqrt(eps_r2)*sqrt(1 - (sin(thetai2).^2)/eps_r2);
gamma_perp2 = (cos(thetai2) - a2) ./ (cos(thetai2) + a2);
E2 = gamma_perp1 .* gamma_perp2 .* exp(-1i.*dn)./dn;

sum = Edir + E11 + E12 + E2*2 + Egnd;
Prx = K*abs(sum).^2;
Prx_dBm = 10*log10(Prx/0.001);

hold on;
semilogx(d1, Prx_dBm);

%%%%%%%%%%%%%%%%%%%%%%%%%%
Edir = 1./d1;

thetai1 = atan( (d1./2)/ 5);
eps_r = 4.5;
a = sqrt(eps_r)*sqrt(1 - (sin(thetai1).^2)/eps_r);
gamma_perp = (cos(thetai1) - a) ./ (cos(thetai1) + a);
dn = sqrt(d1.^2+(10)^2);
E11 = gamma_perp./dn;
%
eps_r = 3.7;
a = sqrt(eps_r)*sqrt(1 - (sin(thetai1).^2)/eps_r);
gamma_perp = (cos(thetai1) - a) ./ (cos(thetai1) + a);
dn = sqrt(d1.^2+(10)^2);
E12 = gamma_perp./dn;
%
thetai2 = atan( (d1./4)/ 5);
eps_r1 = 3.7; eps_r2 =4.5;
a1 = sqrt(eps_r1)*sqrt(1 - (sin(thetai2).^2)/eps_r1);
gamma_perp1 = (cos(thetai2) - a1) ./ (cos(thetai2) + a1);
dn = sqrt(d1.^2+(20)^2);
a2 = sqrt(eps_r2)*sqrt(1 - (sin(thetai2).^2)/eps_r2);
gamma_perp2 = (cos(thetai2) - a2) ./ (cos(thetai2) + a2);
E2 = gamma_perp1 .* gamma_perp2./dn;

sum = abs(Edir) + abs(E11) + abs(E12) + abs(E2)*2;
Prx = K*abs(sum).^2;
Prx_dBm = 10*log10(Prx/0.001);

hold on;
semilogx(d1, Prx_dBm);


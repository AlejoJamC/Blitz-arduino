[xn fs]=audioread('D:\UDI\proyecto\recursos\sonidos simulador\110101_010.mp3');
nf=1024; %number of point in DTFT
Y = fft(xn,nf);
f = fs/2*linspace(0,1,nf/2+1);
plot(f,abs(Y(1:nf/2+1)));
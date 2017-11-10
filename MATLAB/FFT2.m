[y,fs]=audioread('D:\UDI\proyecto\recursos\sonidos simulador\110101_011.wav');
t=linspace(0,length(y)/fs,length(y));
subplot(211); plot(t,y,'k'); axis tight
Nfft=2000;
f=linspace(0,fs,Nfft);
G=abs(fft(y,Nfft));
subplot(212); plot(f(1:Nfft/2),G(1:Nfft/2)); axis tight
filename = 'Mitral_Stenosis-_slow.wav';
[y,Fs] = audioread(filename);
figure(1),plot(y(:,1));
%figure(2),plot(y(:,2));
L=length(y(:,1));
NFFT = 2^nextpow2(L); % Next power of 2 from length of y
ft = abs(fft(y(:,1),NFFT)/L);
f = Fs/2*linspace(0,1,NFFT/2+1);

figure(2),plot(f(1:2800),ft(1:2800));
 

filenameNormal = 'blitz.wav';
[yn,Fsn] = audioread(filenameNormal);
figure(3),plot(yn(:,1));
Ln = length(yn(:,1));
NFFTn = 2^nextpow2(Ln); % Next power of 2 from length of y
ftn = abs(fft(yn(:,1),NFFTn)/Ln);
fn = Fsn/2*linspace(0,1,NFFTn/2+1);

figure(4),plot(fn(1:2800),ftn(1:2800));

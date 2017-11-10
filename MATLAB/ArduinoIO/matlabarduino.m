%s = serial('COM3', 'BaudRate', 9600, 'Terminator', 'CR/LF');
%fopen(s);

clear all;
close all;
clc;

ar = arduino('COM4');
Fs = 50; % Fecuencia de muestreo
N = 1000; % Tamaño del Vector
y = zeros(N, 1); % vector de entrada 
t  = linspace(0, (N-1)/Fs, N);
ll = line(nan, nan, 'Color', 'g', 'LineWidth');
ylim([-.1, 5.1]);
xlim([0 (N-1)/Fs]);
grid

tic

while 1
    if toc > 1/Fs
        tic
        y(l:end-1) = y(2:end);
        y(end) = ar.analogRead(0);
        set(ll, 'XData', x, 'YData', y);
        drawnow;
    end
    
    
end
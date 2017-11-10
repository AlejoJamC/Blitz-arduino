close all
clc;
numero_muestras=10000;

filename = strcat('blitz_', rand);

y=zeros(1,numero_muestras+1); %Vector donde se guardarán los datos

%Inicializo el puerto serial que utilizaré
delete(instrfind({'Port'},{'COM5'}));
puerto_serial=serial('COM5');
puerto_serial.BaudRate=9600;
warning('off','MATLAB:serial:fscanf:unsuccessfulRead');

%Abro el puerto serial
fopen(puerto_serial); 

%Declaro un contador del número de muestras ya tomadas
contador_muestras=1;

fprintf('Inicial la lectura\n');

%{
%Creo una ventana para la gráfica
figure('Name','Serial communication: Matlab + Arduino. Mario Pérez Esteso')
title('SERIAL COMMUNICATION MATLAB+ARDUINO');
xlabel('Número de muestra');
ylabel('Voltaje (V)');
grid on;
hold on;
%}

%Bucle while para que tome y dibuje las muestras que queremos
fprintf('Impresion en tiempo real\n');
while contador_muestras<=numero_muestras
        ylim([-1 2.5]); 
        xlim([contador_muestras-20 contador_muestras+5]);
        valor_analogo=fscanf(puerto_serial,'%d')';
        voltaje = (valor_analogo)*5/1024;
        if voltaje >= 0
            y(contador_muestras)=voltaje;
        end
        
        %plot(contador_muestras,y(contador_muestras),'X-r');        
        %drawnow
        contador_muestras=contador_muestras+1;
end
fprintf('voltajes capturados\n');

%Cierro la conexión con el puerto serial y elimino las variables
fclose(puerto_serial); 
delete(puerto_serial);

close all

figure('Name','Comunicaciones serial Arduino - Matlab: RESULTADO')
title('COMUNICACION SERIAL ARDUINO - MATLAB: RESULTADO');
xlabel('Número de muestra');
ylabel('Voltaje (V)');
grid on;
hold on;

% Remove zero rows
y( all(~y,2), : ) = [];
% Remove zero columns
y( :, all(~y,1) ) = [];

plot(y)
fprintf('Muestreo de voltaje resultante\n');


filename = 'blitz.wav';
fprintf('archivo: %s \n', filename);

audiowrite(filename,y,4000);

%clear all;
close all
clear all
clc

y=zeros(1,1000); %Vector donde se guardarán los datos

%Inicializo el puerto serial que utilizaré
delete(instrfind({'Port'},{'COM4'}));
puerto_serial=serial('COM4');
puerto_serial.BaudRate=9600;
warning('off','MATLAB:serial:fscanf:unsuccessfulRead');

%Abro el puerto serial
fopen(puerto_serial); 

%Declaro un contador del número de muestras ya tomadas
contador_muestras=1;

%Creo una ventana para la gráfica
close all
fprintf('Inicial la lectura');
figure('Name','Comunicaciones serial Arduino - Matlab: Captura de datos')
title('COMUNICACION SERIAL ARDUINO - MATLAB: CAPTURA DE DATOS');
xlabel('Tiempo');
ylabel('Voltaje (V)');
grid on;
hold on;


Tmax = 10; % Total time for data collection (s)
Ts = 0.02; % Sampling time (s)
i = 0;
tic % Start timer
fprintf('Impresion en tiempo real');
%Bucle while para que tome y dibuje las muestras que queremos
while toc <= Tmax
    i = i + 1;
    ylim([0 2.5]); 
    xlim([0 Tmax+1]);
    valor_potenciometro=fscanf(puerto_serial,'%d')';
    voltage = (valor_potenciometro(1))*5/1024;
    y(i) = voltage;
    %y(contador_muestras)=voltage;
     
    % Read time stamp
    % If reading faster than sampling rate, force sampling time.
    % If reading slower than sampling rate, nothing can be done. Consider
    % decreasing the set sampling time Ts
    t(i) = toc;
    if i > 1
        T = toc - t(i-1);
        while T < Ts
            T = toc - t(i-1);
        end
    end
    t(i) = toc;
    
    if i > 1
        plot(toc,y(i),'-mo',...
        'LineWidth',2,...
        'MarkerEdgeColor','k',...
        'MarkerFaceColor',[.49 1 .63],...
        'MarkerSize',10); 
        
        drawnow
    end
    
end

fprintf('\n voltajes capturados\n');

%Cierro la conexión con el puerto serial y elimino las variables
fclose(puerto_serial); 
delete(puerto_serial);

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
fprintf('Muestreo de voltaje resultante');

%clear all;
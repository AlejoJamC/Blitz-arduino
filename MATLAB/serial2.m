delete(instrfind({'port'},{'COM4'}));

s = serial('COM4', 'BaudRate', 9600, 'Terminator', 'CR/LF');

fopen(s);


tmax=5;
rate=33;

f= figure('Name', 'Capture');
a  = axes('XLim', [0 tmax], 'YLim', [-1 5.1]);
l1 = line(nan,nan, 'Color', 'r', 'LineWidth', 2);
%l2 = line(nan,nan, 'Color', 'b', 'LineWidth', 2);

xlabel('Tiempo (s)');
ylabel('voltaje (V)');
title('Captura de voltaje en tiempo real con Arduino');
grid on;
hold on;

v1 = zeros(1, tmax*rate);
v2 = zeros(1, tmax*rate);
x2 = zeros(1, tmax*rate);
i=1;
t=0;

tic

while t < tmax
    t = toc;
    a= fscanf(s, '%d, %d');
    v1(i)=a(1)*5/1024;
    %v2(i)=a(2)*5/1024;
    x2= linspace(0, tmax, tmax*rate);
    x = linspace(0, i/rate, i);
    set(l1, 'YData', v1(1:i), 'XData', x);
    %set(l2, 'YData', v2(l:i), 'XData', x);
    drawnow;
    v1;
    datax = [x, v1(1:i)];
    i = i+1;
end


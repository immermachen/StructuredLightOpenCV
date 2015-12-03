h = imread('code.bmp');
hphase = imread('phase.bmp');
v = imread('v.bmp');
vphase = imread('vphase.bmp');

%Capture39 = imread('Capture-39.bmp');
%Capture40 = imread('Capture-40.bmp');

gray0 = imread('m_mGray0.PNG');
lineX = gray0(1000,1000:1050);
figure;
hold on;
x = 1000:1:1050;
plot(x,lineX');
plot(x,lineX','b.');
hold off;

lineX = gray0(1000,:);
figure;
hold on;
x = 1:1:2448;
plot(x,lineX');
plot(x,lineX','b.');
hold off;
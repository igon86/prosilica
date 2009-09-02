function [A,x_0,y_0,sigma_x,sigma_y,a,b,c] = fastGaussianFit( image,massimo,minimo,centro_x,centro_y,var_x,var_y)
%FASTGAUSSIANFIT 
%   Detailed explanation goes here -> un cio voglia

imag = double(image);

% %plot della roba da fittare
% figure(1);
% mesh(imag);
% title('toBeFitted');

%scelgo quanto iterare (ancora non funziona abbastanza bene da poter usare i residui come criterio di arresto)
iterazioni = 5;
R = zeros(iterazioni+1,1);


%parameter inizialization
A = massimo;
x_0 = centro_x;
y_0 = centro_y;
sigma_x = var_x;
sigma_y = var_y;
a = 0;
b = 0;
c = minimo;

%support data arrays inizialization
[dimx,dimy] = size(image);
img = imag(:);
[m,thrash]=size(img);
differenze = zeros(m,1);
immagine = zeros(m,1);
M = zeros(m,8);

% %predition plot
% for i=1:m
%     x = mod(i ,  dimx);
%     y = floor(i/dimx);
%     immagine(i) = valutaPunto(A,x_0,y_0,sigma_x,sigma_y,a,b,c,x,y);
% end
% predition = reshape(immagine,dimx,dimy);
% figure(2);
% mesh(predition);
% title('initialPredition');
% drawnow;

%%%%%%%%%%%%%%%%%%%%%%  ITERATION   %%%%%%%%%%%%%%%%%%%%%%%%%%%

for j=1:iterazioni

    for i=1:m
        x = mod(i ,  dimx);
        y = floor(i/dimx);
        
        test = valutaPunto(A,x_0,y_0,sigma_x,sigma_y,a,b,c,x,y);
        
        differenze(i) = img(i) - test;
        
        
        M(i,1) = 1/exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2);
        M(i,2) = (A*(2*x - 2*x_0))/(sigma_x^2*exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2));
        M(i,3) = (A*(2*y - 2*y_0))/(sigma_y^2*exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2));
        M(i,4) = (2*A*(x - x_0)^2)/(sigma_x^3*exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2));
        M(i,5) = (2*A*(y - y_0)^2)/(sigma_y^3*exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2));
        %derivative of the slopePlan!
        M(i,6) = x;
        M(i,7) = y;
        
    end
    
    M(:,8) = 1;
    
    %     %plot della differenza della predizione
    %     if j == 1
    %         figure(6);
    %         diffmap = reshape(differenze,dimx,dimy);
    %         mesh(diffmap);
    %         title('Predition residual map');
    %         drawnow;
    %     end
    
    %calcolo la matrice di iterazione a e b
    matrix = M'*M;
    
    vector = M'*differenze;
    
    %guardo il livello di schifezza
    R(j) = differenze'*differenze;
    
    %risolvo il sistema lineare per avere delta
    delta = matrix\vector;
    
    %aggiungo delta
    A = A+delta(1);
    x_0 = x_0 + delta(2);
    y_0 = y_0 + delta(3);
    sigma_x = sigma_x + delta(4);
    sigma_y = sigma_y + delta(5);
    a = a+delta(6);
    b = b+delta(7);
    c = c+delta(8);
end

%%%%%%%%%%%%%%%%%%%%%%%%%     FINAL PLOTS      %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%calcolo l'immagine ottenuta dal fit
for i=1:m
    x = mod(i ,  dimx);
    y = floor(i/dimx);
    immagine(i) = valutaPunto(A,x_0,y_0,sigma_x,sigma_y,a,b,c,x,y);
end

% %plot3D iterata
% gaussiana = reshape(immagine,dimx,dimy);
% 
% figure(3);
% mesh(real(gaussiana));
% title('fittedSolution');

%calcolo mappa residui

differenze = img - immagine;

diffmap = reshape(differenze,dimx,dimy);
mesh(diffmap);
% compensated_diff = differenze./(1+alpha*differenze);
% compensated_diffmap = reshape(compensated_diff,dimx,dimy);

%calcolo SSerr
R(iterazioni+1) = differenze' * differenze;
SSerr = R(iterazioni+1);

%calcolo SStot
f_avg = mean(img);
SStot = 0;
for i = 1:m
    SStot = SStot + (img(i) - f_avg)^2;
end
%calcolo R2
R2 = 1 - (SSerr/SStot);


end

    
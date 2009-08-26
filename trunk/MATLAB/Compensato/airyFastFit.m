function [R2] = airyFastFit( image,massimo,minimo,centro_x,centro_y,var_x,var_y,stat)
%AIRYFASTFIT Function which determine the best alpha correction parameter
%for an image
%   Detailed explanation goes here -> un cio voglia

imag = double(image);

% %plot della roba da fittare
% figure(1);
% mesh(imag);
% title('toBeFitted');

%scelgo quanto iterare (ancora non funziona abbastanza bene da poter usare i residui come criterio di arresto)
iterazioni = 10;
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
    %a questo punto calcolo il vettore delle differenze e la matrice M
    %fprintf(1,'Iterazione %d\n',j);
    for i=1:m
        x = mod(i ,  dimx);
        y = floor(i/dimx);
        
        test = valutaPunto(A,x_0,y_0,sigma_x,sigma_y,a,b,c,x,y);
        
        differenze(i) = img(i) - test;
        
        %             r = (((x-x_0)^2) + ((y-y_0)^2))^0.5;
        %             % I teach matlab how sin(x)/x works
        %             limite_1 = sin(beta*(r))/(beta*r);
        %             if isnan(limite_1)
        %                 limite_1 = 1;
        %             end
        
        
        M(i,1) = sin((sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^(1/2))^2/(sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2);
        M(i,2) = (A*sigma_x*sin((sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^(1/2))^2*(2*x - 2*x_0))/(sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^2 - (A*sigma_x*cos((sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^(1/2))*sin((sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^(1/2))*(2*x - 2*x_0))/(sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^(3/2);
        M(i,3) = (A*sigma_y*sin((sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^(1/2))^2*(2*y - 2*y_0))/(sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^2 - (A*sigma_y*cos((sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^(1/2))*sin((sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^(1/2))*(2*y - 2*y_0))/(sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^(3/2);
        M(i,4) = (A*cos((sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^(1/2))*sin((sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^(1/2))*(x - x_0)^2)/(sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^(3/2) - (A*sin((sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^(1/2))^2*(x - x_0)^2)/(sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^2;
        M(i,5) = (A*cos((sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^(1/2))*sin((sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^(1/2))*(y - y_0)^2)/(sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^(3/2) - (A*sin((sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^(1/2))^2*(y - y_0)^2)/(sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2)^2;
        
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
    
    
    %         for cazzo = 1:7
    %             for topa=1:7
    %                 X = M(:,cazzo).*M(:,topa);
    %                 %fprintf(1,'vettore di %d - %d',cazzo,topa);
    %                 matrix(cazzo,topa) = sum(X);
    %             end
    %         end
    
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

%plot3D iterata
% gaussiana = reshape(immagine,dimx,dimy);
%
% figure(3);
% mesh(real(gaussiana));
% title('fittedSolution');

%calcolo mappa residui

differenze = img - immagine;

% diffmap = reshape(differenze,dimx,dimy);
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

%     chi = 0;
%     for i=1:m
%         chi = chi + (differenze(i)^2)/immagine(i);
%     end

% %plot2D residui
% figure(4);
% if iterazioni > 10
%     loglog(R);
% else
%     semilogy(R);
% end
% title(['square residuals, R2 is: ',num2str(SSerr)]);


% %plot(mappa residui)
% figure(5);
% mesh(compensated_diffmap);
% dimension = min(m,mask_x*mask_y);
% %goodness = R2/dimension;
% %goodness = SSerr/dimesion;
% %goodness = chi/dimension;
% title(['Residuals map, goodness of fit is: ',num2str(R2)]);

%%%%%%%%%%%%%%%%%%%%%%%%%     STATISTICS  x_0    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if stat == 1
    
    
    %confidence of x
    
    stat_thres = 5;
    threshold = SSerr*((100+stat_thres)/100);
    epsilon = SSerr/100;
    upthreshold = threshold + epsilon;
    lowthreshold = threshold - epsilon;
    
    %left border -> algoritmo di bisezione
    
    left = 0;
    right = x_0;
    center = x_0/2;
    quadrati = 2*SSerr;
    
    iterazione=1;
    
    while quadrati > upthreshold || quadrati < lowthreshold
        %fprintf(1,'Iterazione %d con residuo %d che lavora su %d   %d  %d\n',iterazione,quadrati,left,center,right);
        %adjustment of the section
        if quadrati > upthreshold
            left = center;
            center = left + ((right - left)/2);
        else
            right = center;
            center = left + ((right - left)/2);
        end
        
        %new error calculation
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = valutaPunto(A,center,y_0,sigma_x,sigma_y,a,b,c,x,y);
        end
        
        differenze = img - immagine;
        quadrati = differenze' * differenze;
        
        iterazione = iterazione +1;
    end
    
    left_confidence_x = center
    left_spot_x = quadrati
    
    x_0
    
    %right border -> algoritmo di bisezione
    
    left = x_0;
    right = dimx;
    center = left + (right-left)/2;
    quadrati = 2*SSerr;
    
    iterazione=1;
    
    while quadrati > upthreshold || quadrati < lowthreshold
        %fprintf(1,'Iterazione %d con residuo %d che lavora su %d   %d  %d\n',iterazione,quadrati,left,center,right);
        %adjustment of the section
        if quadrati > upthreshold
            right = center;
            center = left + (right - left)/2;
        else
            left = center;
            center = left + (right - left)/2;
        end
        
        %new error calculation
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = valutaPunto(A,center,y_0,sigma_x,sigma_y,a,b,c,x,y);
        end
        
        differenze = img - immagine;
        quadrati = differenze' * differenze;
    end
    
    right_confidence_x = center
    right_spot_x = quadrati
    
    % plot su x
    s = 100;
    x_s = linspace(x_0 -10,x_0 +10,s);
    
    stat_x = zeros(s,1);
    for j=1:s
        %calcolo l'immagine shiftata
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = valutaPunto(A,x_s(j),y_0,sigma_x,sigma_y,a,b,c,x,y);
        end
        
        differenze = img - immagine;
        quadrati = differenze' * differenze;
        stat_x(j) = quadrati;
        %         if(quadrati < R2)
        %             fprintf('Non ha convergiuto completamente, R: %d',quadrati);
        %         end
    end
    
    figure(7);
    plot(x_s,stat_x,left_confidence_x,left_spot_x,'ro',right_confidence_x,right_spot_x,'ro')
    title([' chi of x, x_0 is: ',num2str(x_0),' and is between ',num2str(left_confidence_x),' and ',num2str(right_confidence_x)]);
    
    %     figure(8);
    %     semilogy(x_s,stat_x)
    %     title(['confidence of x, x_0 is: ',num2str(x_0),' (ylogscale)']);
    
    
    
%%%%%%%%%%%%%%%%%%%%%%%%%     STATISTICS  y_0    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    
    
    
    
    %confidence of y
    
    stat_thres = 5;
    threshold = SSerr*((100+stat_thres)/100);
    epsilon = SSerr/100;
    upthreshold = threshold + epsilon;
    lowthreshold = threshold - epsilon;
    
    %left border -> algoritmo di bisezione
    
    left = 0;
    right = y_0;
    center = y_0/2;
    quadrati = 2*SSerr;
    
    iterazione=1;
    
    while quadrati > upthreshold || quadrati < lowthreshold
        %fprintf(1,'Iterazione %d con residuo %d che lavora su %d   %d  %d\n',iterazione,quadrati,left,center,right);
        %adjustment of the section
        if quadrati > upthreshold
            left = center;
            center = left + ((right - left)/2);
        else
            right = center;
            center = left + ((right - left)/2);
        end
        
        %new error calculation
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = valutaPunto(A,x_0,center,sigma_x,sigma_y,a,b,c,x,y);
        end
        
        differenze = img - immagine;
        quadrati = differenze' * differenze;
        
        iterazione = iterazione +1;
    end
    
    left_confidence_y = center;
    left_spot_y = quadrati
    
    y_0
    
    %right border -> algoritmo di bisezione
    
    left = y_0;
    right = dimy;
    center = left + (right-left)/2;
    quadrati = 2*SSerr;
    
    iterazione=1;
    
    while quadrati > upthreshold || quadrati < lowthreshold
        %fprintf(1,'Iterazione %d con residuo %d che lavora su %d   %d  %d\n',iterazione,quadrati,left,center,right);
        %adjustment of the section
        if quadrati > upthreshold
            right = center;
            center = left + (right - left)/2;
        else
            left = center;
            center = left + (right - left)/2;
        end
        
        %new error calculation
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = valutaPunto(A,x_0,center,sigma_x,sigma_y,a,b,c,x,y);
        end
        
        differenze = img - immagine;
        quadrati = differenze' * differenze;
    end
    
    right_confidence_y = center
    right_spot_y = quadrati
    
    % plot su x
    s = 100;
    x_s = linspace(x_0 -10,x_0 +10,s);
    
    stat_x = zeros(s,1);
    for j=1:s
        %calcolo l'immagine shiftata
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = valutaPunto(A,x_s(j),y_0,sigma_x,sigma_y,a,b,c,x,y);
        end
        
        differenze = img - immagine;
        quadrati = differenze' * differenze;
        stat_x(j) = quadrati;
        %         if(quadrati < R2)
        %             fprintf('Non ha convergiuto completamente, R: %d',quadrati);
        %         end
    end
    
    figure(8);
    plot(x_s,stat_x,left_confidence_y,left_spot_y,'ro',right_confidence_y,right_spot_y,'ro')
    title([' chi of y, y_0 is: ',num2str(y_0),' and is between ',num2str(left_confidence_y),' and ',num2str(right_confidence_y)]);
    
end
end

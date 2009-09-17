function [A,x_0,y_0,sigma_x,sigma_y,a,b,c] = oneShotAiryFit(image,massimo,minimo,centro_x,centro_y,var_x,var_y,pa,pb)
%ONESHOTAIRYFIT Function which determine the best alpha correction parameter
%for an image
%   Detailed explanation goes here -> un cio voglia

imag = double(image);

%parameter inizialization
A = massimo;
x_0 = centro_x;
y_0 = centro_y;
sigma_x = var_x;
sigma_y = var_y;
a = pa;
b = pb;
c = minimo;

%support data arrays inizialization
dimx = size(image,1);
img = imag(:);
m=size(img,1);
differenze = zeros(m,1);

M = zeros(m,8);

%%%%%%%%%%%%%%%%%%%%%%  ITERATION   %%%%%%%%%%%%%%%%%%%%%%%%%%%

    for i=1:m
        x = mod(i ,  dimx);
        y = floor(i/dimx);
        
        test = valutaAiry(A,x_0,y_0,sigma_x,sigma_y,a,b,c,x,y);
        
        differenze(i) = img(i) - test;
        
        %             r = (((x-x_0)^2) + ((y-y_0)^2))^0.5;
        %             % I teach matlab how sin(x)/x works
        %             limite_1 = sin(beta*(r))/(beta*r);
        %             if isnan(limite_1)
        %                 limite_1 = 1;
        %             end
        
        
        Z = (sigma_x*(x - x_0)^2 + sigma_y*(y - y_0)^2);
        Z2 = Z^2;
        Z3 = Z^(1.5);
        r = Z^(0.5);
        coso = cos(r);
        sino = sin(r);
        
        M(i,1) = sino^2/(Z);
        M(i,2) = (A*sigma_x*sino^2*(2*x - 2*x_0))/Z2 - (A*sigma_x*coso*sino*(2*x - 2*x_0))/Z3;
        M(i,3) = (A*sigma_y*sino^2*(2*y - 2*y_0))/Z2 - (A*sigma_y*coso*sino*(2*y - 2*y_0))/Z3;
        M(i,4) = (A*coso*sino*(x - x_0)^2)/Z3 - (A*sino^2*(x - x_0)^2)/Z2;
        M(i,5) = (A*coso*sino*(y - y_0)^2)/Z3 - (A*sino^2*(y - y_0)^2)/Z2;
        
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
    
%     %guardo il livello di schifezza
%     R(j) = differenze'*differenze;
    
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

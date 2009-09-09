%max,x_0,y_0 vengono dalla preelaborazione in C, sigma_x e sigma_y si
%possono ricavare dall'immagine come bordi della maschera (forse...)
function [A,x_0,y_0,sigma_x,sigma_y,a,b,c] = gaussianFit(image,max,min,centro_x,centro_y,var_x,var_y)
    
    %plot della roba da fittare
    figure(1);
    imag = double(image);
    mesh(imag);
    title('toBeFitted');
    
    %scelgo quanto iterare (ancora non funziona abbastanza bene da poter usare i residui come criterio di areesto)
    iterazioni = 1;
    R = zeros(iterazioni,1);
    
    %parameter inizialization
    A = max;
    x_0 = centro_x;
    y_0 = centro_y;
    sigma_x = var_x;
    sigma_y = var_y;
    a = 0;
    b = 0;
    c = min;
    
    %support data arrays inizialization
    [dimx,dimy] = size(image);
    img = imag(:);
    m=size(img);
    differenze = zeros(m,1);
    immagine = zeros(m,1);
    M = zeros(m,6);
    
    %predition plot
    for i=1:m
        x = mod(i ,  dimx);
        y = floor(i/dimx);
        immagine(i) = valutaPunto(A,x_0,y_0,sigma_x,sigma_y,a,b,c,x,y);
    end
    predition = reshape(immagine,dimx,dimy);
    figure(2);
    mesh(predition);
    title('initialPredition (C++ RMS pre-elaboration)');
    drawnow;
    %lo faccio girare per un po
    for j=1:iterazioni
        %a questo punto calcolo il vettore delle differenze e la matrice M
        %fprintf(1,'Iterazione %d\n',j);
        for i=1:m
            x = mod(i-1,  dimx);
            y = floor(i/dimx);
            test = valutaPunto(A,x_0,y_0,sigma_x,sigma_y,a,b,c,x,y);
            %tutto il problema e` qui! -> il round e` una vera merda
%             if mask(i) == 1
                   differenze(i) = img(i) - test;
%             end
            M(i,1) = 1/exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2);
            M(i,2) = (A*(2*x - 2*x_0))/(sigma_x^2*exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2));
            M(i,3) = (A*(2*y - 2*y_0))/(sigma_y^2*exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2));
            M(i,4) = (2*A*(x - x_0)^2)/(sigma_x^3*exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2));
            M(i,5) = (2*A*(y - y_0)^2)/(sigma_y^3*exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2));
            %derivative of the slopePlan!
            M(i,6) = x;
            M(i,7) = y;
            M(i,8) = 1;
        end
        for index1 = 1:8
            fprintf(1,'%f ',M(2,index1));
        end
        fprintf(1,'\n');

        %plot della differenza della predizione
        if j == 1
            figure(6);
            diffmap = reshape(differenze,dimx,dimy);
            mesh(diffmap);
            title('Predition residual map');
        end
        %calcolo la matrice di iterazione a e b
        matrix = M'*M;
        for k=1:8
        fprintf(1,'%8f\t %8f\t %8f\t %8f\t %8f\t %8f\t %8f\t %8f\n',matrix(k,1),matrix(k,2),matrix(k,3),matrix(k,4),matrix(k,5),matrix(k,6),matrix(k,7),matrix(k,8));
        end
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
    
    
    
    %plot3D iterata
    for i=1:m
        x = mod(i ,  dimx);
        y = floor(i/dimx);
        immagine(i) = valutaPunto(A,x_0,y_0,sigma_x,sigma_y,a,b,c,x,y);
    end
    gaussiana = reshape(immagine,dimx,dimy);
    figure(3);
    mesh(gaussiana);
    title('fittedSolution');
    
   
    %stampo errore
    R2 = R(iterazioni);
    
    %plot2D residui
    figure(4);
    plot(R);
    title(['R2 square residuals, R2 is: ',num2str(R2)]);    
    
    %plot(mappa residui)
    figure(5);
    %calcolo per l'ultima volta
    differenze = img - immagine;
    diffmap = reshape(differenze,dimx,dimy);
    mesh(diffmap);
    title('Residua map');
end
    
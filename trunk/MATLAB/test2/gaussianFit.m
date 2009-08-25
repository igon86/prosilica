%max,x_0,y_0 vengono dalla preelaborazione in C, sigma_x e sigma_y si
%possono ricavare dall'immagine come bordi della maschera (forse...)
function [A,b,x_0,y_0,sigma_x,sigma_y,differenze] = gaussianFit(image,max,min,centro_x,centro_y,var_x,var_y)
    
    %plot della roba da fittare
    figure(1);
    imag = double(image);
    mesh(imag);
    title('toBeFitted');
    
    %scelgo quanto iterare (ancora non funziona abbastanza bene da poter usare i residui come criterio di areesto)
    iterazioni = 10;
    R = zeros(iterazioni,1);
    
    %inizializzazione parametri a partire da quelli preelaborati
    A = max;
    x_0 = centro_x;
    y_0 = centro_y;
    sigma_x = var_x;
    sigma_y = var_y;
    b = min;
    
    %srotolamento dell'immagine come vettore e inizializzazione vettori di
    %supporto
    [dimx,dimy] = size(image);
    img = imag(:);
    m=size(img);
    differenze = zeros(m,1);
    immagine = zeros(m,1);
    M = zeros(m,6);
    
    %plot predizione
    for i=1:m
        x = mod(i ,  dimx);
        y = floor(i/dimx);
        immagine(i) = valutaPunto(A,b,x_0,y_0,sigma_x,sigma_y,x,y);
    end
    predition = reshape(immagine,dimx,dimy);
    figure(2);
    mesh(predition);
    title('initialPredition (C++ RMS pre-elaboration)');
    drawnow;
    
    %lo faccio girare per un po
    for j=1:iterazioni
        j
        %a questo punto calcolo il vettore delle differenze e la matrice M
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            test = valutaPunto(A,b,x_0,y_0,sigma_x,sigma_y,x,y);
            %tutto il problema e` qui! -> il round e` una vera merda
            differenze(i) = (double(img(i)) - test);
                
            M(i,1) = 1/exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2);
            M(i,2) = (A*(2*x - 2*x_0))/(sigma_x^2*exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2));
            M(i,3) = (A*(2*y - 2*y_0))/(sigma_y^2*exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2));
            M(i,4) = (2*A*(x - x_0)^2)/(sigma_x^3*exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2));
            M(i,5) = (2*A*(y - y_0)^2)/(sigma_y^3*exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2));
            %derivative of a constant!
            M(i,6) = 1;
        end
        %plot della differenza della predizione
        if j == 1
            figure(6);
            diffmap = reshape(differenze,dimx,dimy);
            mesh(double(diffmap));
            title('Predition residual map');
        end
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
        b = b+delta(6);
    end
    
    
    
    %plot3D iterata
    for i=1:m
        x = mod(i ,  dimx);
        y = floor(i/dimx);
        immagine(i) = valutaPunto(A,b,x_0,y_0,sigma_x,sigma_y,x,y);
    end
    gaussiana = reshape(immagine,dimx,dimy);
    figure(3);
    mesh(gaussiana);
    title('fittedSolution');
    
   
    %stampo errore
    R2 = R(iterazioni)
    
    %plot2D residui
    figure(4);
    plot(R);
    title(['R2 square residuals, R2 is: ',num2str(R2)]);    
    
    %plot(mappa residui)
    figure(5);
    %calcolo per l'ultima volta
    differenze = double(img) - immagine;
    diffmap = reshape(differenze,dimx,dimy);
    mesh(double(diffmap));
    title('Residua map');
end
    
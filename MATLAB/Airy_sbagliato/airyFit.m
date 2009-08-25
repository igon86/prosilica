%max,x_0,y_0 vengono dalla preelaborazione in C, sigma_x e sigma_y si
%possono ricavare dall'immagine come bordi della maschera (forse...)
function [A,x_0,y_0,beta,a,b,c] = airyFit(image,massimo,minimo,centro_x,centro_y,varianza)
    
    %plot della roba da fittare
    figure(1);
    imag = double(image);
    mesh(imag);
    title('toBeFitted');
    
    %scelgo quanto iterare (ancora non funziona abbastanza bene da poter usare i residui come criterio di areesto)
    iterazioni = 50;
    R = zeros(iterazioni,1);
    
    %parameter inizialization
    A = massimo;
    x_0 = centro_x;
    y_0 = centro_y;
    beta = varianza;
    a = 0;
    b = 0;
    c = minimo;
    
    %support data arrays inizialization
    [dimx,dimy] = size(image);
    img = imag(:);
    [m,thrash]=size(img);
    differenze = zeros(m,1);
    immagine = zeros(m,1);
    M = zeros(m,6);
    
    %predition plot
    for i=1:m
        x = mod(i ,  dimx);
        y = floor(i/dimx);
        immagine(i) = valutaPunto(A,x_0,y_0,beta,a,b,c,x,y);
    end
    predition = reshape(immagine,dimx,dimy);
    figure(2);
    mesh(predition);
    title('initialPredition');
    drawnow;

    %%%%%%%%%%%%%%%%%%%%%%  ITERATION   %%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    for j=1:iterazioni
        %a questo punto calcolo il vettore delle differenze e la matrice M
        %fprintf(1,'Iterazione %d\n',j);
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            
            test = valutaPunto(A,x_0,y_0,beta,a,b,c,x,y);
           
            differenze(i) = img(i) - test;
            
            r = (((x-x_0)^2) + ((y-y_0)^2))^0.5;
            % I teach matlab how sin(x)/x works
            limite_1 = sin(beta*(r))/(beta*r);
            if isnan(limite_1)
                limite_1 = 1;
            end
            
            alfonso = ( (x_0 -x)/(r^(3/2)) );
            astolfo = ( (y_0 -y)/(r^(3/2)) );
            M(i,1) = limite_1;
            M(i,2) = A*(((cos(beta*r)) *alfonso ) - limite_1 *alfonso );
            M(i,3) = A*(((cos(beta*r)) *astolfo )- limite_1 *astolfo );
            M(i,4) = A*( ( (cos(beta*r))/(beta) )  - ((sin(beta*r))/(beta^2*r) ) );
           
            %derivative of the slopePlan!
            M(i,5) = x;
            M(i,6) = y;
            M(i,7) = 1;
        end

        %plot della differenza della predizione
        if j == 1
            figure(6);
            diffmap = reshape(differenze,dimx,dimy);
            mesh(diffmap);
            title('Predition residual map');
        end
        
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
        beta = beta + delta(4);
        a = a+delta(5);
        b = b+delta(6);
        c = c+delta(7);
    end
    
    %%%%%%%%%%%%%%%%%%%%%%%%%     FINAL PLOTS      %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    %plot3D iterata
    for i=1:m
        x = mod(i ,  dimx);
        y = floor(i/dimx);
        immagine(i) = valutaPunto(A,x_0,y_0,beta,a,b,c,x,y);
    end
    gaussiana = reshape(immagine,dimx,dimy);
    figure(3);
    mesh(gaussiana);
    title('fittedSolution');
    
   
    %stampo errore
    R2 = R(iterazioni);
    
    %plot2D residui
    figure(4);
    semilogy(R);
    title(['R2 square residuals, R2 is: ',num2str(R2)]);    
    
     %plot(mappa residui)
    figure(5);
    %calcolo per l'ultima volta
    mask_x = 1000;
    mask_y = 1000;
    if (x-x_0)^2/mask_x^2 + (y-y_0)^2/mask_y^2 < 1 
        differenze = img - immagine;
    else
        differenze(i)=0;
    end
    diffmap = reshape(differenze,dimx,dimy);
    mesh(diffmap);
    dimension = min(m,mask_x*mask_y);
    goodness = R2/dimension;
    title(['Residuals map, goodness of fit is: ',num2str(goodness)]);
    
    %%%%%%%%%%%%%%%%%%%%%%%%%     STATISTICS      %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    % statistiche su x
    s = 10;
    x_s = linspace(x_0 -10,x_0 +10,s);
    
    stat_x = zeros(s,1);
    for j=1:s
        %calcolo l'immagine shiftata
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = valutaPunto(A,x_s(j),y_0,beta,a,b,c,x,y);
        end
        
        differenze = img - immagine;
        quadrati = differenze' * differenze;
        stat_x(j) = quadrati;
        if(quadrati < R2)
            fprintf('Non ha convergiuto completamente, R: %d',quadrati);
        end
    end
    
    figure(7);
    plot(x_s,stat_x)
    title(['confidence of x, x_0 is: ',num2str(x_0)]);
    
%     figure(8);
%     semilogy(x_s,stat_x)
%     title(['confidence of x, x_0 is: ',num2str(x_0),' (ylogscale)']);
end